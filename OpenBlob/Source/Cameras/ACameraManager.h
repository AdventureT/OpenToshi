#pragma once
#include "ACamera.h"

class ACameraManager : public Toshi::TSingleton<ACameraManager>
{
public:
	static constexpr TUINT8 MAX_CAMERA = 14;

public:
	ACameraManager();

	ACamera* CreateCamera(ACamera::CameraType a_eCamType);

	ACamera* GetCurrentCamera() const
	{
		if (m_iCurrentCamera >= 0)
		{
			TASSERT(m_iCurrentCamera >= 0 && m_iCurrentCamera < MAX_CAMERA);
			return m_ppCameras[m_iCurrentCamera];
		}

		return TNULL;
	}

	template <class T>
	T* CreateCamera()
	{
		static_assert(std::is_base_of_v<ACamera, T> == TTRUE);

		for (TUINT8 i = 0; i < MAX_CAMERA; i++)
		{
			if (m_ppCameras[i] == TNULL)
			{
				m_ppCameras[i] = new T();

				if (m_iCurrentCamera == -1)
					m_iCurrentCamera = i;

				if (m_ppCameras[i] != TNULL)
				{
					if (m_ppCameras[i]->GetSomeFlag())
						m_iSomeCount++;

					m_ppCameras[i]->SetSomeFlag(TFALSE);
				}

				return TSTATICCAST(T, m_ppCameras[i]);
			}
		}

		return TNULL;
	}

private:
	TBOOL    m_bFlag;                 // 0x0000
	ACamera* m_ppCameras[MAX_CAMERA]; // 0x0340
	TINT8    m_iCurrentCamera;        // 0x0378
	TINT     m_iSomeCount;            // 0x037C
};
