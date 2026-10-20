#pragma once
#include "../A2DHashItem.h"
#include "APropPlatform.h"

#include <Math/TVector4.h>

class APooledEntity;

//-----------------------------------------------------------------------------
// Purpose: a civilian spawned by ASpawnedCivManager: an A2DHash item at its position and radius, with the pooled ACiv
// (m_pPooledEntity, TNULL when the pool had none, then the wrapper is only a point). Its respawn data (0x34,
// APropPlatform::RespawnData: the waypoint set at 0x50, the next point at 0x5A, the flags at 0x5E) is what the ACiv is
// started from (ACiv::SetWaypointSet).
//-----------------------------------------------------------------------------
class ACivWrapper
    : public A2DHashItem
{
public:
	ACivWrapper();
	virtual ~ACivWrapper();

	virtual const Toshi::TVector4* GetPosition(); // +0x04
	virtual TFLOAT                 GetRadius();   // +0x08
	// $deBlob: FUNCTION 00763ae0
	const Toshi::TVector4& GetFacing() const { return m_vFacing; }

public:
	Toshi::TVector4            m_vPosition;     // 0x14 the position of the civilian
	Toshi::TVector4            m_vFacing;       // 0x24
	APropPlatform::RespawnData m_RespawnData;   // 0x34 (0x2C bytes)
	TUINT8                     m_uiColour;      // 0x60
	TUINT8                     m_aUnk61[ 3 ];   // 0x61
	TFLOAT                     m_fRadius;       // 0x64
	APooledEntity*             m_pPooledEntity; // 0x68
	TINT                       m_iParam;        // 0x6C
};
