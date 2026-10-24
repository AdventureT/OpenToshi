#include "pch.h"
#include "ABlobSplatTrail.h"
#include "ABlobEntity.h"
#include "ABlobColourComponent.h"
#include "GameInterface/APlayerManager.h"
#include "Render/Decal/ADecalManager.h"
#include "ARandom.h"

#include <Toshi/TSystem.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007b9f90
ABlobSplatTrail::ABlobSplatTrail( ABlobEntity* a_pBlob )
    : m_pBlob( a_pBlob )
    , m_vLastPos( TVector4::VEC_ZERO )
{
	m_fLastSquashCompensation = 1.0f;
	m_uiLastFrame             = 0;
}

// $deBlob: FUNCTION 007b9fc0
void ABlobSplatTrail::Update()
{
	if ( ( m_pBlob->m_uiFlags & 0xC ) == 0 )
		return;

	TVector4 vPos         = m_pBlob->m_oPhysicsMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
	TUINT32  uiFrame      = TSystemManager::GetSingleton()->GetFrameCount();
	TFLOAT   fDistance    = TVector4::Distance( m_vLastPos, vPos );
	TFLOAT   fRadius      = ABlobEntity::ms_pBlobProperties[ m_pBlob->m_iSize ].m_fRadius;
	TFLOAT   fSize        = fRadius * 0.66f;
	TFLOAT   fMinDistance = fSize * 0.8f;

	if ( uiFrame != m_uiLastFrame &&
	     ( fMinDistance < fDistance || 0.1f < TMath::Abs( m_fLastSquashCompensation - m_pBlob->m_fSquashCompensation ) ) &&
	     TUINT8( m_pBlob->m_oMotion.m_uiContactCountAndGround ) != 0 )
	{
		// The next splat: fMinDistance on from the last one, or here if the Blob went much further
		TVector4 vSplatPos;

		if ( fDistance <= fMinDistance * 2.0f )
		{
			vSplatPos = m_vLastPos;

			if ( 0.0f < fDistance )
				vSplatPos.Lerp3( vSplatPos, vPos, fMinDistance / fDistance );
		}
		else
		{
			vSplatPos = vPos;
		}

		m_vLastPos = vSplatPos;

		TFLOAT fRandom = ARandom::GetSingleton()->GetRandom().GetFloat();
		ADecalManager::GetSingleton()->SetRotation( fRandom * TMath::TWO_PI );

		ABlobColourComponent* pColourComponent = m_pBlob->m_pColourComponent;

		if ( pColourComponent->GetCurrentColour() != 0 )
		{
			TUINT32      uiColour = pColourComponent->GetFadeColour2();
			ABlobEntity* pPlayer  = TSTATICCAST( ABlobEntity, APlayerManager::GetSingleton()->GetPlayerBlob( 0 ) );

			ADecalManager::GetSingleton()->AddDecal( &vSplatPos, fSize, fRadius, uiColour, pPlayer->GetSplatMaterial(), ( fRandom <= 0.5f ) ? ADecalManager::Type_BottomLeft : ADecalManager::Type_Unk1 );
		}

		m_uiLastFrame             = uiFrame;
		m_fLastSquashCompensation = m_pBlob->m_fSquashCompensation;
	}
}
