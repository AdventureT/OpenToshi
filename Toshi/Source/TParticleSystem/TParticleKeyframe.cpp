#include "ToshiPCH.h"
#include "TParticleKeyframe.h"
#include "TGenericParticles.h"
#include "TGenericParticlesSettings.h"
#include "TParticleSystem.h"
#include "TParticleEffect.h"
#include "TParticleEffectInstance.h"
#include "TParticleEffectManager.h"
#include "Math/TMath.h"
#include "Math/TMathInline.h"
#include "Math/TRandom.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// The default callbacks of a keyframe
void __stdcall Keyframe_DefaultStart( TParticleKeyframe* a_pKey, TParticleKeyframe* a_pPrevKey )
{
	// DOES_NOTHING
}

void __stdcall Keyframe_DefaultUpdate( TParticleKeyframe* a_pKey, TFLOAT a_fDeltaTime, TFLOAT a_fT, TParticleKeyframeState* a_pState )
{
	// DOES_NOTHING
}

void __stdcall Keyframe_DefaultUpdateParticle( TParticleKeyframe* a_pKey, TGenericParticles* a_pParticles, TFLOAT a_fT, TINT a_iParticle )
{
	// DOES_NOTHING
}

// The start function of most keyframe types: remembers the key before
// $deBlob: FUNCTION 0071bfb0
// $deBlob: FUNCTION 0071c1a0
// $deBlob: FUNCTION 0071c290
void __stdcall Keyframe_SetPrevKey( TParticleKeyframe* a_pKey, TParticleKeyframe* a_pPrevKey )
{
	a_pKey->m_pPrev = a_pPrevKey;
}

// $deBlob: FUNCTION 0071c7c0
TParticleKeyframe::TParticleKeyframe( const TParticleKeyframe& a_rcOrig )
{
	// NOTE: retail's TRBHeader is 0x14 bytes, the runtime pointers after it belong to the keyframe data
	TASSERT( a_rcOrig.m_pData->m_uiSize >= 0x14 );

	m_pData = TSTATICCAST( TRBHeader, TMalloc( a_rcOrig.m_pData->m_uiSize ) );
	TUtil::MemCopy( m_pData, a_rcOrig.m_pData, a_rcOrig.m_pData->m_uiSize );
	m_pPrev    = a_rcOrig.m_pPrev;
	m_fnStart          = a_rcOrig.m_fnStart;
	m_fnUpdate         = a_rcOrig.m_fnUpdate;
	m_fnUpdateParticle = a_rcOrig.m_fnUpdateParticle;
}

// $deBlob: FUNCTION 0071c840
// $deBlob: FUNCTION 0071bbd0
TParticleKeyframe::~TParticleKeyframe()
{
	TFree( m_pData );
}

// $deBlob: FUNCTION 0071c850
void TParticleKeyframe::Link()
{
	m_fnStart          = Keyframe_DefaultStart;
	m_fnUpdate         = Keyframe_DefaultUpdate;
	m_fnUpdateParticle = Keyframe_DefaultUpdateParticle;
	m_pPrev            = TNULL;

	switch ( m_pData->m_eType )
	{
		case KEYFRAMETYPE_COL:
			TParticleKeyframeColour::Link( this );
			return;
		case KEYFRAMETYPE_ALPHA:
			TParticleKeyframeColour::LinkAlpha( this );
			return;
		case KEYFRAMETYPE_MASS:
			TParticleKeyframeMass::Link( this );
			return;
		case KEYFRAMETYPE_SIZE:
		case KEYFRAMETYPE_SIZEMULT:
			TParticleKeyframeSize::Link( this );
			return;
		case KEYFRAMETYPE_UV:
			TParticleKeyframeUV::Link( this );
			return;
		case KEYFRAMETYPE_PARTICLESYSTEM:
			TKeyframeParticles::Link( this );
			return;
		case KEYFRAMETYPE_REVERSEVELDIRS:
			TParticleKeyframeEventReverseVelDirs::Link( this );
			return;
		case KEYFRAMETYPE_RANDOMISEVELDIRS:
			TParticleKeyframeEventRandomiseVelDirs::Link( this );
			return;
	}

	TASSERT( !"Unknown key type!\n" );
}

TOSHI_NAMESPACE_END
