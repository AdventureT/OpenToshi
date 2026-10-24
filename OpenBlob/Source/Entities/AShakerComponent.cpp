#include "pch.h"
#include "AShakerComponent.h"
#include "ABaseEntity.h"
#include "AEntityData.h"

#include <Math/TMatrix44.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AShakerComponent::Data* AShakerComponent::sm_pData;

// The directions of GetDirection, and the origin
static const TVector4 s_vDirPosX( 1.0f, 0.0f, 0.0f, 1.0f );
static const TVector4 s_vDirNegX( -1.0f, 0.0f, 0.0f, 1.0f );
static const TVector4 s_vDirNegZ( 0.0f, 0.0f, -1.0f, 1.0f );
static const TVector4 s_vDirPosZ( 0.0f, 0.0f, 1.0f, 1.0f );
static const TVector4 s_vOrigin( 0.0f, 0.0f, 0.0f, 1.0f );

// $deBlob: FUNCTION 007df2d0
AShakerComponent::AShakerComponent( ABaseEntity* a_pEntity )
    : AEntityComponent2( a_pEntity )
{
	m_iSwing       = -1;
	m_fAngle       = 0.0f;
	m_bSwingBack   = TFALSE;
	m_fBounce      = 0.0f;
	m_fBounceSize  = 0.1f;
	m_fBounceBaseY = 0.0f;
	m_iBounce      = -1;
	m_bBounceDown  = TFALSE;
}

// $deBlob: FUNCTION 007df980
// $deBlob: FUNCTION 007df320
AShakerComponent::~AShakerComponent()
{
}

// $deBlob: FUNCTION 007df330
void AShakerComponent::LoadData()
{
	sm_pData = TSTATICCAST( Data, AEntityData::GetSingleton()->GetEntityData( "GlobalData_AShakerComponentData_EntityData" ) );
}

// $deBlob: FUNCTION 007df350
void AShakerComponent::ReloadData()
{
	sm_pData = TNULL;
	sm_pData = TSTATICCAST( Data, AEntityData::GetSingleton()->GetEntityData( "GlobalData_AShakerComponentData_EntityData" ) );
}

// $deBlob: FUNCTION 007df370
void AShakerComponent::UnloadData()
{
	sm_pData = TNULL;
}

// $deBlob: FUNCTION 007df380
void AShakerComponent::Tick( TFLOAT a_fDeltaTime )
{
	// The swings: a half swing to the angle, a half swing back, then the next (damped) swing
	if ( m_iSwing >= 0 && m_iSwing < NUM_SWINGS )
	{
		TMatrix44 mMatrix = m_pEntity->GetMatrix();

		TFLOAT fTarget = m_aSwings[ m_iSwing ].m_fAngle;
		TFLOAT fStep   = sm_pData->m_fSwingSpeed * a_fDeltaTime;

		if ( m_bSwingBack == TTRUE )
			fTarget *= -1.0f;

		if ( fTarget < 0.0f || m_fAngle + fStep <= fTarget )
		{
			if ( fTarget < 0.0f )
			{
				if ( m_fAngle - fStep < fTarget )
					fStep = ( fTarget - m_fAngle ) * -1.0f;

				fStep = -fStep;
			}
		}
		else
		{
			fStep = fTarget - m_fAngle;
		}

		TINT iDirection = m_aSwings[ m_iSwing ].m_iDirection;

		if ( iDirection == 0 || iDirection == 1 )
			mMatrix.RotateZ( fStep );
		else
			mMatrix.RotateX( fStep );

		m_pEntity->SetMatrix( mMatrix );
		m_fAngle += fStep;

		if ( ( fStep < 0.0f && m_fAngle <= fTarget ) || ( fStep >= 0.0f && fTarget <= m_fAngle ) )
		{
			m_fAngle = 0.0f;

			if ( !m_bSwingBack )
			{
				m_bSwingBack = TTRUE;
			}
			else
			{
				m_iSwing++;
				m_bSwingBack = TFALSE;
			}
		}
	}

	// The bounces
	if ( m_iBounce >= 0 && m_iBounce < NUM_SWINGS )
	{
		TVector4 vPosition = m_pEntity->GetMatrix().GetTranslation();

		// NOTE: the sign comes from the swing's m_bSwingBack, not from m_bBounceDown
		TFLOAT fOffset = TMath::LERP( 0.0f, m_fBounceSize, m_fBounce );
		vPosition.y    = TFLOAT( m_bSwingBack ? 1 : -1 ) * fOffset + m_fBounceBaseY;
		m_pEntity->SetPosition( vPosition );

		if ( !m_bBounceDown )
		{
			if ( 1.0f <= m_fBounce )
				m_bBounceDown = TTRUE;
		}
		else if ( m_fBounce <= 0.0f )
		{
			m_iBounce++;
			m_bBounceDown = TFALSE;
		}

		m_fBounce = TFLOAT( !m_bBounceDown ? 1 : -1 ) * sm_pData->m_fBounceSpeed * a_fDeltaTime + m_fBounce;
		TMath::Clip( m_fBounce, 0.0f, 1.0f );
	}
}

void AShakerComponent::SetupShake( TVector4& a_rvDirection )
{
	a_rvDirection.Normalise();

	TFLOAT fAngle     = sm_pData->m_fSwingAngle * 0.017453292f;
	TINT   iDirection = GetDirection( a_rvDirection );

	if ( iDirection % 2 == 1 )
		fAngle *= -1.0f;

	m_aSwings[ 0 ].m_fAngle     = fAngle;
	m_aSwings[ 0 ].m_iDirection = iDirection;
	m_aSwings[ 1 ].m_iDirection = iDirection;
	m_aSwings[ 1 ].m_fAngle     = -( sm_pData->m_fSwingDamp * fAngle );
	m_aSwings[ 2 ].m_iDirection = iDirection;
	m_aSwings[ 2 ].m_fAngle     = -( sm_pData->m_fSwingDamp * m_aSwings[ 1 ].m_fAngle );

	m_iSwing      = 0;
	m_fAngle      = 0.0f;
	m_bSwingBack  = TFALSE;
	m_fBounceSize = sm_pData->m_fBounceSize;
	m_fBounce     = 0.0f;
	m_iBounce     = 0;
	m_bBounceDown = TFALSE;

	m_fBounceBaseY = m_pEntity->GetMatrix().GetTranslation().y;
}

// a_pUnk1: the hitting entity (ABaseEntity), else the position of a_pUnk2 (+0x04: a pointer to an object with a
// TVector4 at +0x04, TNULL: the origin) is used
// $deBlob: FUNCTION 007df5d0
void AShakerComponent::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( ( m_iSwing < 0 || m_iSwing >= NUM_SWINGS ) && ( m_iBounce < 0 || m_iBounce >= NUM_SWINGS ) )
	{
		const TVector4& rvEntity = m_pEntity->GetMatrix().GetTranslation();
		TVector4        vDirection;

		if ( a_pCaller == TNULL )
		{
			TUINT8*         pSource = TREINTERPRETCAST( TUINT8*, a_rPacket.m_iData );
			const TVector4& rvFrom  = ( pSource == TNULL ) ? s_vOrigin : *TREINTERPRETCAST( TVector4*, pSource + 4 );

			vDirection = TVector4( rvFrom.x - rvEntity.x, rvFrom.y - rvEntity.y, rvFrom.z - rvEntity.z, rvFrom.w );
		}
		else
		{
			const TVector4& rvFrom = a_pCaller->GetMatrix().GetTranslation();
			vDirection             = TVector4( rvFrom.x - rvEntity.x, rvFrom.y - rvEntity.y, rvFrom.z - rvEntity.z, rvFrom.w );
		}

		SetupShake( vDirection );
	}
}

// $deBlob: FUNCTION 007df770
void AShakerComponent::Stop()
{
	m_iSwing  = -1;
	m_iBounce = -1;
}

// $deBlob: FUNCTION 007df780
void AShakerComponent::Start( ABaseEntity* a_pHitter )
{
	if ( ( m_iSwing < 0 || m_iSwing >= NUM_SWINGS ) && ( m_iBounce < 0 || m_iBounce >= NUM_SWINGS ) )
	{
		TVector4 vDirection;

		if ( a_pHitter == TNULL )
		{
			vDirection = s_vDirPosX;
		}
		else
		{
			const TVector4& rvFrom   = a_pHitter->GetMatrix().GetTranslation();
			const TVector4& rvEntity = m_pEntity->GetMatrix().GetTranslation();
			vDirection               = TVector4( rvFrom.x - rvEntity.x, rvFrom.y - rvEntity.y, rvFrom.z - rvEntity.z, rvFrom.w );
		}

		SetupShake( vDirection );
	}
}

// $deBlob: FUNCTION 007df8e0
TINT AShakerComponent::GetDirection( TVector4 a_vDirection )
{
	TINT   iDirection = 0;
	TFLOAT fBest      = TVector4::DotProduct4( a_vDirection, s_vDirPosX );
	TFLOAT fDot       = TVector4::DotProduct4( a_vDirection, s_vDirNegX );

	if ( fDot > fBest )
	{
		fBest      = fDot;
		iDirection = 1;
	}

	fDot = TVector4::DotProduct4( a_vDirection, s_vDirNegZ );

	if ( fDot > fBest )
	{
		fBest      = fDot;
		iDirection = 2;
	}

	fDot = TVector4::DotProduct4( a_vDirection, s_vDirPosZ );

	if ( fDot > fBest )
		iDirection = 3;

	return iDirection;
}
