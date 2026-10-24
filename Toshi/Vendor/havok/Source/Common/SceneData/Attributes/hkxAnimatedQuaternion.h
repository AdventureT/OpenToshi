/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_SCENEDATA_HKX_ANIMATED_QUATERNION_H
#define HK_SCENEDATA_HKX_ANIMATED_QUATERNION_H

extern const hkClass hkxAnimatedQuaternionClass;

	/// An hkxAnimatedQuaternion stores the values associated with an hkxAttribute of type quaternion.
struct hkxAnimatedQuaternion
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxAnimatedQuaternion );
	HK_DECLARE_REFLECTION();

		/// The array of sampled rotations. Array can be of length 1 for not animated, 
		/// or one per frame so assumes that the value is then fully sampled.
	hkQuaternion* m_quaternions;
	int m_numQuaternions;
};


#endif // HK_SCENEDATA_HKX_ANIMATED_QUATERNION_H

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
