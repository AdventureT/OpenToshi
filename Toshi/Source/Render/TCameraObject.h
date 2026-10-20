#pragma once
#include "TTransformObject.h"
#include "TRender.h"

#include <Math/TMathInline.h>

TOSHI_NAMESPACE_START

class TCameraObject
{
public:
	static const TFLOAT s_kMinFOV;
	static const TFLOAT s_kMaxFOV;

	enum FOV_SCALING_TYPE : TINT
	{
		FOV_SCALING_TYPE_HORIZONTAL = 0,
		FOV_SCALING_TYPE_VERTICAL   = 1
	};

public:
	TCameraObject();
	~TCameraObject();

	void Render();

	// $deBlob: FUNCTION 006add00
	// $Barnyard: FUNCTION 006cd080
	TFLOAT SetNear( TFLOAT fNear ) { return TExchange( m_fNear, fNear ); }

	// $deBlob: FUNCTION 006add20
	// $Barnyard: FUNCTION 006cd090
	TFLOAT SetFar( TFLOAT fFar ) { return TExchange( m_fFar, fFar ); }

	// $deBlob: FUNCTION 006add40
	FOV_SCALING_TYPE SetFOVScaling( FOV_SCALING_TYPE a_eScaling ) { return TExchange( m_eScaling, a_eScaling ); }

	// $deBlob: FUNCTION 006add50
	// $Barnyard: FUNCTION 006cd220
	TFLOAT SetFOV( TFLOAT fFOV )
	{
		TMath::Clip( fFOV, s_kMinFOV, s_kMaxFOV );
		return TExchange( m_fFOV, fFOV );
	}

	// $deBlob: FUNCTION 006addb0
	// $Barnyard: FUNCTION 006cd270
	TFLOAT SetProjectionCentreX( TFLOAT fCentreX )
	{
		TMath::Clip( fCentreX, 0.0f, 1.0f );
		return TExchange( m_fCentreX, fCentreX );
	}

	// $deBlob: FUNCTION 006ade00
	// $Barnyard: FUNCTION 006cd2d0
	TFLOAT SetProjectionCentreY( TFLOAT fCentreY )
	{
		TMath::Clip( fCentreY, 0.0f, 1.0f );
		return TExchange( m_fCentreY, fCentreY );
	}

	// $deBlob: FUNCTION 006ade60
	TFLOAT GetNear() const { return m_fNear; }
	// $deBlob: FUNCTION 006ade70
	TFLOAT GetFar() const { return m_fFar; }
	// $deBlob: FUNCTION 006ade80
	FOV_SCALING_TYPE GetFOVScaling() const { return m_eScaling; }
	// $deBlob: FUNCTION 006ade90
	TFLOAT GetFOV() const { return m_fFOV; }
	// $deBlob: FUNCTION 006adea0
	TFLOAT GetProjectionCentreX() const { return m_fCentreX; }
	// $deBlob: FUNCTION 006adeb0
	TFLOAT GetProjectionCentreY() const { return m_fCentreY; }

	// $deBlob: FUNCTION 006adef0
	TRenderContext::CameraMode GetMode() const { return m_eMode; }
	// $deBlob: FUNCTION 006ade50
	void                       SetMode( TRenderContext::CameraMode a_eMode ) { m_eMode = a_eMode; }

	// $deBlob: FUNCTION 006adf00
	TBOOL IsEnabled() const { return m_bEnabled; }
	// $deBlob: FUNCTION 006aded0
	TBOOL SetEnabled( TBOOL a_bEnabled ) { return TExchange( m_bEnabled, a_bEnabled ); }

	// $deBlob: FUNCTION 006adec0
	TTransformObject& GetTransformObject() { return m_TransformObject; }

private:
	TFLOAT                     m_fNear;           // 0x00
	TFLOAT                     m_fFar;            // 0x04
	TFLOAT                     m_fFOV;            // 0x08
	FOV_SCALING_TYPE           m_eScaling;        // 0x0C
	TRenderContext::CameraMode m_eMode;           // 0x10
	TFLOAT                     m_fCentreX;        // 0x14
	TFLOAT                     m_fCentreY;        // 0x18
	TTransformObject           m_TransformObject; // 0x1C
	TBOOL                      m_bEnabled;        // 0xC0
	TFLOAT                     m_fUnk;            // 0xC4 PROJECTIONPARAMS::m_fUnk (1.0)
}; // 0xC8

TOSHI_NAMESPACE_END
