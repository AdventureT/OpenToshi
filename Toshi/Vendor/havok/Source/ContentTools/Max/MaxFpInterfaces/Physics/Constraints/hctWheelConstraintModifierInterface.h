/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_HK_WHEEL_CONSTRAINT_MODIFIER_INTERFACE__H
#define INC_HK_WHEEL_CONSTRAINT_MODIFIER_INTERFACE__H

// Class ID
#define HK_CONSTRAINT_WHEEL_CLASS_ID Class_ID(0x19002822, 0x7c0c6b51)

// Parameter Blocks
enum
{
	PB_WHEEL_MOD_PBLOCK = PB_CONSTRAINT_MOD_FIRST_FREE_PARAMBLOCK
};

// Parameters
enum
{
	PA_WHEEL_MOD_SUSPENSION_LIMIT_MIN,
	PA_WHEEL_MOD_SUSPENSION_LIMIT_MAX,
	PA_WHEEL_MOD_SUSPENSION_STRENGTH,
	PA_WHEEL_MOD_SUSPENSION_DAMPING,
	PA_WHEEL_MOD_SUSPENSION_OFFSET
};


#endif //INC_HK_WHEEL_CONSTRAINT_MODIFIER_INTERFACE__H

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
