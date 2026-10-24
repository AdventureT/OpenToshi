/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_HK_EXPORT_CHANNEL_MODIFIER_INTERFACE__H
#define INC_HK_EXPORT_CHANNEL_MODIFIER_INTERFACE__H

// Class ID
#define HK_EXPORT_CHANNEL_MODIFIER_CLASS_ID Class_ID(0x2de66bcf, 0x5113567d)

// Normal Parameter Blocks
enum
{
	PB_EXPORT_CHANNEL_MOD_PBLOCK 
};

// Normal Parameters
enum
{
	// GENERAL PROPERTIES ROLLOUT
	PA_EXPORT_CHANNEL_MOD_EXPORT_NAME = 0,
	PA_EXPORT_CHANNEL_MOD_CHANNEL_ID,
	PA_EXPORT_CHANNEL_MOD_CHANNEL_ID_SPIN,
	PA_EXPORT_CHANNEL_MOD_CHANNEL_NAME,
	PA_EXPORT_CHANNEL_MOD_TYPE,
	PA_EXPORT_CHANNEL_MOD_RESCALE,
	PA_EXPORT_CHANNEL_MOD_RESCALE_MIN_DIST,
	PA_EXPORT_CHANNEL_MOD_RESCALE_MAX_DIST,
	PA_EXPORT_CHANNEL_MOD_RESCALE_MIN_FLOAT,
	PA_EXPORT_CHANNEL_MOD_RESCALE_MAX_FLOAT,
	PA_EXPORT_CHANNEL_MOD_DISPLAY_ACTIVE,
	PA_EXPORT_CHANNEL_MOD_DISPLAY_SELECTED,
	PA_EXPORT_CHANNEL_MOD_USE_TRANSPARENCY,
	PA_EXPORT_CHANNEL_MOD_OPACITY,
	PA_EXPORT_CHANNEL_MOD_DISPLAY_TYPE
};


enum HavokChannelType
{
	HAVOK_CHANNEL_FLOAT = 0,
	HAVOK_CHANNEL_DISTANCE,
	HAVOK_CHANNEL_ANGLE
};

enum HavokChannelDisplayType
{
	HAVOK_CHANNEL_DISPLAY_RADIUS,
	HAVOK_CHANNEL_DISPLAY_NORMAL_DISTANCE
};


#endif // INC_HK_EXPORT_CHANNEL_MODIFIER_INTERFACE__H

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
