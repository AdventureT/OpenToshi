#pragma once
#include <Toshi/TObject.h>
#include <Math/TMatrix44.h>
#include <Math/TVector4.h>
#include <Math/TPlane.h>

//-----------------------------------------------------------------------------
// Purpose: a camera of ACameraManager.
//-----------------------------------------------------------------------------
class ACamera : public Toshi::TObject
{
public:
	TDECLARE_CLASS( ACamera, Toshi::TObject );

public:
	// The view of the camera (0x98 bytes)
	struct CameraMatrix
	{
		Toshi::TMatrix44 m_mMatrix;            // 0x00
		Toshi::TMatrix44 m_mShakeMatrix;       // 0x40
		TFLOAT           m_fNear;              // 0x80
		TFLOAT           m_fFar;               // 0x84
		TFLOAT           m_fFOV;               // 0x88
		TFLOAT           m_fAspect;            // 0x8C
		TFLOAT           m_fProjectionCentreX; // 0x90
		TFLOAT           m_fProjectionCentreY; // 0x94
	};

	typedef TINT CameraType;
	enum CameraType_ : CameraType
	{
		CT_DEFAULT,
		CT_BLOBFOLLOW,
		CT_SCRIPTED,
		CT_FREE,
		CT_KEYFRAME,
		CT_TYPECOUNT
	};

public:
	ACamera();
	ACamera( CameraType a_eCamType );
	~ACamera();

	// $deBlob: FUNCTION 0058d1c0
	virtual void Update( TFLOAT a_fDeltaTime ) {} // +0x0C

	// $deBlob: FUNCTION 0058cda0
	virtual void Lock( TBOOL a_bLock ) {} // +0x10

	// $deBlob: FUNCTION 0058cd50
	virtual TBOOL IsLocked() { return TFALSE; } // +0x14

	// Called by ACameraManager::DeactivateCameras
	// $deBlob: FUNCTION 0058db90
	virtual void OnSuspend() {} // +0x18

	// Called by ACameraManager::Activate
	// $deBlob: FUNCTION 0058db70
	virtual void Invalidate() {} // +0x1C

	// Called by ACameraManager::Deactivate
	// $deBlob: FUNCTION 0058db80
	virtual void OnDeactivate() {} // +0x20

	// Called by ACameraManager::DestroyCamera
	// $deBlob: FUNCTION 0058dba0
	virtual void OnDestroy() {} // +0x24

	void LookAtPoint( const Toshi::TVector4& a_rvEye, const Toshi::TVector4& a_rvTarget );
	void LookAtPoint( const Toshi::TVector4& a_rvTarget );
	void LookAtDirection( const Toshi::TVector4& a_rvDir );
	void LookAtDirection( const Toshi::TVector4& a_rvDir, const Toshi::TVector4& a_rvUp );
	void RotateAroundWorldUp( TFLOAT a_fRotation );
	void RotateAroundRight( TFLOAT a_fRotation, TFLOAT a_fLimit );
	// NOTE: retail ignores a_rvAxis and rotates around the world up
	void RotateAroundAxis( const Toshi::TVector4& a_rvAxis, TFLOAT a_fRotation );
	// The roll (0), pitch and yaw of the look direction
	void GetAppSideEuler( TFLOAT& a_rfRoll, TFLOAT& a_rfPitch, TFLOAT& a_rfYaw ) const;
	// The point is on the inner side of the 6 view planes
	TBOOL IsPointInsideFrustum( const Toshi::TVector4& a_rvPoint ) const;
	// The camera's right axis in the horizontal plane (forward x world up, normalised)
	void  GetHorizRight( Toshi::TVector4& a_rvHorizRight ) const;
	TBOOL IsInViewCone( const Toshi::TVector4& a_rvPos, TFLOAT a_fSphereRadius ) const;
	// The sphere at a_rvPos (a_fSphereRadius) is in the view cone up to a_fConeLength away
	TBOOL IsInViewCone( const Toshi::TVector4& a_rvPos, TFLOAT a_fSphereRadius, TFLOAT a_fConeLength ) const;

	void SetMatrix( const Toshi::TMatrix44& a_rMatrix );
	void SetShakeMatrix( const Toshi::TMatrix44& a_rShakeMatrix );

	// Copies (at most 6 of) the world planes ARenderer::RenderMainScene rendered this camera with
	void                 SetViewPlanes( const Toshi::TPlane* a_pPlanes, TINT a_iNumPlanes );
	const Toshi::TPlane* GetViewPlanes() const { return m_aViewPlanes; }

	static void ResetCameraMatrix( CameraMatrix& a_rCamMat );

	TBOOL                   IsActive() const { return m_bActive; }
	void                    SetActive( TBOOL a_bActive ) { m_bActive = a_bActive; }
	CameraType              GetCameraType() const { return m_eCamType; }
	CameraMatrix&           GetCameraMatrix() { return m_Matrix; }
	const Toshi::TMatrix44& GetMatrix() const { return m_Matrix.m_mMatrix; }
	Toshi::TMatrix44&       GetShakeMatrix() { return m_Matrix.m_mShakeMatrix; }

	// The camera's up direction (the matrix' second row negated, w = 1)
	// $deBlob: FUNCTION 0056c660
	Toshi::TVector4 GetUp() const
	{
		Toshi::TVector4 vUp = m_Matrix.m_mMatrix.AsBasisVector4( Toshi::BASISVECTOR_UP );
		vUp.w               = 1.0f;
		vUp.Negate3();
		return vUp;
	}

	// The camera's forward direction flattened onto the ground plane
	void GetFlatForward( Toshi::TVector4& a_rOutForward ) const;

	// $deBlob: FUNCTION 0058cb00
	void GetMatrix( Toshi::TMatrix44& a_rOutMatrix ) const { a_rOutMatrix = m_Matrix.m_mMatrix; }

	// $deBlob: FUNCTION 0058cf40
	void SetNearClip( TFLOAT a_fNear )
	{
		TASSERT( a_fNear >= 0.0f && a_fNear < m_Matrix.m_fFar );
		m_Matrix.m_fNear = a_fNear;
	}

public:
	static const TFLOAT          sm_fDefaultFOV; // 60 degrees
	static const Toshi::TVector4 sm_vWorldUp;

protected:
	TBOOL         m_bActive;          // 0x04 updated by ACameraManager (counted in its m_iNumActive)
	TBOOL         m_bUnk05;           // 0x05
	CameraType    m_eCamType;         // 0x08
	Toshi::TPlane m_aViewPlanes[ 6 ]; // 0x0C the world planes of the frustum it was last rendered with (SetViewPlanes)
	CameraMatrix  m_Matrix;           // 0x6C
};
