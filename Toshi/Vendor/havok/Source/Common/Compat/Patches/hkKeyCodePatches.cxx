/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#if !defined(HK_PRODUCT_PATCH)
#error Please define HK_PRODUCT_PATCH(NAME) macro first.
#endif

#if defined(USING_HAVOK_PHYSICS)
	HK_PRODUCT_PATCH(Physics)
#endif
#if defined(USING_HAVOK_ANIMATION)
	HK_PRODUCT_PATCH(Animation)
#endif
#if defined(USING_HAVOK_BEHAVIOR)
	HK_PRODUCT_PATCH(Behavior)
#endif
#if defined(USING_HAVOK_DESTRUCTION)
	HK_PRODUCT_PATCH(Destruction)
#endif
#if defined(USING_HAVOK_CLOTH)
	HK_PRODUCT_PATCH(Cloth)
#endif

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
