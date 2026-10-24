#include "pch.h"
#include "AIdleEntity.h"
#include "AEntityParams.h"
#include "AEntityIOPacket.h"
#include "AEntityModelComponent.h"
#include "Animation/AModelInstance2.h"
#include "Animation/AToshiAnimInterface2.h"

#include <Render/TAnimation.h>
#include <Render/TSkeleton.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AIdleEntity ) == 0xA0 );

// $deBlob: FUNCTION 007b60c0
// $deBlob: FUNCTION 007b60d0
// $deBlob: FUNCTION 007b6160
// $deBlob: FUNCTION 007b61e0
// $deBlob: FUNCTION 007b61f0
// $deBlob: FUNCTION 005416c0
// $deBlob: FUNCTION 0085a030
TDEFINE_CLASS( AIdleEntity );

// $deBlob: FUNCTION 007b6200
AIdleEntity::AIdleEntity()
{
	m_uiIdleFlags = 0;
	SetHash( TFALSE );
}

// $deBlob: FUNCTION 007b64e0
// $deBlob: FUNCTION 007b64f0
AIdleEntity::~AIdleEntity()
{
}

// $deBlob: FUNCTION 007b6260
void AIdleEntity::Create()
{
	// NOTE: retail skips AVisibleEntity::Create
	ABaseEntity::Create();
}

// $deBlob: FUNCTION 007b6270
void AIdleEntity::ProcessParams( const AEntityParams& a_rParams )
{
	AVisibleEntity::ProcessParams( a_rParams );

	const TCHAR* modelName = TNULL;
	a_rParams.GetValue( modelName, "Mesh" );
	TASSERT( TNULL != modelName && 0 != modelName[ 0 ] );

	CreateModelComponent( modelName );
	CreateFXBreakpointComponent( modelName );

	// NOTE: retail processes the base's params a second time
	AVisibleEntity::ProcessParams( a_rParams );

	const TCHAR* szAnimation = TNULL;
	a_rParams.GetValue( szAnimation, "DefaultAnimation" );

	if ( szAnimation != TNULL && szAnimation[ 0 ] != '\0' )
	{
		AToshiAnimInterface2* pAnimInterface = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetAnimInterface();
		TUINT16               iAnim          = TUINT16( pAnimInterface->GetAnimID( szAnimation ) );
		TASSERT( TUINT16( -1 ) != iAnim, "IdleEntity animation name '%s' is not found" );

		if ( iAnim != TUINT16( -1 ) )
		{
			TAnimation* pAnimation = pAnimInterface->AddAnim( TINT16( iAnim ), 0.15f, 1.0f, 1.0f );

			if ( pAnimation != TNULL )
			{
				TFLOAT fStartTime = 0.0f;
				a_rParams.GetValue( fStartTime, "DefaultAnimationStartTime" );

				if ( fStartTime >= 0.0f )
					pAnimation->SetTime( fStartTime );
				else
					pAnimation->SetTime( pAnimation->GetSequencePtr()->GetDuration() );

				pAnimation->SetState( TAnimation::STATE_BLENDING_IN );
			}
		}
	}

	TBOOL bStartVisible = TFALSE;
	a_rParams.GetValue( bStartVisible, "StartVisible" );

	if ( bStartVisible )
		m_uiIdleFlags |= IDLEFLAG_StartVisible;

	ApplyStartVisible();
}

// $deBlob: FUNCTION 007b6450
void AIdleEntity::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	AVisibleEntity::HandleEntityIO( a_pCaller, a_rPacket );

	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_TRIGGER:
		case AEntityIOPacket::MESSAGE_TOGGLE:
			SetVisible( TTRUE );
			m_Flags &= ~FLAG_DontTick;
			break;
	}
}

// $deBlob: FUNCTION 007b6490
void AIdleEntity::Reset( ABaseEntity* a_pEntity )
{
	AVisibleEntity::Reset( a_pEntity );
	ApplyStartVisible();
}

void AIdleEntity::ApplyStartVisible()
{
	SetVisible( m_uiIdleFlags & IDLEFLAG_StartVisible );

	if ( m_uiIdleFlags & IDLEFLAG_StartVisible )
		m_Flags &= ~FLAG_DontTick;
	else
		m_Flags |= FLAG_DontTick;
}
