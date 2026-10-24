#include "pch.h"
#include "ASafeZone.h"
#include "AGUIManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The colour of the borders
static const TUINT32 s_uiSafeZoneColour = 0xFFFFFFFF;

// $deBlob: FUNCTION 005b21b0
ASafeZone::ASafeZone()
{
}

// $deBlob: FUNCTION 005b2220
// $deBlob: FUNCTION 005b2540
ASafeZone::~ASafeZone()
{
	// The borders are removed before they're destroyed
	for ( TINT i = 0; i < 4; i++ )
		m_aBorders[ i ].RemoveSelf();
}

// $deBlob: FUNCTION 005b2300
void ASafeZone::Create()
{
	TFLOAT fWidth, fHeight;
	AGUIManager::GetSingleton()->GetSafeArea()->GetDimensions( fWidth, fHeight );

	SetDimensions( fWidth, fHeight );
	m_aBorders[ 2 ].SetDimensions( fWidth + 8.0f, 4.0f );
	m_aBorders[ 3 ].SetDimensions( fWidth + 8.0f, 4.0f );
	m_aBorders[ 0 ].SetDimensions( 4.0f, fHeight + 8.0f );
	m_aBorders[ 1 ].SetDimensions( 4.0f, fHeight + 8.0f );

	m_aBorders[ 2 ].SetAttachment( T2GUIATTACHMENT_TOPCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_aBorders[ 3 ].SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_TOPCENTER );
	m_aBorders[ 0 ].SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLERIGHT );
	m_aBorders[ 1 ].SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLELEFT );

	for ( TINT i = 0; i < 4; i++ )
	{
		m_aBorders[ i ].SetColour( s_uiSafeZoneColour );
		AddChildTail( &m_aBorders[ i ] );
	}

	SetVisible( TFALSE );
}
