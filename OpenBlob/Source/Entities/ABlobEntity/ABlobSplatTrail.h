#pragma once
#include <Math/TVector4.h>

class ABlobEntity;

//-----------------------------------------------------------------------------
// Purpose: the paint splats a coloured Blob leaves while rolling (ABlobEntity+0x3CC, ticked by ABlobEntity::Tick): a
// decal of the Blob's colour every time it moved far enough on the ground (or its 0x628 value changed), at most one per
// frame. NOTE: class.
//-----------------------------------------------------------------------------
class ABlobSplatTrail
{
public:
	ABlobSplatTrail( ABlobEntity* a_pBlob );

	void Update();

private:
	ABlobEntity*    m_pBlob;                   // 0x00
	Toshi::TVector4 m_vLastPos;                // 0x04 where the last splat went
	TFLOAT          m_fLastSquashCompensation; // 0x14 the Blob's 0x628 then
	TUINT32         m_uiLastFrame;             // 0x18 the frame of the last splat
};
