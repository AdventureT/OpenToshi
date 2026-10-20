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

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkpStorageExtendedMeshShapeMaterial", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkpMeshMaterial")
	HK_PATCH_MEMBER_ADDED("restitution", TYPE_STRUCT, "hkHalf", 0)
	HK_PATCH_MEMBER_ADDED("friction", TYPE_STRUCT, "hkHalf", 0)
	HK_PATCH_MEMBER_ADDED("userData", TYPE_INT, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkpMeshMaterial", 0)
	HK_PATCH_DEPENDS("hkHalf", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkpFirstPersonGun", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkReferencedObject")
	HK_PATCH_MEMBER_ADDED("name", TYPE_CSTRING, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("keyboardKey", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkpBallGun", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkpFirstPersonGun")
	HK_PATCH_MEMBER_ADDED("bulletRadius", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("bulletVelocity", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("bulletMass", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("damageMultiplier", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("maxBulletsInWorld", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("bulletOffsetFromCenter", TYPE_VEC_4, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkpFirstPersonGun", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkpGravityGun", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkpFirstPersonGun")
	HK_PATCH_MEMBER_ADDED("maxNumObjectsPicked", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("maxMassOfObjectPicked", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("maxDistOfObjectPicked", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("impulseAppliedWhenObjectNotPicked", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("throwVelocity", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("capturedObjectPosition", TYPE_VEC_4, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("capturedObjectsOffset", TYPE_VEC_4, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkpFirstPersonGun", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkpProjectileGun", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkpFirstPersonGun")
	HK_PATCH_MEMBER_ADDED("maxProjectiles", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("reloadTime", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkpFirstPersonGun", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkpMountedBallGun", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkpBallGun")
	HK_PATCH_MEMBER_ADDED("position", TYPE_VEC_4, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkpBallGun", 0)
	HK_PATCH_DEPENDS("hkpFirstPersonGun", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkpShapeRayBundleCastInput", 0)
	HK_PATCH_MEMBER_ADDED("from", TYPE_TUPLE_VEC_4, HK_NULL, 3)
	HK_PATCH_MEMBER_ADDED("to", TYPE_TUPLE_VEC_4, HK_NULL, 3)
	HK_PATCH_MEMBER_ADDED("filterInfo", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("rayShapeCollectionFilter", TYPE_OBJECT, "hkpRayShapeCollectionFilter", 0)
	HK_PATCH_DEPENDS("hkpRayShapeCollectionFilter", 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkpExtendedMeshShape", 0, "hkpExtendedMeshShape", 1)
	HK_PATCH_MEMBER_ADDED("defaultCollisionFilterInfo", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkpExtendedMeshShapeTrianglesSubpart", 0, "hkpExtendedMeshShapeTrianglesSubpart", 1)
	HK_PATCH_MEMBER_ADDED("userData", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkpConstraintInstance", 0, "hkpConstraintInstance", 1)
	HK_PATCH_MEMBER_ADDED("destructionRemapInfo", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_FUNCTION(hkpConstraintInstance_0_to_1)
HK_PATCH_END()

HK_PATCH_BEGIN("hkpStorageExtendedMeshShapeShapeSubpartStorage", 0, "hkpStorageExtendedMeshShapeShapeSubpartStorage", 1)
	HK_PATCH_MEMBER_RENAMED("materials", "int_materials")
	HK_PATCH_MEMBER_ADDED("materials", TYPE_ARRAY_STRUCT, "hkpStorageExtendedMeshShapeMaterial", 0)
	HK_PATCH_FUNCTION(hkpStorageExtendedMeshShapeShapeSubpartStorage_0_to_1)
	HK_PATCH_MEMBER_REMOVED("int_materials", TYPE_ARRAY_INT, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkpMeshMaterial", 0)
	HK_PATCH_DEPENDS("hkpStorageExtendedMeshShapeMaterial", 0)
	HK_PATCH_DEPENDS("hkHalf", 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkpStorageExtendedMeshShapeMeshSubpartStorage", 0, "hkpStorageExtendedMeshShapeMeshSubpartStorage", 1)
	HK_PATCH_MEMBER_RENAMED("materials", "int_materials")
	HK_PATCH_MEMBER_ADDED("materials", TYPE_ARRAY_STRUCT, "hkpStorageExtendedMeshShapeMaterial", 0)
	HK_PATCH_FUNCTION(hkpStorageExtendedMeshShapeMeshSubpartStorage_0_to_1)
	HK_PATCH_MEMBER_REMOVED("int_materials", TYPE_ARRAY_INT, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkpMeshMaterial", 0)
	HK_PATCH_DEPENDS("hkpStorageExtendedMeshShapeMaterial", 0)
	HK_PATCH_DEPENDS("hkHalf", 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkpWorldCinfo", 0, "hkpWorldCinfo", 1)
	HK_PATCH_MEMBER_ADDED("useKdTree", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("autoUpdateKdTree", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_FUNCTION(hkpWorldCinfo_0_to_1)
HK_PATCH_END()

HK_PATCH_BEGIN("hkpMotion", 0, "hkpMotion", 1)
	HK_PATCH_MEMBER_RENAMED("savedMotion", "max_savedMotion")
	HK_PATCH_MEMBER_ADDED("savedMotion", TYPE_OBJECT, "hkpMotion", 0)
	HK_PATCH_MEMBER_ADDED("gravityFactor", TYPE_STRUCT, "hkHalf", 0)
	HK_PATCH_FUNCTION(hkpMotion_0_to_1)
	HK_PATCH_MEMBER_REMOVED("max_savedMotion", TYPE_OBJECT, "hkpMaxSizeMotion", 0)
	HK_PATCH_DEPENDS("hkpKeyframedRigidMotion", 0)
	HK_PATCH_DEPENDS("hkpMaxSizeMotion", 0)
	HK_PATCH_DEPENDS("hkHalf", 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkpEntity", 0, "hkpEntity", 1)
	HK_PATCH_MEMBER_RENAMED("motion", "max_motion")
	HK_PATCH_MEMBER_ADDED("motion", TYPE_STRUCT, "hkpMotion", 0)
	HK_PATCH_FUNCTION(hkpEntity_0_to_1)
	HK_PATCH_MEMBER_REMOVED("max_motion", TYPE_STRUCT, "hkpMaxSizeMotion", 0)
	HK_PATCH_DEPENDS("hkpMotion", 0)
	HK_PATCH_DEPENDS("hkpKeyframedRigidMotion", 0)
	HK_PATCH_DEPENDS("hkpMaxSizeMotion", 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkpPairwiseCollisionFilterCollisionPair", 0, HK_NULL, HK_CLASS_REMOVED)
	HK_PATCH_MEMBER_REMOVED("a", TYPE_OBJECT, "hkpEntity", 0)
	HK_PATCH_MEMBER_REMOVED("b", TYPE_OBJECT, "hkpEntity", 0)
	HK_PATCH_DEPENDS("hkpEntity", 1)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkpWorldObject", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkpPairwiseCollisionFilter", 0, HK_NULL, HK_CLASS_REMOVED)
	HK_PATCH_PARENT_SET("hkpCollisionFilter", HK_NULL)
	HK_PATCH_MEMBER_REMOVED("disabledPairs", TYPE_ARRAY_STRUCT, "hkpPairwiseCollisionFilterCollisionPair", 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
	HK_PATCH_DEPENDS("hkpCollisionFilter", 0)
	HK_PATCH_DEPENDS("hkpPairwiseCollisionFilterCollisionPair", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkpCenterOfMassChangerModifierConstraintAtom", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkpModifierConstraintAtom")
	HK_PATCH_MEMBER_ADDED("displacementA", TYPE_VEC_4, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("displacementB", TYPE_VEC_4, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkpModifierConstraintAtom", 0)
	HK_PATCH_DEPENDS("hkpConstraintAtom", 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkpMassChangerModifierConstraintAtom", 0, "hkpMassChangerModifierConstraintAtom", 1)
	HK_PATCH_MEMBER_RENAMED("factorA", "old_factorA")
	HK_PATCH_MEMBER_RENAMED("factorB", "old_factorB")
	HK_PATCH_MEMBER_ADDED("factorA", TYPE_VEC_4, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("factorB", TYPE_VEC_4, HK_NULL, 0)
	HK_PATCH_FUNCTION(hkpMassChangerModifierConstraintAtom_0_to_1)
	HK_PATCH_MEMBER_REMOVED("old_factorA", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_REMOVED("old_factorB", TYPE_REAL, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkpVehicleInstanceWheelInfo", 0, "hkpVehicleInstanceWheelInfo",1)
	HK_PATCH_MEMBER_RENAMED("contactShapeKey", "old_contactShapeKey")
	HK_PATCH_MEMBER_ADDED("contactShapeKey", TYPE_TUPLE_INT, HK_NULL, 8)
	HK_PATCH_FUNCTION(hkpVehicleInstanceWheelInfo_0_to_1)
	HK_PATCH_MEMBER_REMOVED("old_contactShapeKey", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkpWorldCinfo", 1, "hkpWorldCinfo", 2)
	HK_PATCH_MEMBER_ADDED("numToisTillAllowedPenetrationSimplifiedToi", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("numToisTillAllowedPenetrationToi", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("numToisTillAllowedPenetrationToiHigher", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("numToisTillAllowedPenetrationToiForced", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_FUNCTION(hkpWorldCinfo_1_to_2) // set defaults
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkpConstraintInstanceSmallArraySerializeOverrideType", 0)
	HK_PATCH_MEMBER_ADDED("data", TYPE_OBJECT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("size", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("capacityAndFlags", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkpBallSocketConstraintAtom", 0, "hkpBallSocketConstraintAtom", 1)
	HK_PATCH_MEMBER_ADDED("stabilizationFactor", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_FUNCTION(hkpBallSocketConstraintAtom_0_to_1) // set defaults
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkpBreakableBody", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkReferencedObject")
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkpStorageExtendedMeshShapeMeshSubpartStorage", 1, "hkpStorageExtendedMeshShapeMeshSubpartStorage", 2)
	HK_PATCH_MEMBER_ADDED("indices8", TYPE_ARRAY_BYTE, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkpExtendedMeshShapeTrianglesSubpart", 1, "hkpExtendedMeshShapeTrianglesSubpart", 2)
	HK_PATCH_FUNCTION(hkpExtendedMeshShapeTrianglesSubpart_1_to_2)
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
