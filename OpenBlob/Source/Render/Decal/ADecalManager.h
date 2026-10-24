#pragma once
#include "ADecalShader_DX11.h"

#include <Toshi/TSingleton.h>
#include <Toshi/T2DList.h>
#include <Toshi/T2Vector.h>
#include <Math/TVector2.h>
#include <Math/TVector4.h>
#include <Math/TPlane.h>
#include <Math/TMath.h>
#include <Render/TOrderTable.h>

class ACamera;

namespace Toshi
{
class TMaterial;
struct TPhysicsShapeContact;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: the decals: the paint, ink and landing splats left on the world. Up to 5 decals are queued per frame
// (AddDecal); in the pre-flush callback of the system shader's first order table each one collides a sphere with the
// world's meshes (the decal world first, then the game play one), clips the triangles it touches against the 4 planes
// of its square (or uses a plain quad when they form a flat square), maps one quarter of the splat texture on them and
// appends them to the ring vertex buffer of ADecalShader, which draws the whole ring every frame. The older decal
// records and per terrain cell lists are still allocated but unused. Created by AAppInitState, initialised when a level
// is loaded, deinitialised when the in-game state is removed.
//-----------------------------------------------------------------------------
class ADecalManager
    : public Toshi::TSingleton<ADecalManager>
{
public:
	static TCONSTEXPR_VAR TINT MAX_PENDING_DECALS = 5;
	static TCONSTEXPR_VAR TINT MAX_RENDER_CALLS   = 4;
	static TCONSTEXPR_VAR TINT MAX_DECALS         = 5000;
	static TCONSTEXPR_VAR TINT MAX_DECAL_POLYS    = 10000;
	static TCONSTEXPR_VAR TINT NUM_DECALS_BY_POS  = 0xC800;
	static TCONSTEXPR_VAR TINT MAX_OUT_VERTS      = 64; // s_iMaxNumOutVerts
	static TCONSTEXPR_VAR TINT NUM_CLIP_PLANES    = 4;
	static TCONSTEXPR_VAR TINT NUM_AXES           = 6;

	// The type of a decal: the quarter of the texture it uses (texture atlas of 2x2 splats)
	enum Type
	{
		Type_TopLeft     = 0,
		Type_Unk1        = 1,
		Type_TopRight    = 2,
		Type_BottomLeft  = 3,
		Type_BottomRight = 4,
	};

	enum Flags : TUINT8
	{
		Flags_Rotation = 1, // m_fRotation is used (SetRotation) for the next decal
		Flags_Rainbow  = 2, // the decals cycle through the colours (Blob's rainbow paint)
	};

	struct DecalPoly;

	// An old decal record (0x38 bytes); unused but its mesh is destroyed
	struct Decal
	    : public Toshi::T2DList<Decal>::Node
	{
		// $deBlob: FUNCTION 005f6830
		Decal()
		{
			m_uiUnk08      = 0;
			m_pMesh        = TNULL;
			m_pStoredDecal = TNULL;
		}

		TUINT32             m_uiUnk08;        // 0x08
		TUINT8              m_aUnk0C[ 0x20 ]; // 0x0C
		ADecalShader::Mesh* m_pMesh;          // 0x2C
		TUINT32             m_uiUnk30;        // 0x30
		DecalPoly*          m_pStoredDecal;   // 0x34 its compressed copy (StoreDecal)
	};

	// A triangle of a stored decal (0x16 bytes): each vertex is stored relative to the terrain sub cell it is in
	// (CompressPos), the normal in 3 bytes (CompressNormal)
	struct StoredTri
	{
		TUINT16 m_auiY[ 3 ];       // 0x00
		TUINT8  m_auiZ[ 3 ];       // 0x06
		TUINT8  m_auiX[ 3 ];       // 0x09
		TINT8   m_aiNormal[ 3 ];   // 0x0C
		TUINT8  m_auiSubCell[ 3 ]; // 0x0F
		TUINT8  m_auiCell[ 3 ];    // 0x12
		TUINT8  m_uiPad15;         // 0x15
	};

	// An old stored (compressed) decal record (0x80 bytes), in the list of its sub cell (DecalsByPos); unused
	// (StoreDecal, which fills it, is never called)
	struct DecalPoly
	    : public Toshi::T2DList<DecalPoly>::Node
	{
		static TCONSTEXPR_VAR TINT s_iMaxTrisPerDecal = 4;

		StoredTri  m_aTris[ s_iMaxTrisPerDecal ]; // 0x08
		TUINT32    m_uiUnk60;                     // 0x60
		TUINT16    m_uiY;                         // 0x64 of the centre
		TUINT8     m_uiNumTris;                   // 0x66
		TUINT8     m_uiCellIndex;                 // 0x67
		TUINT8     m_uiSubCell;                   // 0x68
		TUINT8     m_uiX;                         // 0x69
		TUINT8     m_uiZ;                         // 0x6A
		TUINT8     m_uiFlags;                     // 0x6B
		TUINT8     m_uiUnk6C;                     // 0x6C
		TUINT8     m_uiSize;                      // 0x6D (m_uiSize / 256 * s_fMaxDecalSize)
		TUINT8     m_aUnk6E[ 2 ];                 // 0x6E
		TINT       m_iFrame;                      // 0x70
		DecalPoly* m_pNextFree;                   // 0x74
		TUINT32    m_uiUnk78;                     // 0x78
		Decal*     m_pDecal;                      // 0x7C
	};

	// The decals by terrain sub cell (16x16 per cell); unused
	struct DecalsByPos
	{
		void Validate();

		// The list of the sub cell a_pPos is in (TNULL: outside the terrain)
		Toshi::T2DList<DecalPoly>* GetDecals( const Toshi::TVector4* a_pPos );

		// The lists of the 32x32 sub cells around the camera (TNULL: culled by its view planes when a_bCull)
		TBOOL FindDecals( ACamera* a_pCamera, Toshi::T2DList<DecalPoly>** a_ppResults, TBOOL a_bCull );

		// $deBlob: FUNCTION 005f7160
		void AddDecal( DecalPoly* a_pDecal )
		{
			TUINT32 iIndex = a_pDecal->m_uiSubCell + ( a_pDecal->m_uiCellIndex << 8 );
			TASSERT( iIndex < TARRAYSIZE( m_aDecalsByPos ) && "iIndex < TNumOf(m_aDecalsByPos)" );
			m_aDecalsByPos[ iIndex ].PushBack( a_pDecal );
		}

		Toshi::T2DList<DecalPoly> m_aDecalsByPos[ NUM_DECALS_BY_POS ]; // 0x00
		TINT                      m_iLastCell;                         // 0x64000
		TINT                      m_iLastSearchCell;                   // 0x64004
	};

	// A render call of the frame (ARenderer::MainScene)
	struct RenderCall
	{
		ACamera* m_pCamera;    // 0x0
		TFLOAT   m_fDeltaTime; // 0x4
	};

public:
	ADecalManager();
	~ADecalManager();

	// Called when the level's data is loaded
	void Initialise();
	// Called when the in-game state is removed
	void Deinitialise();

	// Removes every decal
	void Reset();

	// Queues a decal at a_pPos (FALSE if 5 are queued already this frame)
	TBOOL AddDecal( const Toshi::TVector4* a_pPos, TFLOAT a_fSize, TFLOAT a_fRadius, TUINT32 a_uiColour, Toshi::TMaterial* a_pMaterial, TINT a_eType );

	void AddRenderCall( TFLOAT a_fDeltaTime, ACamera* a_pCamera );

	// Creates the queued decals and draws them (the body of RenderCallback, not called on its own)
	void Render( const RenderCall* a_pRenderCall );

	// $deBlob: FUNCTION 005f7500
	void SetRotation( TFLOAT a_fRot )
	{
		TASSERT( a_fRot >= 0.0f && a_fRot <= Toshi::TMath::TWO_PI );
		m_uiFlags |= Flags_Rotation;
		m_fRotation = a_fRot;
	}

	void SetRainbow( TBOOL a_bRainbow )
	{
		if ( a_bRainbow )
			m_uiFlags |= Flags_Rainbow;
		else
			m_uiFlags &= ~Flags_Rainbow;
	}

private:
	void CreateDecal( const Toshi::TVector4* a_pPos, TFLOAT a_fSize, TFLOAT a_fRadius, TUINT32 a_uiColour, Toshi::TMaterial* a_pMaterial, TINT a_eType );
	void ProcessContacts( Toshi::TPhysicsShapeContact* a_pContacts, TINT a_iNumContacts, TBOOL a_bSortByAxis, TINT a_eType, const Toshi::TVector4* a_pPos, TFLOAT a_fSize, TUINT32 a_uiColour, Toshi::TMaterial* a_pMaterial );
	void ProjectTriangles( Toshi::T2Vector<Toshi::TVector4, 128>** a_ppTris, Toshi::TMaterial* a_pMaterial, const Toshi::TVector4* a_pPos, TFLOAT a_fSize, TUINT32 a_uiColour, TINT a_eType, TINT a_iUnused );
	void ClipTriangle( const Toshi::TVector4* a_pTriangle, Toshi::TVector4 a_vNormal );
	// a_pNormal points away from m_vNormal
	TBOOL IsBackFacing( const Toshi::TVector4* a_pNormal ) const;
	TBOOL AddQuad( const Toshi::TPlane* a_pPlane, const Toshi::TVector4* a_pPos, const Toshi::TVector4* a_pDir );
	void  ComputeUVs( TINT a_iStart, TINT a_iEnd, TINT a_eType );
	TBOOL WriteToMesh( Toshi::TMaterial* a_pMaterial, const Toshi::TVector4* a_pPos, TFLOAT a_fSize, TUINT32 a_uiColour, TINT a_iFrame, TFLOAT a_fAlpha );
	void  SetupClipPlanes( const Toshi::TVector4* a_pForward, Toshi::TVector4* a_pUp, Toshi::TVector4* a_pRight, TFLOAT a_fSize );

	// Stores a compressed copy of a decal's triangles (5 entries each, see ProcessContacts) in the list of its sub
	// cell; never called
	void StoreDecal( Decal* a_pDecal, Toshi::T2Vector<Toshi::TVector4, 128>* a_pvTris, TUINT32 a_uiUnk60, TUINT8 a_uiUnk6C, TFLOAT a_fWidth );

	// $deBlob: FUNCTION 005f6070
	DecalPoly* AllocDecalPoly()
	{
		DecalPoly* pDecal = m_pFreeDecalPoly;
		m_pFreeDecalPoly  = pDecal->m_pNextFree;
		m_pLastDecalPoly  = pDecal;

		if ( pDecal->IsLinked() )
			pDecal->Remove();

		return pDecal;
	}

	void AddOutVert( const Toshi::TVector4& a_rcVert, const Toshi::TVector4& a_rcNormal )
	{
		m_avOutNormals[ m_iCurNumOutVerts ] = a_rcNormal;
		m_avOutVerts[ m_iCurNumOutVerts ]   = a_rcVert;
		m_iCurNumOutVerts++;
	}

	static TBOOL __stdcall IsCoplanar( Toshi::T2Vector<Toshi::TVector4, 128>* a_pTris, Toshi::TPlane* a_pOutPlane );
	static void __stdcall GetTriangleNormal( const Toshi::TVector4** a_ppVerts, Toshi::TVector4* a_pOutNormal );
	static void __stdcall ClassifyTriangle( const Toshi::TPlane* a_pPlane, TBOOL* a_pOutInFront, const Toshi::TVector4* a_pVerts );
	static void __stdcall CompressNormal( const Toshi::TVector4* a_pNormal, TINT8* a_pX, TINT8* a_pY, TINT8* a_pZ );
	static void __stdcall CompressPos( const Toshi::TVector4* a_pCellPos, const Toshi::TVector4* a_pPos, TUINT8* a_pX, TUINT16* a_pY, TUINT8* a_pZ, TUINT8* a_pSubCell );
	static TBOOL __stdcall IsSphereInsidePlanes( const Toshi::TVector4* a_pSphere, const Toshi::TPlane* a_pPlanes, TINT a_iNumPlanes );
	static void __stdcall DecompressNormal( TINT8 a_iX, TINT8 a_iY, TINT8 a_iZ, Toshi::TVector4* a_pOutNormal );
	static Toshi::TVector4* __stdcall DecompressPos( const Toshi::TVector4* a_pCellPos, TUINT8 a_uiSubCell, TUINT8 a_uiX, TUINT16 a_uiY, TUINT8 a_uiZ, Toshi::TVector4* a_pOutPos );
	static void __stdcall RenderCallback( Toshi::TOrderTable* a_pOrderTable, void* a_pDecalManager );

private:
	Toshi::TVector4       m_avOutVerts[ MAX_OUT_VERTS ];             // 0x000
	Toshi::TVector2       m_avOutUVs[ MAX_OUT_VERTS ];               // 0x400
	Toshi::TVector4       m_avOutNormals[ MAX_OUT_VERTS ];           // 0x600
	TINT                  m_iCurNumOutVerts;                         // 0xA00
	Toshi::TPlane         m_aClipPlanes[ NUM_CLIP_PLANES ];          // 0xA04 right, -right, up, -up
	Toshi::TVector4       m_vPos;                                    // 0xA44
	Toshi::TVector4       m_vNormal;                                 // 0xA54
	TFLOAT                m_fSize;                                   // 0xA64
	TINT                  m_iNumPendingDecals;                       // 0xA68
	TUINT32               m_uiUnkA6C;                                // 0xA6C
	Toshi::TVector4       m_avPendingPos[ MAX_PENDING_DECALS ];      // 0xA70
	Toshi::TVector4       m_avUnkAC0[ MAX_PENDING_DECALS ];          // 0xAC0 (unused)
	TFLOAT                m_afPendingSize[ MAX_PENDING_DECALS ];     // 0xB10
	TFLOAT                m_afPendingRadius[ MAX_PENDING_DECALS ];   // 0xB24
	TINT                  m_aePendingType[ MAX_PENDING_DECALS ];     // 0xB38
	TUINT32               m_auiPendingColour[ MAX_PENDING_DECALS ];  // 0xB4C
	Toshi::TMaterial*     m_apPendingMaterial[ MAX_PENDING_DECALS ]; // 0xB60
	TUINT32               m_uiUnkB74;                                // 0xB74
	Decal                 m_aDecals[ MAX_DECALS ];                   // 0xB78
	DecalPoly             m_aDecalPolys[ MAX_DECAL_POLYS ];          // 0x45138
	Toshi::T2DList<Decal> m_llDecals;                                // 0x17D938
	TUINT8                m_aUnk17D940[ 0x4E20 ];                    // 0x17D940
	TUINT32               m_uiUnk182760;                             // 0x182760
	TUINT32               m_uiUnk182764;                             // 0x182764
	DecalPoly*            m_pFreeDecalPoly;                          // 0x182768 (never initialised)
	DecalPoly*            m_pLastDecalPoly;                          // 0x18276C
	DecalsByPos           m_oDecalsByPos;                            // 0x182770
	TFLOAT                m_fUnk1E6778;                              // 0x1E6778
	TFLOAT                m_fRotation;                               // 0x1E677C
	TUINT8                m_uiFlags;                                 // 0x1E6780
	ACamera*              m_pCamera;                                 // 0x1E6784 (unused)
	TINT                  m_iFrame;                                  // 0x1E6788
	ADecalShader::Mesh*   m_pMesh;                                   // 0x1E678C
	TINT                  m_iMaxDecals;                              // 0x1E6790
	TBOOL                 m_bEnabled;                                // 0x1E6794
	RenderCall            m_aRenderCalls[ MAX_RENDER_CALLS ];        // 0x1E6798
	TINT                  m_iNumRenderCalls;                         // 0x1E67B8
};
