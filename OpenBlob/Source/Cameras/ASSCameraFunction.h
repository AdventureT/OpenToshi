#pragma once
#include <Math/TVector4.h>
#include <Math/TQuaternion.h>
#include <Math/TVector2.h>

//-----------------------------------------------------------------------------
// Purpose: the scripted sequence cameras of the level (SSCamHeader of SSCamera.trb) at 0xC of ACameraManager.
//-----------------------------------------------------------------------------
class ASSCameraFunction
{
public:
	// A key frame of a camera (0x50 bytes)
	struct KeyFrame
	{
		Toshi::TQuaternion m_qOrient;         // 0x00
		Toshi::TVector4    m_vPos;            // 0x10
		TFLOAT             m_fFOV;            // 0x20
		TFLOAT             m_fTime;           // 0x24 the time of the key frame in the camera
		TFLOAT             m_fUnk28;          // 0x28 path cameras: the time skipped at the start/end; GetPathKeyFrame: the time since the key frame
		TFLOAT             m_fTension;        // 0x2C the weight of the tangents (GetPathKeyFrame)
		TFLOAT             m_fFade;           // 0x30 the fade to black (AKeyFrameCamera::UpdateFade)
		TFLOAT*            m_pPosCurve;       // 0x34 the easing of the position: (time, value) pairs
		TFLOAT*            m_pOrientCurve;    // 0x38 the easing of the orientation: (time, value) pairs
		TINT               m_iNumPosCurve;    // 0x3C
		TINT               m_iNumOrientCurve; // 0x40
		TINT               m_iUsePrev;        // 0x44 0: the previous key frame isn't used for the tangents
		TINT               m_iUseNext;        // 0x48 0: the key frame after the next one isn't used for the tangents
		TUINT8             m_aUnk4C[ 0x04 ];  // 0x4C
	};

	// A camera (0x50 bytes)
	struct Camera
	{
		Toshi::TVector4 m_vLineStart;     // 0x00 the line the followed position is projected on
		Toshi::TVector4 m_vLineEnd;       // 0x10
		TINT            m_iID;            // 0x20
		KeyFrame*       m_pKeyFrames;     // 0x24
		TINT            m_iCount;         // 0x28
		TINT            m_iNextCamera;    // 0x2C the camera played after this one (-1: none)
		TUINT32         m_uiFlags;        // 0x30 CameraFlag
		TINT            m_iHelpTopic;     // 0x34 the help topic shown after it (APlayerManager::PlaySSCamera keeps it in BlobData::m_iHelpTopic)
		TINT16          m_iHelpString;    // 0x38 the help string shown after it (BlobData::m_iHelpString)
		TUINT8          m_aUnk3A[ 0x02 ]; // 0x3A
		TFLOAT          m_fPosRate;       // 0x3C path cameras: the speed the position follows the key frames
		TFLOAT          m_fOrientRate;    // 0x40 path cameras: the speed the orientation follows the key frames
		TFLOAT          m_fSnapDistSq;    // 0x44 path cameras: closer than this the position snaps
		TFLOAT          m_fSnapOrient;    // 0x48 path cameras: closer than this the orientation snaps
		TUINT8          m_aUnk4C[ 0x04 ]; // 0x4C
	};

	// The flags of a camera (Camera::m_uiFlags)
	enum CameraFlag : TUINT32
	{
		CF_PATH = 0x1, // interpolated as a path (GetPathKeyFrame), else key frame to key frame (GetKeyFrame)
		CF_HELP = 0x2, // triggers the letterbox helps (APropTriggerHelp, help type 3) around the camera
	};

	struct Header
	{
		Camera* m_pCameras;    // 0x00
		TINT    m_iNumCameras; // 0x04
	};

	// Flags of the spline cameras of ANedCameraFunction that choose the interpolation
	enum InterpolationFlag : TUINT32
	{
		IF_PATH        = 0x80000,
		IF_LINE        = 0x100000,
		IF_LINE_LOOPED = 0x200000,
	};

public:
	// $deBlob: FUNCTION 00594df0
	ASSCameraFunction()
	    : m_pHeader( TNULL )
	{}

	// $deBlob: FUNCTION 00594e00
	~ASSCameraFunction() { m_pHeader = TNULL; }

	// $deBlob: FUNCTION 00594e40
	// $deBlob: FUNCTION 0058e6c0
	void Clear() { m_pHeader = TNULL; }

	// Reads the cameras of the level
	void Load();

	// Returns the camera with the ID or TNULL
	Camera* FindCamera( TINT a_iID ) const;

	// The number of cameras of the level
	TINT GetNumCameras() const;

	// Position, orientation and FOV of a camera for the followed position (a_uiFlags: InterpolationFlag)
	static void GetCameraTransform( const Camera* a_pCam, const Toshi::TVector4& a_rvLineStart, const Toshi::TVector4& a_rvLineEnd, const Toshi::TVector4& a_rvFollowPos, const Toshi::TVector4& a_rvViewPos, Toshi::TVector4& a_rvPos, Toshi::TQuaternion& a_rqOrient, TFLOAT& a_rfFOV, TFLOAT a_fRadius, TFLOAT a_fMinOffset, TFLOAT a_fOffsetRadiantAngle, TFLOAT a_fLineRadius, TUINT32 a_uiFlags );

	Header* GetHeader() const { return m_pHeader; }

	// The key frame of a camera at a_fTime, interpolated from key frame to key frame (Catmull-Rom position, slerped
	// orientation); the key frames around it in the out pointers. TFALSE if the camera has no key frames.
	static TBOOL GetKeyFrame( const Camera* a_pCam, TFLOAT a_fTime, KeyFrame* a_pOut, const KeyFrame** a_ppCurrent, const KeyFrame** a_ppPrev, const KeyFrame** a_ppNext, const KeyFrame** a_ppNextNext );

	// The key frame of a path camera (CF_PATH) at a_fTime (Hermite curves with the eased times); TFALSE if the camera
	// has no key frames
	static TBOOL GetPathKeyFrame( const Camera* a_pCam, TFLOAT a_fTime, KeyFrame* a_pOut, const KeyFrame** a_ppCurrent, const KeyFrame** a_ppNext, const KeyFrame** a_ppNextNext );

	// The key frame a_iKeyFrame of a camera (TNULL if out of range)
	static KeyFrame* GetKeyFrame( const Camera* a_pCam, TINT a_iKeyFrame );

	// The time of the last key frame (0 without key frames)
	static TFLOAT GetTotalTime( const Camera* a_pCam );

	// The number of key frames (0 without a camera)
	static TINT GetNumKeyFrames( const Camera* a_pCam );

public:
	// Also used by ABlobFollowCamera.cpp's MoveAlongSpline
	static void CatmullRom( Toshi::TVector4& a_rvOut, const Toshi::TVector4& a_rvP0, const Toshi::TVector4& a_rvP1, const Toshi::TVector4& a_rvP2, const Toshi::TVector4& a_rvP3, TFLOAT a_fT );
	// The same in 2D
	static void CatmullRom( Toshi::TVector2& a_rvOut, const Toshi::TVector2& a_rvP0, const Toshi::TVector2& a_rvP1, const Toshi::TVector2& a_rvP2, const Toshi::TVector2& a_rvP3, TFLOAT a_fT );

private:
	// The tangent at a_rvP1 from the half differences to its neighbours, each over its time, scaled by a_fScale (two
	// identical copies). Retail divides the first difference twice and never the second
	static void CalcTangent( Toshi::TVector4& a_rvOut, const Toshi::TVector4& a_rvP0, const Toshi::TVector4& a_rvP1, const Toshi::TVector4& a_rvP2, TFLOAT a_fTime0, TFLOAT a_fTime1, TFLOAT a_fScale );
	static void InterpolateAlongLine( const Camera* a_pCam, const Toshi::TVector4& a_rvLineStart, const Toshi::TVector4& a_rvLineEnd, const Toshi::TVector4& a_rvFollowPos, const Toshi::TVector4& a_rvViewPos, Toshi::TVector4& a_rvPos, Toshi::TQuaternion& a_rqOrient, TFLOAT& a_rfFOV, TFLOAT a_fLineRadius, TBOOL a_bLooped );
	static void InterpolateAlongPath( const Camera* a_pCam, const Toshi::TVector4& a_rvFollowPos, const Toshi::TVector4& a_rvViewPos, Toshi::TVector4& a_rvPos, Toshi::TQuaternion& a_rqOrient, TFLOAT& a_rfFOV, TFLOAT a_fRadius, TFLOAT a_fMinOffset, TFLOAT a_fOffsetRadiantAngle );
	static void InterpolateKeyFrames( const Camera* a_pCam, Toshi::TVector4& a_rvPos, Toshi::TQuaternion& a_rqOrient, TFLOAT& a_rfFOV, TINT a_iFrame1, TINT a_iFrame2, TBOOL a_bLooped );
	static void InterpolateOrientation( const Camera* a_pCam, const Toshi::TVector4& a_rvPos, Toshi::TQuaternion& a_rqOrient, TFLOAT a_fT, TINT a_iFrame0, TINT a_iFrame1, TINT a_iFrame2, TINT a_iFrame3 );

	static void SetFromKeyFrame( const KeyFrame& a_rKeyFrame, Toshi::TVector4& a_rvPos, Toshi::TQuaternion& a_rqOrient, TFLOAT& a_rfFOV );

	// The eased value of a curve of a_iCount (time, value) pairs at a_fTime (0..1)
	static TFLOAT EvaluateCurve( const TFLOAT* a_pCurve, TINT a_iCount, TFLOAT a_fTime );

	// The orientation of a path camera between a_pFrame1 and a_pFrame2 (Hermite of the up and forward axes)
	static void InterpolatePathOrientation( const Camera* a_pCam, Toshi::TQuaternion& a_rqOut, const KeyFrame* a_pFrame0, const KeyFrame* a_pFrame1, const KeyFrame* a_pFrame2, const KeyFrame* a_pFrame3, TFLOAT a_fT );

	// A Hermite curve through the points of a_pvP1 and a_pvP2 with the tangents from a_pvP0 and a_pvP3, the key frame
	// times and tensions (each component on its own)
	static void HermiteKeyFrames( Toshi::TVector4& a_rvOut, const Toshi::TVector4* a_pvP0, const Toshi::TVector4* a_pvP1, const Toshi::TVector4* a_pvP2, const Toshi::TVector4* a_pvP3, TFLOAT a_fTime0, TFLOAT a_fTime1, TFLOAT a_fTime2, TFLOAT a_fTime3, TFLOAT a_fTension0, TFLOAT a_fTension1, TFLOAT a_fTension2, TFLOAT a_fTension3, TFLOAT a_fT );

	// The cubic Hermite of two points and their tangents (by value)
	static void Hermite( Toshi::TVector4& a_rvOut, Toshi::TVector4 a_vP1, Toshi::TVector4 a_vP2, Toshi::TVector4 a_vT1, Toshi::TVector4 a_vT2, TFLOAT a_fT );

private:
	Header* m_pHeader; // 0x0
};
