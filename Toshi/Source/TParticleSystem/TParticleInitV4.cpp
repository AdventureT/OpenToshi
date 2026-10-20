#include "ToshiPCH.h"
#include "TParticleInit.h"
#include "TGenericParticles.h"
#include "Math/TRandom.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $deBlob: FUNCTION 0053ffd0
static TRandom s_oRandom;

// The data of the init types that only have floats after TParticleInit::TRBHeader
struct InitV4FloatData : TParticleInit::TRBHeader
{
	TFLOAT m_afValues[ 4 ]; // 0x0C
};

static InitV4FloatData* GetFloatData( TParticleInit::TRBHeader* a_pData )
{
	return TSTATICCAST( InitV4FloatData, a_pData );
}

// $deBlob: FUNCTION 0071ca70
void __stdcall TParticleInitV4::InitFixed( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	FixedData* pData = TSTATICCAST( FixedData, a_pData );
	TFLOAT     fX    = pData->m_vValue.x;
	TFLOAT     fY    = pData->m_vValue.y;
	TFLOAT     fZ    = pData->m_vValue.z;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVec = ( *a_pBuffer )[ i ];
		rVec.x         = fX;
		rVec.y         = fY;
		rVec.z         = fZ;
		rVec.w         = 1.0f;
	}
}

// $deBlob: FUNCTION 0071ccb0
void __stdcall TParticleInitV4::InitMinMax( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	MinMaxData* pData = TSTATICCAST( MinMaxData, a_pData );
	TFLOAT      fMinX = pData->m_vMin.x;
	TFLOAT      fMinY = pData->m_vMin.y;
	TFLOAT      fMinZ = pData->m_vMin.z;
	TFLOAT      fMaxX = pData->m_vMax.x;
	TFLOAT      fMaxY = pData->m_vMax.y;
	TFLOAT      fMaxZ = pData->m_vMax.z;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVec = ( *a_pBuffer )[ i ];
		rVec.x         = s_oRandom.GetFloatMinMax( fMinX, fMaxX );
		rVec.y         = s_oRandom.GetFloatMinMax( fMinY, fMaxY );
		TFLOAT fZ      = s_oRandom.GetFloatMinMax( fMinZ, fMaxZ );
		rVec.w         = 1.0f;
		rVec.z         = fZ;
	}
}

// $deBlob: FUNCTION 0071ce50
void __stdcall TParticleInitV4::InitRandDir( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVec = ( *a_pBuffer )[ i ];
		rVec.x         = s_oRandom.GetFloat2();
		rVec.y         = s_oRandom.GetFloat2();
		rVec.z         = s_oRandom.GetFloat2();
		rVec.Normalise();
		rVec.Multiply( GetFloatData( a_pData )->m_afValues[ 0 ] );
		rVec.w = 1.0f;
	}
}

// $deBlob: FUNCTION 0071cfb0
void __stdcall TParticleInitV4::InitDirTowardsCenter( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	TParticleV4Buffer& rPositions = TParticleInit::sm_pCurParticles->m_avPosition;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVec = ( *a_pBuffer )[ i ];
		rVec.Negate3( rPositions[ i ] );
		rVec.Normalise();
		rVec.Multiply( GetFloatData( a_pData )->m_afValues[ 0 ] );
		rVec.w = 1.0f;
	}
}

// $deBlob: FUNCTION 0071d050
void __stdcall TParticleInitV4::InitDirAwayFromCenter( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	TParticleV4Buffer& rPositions = TParticleInit::sm_pCurParticles->m_avPosition;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVec = ( *a_pBuffer )[ i ];
		rVec           = rPositions[ i ];
		rVec.Normalise();
		rVec.Multiply( GetFloatData( a_pData )->m_afValues[ 0 ] );
		rVec.w = 1.0f;
	}
}

// $deBlob: FUNCTION 0071d2a0
void __stdcall TParticleInitV4::InitDirInCone2( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	InitV4FloatData* pData   = GetFloatData( a_pData );
	TFLOAT           fRadius = pData->m_afValues[ 0 ];
	TFLOAT           fMin    = pData->m_afValues[ 2 ];
	TFLOAT           fHeight = pData->m_afValues[ 1 ];
	TFLOAT           fMax    = pData->m_afValues[ 3 ];

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TFLOAT    fRand = s_oRandom.GetFloat();
		TVector4& rVec  = ( *a_pBuffer )[ i ];
		rVec.y          = 0.0f;
		rVec.z          = 0.0f;
		rVec.w          = 1.0f;
		rVec.x          = fRand * fRadius;
		rVec.RotateY( s_oRandom.GetFloat2() * TMath::PI );
		rVec.y = fHeight;
		rVec.Normalise();
		rVec.Multiply( s_oRandom.GetFloatMinMax( fMin, fMax ) );
		rVec.w = 1.0f;
	}
}

// $deBlob: FUNCTION 0071d440
void __stdcall TParticleInitV4::InitDirInCone( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	InitV4FloatData* pData     = GetFloatData( a_pData );
	TFLOAT           fMin      = pData->m_afValues[ 2 ];
	TFLOAT           fMax      = pData->m_afValues[ 3 ];
	TFLOAT           fMinAngle = pData->m_afValues[ 0 ];
	TFLOAT           fMaxAngle = pData->m_afValues[ 1 ];

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TFLOAT    fAngle = s_oRandom.GetFloatMinMax( fMinAngle, fMaxAngle );
		TVector4& rVec   = ( *a_pBuffer )[ i ];
		rVec.x           = 0.0f;
		rVec.y           = -1.0f;
		rVec.z           = 0.0f;
		rVec.w           = 1.0f;
		rVec.RotateZ( fAngle );
		rVec.RotateY( s_oRandom.GetFloat2() * TMath::PI );
		rVec.Normalise();
		rVec.Multiply( s_oRandom.GetFloatMinMax( fMin, fMax ) );
		rVec.w = 1.0f;
	}
}

// $deBlob: FUNCTION 0071d600
void __stdcall TParticleInitV4::InitRandDirRange( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVec = ( *a_pBuffer )[ i ];
		rVec.x         = s_oRandom.GetFloat2();
		rVec.y         = s_oRandom.GetFloat2();
		rVec.z         = s_oRandom.GetFloat2();
		rVec.Normalise();
		InitV4FloatData* pData = GetFloatData( a_pData );
		rVec.Multiply( s_oRandom.GetFloatMinMax( pData->m_afValues[ 0 ], pData->m_afValues[ 1 ] ) );
		rVec.w = 1.0f;
	}
}

// $deBlob: FUNCTION 0071d7d0
void __stdcall TParticleInitV4::InitDirTowardsCenterRange( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	TParticleV4Buffer& rPositions = TParticleInit::sm_pCurParticles->m_avPosition;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVec = ( *a_pBuffer )[ i ];
		rVec.Negate3( rPositions[ i ] );
		rVec.Normalise();
		InitV4FloatData* pData = GetFloatData( a_pData );
		rVec.Multiply( s_oRandom.GetFloatMinMax( pData->m_afValues[ 0 ], pData->m_afValues[ 1 ] ) );
		rVec.w = 1.0f;
	}
}

// $deBlob: FUNCTION 0071d8e0
void __stdcall TParticleInitV4::InitDirAwayFromCenterRange( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	TParticleV4Buffer& rPositions = TParticleInit::sm_pCurParticles->m_avPosition;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVec = ( *a_pBuffer )[ i ];
		rVec           = rPositions[ i ];
		rVec.Normalise();
		InitV4FloatData* pData = GetFloatData( a_pData );
		rVec.Multiply( s_oRandom.GetFloatMinMax( pData->m_afValues[ 0 ], pData->m_afValues[ 1 ] ) );
		rVec.w = 1.0f;
	}
}

// $deBlob: FUNCTION 0071d9c0
void __stdcall TParticleInitV4::InitSphereArea( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVec = ( *a_pBuffer )[ i ];
		rVec.x         = s_oRandom.GetFloat2();
		rVec.y         = s_oRandom.GetFloat2();
		rVec.z         = s_oRandom.GetFloat2();
		rVec.Normalise();
		rVec.Multiply( s_oRandom.GetFloat() * GetFloatData( a_pData )->m_afValues[ 0 ] );
		rVec.w = 1.0f;
	}
}

// $deBlob: FUNCTION 0071db70
void __stdcall TParticleInitV4::InitCubeSurface( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	InitV4FloatData* pData = GetFloatData( a_pData );

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVec  = ( *a_pBuffer )[ i ];
		TINT      iFace = s_oRandom.GetInt( CUBEFACE_NUMOF );

		if ( iFace == CUBEFACE_PLUSZ )
		{
			rVec.x = s_oRandom.GetFloat2() * pData->m_afValues[ 0 ];
			rVec.y = s_oRandom.GetFloat2() * pData->m_afValues[ 1 ];
			rVec.z = pData->m_afValues[ 2 ];
		}
		else if ( iFace == CUBEFACE_MINUSZ )
		{
			rVec.x = s_oRandom.GetFloat2() * pData->m_afValues[ 0 ];
			rVec.y = s_oRandom.GetFloat2() * pData->m_afValues[ 1 ];
			rVec.z = -pData->m_afValues[ 2 ];
		}
		else if ( iFace == CUBEFACE_PLUSY )
		{
			rVec.x = s_oRandom.GetFloat2() * pData->m_afValues[ 0 ];
			rVec.y = pData->m_afValues[ 1 ];
			rVec.z = s_oRandom.GetFloat2() * pData->m_afValues[ 2 ];
		}
		else if ( iFace == CUBEFACE_MINUSY )
		{
			rVec.x = s_oRandom.GetFloat2() * pData->m_afValues[ 0 ];
			rVec.y = -pData->m_afValues[ 1 ];
			rVec.z = s_oRandom.GetFloat2() * pData->m_afValues[ 2 ];
		}
		else if ( iFace == CUBEFACE_PLUSX )
		{
			rVec.x = pData->m_afValues[ 0 ];
			rVec.y = s_oRandom.GetFloat2() * pData->m_afValues[ 1 ];
			rVec.z = s_oRandom.GetFloat2() * pData->m_afValues[ 2 ];
		}
		else if ( iFace == CUBEFACE_MINUSX )
		{
			// NOTE: retail picks the -X face's points from 0 to 1 instead of -1 to 1 (the other faces use GetFloat2)
			rVec.x = -pData->m_afValues[ 0 ];
			rVec.y = s_oRandom.GetFloat() * pData->m_afValues[ 1 ];
			rVec.z = s_oRandom.GetFloat() * pData->m_afValues[ 2 ];
		}

		rVec.w = 1.0f;
	}
}

// $deBlob: FUNCTION 0071def0
void __stdcall TParticleInitV4::InitCircleSurface( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	TFLOAT fRadius = GetFloatData( a_pData )->m_afValues[ 0 ];

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVec = ( *a_pBuffer )[ i ];
		rVec.x         = fRadius;
		rVec.y         = 0.0f;
		rVec.z         = 0.0f;
		rVec.w         = 1.0f;
		rVec.RotateY( s_oRandom.GetFloat2() * TMath::PI );
		rVec.w = 1.0f;
	}
}

// $deBlob: FUNCTION 0071dfc0
void __stdcall TParticleInitV4::InitCircleArea( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	TFLOAT fRadius = GetFloatData( a_pData )->m_afValues[ 0 ];

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVec = ( *a_pBuffer )[ i ];
		rVec.x         = fRadius;
		rVec.y         = 0.0f;
		rVec.z         = 0.0f;
		rVec.w         = 1.0f;
		rVec.x         = s_oRandom.GetFloat() * GetFloatData( a_pData )->m_afValues[ 0 ];
		rVec.RotateY( s_oRandom.GetFloat2() * TMath::PI );
		rVec.w = 1.0f;
	}
}

// $deBlob: FUNCTION 0071e0e0
void __stdcall TParticleInitV4::InitConeArea( TParticleV4Buffer* a_pBuffer, TINT a_iStart, TINT a_iEnd, TParticleInit::TRBHeader* a_pData )
{
	InitV4FloatData* pData   = GetFloatData( a_pData );
	TFLOAT           fRadius = pData->m_afValues[ 0 ];
	TFLOAT           fHeight = pData->m_afValues[ 1 ];

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVec  = ( *a_pBuffer )[ i ];
		TFLOAT    fRand = s_oRandom.GetFloat();
		rVec.y          = 0.0f;
		rVec.z          = 0.0f;
		rVec.w          = 1.0f;
		rVec.x          = fRand * fRadius;
		rVec.RotateY( s_oRandom.GetFloat2() * TMath::PI );
		rVec.w = 1.0f;

		TFLOAT fInv = 1.0f - fRand;
		rVec.y      = ( 1.0f - fInv * fInv ) * fHeight + rVec.y;
	}
}

TOSHI_NAMESPACE_END
