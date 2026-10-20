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

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkLocalFrameGroup", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkReferencedObject")
	HK_PATCH_MEMBER_ADDED("name", TYPE_CSTRING, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkSemanticsAttribute", 0)
	HK_PATCH_MEMBER_ADDED("type", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkRangeInt32Attribute", 0)
	HK_PATCH_MEMBER_ADDED("absmin", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("absmax", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("softmin", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("softmax", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkMeshVertexBuffer", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkReferencedObject")
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkArrayTypeAttribute", 0)
	HK_PATCH_MEMBER_ADDED("type", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkDemoReplayUtilityFrame", 0)
	HK_PATCH_MEMBER_ADDED("keysDown", TYPE_STRUCT, "hkBitField", 0)
	HK_PATCH_MEMBER_ADDED("camera", TYPE_STRUCT, "hkDemoReplayUtilityCamera", 0)
	HK_PATCH_MEMBER_ADDED("mouseX", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("mouseY", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("padButtons", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("mouseButtons", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("sticks", TYPE_TUPLE_REAL, HK_NULL, 4)
	HK_PATCH_DEPENDS("hkBitField", 0)
	HK_PATCH_DEPENDS("hkDemoReplayUtilityCamera", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkRangeRealAttribute", 0)
	HK_PATCH_MEMBER_ADDED("absmin", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("absmax", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("softmin", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("softmax", TYPE_REAL, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkDataObjectTypeAttribute", 0)
	HK_PATCH_MEMBER_ADDED("typeName", TYPE_CSTRING, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkDemoReplayUtilityMouseCallbacks", 0)
	HK_PATCH_MEMBER_ADDED("mouseMoveFunc", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("mouseWheelFunc", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("mouseButtonFunc", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkDemoReplayUtilityReplayData", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkReferencedObject")
	HK_PATCH_MEMBER_ADDED("framesWereAllocatedExternally", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("frames", TYPE_ARRAY_OBJECT, "hkDemoReplayUtilityFrame", 0)
	HK_PATCH_MEMBER_ADDED("initialMouseX", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("initialMouseY", TYPE_INT, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
	HK_PATCH_DEPENDS("hkDemoReplayUtilityFrame", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkLinkAttribute", 0)
	HK_PATCH_MEMBER_ADDED("type", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkDemoReplayUtility", 0)
	HK_PATCH_MEMBER_ADDED("replayData", TYPE_OBJECT, "hkDemoReplayUtilityReplayData", 0)
	HK_PATCH_MEMBER_ADDED("currentFrame", TYPE_INT, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkDemoReplayUtilityReplayData", 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkDescriptionAttribute", 0)
	HK_PATCH_MEMBER_ADDED("string", TYPE_CSTRING, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkMeshShape", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkReferencedObject")
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkGizmoAttribute", 0)
	HK_PATCH_MEMBER_ADDED("visible", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("label", TYPE_CSTRING, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("type", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkMeshSectionCinfo", 0)
	HK_PATCH_MEMBER_ADDED("vertexBuffer", TYPE_OBJECT, "hkMeshVertexBuffer", 0)
	HK_PATCH_MEMBER_ADDED("material", TYPE_OBJECT, "hkMeshMaterial", 0)
	HK_PATCH_MEMBER_ADDED("primitiveType", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("numPrimitives", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("indexType", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("indices", TYPE_OBJECT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("vertexStartIndex", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("transformIndex", TYPE_INT, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkMeshVertexBuffer", 0)
	HK_PATCH_DEPENDS("hkMeshMaterial", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkUiAttribute", 0)
	HK_PATCH_MEMBER_ADDED("visible", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("hideInModeler", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("label", TYPE_CSTRING, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("group", TYPE_CSTRING, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("endGroup", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("advanced", TYPE_BYTE, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkDocumentationAttribute", 0)
	HK_PATCH_MEMBER_ADDED("docsSectionTag", TYPE_CSTRING, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkMemoryMeshShape", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkMeshShape")
	HK_PATCH_MEMBER_ADDED("sections", TYPE_ARRAY_STRUCT, "hkMeshSectionCinfo", 0)
	HK_PATCH_MEMBER_ADDED("indices16", TYPE_ARRAY_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("indices32", TYPE_ARRAY_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("name", TYPE_CSTRING, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkMeshShape", 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
	HK_PATCH_DEPENDS("hkMeshSectionCinfo", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkMeshMaterial", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkReferencedObject")
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkMeshSection", 0)
	HK_PATCH_MEMBER_ADDED("primitiveType", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("numPrimitives", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("numIndices", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("vertexStartIndex", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("transformIndex", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("indexType", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("indices", TYPE_OBJECT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("vertexBuffer", TYPE_OBJECT, "hkMeshVertexBuffer", 0)
	HK_PATCH_MEMBER_ADDED("material", TYPE_OBJECT, "hkMeshMaterial", 0)
	HK_PATCH_MEMBER_ADDED("sectionIndex", TYPE_INT, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkMeshVertexBuffer", 0)
	HK_PATCH_DEPENDS("hkMeshMaterial", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkModelerNodeTypeAttribute", 0)
	HK_PATCH_MEMBER_ADDED("type", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkHalf", 0)
	HK_PATCH_MEMBER_ADDED("value", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkDemoReplayUtilityCamera", 0)
	HK_PATCH_MEMBER_ADDED("from", TYPE_TUPLE_REAL, HK_NULL, 3)
	HK_PATCH_MEMBER_ADDED("to", TYPE_TUPLE_REAL, HK_NULL, 3)
	HK_PATCH_MEMBER_ADDED("up", TYPE_TUPLE_REAL, HK_NULL, 3)
HK_PATCH_END()

HK_PATCH_BEGIN("hkSimpleLocalFrame", 0, "hkSimpleLocalFrame", 1)
	HK_PATCH_MEMBER_ADDED("group", TYPE_OBJECT, "hkLocalFrameGroup", 0)
	HK_PATCH_DEPENDS("hkLocalFrameGroup", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkAlignSceneToNodeOptions", 0)
	HK_PATCH_MEMBER_ADDED("invert", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("transformPositionX", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("transformPositionY", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("transformPositionZ", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("transformRotation", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("transformScale", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("transformSkew", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("keyframe", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("nodeName", TYPE_ARRAY_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkxVertexP4N4C1T10", 0, "hkxVertexP4N4C1T10", 1) 
	HK_PATCH_MEMBER_RENAMED("diffuse", "diffuseA")
	HK_PATCH_MEMBER_REMOVED("padding", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("diffuseB", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkxVertexP4N4T4B4W4I4C1T12", 0, "hkxVertexP4N4T4B4W4I4C1T12", 1) 
	HK_PATCH_MEMBER_RENAMED("diffuse", "diffuseA")
	HK_PATCH_MEMBER_REMOVED("padding", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("diffuseB", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkxVertexP4N4C1T2", 0, "hkxVertexP4N4C1T2", 1) 
	HK_PATCH_MEMBER_RENAMED("diffuse", "diffuseA")
	HK_PATCH_MEMBER_REMOVED("padding", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("diffuseB", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkxVertexP4N4C1T6", 0, "hkxVertexP4N4C1T6", 1) 
	HK_PATCH_MEMBER_RENAMED("diffuse", "diffuseA")
	HK_PATCH_MEMBER_REMOVED("padding", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("diffuseB", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkxVertexP4N4T4B4C1T2", 0, "hkxVertexP4N4T4B4C1T2", 1) 
	HK_PATCH_MEMBER_RENAMED("diffuse", "diffuseA")
	HK_PATCH_MEMBER_REMOVED("padding", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("diffuseB", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkxVertexP4N4T4B4C1T6", 0, "hkxVertexP4N4T4B4C1T6", 1) 
	HK_PATCH_MEMBER_RENAMED("diffuse", "diffuseA")
	HK_PATCH_MEMBER_REMOVED("padding", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("diffuseB", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkxVertexP4N4T4B4C1T10", 0, "hkxVertexP4N4T4B4C1T10", 1) 
	HK_PATCH_MEMBER_RENAMED("diffuse", "diffuseA")
	HK_PATCH_MEMBER_REMOVED("padding", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("diffuseB", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkxVertexP4N4T4B4W4I4C1T4", 0, "hkxVertexP4N4T4B4W4I4C1T4", 1) 
	HK_PATCH_MEMBER_RENAMED("diffuse", "diffuseA")
	HK_PATCH_MEMBER_REMOVED("padding", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("diffuseB", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkxVertexP4N4T4B4W4I4C1T8", 0, "hkxVertexP4N4T4B4W4I4C1T8", 1) 
	HK_PATCH_MEMBER_RENAMED("diffuse", "diffuseA")
	HK_PATCH_MEMBER_REMOVED("padding", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("diffuseB", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()


HK_PATCH_BEGIN("hkxVertexP4N4W4I4C1T4", 0, "hkxVertexP4N4W4I4C1T4", 1)
	HK_PATCH_MEMBER_RENAMED("diffuse", "diffuseA")
	HK_PATCH_MEMBER_REMOVED("padding", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("diffuseB", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkxVertexP4N4W4I4C1T8", 0, "hkxVertexP4N4W4I4C1T8", 1)
	HK_PATCH_MEMBER_RENAMED("diffuse", "diffuseA")
	HK_PATCH_MEMBER_REMOVED("padding", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("diffuseB", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkxVertexP4N4W4I4C1T12", 0, "hkxVertexP4N4W4I4C1T12", 1)
	HK_PATCH_MEMBER_RENAMED("diffuse", "diffuseA")
	HK_PATCH_MEMBER_REMOVED("padding", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("diffuseB", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkUiAttribute", 0, "hkUiAttribute", 1)
	HK_PATCH_MEMBER_ADDED("endGroup2", TYPE_BYTE, HK_NULL, 0)
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
