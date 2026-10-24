#pragma once
#include <Render/TMesh.h>

//-----------------------------------------------------------------------------
// Purpose: the base of the sprite meshes (ASpriteMesh). The virtuals are the TMesh's. NOTE: the two dwords after TMesh
// are not named.
//-----------------------------------------------------------------------------
class AMesh
    : public Toshi::TMesh
{
public:
	TDECLARE_CLASS( AMesh, Toshi::TMesh );

public:
	AMesh();
	virtual ~AMesh();

private:
	TUINT32 m_Unk10; // 0x10
	TUINT32 m_Unk14; // 0x14
};                   // 0x18
