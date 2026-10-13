#pragma once
#include "Toshi/Shaders/SysShader/TSysShader.h"
#include "TSysMaterialHAL_SDL.h"
#include "TSysMeshHAL_SDL.h"

TOSHI_NAMESPACE_START

class TSysShaderHAL : public TGenericClassDerived<TSysShaderHAL, TSysShader, "TSysShaderHAL", TMAKEVERSION(1, 0), TFALSE>
{
public:
	struct OrderTablePriorities
	{
		TUINT32 Priority1;
		TUINT32 Priority2;
		TUINT32 Priority3;
		TUINT32 Priority4;
		TUINT32 Priority5;
		TUINT32 Priority6;
		TUINT32 Priority7;
	};

public:
	TSysShaderHAL(OrderTablePriorities* priorities);
	~TSysShaderHAL();

	virtual void          OnDestroy() override;
	virtual void          Flush() override;
	virtual void          StartFlush() override;
	virtual void          EndFlush() override;
	virtual TBOOL         Create() override;
	virtual TBOOL         Validate() override;
	virtual void          Invalidate() override;
	virtual void          Render(TRenderPacket* pRenderPacket) override;
	virtual TSysMaterial* CreateMaterial() override;
	virtual TSysMesh*     CreateMesh(const TCHAR* name) override;

	TOrderTable* GetOrderTable(size_t index) { return &m_OrderTables[index]; }

	/*ID3D11PixelShader* GetPixelShaderNoTex() const
		{
			return m_pPixelShaderNoTex;
		}

		ID3D11PixelShader* GetPixelShaderTexture() const
		{
			return m_pPixelShaderTexture;
		}

		ID3D11PixelShader* GetPixelShaderAlphaTexture() const
		{
			return m_pPixelShaderAlphaTexture;
		}*/

private:
	TOrderTable          m_OrderTables[6];
	OrderTablePriorities m_Priorities;
	/*ID3D11VertexShader* m_pVertexShader;
		ID3D11PixelShader* m_pPixelShaderNoTex;
		ID3D11PixelShader* m_pPixelShaderTexture;
		ID3D11PixelShader* m_pPixelShaderAlphaTexture;
		ID3D11InputLayout* m_pInputLayout;*/
};

TOSHI_NAMESPACE_END
