#pragma once
#include "Math/Math.h"
#include "Math/TSphere.h"
#include "Toshi/TStack.h"

TOSHI_NAMESPACE_START

class TRender;
class TSkeletonInstance;
class TCameraObject;

using TLightID = TINT8;

struct TLightIDList
{
	static TCONSTEXPR_VAR TINT MAX_NUM_LIGHTS = 4;

	void Add( TLightID a_iLightId );
	void Reset();

	TLightID& operator[]( TINT a_iIndex );

	TLightID aIDs[ MAX_NUM_LIGHTS ];
};

typedef TUINT32 WORLDPLANE;
enum WORLDPLANE_ : WORLDPLANE
{
	WORLDPLANE_LEFT,
	WORLDPLANE_RIGHT,
	WORLDPLANE_BOTTOM,
	WORLDPLANE_TOP,
	WORLDPLANE_NEAR,
	WORLDPLANE_FAR,
	WORLDPLANE_NUMOF
};

using TFrustum = TPlane[ WORLDPLANE_NUMOF ];

//-----------------------------------------------------------------------------
// de Blob's TRenderContext. Barnyard's layout with a 7th float in PROJECTIONPARAMS (everything after 0x48 is 4 bytes
// further).
//-----------------------------------------------------------------------------
class TRenderContext
{
public:
	typedef TUINT32 FLAG;
	enum FLAG_ : FLAG
	{
		FLAG_DIRTY                  = BITFLAG( 0 ),
		FLAG_FOG                    = BITFLAG( 1 ),
		FLAG_HAS_MODELWORLDMATRIX   = BITFLAG( 2 ),
		FLAG_HAS_VIEWWORLDMATRIX    = BITFLAG( 3 ),
		FLAG_UNK4                   = BITFLAG( 4 ),
		FLAG_UNK5                   = BITFLAG( 5 ),
		FLAG_HAS_WORLDPLANES        = BITFLAG( 6 ),
		FLAG_UNK7                   = BITFLAG( 7 ), // has the world guard planes
		FLAG_DIRTY_WORLDMODELMATRIX = BITFLAG( 8 ),
		FLAG_DIRTY_VIEWMODELMATRIX  = BITFLAG( 9 ),
		FLAG_DISTANCEFADE1          = BITFLAG( 10 ), // closer than the first fade end, see TSceneObject::Render (GetUnk16C().w)
		FLAG_DISTANCEFADE2          = BITFLAG( 11 ), // closer than the second fade end, see TSceneObject::Render (m_fUnk3AC)
	};

	// One bit per plane of the frustum (WORLDPLANE) that still has to be tested
	using CLIPFLAGS = TUINT32;
	enum CLIPFLAGS_ : CLIPFLAGS
	{
		CLIPFLAGS_NONE   = 0,
		CLIPFLAGS_LEFT   = BITFLAG( WORLDPLANE_LEFT ),
		CLIPFLAGS_RIGHT  = BITFLAG( WORLDPLANE_RIGHT ),
		CLIPFLAGS_BOTTOM = BITFLAG( WORLDPLANE_BOTTOM ),
		CLIPFLAGS_TOP    = BITFLAG( WORLDPLANE_TOP ),
		CLIPFLAGS_NEAR   = BITFLAG( WORLDPLANE_NEAR ),
		CLIPFLAGS_FAR    = BITFLAG( WORLDPLANE_FAR ),
		CLIPFLAGS_ALL    = CLIPFLAGS_LEFT | CLIPFLAGS_RIGHT | CLIPFLAGS_BOTTOM | CLIPFLAGS_TOP | CLIPFLAGS_NEAR | CLIPFLAGS_FAR,
	};

	typedef TUINT32 CameraMode;
	enum CameraMode_ : CameraMode
	{
		CameraMode_Perspective,
		CameraMode_Orthographic,
	};

	struct VIEWPORTPARAMS
	{
		TFLOAT fX;
		TFLOAT fY;
		TFLOAT fWidth;
		TFLOAT fHeight;
		TFLOAT fMinZ;
		TFLOAT fMaxZ;

		// $deBlob: FUNCTION 006935d0
		void Set( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fWidth, TFLOAT a_fHeight, TFLOAT a_fMinZ, TFLOAT a_fMaxZ )
		{
			fX      = a_fX;
			fY      = a_fY;
			fWidth  = a_fWidth;
			fHeight = a_fHeight;
			fMinZ   = a_fMinZ;
			fMaxZ   = a_fMaxZ;
		}
	};

	struct PROJECTIONPARAMS
	{
		TVector2 m_Centre;
		TVector2 m_Proj;
		TFLOAT   m_fNearClip;
		TFLOAT   m_fFarClip;
		TFLOAT   m_fUnk; // 0x18, de Blob only (1.0 by the constructor)

		// $deBlob: FUNCTION 00693580
		void Set( TFLOAT a_fCentreX, TFLOAT a_fCentreY, TFLOAT a_fProjX, TFLOAT a_fProjY, TFLOAT a_fNearClip, TFLOAT a_fFarClip, TFLOAT a_fUnk )
		{
			m_Centre.x  = a_fCentreX;
			m_Centre.y  = a_fCentreY;
			m_Proj.x    = a_fProjX;
			m_Proj.y    = a_fProjY;
			m_fNearClip = a_fNearClip;
			m_fFarClip  = a_fFarClip;
			m_fUnk      = a_fUnk;
		}

		void SetFromFOV( TFLOAT a_fViewportWidth, TFLOAT a_fViewportHeight, TFLOAT a_fFOV, TFLOAT a_fNearPlane, TFLOAT a_fFarPlane );
	};

public:
	void SetDirty( TBOOL a_bDirty );
	void SetFlag( FLAG a_eFlag, TBOOL a_bEnable );

public:
	TRenderContext( TRender* pRender );
	virtual ~TRenderContext();

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual void SetModelViewMatrix( const TMatrix44& a_rMatrix );
	virtual void SetWorldViewMatrix( const TMatrix44& a_rMatrix );
	virtual void Update() = 0;

	void ClearLightIDs();
	void AddLight( TLightID a_iLightId );

	const PROJECTIONPARAMS& GetProjectionParameters() const { return m_oProjParams; }
	void                    SetProjectionParameters( const PROJECTIONPARAMS& a_rParams );

	const VIEWPORTPARAMS& GetViewportParameters() const { return m_oViewportParams; }
	void                  SetViewportParameters( const VIEWPORTPARAMS& a_rParams );

	CameraMode GetProjType() { return m_eCameraMode; }
	void       SetProjType( CameraMode a_eCameraMode );

	void EnableFog( TBOOL a_bEnable );
	void SetFogColor( const TVector4& a_rcFogColor );
	void SetFogDistance( TFLOAT a_fStart, TFLOAT a_fEnd );

	TFLOAT GetX() const { return m_oViewportParams.fX; }
	TFLOAT GetY() const { return m_oViewportParams.fY; }
	TFLOAT GetWidth() const { return m_oViewportParams.fWidth; }
	TFLOAT GetHeight() const { return m_oViewportParams.fHeight; }

	TBOOL IsFogEnabled() const { return m_eFlags & FLAG_FOG; }
	FLAG  GetFlags() const { return m_eFlags; }

	const TVector4& GetFogColour() const { return m_FogColor; }
	TFLOAT          GetFogDistanceStart() const { return m_fFogDistanceStart; }
	TFLOAT          GetFogDistanceEnd() const { return m_fFogDistanceEnd; }
	// $deBlob: FUNCTION 00693520
	// $Barnyard: FUNCTION 006ce980
	TBOOL IsDirty() const { return m_eFlags & FLAG_DIRTY; }

	TSkeletonInstance* GetSkeletonInstance() const { return m_pCurrentSkeletonInstance; }
	// $deBlob: FUNCTION 0054f2a0
	void               SetSkeletonInstance( TSkeletonInstance* a_pSkeletonInstance ) { m_pCurrentSkeletonInstance = a_pSkeletonInstance; }

	TUINT GetClipFlags() const;
	TUINT SetClipFlags( TUINT a_uiClipFlags );

	const TPlane* GetWorldPlanes();
	const TPlane* GetWorldGuardPlanes();

	// Copy the first a_iNumPlanes planes to a_pOutPlanes
	void GetViewPlanes( TPlane* a_pOutPlanes, TINT a_iNumPlanes ) const;
	void GetGuardPlanes( TPlane* a_pOutPlanes, TINT a_iNumPlanes ) const;
	void GetWorldPlanes( TPlane* a_pOutPlanes, TINT a_iNumPlanes );
	void GetWorldGuardPlanes( TPlane* a_pOutPlanes, TINT a_iNumPlanes );

	const TMatrix44& GetViewWorldMatrix();
	const TMatrix44& GetWorldModelMatrix();
	const TMatrix44& GetModelWorldMatrix();
	const TMatrix44& GetViewModelMatrix();
	// $deBlob: FUNCTION 0054f020
	TMatrix44&       GetModelViewMatrix() { return m_oModelViewMatrix; }
	TMatrix44&       GetWorldViewMatrix() { return m_oWorldViewMatrix; }

	// $deBlob: FUNCTION 00693570
	const TPlane* GetFrustumPlanes() const { return m_aFrustumPlanes1; }
	// $deBlob: FUNCTION 00693560
	const TPlane* GetGuardPlanes() const { return m_aGuardPlanes; }
	TUINT         GetGuardFlags() const { return m_eGuardFlags; }

	void           SetCameraObject( TCameraObject* a_pCameraObject ) { m_pCurrentCameraObject = a_pCameraObject; }
	// $deBlob: FUNCTION 00554c50
	TCameraObject* GetCameraObject() const { return m_pCurrentCameraObject; }

	// $deBlob: FUNCTION 0054efa0
	TFLOAT GetAlphaBlend() const { return m_fAlphaBlend; }
	void   SetAlphaBlend( TFLOAT val ) { m_fAlphaBlend = val; }

	// $deBlob: FUNCTION 0054f030
	TFLOAT GetShadeCoeff() const { return m_fShadeCoeff; }
	// $deBlob: FUNCTION 0054f220
	void SetShadeCoeff( TFLOAT a_fMag )
	{
		TASSERT( a_fMag >= 0.0f && a_fMag <= 1.0f );
		m_fShadeCoeff = a_fMag;
	}

	void SetAmbientColour( const TVector4& a_rcColour ) { m_AmbientColor = a_rcColour; }

	// NOTE: an instance of it lives in the game code
	// $Barnyard: FUNCTION 0041ce00
	void SetAmbientColour( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue ) { m_AmbientColor.Set( a_fRed, a_fGreen, a_fBlue, 0.0f ); }

	// $deBlob: FUNCTION 0054ef90
	const TVector4& GetAmbientColour() const { return m_AmbientColor; }
	TVector4&       GetUnk16C() { return m_vUnk16C; }
	// $deBlob: FUNCTION 0054f040
	TRender* GetRenderer() const { return m_pRenderer; }
	// $deBlob: FUNCTION 0054f290
	void SetRenderer( TRender* a_pRenderer ) { m_pRenderer = a_pRenderer; }
	TFLOAT          GetUnk3AC() const { return m_fUnk3AC; }
	void            SetUnk3AC( TFLOAT a_fValue ) { m_fUnk3AC = a_fValue; }

	static void ComputePerspectiveProjection( TMatrix44& a_rOutProjection, const VIEWPORTPARAMS& a_rViewportParams, const PROJECTIONPARAMS& a_rProjParams );
	static void ComputeOrthographicProjection( TMatrix44& a_rOutProjection, const VIEWPORTPARAMS& a_rViewportParams, const PROJECTIONPARAMS& a_rProjParams );

	static void ComputePerspectiveFrustum( TFrustum& a_rcFrustum, const VIEWPORTPARAMS& a_rViewportParams, const PROJECTIONPARAMS& a_rProjParams );
	static void ComputeOrthographicFrustum( TFrustum& a_rcFrustum, const VIEWPORTPARAMS& a_rViewportParams, const PROJECTIONPARAMS& a_rProjParams );

	static TBOOL CullSphereToFrustumSimple( const TSphere& a_rSphere, const TPlane* a_pPlanes, TINT a_iNumPlanes );
	static TINT  CullSphereToFrustum( const TSphere& a_rSphere, const TPlane* a_pPlanes, TINT a_iClipFlags, TINT a_iClipFlagsMask );
	static TINT  CullAABoxToFrustum( const TVector3* a_pBox, const TPlane* a_pPlanes, TINT a_iClipFlags, TINT a_iClipFlagsMask );

#ifndef TOSHI_SDK
protected:
#endif

	TRender*           m_pRenderer;                // 0x0004
	FLAG               m_eFlags;                   // 0x0008
	CLIPFLAGS          m_eClipFlags;               // 0x000C
	TUINT              m_eGuardFlags;              // 0x0010
	CameraMode         m_eCameraMode;              // 0x0014
	VIEWPORTPARAMS     m_oViewportParams;          // 0x0018
	PROJECTIONPARAMS   m_oProjParams;              // 0x0030
	TSkeletonInstance* m_pCurrentSkeletonInstance; // 0x004C
	TMatrix44          m_oModelViewMatrix;         // 0x0050
	TMatrix44          m_oWorldViewMatrix;         // 0x0090
	TMatrix44          m_oModelWorldMatrix;        // 0x00D0
	TMatrix44          m_oViewWorldMatrix;         // 0x0110
	TVector4           m_AmbientColor;             // 0x0150
	TVector4           m_FogColor;                 // 0x0160
	TVector4           m_vUnk16C;                  // 0x0170 (Barnyard 0x16C), copied by AGUI2ModelRenderer::CreateFromViewport, w is a distance fade set by TSceneObject::Render (FLAG_DISTANCEFADE1)
	TVector4           m_vUnk17C;                  // 0x0180
	TVector3           m_vUnk18C;                  // 0x0190
	TFLOAT             m_fUnk198;                  // 0x019C
	TFLOAT             m_fUnk19C;                  // 0x01A0

	TFLOAT   m_fFogDistanceStart; // 0x01A4
	TFLOAT   m_fFogDistanceEnd;   // 0x01A8
	TFrustum m_aFrustumPlanes1;   // 0x01AC
	TFrustum m_aWorldPlanes;      // 0x020C
	TFrustum m_aGuardPlanes;      // 0x026C, a copy of m_aFrustumPlanes1 on PC

	TFrustum m_aWorldGuardPlanes; // 0x02CC

	TMatrix44 m_oWorldModelMatrix; // 0x032C
	TMatrix44 m_oViewModelMatrix;  // 0x036C
	TFLOAT    m_fAlphaBlend;       // 0x03AC
	TFLOAT    m_fUnk3AC;           // 0x03B0 (Barnyard 0x3AC), distance fade set by TSceneObject::Render (FLAG_DISTANCEFADE2)

	TFLOAT m_fShadeCoeff; // 0x03B4

	TBOOL PADDING4; // 0x03B8

	TLightIDList   m_oLightIds;            // 0x03B9
	TCameraObject* m_pCurrentCameraObject; // 0x03C0
};

TOSHI_NAMESPACE_END
