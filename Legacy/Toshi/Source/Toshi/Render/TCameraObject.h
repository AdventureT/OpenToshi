#pragma once
#include "TTransformObject.h"
#include "TRender.h"

TOSHI_NAMESPACE_START

class TCameraObject
{
public:
	static constexpr TFLOAT s_kMinFOV = (1.0f / 180.0f) * TMath::PI;
	static constexpr TFLOAT s_kMaxFOV = TMath::PI;

public:
	TCameraObject();

	void Render();

	// $deBlob: FUNCTION 006add00
	TFLOAT SetNear(TFLOAT fNear) { return std::exchange(m_fNear, fNear); }

	// $deBlob: FUNCTION 006add20
	TFLOAT SetFar(TFLOAT fFar) { return std::exchange(m_fFar, fFar); }

	// $deBlob: FUNCTION 006add50
	TFLOAT SetFOV(TFLOAT fFOV)
	{
		TMath::Clip(fFOV, s_kMinFOV, s_kMaxFOV);
		return std::exchange(m_fFOV, fFOV);
	}

	// $deBlob: FUNCTION 006addb0
	TFLOAT SetProjectionCentreX(TFLOAT fCentreX)
	{
		TMath::Clip(fCentreX, 0.0f, 1.0f);
		return std::exchange(m_fCentreX, fCentreX);
	}

	// $deBlob: FUNCTION 006ade00
	TFLOAT SetProjectionCentreY(TFLOAT fCentreY)
	{
		TMath::Clip(fCentreY, 0.0f, 1.0f);
		return std::exchange(m_fCentreY, fCentreY);
	}

	TFLOAT GetNear() const { return m_fNear; }

	// $deBlob: FUNCTION 006ade70
	TFLOAT GetFar() const { return m_fFar; }

	TFLOAT GetFOV() const { return m_fFOV; }

	TRenderContext::CameraMode GetMode() const { return m_eMode; }

	TBOOL IsEnabled() const { return m_bEnabled; }

	// $deBlob: FUNCTION 006adec0
	TTransformObject& GetTransformObject() { return m_TransformObject; }

private:
	TFLOAT                     m_fNear;
	TFLOAT                     m_fFar;
	TFLOAT                     m_fFOV;
	TINT                       m_Unk1;
	TRenderContext::CameraMode m_eMode;
	TFLOAT                     m_fCentreX;
	TFLOAT                     m_fCentreY;
	TTransformObject           m_TransformObject;
	TMatrix44                  m_TransformMatrix;
	TBOOL                      m_bEnabled;
	TFLOAT                     m_fUnk2;
};

TOSHI_NAMESPACE_END
