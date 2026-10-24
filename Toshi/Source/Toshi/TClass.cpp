#include "ToshiPCH.h"
#include "TClass.h"
#include "TSystem.h"
#include "T2String8.h"
#include "Toshi/Typedefs.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 0065ca80
// $Barnyard: FUNCTION 006bbf30
TClass::TClass( const TCHAR* a_szName, TClass* a_pParentClass, CreateObject_t a_fnCreate, CreateObjectInPlace_t a_fnCreateInPlace, Initialise_t a_fnInitialise, Deinitialise_t a_fnUnitialise, TUINT32 a_uiVersion, TUINT32 a_uiClassSize, TUINT32 a_uiClassAlignment )
    : m_szName( a_szName ), m_pParent( a_pParentClass ), m_fnCreate( a_fnCreate ), m_fnCreateInPlace( a_fnCreateInPlace ), m_fnInitialise( a_fnInitialise ), m_fnDeinitialise( a_fnUnitialise ), m_uiVersion( a_uiVersion ), m_uiClassSize( a_uiClassSize ), m_uiClassAlignment( a_uiClassAlignment ), m_bInitialised( TFALSE )
{
	if ( m_pParent )
	{
		TClass* pClass = m_pParent->m_pLastChildren;

		while ( pClass != TNULL )
		{
			if ( pClass == this ) return;
			pClass = pClass->m_pPrevious;
		}

		m_pPrevious                = m_pParent->m_pLastChildren;
		m_pParent->m_pLastChildren = this;
	}
}

// $deBlob: FUNCTION 0065caf0
// $Barnyard: FUNCTION 006bbfa0
TClass::~TClass()
{
	m_pLastChildren = TNULL;
	m_pPrevious     = TNULL;
	m_pParent       = TNULL;

	// Uninitialize static
	if ( m_bInitialised == TTRUE && m_fnDeinitialise )
	{
		m_bInitialised = TFALSE;
		m_fnDeinitialise();
	}
}

// $deBlob: FUNCTION 0065cba0
// $Barnyard: FUNCTION 006bbd20
void TClass::Deinitialize()
{
	if ( m_fnDeinitialise )
	{
		TASSERT( TTRUE == m_bInitialised );
		m_bInitialised = TFALSE;
		m_fnDeinitialise();
	}
}

// $deBlob: FUNCTION 0065cb50
// $Barnyard: FUNCTION 006bbd00
void TClass::Initialize()
{
	if ( m_fnInitialise )
	{
		TASSERT( TFALSE == m_bInitialised );
		m_fnInitialise();
		m_bInitialised = TTRUE;
	}
}

// $deBlob: FUNCTION 0065cfc0
// $Barnyard: FUNCTION 006bbfe0
void TClass::RecurseTree( RecurseTreeBaseBeginCb_t fBaseBegin, RecurseTreeBaseEndCb_t fBaseEnd, RecurseTreeCheck_t fCheck, void* custom )
{
	if ( fCheck( this, custom ) == TTRUE )
	{
		if ( fBaseBegin ) fBaseBegin( this, custom );
		RecurseTree2( fBaseBegin, fBaseEnd, fCheck, custom );
		if ( fBaseEnd ) fBaseEnd( this, custom );
	}
}

// $deBlob: FUNCTION 0065d020
// $Barnyard: FUNCTION 006bbe10
void TClass::RecurseTree2( RecurseTreeBaseBeginCb_t fBaseBegin, RecurseTreeBaseEndCb_t fBaseEnd, RecurseTreeCheck_t fCheck, void* custom )
{
	for ( TClass* pClass = m_pLastChildren; pClass != TNULL; pClass = pClass->m_pPrevious )
	{
		if ( fCheck ) fCheck( pClass, custom );

		if ( pClass->m_pLastChildren )
		{
			// NOTE: de Blob passes this class (the one being iterated), not the child, to the begin/end callbacks
			if ( fBaseBegin ) fBaseBegin( this, custom );
			pClass->RecurseTree2( fBaseBegin, fBaseEnd, fCheck, custom );
			if ( fBaseEnd ) fBaseEnd( this, custom );
		}
	}
}

// $deBlob: FUNCTION 0065d090
// $Barnyard: FUNCTION 006bbdb0
const TClass* TClass::FindRecurse( const TCHAR* name, const TClass* parent, TBOOL hasPrevious )
{
	while ( parent )
	{
		const TClass* previous   = hasPrevious == TTRUE ? parent->m_pPrevious : TNULL;
		TINT          difference = Toshi::TStringManager::String8CompareNoCase( parent->m_szName, name, -1 );

		if ( difference == 0 )
		{
			return parent;
		}

		if ( parent->m_pLastChildren )
		{
			const TClass* result = FindRecurse( name, parent->m_pLastChildren, TTRUE );

			if ( result )
			{
				return result;
			}
		}

		parent = previous;
	}

	return TNULL;
}

// $deBlob: FUNCTION 0065ced0
// $Barnyard: FUNCTION 006bbfc0
const TClass* TClass::Find( const TCHAR* a_pcClassName, const TClass* parent )
{
	TASSERT( a_pcClassName[ 1 ] != 0 );

	if ( parent == TNULL ) parent = &TObject::TClassObjectName;
	return FindRecurse( a_pcClassName, parent, TFALSE );
}

// $deBlob: FUNCTION 0065cb20
// $Barnyard: FUNCTION 006bbce0
TObject* TClass::CreateObject() const
{
	if ( m_fnCreate != TNULL )
	{
		return m_fnCreate();
	}

	return TNULL;
}

// $deBlob: FUNCTION 0065cd60
// $Barnyard: FUNCTION 006bbd60
TBOOL TClass::IsA( const TClass* cmpClass ) const
{
	if ( cmpClass->m_pLastChildren == TNULL )
	{
		return ( this == cmpClass );
	}
	else
	{
		const TClass* pClass = this;

		while ( pClass != TNULL )
		{
			if ( pClass == cmpClass )
			{
				return TTRUE;
			}

			pClass = pClass->m_pParent;
		}
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0065cc50
// $Barnyard: FUNCTION 006bc020
TUINT32 TClass::GetMaxSizeOfDerivedClasses()
{
	// $deBlob: FUNCTION 0065d190
	// $Barnyard: FUNCTION 006bbe80
	auto GetMaxSizeOfClass = []( TClass* a_pClass, void* a_pSize ) -> TBOOL {
		// NOTE: a signed comparison in retail
		if ( TINT( a_pClass->m_uiClassSize ) > *(TINT*)a_pSize )
		{
			*(TUINT32*)a_pSize = a_pClass->m_uiClassSize;
		}

		return TTRUE;
	};

	TUINT32 uiClassSize = 0;
	if ( GetMaxSizeOfClass( this, &uiClassSize ) )
	{
		RecurseTree2( TNULL, TNULL, GetMaxSizeOfClass, &uiClassSize );
	}

	return uiClassSize;
}

// $deBlob: FUNCTION 0065cce0
// $Barnyard: FUNCTION 006bc060
TUINT32 TClass::GetMaxAlignmentOfDerivedClasses()
{
	// $deBlob: FUNCTION 0065d1b0
	// $Barnyard: FUNCTION 006bbea0
	auto GetMaxAlignmentOfClass = []( TClass* a_pClass, void* a_pAlignment ) -> TBOOL {
		// NOTE: a signed comparison in retail
		if ( TINT( a_pClass->m_uiClassAlignment ) > *(TINT*)a_pAlignment )
		{
			*(TUINT32*)a_pAlignment = a_pClass->m_uiClassAlignment;
		}

		return TTRUE;
	};

	TUINT32 uiAlignment = 0;
	if ( GetMaxAlignmentOfClass( this, &uiAlignment ) )
	{
		RecurseTree2( TNULL, TNULL, GetMaxAlignmentOfClass, &uiAlignment );
	}

	return uiAlignment;
}

// $deBlob: FUNCTION 0065cda0
TBOOL TClass::IsAttached() const
{
	if ( m_pParent == TNULL )
		return TFALSE;

	for ( TClass* pClass = m_pParent->m_pLastChildren; pClass != TNULL; pClass = pClass->m_pPrevious )
	{
		if ( pClass == this )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0065cde0
TBOOL TClass::AttachClassToParent()
{
	if ( m_pParent != TNULL )
	{
		TClass* pClass = m_pParent->m_pLastChildren;

		while ( pClass != TNULL )
		{
			if ( pClass == this )
				return TFALSE;

			pClass = pClass->m_pPrevious;
		}

		m_pPrevious                = m_pParent->m_pLastChildren;
		m_pParent->m_pLastChildren = this;
		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 0065ce20
const TClass* TClass::FindCommonBaseClass( const TClass* a_pClass1, const TClass* a_pClass2 )
{
	if ( a_pClass1 == a_pClass2 )
		return a_pClass1;

	const TClass* aChain1[ 16 ];
	const TClass* aChain2[ 16 ];
	TINT          iCount1 = 0;
	TINT          iCount2 = 0;

	for ( const TClass* pClass = a_pClass1; pClass != TNULL; pClass = pClass->m_pParent )
		aChain1[ iCount1++ ] = pClass;

	for ( const TClass* pClass = a_pClass2; pClass != TNULL; pClass = pClass->m_pParent )
		aChain2[ iCount2++ ] = pClass;

	// Walk both chains down from the root while they match
	const TClass* pCommon = TNULL;

	while ( iCount1 != 0 && iCount2 != 0 && aChain1[ iCount1 - 1 ] == aChain2[ iCount2 - 1 ] )
	{
		pCommon = aChain1[ iCount1 - 1 ];
		iCount1--;
		iCount2--;
	}

	return pCommon;
}

static TINT s_iClassTreeDepth;

// $deBlob: FUNCTION 0065cf30
void TClass::DumpObjectClassTree()
{
	// $deBlob: FUNCTION 0065d1d0
	auto BaseBegin = []( TClass* a_pClass, void* a_pUserData ) -> TBOOL {
		s_iClassTreeDepth++;
		return TTRUE;
	};

	// $deBlob: FUNCTION 0065d1f0
	auto BaseEnd = []( TClass* a_pClass, void* a_pUserData ) -> TBOOL {
		s_iClassTreeDepth--;
		return TTRUE;
	};

	// $deBlob: FUNCTION 0065d1e0
	auto Check = []( TClass* a_pClass, void* a_pUserData ) -> TBOOL {
		// NOTE: the printing has been compiled out of retail
		return TTRUE;
	};

	s_iClassTreeDepth = 0;

	TClass* pClass;

	if ( T2String8::CompareNoCase( TGetClass( TObject ).m_szName, "TObject", -1 ) == 0 )
	{
		pClass = &TGetClass( TObject );
	}
	else
	{
		pClass = TGetClass( TObject ).m_pLastChildren ?
		    const_cast<TClass*>( FindRecurse( "TObject", TGetClass( TObject ).m_pLastChildren, TTRUE ) ) :
		    TNULL;
	}

	pClass->RecurseTree( BaseBegin, BaseEnd, Check, TNULL );
}

TBOOL TClass::TryInitialize( TClass* tClass )
{
	if ( !tClass->IsInitialized() )
	{
		tClass->Initialize();
	}

	return TTRUE;
}

TOSHI_NAMESPACE_END
