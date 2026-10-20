/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

//=======
// 650b1
//=======

HK_PATCH_BEGIN("hclTransitionConstraintSetPerParticle", 0, "hclTransitionConstraintSetPerParticle", 1)
	HK_PATCH_MEMBER_ADDED("toAnimDelay", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("toSimDelay", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("toSimMaxDistance", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_FUNCTION(hclTransitionConstraintSetPerParticle_0_to_1)
	HK_PATCH_MEMBER_REMOVED("particleDelay", TYPE_REAL, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hclSkinOperator", 0, "hclSkinOperator", 1)
	HK_PATCH_MEMBER_ADDED("startVertex", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("endVertex", TYPE_INT, HK_NULL, 0)
	HK_PATCH_FUNCTION(hclSkinOperator_0_to_1)
HK_PATCH_END()

HK_PATCH_BEGIN("hclTransitionConstraintSet", 0, "hclTransitionConstraintSet", 1)
	HK_PATCH_MEMBER_ADDED("toAnimPeriod", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("toAnimPlusDelayPeriod", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("toSimPeriod", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("toSimPlusDelayPeriod", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_FUNCTION(hclTransitionConstraintSet_0_to_1)
	HK_PATCH_MEMBER_REMOVED("transitionPeriod", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_REMOVED("transitionPlusDelayPeriod", TYPE_REAL, HK_NULL, 0)
HK_PATCH_END()


//=======
// 650rc1
//=======

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hclScratchBufferDefinition", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hclBufferDefinition")
	HK_PATCH_MEMBER_ADDED("storeNormals", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("storeTangentsAndBiTangents", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("triangleIndices", TYPE_ARRAY_INT, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hclBufferDefinition", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hclMeshMeshDeformOperator", 0, "hclMeshMeshDeformOperator", 1)
	HK_PATCH_MEMBER_ADDED("startVertex", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("endVertex", TYPE_INT, HK_NULL, 0)
	HK_PATCH_FUNCTION(hclMeshMeshDeformOperator_0_to_1)
HK_PATCH_END()

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
