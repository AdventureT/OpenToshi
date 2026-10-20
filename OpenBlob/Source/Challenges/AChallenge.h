#pragma once
#include <Toshi/T2DList.h>
#include <Math/TMatrix44.h>
#include <Physics/TPhysicsResponder.h>

TOSHI_NAMESPACE_START
class TPhysicsObjectContainer;
TOSHI_NAMESPACE_END

class ABaseEntity;
class ARevolutionary;

//-----------------------------------------------------------------------------
// Purpose: a challenge of the level (19 slots), owned by AChallengeManager which builds one per entry of the level's
// "Challenges" symbol. Its marker is an ARevolutionary entity (m_pRevolutionary) and its trigger a phantom body with
// m_oCollisionHandler.
//-----------------------------------------------------------------------------
class AChallenge
{
public:
	friend class AChallengeManager;

	enum Type : TINT
	{
		Type_Paint    = 0,
		Type_Landmark = 1,
		Type_Civilian = 2,
		Type_ZJump    = 3,
		Type_Combat   = 4,
		Type_Race     = 5,
		Type_Car      = 6,
		Type_Null     = 7,
		Type_Uber     = 8,
	};

	// m_uiFlags (0x128)
	enum Flags : TUINT32
	{
		Flags_Completed = 0x01,
		Flags_Failed    = 0x02,
		Flags_Available = 0x04,
		Flags_Starting  = 0x08,
		Flags_Restarted = 0x10,
	};

	// The data of a challenge in the level's TRB (0x38 bytes, followed by the type's own fields)
	struct Data
	{
		// Data::m_uiFlags
		enum Flags : TUINT32
		{
			Flags_DifficultyEasy           = 0x002,
			Flags_DifficultyMedium         = 0x004,
			Flags_DifficultyHard           = 0x008,
			Flags_CanQuit                  = 0x010, // may be quit (AChallengeManager::QuitChallenge)
			Flags_HasCamera                = 0x020, // the follow camera looks at the marker while it's near
			Flags_MirrorLayout             = 0x040,
			Flags_SubChallenge             = 0x100, // a sub challenge of an AUberChallenge
			Flags_CompletedCameraOnRequest = 0x200,
			Flags_NoFail                   = 0x400, // a paint challenge isn't failed when Blob lands (APaintChallenge)
			Flags_LockBuildings            = 0x800, // a paint challenge locks its buildings until it starts (APaintChallenge)
		};

		struct LinkedBody
		{
			Toshi::TMatrix44* m_pMatrix; // 0x00
			Toshi::TVector4*  m_pOffset; // 0x04
		};

		Toshi::TMatrix44* m_pMatrix; // 0x00 the marker's transform
		union
		{
			TINT*        m_piLinkedChallengeGUIDs; // 0x04 the GUIDs in the TRB...
			AChallenge** m_ppLinkedChallenges;     // 0x04 ...replaced by the challenges (LinkChallenges)
		};
		TINT16*     m_pCameraRegions; // 0x08 ANedCameraFunction region IDs
		LinkedBody* m_pBodies;        // 0x0C m_uiNumBodies extra trigger bodies
		union
		{
			TINT*         m_piEntityIDs; // 0x10 the IDs in the TRB...
			ABaseEntity** m_ppEntities;  // 0x10 ...replaced by the entities (Create)
		};
		TUINT32 m_uiFlags;               // 0x14 Flags
		TINT    m_iStringID;             // 0x18
		TUINT32 m_auiStringIDs[ 2 ];     // 0x1C
		TINT    m_iGUID;                 // 0x24
		TINT16  m_iTimeLimit;            // 0x28 in seconds, -1: none
		TINT16  m_iUnk2A;                // 0x2A
		TINT16  m_iSSCamera;             // 0x2C
		TINT16  m_iCompletedSSCamera;    // 0x2E -1: none
		TUINT16 m_uiUnk30;               // 0x30
		TINT8   m_iNumCameraRegions;     // 0x32
		TUINT8  m_uiNumEntities;         // 0x33
		TUINT8  m_uiNumLinkedChallenges; // 0x34
		TUINT8  m_uiNumBodies;           // 0x35
		TUINT16 m_uiUnk36;               // 0x36
	};

	// Receives the trigger events of the challenge's bodies
	class CollisionHandler
	    : public Toshi::TPhysicsResponder
	{
	public:
		// $deBlob: FUNCTION 0059c970
		CollisionHandler( AChallenge* a_pChallenge )
		    : m_pChallenge( a_pChallenge ) {}

		virtual void OnTriggerEnter( Toshi::TPhysicsObjectContainer* a_pBody, Toshi::TPhysicsObjectContainer* a_pOther ) override; // +0x20
		virtual void OnTriggerLeave( Toshi::TPhysicsObjectContainer* a_pBody, Toshi::TPhysicsObjectContainer* a_pOther ) override; // +0x24

	public:
		AChallenge* m_pChallenge; // 0x04
	};

	using LinkNode = Toshi::T2DListNodeWrapper<AChallenge>; // 0xC bytes

public:
	AChallenge( Type a_eType );
	virtual ~AChallenge(); // +0x00

	// Flags_Starting (0x128 bit 3), set while the challenge starts
	// $deBlob: FUNCTION 0059d0a0
	TBOOL IsStarting() const { return ( m_uiFlags & Flags_Starting ) != 0; }

	virtual void Start();    // +0x04
	virtual void Complete(); // +0x08
	// $deBlob: FUNCTION 00599780
	virtual void OnEnd() {} // +0x0C
	// NOTE: the events forwarded by AChallengeManager to the current challenge
	// $deBlob: FUNCTION 00599850
	virtual void OnPaintEvent( void* a_pBuilding ) {} // +0x10 APaintChallenge
	// $deBlob: FUNCTION 005998c0
	virtual void OnZJumpEvent( void* a_pSet ) {} // +0x14 AZJumpChallenge
	// $deBlob: FUNCTION 00599880
	virtual void OnCombatEvent( void* a_pEntityID ) {} // +0x18 ACombatChallenge
	// $deBlob: FUNCTION 00599870
	virtual void OnCivilianEvent( void* a_pEntityID ) {} // +0x1C ACivilianChallenge
	// $deBlob: FUNCTION 00599860
	virtual void OnCarEvent( void* a_pEntityID ) {} // +0x20 ACarChallenge
	// $deBlob: FUNCTION 005998a0
	virtual void OnLandmarkEvent( void* a_pLandmark ) {} // +0x24 ALandmarkChallenge
	// $deBlob: FUNCTION 00599890
	virtual void OnLandmarkEvent2( void* a_pLandmarkInstance ) {} // +0x28 ALandmarkChallenge
	// $deBlob: FUNCTION 005998b0
	virtual void  OnPaintEvent2( void* a_pTree ) {} // +0x2C APaintChallenge
	virtual void  Update( TFLOAT a_fDeltaTime );   // +0x30
	virtual TBOOL Create( Data* a_pData );         // +0x34
	virtual void  LinkChallenges();                // +0x38
	virtual void  Reset( TBOOL a_bUnk );           // +0x3C
	// $deBlob: FUNCTION 005997b0
	virtual TINT GetScore() { return 0; } // +0x40
	// $deBlob: FUNCTION 00599760
	virtual TBOOL IsAlreadyCompleted() { return TFALSE; } // +0x44
	// The help shown every 30 seconds during the challenge, -1: none
	// $deBlob: FUNCTION 005997a0
	virtual TINT GetHintStringID() { return -1; } // +0x48

	// Fails the challenge (time out or quit): m_uiFlags |= 0x16
	void Fail();

	// Quits the challenge: m_uiFlags |= 0x14
	void Quit();

	// Shows the marker's start prompt
	void ShowStartPrompt();

	// Hides the marker's start prompt
	void HideStartPrompt();

	// $deBlob: FUNCTION 00598900
	void SetRestartedFlag() { m_uiFlags |= Flags_Restarted; }

	// Sends MESSAGE_SKIP to the linked entities
	void SendSkipToEntities();

	// Makes the challenge available (or not) and updates the challenges linked to it
	void SetAvailable( TBOOL a_bCompleted, TBOOL a_bUnk );

	// Whether a_iGUID is one of the challenges linked to this one
	TBOOL IsLinkedTo( TINT a_iGUID ) const;

	// Adds a_pChallenge to the challenges that link to this one
	void AddLinkedFrom( AChallenge* a_pChallenge );

	// Adds a_fDeltaTime to the time left (and to the level's time in a sprint level)
	void AddTime( TFLOAT a_fDeltaTime );

	// The position of the marker
	// $deBlob: FUNCTION 0059d010
	void GetPosition( Toshi::TVector4& a_rvPos ) const { a_rvPos = m_pData->m_pMatrix->GetTranslation(); }

	// $deBlob: FUNCTION 0059d0b0
	void SetFlags( TUINT32 a_uiFlags ) { m_uiFlags = a_uiFlags; }

	// $deBlob: FUNCTION 0059d120
	void SetStarting( TBOOL a_bStarting )
	{
		if ( a_bStarting )
			m_uiFlags |= Flags_Starting;
		else
			m_uiFlags &= ~Flags_Starting;
	}

	// $deBlob: FUNCTION 00580510
	Type GetType() const { return m_eType; }
	// $deBlob: FUNCTION 0058c970
	Data*   GetData() const { return m_pData; }
	TUINT32 GetFlags() const { return m_uiFlags; }
	TFLOAT  GetTimeLeft() const { return m_fTimeLeft; }
	// $deBlob: FUNCTION 005a0450
	void SetTimeLeft( TFLOAT a_fTimeLeft ) { m_fTimeLeft = a_fTimeLeft; }
	// $deBlob: FUNCTION 0058c990
	ARevolutionary* GetRevolutionary() const { return m_pRevolutionary; }

	TBOOL IsCompleted() const { return m_uiFlags & Flags_Completed; }
	// $deBlob: FUNCTION 005997e0
	TUINT32 IsAvailable() const { return ( m_uiFlags >> 2 ) & 1; }

protected:
	Data*                            m_pData;             // 0x004
	Toshi::T2DList<LinkNode>         m_oLinkedFrom;       // 0x008 the challenges linking to this one
	TFLOAT                           m_fHintTimer;        // 0x010
	TFLOAT                           m_fTimeLeft;         // 0x014 -1: no time limit
	Toshi::TPhysicsObjectContainer*  m_pBody;             // 0x018 the trigger
	Toshi::TPhysicsObjectContainer** m_ppBodies;          // 0x01C the extra trigger bodies (m_pData->m_uiNumBodies)
	CollisionHandler                 m_oCollisionHandler; // 0x020
	TUINT8                           m_aUnk28[ 0x100 ];   // 0x028
	TUINT32                          m_uiFlags;           // 0x128 Flags
	ARevolutionary*                  m_pRevolutionary;    // 0x12C the marker entity
	Toshi::TMatrix44                 m_oMatrix;           // 0x130
	Type                             m_eType;             // 0x170
};

TSTATICASSERT( sizeof( AChallenge ) == 0x174 );

// $deBlob: FUNCTION 0059c5f0
inline AChallenge::AChallenge( Type a_eType )
    : m_pData( TNULL )
    , m_fHintTimer( 0.0f )
    , m_fTimeLeft( -1.0f )
    , m_ppBodies( TNULL )
    , m_oCollisionHandler( this )
    , m_eType( a_eType )
    , m_uiFlags( 0 )
    , m_pRevolutionary( TNULL )
{
#ifdef DEBLOB_COMMUNITY_PATCH
	// Retail doesn't initialise m_pBody
	m_pBody = TNULL;
#endif
}

