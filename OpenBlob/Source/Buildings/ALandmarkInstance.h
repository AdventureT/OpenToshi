#pragma once
#include "ABuildingInstance.h"

#include <Physics/TPhysicsResponder.h>
#include <Math/TMatrix44.h>

namespace Toshi
{
class TXUIScene;
class TXUIElement;
class TXUIFigure;
class TMaterial;
class TTexture;
class TMesh;
class TParticleEffect;
} // namespace Toshi

class AFXBreakpointComponent;
class AEnvEmitter;

//-----------------------------------------------------------------------------
// Purpose: a landmark (cell instance type 2, TerrainTypes::InstanceType_Landmark): a building Blob paints by bouncing
// on its triggers (the "Landmark_Target_New" models with a count of the paint left above them). When every trigger is
// painted, the landmark completes: its idle model is swapped for the transform model, the score is added and
// ALandmarkChallenge is told (AChallengeManager's landmark events). A billboard (LandmarkData::Flags_BillBoard) has no
// triggers: Blob touching it transforms it. Created by AModel2::CreateInstance (type 2), its terrain callbacks are in
// ATerrain2Interface's per type tables.
//-----------------------------------------------------------------------------
class ALandmarkInstance
    : public ABuildingInstance
{
public:
	static TCONSTEXPR_VAR TINT MAX_LANDMARK_MESHES           = 20;
	static TCONSTEXPR_VAR TINT MAX_LANDMARK_ACCESSORY_MESHES = 30;

	// m_uiFlags (on top of ABuildingInstance's)
	enum LandmarkFlags : TUINT32
	{
		Flags_TriggersHidden   = BITFLAG( 13 ), // the triggers are hidden (SetTriggersHidden)
		Flags_Completed        = BITFLAG( 14 ), // transformed
		Flags_MeshesMatched    = BITFLAG( 15 ), // m_aiMeshMaterial was built (MatchMeshes)
		Flags_LetterboxEnded   = BITFLAG( 16 ), // the letterbox ended: Blob leaves the landmark state
		Flags_PowerGen         = BITFLAG( 17 ), // "powergen_main": the breakpoint FX are seen from further away
		Flags_WaitForLetterbox = BITFLAG( 18 ), // the completion waits for the HUD to show the letterbox
	};

	// The data of a landmark (CellInstance::m_pTypeData)
	struct LandmarkData
	{
		enum Flags : TUINT8
		{
			Flags_BillBoard = BITFLAG( 0 ),
		};

		struct TriggerData // 0xC bytes
		{
			Toshi::TMatrix44* m_pMatrix;  // 0x00
			Toshi::TVector4*  m_pExtents; // 0x04 two TVector3: the min and the max of the trigger's box
			TINT16            m_iColour;  // 0x08 ABlobColourComponent::COLOUR, 0: any colour
		};

		const TCHAR*  m_szIdleModel;       // 0x00
		const TCHAR*  m_szTransformModel;  // 0x04
		const TCHAR** m_ppMeshNames;       // 0x08 the materials of the low LOD meshes painted
		const TCHAR** m_ppTextureNames;    // 0x0C one per m_ppMeshNames
		TriggerData*  m_pTriggers;         // 0x10
		TUINT8        m_uiFlags;           // 0x14 Flags
		TUINT8        m_uiNumTriggers;     // 0x15
		TUINT8        m_uiNumMeshes;       // 0x16
		TUINT8        m_uiPaintPerTrigger; // 0x17 the bounces a trigger needs
		TFLOAT        m_fTransformTime;    // 0x18
		TFLOAT        m_fBlobDelay;        // 0x1C added to the letterbox time when the player's blob data says so
	};

	// A model of the landmark (the idle, transform and trigger models)
	struct ModelRef
	{
		ALandmarkInstance* m_pOwner;    // 0x00
		AModelInstance2*   m_pInstance; // 0x04
	};

	// A paint target of the landmark (0x8C bytes)
	struct Trigger
	{
		// Receives the contacts of the trigger's bodies
		class CollisionHandler
		    : public Toshi::TPhysicsResponder
		{
		public:
			CollisionHandler( Trigger* a_pTrigger )
			    : m_pTrigger( a_pTrigger ) {}

			// $deBlob: FUNCTION 00580280
			// $deBlob: FUNCTION 0059ce90
			// $deBlob: FUNCTION 0059fd60
			virtual ~CollisionHandler() override = default;

			virtual void OnCollisionConfirmed( Toshi::TPhysicsCollisionInfo* a_pInfo ) override; // +0x08

		public:
			Trigger* m_pTrigger; // 0x04
		};

		Trigger();
		~Trigger();

		// Blob bounced on the trigger
		void OnBlobContact( ABlobEntity* a_pBlob );

		// The trigger got its last paint
		void OnPainted( ABlobEntity* a_pBlob );

		// $deBlob: FUNCTION 0057bd40
		void SetPaintLeft( TINT a_iPaintLeft )
		{
			m_iPaintLeft = a_iPaintLeft;
			Toshi::TStringManager::String16Format( m_wszPaintLeft, 8, L"%d", a_iPaintLeft );
		}

		void Hide();
		void Show();
		void Disable();

		Toshi::TMatrix44    m_oMatrix;           // 0x00
		ModelRef            m_oModel;            // 0x40 "Landmark_Target_New"
		CollisionHandler    m_oCollisionHandler; // 0x48
		ALandmarkInstance*  m_pLandmark;         // 0x50
		Toshi::TXUIScene*   m_pScene;            // 0x54 "LandmarkTarget" of hud.xur
		TINT                m_iColour;           // 0x58 LandmarkData::TriggerData::m_iColour
		TINT                m_iPaintLeft;        // 0x5C
		AEnvEmitter*        m_pEmitter;          // 0x60 AEnvParticles' "Blob_LandmarkTarget"
		TWCHAR              m_wszPaintLeft[ 8 ]; // 0x64
		TINT16              m_iBody;             // 0x74 the trigger body of m_oModel, -1: none
		TUINT16             m_uiBodyLayer;       // 0x76 its collision layer
		TBOOL               m_bPainted;          // 0x78
		Toshi::TXUIFigure*  m_pDropletLow;       // 0x7C
		Toshi::TXUIFigure*  m_pDropletHigh;      // 0x80
		Toshi::TXUIElement* m_pTextPresenter1;   // 0x84
		Toshi::TXUIElement* m_pTextPresenter2;   // 0x88
	};

	// The links of a landmark's cell instance (CellInstance::m_pLinks)
	struct Links
	{
		TerrainTypes::CellInstance** m_ppInstances;   // 0x00 the instances that become this one's children
		TINT16*                      m_piEntityIDs;   // 0x04
		TINT16                       m_iNumInstances; // 0x08
		TINT16                       m_iNumEntities;  // 0x0A
	};

public:
	ALandmarkInstance( AModel2* a_pModel, TBOOL a_bCreateSoundBreakpoints );
	~ALandmarkInstance();

	virtual void UpdateImp( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x00
	virtual TINT Reset() OVERRIDE;                          // +0x14

	// The callbacks of TerrainTypes::InstanceType_Landmark in ATerrain2Interface's per type tables
	static void OnCreated( TerrainTypes::CellInstance* a_pInstance );
	static void OnBlobTouched( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob, const Toshi::TVector4* a_pPosition );
	static void OnAddedToWorld( TerrainTypes::CellInstance* a_pInstance );
	static void OnRemovedFromWorld( TerrainTypes::CellInstance* a_pInstance );
	static void OnTable97b978( TerrainTypes::CellInstance* a_pInstance, ABlobEntity* a_pBlob );

	// The particle effects of the landmarks (ABuildingManager::Init)
	static void FindEffects();

	// A billboard: no triggers, a touch transforms it
	TBOOL IsABillBoard() const;

	// The triggers that aren't painted yet
	TINT GetNumTriggersLeft() const;

	TINT GetNumTriggers() const { return m_iNumTriggers; }

	// $deBlob: FUNCTION 006505a0
	TINT GetTriggerColour( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex < m_iNumTriggers );
		return m_aTriggers[ a_iIndex ].m_iColour;
	}

	// $deBlob: FUNCTION 00650600
	TINT GetTriggerPaintLeft( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex < m_iNumTriggers );
		return m_aTriggers[ a_iIndex ].m_iPaintLeft;
	}
	// The trigger a_iIndex still has to be painted
	// $deBlob: FUNCTION 0076ac10
	TBOOL IsTriggerUnpainted( TINT a_iIndex ) const
	{
		TASSERT( a_iIndex < m_iNumTriggers );
		return !m_aTriggers[ a_iIndex ].m_bPainted;
	}

	TINT  GetTriggerIndex( const Trigger* a_pTrigger ) const;
	void  GetTriggerPosition( TINT a_iIndex, Toshi::TVector4& a_rOutPosition ) const;
	void  GetTriggerMatrix( TINT a_iIndex, Toshi::TMatrix44& a_rOutMatrix ) const;
	TBOOL GetTriggerExtents( TINT a_iIndex, Toshi::TVector4& a_rOutMin, Toshi::TVector4& a_rOutMax ) const;

	// Blob bounced on the trigger a_iIndex: one paint less
	void PaintTrigger( TINT a_iIndex, ABlobEntity* a_pBlob );

	// The landmark challenge hides the triggers (a_bHide) or shows the ones left
	void SetTriggersHidden( TBOOL a_bHide );
	// Hides or shows every trigger, painted or not
	void SetAllTriggersHidden( TBOOL a_bHide );

	void SetTargetScenesVisible( TBOOL a_bVisible );
	void DisableTriggerBodies();

	// The landmark is completed by a_pBlob (a billboard transforms)
	void Complete( ABlobEntity* a_pBlob );

	// Starts the transform: score, models, FX
	void StartTransform();

	// Ends Blob's landmark state with the letterbox
	void ShowLetterbox();

	void UncountCivs();

	// The paint materials of the low LOD meshes, swapped in around their render (RenderLightmappedInstance)
	void SetLODMeshMaterials();
	void RestoreLODMeshMaterials();

private:
	AModelInstance2* CreateModel( ModelRef* a_pRef, const TCHAR* a_szName, const Toshi::TMatrix44& a_rMatrix, TBOOL a_bCollision, Toshi::TPhysicsResponder* a_pListener );
	void             Init();
	void             MatchMeshes();
	void             RestoreMaterials();
	void             AddPaintPass();
	void             RemovePaintPass();
	void             SetPaintTextures();
	void             UpdateTriggers( TFLOAT a_fDeltaTime );

	static TFLOAT __stdcall GetDistanceSqToViewports( const Toshi::TVector4& a_rPosition );

private:
	static Toshi::TParticleEffect* ms_pRingsEffect;   // "Blob_LandmarkRings"
	static Toshi::TParticleEffect* ms_pGraphicEffect; // "Blob_LandmarkGraphic"
	static Toshi::TParticleEffect* ms_pTargetEffect;  // "Blob_LandmarkTarget"

public:
	Toshi::TMaterial*       m_apOriginalMaterials[ MAX_LANDMARK_MESHES ];               // 0x0CC of the instance meshes
	Toshi::TMaterial*       m_apMaterials[ MAX_LANDMARK_MESHES ];                       // 0x11C "landmark" world materials
	Toshi::TMaterial*       m_apLODOriginalMaterials[ MAX_LANDMARK_MESHES ];            // 0x16C of the low LOD model's meshes
	Toshi::TMaterial*       m_apLODMaterials[ MAX_LANDMARK_MESHES ];                    // 0x1BC
	Toshi::TTexture*        m_apTextures[ MAX_LANDMARK_MESHES ];                        // 0x20C LandmarkData::m_ppTextureNames
	Toshi::TMaterial*       m_aTransformAccessoryMats[ MAX_LANDMARK_ACCESSORY_MESHES ]; // 0x25C
	ModelRef                m_oIdleModel;                                               // 0x2D4
	ModelRef                m_oTransformModel;                                          // 0x2DC
	AFXBreakpointComponent* m_pIdleFX;                                                  // 0x2E4
	AFXBreakpointComponent* m_pTransformFX;                                             // 0x2E8
	Trigger*                m_aTriggers;                                                // 0x2EC [ m_iNumTriggers ]
	ALandmarkInstance*      m_pParentLandmark;                                          // 0x2F0 the landmark that transformed it (StartTransform)
	TFLOAT                  m_fBounceTimer;                                             // 0x2F4 the scale bounce of the completion
	TFLOAT                  m_fFadeTimer;                                               // 0x2F8 -1: none
	TFLOAT                  m_fTransformTimer;                                          // 0x2FC -1: none
	TFLOAT                  m_fBodyTimer;                                               // 0x300 the trigger bodies come back when it runs out
	TINT                    m_iPaintPerTrigger;                                         // 0x304
	TINT                    m_iIdleId;                                                  // 0x308 "idle_loop" of the idle model
	TINT                    m_iTransformId;                                             // 0x30C "transform"
	TINT                    m_iTransformIdleId;                                         // 0x310 "idle_loop" of the transform model
	TINT                    m_iNumTriggers;                                             // 0x314
	TINT8                   m_aiMeshMaterial[ MAX_LANDMARK_MESHES ];                    // 0x318 per instance mesh: index into m_ppMeshNames, -1
};

TSTATICASSERT( sizeof( ALandmarkInstance::Trigger ) == 0x8C );
TSTATICASSERT( sizeof( ALandmarkInstance ) == 0x32C );
