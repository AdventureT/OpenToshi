#include "pch.h"
#include "AParticleEffectHelper.h"

#include <TParticleSystem/TParticleEffect.h>
#include <TParticleSystem/TGenericParticlesSettings.h>
#include <TParticleSystem/TParticleInit.h>
#include <TParticleSystem/TParticleKeyframe.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005b8200
void AParticleEffectHelper::SetColour( TParticleEffect* a_pEffect, TUINT32 a_uiColour, const TCHAR* a_szSystem )
{
	for ( TINT i = 0; i < a_pEffect->GetNumSystems(); i++ )
	{
		TParticleEffectSystem& rSystem = a_pEffect->GetSystem( i );

		if ( a_szSystem != TNULL && T2String8::CompareNoCase( a_szSystem, rSystem.m_szSettings, T2String8::Length( a_szSystem ) ) != 0 )
			continue;

		TGenericParticlesSettings* pSettings = rSystem.m_pSettings;
		TParticleInit*             pInit     = pSettings->m_pColourInit;
		TASSERT( pInit->GetType() == TParticleInit::TYPECOLOUR_FIXED );

		// Keeps the alpha
		TUINT32 uiRGB = ( ( ( ( a_uiColour >> 16 ) & 0xFF ) << 8 | ( ( a_uiColour >> 8 ) & 0xFF ) ) << 8 ) | ( a_uiColour & 0xFF );

		TParticleInitColour::FixedData* pData = pInit->GetData<TParticleInitColour::FixedData>();
		pData->m_uiColour                     = ( pData->m_uiColour & 0xFF000000 ) | uiRGB;

		TParticleKeyframeTimeline* pColKeys = pSettings->m_pColourKeys;

		if ( pColKeys != TNULL )
		{
			for ( TINT j = 0; j < pColKeys->GetNumKeyframes(); j++ )
			{
				TParticleKeyframe* pColKey = pColKeys->GetKeyframe( j );
				TASSERT( TParticleKeyframe::KEYFRAMETYPE_COL == pColKey->GetType() );

				TParticleKeyframe::ColourData* pKeyData = pColKey->GetData<TParticleKeyframe::ColourData>();
				pKeyData->m_uiColour                    = ( pKeyData->m_uiColour & 0xFF000000 ) | uiRGB;
			}
		}
	}
}

// The float data of the size inits (TYPEFLOAT_FIXED: m_fMin, TYPEFLOAT_MINMAX: both)
struct SizeInitData : TParticleInit::TRBHeader
{
	TFLOAT m_fMin; // 0x0C
	TFLOAT m_fMax; // 0x10
};

// $deBlob: FUNCTION 005b8390
void AParticleEffectHelper::SetPositionRadius( TParticleEffect* a_pEffect, TFLOAT a_fRadius )
{
	TINT iNumSystems = a_pEffect->GetNumSystems();

	for ( TINT i = 0; i < iNumSystems; i++ )
	{
		TParticleInit::TRBHeader* pData = a_pEffect->GetSystem( i ).m_pSettings->m_pPositionInit->GetData<TParticleInit::TRBHeader>();

		if ( pData->m_eType == TParticleInit::TYPEV4_RANDDIR2 )
			TSTATICCAST( SizeInitData, pData )->m_fMin = a_fRadius;
	}
}

// $deBlob: FUNCTION 005b8420
void AParticleEffectHelper::SetPositionExtentsXZ( TParticleEffect* a_pEffect, TFLOAT a_fXZ )
{
	SetPositionExtents( a_pEffect, a_fXZ, -1.0f, a_fXZ );
}

// $deBlob: FUNCTION 005b8450
void AParticleEffectHelper::SetPositionExtents( TParticleEffect* a_pEffect, TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ )
{
	TINT iNumSystems = a_pEffect->GetNumSystems();

	for ( TINT i = 0; i < iNumSystems; i++ )
	{
		TParticleInit* pInit = a_pEffect->GetSystem( i ).m_pSettings->m_pPositionInit;

		if ( pInit->GetType() == TParticleInit::TYPEV4_MINMAX )
		{
			TParticleInitV4::MinMaxData* pData = pInit->GetData<TParticleInitV4::MinMaxData>();

			pData->m_vMax.z = a_fZ;
			pData->m_vMin.x = -a_fX;
			pData->m_vMin.z = -a_fZ;
			pData->m_vMax.x = a_fX;

			if ( 0.0f <= a_fY )
			{
				pData->m_vMax.y = a_fY;
				pData->m_vMin.y = -a_fY;
			}
		}
		else if ( pInit->GetType() == TParticleInit::TYPEV4_CUBESURFACE )
		{
			// NOTE: the size of the cube (0x0C) has the layout of TYPEV4_FIXED's value
			TParticleInitV4::FixedData* pData = pInit->GetData<TParticleInitV4::FixedData>();

			pData->m_vValue.x = a_fX;

			if ( 0.0f <= a_fY )
				pData->m_vValue.y = a_fY;

			pData->m_vValue.z = a_fZ;
		}
	}
}

// $deBlob: FUNCTION 005b8560
void AParticleEffectHelper::ScalePositionInit( TParticleEffect* a_pEffect, const TParticleEffect* a_pBaseEffect, const TVector4* a_pScale )
{
	TINT iNumSystems = a_pEffect->GetNumSystems();
	TASSERT( iNumSystems == a_pBaseEffect->GetNumSystems() );

	for ( TINT i = 0; i < iNumSystems; i++ )
	{
		TParticleInit* pInit     = a_pEffect->GetSystem( i ).m_pSettings->m_pPositionInit;
		TParticleInit* pBaseInit = a_pBaseEffect->GetSystem( i ).m_pSettings->m_pPositionInit;
		TASSERT( pInit->GetType() == pBaseInit->GetType() );

		if ( pInit->GetType() == TParticleInit::TYPEV4_MINMAX )
		{
			TParticleInitV4::MinMaxData* pData     = pInit->GetData<TParticleInitV4::MinMaxData>();
			TParticleInitV4::MinMaxData* pBaseData = pBaseInit->GetData<TParticleInitV4::MinMaxData>();

			pData->m_vMin.x = pBaseData->m_vMin.x * a_pScale->x;
			pData->m_vMin.z = pBaseData->m_vMin.z * a_pScale->z;
			pData->m_vMax.x = pBaseData->m_vMax.x * a_pScale->x;
			pData->m_vMax.z = pBaseData->m_vMax.z * a_pScale->z;

			if ( a_pScale->y >= 0.0f )
			{
				pData->m_vMin.y = pBaseData->m_vMin.y * a_pScale->y;
				pData->m_vMax.y = pBaseData->m_vMax.y * a_pScale->y;
			}
		}
		else if ( pInit->GetType() == TParticleInit::TYPEV4_CUBESURFACE )
		{
			// NOTE: the size of the cube (0x0C) has the layout of TYPEV4_FIXED's value
			TParticleInitV4::FixedData* pData     = pInit->GetData<TParticleInitV4::FixedData>();
			TParticleInitV4::FixedData* pBaseData = pBaseInit->GetData<TParticleInitV4::FixedData>();

			pData->m_vValue.x = pBaseData->m_vValue.x * a_pScale->x;

			if ( a_pScale->y >= 0.0f )
				pData->m_vValue.y = pBaseData->m_vValue.y * a_pScale->y;

			pData->m_vValue.z = pBaseData->m_vValue.z * a_pScale->z;
		}
	}
}

// $deBlob: FUNCTION 005b8770
void AParticleEffectHelper::SetNumParticles( TParticleEffect* a_pEffect, TINT a_iNumParticles )
{
	TASSERT( 1 == a_pEffect->GetNumSystems() );
	a_pEffect->GetSystem( 0 ).m_pSettings->m_iNumParticles = a_iNumParticles;
}

// $deBlob: FUNCTION 005b8800
TINT AParticleEffectHelper::ScaleNumParticles( const TParticleEffect* a_pSrcEffect, TParticleEffect* a_pDstEffect, TFLOAT a_fScale, const TCHAR* a_szSystem )
{
	TASSERT( a_pSrcEffect->GetNumSystems() == a_pDstEffect->GetNumSystems() );

	TINT iMaxParticles = 0;

	for ( TINT i = 0; i < a_pSrcEffect->GetNumSystems(); i++ )
	{
		TParticleEffectSystem& rSrcSystem = a_pSrcEffect->GetSystem( i );

		if ( a_szSystem == TNULL || T2String8::CompareNoCase( a_szSystem, rSrcSystem.m_szSettings, T2String8::Length( a_szSystem ) ) == 0 )
		{
			TINT& riNumParticles = a_pDstEffect->GetSystem( i ).m_pSettings->m_iNumParticles;
			riNumParticles       = TINT( TFLOAT( rSrcSystem.m_pSettings->m_iNumParticles ) * a_fScale );

			if ( iMaxParticles < riNumParticles )
				iMaxParticles = riNumParticles;
		}
	}

	return iMaxParticles;
}

// $deBlob: FUNCTION 005b8960
TINT AParticleEffectHelper::GetSize( TParticleEffect* a_pEffect, TINT a_iSystem, TFLOAT& a_rfMin, TFLOAT& a_rfMax )
{
	TASSERT( -1 < a_iSystem );
	TASSERT( a_iSystem < a_pEffect->GetNumSystems() );

	SizeInitData* pData = a_pEffect->GetSystem( a_iSystem ).m_pSettings->m_pSizeInit->GetData<SizeInitData>();

	if ( pData->m_eType == TParticleInit::TYPEFLOAT_FIXED )
	{
		a_rfMax = pData->m_fMin;
		a_rfMin = pData->m_fMin;
	}
	else if ( pData->m_eType == TParticleInit::TYPEFLOAT_MINMAX )
	{
		a_rfMin = pData->m_fMin;
		a_rfMax = pData->m_fMax;
	}

	return pData->m_eType;
}

// $deBlob: FUNCTION 005b8a60
void AParticleEffectHelper::SetSize( TParticleEffect* a_pEffect, TFLOAT a_fMin, TFLOAT a_fMax, TINT a_iType )
{
	TINT iNumSystems = a_pEffect->GetNumSystems();

	for ( TINT i = 0; i < iNumSystems; i++ )
	{
		TParticleInit* pInit = a_pEffect->GetSystem( i ).m_pSettings->m_pSizeInit;

		// NOTE: only the type changes, not the init function
		if ( -1 < a_iType )
			pInit->GetData<TParticleInit::TRBHeader>()->m_eType = a_iType;

		SizeInitData* pData = pInit->GetData<SizeInitData>();

		if ( pData->m_eType == TParticleInit::TYPEFLOAT_FIXED )
		{
			pData->m_fMin = a_fMin;
		}
		else if ( pData->m_eType == TParticleInit::TYPEFLOAT_MINMAX )
		{
			pData->m_fMin = a_fMin;
			pData->m_fMax = a_fMax;
		}
	}
}
