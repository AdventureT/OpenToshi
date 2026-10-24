#pragma once
#include <Render/TViewport.h>
#include <Render/TCameraObject.h>

class ACamera;

//-----------------------------------------------------------------------------
// Purpose: the viewports of the split screens (the m_pViewportManager of ARenderer): a TViewport and a TCameraObject
// per layout, and the ACamera each used viewport renders (TNULL: the current camera of ACameraManager).
//-----------------------------------------------------------------------------
class AXYZViewportManager
{
public:
	typedef TINT VIEWPORT;
	enum VIEWPORT_ : VIEWPORT
	{
		VIEWPORT_NONE = -1,
		VIEWPORT_FullScreen,
		VIEWPORT_TopLeft,
		VIEWPORT_TopRight,
		VIEWPORT_BottomLeft,
		VIEWPORT_BottomRight,
		VIEWPORT_Top,
		VIEWPORT_Bottom,
		VIEWPORT_Left,
		VIEWPORT_Right,
		VIEWPORT_ThirdLeft,
		VIEWPORT_ThirdMiddle,
		VIEWPORT_ThirdRight,
		VIEWPORT_Count
	};

public:
	AXYZViewportManager();
	~AXYZViewportManager();

	static Toshi::TViewport* CreateViewport( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fWidth, TFLOAT a_fHeight );

	void DisableAllViewports();
	void ClearCameras();
	void RemoveCamera( ACamera* a_pCamera );
	void SetCamera( VIEWPORT a_eViewport, ACamera* a_pCamera );
	void EnableViewport( VIEWPORT a_eViewport, ACamera* a_pCamera );

	void SetViewportUsed( VIEWPORT a_eViewport, TBOOL a_bUsed );
	// Marks a_eViewport used, the others unused first when a_bExclusive
	void UseViewport( VIEWPORT a_eViewport, TBOOL a_bExclusive );
	// The first viewport containing the screen position (VIEWPORT_NONE: none)
	VIEWPORT GetViewportAt( TINT a_iX, TINT a_iY ) const;
	// The ray from the camera of the viewport under the mouse cursor through the cursor (a_rOutOrigin, a_rOutDirection
	// normalised); TFALSE if the cursor isn't in a viewport
	TBOOL GetCursorRay( Toshi::TVector4& a_rOutOrigin, Toshi::TVector4& a_rOutDirection );

	// The sphere is in the view of the camera of a used viewport (APropWaterHydrant::Tick)
	TBOOL IsSphereVisible( const Toshi::TSphere& a_rcSphere ) const;

	// $deBlob: FUNCTION 005c66d0
	Toshi::TViewport* GetViewport( VIEWPORT a_eViewport ) const
	{
		TASSERT( a_eViewport >= VIEWPORT_FullScreen && a_eViewport < VIEWPORT_Count );
		return m_apViewports[ a_eViewport ];
	}

	// $deBlob: FUNCTION 005f02e0
	Toshi::TCameraObject* GetCameraObject( VIEWPORT a_eViewport ) const
	{
		TASSERT( a_eViewport >= VIEWPORT_FullScreen && a_eViewport < VIEWPORT_Count );
		return m_apCameraObjects[ a_eViewport ];
	}

	// $deBlob: FUNCTION 00554d30
	ACamera* GetCamera( VIEWPORT a_eViewport ) const
	{
		TASSERT( a_eViewport >= VIEWPORT_FullScreen && a_eViewport < VIEWPORT_Count );
		return m_apCameras[ a_eViewport ];
	}

	// The viewport rendering a_pCamera (VIEWPORT_NONE: none)
	// $deBlob: FUNCTION 005f1a70
	VIEWPORT GetCameraViewport( ACamera* a_pCamera ) const
	{
		if ( a_pCamera != TNULL )
		{
			for ( TINT i = 0; i < VIEWPORT_Count; i++ )
			{
				if ( m_apCameras[ i ] == a_pCamera )
					return i;
			}
		}

		return VIEWPORT_NONE;
	}

	// The viewport m_uiCurrentViewport selects
	Toshi::TViewport* GetCurrentViewport() const { return m_apViewports[ m_uiCurrentViewport ]; }

	Toshi::TCameraObject* GetCurrentCameraObject() const { return m_apCameraObjects[ m_uiCurrentViewport ]; }

	// The TViewport rendering a_pCamera (TNULL: none)
	// $deBlob: FUNCTION 005f1a40
	Toshi::TViewport* GetCameraTViewport( ACamera* a_pCamera ) const
	{
		for ( TINT i = 0; i < VIEWPORT_Count; i++ )
		{
			if ( m_apCameras[ i ] == a_pCamera )
				return m_apViewports[ i ];
		}

		return TNULL;
	}

	// $deBlob: FUNCTION 0056c6f0
	TBOOL IsViewportUsed( VIEWPORT a_eViewport ) const
	{
		TASSERT( a_eViewport >= VIEWPORT_FullScreen && a_eViewport < VIEWPORT_Count );
		return m_abUsed[ a_eViewport ];
	}

	// $deBlob: FUNCTION 00554ec0
	VIEWPORT GetViewportBeingRendered() const { return m_eRenderingViewport; }
	void     SetViewportBeingRendered( VIEWPORT a_eViewport ) { m_eRenderingViewport = a_eViewport; }

private:
	Toshi::TViewport*     m_apViewports[ VIEWPORT_Count ];     // 0x00
	Toshi::TCameraObject* m_apCameraObjects[ VIEWPORT_Count ]; // 0x30
	ACamera*              m_apCameras[ VIEWPORT_Count ];       // 0x60
	TBOOL                 m_abUsed[ VIEWPORT_Count ];          // 0x90
	TUINT32               m_uiCurrentViewport;                 // 0x9C
	VIEWPORT              m_eRenderingViewport;                // 0xA0
};
