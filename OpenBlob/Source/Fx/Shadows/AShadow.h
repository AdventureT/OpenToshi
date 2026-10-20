#pragma once
#include <Math/TVector4.h>
#include <Math/TMatrix44.h>

TOSHI_NAMESPACE_START
class TPhysicsPhantomContainer;
TOSHI_NAMESPACE_END

//-----------------------------------------------------------------------------
// Purpose: a blob shadow under an entity: a ray cast down from the owner finds the ground, the shadow model
// (Fx_Shadow.trb) is drawn there by AInstanceRenderer, faded out with the distance to the camera. The visible shadows
// of the frame are collected by Update and drawn by BatchSubmit (ARenderer::MainScene).
// de Blob's world is Y-down: the ray goes along +Y and a ground normal has y <= -0.5
//-----------------------------------------------------------------------------
class AShadow
{
public:
	enum FLAG : TUINT8
	{
		FLAG_Dirty       = BITFLAG( 0 ), // ray cast again at the next Update
		FLAG_OnGround    = BITFLAG( 1 ), // in range and on the ground: drawn
		FLAG_Hidden      = BITFLAG( 2 ),
		FLAG_Initialised = BITFLAG( 4 ),
	};

	static TCONSTEXPR_VAR TINT MAX_SHADOWS = 128;

	struct ShadowType
	{
		const TCHAR* m_szModel;  // 0x00
		TUINT32      m_uiColour; // 0x04
	};

public:
	AShadow( TFLOAT a_fSize, TUINT8 a_eType );
	~AShadow();

	void Init( TFLOAT a_fSize, TUINT8 a_eType );
	void SetHidden( TBOOL a_bHidden );

	// Updates the shadow of an owner at a_rPosition facing a_rDirection (a_bForce: ray cast now)
	void Update( const Toshi::TVector4& a_rPosition, const Toshi::TVector4& a_rDirection, TBOOL a_bForce );

	TBOOL IsInitialised() const { return ( m_uiFlags & FLAG_Initialised ) != 0; }
	// $deBlob: FUNCTION 0077d160
	void SetAlpha( TFLOAT a_fAlpha ) { m_fAlpha = a_fAlpha; }
	void SetHeight( TFLOAT a_fHeight ) { m_fHeight = a_fHeight; }
	void SetSize( TFLOAT a_fWidth, TFLOAT a_fLength )
	{
		m_fWidth  = a_fWidth;
		m_fLength = a_fLength;
	}

	static void BatchSumbit();

	// Forgets the registered shadows (AInGameState::OnRemoval)
	static void ResetShadowCount() { ms_iNumShadows = 0; }

private:
	void UpdateVisibility( const Toshi::TVector4& a_rPosition, const Toshi::TVector4& a_rDirection, TBOOL a_bForce );
	void UpdatePosition( const Toshi::TVector4& a_rPosition, const Toshi::TVector4& a_rDirection, TBOOL a_bForce );

private:
	static ShadowType ms_aTypes[];
	static TBOOL      ms_bFixRenderVolume; // the first Init zeroes the model's radius
	static TFLOAT     ms_fFadeStartSq;     // 90 m
	static TFLOAT     ms_fFadeEndSq;       // 100 m
	static TFLOAT     ms_fFlatNormalY;
	static TFLOAT     ms_fNoGroundNormalY;
	static TFLOAT     ms_fMoveThreshold; // (times the squared camera distance)

	static AShadow*         ms_apShadows[ MAX_SHADOWS ];
	static TINT             ms_iNumShadows;
	static Toshi::TMatrix44 ms_aMatrices[ MAX_SHADOWS ];

private:
	Toshi::TVector4                  m_vPosition;         // 0x00
	Toshi::TVector4                  m_vDirection;        // 0x10
	Toshi::TVector4                  m_Normal;            // 0x20
	Toshi::TVector4                  m_vRayPosition;      // 0x30 the owner's position at the last ray cast
	TFLOAT                           m_fGroundNormalY;    // 0x40
	TFLOAT                           m_fFade;             // 0x44
	TFLOAT                           m_fAlpha;            // 0x48
	TFLOAT                           m_fWidth;            // 0x4C
	TFLOAT                           m_fLength;           // 0x50
	TFLOAT                           m_fHeight;           // 0x54
	Toshi::TPhysicsPhantomContainer* m_shadowQueryHelper; // 0x58
	TINT8                            m_iMesh;             // 0x5C AInstanceRenderer
	TUINT8                           m_uiFlags;           // 0x5D FLAG
	TUINT8                           m_eType;             // 0x5E ms_aTypes
};
