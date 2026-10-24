#pragma once
#include "AGndMotorComponent.h"

#include <Toshi/TSingleton.h>
#include <Toshi/T2DList.h>

//-----------------------------------------------------------------------------
// Purpose: the list of the active ground motors. Each frame it asks one resting motor to probe its ground contact again
// (AGndMotorComponent::FLAG_ProbeGround), round-robin.
//-----------------------------------------------------------------------------
class AGndMotorManager
    : public Toshi::TSingleton<AGndMotorManager>
{
public:
	AGndMotorManager();
	~AGndMotorManager();

	void RegisterMotorComponent( AGndMotorComponent* a_pMotor );
	void UnregisterMotorComponent( AGndMotorComponent* a_pMotor );
	void Update( TFLOAT a_fDeltaTime );

	// Makes every motor forget a_pObject as its ground (ACellBucket::Unload, before the cell's collision object is
	// deleted)
	void OnCollisionObjectDestroyed( ACollisionObject* a_pObject );

private:
	Toshi::T2DList<AGndMotorNode>           m_llMotors; // 0x0
	Toshi::T2DList<AGndMotorNode>::Iterator m_itNext;   // 0x8 the last motor asked to probe
};
