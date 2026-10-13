#pragma once
#include "Cameras/ACamera.h"

class AFreeCamera : public ACamera
{
public:
	// $deBlob: FUNCTION 005902d0
	AFreeCamera()
	    : ACamera(CT_FREE)
	{
	}

	// $deBlob: FUNCTION 005909f0
	void PushRight(TFLOAT fVal)
	{
		TIMPLEMENT();
	}

	// $deBlob: FUNCTION 0058d610
	void Yaw(TFLOAT fYaw)
	{
		TTODO("Is it really -fYaw? Needs testing");
		RotateAroundWorldUp(-fYaw);
	}

	// $deBlob: FUNCTION 0058d670
	void Pitch(TFLOAT fPitch)
	{
		TTODO("Is it really -fPitch? Needs testing");
		RotateAroundRight(-fPitch, 0.01f);
	}

	// $deBlob: FUNCTION 00590780
	void SetFreeCameraSpeed(TFLOAT a_fFreeCamSpeed)
	{
		TASSERT(m_fFreeCamSpeed > 0.0f, "Free Camera Speed must be larger than zero\n");
		m_fFreeCamSpeed = a_fFreeCamSpeed;
	}

private:
	TFLOAT m_fFreeCamSpeed; // 0x104
};
