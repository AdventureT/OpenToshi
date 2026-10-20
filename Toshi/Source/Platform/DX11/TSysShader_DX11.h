#pragma once
#include "Shaders/SysShader/TSysShader.h"
#include "Render/TOrderTable.h"
#include "Math/TVector4.h"

#include <d3d11.h>

TOSHI_NAMESPACE_START

class TSysShaderHAL;

//-----------------------------------------------------------------------------
// Purpose: DX11 part of TSysMaterial: picks the order table of the blend mode and sets the pixel shader, the blend and
// depth states and the alpha test before the material's meshes are drawn.
//-----------------------------------------------------------------------------
class TSysMaterialHAL : public TSysMaterial
{
public:
	TDECLARE_CLASS( TSysMaterialHAL, TSysMaterial );

public:
	TSysMaterialHAL();
	~TSysMaterialHAL();

	//-----------------------------------------------------------------------------
	// TMaterial
	//-----------------------------------------------------------------------------
	virtual void PreRender() OVERRIDE;  // +0x14
	virtual void PostRender() OVERRIDE; // +0x18

	//-----------------------------------------------------------------------------
	// TSysMaterial
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( BLENDMODE a_eBlendMode ) OVERRIDE;       // +0x1C
	virtual void  SetBlendMode( BLENDMODE a_eBlendMode ) OVERRIDE; // +0x20

	using TMaterial::Create;

	void SetOrderTable( TOrderTable* a_pOrderTable );

public:
	// The values of s_eSysMaterialLazyState: the render state PreRender set last (a bit per blend mode)
	enum LAZYSTATE : TUINT32
	{
		LAZYSTATE_RESET                = BITFLAG( 0 ), // set again by StartFlush and Render: the next PreRender sets its state
		LAZYSTATE_DEFAULT_ALPHATEXTURE = BITFLAG( 2 ),
		LAZYSTATE_DEFAULT_UNTEXTURED   = BITFLAG( 3 ),
		LAZYSTATE_DEFAULT_TEXTURED     = BITFLAG( 4 ),
		LAZYSTATE_ADDITIVE             = BITFLAG( 5 ),
		LAZYSTATE_BLENDMODE_1          = BITFLAG( 6 ),
		LAZYSTATE_BLENDMODE_2          = BITFLAG( 7 ),
		LAZYSTATE_BLENDMODE_4          = BITFLAG( 8 ),
		LAZYSTATE_ALPHATEST            = BITFLAG( 9 ),
		LAZYSTATE_BLENDMODE_6_7        = BITFLAG( 10 ),
	};

	static TUINT32 s_eSysMaterialLazyState;
	// The pixel shaders' alphaTest constant: x > 0 enables it, y is the reference
	static TVector4 s_AlphaTest;

private:
	TOrderTable* m_pOrderTable; // 0x58
}; // 0x5C

//-----------------------------------------------------------------------------
// Purpose: DX11 part of TSysMesh: dynamic vertex (0x18 bytes: position, B8G8R8A8 colour, UV) and index buffers mapped by
// Lock, drawn as a triangle strip by TSysShaderHAL::Render. NOTE: retail has no OnDestroy of its own (TMesh's), the
// buffers are never released
//-----------------------------------------------------------------------------
class TSysMeshHAL : public TSysMesh
{
public:
	TDECLARE_CLASS( TSysMeshHAL, TSysMesh );

public:
	TSysMeshHAL();
	~TSysMeshHAL();

	//-----------------------------------------------------------------------------
	// TMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Render() OVERRIDE; // +0x18

	//-----------------------------------------------------------------------------
	// TSysMesh
	//-----------------------------------------------------------------------------
	virtual TBOOL Create( FLAGS a_uiFlags, TUINT16 a_uiMaxVertices, TUINT16 a_uiMaxIndices ) OVERRIDE; // +0x20
	virtual TBOOL Lock( TLockBuffer* a_pLockBuffer ) OVERRIDE;                                         // +0x24
	virtual void  Unlock( TUINT32 a_uiNumVertices, TUINT32 a_uiNumIndices ) OVERRIDE;                   // +0x28
	virtual void  SetZBias( TINT a_iZBias ) OVERRIDE;                                                  // +0x34

	using TMesh::Create;

	ID3D11Buffer* GetVertexBuffer() const { return m_pVertexBuffer; }
	ID3D11Buffer* GetIndexBuffer() const { return m_pIndexBuffer; }
	TUINT16       GetNumVertices() const { return m_uiNumVertices; }
	TUINT16       GetIndexCount() const { return m_uiIndexCount; }

public:
	static TBOOL s_bStopRendering;

private:
	ID3D11Buffer* m_pVertexBuffer; // 0x24
	ID3D11Buffer* m_pIndexBuffer;  // 0x28
	TUINT16       m_uiNumVertices; // 0x2C
	TUINT16       m_uiIndexCount;  // 0x2E
}; // 0x30

//-----------------------------------------------------------------------------
// Purpose: the system shader (DX11): six order tables (one per blend mode group), a vertex shader and three pixel
// shaders (colour, texture, alpha texture) compiled from the HLSL embedded in the exe. Created by ARenderer::Create with its own
// priorities, followed by Create.
//-----------------------------------------------------------------------------
class TSysShaderHAL : public TSysShader
{
public:
	TDECLARE_CLASS( TSysShaderHAL, TSysShader );

public:
	static TCONSTEXPR_VAR TINT NUM_ORDERTABLES = 6;

	struct OrderTablePriorities
	{
		TINT iPriority1; // 0x00 order table 0
		TINT iPriority2; // 0x04 order table 1
		TINT iPriority3; // 0x08 order table 2
		TINT iPriority4; // 0x0C (unused)
		TINT iPriority5; // 0x10 order table 4
		TINT iPriority6; // 0x14 order table 5
		TINT iPriority7; // 0x18 order table 3
	};

	friend TSysMaterialHAL;

public:
	TSysShaderHAL( const OrderTablePriorities* a_pPriorities = TNULL );
	~TSysShaderHAL();

	//-----------------------------------------------------------------------------
	// TShader
	//-----------------------------------------------------------------------------
	virtual void  OnDestroy() OVERRIDE;                              // +0x0C
	virtual void  Flush() OVERRIDE;                                  // +0x10
	virtual void  StartFlush() OVERRIDE;                             // +0x14
	virtual void  EndFlush() OVERRIDE;                               // +0x18
	virtual TBOOL Create() OVERRIDE;                                 // +0x1C
	virtual TBOOL Validate() OVERRIDE;                               // +0x20
	virtual void  Invalidate() OVERRIDE;                             // +0x24
	virtual void  Render( TRenderPacket* a_pRenderPacket ) OVERRIDE; // +0x34

	//-----------------------------------------------------------------------------
	// TSysShader
	//-----------------------------------------------------------------------------
	virtual TSysMaterial* CreateMaterial( const TCHAR* a_szName ) OVERRIDE; // +0x38
	virtual TSysMesh*     CreateMesh( const TCHAR* a_szName ) OVERRIDE;     // +0x3C

	TOrderTable* GetOrderTable( TINT a_iIndex ) { return &m_aOrderTables[ a_iIndex ]; }

private:
	TOrderTable          m_aOrderTables[ NUM_ORDERTABLES ]; // 0x0C
	OrderTablePriorities m_oPriorities;                     // 0xFC
	ID3D11VertexShader*  m_pVertexShader;                   // 0x118
	ID3D11PixelShader*   m_pPixelShaderNoTex;               // 0x11C
	ID3D11PixelShader*   m_pPixelShaderTexture;             // 0x120
	ID3D11PixelShader*   m_pPixelShaderAlphaTexture;        // 0x124
	ID3D11InputLayout*   m_pInputLayout;                    // 0x128
}; // 0x12C

TOSHI_NAMESPACE_END
