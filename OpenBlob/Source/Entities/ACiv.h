#pragma once
#include "Entities/APropPlatform.h"
#include "Entities/AEntityFactory.h"
#include "Entities/AEntityPoseController.h"
#include <Math/TRandom.h>

class ABlobEntity;

TOSHI_NAMESPACE_START
class TParticleEffect;
TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Purpose: a civilian (Graydian) walking a spline, liberated by Blob; pooled by ACivSpawner.
//-----------------------------------------------------------------------------
class ACiv
    : public APropPlatform
{
public:
	TDECLARE_CLASS( ACiv, APropPlatform );

	enum CIVFLAG : TUINT16
	{
		CIVFLAG_ColourMask  = 0xFF,  // the colour of the liberated civilian
		CIVFLAG_TypeMask    = 0x300, // CivType
		CIVFLAG_Transformed = 0x400,
		CIVFLAG_NoGraydian  = 0x800,
		CIVFLAG_Laughing    = 0x1000, // m_fLaughTimer runs until it laughs
		CIVFLAG_NoLiberate  = 0x2000,
	};

	static TCONSTEXPR_VAR TUINT8 CivType_Count = 4;

	// The civilian's "<type>_data_EntityData"
	struct CivData
	    : public AEntityTypeData::TRBData
	{
		Toshi::TVector4* pShellBBox; // 0x54 the bounding box in the shell (InitPooled)
		TFLOAT           fSpeed;     // 0x58
	};

public:
	ACiv();
	virtual ~ACiv() override; // +0x08

	virtual void  Tick( TFLOAT a_fDeltaTime ) override;                                                // +0x0C
	virtual void  PostCreate() override;                                                               // +0x20
	virtual void  ProcessParams( const AEntityParams& a_rParams ) override;                            // +0x2C
	virtual void  InitPooled( TINT a_eType ) override;                                                 // +0x30
	virtual void  HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket ) override; // +0x5C
	virtual TBOOL IsTargetable() override;                                                             // +0xA0
	virtual TINT  GetTargetColour() override;                                                          // +0xB4
	// $deBlob: FUNCTION 0076de50
	virtual void SetPoolOwner( TINT a_iOwner ) override { m_iPoolOwner = a_iOwner; } // +0xC0
	// $deBlob: FUNCTION 0076ddf0
	virtual TINT         GetPoolOwner() override { return m_iPoolOwner; }                                                                                       // +0xC4
	virtual void         Reset( ABaseEntity* a_pEntity ) override;                                                                                              // +0xC8
	virtual const TCHAR* GetModelName() override;                                                                                                               // +0xEC
	virtual void         SetWaypointSet( TINT a_iWaypointSetID, TINT a_iStartPoint, const Toshi::TVector4* a_pPosition, RespawnData* a_pRespawnData ) override; // +0xF8
	virtual void         StopMoving() override;                                                                                                                 // +0x104
	// $deBlob: FUNCTION 0076def0
	virtual TBOOL QueuesBehindOthers() override { return TTRUE; } // +0x110

	TBOOL IsTransformed() const { return ( TUINT8( m_uiCivFlags >> 8 ) & 4 ) != 0; }

	// The civilian walks a waypoint set
	TBOOL IsAttachedToSpline() const { return m_pWaypointSet != TNULL; }

	// The low byte of m_uiCivFlags, the colour the civilian was (ASpawnedCivManager reads it as 0x60 of its wrapper)
	TUINT8 GetColour() const { return TUINT8( m_uiCivFlags ); }

	// ASpawnedCivManager starts and updates the civilians it spawned (its Update)
	friend class ASpawnedCivManager;
	friend class ACivSpawner;

	void  Liberate( TUINT32 a_uiColour, TBOOL a_bQuiet );
	void  SetTypeAndReset( TUINT8 a_uiType );
	void  StartLaughTimer();
	TBOOL CanBeLiberated() const;
	TBOOL IsExploding() const;
	void  SetCivType( TUINT8 a_uiType );

	static void SquashTransformedCivs( ABlobEntity* a_pBlob );

	// Clones "ACiv_PoseMap_Main" for the models Civ1.trb and Civ2.trb (AEntityData::Load), and destroys the clones
	// (AEntityData::Unload)
	static void LoadPoseMaps();
	static void UnloadPoseMaps();

private:
	const CivData* GetCivData() const { return TSTATICCAST( const CivData, m_pTypeData->m_pTRBData ); }

	void ResumeWalking();
	void Explode();
	void SetColour( TUINT8 a_uiColour );
	void CreateCivModel( TBOOL a_bCreateCollision );
	void Transform( TUINT32 a_uiColour, TBOOL a_bQuiet );

	static TFLOAT RandomRange( Toshi::TRandom& a_rRandom, TFLOAT a_fMin, TFLOAT a_fRange );

private:
	static TBOOL                           ms_bSkinMaterialsSet;
	static AEntityPoseController::PoseMap* ms_apPoseMaps[ 2 ]; // of Civ1.trb and Civ2.trb

protected:
	TUINT16                 m_uiCivFlags;           // 0x1F0 CIVFLAG
	void*                   m_pShellModelComponent; // 0x1F4 AEntityModelComponent swapped with m_pModelComponent (the shell)
	TINT                    m_iPoolOwner;           // 0x1F8
	Toshi::TParticleEffect* m_pShellPopFX;          // 0x1FC "Blob_CivShellPop"
	TFLOAT                  m_fLaughTimer;          // 0x200 -1.0f
};
