#include "pch.h"
#include "APlatformManager.h"
#include "APropPlatform.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

APlatformManager* APlatformManager::ms_Singleton;

TSTATICASSERT( sizeof( APlatformManager ) == 0x4 );
TSTATICASSERT( sizeof( APlatformManager::Group ) == 0xC );
TSTATICASSERT( sizeof( APlatformManager::PlatformNode ) == 0xC );

// $deBlob: FUNCTION 007917c0
APlatformManager::APlatformManager()
{
	TASSERT( ms_Singleton == 0 );
	ms_Singleton = this;
}

// $deBlob: FUNCTION 00791810
// $deBlob: FUNCTION 0078da90
APlatformManager::~APlatformManager()
{
	ClearGroups();

	while ( m_oGroups.Begin() != m_oGroups.End() )
	{
		Group* pGroup = TSTATICCAST( Group, m_oGroups.PopFront() );

		if ( pGroup != TNULL )
		{
			pGroup->m_oPlatforms.Clear();
			delete pGroup;
		}
	}

	TASSERT( ms_Singleton == this );
	ms_Singleton = TNULL;
	m_oGroups.Clear();
}

// Unlinks the platforms of every group
// $deBlob: FUNCTION 00791cf0
void APlatformManager::ClearGroups()
{
	for ( T2GenericSList::Node* pNode = m_oGroups.Begin(); pNode != m_oGroups.End(); pNode = pNode->Next() )
	{
		TASSERT( pNode != TNULL );
		TSTATICCAST( Group, pNode )->m_oPlatforms.Clear();
	}
}

// $deBlob: FUNCTION 007918d0
void APlatformManager::AddPlatform( APropPlatform* a_pPlatform, AWaypointData::Set* a_pSet )
{
	Group*        pGroup = GetGroup( a_pSet );
	PlatformNode& rNode  = a_pPlatform->GetPlatformNode();

	TASSERT( TNULL == rNode.m_Group );
	rNode.m_Group = pGroup;
	rNode.InsertAfter( pGroup->m_oPlatforms.End() );
}

// $deBlob: FUNCTION 00791940
// NOTE: a_pSet is unused
void APlatformManager::RemovePlatform( APropPlatform* a_pPlatform, AWaypointData::Set* a_pSet )
{
	PlatformNode&         rNode      = a_pPlatform->GetPlatformNode();
	T2GenericSList&       rPlatforms = TSTATICCAST( Group, rNode.m_Group )->m_oPlatforms;
	T2GenericSList::Node* pNode      = rPlatforms.Begin();

	if ( pNode == rPlatforms.End() )
		return;

	while ( TTRUE )
	{
		TASSERT( pNode != TNULL );

		if ( TSTATICCAST( PlatformNode, pNode )->m_pPlatform == a_pPlatform )
			break;

		pNode = pNode->Next();

		if ( pNode == rPlatforms.End() )
			return;
	}

	rPlatforms.Erase( pNode, pNode->Next() );
	rNode.m_Group = TNULL;
}

// $deBlob: FUNCTION 007919f0
void APlatformManager::Tick()
{
	for ( T2GenericSList::Node* pGroupNode = m_oGroups.Begin(); pGroupNode != m_oGroups.End(); pGroupNode = pGroupNode->Next() )
	{
		TASSERT( pGroupNode != TNULL );
		T2GenericSList& rPlatforms = TSTATICCAST( Group, pGroupNode )->m_oPlatforms;

		TBOOL bTick = TFALSE;

		for ( T2GenericSList::Node* pNode = rPlatforms.Begin(); pNode != rPlatforms.End(); pNode = pNode->Next() )
		{
			TASSERT( pNode != TNULL );
			APropPlatform* pPlatform = TSTATICCAST( PlatformNode, pNode )->m_pPlatform;
			TUINT32        uiFlags   = pPlatform->GetFlags();

			if ( !( uiFlags & ABaseEntity::FLAG_DontTick ) && ( !( uiFlags & ABaseEntity::FLAG_LowLOD ) || pPlatform->TicksAtLowLOD() ) )
			{
				bTick = TTRUE;
				break;
			}
		}

		for ( T2GenericSList::Node* pNode = rPlatforms.Begin(); pNode != rPlatforms.End(); pNode = pNode->Next() )
		{
			TASSERT( pNode != TNULL );
			APropPlatform* pPlatform = TSTATICCAST( PlatformNode, pNode )->m_pPlatform;

			pPlatform->SetTicking( bTick && !( pPlatform->GetFlags() & ABaseEntity::FLAG_DontTick ) );
		}
	}
}

// $deBlob: FUNCTION 00791c50
TBOOL APlatformManager::IsPathBlocked( Group* a_pGroup, TINT a_iPlatformID, TINT a_iFrom, TINT a_iTo, TFLOAT a_fDistance )
{
	T2GenericSList& rPlatforms = a_pGroup->m_oPlatforms;

	for ( T2GenericSList::Node* pNode = rPlatforms.Begin(); pNode != rPlatforms.End(); pNode = pNode->Next() )
	{
		TASSERT( pNode != TNULL );

		if ( TSTATICCAST( PlatformNode, pNode )->m_pPlatform->IsBlocking( a_iPlatformID, a_iFrom, a_iTo, a_fDistance ) )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 00791da0
APlatformManager::Group* APlatformManager::GetGroup( AWaypointData::Set* a_pSet )
{
	for ( T2GenericSList::Node* pNode = m_oGroups.Begin(); pNode != m_oGroups.End(); pNode = pNode->Next() )
	{
		TASSERT( pNode != TNULL );

		if ( TSTATICCAST( Group, pNode )->m_pSet == a_pSet )
			return TSTATICCAST( Group, pNode );
	}

	Group* pGroup  = new Group();
	pGroup->m_pSet = a_pSet;
	m_oGroups.PushBack( pGroup );

	return pGroup;
}
