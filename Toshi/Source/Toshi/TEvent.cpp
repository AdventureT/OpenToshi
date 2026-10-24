#include "ToshiPCH.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 00663240
// $Barnyard: FUNCTION 006c4630
void TGenericListener::Connect( TGenericEmitter& a_rEmitter, void* a_pCaller, t_Callback a_pCallback, TINT a_iPriority )
{
	TASSERT( IsLinked() == TFALSE );
	m_pCaller   = a_pCaller;
	m_pCallback = a_pCallback;
	SetPriority( a_iPriority );
	a_rEmitter.GetListeners().Insert( this );
}

// $deBlob: FUNCTION 006632a0
// $Barnyard: FUNCTION 006c46c0
void TGenericListener::Disconnect()
{
	Remove();
	m_pCaller   = TNULL;
	m_pCallback = TNULL;
}

// $deBlob: FUNCTION 006631e0
TGenericListener::TGenericListener( TGenericEmitter& a_rEmitter, void* a_pCaller, t_Callback a_pCallback, TINT a_iPriority )
{
	m_pCaller   = a_pCaller;
	m_pCallback = a_pCallback;
	SetPriority( a_iPriority );
	a_rEmitter.GetListeners().Insert( this );
}

// $deBlob: FUNCTION 00663210
TGenericListener::~TGenericListener()
{
	Disconnect();
}

TBOOL TGenericListener::Execute( void* pOwner, void* pData )
{
	return m_pCallback( m_pCaller, pOwner, pData );
}

// $deBlob: FUNCTION 00663140
// $Barnyard: FUNCTION 006c4680
void TGenericEmitter::Destroy()
{
	// NOTE: Disconnect unlinks the listener, so always take the first one
	TGenericPriList::TNode* pNode = m_Listeners.TGenericPriList::Begin();

	while ( pNode != m_Listeners.TGenericPriList::End() )
	{
		pNode->As<TGenericListener>()->Disconnect();
		pNode = m_Listeners.TGenericPriList::Begin();
	}

	m_pOwner = TNULL;
}

// $deBlob: FUNCTION 00663130
void TGenericEmitter::Create( void* a_pOwner )
{
	m_pOwner = a_pOwner;
}

// NOTE: in de Blob a listener returning TFALSE stops the event (Barnyard ignores the result)
// $deBlob: FUNCTION 00663190
// $Barnyard: FUNCTION 006c45e0
void TGenericEmitter::Throw( void* a_pData )
{
	TGenericListener* pListener = m_Listeners.TGenericPriList::Begin()->As<TGenericListener>();

	while ( pListener != m_Listeners.TGenericPriList::End() )
	{
		TGenericListener* pNextListener = pListener->Next()->As<TGenericListener>();

		if ( !pListener->Execute( m_pOwner, a_pData ) )
			break;

		pListener = pNextListener;
	}
}

// $deBlob: FUNCTION 00663120
TGenericEmitter::~TGenericEmitter()
{
	Destroy();
}

// $deBlob: FUNCTION 00663100
// $Barnyard: FUNCTION 006c4660
TGenericEmitter::TGenericEmitter( void* a_pOwner )
{
	Create( a_pOwner );
}

TGenericEmitter::TGenericEmitter()
{
	Create( TNULL );
}

TOSHI_NAMESPACE_END
