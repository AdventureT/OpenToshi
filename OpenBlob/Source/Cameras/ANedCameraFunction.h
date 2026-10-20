#pragma once
#include <Math/TQuaternion.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: the camera regions of the level (CameraHeader of NEDCamera.trb, made in the NED level editor) at 0x4 of
// ACameraManager (8 bytes). A region is a box or a vertical cylinder; it holds the follow distances of the blob
// camera in it, its fixed cameras (landmarks...), and two more lists.
//-----------------------------------------------------------------------------
class ANedCameraFunction
{
public:
	// Region/fixed camera flags (m_uiFlags/m_iFLAGS); the categories ABlobFollowCamera searches are the low bits
	enum RegionFlag : TUINT32
	{
		RS_LANDMARK = 0x00000002,
		RS_BLEND    = 0x00002000, // the distances blend into the region's
		RS_CYLINDER = 0x00008000, // the shape is a vertical cylinder (else an oriented box)
		RS_ENABLED  = 0x00010000,
		RS_UNK20000 = 0x00020000,
		RS_UNK40000 = 0x00040000,
	};

	// The shape of a region
	struct Shape
	{
		Toshi::TVector4 m_vPosition;       // 0x00 cylinder: its top
		Toshi::TVector4 m_avAxes[ 3 ];     // 0x10 box: its axes; cylinder: m_avAxes[ 0 ].x the radius, .y the height
		TFLOAT          m_afHalfSize[ 3 ]; // 0x40 box: the half extents along the axes
	};

	// A fixed camera of a region (0x60 bytes)
	struct FixedCam
	{
		// m_iFLAGS
		enum Flag : TUINT32
		{
			FixedCam_Vista = 0x40, // a vista of the level (ABlobVistasMenu, ABlobFollowCamera's category 0x40)
		};

		Toshi::TQuaternion m_qOrientation;   // 0x00
		Toshi::TVector4    m_vPosition;      // 0x10
		TFLOAT             m_fFOV;           // 0x20
		TFLOAT             m_fFOVSpeed;      // 0x24
		TFLOAT             m_fStartDelay;    // 0x28 before a new one takes over
		TFLOAT             m_fHoldTime;      // 0x2C the least time it's kept
		TFLOAT             m_fViewAngle;     // 0x30 the blob must be within this angle of it (around the target), 0 or >= PI: any
		TUINT32            m_iFLAGS;         // 0x34
		TINT16             m_iMaxUses;       // 0x38 -1: no limit
		TINT16             m_iNumUses;       // 0x3A
		TINT16             m_iVista;         // 0x3C the vista index of a FixedCam_Vista camera (ABlobVistasMenu)
		TINT16             m_iUnk3E;         // 0x3E (APlayerManager::GetFixedCamUnk3E)
		void**             m_ppInstances;    // 0x40 terrain instance indices, made pointers by ResolveInstances
		TINT               m_iNumInstances;  // 0x44
		TUINT8             m_aUnk48[ 0x18 ]; // 0x48
	};

	// An entry of the second list of a region (0x10 bytes)
	struct SSCamTrigger
	{
		TUINT32 m_iSSCam;   // 0x0
		TUINT32 m_uiFlags;  // 0x4
		TINT    m_iMaxUses; // 0x8 -1: no limit
		TINT    m_iCount;   // 0xC
	};

	// A spline (scripted sequence) camera of a region (0x38 bytes)
	struct SplineCam
	{
		TINT    m_iSSCam;         // 0x00 the ASSCameraFunction camera
		TUINT32 m_uiFlags;        // 0x04
		TFLOAT  m_fRadius;        // 0x08 ASSCameraFunction::IF_PATH: radius around the followed position
		TFLOAT  m_fMinOffset;     // 0x0C IF_PATH: least offset along the path
		TFLOAT  m_fOffsetAngle;   // 0x10 IF_PATH: offset angle (radians, [0, HALF_PI))
		TFLOAT  m_fBlendSpeed;    // 0x14 of the followed position
		TFLOAT  m_fLookSpeed;     // 0x18 0x2000 cameras: the speed the view blends to it (<= 0: 2 or none)
		TFLOAT  m_fFOVSpeed;      // 0x1C
		TFLOAT  m_fStartDelay;    // 0x20 before a new one takes over
		TFLOAT  m_fHoldTime;      // 0x24 the least time it's kept
		TUINT8  m_aUnk28[ 0x10 ]; // 0x28
	};

	// A region (0x34 bytes)
	struct Region
	{
		Shape*        m_pShape;            // 0x00
		FixedCam*     m_pFixedCams;        // 0x04
		SSCamTrigger* m_pSSCamTriggers;    // 0x08
		SplineCam*    m_pSplineCams;       // 0x0C
		TFLOAT        m_fCameraMinDist;    // 0x10
		TFLOAT        m_fCameraIdealDist;  // 0x14
		TFLOAT        m_fCameraMaxDist;    // 0x18
		TUINT32       m_uiData;            // 0x1C
		TUINT32       m_uiFlags;           // 0x20 RegionFlag
		TINT          m_iID;               // 0x24
		TINT          m_iNumFixedCams;     // 0x28
		TINT          m_iNumSSCamTriggers; // 0x2C
		TINT          m_iNumSplineCams;    // 0x30
	};

	struct Header
	{
		Region* m_pRegions;    // 0x0
		TINT    m_iNumRegions; // 0x4
	};

public:
	// $deBlob: FUNCTION 00593a00
	ANedCameraFunction()
	    : m_pHeader( TNULL ), m_pCurrentRegion( TNULL )
	{}

	// $deBlob: FUNCTION 00593a10
	~ANedCameraFunction()
	{
		m_pHeader        = TNULL;
		m_pCurrentRegion = TNULL;
	}

	// $deBlob: FUNCTION 00593a80
	// $deBlob: FUNCTION 0058e6a0
	void Clear()
	{
		m_pHeader        = TNULL;
		m_pCurrentRegion = TNULL;
	}

	// Reads the regions of the level and enables them all
	void Load();

	// The region with a_iID, TNULL if there's none
	Region* FindRegion( TINT a_iID ) const;

	// Enables/disables the region with a_iID (enabling resets the uses of its RS_UNK40000 fixed cameras)
	void SetRegionEnabled( TINT a_iID, TBOOL a_bEnabled );

	// Enables/disables every region
	void SetAllRegionsEnabled( TBOOL a_bEnabled );

	// Resets the uses of the fixed cameras and the counts of the SSCamTrigger entries of every region
	void ResetUses();

	// The last enabled region with all of a_uiFlags containing a_rvPos (the smallest cylinder for cylinders)
	Region* FindRegionAt( const Toshi::TVector4& a_rvPos, TUINT32 a_uiFlags ) const;

	// The last unused SSCamTrigger entry with a_uiFlags of the region at a_rvPos
	SSCamTrigger* FindSSCamTriggerAt( const Toshi::TVector4& a_rvPos, TUINT32 a_uiFlags ) const;

	// The vista camera a_iVista (FixedCam_Vista), TNULL if there's none
	FixedCam* FindVistaCam( TINT a_iVista ) const;

	// The last spline camera with a_uiFlags of the region at a_rvPos
	SplineCam* FindSplineCamAt( const Toshi::TVector4& a_rvPos, TUINT32 a_uiFlags ) const;

	// Makes the region with a_iID the current one (a cutscene region, e.g. of APropGrassBomb), TNULL if a_iID < 0; an
	// unknown ID keeps the current one
	void SetCurrentRegion( TINT a_iID );

	// The last unused SSCamTrigger entry of m_pCurrentRegion
	SSCamTrigger* FindCurrentSSCamTrigger() const;

	// The closest usable fixed camera with a_uiFlags of the region at a_rvTarget that sees a_rvBlobPos
	FixedCam* FindFixedCamAt( const Toshi::TVector4& a_rvTarget, const Toshi::TVector4& a_rvBlobPos, TUINT32 a_uiFlags ) const;

	// The same in m_pCurrentRegion with its flags
	FixedCam* FindCurrentFixedCam( const Toshi::TVector4& a_rvTarget, const Toshi::TVector4& a_rvBlobPos ) const;

	// The closest usable fixed camera with a_uiFlags of a_pRegion that sees a_rvBlobPos from a_rvTarget
	static FixedCam* FindFixedCam( const Toshi::TVector4& a_rvTarget, const Toshi::TVector4& a_rvBlobPos, Region* a_pRegion, TUINT32 a_uiFlags );

	// Is a_rvBlobPos within a_fAngle (around the vertical) of the direction from a_rvTarget to a_rvCamPos?
	// The 8 corners of the box at a_rvCentre with the axes a_rvAxisX/Y/Z and the half extents a_pfHalfExtents[ 3 ]
	// (bottom four, then top four)
	static void  GetBoxCorners( Toshi::TVector4* a_pOutCorners, const Toshi::TVector4& a_rvCentre, const Toshi::TVector4& a_rvAxisX, const Toshi::TVector4& a_rvAxisY, const Toshi::TVector4& a_rvAxisZ, const TFLOAT* a_pfHalfExtents );
	static TBOOL IsInViewAngle( const Toshi::TVector4& a_rvBlobPos, const Toshi::TVector4& a_rvCamPos, const Toshi::TVector4& a_rvTarget, TFLOAT a_fAngle );

	// Is a_rvPos inside a_rRegion's shape?
	static TBOOL Contains( const Region& a_rRegion, const Toshi::TVector4& a_rvPos );

	// $deBlob: FUNCTION 00593370
	Header* GetHeader() const { return m_pHeader; }

private:
	// The terrain instance indices of the fixed cameras become pointers
	void ResolveInstances();

	// a_iIndex's region, or (a cylinder) the smallest enabled cylinder with a_uiFlags containing a_rvPos
	Region* GetSmallestCylinder( TINT a_iIndex, const Toshi::TVector4& a_rvPos, TUINT32 a_uiFlags ) const;

private:
	Header* m_pHeader;        // 0x0
	Region* m_pCurrentRegion; // 0x4
};
