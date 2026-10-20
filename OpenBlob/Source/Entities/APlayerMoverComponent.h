#pragma once
#include "Entities/AEntityComponent2.h"

#include <Math/TVector4.h>
#include <Math/TQuaternion.h>

//-----------------------------------------------------------------------------
// Purpose: the movement of an entity the players can stand on (a member of AIOCollisionComponent): it keeps the
// entity's position and rotation of this and the last frame, so the velocity at a point (GetReactionForce) and the
// rotation (GetReactionRotation) can be given to what touches it.
//-----------------------------------------------------------------------------
class APlayerMoverComponent
    : public AEntityComponent2
{
public:
	APlayerMoverComponent( ABaseEntity* a_pEntity );
	virtual ~APlayerMoverComponent() override; // +0x00

	// Keeps last frame's transform and reads the new one; when the entity moved, queries the players around it
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x04

	// The velocity of the point at a_pEntity's position (rotation and translation since the last frame); w is 1
	Toshi::TVector4 GetReactionForce( const Toshi::TVector4& a_rvPoint, ABaseEntity* a_pEntity );

	// The rotation since the last frame (normalised)
	Toshi::TQuaternion GetReactionRotation( ABaseEntity* a_pEntity );

private:
	Toshi::TVector4    m_vPosition;        // 0x14
	Toshi::TQuaternion m_quatRotation;     // 0x24
	Toshi::TVector4    m_vPrevPosition;    // 0x34
	Toshi::TQuaternion m_quatPrevRotation; // 0x44
};                                         // 0x54
