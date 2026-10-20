#pragma once
#include "TXUIScene.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// TXUIMessageBox (0x130): a scene class without its own data or virtuals (created through ClassOverride)
//-----------------------------------------------------------------------------
class TXUIMessageBox : public TXUIScene
{
public:
	TDECLARE_CLASS( TXUIMessageBox, TXUIScene );

public:
	TXUIMessageBox();
	~TXUIMessageBox();
};

TOSHI_NAMESPACE_END
