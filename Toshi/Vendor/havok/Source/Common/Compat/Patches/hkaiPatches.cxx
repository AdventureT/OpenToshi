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

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiTriangleMesh", 0)
	HK_PATCH_MEMBER_ADDED("triangles", TYPE_ARRAY_STRUCT, "hkaiTriangleMeshTriangle", 0)
	HK_PATCH_MEMBER_ADDED("vertices", TYPE_ARRAY_VEC_4, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkaiTriangleMeshTriangle", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiDefaultNavMeshCostModifier", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkReferencedObject")
	HK_PATCH_MEMBER_ADDED("edgeMaskLookupTable", TYPE_TUPLE_INT, HK_NULL, 32)
	HK_PATCH_MEMBER_ADDED("maxCostPenalty", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiNavMeshCutter", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkReferencedObject")
	HK_PATCH_MEMBER_ADDED("originalNumFaces", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("originalNumEdges", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("originalNumVertices", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("magic", TYPE_ARRAY_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("mesh", TYPE_OBJECT, "hkaiNavMesh", 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkaiNavMesh", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiWorld", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkReferencedObject")
	HK_PATCH_MEMBER_ADDED("up", TYPE_VEC_4, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("navMesh", TYPE_OBJECT, "hkaiNavMesh", 0)
	HK_PATCH_MEMBER_ADDED("characters", TYPE_ARRAY_OBJECT, "hkaiCharacter", 0)
	HK_PATCH_MEMBER_ADDED("bounds", TYPE_STRUCT, "hkAabb", 0)
	HK_PATCH_MEMBER_ADDED("dynamicNavMesh", TYPE_STRUCT, "hkaiNavMesh", 0)
	HK_PATCH_MEMBER_ADDED("cutter", TYPE_STRUCT, "hkaiNavMeshCutter", 0)
	HK_PATCH_MEMBER_ADDED("cutOptions", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("silhouetteQuantizationBits", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("silhouetteRadius", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkAabb", 0)
	HK_PATCH_DEPENDS("hkaiNavMesh", 0)
	HK_PATCH_DEPENDS("hkaiNavMeshCutter", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
	HK_PATCH_DEPENDS("hkaiCharacter", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiCutNavMeshQueryMediator", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkaiNavMeshQueryMediator")
	HK_PATCH_MEMBER_ADDED("cutter", TYPE_OBJECT, "hkaiNavMeshCutter", 0)
	HK_PATCH_MEMBER_ADDED("orig", TYPE_OBJECT, "hkaiNavMeshQueryMediator", 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkaiNavMeshCutter", 0)
	HK_PATCH_DEPENDS("hkaiNavMeshQueryMediator", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiNavMeshQueryMediator", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkReferencedObject")
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiNavMeshCostModifier", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiPathFollowingProperties", 0)
	HK_PATCH_MEMBER_ADDED("searchPathLimit", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("changeSegmentDist", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("desiredSpeedAtEnd", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("slowdownToEndDist", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("goalDistTolerance", TYPE_REAL, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiNavMeshUserEdge", 0)
	HK_PATCH_MEMBER_ADDED("startPoint", TYPE_VEC_4, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("endPoint", TYPE_VEC_4, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("faceIndex", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("oppositeFace", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("data", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("cost", TYPE_REAL, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiNavMeshEdge", 0)
	HK_PATCH_MEMBER_ADDED("a", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("b", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("oppositeEdge", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("faceIndex", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("oppositeFace", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("vertexClearance", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("flags", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("data", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiTriangleMeshTriangle", 0)
	HK_PATCH_MEMBER_ADDED("userData", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("vertexIndices", TYPE_TUPLE_INT, HK_NULL, 3)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiNavMeshFace", 0)
	HK_PATCH_MEMBER_ADDED("startEdgeIndex", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("startUserEdgeIndex", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("numEdges", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("numUserEdges", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("regionId", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("data", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("flags", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiNavMesh", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkReferencedObject")
	HK_PATCH_MEMBER_ADDED("faces", TYPE_ARRAY_STRUCT, "hkaiNavMeshFace", 0)
	HK_PATCH_MEMBER_ADDED("edges", TYPE_ARRAY_STRUCT, "hkaiNavMeshEdge", 0)
	HK_PATCH_MEMBER_ADDED("vertices", TYPE_ARRAY_VEC_4, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("userEdges", TYPE_ARRAY_STRUCT, "hkaiNavMeshUserEdge", 0)
	HK_PATCH_MEMBER_ADDED("numRegions", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("queryMediator", TYPE_OBJECT, "hkaiNavMeshQueryMediator", 0)
	HK_PATCH_DEPENDS("hkaiNavMeshFace", 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkaiNavMeshEdge", 0)
	HK_PATCH_DEPENDS("hkaiNavMeshQueryMediator", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
	HK_PATCH_DEPENDS("hkaiNavMeshUserEdge", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiCharacter", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkReferencedObject")
	HK_PATCH_MEMBER_ADDED("position", TYPE_VEC_4, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("forward", TYPE_VEC_4, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("goalPosition", TYPE_VEC_4, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("velocity", TYPE_VEC_4, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("desiredSpeed", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("angularVelocity", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("id", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("pathFollowingProperties", TYPE_STRUCT, "hkaiPathFollowingProperties", 0)
	HK_PATCH_MEMBER_ADDED("state", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("currentPathSegment", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("startNode", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("goalNode", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("debugDisplayEnabled", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkaiPathFollowingProperties", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

// *****    DONE    *****

//=======
// 650r1
//=======

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiPath", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkReferencedObject")
	HK_PATCH_MEMBER_ADDED("positions", TYPE_ARRAY_VEC_4, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("navMeshEdges", TYPE_ARRAY_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("startNode", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("goalNode", TYPE_INT, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiAvoidanceSolverMovementProperties", 0)
	HK_PATCH_MEMBER_ADDED("minVelocity", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("maxVelocity", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("maxAcceleration", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("maxDeceleration", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("maxAngularVelocity", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("maxAngularAcceleration", TYPE_REAL, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiAvoidanceSolverAvoidanceProperties", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkReferencedObject")
	HK_PATCH_MEMBER_ADDED("movementProperties", TYPE_STRUCT, "hkaiAvoidanceSolverMovementProperties", 0)
	HK_PATCH_MEMBER_ADDED("radius", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("weight", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("cosViewAngle", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("agentLimitTime", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("agentPenaltyWeight", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("obstacleLimitTime", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("obstaclePenaltyWeight", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("boundaryLimitTime", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("boundaryPenaltyWeight", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
	HK_PATCH_DEPENDS("hkaiAvoidanceSolverMovementProperties", 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkaiCharacter", 0, "hkaiCharacter", 1)
	HK_PATCH_MEMBER_ADDED("path", TYPE_OBJECT, "hkaiPath", 0)
	HK_PATCH_MEMBER_ADDED("avoidanceProperties", TYPE_OBJECT, "hkaiAvoidanceSolverAvoidanceProperties", 0)
	HK_PATCH_DEPENDS("hkaiPath", 0)
HK_PATCH_END()

//=======
// 660b1
//=======

HK_PATCH_BEGIN("hkaiWorld", 0, "hkaiWorld", 1)
	HK_PATCH_MEMBER_ADDED("characterRadiusMultiplier", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("repathingThreshold", TYPE_REAL, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkaiWorld", 1, "hkaiWorld", 2)
	HK_PATCH_MEMBER_ADDED("maxObjectSize", TYPE_REAL, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkaiCutNavMeshQueryMediator", 0, "hkaiCutNavMeshQueryMediator", 1)
	HK_PATCH_MEMBER_ADDED("cutAabbTolerance", TYPE_REAL, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkaiPath", 0, "hkaiPath", 1)
	HK_PATCH_MEMBER_ADDED("navMesh", TYPE_OBJECT, "hkaiNavMesh", 0)
	HK_PATCH_DEPENDS("hkaiNavMesh", 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkaiAvoidanceSolverMovementProperties", 0, "hkaiAvoidanceSolverMovementProperties", 1)
	HK_PATCH_MEMBER_ADDED("maxTurnVelocity", TYPE_REAL, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiNavMeshCutterDebugSnapshot", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkReferencedObject")
	HK_PATCH_MEMBER_ADDED("cutter", TYPE_OBJECT, "hkaiNavMeshCutter", 0)
	HK_PATCH_MEMBER_ADDED("sils", TYPE_ARRAY_OBJECT, "hkaiCompoundSilhouette", 0)
	HK_PATCH_MEMBER_ADDED("faces", TYPE_ARRAY_INT, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkaiNavMeshCutter", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
	HK_PATCH_DEPENDS("hkaiCompoundSilhouette", 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiCompoundSilhouette", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkReferencedObject")
	HK_PATCH_MEMBER_ADDED("points", TYPE_ARRAY_VEC_4, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("new", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("dirty", TYPE_BYTE, HK_NULL, 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkaiNavMeshCutter", 0, "hkaiNavMeshCutter", 1)
	HK_PATCH_MEMBER_ADDED("stepHeight", TYPE_REAL, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkaiCharacter", 1, "hkaiCharacter", 2)
	HK_PATCH_MEMBER_ADDED("agentFilterInfo", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiCarver", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkReferencedObject")
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()


HK_PATCH_BEGIN("hkaiCharacter", 2, "hkaiCharacter", 3)
	HK_PATCH_MEMBER_ADDED("potentialGoalPositions", TYPE_ARRAY_VEC_4, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("goalPositionSet", TYPE_BYTE, HK_NULL, 0)
HK_PATCH_END()


HK_PATCH_BEGIN("hkaiWorld", 2, "hkaiWorld", 3)
	HK_PATCH_MEMBER_ADDED("maxCharRepathPerFrame", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("cutCounter", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("projectionMode", TYPE_BYTE, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkaiCharacter", 3, "hkaiCharacter", 4)
	HK_PATCH_MEMBER_ADDED("userData", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("repathDelay", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkaiPath", 1, "hkaiPath", 2)
	
HK_PATCH_END()

HK_PATCH_BEGIN("hkaiPath", 2, "hkaiPath", 3)
	HK_PATCH_MEMBER_ADDED("hasUserEdges", TYPE_BYTE, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkaiAvoidanceSolverAvoidanceProperties", 0, "hkaiAvoidanceSolverAvoidanceProperties", 1)
	HK_PATCH_MEMBER_ADDED("limitObstacleDistance", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("limitObstacleVelocity", TYPE_REAL, HK_NULL, 0)
	HK_PATCH_FUNCTION(hkaiAvoidanceSolverAvoidanceProperties_0_to_1)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiEdgeGeometryEdge", 0)
	HK_PATCH_MEMBER_ADDED("a", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("b", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("face", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("data", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiEdgeGeometryFace", 0)
	HK_PATCH_MEMBER_ADDED("data", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("faceIndex", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN(HK_NULL, HK_CLASS_ADDED, "hkaiEdgeGeometry", 0)
	HK_PATCH_PARENT_SET(HK_NULL, "hkReferencedObject")
	HK_PATCH_MEMBER_ADDED("edges", TYPE_ARRAY_STRUCT, "hkaiEdgeGeometryEdge", 0)
	HK_PATCH_MEMBER_ADDED("faces", TYPE_ARRAY_STRUCT, "hkaiEdgeGeometryFace", 0)
	HK_PATCH_MEMBER_ADDED("vertices", TYPE_ARRAY_VEC_4, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("zeroFace", TYPE_STRUCT, "hkaiEdgeGeometryFace", 0)
	HK_PATCH_DEPENDS("hkBaseObject", 0)
	HK_PATCH_DEPENDS("hkaiEdgeGeometryEdge", 0)
	HK_PATCH_DEPENDS("hkaiEdgeGeometryFace", 0)
	HK_PATCH_DEPENDS("hkReferencedObject", 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkaiAvoidanceSolverAvoidanceProperties", 1, "hkaiAvoidanceSolverAvoidanceProperties", 2)
	HK_PATCH_MEMBER_ADDED("limitBoundaryDistance", TYPE_REAL, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkaiNavMeshEdge", 0, "hkaiNavMeshEdge", 1)
	HK_PATCH_MEMBER_REMOVED("faceIndex", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("oppositeSectionUid", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("oppositeRuntimeId", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()

HK_PATCH_BEGIN("hkaiNavMesh", 0, "hkaiNavMesh", 1)
	HK_PATCH_MEMBER_ADDED("sectionDependencies", TYPE_ARRAY_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("sectionUid", TYPE_INT, HK_NULL, 0)
	HK_PATCH_MEMBER_ADDED("runtimeId", TYPE_INT, HK_NULL, 0)
HK_PATCH_END()


/*
* Havok SDK - NO SOURCE PC DOWNLOAD, BUILD(#20090704)
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
