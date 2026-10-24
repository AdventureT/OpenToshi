#pragma once
#include "AEnemyComponentManager.h"
#include "AEnemy.h"

#include <Toshi/TSingleton.h>
#include <Toshi/T2DList.h>
#include <Toshi/T2SortedList.h>
#include "AEnemyTarget.h"

class AEnemyWrapper;
class AEnemyTarget;

//-----------------------------------------------------------------------------
// Purpose: an enemy of AEnemyManager's free or active lists (a virtual destructor).
//-----------------------------------------------------------------------------
class AEnemyListNode
    : public Toshi::T2DList<AEnemyListNode>::Node
{
public:
	AEnemyListNode() = default;
	// $deBlob: FUNCTION 0078bfb0
	// $deBlob: FUNCTION 0078baa0
	virtual ~AEnemyListNode() = default; // +0x00

	AEnemy* m_pEnemy = TNULL; // 0x0C
};

//-----------------------------------------------------------------------------
// Purpose: an enemy of one of AEnemyManager's update lists (a virtual destructor), sorted by the time since its last
// update (the longest first).
//-----------------------------------------------------------------------------
class AEnemyUpdateNode
    : public Toshi::T2DList<AEnemyUpdateNode>::Node
{
public:
	struct EnemySortResults
	{
		// $deBlob: FUNCTION 007d0f10
		TINT operator()( const AEnemyUpdateNode& a_rcVal1, const AEnemyUpdateNode& a_rcVal2 ) const
		{
			return ( a_rcVal1.m_fTime > a_rcVal2.m_fTime ) ? -1 : 1;
		}
	};

public:
	// $deBlob: FUNCTION 0078b990
	AEnemyUpdateNode() = default;
	// $deBlob: FUNCTION 0078bf20
	// $deBlob: FUNCTION 0078bae0
	virtual ~AEnemyUpdateNode() = default; // +0x00

	AEnemy* m_pEnemy = TNULL; // 0x0C
	TFLOAT  m_fTime  = 0.0f;  // 0x10
};

// The settings of "Enemies_data_EntityData" (AEnemyManager::GetEnemiesData)
struct EnemiesData
{
	TUINT8 m_aUnk00[ 0x08 ];       // 0x00
	TINT   m_iEnvironmentFrames;   // 0x08 frames between two AEnemyBehaviour::UpdateEnvironment
	TINT   m_iEnemyListFrames;     // 0x0C frames between two AEnemyBehaviour::UpdateEnemyList
	TUINT8 m_aUnk10[ 0x08 ];       // 0x10
	TFLOAT m_fFleeRange;           // 0x18 AEnemy::GetFleeRange
	TUINT8 m_aUnk1C[ 0x04 ];       // 0x1C
	TFLOAT m_afThreatFactors[ 6 ]; // 0x20 AEnemyBehaviour::GetThreatFactors
	TFLOAT m_fFrustrationStep;     // 0x38 AEnemyBehaviour::UpdateFrustration
	TFLOAT m_fFrustrationMax;      // 0x3C
	TUINT8 m_aUnk40[ 0x14 ];       // 0x40
	TFLOAT m_fMaxInactiveTime;     // 0x54 AEnemy::UpdateActiveTime: out of the high LOD for that long
};

//-----------------------------------------------------------------------------
// Purpose: the pools of the INKT enemies (Enemies\AEnemyManager.cpp): the level's enemy wrappers get a free enemy of
// their type (CreateEnemy) and give it back (ReleaseEnemy); ticks the enemy components by type (AEnemyComponentManager)
// and spreads the costly behaviour updates (enemy list, environment) over the frames through two update lists.
//-----------------------------------------------------------------------------
class AEnemyManager
    : public Toshi::TSingleton<AEnemyManager>
{
public:
	// AEnemyTypes::EnemyType
	enum EnemyType
	{
		ENEMYTYPE_INVALID        = -1,
		ENEMYTYPE_InkyBaton      = 0,
		ENEMYTYPE_InkyPen        = 1,
		ENEMYTYPE_LeechBot2      = 2,
		ENEMYTYPE_InkyBatonElite = 3,
		ENEMYTYPE_Count          = 4,
	};

	// The update lists
	enum UpdateTypeEnum
	{
		UpdateType_EnemyList   = 0, // AEnemyBehaviour::UpdateEnemyList
		UpdateType_Environment = 1, // AEnemyBehaviour::UpdateEnvironment
		UpdateType_Count       = 2,
	};

	using EnemyList  = Toshi::T2DList<AEnemyListNode>;
	using UpdateList = Toshi::T2SortedList<AEnemyUpdateNode, Toshi::T2DList<AEnemyUpdateNode>, AEnemyUpdateNode::EnemySortResults>;

public:
	AEnemyManager();
	~AEnemyManager();

	// Creates a_iCount enemies of a_eType (their class is GetEnemyClassName) into the free list
	void InitEnemyPool( EnemyType a_eType, TINT a_iCount );
	// Allocates the update nodes of a_iNumEnemies enemies (one per update list)
	void InitEnemyUpdatePool( TINT a_iNumEnemies );
	// Creates the components of every pooled enemy (not updated)
	void InitEnemyComponents();
	void DestroyPools();
	void InitEnemies();

	// Gives a free enemy of a_eType (or the one of a suicider) to a_pWrapper and places it
	AEnemy* CreateEnemy( EnemyType a_eType, const Toshi::TVector4& a_rvPosition, const Toshi::TVector4& a_rvFacing, AEnemyWrapper* a_pWrapper, TINT8 a_iPickupType, TINT16 a_iNumPickups, TBOOL a_bUnk7, ABaseEntity* a_pSpawner, TINT a_iCustomData );
	// Gives a_pEnemy back to the free list of its type
	void ReleaseEnemy( AEnemy* a_pEnemy );
	// Gives every active enemy of a_eType back (the level restart, AInGameState)
	void ReleaseActiveEnemies( EnemyType a_eType );

	void Tick( TFLOAT a_fDeltaTime );

	// Only the components (while the game is paused?)
	void TickComponents( TFLOAT a_fDeltaTime );

	// A player blob becomes a target of the enemies (and of the grapplers)
	void          AddTarget( ABaseEntity* a_pTarget );
	AEnemyTarget* FindTarget( ABaseEntity* a_pTarget );

	// Re-sorts a_pEnemy in the update lists
	void OnLODChanging( AEnemy* a_pEnemy );
	// Moves the components of a_pEnemy to the update lists of its music mode (0: all, 1: AI, 2: none)
	void UpdateEnemyMusicMode( AEnemy* a_pEnemy, TINT a_iMusicMode );
	void PauseForCutscene( TBOOL a_bPause );
	void LoadTRB();

	AEnemyComponentManager& GetEnemyComponentMgr( AEnemy::ComponentType a_eType ) { return m_aComponentManagers[ a_eType ]; }
	EnemyList&              GetEnemyList( EnemyType a_eType ) { return m_ActiveList[ a_eType ]; }
	// One costly update per frame (UpdateEnvironments, UpdateEnemyList, AEnemyBehaviour::GetSurroundPoint)
	// $deBlob: FUNCTION 007d0c00
	TBOOL        CanUpdate() const { return m_bCanUpdate; }
	EnemiesData* GetEnemiesData() const { return TSTATICCAST( EnemiesData, m_pEnemiesData ); }

	// The entity type (data\entities) of a_eType
	static const TCHAR* GetEnemyClassName( EnemyType a_eType );

private:
	void UpdateTargets( TFLOAT a_fDeltaTime );
	void UpdateEnvironments( TFLOAT a_fDeltaTime );
	void UpdateEnemyList( TFLOAT a_fDeltaTime );
	void UpdateMovement( TFLOAT a_fDeltaTime );

	AEnemyListNode* FindEnemyNode( AEnemy* a_pEnemy );
	AEnemy*         FindSuicider( EnemyType a_eType );

	void RemoveFromUpdateList( UpdateList& a_rUpdateList, AEnemy* a_pEnemy );
	void ResortUpdateList( UpdateList& a_rUpdateList );
	void InitProjectilePool( AEnemy* a_pEnemy, TINT a_iCount );

	static void InitialiseSchedules();
	static void InitialiseBehaviours();
	static void DeinitialiseBehaviours();
	void        ActivateComponents( AEnemy* a_pEnemy );

	// The "enemy_create" console variable: an enemy of that class near the player
	void CreateDebugEnemy();

private:
	EnemyList                        m_FreeList[ ENEMYTYPE_Count ];                       // 0x00
	EnemyList                        m_ActiveList[ ENEMYTYPE_Count ];                     // 0x20
	TINT                             m_iResortTimer;                                      // 0x40 frames until the update lists are re-sorted
	TBOOL                            m_bCanUpdate;                                        // 0x44 one costly update per frame
	AEnemy*                          m_pEnvironmentEnemy;                                 // 0x48 the enemy whose environment was updated last
	TINT                             m_iEnvironmentTimer;                                 // 0x4C frames
	TINT                             m_iEnemyListTimer;                                   // 0x50 frames
	TBOOL                            m_bPaused;                                           // 0x54
	UpdateList                       m_aUpdateLists[ UpdateType_Count ];                  // 0x58
	Toshi::T2DList<AEnemyUpdateNode> m_FreeUpdateNodes;                                   // 0x68
	AEnemyUpdateNode*                m_pUpdateNodes;                                      // 0x70
	void*                            m_pEnemiesData;                                      // 0x74 "Enemies_data_EntityData"
	Toshi::T2DList<AEnemyTarget>     m_Targets;                                           // 0x78 one per player blob (AddTarget)
	AEnemyComponentManager           m_aComponentManagers[ AEnemy::ComponentType_Count ]; // 0x80
};
