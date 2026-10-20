#include "ToshiPCH.h"
#include "TParticleInit.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TGenericParticles* TParticleInit::sm_pCurParticles = TNULL;

// $deBlob: FUNCTION 0071c400
TParticleInit::TParticleInit( const TParticleInit& a_rcOrig )
{
	m_fnInit = a_rcOrig.m_fnInit;
	m_pData  = TSTATICCAST( TRBHeader, TMalloc( a_rcOrig.m_pData->m_uiSize ) );
	TUtil::MemCopy( m_pData, a_rcOrig.m_pData, a_rcOrig.m_pData->m_uiSize );
}

// $deBlob: FUNCTION 0071c440
// $deBlob: FUNCTION 0071af90
TParticleInit::~TParticleInit()
{
	TFree( m_pData );
}

// $deBlob: FUNCTION 0071c450
void TParticleInit::Create()
{
	switch ( m_pData->m_eType )
	{
		case TYPEV4_FIXED:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitV4::InitFixed );
			return;
		case TYPEV4_MINMAX:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitV4::InitMinMax );
			return;
		case TYPEV4_RANDDIR:
		case TYPEV4_RANDDIR2:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitV4::InitRandDir );
			return;
		case TYPEV4_DIRTOWARDSCENTER:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitV4::InitDirTowardsCenter );
			return;
		case TYPEV4_DIRAWAYFROMCENTER:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitV4::InitDirAwayFromCenter );
			return;
		case TYPEV4_DIRINCONE2:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitV4::InitDirInCone2 );
			return;
		case TYPEV4_DIRINCONE:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitV4::InitDirInCone );
			return;
		case TYPEV4_SPHEREAREA:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitV4::InitSphereArea );
			return;
		case TYPEV4_CUBESURFACE:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitV4::InitCubeSurface );
			return;
		case TYPEV4_CIRCLEAREA:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitV4::InitCircleArea );
			return;
		case TYPEV4_CIRCLESURFACE:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitV4::InitCircleSurface );
			return;
		case TYPEV4_CONEAREA:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitV4::InitConeArea );
			return;
		case TYPEV4_RANDDIRRANGE:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitV4::InitRandDirRange );
			return;
		case TYPEV4_DIRTOWARDSCENTERRANGE:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitV4::InitDirTowardsCenterRange );
			return;
		case TYPEV4_DIRAWAYFROMCENTERRANGE:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitV4::InitDirAwayFromCenterRange );
			return;
		case TYPEFLOAT_FIXED:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitFloat::InitFixed );
			return;
		case TYPEFLOAT_MINMAX:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitFloat::InitMinMax );
			return;
		case TYPEFLOAT_FROMSIZE:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitFloat::InitFromSize );
			return;
		case TYPEINT_FIXED:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitInt32::InitFixed );
			return;
		case TYPEINT_MINMAX:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitInt32::InitMinMax );
			return;
		case TYPECOLOUR_FIXED:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitColour::InitFixed );
			GetData<TParticleInitColour::FixedData>()->ConvertColourToSKUColour();
			return;
		case TYPECOLOUR_MINMAX:
			m_fnInit = TREINTERPRETCAST( t_InitFn, TParticleInitColour::InitMinMax );
			GetData<TParticleInitColour::MinMaxData>()->ConvertColourToSKUColour();
			return;
	}

	TASSERT( !"unknown init type!" );
}

TOSHI_NAMESPACE_END
