#pragma once
#include "Toshi/Shaders/SysShader/TSysMesh.h"

TOSHI_NAMESPACE_START

class TSysMeshHAL : public TGenericClassDerived<TSysMeshHAL, TSysMesh, "TSysMeshHAL", TMAKEVERSION(1, 0), TFALSE>
{
public:
	virtual TBOOL Render() override;
	virtual TBOOL Create(FLAGS uiFlags, TUINT16 uiMaxVertices, TUINT16 uiMaxIndices) override;
	virtual TBOOL Lock(TLockBuffer* pBufferLock) override;
	virtual void  Unlock(TUINT16 uiNumVertices, TUINT16 uiNumIndices) override;
	virtual void  SetZBias(TINT zBias) override;
	virtual void  OnDestroy() override;

	/*ID3D11Buffer* GetVertexBuffer() const
		{
			return m_pVertexBuffer;
		}

		ID3D11Buffer* GetIndexBuffer() const
		{
			return m_pIndexBuffer;
		}*/

	TUINT16 GetNumVertices() const { return m_NumVertices; }

	TUINT16 GetIndexCount() const { return m_IndexCount; }

private:
	inline static TBOOL s_bStopRendering = TFALSE;

private:
	/*ID3D11Buffer* m_pVertexBuffer;
		ID3D11Buffer* m_pIndexBuffer;*/
	TUINT16 m_NumVertices;
	TUINT16 m_IndexCount;
	// ...
};

TOSHI_NAMESPACE_END
