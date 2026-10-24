#include "ToshiPCH.h"
#include "TResource.h"
#include "TRender.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 006b5090
// $deBlob: FUNCTION 006b50a0
// $deBlob: FUNCTION 006b5100
// $deBlob: FUNCTION 006b5160
// $deBlob: FUNCTION 006b5170
// $deBlob: FUNCTION 0053ed70
// $deBlob: FUNCTION 00859000
TDEFINE_CLASS( TResource );

// $deBlob: FUNCTION 006b5180
// $Barnyard: FUNCTION 006d8240
TResource::TResource()
{
	m_State     = 0;
	m_UId       = 0;
	m_pRenderer = TNULL;
	m_Name[ 0 ] = 0;
}

// $deBlob: FUNCTION 006b55a0
// $deBlob: FUNCTION 006b57f0
// $Barnyard: FUNCTION 006d8220
// $Barnyard: FUNCTION 006d8280
TResource::~TResource()
{
	TASSERT( 0 == ( m_State & TResourceState_Dead ) );
	m_State |= TResourceState_Dead;
}

// $deBlob: FUNCTION 006b51d0
// $Barnyard: FUNCTION 006d8160
TBOOL TResource::Create()
{
	TASSERT( TFALSE == IsCreated() );

	m_State |= TResourceState_Created;
	return TTRUE;
}

// $deBlob: FUNCTION 006b5210
// $Barnyard: FUNCTION 006d8180
TBOOL TResource::Validate()
{
	TASSERT( ( TNULL == Parent() ) || ( TFALSE == Parent()->IsDying() ) );
	TASSERT( !( m_State & TResourceState_Dying ) );

	if ( IsDying() ) return TFALSE;

	m_State |= TResourceState_Valid;
	return TTRUE;
}

// $deBlob: FUNCTION 006b5820
// $Barnyard: FUNCTION 006bffd0
void TResource::Invalidate()
{
	m_State &= ~TResourceState_Valid;
}

// $deBlob: FUNCTION 006b5320
// $Barnyard: FUNCTION 006d8210
void TResource::DestroyResource()
{
	GetRenderer()->DestroyResource( this );
}

// $deBlob: FUNCTION 006b5580
TBOOL TResource::TryInvalidate()
{
	return TTRUE;
}

// $deBlob: FUNCTION 006b5590
TBOOL TResource::TryValidate()
{
	return TTRUE;
}

// $deBlob: FUNCTION 006b5620
// $Barnyard: FUNCTION 006d8170
void TResource::OnDestroy()
{
	TASSERT( TTRUE == IsCreated(), "Tried to destroy resource that wasn't created" );

	m_State &= ~TResourceState_Created;
	m_State |= TResourceState_Dying;
}

// $deBlob: FUNCTION 006b5400
// $Barnyard: FUNCTION 006d82b0
void TResource::SetParent( TResource* a_pParent )
{
	TASSERT( ( TNULL == a_pParent ) || ( TTRUE == IsDying() ) || ( TFALSE == a_pParent->IsDying() ) );
	TASSERT( TNULL != Tree() );

	TNodeTree<TResource>* pTree = TNode::Tree();
	pTree->Remove( this );

	if ( !a_pParent && pTree )
		pTree->InsertAtRoot( this );
	else
		pTree->Insert( a_pParent, this );
}

// $deBlob: FUNCTION 006b5350
// $Barnyard: FUNCTION 006d81a0
void TResource::SetName( const TCHAR* name )
{
	TCHAR szName[ 16 ];

	// No name: "res:<uid>"
	if ( name == TNULL )
	{
		szName[ 0 ] = 'r';
		szName[ 1 ] = 'e';
		szName[ 2 ] = 's';
		szName[ 3 ] = ':';
		name        = szName;
		T2String8::IntToString( m_UId, szName + 4, 0xE, 10 );
	}

	if ( TINT( T2String8::Length( name ) ) > TINT( MAXNAMELEN ) )
		TUtil::Log( "[ERROR] %s is to long, needs to be less then %d characters\n", name, MAXNAMELEN );

	TASSERT( T2String8::Length( name ) <= TINT( MAXNAMELEN ) );
	T2String8::Copy( m_Name, name, -1 );
}

// NOTE: retail's callback is __stdcall and the recursion is TNodeTree's;
// without a resource it starts at the tree's first root child (Barnyard: this resource's child)
// $deBlob: FUNCTION 006b54d0
// $Barnyard: FUNCTION 006d8400
TBOOL TResource::RecurseSimple( t_RecurseCb a_pCallback, TResource* a_pResource, void* a_pUserData )
{
	TASSERT( TNULL != Tree() );
	TNodeTree<TResource>* pTree = Tree();
	TASSERT( TNULL != a_pCallback );

	if ( a_pResource )
		return Recurse( a_pCallback, a_pResource, TFALSE, a_pUserData );
	else
		return Recurse( a_pCallback, pTree->ChildOfRoot(), TTRUE, a_pUserData );
}

// $Barnyard: FUNCTION 006beb70
TBOOL TResource::Recurse( t_RecurseCb a_pCallback, TResource* a_pResource, TBOOL a_bFlag, void* a_pUserData )
{
	TResource* pRes = a_pResource;

	if ( TNULL != a_pResource )
	{
		TResource* pNext;

		do
		{
			pNext = pRes->Next();

			if ( pNext == a_pResource || pNext == pRes || !a_bFlag )
				pNext = TNULL;

			if ( !a_pCallback( pRes, a_pUserData ) )
				return TFALSE;

			if ( pRes->Child() && !Recurse( a_pCallback, pRes->Child(), TTRUE, a_pUserData ) )
				return TFALSE;

			pRes = pNext;

		} while ( pNext != TNULL );
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006b5330
Toshi::TRender* TResource::GetRenderer() const
{
	return m_pRenderer;
}

void TResource::SetRenderer( TRender* pRenderer )
{
	m_pRenderer = pRenderer;
}

TUINT32 TResource::GetUId() const
{
	return m_UId;
}

void TResource::SetUId( TUINT32 uid )
{
	m_UId = uid;
}

TOSHI_NAMESPACE_END
