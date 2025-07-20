#pragma once
#include "Toshi/Core/TNodeList.h"

TOSHI_NAMESPACE_START

class TRenderAdapter : public TNodeList<TRenderAdapter>::TNode
{
public:
	class Mode : public TNodeList<Mode>::TNode
	{
	public:
		class Device : public TNodeList<Device>::TNode
		{
		public:
			Device() = default;
		};

	public:
		Mode() = default;

		virtual ~Mode() = default;

		virtual TRenderAdapter*               GetAdapter() const     = 0;
		virtual size_t                        GetModeIndex() const   = 0;
		virtual TUINT32                       GetWidth() const       = 0;
		virtual TUINT32                       GetHeight() const      = 0;
		virtual TBOOL                         SomeCheck1() const     = 0;
		virtual TBOOL                         SomeCheck2() const     = 0;
		virtual TFLOAT                        GetRefreshRate() const = 0;
		virtual TRenderAdapter::Mode::Device* GetDevice(TINT device) = 0;
	};

public:
	TRenderAdapter() = default;

	virtual ~TRenderAdapter() = default;

	TNodeList<Mode>* GetModeList() { return &m_Modes; }

private:
	TNodeList<Mode> m_Modes; // 0x10
};

TOSHI_NAMESPACE_END
