#include "pch.h"
#include "ABlobChallengeDesc.h"
#include "Challenges/AChallengeManager.h"
#include "Locale/ALocaleManager.h"
#include <T2Locale/T2Locale.h>
#include <XUI/TXUIText.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00642b10
// $deBlob: FUNCTION 00642b20
// $deBlob: FUNCTION 00642ba0
// $deBlob: FUNCTION 00642c20
// $deBlob: FUNCTION 00642c30
// $deBlob: FUNCTION 0053dbd0
// $deBlob: FUNCTION 00858a70
TDEFINE_CLASS( ABlobChallengeDesc );

// $deBlob: FUNCTION 00642cf0
ABlobChallengeDesc::ABlobChallengeDesc()
    : ABlobMenu( -1 )
{
}

// $deBlob: FUNCTION 00642d30
// $deBlob: FUNCTION 00642d10
// $deBlob: FUNCTION 00642d22
ABlobChallengeDesc::~ABlobChallengeDesc()
{
}

// $deBlob: FUNCTION 00642c40
void ABlobChallengeDesc::OnCreated()
{
	ABlobMenu::OnCreated();

	TXUIText*   pText          = TSTATICCAST( TXUIText, FindChildXUIElement( 4 ) );
	AChallenge* pCurrChallenge = AChallengeManager::GetSingleton()->GetCurrentChallenge();
	TASSERT( TNULL != pCurrChallenge );

	ALocaleManager* pLocale   = TSTATICCAST( ALocaleManager, T2Locale::GetSingleton() );
	TINT            iStringID = pCurrChallenge->GetData()->m_iStringID;

	if ( iStringID == -1 )
		return;

	pText->SetText( pLocale->GetString( iStringID ) );
	ALocaleManager::StringFormat* pFormat = pLocale->GetStringFormat( iStringID );

	if ( pFormat != TNULL )
		pText->SetColours( pLocale->GetOverwrittenFormatString( iStringID, pFormat->m_wszFormat ), TREINTERPRETCAST( TUINT32*, pLocale->GetFormatUnk04() ) );
	else
		pText->SetColours( TNULL, TNULL );
}
