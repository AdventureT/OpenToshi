#pragma once
#include "Fx/AEnvironmentFX.h"

#include <Math/TVector2.h>
#include <Math/TVector4.h>
#include <Render/TWorldVIS.h>
#include <Render/TOrderTable.h>

#include <d3d11.h>

class ACamera;

namespace Toshi
{
class TModelInstance;
}

//-----------------------------------------------------------------------------
// Purpose: the sky of a level: a gradient quad drawn behind everything before the sky order table is flushed, and the
// cloud models (sky_base, sky_cloudsbig, sky_cloudssmall) around the camera, slowly rotating; "skyspace" levels also
// get star models. Owned by AEnvironmentFX, which sets its colours.
//-----------------------------------------------------------------------------
class ASky
{
public:
	static TCONSTEXPR_VAR TINT MAX_RENDER_CALLS = 4;
	static TCONSTEXPR_VAR TINT NUM_LAYERS       = 3;

	// The order table of the cloud materials (its pre flush callback draws the gradient)
	static TCONSTEXPR_VAR TINT ORDERTABLE_SKY = 9;

public:
	ASky();
	~ASky();

	// Queues the gradient for a_pCamera and renders the cloud models around it
	void Render( ACamera* a_pCamera );
	void Update( TFLOAT a_fDeltaTime );
	void Reset();

	// Sets the colours of the gradient and the layers (blending from the current ones by a_fBlend)
	void SetColours( TFLOAT a_fBlend, const AEnvFXColours& a_rSky, const AEnvFXColours& a_rCloud1, const AEnvFXColours& a_rCloud2, const AEnvFXColours& a_rCloud3 );

	void CreateStars();
	void StartSkySpace( TFLOAT a_fCloudSpeed, TFLOAT a_fCloudDescendTime, TFLOAT a_fStarsHeight, TFLOAT a_fStarsSpeed, TFLOAT a_fStarsTime );

	void SetCloudSpeed( TFLOAT a_fSpeed ) { m_fCloudSpeed = a_fSpeed; }

private:
	void RenderGradient( ACamera* const* a_ppCamera );

	static void __stdcall OnPreFlush( Toshi::TOrderTable* a_pOrderTable, void* a_pSky );
	static void __stdcall RenderCellMesh( Toshi::TTMDWin::TRBWorldCellMesh* a_pCellMesh, Toshi::TWorldVis::RenderData* a_pRenderData );

public:
	static TBOOL s_bHideSky;      // (no writer)
	static TBOOL s_bHideGradient; // (no writer)
	static TBOOL s_bHideClouds;   // (no writer)

private:
	Toshi::TVector2        m_avCorners[ 4 ];                    // 0x000 of the gradient quad (triangle strip)
	TUINT32                m_uiTopColour;                       // 0x020 ARGB
	TUINT32                m_uiBottomColour;                    // 0x024 ARGB
	Toshi::TModelInstance* m_apClouds[ NUM_LAYERS ];            // 0x028
	AEnvFXColours          m_oSkyColours;                       // 0x034
	AEnvFXColours          m_aCloudColours[ NUM_LAYERS ];       // 0x074
	Toshi::TWorldVis       m_aCloudVis[ NUM_LAYERS ];           // 0x134
	ACamera*               m_apRenderCalls[ MAX_RENDER_CALLS ]; // 0x2CC
	TINT                   m_iNumRenderCalls;                   // 0x2DC
	TBOOL                  m_bHasStars;                         // 0x2E0
	Toshi::TVector4        m_vCloudOffset;                      // 0x2E4 from the camera
	TFLOAT                 m_fCloudDescendSpeed;                // 0x2F4
	TFLOAT                 m_fCloudDescendTime;                 // 0x2F8
	TFLOAT                 m_fCloudDescendDuration;             // 0x2FC
	TFLOAT                 m_fCloudDescendAccel;                // 0x300
	TBOOL                  m_bRenderClouds;                     // 0x304
	TFLOAT                 m_fAlpha;                            // 0x308 of the gradient and the clouds
	Toshi::TVector4        m_vStarsOffset;                      // 0x30C from the camera
	TFLOAT                 m_fStarsSpeed;                       // 0x31C
	TFLOAT                 m_fStarsTime;                        // 0x320
	TBOOL                  m_bRenderStars;                      // 0x324
	Toshi::TModelInstance* m_apStars[ NUM_LAYERS ];             // 0x328
	Toshi::TWorldVis       m_aStarsVis[ NUM_LAYERS ];           // 0x334
	AEnvFXColours          m_aStarsColours[ NUM_LAYERS ];       // 0x4CC
	TFLOAT                 m_afStarsPhases[ NUM_LAYERS ];       // 0x58C of the twinkling (1.0 until space is reached)
	ID3D11VertexShader*    m_pVertexShader;                     // 0x598
	ID3D11PixelShader*     m_pPixelShader;                      // 0x59C
	ID3D11InputLayout*     m_pInputLayout;                      // 0x5A0
	TFLOAT                 m_fCloudSpeed;                       // 0x5A4 the settings' rotation speed
	TBOOL                  m_bSkySpace;                         // 0x5A8
	TBOOL                  m_bSkySpaceReached;                  // 0x5A9
};                                                              // 0x5AC
