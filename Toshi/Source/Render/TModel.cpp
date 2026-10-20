#include "ToshiPCH.h"
#include "TModel.h"
#include "TModelManager.h"

#include "Render/TRender.h"
#include "Render/TRenderContext.h"
#include "Render/TCameraObject.h"

#ifdef TRENDERINTERFACE_DX11
#  include "Platform/DX11/TRender_DX11.h"
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TModel::t_TRBLoadCallback TModel::sm_pTRBLoadCallback;
const TCHAR*              TModel::sm_SymbolNamePrefix;
TMemoryHeap*              TModel::s_pMemHeap;
TFLOAT                    TModelInstance::s_fOldLightTexMag;
TUINT                     TModelInstance::s_iOldClipFlags;

static TCHAR s_TranslatedSymbol[ 512 ];

TSTATICASSERT( sizeof( TModelLOD ) == 0x18 );
TSTATICASSERT( sizeof( TModel ) == 0xBC );
TSTATICASSERT( sizeof( TModelInstance ) == 0xE0 );

// $deBlob: FUNCTION 006af3b0
TBOOL TModel::LoadTRB( TTRB* a_pTRB, TBOOL a_bFreeOnUnload )
{
	SetTRB( a_pTRB, a_bFreeOnUnload );
	return LoadTRB();
}

// $deBlob: FUNCTION 006af3d0
void TModel::UnloadTRB( TBOOL a_bFreeTRB )
{
	if ( m_pTRB != TNULL )
	{
		if ( a_bFreeTRB && m_bFreeOnUnload )
		{
			m_pTRB->Close();

			if ( m_pTRB )
				delete m_pTRB;
		}

		m_pTRB = TNULL;
	}

	m_eFlags &= ~Flags_TrbLoaded;
	m_pCollision = TNULL;
	m_pSkeleton  = TNULL;
}

// $deBlob: FUNCTION 006af460
TModelInstance* TModel::CreateInstance()
{
	TModelInstance* pInstance;

	if ( s_pMemHeap != TNULL )
		pInstance = new ( s_pMemHeap ) TModelInstance( T2ModelPtr( GetResourceId() ) );
	else
		pInstance = new TModelInstance( T2ModelPtr( GetResourceId() ) );

	if ( pInstance == TNULL )
		return TNULL;

	if ( m_pSkeleton != TNULL )
	{
		pInstance->m_pSkeletonInstance = m_pSkeleton->CreateInstance( TTRUE );

		if ( pInstance->m_pSkeletonInstance == TNULL )
		{
			// NOTE: the instance's destructor decrements m_iNumInstances although it wasn't incremented yet
			delete pInstance;
			return TNULL;
		}
	}

	m_iNumInstances++;
	return pInstance;
}

// $deBlob: FUNCTION 006af5a0
void TModel::CreateCollision( TModelCollision* a_pCollision )
{
	m_pCollision = a_pCollision;
}

// $deBlob: FUNCTION 006af5b0
void TModel::CreateSkeleton( TTMDBase::SkeletonHeader* a_pSkeletonHeader, TSkeleton* a_pSkeleton, TBOOL a_bLoadAnimations )
{
	if ( a_pSkeletonHeader != TNULL )
	{
		TASSERT( TNULL == m_pSkeleton );
		m_pSkeleton = a_pSkeleton;

		if ( a_bLoadAnimations && a_pSkeletonHeader->m_pTKLName != TNULL )
		{
			// NOTE: the library can legitimately be missing, retail passes TNULL on silently
			TKeyframeLibrary* pLibrary = TRender::GetSingleton()->GetKeyframeLibraryManager().GetLibrary( a_pSkeletonHeader->m_pTKLName );

			a_pSkeleton->GetKeyLibraryInstance().CreateEx(
			    pLibrary,
			    a_pSkeletonHeader->m_iTKeyCount,
			    a_pSkeletonHeader->m_iQKeyCount,
			    a_pSkeletonHeader->m_iSKeyCount,
			    a_pSkeletonHeader->m_iTBaseIndex,
			    a_pSkeletonHeader->m_iQBaseIndex,
			    a_pSkeletonHeader->m_iSBaseIndex
			);
		}
	}
}

// $deBlob: FUNCTION 006af640
TModel::TModel()
{
	m_eFlags        = Flags_None;
	m_iNumInstances = 0;
	m_iLODCount     = 0;
	m_fUnknown      = -1.0f;
	m_pSkeleton     = TNULL;
	m_pCollision    = TNULL;
	m_pTRB          = TNULL;
	m_bFreeOnUnload = TTRUE;
	m_szName        = "unknown";
	m_pAssetData    = TNULL;

	m_aLODDistances[ 0 ] = 5.0f;
	m_aLODDistances[ 1 ] = 20.0f;
	m_aLODDistances[ 2 ] = 40.0f;
	m_aLODDistances[ 3 ] = 80.0f;
}

// $deBlob: FUNCTION 006af7c0
// $deBlob: FUNCTION 006b6140
TModel::~TModel()
{
	TASSERT( TNULL == m_pTRB );

	if ( GetResourcePtr() )
		TModelManager::GetSingleton()->RemoveModel( T2ModelPtr( GetResourceId() ) );

	// NOTE: retail destroys the resource after m_pPrevModel/m_pNextModel (an inlined T2Resource destructor)
	T2ResourceManager::GetSingleton()->DestroyResource( GetResourceId() );
}

// $deBlob: FUNCTION 006af890
TBOOL TModel::LoadTRBFile( const TCHAR* a_szFileName )
{
	if ( m_bFreeOnUnload && m_pTRB != TNULL )
		delete m_pTRB;

	m_pTRB = new TTRB();

	if ( m_pTRB->Load( a_szFileName, 0 ) != TTRB::ERROR_OK )
		return TFALSE;

	return LoadTRB();
}

// $deBlob: FUNCTION 006af900
TBOOL TModel::LoadTRBFile( TFile* a_pFile )
{
	if ( m_bFreeOnUnload && m_pTRB != TNULL )
		delete m_pTRB;

	m_pTRB = new TTRB();

	if ( m_pTRB->Load( a_pFile, 0 ) != TTRB::ERROR_OK )
		return TFALSE;

	return LoadTRB();
}

// $deBlob: FUNCTION 006af970
void TModel::CreateResource( const TCHAR* a_szName )
{
	T2Resource::CreateResource( a_szName, this, ResourceCallback, this );
	TModelManager::GetSingleton()->AddModel( T2ModelPtr( GetResourceId() ) );
}

// $deBlob: FUNCTION 006af9c0
TBOOL TModel::LoadTRB()
{
	auto pSkeletonHeader = TSTATICCAST( TTMDBase::SkeletonHeader, m_pTRB->GetSymbolAddress( TranslateSymbolName( "SkeletonHeader" ) ) );
	auto pSkeleton       = TSTATICCAST( TSkeleton, m_pTRB->GetSymbolAddress( TranslateSymbolName( "Skeleton" ) ) );
	auto pCollision      = TSTATICCAST( TModelCollision, m_pTRB->GetSymbolAddress( TranslateSymbolName( "Collision" ) ) );

	CreateSkeleton( pSkeletonHeader, pSkeleton, TTRUE );
	CreateCollision( pCollision );

	TASSERT( TNULL != sm_pTRBLoadCallback );

	if ( !sm_pTRBLoadCallback( m_pTRB, this ) )
		TASSERT( !"Failed To Load Model TRB" );

	m_eFlags |= Flags_TrbLoaded;
	return TTRUE;
}

// $deBlob: FUNCTION 006afb90
void* __stdcall TModel::ResourceCallback( void* a_pData, TTRB* a_pTRB, TBOOL a_bCreated )
{
	if ( a_bCreated )
	{
		TModel* pModel = TSTATICCAST( TModel, a_pData );

#ifdef TRENDERINTERFACE_DX11
		TRenderDX11::Interface()->WaitForEndOfRender();
#endif

		// UnloadTRB( TFALSE ), inlined
		if ( pModel->m_pTRB != TNULL )
			pModel->m_pTRB = TNULL;

		pModel->m_eFlags &= ~Flags_TrbLoaded;
		pModel->m_pTRB          = a_pTRB;
		pModel->m_pCollision    = TNULL;
		pModel->m_pSkeleton     = TNULL;
		pModel->m_bFreeOnUnload = TTRUE;
		pModel->LoadTRB();

		return pModel;
	}

	return TNULL;
}

// $deBlob: FUNCTION 006afc00
const TCHAR* __stdcall TModel::TranslateSymbolName( const TCHAR* a_szSymbolName )
{
	if ( sm_SymbolNamePrefix == TNULL )
		return a_szSymbolName;

	T2String8::Format( s_TranslatedSymbol, sizeof( s_TranslatedSymbol ), "%s_%s", sm_SymbolNamePrefix, a_szSymbolName );
	return s_TranslatedSymbol;
}

// $deBlob: FUNCTION 006afc40
// $deBlob: FUNCTION 00558050
TModelInstance::~TModelInstance()
{
	if ( m_pSkeletonInstance != TNULL )
	{
		m_pSkeletonInstance->Delete();
		m_pSkeletonInstance = TNULL;
	}

	m_pModel->m_iNumInstances--;
	m_pModel.Release();
}

// $deBlob: FUNCTION 006afc90
void TModelInstance::Update( TFLOAT a_fDeltaTime )
{
	TASSERT( m_pModel.IsValid() );
	m_pSkeletonInstance->UpdateTime( a_fDeltaTime );
}

// $deBlob: FUNCTION 006afcf0
void TModelInstance::Render()
{
	TASSERT( m_pModel.IsValid() );

	if ( ( m_eFlags & Flags_Visible ) && TGFXFileInstanceObjectPreRender() )
	{
		if ( m_fnPreRenderCb != TNULL )
		{
			m_fnPreRenderCb( this );
			TGFXFileInstanceObjectPostRender();
			return;
		}

		if ( m_fnCustomRenderCb != TNULL )
		{
			m_fnCustomRenderCb( this, m_pCustomRenderCbUserData );
			TGFXFileInstanceObjectPostRender();
			return;
		}

		m_pSkeletonInstance->UpdateState( TFALSE );
		TRender::GetSingleton()->GetCurrentContext()->SetSkeletonInstance( m_pSkeletonInstance );

		TModelLOD& rLOD = m_pModel.GetData()->GetLOD( m_iCurrentLOD );

		for ( TINT i = 0; i < rLOD.m_iMeshCount; i++ )
			rLOD.m_pMeshes[ i ]->Render();

		TGFXFileInstanceObjectPostRender();
	}
}

// $deBlob: FUNCTION 006afdd0
void TModelInstance::SetUVHashOffset( void* a_pUVHashOffset )
{
	TASSERT( TNULL != a_pUVHashOffset );
	m_pUVHashOffset = a_pUVHashOffset;
}

// $deBlob: FUNCTION 006afe30
void TModelInstance::SetTextureStages( TINT8 a_iStage0, TINT8 a_iStage1, TINT8 a_iStage2, TINT8 a_iStage3 )
{
	m_aTextureStages[ 0 ] = a_iStage0;
	m_aTextureStages[ 1 ] = a_iStage1;
	m_aTextureStages[ 2 ] = a_iStage2;
	m_aTextureStages[ 3 ] = a_iStage3;
}

// $deBlob: FUNCTION 006afe60
void TModelInstance::SetTextureStagesA( TINT8* a_pStages )
{
	m_aTextureStages[ 0 ] = a_pStages[ 0 ];
	m_aTextureStages[ 1 ] = a_pStages[ 1 ];
	m_aTextureStages[ 2 ] = a_pStages[ 2 ];
	m_aTextureStages[ 3 ] = a_pStages[ 3 ];
}

// $deBlob: FUNCTION 006afea0
void TModelInstance::GetTextureStages( TINT8* a_pOutStages )
{
	a_pOutStages[ 0 ] = m_aTextureStages[ 0 ];
	a_pOutStages[ 1 ] = m_aTextureStages[ 1 ];
	a_pOutStages[ 2 ] = m_aTextureStages[ 2 ];
	a_pOutStages[ 3 ] = m_aTextureStages[ 3 ];
}

// $deBlob: FUNCTION 006afee0
void TModelInstance::SetTextureAlphas( TUINT8 a_uiAlpha0, TUINT8 a_uiAlpha1, TUINT8 a_uiAlpha2, TUINT8 a_uiAlpha3 )
{
	m_aTextureAlphas[ 0 ] = a_uiAlpha0;
	m_aTextureAlphas[ 1 ] = a_uiAlpha1;
	m_aTextureAlphas[ 2 ] = a_uiAlpha2;
	m_aTextureAlphas[ 3 ] = a_uiAlpha3;
}

// $deBlob: FUNCTION 006aff10
void TModelInstance::SetTextureAlphasA( TUINT8* a_pAlphas )
{
	m_aTextureAlphas[ 0 ] = a_pAlphas[ 0 ];
	m_aTextureAlphas[ 1 ] = a_pAlphas[ 1 ];
	m_aTextureAlphas[ 2 ] = a_pAlphas[ 2 ];
	m_aTextureAlphas[ 3 ] = a_pAlphas[ 3 ];
}

// $deBlob: FUNCTION 006aff50
void TModelInstance::GetTextureAlphas( TUINT8* a_pOutAlphas )
{
	a_pOutAlphas[ 0 ] = m_aTextureAlphas[ 0 ];
	a_pOutAlphas[ 1 ] = m_aTextureAlphas[ 1 ];
	a_pOutAlphas[ 2 ] = m_aTextureAlphas[ 2 ];
	a_pOutAlphas[ 3 ] = m_aTextureAlphas[ 3 ];
}

// $deBlob: FUNCTION 006aff90
TBOOL TModelInstance::IsInViewFrustum()
{
	if ( !( m_eFlags & Flags_Visible ) )
		return TFALSE;

	TRenderContext* pContext = TRender::GetSingleton()->GetCurrentContext();
	TSphere         oVolume  = T2ModelPtr( m_pModel )->GetLOD( 0 ).GetRenderVolume();

	const TVector3& rScale = m_TransformObject.GetScale();
	TFLOAT          fMaxScale;
	TSphere         oSphere;

	fMaxScale = ( rScale.x < rScale.y ) ? rScale.y : rScale.x;
	fMaxScale = ( fMaxScale < rScale.z ) ? rScale.z : fMaxScale;

	TVector3 vCentre( oVolume.GetOrigin().x * rScale.x, oVolume.GetOrigin().y * rScale.y, oVolume.GetOrigin().z * rScale.z );
	TFLOAT   fRadius = fMaxScale * oVolume.GetRadius();

	if ( 0.0f < fRadius )
	{
		TMatrix44 oLocalMatrix = m_TransformObject.GetLocalMatrix();
		TMatrix44::TransformVector( vCentre, oLocalMatrix, vCentre );

		if ( !TRenderContext::CullSphereToFrustumSimple( TSphere( vCentre, fRadius ), pContext->GetWorldPlanes(), 6 ) )
			return TFALSE;
	}

	return TTRUE;
}

// $deBlob: FUNCTION 006b00c0
void TModelInstance::CalculateWorldSpacePosition( TVector4& a_rOutPosition )
{
	const TVector3& rScale = m_TransformObject.GetScale();
	TVector4        vScale( rScale.x, rScale.y, rScale.z, 1.0f );

	TSphere  oVolume = T2ModelPtr( m_pModel )->GetLOD( 0 ).GetRenderVolume();
	TVector4 vCentre( oVolume.GetOrigin().x, oVolume.GetOrigin().y, oVolume.GetOrigin().z, 1.0f );

	TVector4 vLocal;
	vLocal.Multiply( vCentre, vScale );

	TMatrix44 oLocalMatrix = m_TransformObject.GetLocalMatrix();
	TMatrix44::TransformVector( a_rOutPosition, oLocalMatrix, vLocal );
}

// $deBlob: FUNCTION 006b01f0
TModelInstance::TModelInstance( T2ModelPtr a_pModel )
    : m_pModel( a_pModel )
{
	m_pSkeletonInstance       = TNULL;
	m_fnPreRenderCb           = TNULL;
	m_fnCustomRenderCb        = TNULL;
	m_pCustomRenderCbUserData = TNULL;
	m_iCurrentLOD             = 0;
	m_pUVHashOffset           = TNULL;
	m_eFlags                  = Flags_None;
	m_fFade                   = 1.0f;
	m_fLightTexMag            = 0.0f;

	TASSERT( m_pModel.IsValid() );
	m_eFlags |= Flags_Visible | Flags_DistanceFade;

	m_aTextureStages[ 0 ] = 0;
	m_aTextureStages[ 1 ] = -1;
	m_aTextureStages[ 2 ] = -1;
	m_aTextureStages[ 3 ] = -1;
	m_aTextureAlphas[ 0 ] = 128;
	m_aTextureAlphas[ 1 ] = 0;
	m_aTextureAlphas[ 2 ] = 0;
	m_aTextureAlphas[ 3 ] = 0;
	m_fFadeStart          = 0.0f;
	m_fFadeEnd            = 0.0f;
	m_fFadeRangeInv       = 1.0f;

	m_eFlags |= Flags_Created;
}

// $deBlob: FUNCTION 006b02e0
TBOOL TModelInstance::TGFXFileInstanceObjectPreRender()
{
	TRenderContext* pContext = TRender::GetSingleton()->GetCurrentContext();
	TSphere         oVolume  = T2ModelPtr( m_pModel )->GetLOD( 0 ).GetRenderVolume();

	s_iOldClipFlags = pContext->GetClipFlags();

	const TVector3& rScale = m_TransformObject.GetScale();
	TFLOAT          fMaxScale;

	fMaxScale = ( rScale.x < rScale.y ) ? rScale.y : rScale.x;
	fMaxScale = ( fMaxScale < rScale.z ) ? rScale.z : fMaxScale;

	TVector3 vCentre( oVolume.GetOrigin().x * rScale.x, oVolume.GetOrigin().y * rScale.y, oVolume.GetOrigin().z * rScale.z );
	TFLOAT   fRadius    = fMaxScale * oVolume.GetRadius();
	TUINT    uiClipFlags = s_iOldClipFlags;

	TMatrix44 oLocalMatrix = m_TransformObject.GetLocalMatrix();
	TMatrix44::TransformVector( vCentre, oLocalMatrix, vCentre );

	TASSERT( TTRUE == IsCreated() );

	if ( 0.0f < fRadius && !TRenderContext::CullSphereToFrustumSimple( TSphere( vCentre, fRadius ), pContext->GetWorldPlanes(), 6 ) )
		return TFALSE;

	TCameraObject* pCamera = pContext->GetCameraObject();
	TVector3       vToCamera;
	vToCamera.Substract( vCentre, pCamera->GetTransformObject().GetTranslation() );

	TFLOAT fDistance = vToCamera.Magnitude();

	if ( !( m_eFlags & Flags_ForceFade ) )
	{
		if ( ( m_eFlags & Flags_DistanceFade ) && fDistance >= m_fFadeStart && m_fFadeStart < m_fFadeEnd && fDistance > m_fFadeEnd )
			return TFALSE;
	}
	else if ( m_fFade <= 0.0f )
	{
		return TFALSE;
	}

	if ( m_fFadeStart == m_fFadeEnd )
	{
		TFLOAT fFar  = pCamera->GetFar();
		TFLOAT fNear = pCamera->GetNear();

		m_fFadeEnd      = ( fFar - fNear ) * 0.25f + fNear;
		m_fFadeStart    = ( fFar - fNear ) * 0.2f + fNear;
		m_fFadeRangeInv = 1.0f / ( m_fFadeEnd - m_fFadeStart );
	}

	T2ModelPtr pModel = m_pModel;

	// Narrow fields of view pick more detailed LODs
	TFLOAT fFOV = pCamera->GetFOV();

	if ( fFOV < 1.0f )
		fDistance -= ( 1.0f - fFOV ) * 16.0f;

	TINT iLOD = pModel->GetLODCount() - 1;

	while ( 0 < iLOD )
	{
		if ( pModel->GetLODDistance( iLOD ) < fDistance )
			break;

		iLOD--;
	}

	if ( pModel->GetLODCount() <= iLOD )
		iLOD = pModel->GetLODCount() - 1;

	SetLOD( iLOD );
	m_TransformObject.Push();

	s_fOldLightTexMag = pContext->GetShadeCoeff();
	pContext->SetClipFlags( uiClipFlags );

	TASSERT( m_fLightTexMag >= 0.0f && m_fLightTexMag <= 1.0f );
	pContext->SetShadeCoeff( m_fLightTexMag );

	if ( m_eFlags & Flags_ForceFade )
		pContext->SetAlphaBlend( m_fFade );

	return TTRUE;
}

// $deBlob: FUNCTION 006b0790
void TModelInstance::TGFXFileInstanceObjectPostRender()
{
	TRenderContext* pContext = TRender::GetSingleton()->GetCurrentContext();

	m_TransformObject.Pop();
	pContext->SetClipFlags( s_iOldClipFlags );

	TASSERT( s_fOldLightTexMag >= 0.0f && s_fOldLightTexMag <= 1.0f );
	pContext->SetShadeCoeff( s_fOldLightTexMag );
}

TOSHI_NAMESPACE_END
