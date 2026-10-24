#include "ToshiPCH.h"
#include "TCameraObject.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

const TFLOAT TCameraObject::s_kMinFOV = ( 1.0f / 180.0f ) * TMath::PI;
const TFLOAT TCameraObject::s_kMaxFOV = TMath::PI;

TSTATICASSERT( sizeof( TCameraObject ) == 0xC8 );

// $deBlob: FUNCTION 006ada60
// $Barnyard: FUNCTION 006cd030
TCameraObject::TCameraObject()
{
	m_fNear    = 1.0f;
	m_fFar     = 100.0f;
	m_fFOV     = TMath::DegToRad( 90.0f );
	m_eScaling = FOV_SCALING_TYPE_HORIZONTAL;
	m_eMode    = TRenderContext::CameraMode_Perspective;
	m_fCentreX = 0.5f;
	m_fCentreY = 0.5f;
	m_bEnabled = TTRUE;
	m_fUnk     = 1.0f;
}

// $deBlob: FUNCTION 006adab0
// $deBlob: FUNCTION 005f1c10
TCameraObject::~TCameraObject()
{
}

// $deBlob: FUNCTION 006adae0
// $Barnyard: FUNCTION 006cd0d0
void TCameraObject::Render()
{
	TRender*        pRender        = TRender::GetSingleton();
	TRenderContext* pRenderContext = pRender->GetCurrentContext();

	if ( !m_bEnabled )
		return;

	TRenderContext::PROJECTIONPARAMS projParams = pRenderContext->GetProjectionParameters();
	projParams.m_Centre.x                       = m_fCentreX * pRenderContext->GetWidth();
	projParams.m_Centre.y                       = m_fCentreY * pRenderContext->GetHeight();

	if ( m_eScaling == FOV_SCALING_TYPE_HORIZONTAL )
	{
		projParams.m_Proj.x = ( pRenderContext->GetWidth() * 0.5f ) / tan( m_fFOV * 0.5f );
		projParams.m_Proj.y = projParams.m_Proj.x;

		if ( m_eMode == TRenderContext::CameraMode_Perspective )
		{
			projParams.m_Proj.y *= pRender->GetResolutionScalar();

			if ( m_eMode == TRenderContext::CameraMode_Perspective && pRender->GetAspectRatio() == TRender::ASPECT_RATIO_16_9 )
			{
				projParams.m_Proj.x *= 0.8660254f;
				projParams.m_Proj.y *= 0.8660254f;
			}
		}
	}
	else
	{
		projParams.m_Proj.y = ( pRenderContext->GetHeight() * 0.5f ) / tan( m_fFOV * 0.5f );
		projParams.m_Proj.x = projParams.m_Proj.y;

		if ( m_eMode == TRenderContext::CameraMode_Perspective )
		{
			projParams.m_Proj.x /= pRender->GetResolutionScalar();

			if ( m_eMode == TRenderContext::CameraMode_Perspective && pRender->GetAspectRatio() == TRender::ASPECT_RATIO_16_9 )
			{
				projParams.m_Proj.x *= 0.8660254f;
				projParams.m_Proj.y *= 0.8660254f;
			}
		}
	}

	projParams.m_fNearClip = m_fNear;
	projParams.m_fFarClip  = m_fFar;
	projParams.m_fUnk      = m_fUnk;
	pRenderContext->SetProjectionParameters( projParams );

	if ( m_eMode == TRenderContext::CameraMode_Perspective )
		pRenderContext->SetProjType( TRenderContext::CameraMode_Perspective );
	else if ( m_eMode == TRenderContext::CameraMode_Orthographic )
		pRenderContext->SetProjType( TRenderContext::CameraMode_Orthographic );

	TMatrix44 matrix = m_TransformObject.GetLocalMatrix();
	matrix.InvertOrthonormal();

	pRenderContext->SetWorldViewMatrix( matrix );
	pRenderContext->SetModelViewMatrix( matrix );
	pRenderContext->Update();
}

TOSHI_NAMESPACE_END
