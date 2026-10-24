#pragma once
#include "ABaseEntity.h"
#include "AStack.h"

#include <Toshi/TSingleton.h>
#include <Toshi/TString8.h>

namespace TerrainTypes
{
struct Cell;
}

//-----------------------------------------------------------------------------
// Purpose: what the factory knows about an entity type (AEntityFactory.cpp): the TClass named by the type's
// "<type>_data_EntityData" symbol in data\entities\EntityData.trb (AEntityData).
//-----------------------------------------------------------------------------
struct AEntityTypeData
{
	// The start of "<type>_data_EntityData"
	struct TRBData
	{
		const TCHAR* m_szClassName; // 0x00
		const TCHAR* m_szModelName; // 0x04 the model name (GetModelName), also the name of the FX breakpoint component
		// The physics body (AVisibleEntity::CreatePhysics)
		TFLOAT       m_fMass;                  // 0x08
		TUINT32      m_uiPhysicsFlags;         // 0x0C ACollisionCommon::SURFACE in the low bits, layer flags 0x2010/0x4400000
		TUINT32      m_aUnk10[ 2 ];            // 0x10
		TFLOAT       m_fLinearDamping;         // 0x18
		TFLOAT       m_fAngularDamping;        // 0x1C
		TFLOAT       m_fMaxLinearVelocity;     // 0x20
		TFLOAT       m_fMaxAngularVelocity;    // 0x24
		TINT         m_abCollisionGroups[ 8 ]; // 0x28 groups 0x81, 2, 4, 8, 0x10, 0x20, 0x40, 0x100
		TINT         m_eMotionType;            // 0x48 1: (0x4C ? 2 : 1), 2: 3, else 0 (TPhysicsManager::CreateBody)
		TINT         m_bMotionSubType;         // 0x4C
		const TCHAR* m_szDespawnEffect;        // 0x50 (APropSwitchPressure's sm_despawnEffect)
	};
	// NOTE: derived type datas (AEnemy::EnemyData) continue at 0x54
	TSTATICASSERT( sizeof( TRBData ) == 0x54 );

	const Toshi::TClass* m_pClass;   // 0x00 TNULL if no class of that name derives from ABaseEntity
	Toshi::TString8      m_strName;  // 0x04 the entity type
	TRBData*             m_pTRBData; // 0x10
};

//-----------------------------------------------------------------------------
// Purpose: the entity manager: creates entities by type name (AEntityTypeData -> TClass), keeps the active and inactive
// entity lists, the always-tick, unlinked and stale lists of their cell pairs, ticks them and calls the registered
// callbacks.
//-----------------------------------------------------------------------------
class AEntityFactory
    : public Toshi::TSingleton<AEntityFactory>
{
public:
	static TCONSTEXPR_VAR TINT MAX_ENTITY_FLAGS = 2048;
	static TCONSTEXPR_VAR TINT MAX_TYPE_DATAS   = 128;

	using Callback_t = void ( * )( void* a_pUserData );

	struct CallbackNode
	    : public Toshi::T2DList<CallbackNode>::Node
	{
		void*      m_pUserData;  // 0x08
		Callback_t m_fnCallback; // 0x0C
	};

public:
	AEntityFactory();
	~AEntityFactory();

	// Creates an entity of the type (a path below data/entities, e.g. "Props/AProp"), sets it up and adds it
	ABaseEntity* CreateEntity( const TCHAR* a_szEntityName );

	// Moves the entity to the active list and adds it (unlinked) to the cells
	void ActivateEntity( ABaseEntity& a_rEntity );
	void ReactivateEntity( ABaseEntity& a_rEntity );
	// Moves the entity to the inactive list
	void DeactivateEntity( ABaseEntity& a_rEntity );
	void ReactivateAllEntities();

	void AddEntity( ABaseEntity& a_rEntity );
	void RemoveEntity( ABaseEntity& a_rEntity );
	void DestroyEntity( ABaseEntity* a_pEntity );
	void DestroyAllEntities();

	void Tick( TFLOAT a_fDeltaTime, TUINT a_uiFlags );
	void PreRenderUpdate();

	void MarkStale( ABaseEntity& a_rEntity );

	void SetAllUnitTimeStamps( TUINT32 a_uiTimeStamp );

	void AddCallback( void* a_pUserData, Callback_t a_fnCallback );
	void RemoveCallback( void* a_pUserData );
	void CallCallbacks();

	void CreatePools();
	void InitPooledEntities();

	// Adds the type data of a_szEntityName with its TRB data
	AEntityTypeData* AddTypeData( const TCHAR* a_szEntityName, AEntityTypeData::TRBData* a_pTRBData );
	AEntityTypeData* AddPrecachedTypeData( const TCHAR* a_szEntityName );

	// $deBlob: FUNCTION 0075f520
	void IncrementEntityFlagsIndex()
	{
		TASSERT( m_EntityFlagsIndex < MAX_ENTITY_FLAGS, "Too many entities in your level" );
		m_EntityFlagsIndex++;
	}

	Toshi::T2DList<ABaseEntity>& GetEntities() { return m_oEntities; }
	Toshi::T2DList<ACellPair>&   GetAlwaysTickEntities() { return m_oAlwaysTickEntities; }

	TINT                   GetNumTypeDatas() const { return m_oTypeDatas.Size(); }
	const AEntityTypeData* GetTypeData( TINT a_iIndex ) const { return m_oTypeDatas[ a_iIndex ]; }

private:
	void ProcessStaleEntity( ABaseEntity& a_rEntity );

	// Picks the entity's LOD from the cameras' distance, its cell and the frustums
	void UpdateEntityLOD( ABaseEntity& a_rEntity );


	// Whether the entity's bounding sphere is in a camera's frustum (never for FLAG_LowLOD unless a_bAnyLOD)
	TBOOL IsInAnyFrustum( ABaseEntity& a_rEntity, TBOOL a_bAnyLOD );

public:
	// The LOD distances of the entities (AEnvironmentFX)
	static void SetLODDistances( TFLOAT a_fMaxDistance, TFLOAT a_fLargeDistance );

	// Updates the LOD of every entity (also called once the level's entities are loaded)
	void UpdateEntitiesLOD();

	// The entities of a cell that is being loaded (updates their LOD, resets their motors) and unloaded (back to FLAG_LOD0)
	void OnCellLoaded( TerrainTypes::Cell* a_pCell );
	void OnCellUnloaded( TerrainTypes::Cell* a_pCell );

private:
	Toshi::T2DList<ABaseEntity>              m_oInactiveEntities;   // 0x000
	Toshi::T2DList<ABaseEntity>              m_oEntities;           // 0x008
	AStack<AEntityTypeData*, MAX_TYPE_DATAS> m_oTypeDatas;          // 0x010
	Toshi::T2DList<ACellPair>                m_oUnlinkedEntities;   // 0x214
	Toshi::T2DList<ACellPair>                m_oAlwaysTickEntities; // 0x21C
	Toshi::T2DList<ACellPair>::Iterator      m_itAlwaysTick;        // 0x224 the next one Tick visits
	Toshi::T2DList<ACellPair::StaleNode>     m_lStaleEntities;      // 0x228
	Toshi::T2DList<CallbackNode>             m_oCallbacks;          // 0x230
	TINT                                     m_EntityFlagsIndex;    // 0x238
};
