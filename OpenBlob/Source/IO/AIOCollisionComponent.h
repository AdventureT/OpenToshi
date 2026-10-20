#pragma once
#include "IO/AIOEntityComponent.h"
#include "Entities/APlayerMoverComponent.h"

#include <Math/TMatrix44.h>
#include <Math/TVector4.h>
#include <Math/TQuaternion.h>

class ACollisionObject;
class AModelInstance2;

//-----------------------------------------------------------------------------
// Purpose: the collision of an IO entity, its component of type 5 (AIOCollisionComponent.cpp): an ACollisionObject
// placed at the data's position and rotation relative to the entity, and an APlayerMoverComponent that gives the
// players standing on it its movement. The action 0x1A enables or disables the collision (its group mask), the action
// 0x49 sets the push along the entity's right axis.
//-----------------------------------------------------------------------------
class AIOCollisionComponent
    : public AIOEntityComponent
{
public:
	static TCONSTEXPR_VAR TINT TYPE = 5;

	enum ACTION : TINT
	{
		ACTION_EnableCollision = 0x1A,
		ACTION_SetPush         = 0x49,
	};

	// The collision group mask of an enabled collision (ACollisionObject's 0x11)
	static TCONSTEXPR_VAR TUINT8 COLLISION_MASK = 0xFB;

	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_Enabled = 0x1,
		FLAGS_Unk8    = 0x8, // sets the owner's flag 0x100000
	};

	// The IO data of the component (SetIOEntityData)
	struct Data
	{
		TINT                   iUnk00;    // 0x00
		TINT                   iUnk04;    // 0x04
		FLAGS                  uiFlags;   // 0x08
		const Toshi::TVector4* pPosition; // 0x0C Z up
		const Toshi::TVector4* pRotation; // 0x10 a quaternion
		const Toshi::TVector4* pUnk14;    // 0x14
		TFLOAT                 fPush;     // 0x18
	};

	struct IOEntityData
	{
		TINT  iType; // 0x00 TYPE
		Data* pData; // 0x04
	};

public:
	AIOCollisionComponent( AIOEntity* a_pOwner );
	virtual ~AIOCollisionComponent() override; // +0x00

	// Tick: ticks the mover
	virtual void Tick( TFLOAT a_fDeltaTime ) override; // +0x08

	// SetIOEntityData: a_pUnk1 is the component's IOEntityData
	virtual void SetIOEntityData( void* a_pData, Toshi::TTRB* a_pTRB ) override; // +0x0C

	// ExecuteAction: the ACTIONs
	virtual TBOOL ExecuteActionInvoker( TINT* a_pMessage, AIOEntity* a_pEntity, ABaseEntity* a_pOther ) override; // +0x10

	// Places the model and the collision with the entity's matrix
	void UpdateWorldTransform( const Toshi::TMatrix44& a_rEntityMatrix );

	void GetMatrix( Toshi::TMatrix44& a_rOutMatrix );
	// NOTE: retail passes the new scale, unused
	void SetCollisionDirty( const Toshi::TVector4& a_rUnused );

	ACollisionObject* GetCollisionObject() const { return m_pCollisionObject; }
	void              GetScale( Toshi::TVector4& a_rOutScale );
	void              GetPosition( Toshi::TVector4& a_rOutPosition );
	void              GetRotationMatrix( Toshi::TMatrix44& a_rOutMatrix );

	// The velocity the collision gives a_pEntity (when the entity collides with its group), its push included
	Toshi::TVector4 GetReactionForce( const Toshi::TVector4& a_rvPoint, ABaseEntity* a_pEntity );

	// The rotation the collision gives a_pEntity (when the entity collides with its group)
	Toshi::TQuaternion GetReactionRotation( ABaseEntity* a_pEntity );

private:
	TINT                   m_iUnk14;           // 0x14
	TINT                   m_iUnk18;           // 0x18
	FLAGS                  m_uiCollisionFlags; // 0x1C
	const Toshi::TVector4* m_pPosition;        // 0x20
	const Toshi::TVector4* m_pRotation;        // 0x24
	const Toshi::TVector4* m_pUnk28;           // 0x28
	TFLOAT                 m_fUnk2C;           // 0x2C the data's push
	Toshi::TMatrix44       m_oLocalMatrix;     // 0x30
	ACollisionObject*      m_pCollisionObject; // 0x70
	AModelInstance2*       m_pModelInstance;   // 0x74
	APlayerMoverComponent  m_oMover;           // 0x78
	TFLOAT                 m_fPush;            // 0xCC along the entity's right axis
};                                             // 0xD0
