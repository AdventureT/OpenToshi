#pragma once
#include "T2GUIElement.h"

TOSHI_NAMESPACE_START

// NOTE: de Blob's T2GUIContext (0x3C) is just its root element (Barnyard's has a renderer, a root and a debug canvas)
class T2GUIContext
{
public:
	T2GUIContext();
	// NOTE: only the root element's destructor
	// $deBlob: FUNCTION 006f5430
	// $deBlob: FUNCTION 006be5f0
	~T2GUIContext() = default;

	void Tick( TFLOAT a_fDeltaTime );
	void Render();

	T2GUIElement* GetRootElement() { return &m_oRootElement; }

private:
	T2GUIElement m_oRootElement; // 0x00
};

TOSHI_NAMESPACE_END
