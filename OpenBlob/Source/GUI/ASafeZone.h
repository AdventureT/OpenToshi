#pragma once
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// Purpose: the frame of the TV safe zone (a T2GUIElement with four border rectangles), shown with a
// debug toggle (AGUIManager::ToggleSafeZone). The size of the safe area, centred in AGUIManager's safe area.
//-----------------------------------------------------------------------------
class ASafeZone : public Toshi::T2GUIElement
{
public:
	ASafeZone();
	~ASafeZone();

	// Sizes the borders to the safe area and hides the frame
	void Create();

private:
	Toshi::T2GUIRectangle m_aBorders[ 4 ]; // 0x3C left, right, top, bottom
};
