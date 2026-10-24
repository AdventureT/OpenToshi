#include "ToshiPCH.h"
#include "TGenericParticlesSettings.h"
#include "TGenericParticles.h"
#include "TParticleSystem.h"
#include "TParticleEffectManager.h"
#include "TParticleMaterialManager.h"
#include "TParticleInit.h"
#include "TParticleAffector.h"
#include "TParticleKeyframe.h"
#include "Render/TModelManager.h"
#include "Math/TRandom.h"
#include "Math/TMathInline.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// The suffix of the copies' names (0x00982734)
static const TCHAR* s_szCopySuffix = "_copy";

// $deBlob: FUNCTION 0071c790
TGenericParticlesConstant::TGenericParticlesConstant( const TGenericParticlesConstant& a_rcOrig )
{
	m_uiUnk00 = a_rcOrig.m_uiUnk00;
	m_uiStart = a_rcOrig.m_uiStart;
	TUtil::MemCopy( m_auiUnk08, a_rcOrig.m_auiUnk08, sizeof( m_auiUnk08 ) );
	m_uiEnd = a_rcOrig.m_uiEnd;
	TUtil::MemCopy( m_auiUnk18, a_rcOrig.m_auiUnk18, sizeof( m_auiUnk18 ) );
}

// $deBlob: FUNCTION 00719ef0
TGenericParticlesSettings::TGenericParticlesSettings( const TGenericParticlesSettings& a_rcOrig )
{
	TSIZE uiNameSize = TStringManager::String8Length( s_szCopySuffix ) + 1 + TStringManager::String8Length( a_rcOrig.m_szName );
	m_uiVer          = s_LatestVer;
	m_szName         = TSTATICCAST( TCHAR, TMemalign( 16, uiNameSize ) );
	TStringManager::String8Format( m_szName, TINT( uiNameSize ), "%s%s", a_rcOrig.m_szName, s_szCopySuffix );

	m_szMaterial = a_rcOrig.m_szMaterial;

#ifdef DEBLOB_COMMUNITY_PATCH
	// NOTE: retail checks its own render style, which isn't set yet (copied below), instead of the original's
	m_szModel = ( a_rcOrig.m_eRenderStyle == RENDERSTYLE_MESH ) ? a_rcOrig.m_szModel : TNULL;
#else
	m_szModel = ( m_eRenderStyle == RENDERSTYLE_MESH ) ? a_rcOrig.m_szModel : TNULL;
#endif

	m_iNumParticles    = a_rcOrig.m_iNumParticles;
	m_fUnk1C           = a_rcOrig.m_fUnk1C;
	m_fLODDistance     = a_rcOrig.m_fLODDistance;
	m_iMinParticles    = a_rcOrig.m_iMinParticles;
	m_bAveragePosition = a_rcOrig.m_bAveragePosition;
	m_bDynamicRadius   = a_rcOrig.m_bDynamicRadius;
	m_fRadius          = a_rcOrig.m_fRadius;
	m_pMaterial        = a_rcOrig.m_pMaterial;
	m_pModel           = a_rcOrig.m_pModel;
	m_uiFlags          = a_rcOrig.m_uiFlags;
	m_uiBuffers        = a_rcOrig.m_uiBuffers;
	m_eRenderStyle     = a_rcOrig.m_eRenderStyle;

	// The affectors (in the same order)
	m_pAffectors                = TNULL;
	TParticleAffector* pPrev    = TNULL;

	for ( TParticleAffector* pOrig = a_rcOrig.m_pAffectors; pOrig != TNULL; pOrig = pOrig->m_pNext )
	{
		TParticleAffector* pAffector = new TParticleAffector( *pOrig );
		pAffector->m_pNext           = TNULL;

		if ( pPrev == TNULL )
			m_pAffectors = pAffector;
		else
			pPrev->m_pNext = pAffector;

		pPrev = pAffector;
	}

	m_pPositionInit = TNULL;
	m_pVelocityInit = TNULL;
	m_pLifeLostInit = TNULL;
	m_pMassInit     = TNULL;
	m_pSizeInit     = TNULL;
	m_pUVInit       = TNULL;
	m_pColourInit   = TNULL;

	if ( a_rcOrig.m_pPositionInit ) m_pPositionInit = new TParticleInit( *a_rcOrig.m_pPositionInit );
	if ( a_rcOrig.m_pVelocityInit ) m_pVelocityInit = new TParticleInit( *a_rcOrig.m_pVelocityInit );
	if ( a_rcOrig.m_pLifeLostInit ) m_pLifeLostInit = new TParticleInit( *a_rcOrig.m_pLifeLostInit );
	if ( a_rcOrig.m_pMassInit ) m_pMassInit = new TParticleInit( *a_rcOrig.m_pMassInit );
	if ( a_rcOrig.m_pSizeInit ) m_pSizeInit = new TParticleInit( *a_rcOrig.m_pSizeInit );
	if ( a_rcOrig.m_pUVInit ) m_pUVInit = new TParticleInit( *a_rcOrig.m_pUVInit );
	if ( a_rcOrig.m_pColourInit ) m_pColourInit = new TParticleInit( *a_rcOrig.m_pColourInit );

	m_pConstColour = new TGenericParticlesConstant( *a_rcOrig.m_pConstColour );
	m_pConstSize   = new TGenericParticlesConstant( *a_rcOrig.m_pConstSize );

	m_pQuadSettings      = TNULL;
	m_pBillboardSettings = TNULL;
	m_pSparkSettings     = TNULL;
	m_pSparkQuadSettings = TNULL;
	m_pRibbonSettings    = TNULL;

	if ( a_rcOrig.m_pQuadSettings )
	{
		m_pQuadSettings  = TSTATICCAST( TQuadRenderSettings, TMalloc( sizeof( TQuadRenderSettings ) ) );
		*m_pQuadSettings = *a_rcOrig.m_pQuadSettings;
	}

	if ( a_rcOrig.m_pBillboardSettings )
	{
		m_pBillboardSettings  = TSTATICCAST( TBillboardQuadRenderSettings, TMalloc( sizeof( TBillboardQuadRenderSettings ) ) );
		*m_pBillboardSettings = *a_rcOrig.m_pBillboardSettings;
	}

	if ( a_rcOrig.m_pSparkSettings )
	{
		m_pSparkSettings  = TSTATICCAST( TSparkRenderSettings, TMalloc( sizeof( TSparkRenderSettings ) ) );
		*m_pSparkSettings = *a_rcOrig.m_pSparkSettings;
	}

	if ( a_rcOrig.m_pSparkQuadSettings )
	{
		m_pSparkQuadSettings  = TSTATICCAST( TSparkQuadRenderSettings, TMalloc( sizeof( TSparkQuadRenderSettings ) ) );
		*m_pSparkQuadSettings = *a_rcOrig.m_pSparkQuadSettings;
	}

	if ( a_rcOrig.m_pRibbonSettings )
	{
		m_pRibbonSettings  = TSTATICCAST( TRibbonRenderSettings, TMalloc( sizeof( TRibbonRenderSettings ) ) );
		*m_pRibbonSettings = *a_rcOrig.m_pRibbonSettings;
	}

	m_pKeys88     = TNULL;
	m_pColourKeys = TNULL;
	m_pMassKeys   = TNULL;
	m_pKeys94     = TNULL;
	m_pSizeKeys   = TNULL;
	m_pUVKeys     = TNULL;
	m_pEventKeys  = TNULL;

	if ( a_rcOrig.m_pKeys88 ) m_pKeys88 = new TParticleKeyframeTimeline( *a_rcOrig.m_pKeys88 );
	if ( a_rcOrig.m_pColourKeys ) m_pColourKeys = new TParticleKeyframeTimeline( *a_rcOrig.m_pColourKeys );
	if ( a_rcOrig.m_pMassKeys ) m_pMassKeys = new TParticleKeyframeTimeline( *a_rcOrig.m_pMassKeys );
	if ( a_rcOrig.m_pKeys94 ) m_pKeys94 = new TParticleKeyframeTimeline( *a_rcOrig.m_pKeys94 );
	if ( a_rcOrig.m_pSizeKeys ) m_pSizeKeys = new TParticleKeyframeTimeline( *a_rcOrig.m_pSizeKeys );
	if ( a_rcOrig.m_pUVKeys ) m_pUVKeys = new TParticleKeyframeTimeline( *a_rcOrig.m_pUVKeys );
	if ( a_rcOrig.m_pEventKeys ) m_pEventKeys = new TParticleKeyframeTimeline( *a_rcOrig.m_pEventKeys );

	TParticleSystem::GetSingleton()->GetEffectManager()->AddSetting( this );
}

// $deBlob: FUNCTION 0071a430
// $deBlob: FUNCTION 00711a00
TGenericParticlesSettings::~TGenericParticlesSettings()
{
	TParticleSystem::GetSingleton()->GetEffectManager()->RemoveSetting( this );

	while ( m_pAffectors != TNULL )
	{
		TParticleAffector* pAffector = m_pAffectors;
		m_pAffectors                 = pAffector->m_pNext;

		if ( pAffector != TNULL )
			delete pAffector;
	}

	operator delete( m_szName );

	if ( m_pPositionInit ) delete m_pPositionInit;
	if ( m_pVelocityInit ) delete m_pVelocityInit;
	if ( m_pLifeLostInit ) delete m_pLifeLostInit;
	if ( m_pMassInit ) delete m_pMassInit;
	if ( m_pSizeInit ) delete m_pSizeInit;
	if ( m_pUVInit ) delete m_pUVInit;
	if ( m_pColourInit ) delete m_pColourInit;

	operator delete( m_pConstColour );
	operator delete( m_pConstSize );
	operator delete( m_pQuadSettings );
	operator delete( m_pBillboardSettings );
	operator delete( m_pSparkQuadSettings );
	operator delete( m_pSparkSettings );
	operator delete( m_pRibbonSettings );

	if ( m_pKeys88 ) delete m_pKeys88;
	if ( m_pColourKeys ) delete m_pColourKeys;
	if ( m_pMassKeys ) delete m_pMassKeys;
	if ( m_pKeys94 ) delete m_pKeys94;
	if ( m_pSizeKeys ) delete m_pSizeKeys;
	if ( m_pUVKeys ) delete m_pUVKeys;
	if ( m_pEventKeys ) delete m_pEventKeys;
}

// $deBlob: FUNCTION 0071a630
void TGenericParticlesSettings::Create()
{
	TASSERT( m_uiVer == s_LatestVer );

	if ( m_uiFlags & FLAGS_CREATED )
		return;

	if ( m_pPositionInit ) m_pPositionInit->Create();
	if ( m_pVelocityInit ) m_pVelocityInit->Create();
	if ( m_pLifeLostInit ) m_pLifeLostInit->Create();
	if ( m_pMassInit ) m_pMassInit->Create();
	if ( m_pSizeInit ) m_pSizeInit->Create();
	if ( m_pUVInit ) m_pUVInit->Create();
	if ( m_pColourInit ) m_pColourInit->Create();

	for ( TParticleAffector* pAffector = m_pAffectors; pAffector != TNULL; pAffector = pAffector->m_pNext )
		pAffector->Create();

	if ( m_pKeys88 ) m_pKeys88->Link();
	if ( m_pColourKeys ) m_pColourKeys->Link();
	if ( m_pMassKeys ) m_pMassKeys->Link();
	if ( m_pKeys94 ) m_pKeys94->Link();
	if ( m_pSizeKeys ) m_pSizeKeys->Link();
	if ( m_pUVKeys ) m_pUVKeys->Link();
	if ( m_pEventKeys ) m_pEventKeys->Link();

	m_uiBuffers = s_uiDefaultBuffers;

	if ( m_pRibbonSettings != TNULL && m_pRibbonSettings->m_uiForwardDir != 0 )
		m_uiBuffers = s_uiDefaultRibbonBuffers;

	if ( m_uiFlags & FLAGS_ROTATIONS )
	{
		if ( m_pQuadSettings ) m_pQuadSettings->Init();
		if ( m_pBillboardSettings ) m_pBillboardSettings->Init();
	}

	TParticleSystem* pSystem = TParticleSystem::GetSingleton();
	m_pMaterial              = pSystem->GetMaterialManager()->FindMaterial( m_szMaterial );

	if ( m_eRenderStyle == RENDERSTYLE_MESH && m_szModel != TNULL )
	{
		m_pModel = TModelManager::GetSingleton()->FindModel( m_szModel );
		TASSERT( m_pModel.IsValid() && "TNULL != m_pModel" );

		// The meshes use the material manager's own material
		m_pMaterial = pSystem->GetMaterialManager()->GetMeshMaterial();
	}

	m_uiFlags |= FLAGS_CREATED;
}

// $deBlob: FUNCTION 0071a810
void TGenericParticlesSettings::InitParticles( TGenericParticles* a_pParticles, TINT a_iStartParticle, TINT a_iEndParticle, TBOOL a_bInitLife )
{
	TASSERT( a_iStartParticle >= 0 );
	TASSERT( a_iStartParticle <= a_iEndParticle );

	if ( a_iStartParticle == a_iEndParticle )
		return;

	TParticleInit::SetCurParticles( a_pParticles );

	if ( m_pPositionInit && HasBuffer( TGenericParticles::FLAGS_POSITION ) )
		m_pPositionInit->Init( a_pParticles->m_avPosition, a_iStartParticle, a_iEndParticle );

	if ( m_pVelocityInit && HasBuffer( TGenericParticles::FLAGS_VELOCITY ) )
		m_pVelocityInit->Init( a_pParticles->m_avVelocity, a_iStartParticle, a_iEndParticle );

	if ( m_pLifeLostInit && a_bInitLife == TTRUE && HasBuffer( TGenericParticles::FLAGS_LIFE ) )
	{
		m_pLifeLostInit->Init( a_pParticles->m_afLifeLostPerSec, a_iStartParticle, a_iEndParticle );

		for ( TINT i = a_iStartParticle; i < a_iEndParticle; i++ )
			a_pParticles->m_afLife[ i ] = 0.0f;
	}

	if ( m_pSizeInit && HasBuffer( TGenericParticles::FLAGS_SIZE ) )
	{
		m_pSizeInit->Init( a_pParticles->m_afSize, a_iStartParticle, a_iEndParticle );

		if ( m_pSizeKeys != TNULL )
		{
			for ( TINT i = a_iStartParticle; i < a_iEndParticle; i++ )
				a_pParticles->m_apCurSizeKey[ i ] = 0;
		}
	}

	if ( m_pMassInit && HasBuffer( TGenericParticles::FLAGS_MASS ) )
		m_pMassInit->Init( a_pParticles->m_afMass, a_iStartParticle, a_iEndParticle );

	if ( m_pUVInit && HasBuffer( TGenericParticles::FLAGS_UVINDEX ) )
		m_pUVInit->InitInt( a_pParticles->m_aiUVIndex, a_iStartParticle, a_iEndParticle );

	if ( m_pColourInit && HasBuffer( TGenericParticles::FLAGS_COLOUR ) )
	{
		m_pColourInit->InitColour( a_pParticles->m_auiColour, a_iStartParticle, a_iEndParticle );

		if ( m_pColourKeys != TNULL )
		{
			for ( TINT i = a_iStartParticle; i < a_iEndParticle; i++ )
				a_pParticles->m_apCurColourKey[ i ] = 0;
		}
	}

	if ( m_pEventKeys != TNULL )
	{
		for ( TINT i = a_iStartParticle; i < a_iEndParticle; i++ )
			a_pParticles->m_auiEventIndex[ i ] = 0;
	}

	TParticleInit::SetCurParticles( TNULL );
}

// $deBlob: FUNCTION 0071ab10
TUINT32 TGenericParticlesSettings::ConvertColourToSKUColour( TUINT32 a_uiColour )
{
	return a_uiColour;
}

// $deBlob: FUNCTION 0071ab20
void TQuadRenderSettings::Init()
{
	TRandom oRandom;

	for ( TINT i = 0; i < m_iNumRotations; i++ )
		m_afRotations[ i ] = TMath::LERP( m_fMinRotation, m_fMaxRotation, oRandom.GetFloat() );
}

// $deBlob: FUNCTION 0071abe0
void TBillboardQuadRenderSettings::Init()
{
	TRandom oRandom;

	for ( TINT i = 0; i < m_iNumRotations; i++ )
	{
		m_afRotationsX[ i ] = TMath::LERP( m_fMinX, m_fMaxX, oRandom.GetFloat() );
		m_afRotationsY[ i ] = TMath::LERP( m_fMinY, m_fMaxY, oRandom.GetFloat() );
		m_afRotationsZ[ i ] = TMath::LERP( m_fMinZ, m_fMaxZ, oRandom.GetFloat() );
	}
}

TOSHI_NAMESPACE_END
