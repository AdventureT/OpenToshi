#pragma once
#include <File/TTRB.h>
#include <Toshi/T2DList.h>
#include <Physics/Havok/TPhysicsManager.h>

#include <Sound/TSound.h>

//-----------------------------------------------------------------------------
// Purpose: the impact and slide sounds of physics bodies, played from
// ACollisionInstanceResponder::OnCollisionConfirmed. Data/Sound/PhysicsSoundsData/PhysicsSoundsData.trb has one sound
// surface per collision surface name (ACollisionCommon::GetSurfaceType) with the FMOD group/event names of its impact
// and slide sounds. Every owner (OwnerInfo::m_pOwner of a body of type 3) gets an emitter that keeps its playing
// sounds; emitters, impacts and slides come from fixed pools (20, 40, 10).
// NOTE: class name and the names in the asserts (pEventSystem, a_pPhysicsSoundEmitterToFree, GetNumActiveSounds,
// a_pPrimarySoundSurface
//-----------------------------------------------------------------------------
class APhysicsSoundManager : public Toshi::TSingleton<APhysicsSoundManager>
{
public:
	static TCONSTEXPR_VAR TINT MAX_EMITTERS = 20;
	static TCONSTEXPR_VAR TINT MAX_IMPACTS  = 40;
	static TCONSTEXPR_VAR TINT MAX_SLIDES   = 10;

	// The sounds of a collision surface (0x44 bytes, data of PhysicsSoundsData.trb)
	struct PhysicsSoundSurface
	{
		enum FLAGS : TUINT32
		{
			FLAGS_OWNIMPACT = 0x10, // hit by a body, the impact sound is this surface's (not the moving body's)
		};

		void SetDefaults();

		const TCHAR* GetSlideEventName() const;
		const TCHAR* GetImpactEventName() const;
		const TCHAR* GetSlideGroupName() const;
		const TCHAR* GetImpactGroupName() const;

		const TCHAR* m_szName;            // 0x00 the collision surface name
		TUINT32      m_uiSurfaceMask;     // 0x04 ACollisionCommon::GetSurfaceType( m_szName ), set by Load
		TFLOAT       m_fSlideFadeInTime;  // 0x08 also the time a slide plays on after its last contact
		TFLOAT       m_fSlideFadeOutTime; // 0x0C
		TFLOAT       m_fImpactThreshold;  // 0x10 an impact if |normal . direction| is above
		TFLOAT       m_fSlideThreshold;   // 0x14 a slide if |normal . direction| is below
		TFLOAT       m_fImpactDelay;      // 0x18 no other impact of the emitter during this time
		TFLOAT       m_fUnk1C;            // 0x1C
		TFLOAT       m_fUnk20;            // 0x20
		TFLOAT       m_fUnk24;            // 0x24
		TFLOAT       m_fMinImpactSpeed;   // 0x28 the relative speed of a silent impact (an impact needs more)
		TFLOAT       m_fMaxImpactSpeed;   // 0x2C the relative speed of a volume 1 impact
		TFLOAT       m_fUnk30;            // 0x30
		TFLOAT       m_fUnk34;            // 0x34
		TUINT32      m_uiFlags;           // 0x38 FLAGS
		TINT16       m_iImpactEvent;      // 0x3C indices into PhysicsSoundsList::m_ppNames
		TINT16       m_iSlideEvent;       // 0x3E
		TINT16       m_iSlideGroup;       // 0x40
		TINT16       m_iImpactGroup;      // 0x42
	};

	TSTATICASSERT( sizeof( PhysicsSoundSurface ) == 0x44 );

	// The "PhysicsSoundsList" symbol of PhysicsSoundsData.trb
	struct PhysicsSoundsList
	{
		TINT                 m_iNumSurfaces; // 0x00
		PhysicsSoundSurface* m_pSurfaces;    // 0x04 the first one is the default
		TINT                 m_iNumNames;    // 0x08
		const TCHAR**        m_ppNames;      // 0x0C FMOD group and event names
	};

	// A playing impact (0xc bytes)
	struct ImpactSound : public Toshi::T2DList<ImpactSound>::Node
	{
		ImpactSound();
		// NOTE: the destructor is the node's (T2DList.h)

		FMOD::Event* m_pEvent; // 0x08
	};

	// A playing slide (0x1c bytes)
	struct SlideSound : public Toshi::T2DList<SlideSound>::Node
	{
		enum STATE : TUINT32
		{
			STATE_FADEIN  = 0x2,
			STATE_PLAYING = 0x4,
			STATE_FADEOUT = 0x8,
		};

		SlideSound();
		// NOTE: the destructor is the node's (T2DList.h)

		void Reset();

		PhysicsSoundSurface* m_pSurface;  // 0x08
		FMOD::Event*         m_pEvent;    // 0x0C
		TFLOAT               m_fFadeTime; // 0x10
		TFLOAT               m_fTimeLeft; // 0x14 the slide fades out once it's below 0
		TUINT32              m_uiState;   // 0x18 STATE
	};

	// The sounds of one owner (0x24 bytes)
	struct PhysicsSoundEmitter : public Toshi::T2DList<PhysicsSoundEmitter>::Node
	{
		PhysicsSoundEmitter();
		~PhysicsSoundEmitter();

		TBOOL PlayImpact( PhysicsSoundSurface* a_pSurface, Toshi::TPhysicsCollisionBody* a_pBody, Toshi::TPhysicsCollisionInfo* a_pInfo, Toshi::T2DList<ImpactSound>* a_pFreeImpacts );
		TBOOL PlaySlide( PhysicsSoundSurface* a_pSurface, Toshi::TPhysicsCollisionBody* a_pBody, TFLOAT a_fRelativeSpeed, const Toshi::TVector4* a_pPosition, Toshi::T2DList<SlideSound>* a_pFreeSlides );
		TBOOL StopImpact( ImpactSound* a_pImpact, Toshi::T2DList<ImpactSound>* a_pFreeImpacts );
		TBOOL StopSlide( SlideSound* a_pSlide, Toshi::T2DList<SlideSound>* a_pFreeSlides );
		void  Update( TFLOAT a_fDeltaTime, Toshi::T2DList<ImpactSound>* a_pFreeImpacts, Toshi::T2DList<SlideSound>* a_pFreeSlides );
		TINT  GetNumActiveSounds();

		Toshi::T2DList<ImpactSound>     m_oImpacts;      // 0x08
		Toshi::T2DList<SlideSound>      m_oSlides;       // 0x10
		void*                           m_pOwner;        // 0x18 OwnerInfo::m_pOwner of the body
		Toshi::TPhysicsObjectContainer* m_PhysicsHandle; // 0x1C
		TFLOAT                          m_fImpactDelay;  // 0x20
	};

	// What OnCollision does (GetCollisionType)
	enum COLLISION : TINT
	{
		COLLISION_NewSlide    = 0, // a slide of an owner without emitter
		COLLISION_Impact      = 1, // an impact of an owner with an emitter
		COLLISION_Slide       = 2, // a slide of an owner with an emitter, not sliding yet
		COLLISION_NewImpact   = 3, // an impact of an owner without emitter
		COLLISION_KeepSliding = 5, // a slide of an owner that already slides
		COLLISION_None        = 6,
	};

public:
	APhysicsSoundManager();
	~APhysicsSoundManager();

	void Load( const TCHAR* a_szFileName );
	void Unload();
	void Destroy();
	void Update( TFLOAT a_fDeltaTime );
	TINT GetNumFreeSounds();
	void OnCollision( Toshi::TPhysicsCollisionInfo* a_pInfo, TINT a_iUnused );
	void StopAll();

	const TCHAR*         GetName( TINT16 a_iIndex );
	PhysicsSoundSurface* FindSurface( TUINT32 a_uiSurfaceMask );

private:
	COLLISION GetCollisionType( Toshi::TPhysicsCollisionBody* a_pPrimary, Toshi::TPhysicsCollisionBody* a_pSecondary, Toshi::TPhysicsCollisionInfo* a_pInfo, PhysicsSoundEmitter** a_ppEmitter, PhysicsSoundSurface** a_ppPrimarySoundSurface, PhysicsSoundSurface** a_ppSecondarySoundSurface );

	static void GetOwner( Toshi::TPhysicsCollisionBody* a_pBody, TINT* a_pType, void** a_ppOwner );

	PhysicsSoundEmitter* AllocEmitter();
	void                 FreeEmitter( PhysicsSoundEmitter* a_pPhysicsSoundEmitterToFree );
	static TBOOL         RefreshSlides( PhysicsSoundEmitter* a_pEmitter );

private:
	Toshi::TTRB                         m_oTRB;          // 0x00
	PhysicsSoundsList*                  m_pList;         // 0x14
	Toshi::T2DList<ImpactSound>         m_oFreeImpacts;  // 0x18
	Toshi::T2DList<SlideSound>          m_oFreeSlides;   // 0x20
	Toshi::T2DList<PhysicsSoundEmitter> m_oFreeEmitters; // 0x28
	Toshi::T2DList<PhysicsSoundEmitter> m_oEmitters;     // 0x30
	PhysicsSoundEmitter*                m_pEmitters;     // 0x38
	ImpactSound*                        m_pImpacts;      // 0x3C
	SlideSound*                         m_pSlides;       // 0x40
};

TSTATICASSERT( sizeof( APhysicsSoundManager ) == 0x44 );
