#pragma once

class ACamera : public Toshi::TGenericClassDerived<ACamera, Toshi::TObject, "ACamera", TMAKEVERSION(1, 0), TFALSE>
{
public:
	struct CameraMatrix
	{
		Toshi::TMatrix44 m_mMatrix;
		Toshi::TMatrix44 m_mShakeMatrix;
		TFLOAT           m_fNear;
		TFLOAT           m_fFar;
		TFLOAT           m_fFOV;
		TFLOAT           m_fAspect;
		TFLOAT           m_fProjectionCentreX;
		TFLOAT           m_fProjectionCentreY;
	};

	enum CameraType
	{
		CT_DEFAULT,
		CT_BLOBFOLLOW,
		CT_SCRIPTED,
		CT_FREE,
		CT_KEYFRAME,
		CT_TYPECOUNT
	};

public:
	// $deBlob: FUNCTION 0058d140
	ACamera(CameraType a_eCamType)
	    : m_eCamType(a_eCamType)
	{
		ResetCameraMatrix(m_Matrix);
		TASSERT(a_eCamType >= 0 && a_eCamType < CT_TYPECOUNT);
	}

	TBOOL IsInViewCone(const Toshi::TVector4& a_rvPos, TFLOAT a_fSphereRadius) const;
	void  LookAtPoint(const Toshi::TVector4& a_vPoint);

	static void ResetCameraMatrix(CameraMatrix& camMat);

	void RotateAroundWorldUp(TFLOAT rotation)
	{
		RotateAroundAxis(sm_vWorldUp, rotation);
	}

	void RotateAroundAxis(const Toshi::TVector4& a_vAxis, TFLOAT rotation);
	void RotateAroundRight(TFLOAT rotation, TFLOAT a_fVal);

	// $deBlob: FUNCTION 0058d210
	void SetMatrix(const Toshi::TMatrix44& a_mMatrix)
	{
		m_Matrix.m_mMatrix = a_mMatrix;
	}

	// $deBlob: FUNCTION 0058d250
	void SetShakeMatrix(const Toshi::TMatrix44& a_mSkakeMatrix)
	{
		m_Matrix.m_mShakeMatrix = a_mSkakeMatrix;
	}

	void LookAtDirection(const Toshi::TVector4& a_vDirection)
	{
		m_Matrix.m_mMatrix.LookAtDirection(a_vDirection, sm_vWorldUp);
	}

	void LookAtDirection(const Toshi::TVector4& a_vDirection, const Toshi::TVector4& a_vVec2)
	{
		m_Matrix.m_mMatrix.LookAtDirection(a_vDirection, a_vVec2);
	}

	TBOOL GetSomeFlag() const
	{
		return m_bSomeFlag;
	}

	void SetSomeFlag(TBOOL bFlag)
	{
		m_bSomeFlag = bFlag;
	}

	CameraMatrix& GetCameraMatrix()
	{
		return m_Matrix;
	}

public:
	static constexpr TFLOAT      sm_fDefaultFOV = Toshi::TMath::DegToRad(60.0f);
	static const Toshi::TVector4 sm_vInitialPos;
	static const Toshi::TVector4 sm_vWorldUp;

protected:
	TBOOL        m_bSomeFlag; // 0x04
	CameraType   m_eCamType;  // 0x08
	CameraMatrix m_Matrix;    // 0x6C
};
