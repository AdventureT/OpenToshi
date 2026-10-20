/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_HK_PRISMATIC_CONSTRAINT_MODIFIER_INTERFACE__H
#define INC_HK_PRISMATIC_CONSTRAINT_MODIFIER_INTERFACE__H

// Class ID
#define HK_CONSTRAINT_PRISMATIC_CLASS_ID Class_ID(0x22ca3797, 0x5b1f29ed)

// Parameter Blocks
enum
{
	PB_PRISMATIC_MOD_PBLOCK = PB_CONSTRAINT_MOD_FIRST_FREE_PARAMBLOCK
};

// Parameters
enum
{
	PA_PRISMATIC_MOD_IS_LIMITED_NOT_USED = 0,
	PA_PRISMATIC_MOD_CHANGE_LIMIT_MIN,
	PA_PRISMATIC_MOD_LIMIT_MIN,
	PA_PRISMATIC_MOD_CHANGE_LIMIT_MAX,
	PA_PRISMATIC_MOD_LIMIT_MAX,
	PA_PRISMATIC_MOD_MAX_FRICTION_TORQUE,
	PA_PRISMATIC_MOD_MOTOR_TYPE,
};

#endif //INC_HK_PRISMATIC_CONSTRAINT_MODIFIER_INTERFACE__H

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
