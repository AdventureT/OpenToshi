/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_HK_RIGIDBODY_MODIFIER_INTERFACE__H
#define INC_HK_RIGIDBODY_MODIFIER_INTERFACE__H

// Class ID
#define HK_RIGIDBODY_MODIFIER_CLASS_ID Class_ID(0xa75686b, 0x374c5712)

// Class Pblocks
enum
{
	// ID must be different than that for normal pblocks
	CPB_RB_MOD_PBLOCK = 100,
};

// Class Parameters
enum
{
	CPA_RB_MOD_MARK_RIGID_BODIES,
};

// Normal Parameter Blocks
enum
{
	PB_RB_MOD_PBLOCK, // has two rollouts
};

// Normal Parameters
enum
{
	// GENERAL PROPERTIES ROLLOUT
	PA_RB_MOD_MASS = 0,
	PA_RB_MOD_RESTITUTION,
	PA_RB_MOD_FRICTION,

	// COM & INERTIA TENSOR ROLLOUT
	PA_RB_MOD_CHANGE_CENTER_OF_MASS = 100,
	PA_RB_MOD_CENTER_OF_MASS,
  
	PA_RB_MOD_CHANGE_INERTIA_TENSOR,
	PA_RB_MOD_INERTIA_TENSOR,

	// ADVANCED PROPERTIES ROLLOUT
	PA_RB_MOD_CHANGE_LINEAR_DAMPING, 
	PA_RB_MOD_LINEAR_DAMPING, 

	PA_RB_MOD_CHANGE_ANGULAR_DAMPING, 
	PA_RB_MOD_ANGULAR_DAMPING,

	PA_RB_MOD_CHANGE_ALLOWED_PENETRATION_DEPTH,
	PA_RB_MOD_ALLOWED_PENETRATION_DEPTH,

	PA_RB_MOD_CHANGE_MAX_LINEAR_VELOCITY,
	PA_RB_MOD_MAX_LINEAR_VELOCITY,

	PA_RB_MOD_CHANGE_MAX_ANGULAR_VELOCITY,
	PA_RB_MOD_MAX_ANGULAR_VELOCITY,

	PA_RB_MOD_CHANGE_COLLISION_FILTER_INFO,
	PA_RB_MOD_COLLISION_FILTER_INFO,

	PA_RB_MOD_CHANGE_QUALITY_TYPE,
	PA_RB_MOD_QUALITY_TYPE,

	PA_RB_MOD_CHANGE_SOLVER_DEACTIVATION,	
	PA_RB_MOD_SOLVER_DEACTIVATION,
	
	PA_RB_MOD_CHANGE_DEACTIVATOR_TYPE,
	PA_RB_MOD_DEACTIVATOR_TYPE

};

/*
** ENUMS (combo boxes)
*/

// Quality Type
enum
{
	QT_FIXED,
	QT_KEYFRAMED,
	QT_KEYFRAMED_REPORTING,
	QT_DEBRIS,
	QT_MOVING,
	QT_CRITICAL,
	QT_BULLET,

	QT_LAST
};


// Solver Deactivation
enum
{
	SD_OFF,
	SD_LOW,
	SD_MEDIUM,
	SD_HIGH
};

// Deactivator Type
enum
{
	DT_SPATIAL,
	DT_LOW
};



#endif //INC_HK_RIGIDBODY_MODIFIER_INTERFACE__H

/*
* Havok SDK - NO SOURCE PC DOWNLOAD, BUILD(#20090216)
* 
* Confidential Information of Havok.  (C) Copyright 1999-2009
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
* 
* Use of this software for evaluation purposes is subject to and indicates
* acceptance of the End User licence Agreement for this product. A copy of
* the license is included with this software and is also available at www.havok.com/tryhavok.
* 
*/
