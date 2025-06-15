#pragma once
#include "Toshi/Core/TObject.h"
#include "Toshi/Render/TMesh.h"

TOSHI_NAMESPACE_START

class TResource;

class TSysMesh : public TGenericClassDerived<TSysMesh, TMesh, "TSysMesh", TMAKEVERSION(1, 0), TFALSE>
{
public:
	typedef TUINT32 FLAGS;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE   = 0,
		FLAGS_LOCKED = 0x8000
	};

public:
	TSysMesh();

	virtual TBOOL      Validate() override;
	virtual void       Invalidate() override;
	virtual TBOOL      Create(FLAGS uiFlags, TUINT16 uiMaxVertices, TUINT16 uiMaxIndices);
	virtual TBOOL      Lock(TLockBuffer* pLockBuffer);
	virtual void       Unlock(TUINT16 uiNumVertices, TUINT16 uiNumIndices);
	virtual TResource* GetVertexPool() const;
	virtual TResource* GetIndexPool() const;
	virtual void       SetZBias(TINT zBias) = 0;
	virtual void       SetAlphaRef() {}

	TBOOL Allocate();
	void  Deallocate();

protected:
	TUINT32    m_uiFlags;
	TUINT16    m_uiMaxVertices;
	TUINT16    m_uiMaxIndices;
	TResource* m_pVertexPool;
	TResource* m_pIndexPool;
};

TOSHI_NAMESPACE_END
