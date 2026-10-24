#include "pch.h"
#include "APhysicsSoundManager.h"
#include "ASoundInterface.h"
#include "Collision/ACollisionCommon.h"
#include "Collision/ACollisionInstance2.h"

#include <Physics/Havok/TPhysicsObjectContainer.h>
#include <Toshi/T2String8.h>
#include <fmod_event.hpp>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 0060c5a0
APhysicsSoundManager::APhysicsSoundManager()
    : m_pList( TNULL )
{
	m_pEmitters = new PhysicsSoundEmitter[ MAX_EMITTERS ];
	m_pImpacts  = new ImpactSound[ MAX_IMPACTS ];
	m_pSlides   = new SlideSound[ MAX_SLIDES ];

	for ( TINT i = 0; i < MAX_EMITTERS; i++ )
		m_oFreeEmitters.PushFront( &m_pEmitters[ i ] );

	for ( TINT i = 0; i < MAX_IMPACTS; i++ )
		m_oFreeImpacts.PushFront( &m_pImpacts[ i ] );

	for ( TINT i = 0; i < MAX_SLIDES; i++ )
		m_oFreeSlides.PushFront( &m_pSlides[ i ] );

	Load( "PhysicsSoundsData.trb" );
}

// $deBlob: FUNCTION 0060c790
// $deBlob: FUNCTION 0056c1c0
APhysicsSoundManager::~APhysicsSoundManager()
{
	Unload();
}

// $deBlob: FUNCTION 0060c860
void APhysicsSoundManager::PhysicsSoundSurface::SetDefaults()
{
	m_uiSurfaceMask     = 0;
	m_fSlideFadeInTime  = 0.25f;
	m_fSlideFadeOutTime = 0.25f;
	m_fImpactThreshold  = 0.5f;
	m_fSlideThreshold   = 0.1f;
	m_fImpactDelay      = 0.4f;
	m_fUnk1C            = 1.0f;
	m_fUnk20            = 1.0f;
	m_fUnk24            = 0.25f;
	m_fMinImpactSpeed   = 1.0f;
	m_fMaxImpactSpeed   = 10.0f;
	m_fUnk30            = 0.5f;
	m_fUnk34            = 3.0f;
	m_uiFlags           = 0;
	m_iImpactEvent      = -1;
	m_iSlideEvent       = -1;
	m_iSlideGroup       = -1;
	m_iImpactGroup      = -1;
}

// $deBlob: FUNCTION 0060c8e0
const TCHAR* APhysicsSoundManager::PhysicsSoundSurface::GetSlideEventName() const
{
	return APhysicsSoundManager::GetSingleton()->GetName( m_iSlideEvent );
}

// $deBlob: FUNCTION 0060c910
const TCHAR* APhysicsSoundManager::PhysicsSoundSurface::GetImpactEventName() const
{
	return APhysicsSoundManager::GetSingleton()->GetName( m_iImpactEvent );
}

// $deBlob: FUNCTION 0060c940
const TCHAR* APhysicsSoundManager::PhysicsSoundSurface::GetSlideGroupName() const
{
	return APhysicsSoundManager::GetSingleton()->GetName( m_iSlideGroup );
}

// $deBlob: FUNCTION 0060c970
const TCHAR* APhysicsSoundManager::PhysicsSoundSurface::GetImpactGroupName() const
{
	return APhysicsSoundManager::GetSingleton()->GetName( m_iImpactGroup );
}

// $deBlob: FUNCTION 0060c9a0
APhysicsSoundManager::ImpactSound::ImpactSound()
{
	m_pEvent = TNULL;
}

// $deBlob: FUNCTION 0060c9b0
APhysicsSoundManager::SlideSound::SlideSound()
{
	m_pSurface  = TNULL;
	m_pEvent    = TNULL;
	m_fFadeTime = 0.0f;
	m_fTimeLeft = 0.0f;
	m_uiState   = 0;
}

// $deBlob: FUNCTION 0060c9e0
void APhysicsSoundManager::SlideSound::Reset()
{
	m_pSurface  = TNULL;
	m_pEvent    = TNULL;
	m_fFadeTime = 0.0f;
	m_fTimeLeft = 0.0f;
	m_uiState   = 0;
}

// $deBlob: FUNCTION 0060ca10
APhysicsSoundManager::PhysicsSoundEmitter::PhysicsSoundEmitter()
{
	// NOTE: m_PhysicsHandle and m_fImpactDelay are left uninitialised in retail
	m_pOwner = TNULL;
}

// $deBlob: FUNCTION 0060ca80
TBOOL APhysicsSoundManager::PhysicsSoundEmitter::PlayImpact( PhysicsSoundSurface* a_pSurface, TPhysicsCollisionBody* a_pBody, TPhysicsCollisionInfo* a_pInfo, T2DList<ImpactSound>* a_pFreeImpacts )
{
	if ( !TSound::GetSingleton()->IsInitialised() || a_pFreeImpacts->Size() == 0 )
		return TFALSE;

	FMOD::EventSystem* pEventSystem = TSound::GetSingleton()->GetEventSystem();
	TASSERT( TNULL != pEventSystem );

	FMOD::EventGroup*   pGroup   = TNULL;
	FMOD::EventProject* pProject = TNULL;
	pEventSystem->getProjectByIndex( 0, &pProject );

	const TCHAR* szGroup = a_pSurface->GetImpactGroupName();

	if ( szGroup == TNULL )
		return TFALSE;

	if ( pProject->getGroup( szGroup, false, &pGroup ) != FMOD_OK )
		return TFALSE;

	TFLOAT fVolume = ( a_pInfo->m_fRelativeSpeed - a_pSurface->m_fMinImpactSpeed ) / ( a_pSurface->m_fMaxImpactSpeed - a_pSurface->m_fMinImpactSpeed );

	if ( fVolume < 0.0f )
		return TFALSE;

	if ( a_pSurface->m_iImpactEvent == -1 )
		return TFALSE;

	FMOD::Event* pEvent;

	if ( pGroup->getEvent( a_pSurface->GetImpactEventName(), FMOD_EVENT_DEFAULT, &pEvent ) != FMOD_OK || pEvent->start() != FMOD_OK )
		return TFALSE;

	pEvent->setVolume( fVolume );
	ASoundInterface::SetPosition( pEvent, a_pInfo->m_vPosition );

	m_fImpactDelay = a_pSurface->m_fImpactDelay;

	ImpactSound* pImpact = a_pFreeImpacts->PopFront();
	pImpact->m_pEvent    = pEvent;

	TINT  iType;
	void* pOwner;
	GetOwner( a_pBody, &iType, &pOwner );

	// NOTE: a recycled emitter keeps the owner it had (FreeEmitter doesn't clear it)
	if ( m_pOwner == TNULL )
		m_pOwner = pOwner;

	m_PhysicsHandle = a_pBody->m_pBody;
	m_oImpacts.PushBack( pImpact );

	return TTRUE;
}

// $deBlob: FUNCTION 0060cc50
TBOOL APhysicsSoundManager::PhysicsSoundEmitter::PlaySlide( PhysicsSoundSurface* a_pSurface, TPhysicsCollisionBody* a_pBody, TFLOAT a_fRelativeSpeed, const TVector4* a_pPosition, T2DList<SlideSound>* a_pFreeSlides )
{
	if ( !TSound::GetSingleton()->IsInitialised() )
		return TFALSE;

	if ( a_pFreeSlides->Size() == 0 )
		return TFALSE;

	FMOD::EventSystem* pEventSystem = TSound::GetSingleton()->GetEventSystem();
	TASSERT( TNULL != pEventSystem );

	FMOD::EventGroup*   pGroup   = TNULL;
	FMOD::EventProject* pProject = TNULL;
	pEventSystem->getProjectByIndex( 0, &pProject );

	// NOTE: unlike PlayImpact the result isn't checked, a missing group crashes on getEvent
	pProject->getGroup( a_pSurface->GetSlideGroupName(), false, &pGroup );

	FMOD::Event* pEvent;

	if ( pGroup->getEvent( a_pSurface->GetSlideEventName(), FMOD_EVENT_DEFAULT, &pEvent ) != FMOD_OK || pEvent->start() != FMOD_OK )
		return TFALSE;

	TINT  iType;
	void* pOwner;
	GetOwner( a_pBody, &iType, &pOwner );
	m_pOwner = pOwner;

	pEvent->setVolume( 0.0f );

	SlideSound* pSlide  = a_pFreeSlides->PopFront();
	pSlide->m_pSurface  = TNULL;
	pSlide->m_pEvent    = TNULL;
	pSlide->m_fTimeLeft = 0.0f;
	pSlide->m_uiState   = SlideSound::STATE_FADEIN;
	pSlide->m_fFadeTime = 0.0f;
	pSlide->m_fTimeLeft = a_pSurface->m_fSlideFadeInTime;
	pSlide->m_pEvent    = pEvent;
	pSlide->m_pSurface  = a_pSurface;
	m_oSlides.PushFront( pSlide );

	ASoundInterface::SetPosition( pEvent, *a_pPosition );

	return TTRUE;
}

// $deBlob: FUNCTION 0060ce00
TBOOL APhysicsSoundManager::RefreshSlides( PhysicsSoundEmitter* a_pEmitter )
{
	if ( a_pEmitter == TNULL )
		return TFALSE;

	for ( auto it = a_pEmitter->m_oSlides.Begin(); it != a_pEmitter->m_oSlides.End(); it++ )
		it->m_fTimeLeft = it->m_pSurface->m_fSlideFadeInTime;

	return TTRUE;
}

// $deBlob: FUNCTION 0060ce90
TINT APhysicsSoundManager::PhysicsSoundEmitter::GetNumActiveSounds()
{
	return m_oSlides.Size() + m_oImpacts.Size();
}

// $deBlob: FUNCTION 0060ceb0
TBOOL APhysicsSoundManager::PhysicsSoundEmitter::StopImpact( ImpactSound* a_pImpact, T2DList<ImpactSound>* a_pFreeImpacts )
{
	auto it = m_oImpacts.Begin();

	if ( it == m_oImpacts.End() )
		return TFALSE;

	while ( a_pImpact != it )
	{
		it++;

		if ( it == m_oImpacts.End() )
			return TFALSE;
	}

	if ( it->m_pEvent != TNULL )
	{
		it->m_pEvent->stop( true );
		it->m_pEvent = TNULL;
	}

	it->Remove();
	a_pFreeImpacts->PushBack( a_pImpact );

	return TTRUE;
}

// $deBlob: FUNCTION 0060cf90
TBOOL APhysicsSoundManager::PhysicsSoundEmitter::StopSlide( SlideSound* a_pSlide, T2DList<SlideSound>* a_pFreeSlides )
{
	auto it = m_oSlides.Begin();

	if ( it == m_oSlides.End() )
		return TFALSE;

	while ( a_pSlide != it )
	{
		it++;

		if ( it == m_oSlides.End() )
			return TFALSE;
	}

	if ( a_pSlide->m_pEvent != TNULL )
	{
		a_pSlide->m_pEvent->stop( true );
		a_pSlide->m_pSurface  = TNULL;
		a_pSlide->m_pEvent    = TNULL;
		a_pSlide->m_fFadeTime = 0.0f;
		a_pSlide->m_fTimeLeft = 0.0f;
		a_pSlide->m_uiState   = 0;
	}

	a_pSlide->Remove();
	a_pFreeSlides->PushBack( a_pSlide );

	return TTRUE;
}

// $deBlob: FUNCTION 0060d090
void APhysicsSoundManager::PhysicsSoundEmitter::Update( TFLOAT a_fDeltaTime, T2DList<ImpactSound>* a_pFreeImpacts, T2DList<SlideSound>* a_pFreeSlides )
{
	m_fImpactDelay -= a_fDeltaTime;

	for ( auto it = m_oImpacts.Begin(); it != m_oImpacts.End(); )
	{
		ImpactSound* pImpact = it;
		TBOOL        bDone   = TFALSE;

		if ( pImpact->m_pEvent != TNULL )
		{
			FMOD_EVENT_STATE uiState;

			if ( pImpact->m_pEvent->getState( &uiState ) != FMOD_OK )
				bDone = TTRUE;
			else if ( !( uiState & FMOD_EVENT_STATE_PLAYING ) )
				bDone = TTRUE;
		}
		else
		{
			bDone = TTRUE;
		}

		it++;

		if ( bDone )
		{
			pImpact->Remove();
			a_pFreeImpacts->PushFront( pImpact );
		}
	}

	for ( auto it = m_oSlides.Begin(); it != m_oSlides.End(); )
	{
		SlideSound* pSlide = it;
		TBOOL       bDone  = TTRUE;

		if ( pSlide->m_pEvent != TNULL )
		{
			// NOTE: retail doesn't check getState's result and reads a stale local when it fails
			FMOD_EVENT_STATE uiState = 0;
			pSlide->m_pEvent->getState( &uiState );

			pSlide->m_fTimeLeft -= a_fDeltaTime;

			if ( uiState & FMOD_EVENT_STATE_PLAYING )
			{
				TUINT32 uiSlideState = pSlide->m_uiState;

				if ( uiSlideState & SlideSound::STATE_FADEIN )
				{
					bDone = TFALSE;
					pSlide->m_fFadeTime += a_fDeltaTime;

					if ( pSlide->m_fFadeTime <= pSlide->m_pSurface->m_fSlideFadeInTime )
					{
						pSlide->m_pEvent->setVolume( pSlide->m_fFadeTime / pSlide->m_pSurface->m_fSlideFadeInTime );
					}
					else
					{
						pSlide->m_uiState = ( uiSlideState & ~( SlideSound::STATE_FADEIN | SlideSound::STATE_PLAYING ) ) | SlideSound::STATE_PLAYING;
						pSlide->m_pEvent->setVolume( 1.0f );
					}
				}
				else if ( uiSlideState & SlideSound::STATE_PLAYING )
				{
					bDone = TFALSE;
					pSlide->m_pEvent->setVolume( 1.0f );

					if ( pSlide->m_fTimeLeft < 0.0f )
					{
						pSlide->m_uiState   = ( pSlide->m_uiState & ~( SlideSound::STATE_PLAYING | SlideSound::STATE_FADEOUT ) ) | SlideSound::STATE_FADEOUT;
						pSlide->m_fFadeTime = pSlide->m_pSurface->m_fSlideFadeOutTime;
					}
				}
				else if ( uiSlideState & SlideSound::STATE_FADEOUT )
				{
					pSlide->m_fFadeTime -= a_fDeltaTime;

					if ( pSlide->m_fFadeTime >= 0.0f )
					{
						bDone = TFALSE;
						pSlide->m_pEvent->setVolume( pSlide->m_fFadeTime / pSlide->m_pSurface->m_fSlideFadeOutTime );

						if ( pSlide->m_fTimeLeft > 0.0f )
							pSlide->m_uiState = ( pSlide->m_uiState & ~( SlideSound::STATE_FADEIN | SlideSound::STATE_FADEOUT ) ) | SlideSound::STATE_FADEIN;
					}
					else
					{
						// NOTE: the event isn't stopped, only silenced
						pSlide->m_pEvent->setVolume( 0.0f );
					}
				}
			}
		}

		it++;

		if ( bDone )
		{
			pSlide->Remove();
			a_pFreeSlides->PushFront( pSlide );
		}
	}
}

// $deBlob: FUNCTION 0060d330
void APhysicsSoundManager::Load( const TCHAR* a_szFileName )
{
	TCHAR szPath[ 128 ];
	T2String8::Format( szPath, sizeof( szPath ), "Data/Sound/PhysicsSoundsData/%s", a_szFileName );

	if ( m_oTRB.Load( szPath ) == TTRB::ERROR_OK )
	{
		m_pList = TSTATICCAST( PhysicsSoundsList, m_oTRB.GetSymbolAddress( "PhysicsSoundsList" ) );

		for ( TINT i = 0; i < m_pList->m_iNumSurfaces; i++ )
			m_pList->m_pSurfaces[ i ].m_uiSurfaceMask = ACollisionCommon::GetSurfaceType( m_pList->m_pSurfaces[ i ].m_szName );
	}
	else
	{
		m_oTRB.Close();
	}
}

// $deBlob: FUNCTION 0060d3f0
void APhysicsSoundManager::Unload()
{
	m_oTRB.Close();
	m_pList = TNULL;
	Destroy();
}

// $deBlob: FUNCTION 0060d410
void APhysicsSoundManager::Destroy()
{
	for ( auto it = m_oFreeImpacts.Begin(); it != m_oFreeImpacts.End(); )
	{
		auto itNext = it;
		itNext++;
		it->Remove();
		it = itNext;
	}

	for ( auto it = m_oFreeSlides.Begin(); it != m_oFreeSlides.End(); )
	{
		auto itNext = it;
		itNext++;
		it->Remove();
		it = itNext;
	}

	for ( auto it = m_oFreeEmitters.Begin(); it != m_oFreeEmitters.End(); )
	{
		auto itNext = it;
		itNext++;
		it->Remove();
		it = itNext;
	}

	// NOTE: the pointers aren't cleared
	if ( m_pEmitters != TNULL )
		delete[] m_pEmitters;

	if ( m_pImpacts != TNULL )
		delete[] m_pImpacts;

	if ( m_pSlides != TNULL )
		delete[] m_pSlides;
}

// $deBlob: FUNCTION 0060d670
void APhysicsSoundManager::Update( TFLOAT a_fDeltaTime )
{
	for ( auto it = m_oEmitters.Begin(); it != m_oEmitters.End(); )
	{
		PhysicsSoundEmitter* pEmitter = it;
		pEmitter->Update( a_fDeltaTime, &m_oFreeImpacts, &m_oFreeSlides );

		it++;

		if ( pEmitter->m_oSlides.Size() == 0 && pEmitter->m_oImpacts.Size() == 0 )
			FreeEmitter( pEmitter );
	}
}

// $deBlob: FUNCTION 0060d7d0
TINT APhysicsSoundManager::GetNumFreeSounds()
{
	TINT iNumFree = MAX_IMPACTS + MAX_SLIDES;

	for ( auto it = m_oEmitters.Begin(); it != m_oEmitters.End(); )
	{
		iNumFree -= it->m_oSlides.Size() + it->m_oImpacts.Size();

#ifdef DEBLOB_COMMUNITY_PATCH
		// NOTE: retail never advances the iterator (an endless loop once an emitter is active)
		it++;
#endif
	}

	return iNumFree;
}

// $deBlob: FUNCTION 0060d830
void APhysicsSoundManager::OnCollision( TPhysicsCollisionInfo* a_pInfo, TINT a_iUnused )
{
	// The faster body is the primary one
	TPhysicsCollisionBody* pPrimary   = &a_pInfo->m_self;
	TPhysicsCollisionBody* pSecondary = &a_pInfo->m_other;

	TFLOAT fSelfSpeedSq  = a_pInfo->m_self.m_vVelocity.MagnitudeSq();
	TFLOAT fOtherSpeedSq = a_pInfo->m_other.m_vVelocity.MagnitudeSq();

	if ( fOtherSpeedSq > fSelfSpeedSq )
	{
		pPrimary   = &a_pInfo->m_other;
		pSecondary = &a_pInfo->m_self;
	}

	PhysicsSoundEmitter* pEmitter          = TNULL;
	PhysicsSoundSurface* pPrimarySurface   = TNULL;
	PhysicsSoundSurface* pSecondarySurface = TNULL;

	COLLISION eCollision = GetCollisionType( pPrimary, pSecondary, a_pInfo, &pEmitter, &pPrimarySurface, &pSecondarySurface );

	if ( eCollision == COLLISION_None )
		return;

	if ( eCollision == COLLISION_NewSlide || eCollision == COLLISION_NewImpact )
		pEmitter = AllocEmitter();

	if ( pEmitter == TNULL || pPrimarySurface == TNULL )
		return;

	if ( eCollision == COLLISION_NewImpact || eCollision == COLLISION_Impact )
		pEmitter->PlayImpact( ( pSecondarySurface->m_uiFlags & PhysicsSoundSurface::FLAGS_OWNIMPACT ) ? pSecondarySurface : pPrimarySurface, pPrimary, a_pInfo, &m_oFreeImpacts );

	if ( eCollision == COLLISION_NewSlide || eCollision == COLLISION_Slide )
		pEmitter->PlaySlide( pSecondarySurface, pPrimary, a_pInfo->m_fRelativeSpeed, &a_pInfo->m_vPosition, &m_oFreeSlides );

	if ( eCollision == COLLISION_KeepSliding )
		RefreshSlides( pEmitter );
}

// $deBlob: FUNCTION 0060d940
void APhysicsSoundManager::StopAll()
{
	for ( auto it = m_oEmitters.Begin(); it != m_oEmitters.End(); it++ )
	{
		PhysicsSoundEmitter* pEmitter = it;

		for ( auto itImpact = pEmitter->m_oImpacts.Begin(); itImpact != pEmitter->m_oImpacts.End(); )
		{
			ImpactSound* pImpact = itImpact;
			itImpact++;
			pEmitter->StopImpact( pImpact, &m_oFreeImpacts );
		}

		for ( auto itSlide = pEmitter->m_oSlides.Begin(); itSlide != pEmitter->m_oSlides.End(); )
		{
			SlideSound* pSlide = itSlide;
			itSlide++;
			pEmitter->StopSlide( pSlide, &m_oFreeSlides );
		}
	}

	Update( 0.0f );
}

// $deBlob: FUNCTION 0060db50
const TCHAR* APhysicsSoundManager::GetName( TINT16 a_iIndex )
{
	if ( m_pList != TNULL && m_pList->m_iNumNames > a_iIndex && a_iIndex >= 0 )
		return m_pList->m_ppNames[ a_iIndex ];

	return TNULL;
}

// $deBlob: FUNCTION 0060db80
APhysicsSoundManager::PhysicsSoundSurface* APhysicsSoundManager::FindSurface( TUINT32 a_uiSurfaceMask )
{
	for ( TINT i = 0; i < m_pList->m_iNumSurfaces; i++ )
	{
		if ( m_pList->m_pSurfaces[ i ].m_uiSurfaceMask & a_uiSurfaceMask )
			return &m_pList->m_pSurfaces[ i ];
	}

	return m_pList->m_pSurfaces;
}

// $deBlob: FUNCTION 0060dbd0
APhysicsSoundManager::COLLISION APhysicsSoundManager::GetCollisionType( TPhysicsCollisionBody* a_pPrimary, TPhysicsCollisionBody* a_pSecondary, TPhysicsCollisionInfo* a_pInfo, PhysicsSoundEmitter** a_ppEmitter, PhysicsSoundSurface** a_ppPrimarySoundSurface, PhysicsSoundSurface** a_ppSecondarySoundSurface )
{
	ACollisionInstance2::OwnerInfo* pOwnerInfo = TSTATICCAST( ACollisionInstance2::OwnerInfo, a_pPrimary->m_pBody != TNULL ? a_pPrimary->m_pBody->m_pUserData : TNULL );
	void*                           pOwner     = pOwnerInfo->m_pOwner;

	if ( pOwnerInfo->m_eType != 3 )
		return COLLISION_None;

	TUINT32  uiPrimaryMaterial;
	TUINT32  uiSecondaryMaterial;
	TVector4 vDirection;

	if ( a_pInfo->m_self.m_pBody == a_pPrimary->m_pBody )
	{
		uiPrimaryMaterial   = a_pInfo->m_self.m_uiMaterial;
		uiSecondaryMaterial = a_pInfo->m_other.m_uiMaterial;
		vDirection          = a_pInfo->m_self.m_vVelocity;
	}
	else
	{
		uiPrimaryMaterial   = a_pInfo->m_other.m_uiMaterial;
		uiSecondaryMaterial = a_pInfo->m_self.m_uiMaterial;
		vDirection          = a_pInfo->m_other.m_vVelocity;
	}

	vDirection.Normalise();

	*a_ppSecondarySoundSurface             = FindSurface( uiSecondaryMaterial );
	PhysicsSoundSurface* pPrimarySurface   = FindSurface( uiPrimaryMaterial );
	*a_ppPrimarySoundSurface               = pPrimarySurface;
	PhysicsSoundSurface* pSecondarySurface = *a_ppSecondarySoundSurface;

	TFLOAT fDot = TMath::Abs( ( a_pInfo->m_vNormal.y * vDirection.y + a_pInfo->m_vNormal.x * vDirection.x ) + a_pInfo->m_vNormal.z * vDirection.z );

	if ( pSecondarySurface == TNULL )
		return COLLISION_None;

	TBOOL bSliding = TFALSE;
	TBOOL bImpact  = fDot > pSecondarySurface->m_fImpactThreshold;

	if ( pSecondarySurface->m_fSlideThreshold > fDot )
	{
		TINT  iType;
		void* pSecondaryOwner;
		GetOwner( a_pSecondary, &iType, &pSecondaryOwner );

		// Sliding on a terrain cell
		bSliding = iType == 4;
	}

	if ( bImpact && pSecondarySurface->m_fMinImpactSpeed > a_pInfo->m_fRelativeSpeed && pPrimarySurface->m_fMinImpactSpeed > a_pInfo->m_fRelativeSpeed )
		return COLLISION_None;

	for ( auto it = m_oEmitters.Begin(); it != m_oEmitters.End(); it++ )
	{
		if ( it->m_pOwner == pOwner )
		{
			if ( a_ppEmitter != TNULL )
				*a_ppEmitter = it;

			if ( bImpact )
				return ( it->m_fImpactDelay > 0.0f ) ? COLLISION_None : COLLISION_Impact;

			if ( bSliding )
				return ( it->m_oSlides.Size() != 0 ) ? COLLISION_KeepSliding : COLLISION_Slide;
		}
		else
		{
			// NOTE: retail compares the address of the out parameter with the body, so it never fails
			TASSERT( TREINTERPRETCAST( void*, a_ppPrimarySoundSurface ) != it->m_PhysicsHandle );
		}
	}

	if ( bSliding )
		return COLLISION_NewSlide;

	if ( bImpact )
		return COLLISION_NewImpact;

	return COLLISION_None;
}

// $deBlob: FUNCTION 0060de50
void APhysicsSoundManager::GetOwner( TPhysicsCollisionBody* a_pBody, TINT* a_pType, void** a_ppOwner )
{
	// NOTE: retail reads the owner info even without a body (a null pointer read)
	ACollisionInstance2::OwnerInfo* pOwnerInfo = TSTATICCAST( ACollisionInstance2::OwnerInfo, a_pBody->m_pBody != TNULL ? a_pBody->m_pBody->m_pUserData : TNULL );

	*a_pType   = pOwnerInfo->m_eType;
	*a_ppOwner = pOwnerInfo->m_pOwner;
}

// $deBlob: FUNCTION 0060de90
APhysicsSoundManager::PhysicsSoundEmitter* APhysicsSoundManager::AllocEmitter()
{
	if ( m_oFreeEmitters.Size() == 0 )
		return TNULL;

	PhysicsSoundEmitter* pEmitter = m_oFreeEmitters.PopFront();
	m_oEmitters.PushBack( pEmitter );

	return pEmitter;
}

// $deBlob: FUNCTION 0060df00
void APhysicsSoundManager::FreeEmitter( PhysicsSoundEmitter* a_pPhysicsSoundEmitterToFree )
{
	TASSERT( a_pPhysicsSoundEmitterToFree->GetNumActiveSounds() == 0 );

	a_pPhysicsSoundEmitterToFree->Remove();
	m_oFreeEmitters.PushFront( a_pPhysicsSoundEmitterToFree );
}

// $deBlob: FUNCTION 0060e230
// $deBlob: FUNCTION 0060e610
APhysicsSoundManager::PhysicsSoundEmitter::~PhysicsSoundEmitter()
{
}
