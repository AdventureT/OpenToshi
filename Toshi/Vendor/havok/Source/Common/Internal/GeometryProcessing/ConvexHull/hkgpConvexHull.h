/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKGP_CONVEX_HULL_H
#define HKGP_CONVEX_HULL_H

class hkAabb;
struct hkStridedVertices; 

/// Default cosinus of the angle above which two plane are merged
#define HKGPCONVEXHULL_DEFAULT_MIN_COS_ANGLE			0.9999f

/// Default thickness used for splitting
#define HKGPCONVEXHULL_DEFAULT_SPLIT_THICKNESS			0.0000001f


/// Convex hull generation, also expose utilities such as connectivity, mass properties, boolean operations.
/// Fast enought to be called at runtime on moderate input size ( < 10000 points )
///
/// Handle convex hull of dimensions up to 3 such as:
///
///  - 0D : Point
///  - 1D : Line
///  - 2D : Plane convex hull
///  - 3D : Volume convex hull
///
/// Notes A): Internaly, the convex hull store two set of coordinates for each input points, 
///           the internal representation may differe slightly from the actual inputs.
///           For that reason, some methods need to be called with the coordinates type to be used,
///           in this case, 'Source' refer to the input coordinates used to build the convex hull.
///
/// Notes B): Sources are never modified, including the 'W' component.
/// 
class hkgpConvexHull : public hkReferencedObject
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_GEOMETRY);
		struct	Vertex		{};
		struct	Triangle	{};		
	public:
				
		/// Build configuration structure
		struct BuildConfig
		{
			BuildConfig();
			hkBool		m_allowLowerDimensions;			///
			hkReal		m_minCosAngle;					///
			hkBool		m_setSourceIndices;				///
			hkBool		m_buildIndices;					///
			hkBool		m_buildMassProperties;			///
			hkBool		m_sortInputs;					///
			hkBool		m_internalInputs;				///
			
			/// Internally the algorithm might modifies input coordinates to ensure numericaly robust results.
			/// As a resuls the planes equation might not exactly confine the input points set.
			/// Setting this value to true will make sure the plane equations enclose both sources and internals points representations. (default: false).
			hkBool		m_ensurePlaneEnclosing;			
		};

		/// Simplification configuration structure
		struct SimplifyConfig
		{
			SimplifyConfig();
			int			m_maxVertices;					/// See 'decimateVertices', if less then zero, do not simplify.
			hkBool		m_ensureContainment;			/// See 'decimateVertices'.
			hkReal		m_minArea;						/// Minimum triangle area.
			hkReal		m_minEdgeLength;				/// Minimum edge length.
			hkBool		m_removeUnreferencedVertices;	/// Remove vertices not referenced by hkgpConvexHull::generateIndexedFaces.
			hkBool		m_forceRebuild;					/// Force rebuild after simplification.
		};

		/// Input coordinates to use
		enum Inputs
		{
			SOURCE_VERTICES,							///
			INTERNAL_VERTICES,							///
		};

		/// Result of 'classify' and 'splitByPlane'
		enum Side
		{
			ZERO		,								///
			POSITIVE	,								///
			NEGATIVE	,								///
			CROSSING	,								///
			INVALID										///
		};

		/// ByteArray
		typedef hkArray<hkUint8>	ByteArray;

	public:

		/// ctor
		hkgpConvexHull();

		/// Destructor.
		~hkgpConvexHull();
		
		/// Get a unique identifier for that convex hull.
		hkUint32			uid() const;

		/// Reset the convex hull to an empty set of dimension-1.
		void				reset();
		
		/// Clone the convex hull, userData and properties.
		hkgpConvexHull*		clone() const;

		/// Get user data.
		void*&				userData() const;		

		/// Get a array of bytes that can be used to store properties
		ByteArray&			properties();

		/// Get a array of bytes that can be used to store properties
		const ByteArray&	properties() const;

		/// Build a convex hull given a points set, return the resulting convex hull dimension or -1 is failed.
		int					build(const hkVector4* points, int numPoints, const BuildConfig& config=BuildConfig());

		/// Build a convex hull given a points set, return the resulting convex hull dimension or -1 is failed.
		int					build(const hkStridedVertices& points, const BuildConfig& config=BuildConfig());

		/// Build a planar convex hull given a points set and a projection plane, return the resulting convex hull dimension or -1 is failed.
		int					buildPlanar(const hkVector4* points, int numPoints, const hkVector4& plane, const BuildConfig& config=BuildConfig());

		/// Build a planar convex hull given a points set and a projection plane, return the resulting convex hull dimension or -1 is failed.
		int					buildPlanar(const hkStridedVertices& points, const hkVector4& plane, const BuildConfig& config=BuildConfig());

		/// Build a convex hull given a planes set, return the resulting convex hull dimension or -1 is failed.
		int					buildFromPlanes(const hkVector4* planes, int numPlanes, const BuildConfig& config=BuildConfig());

		/// Simplify a convex hull
		void				simplify(const SimplifyConfig& config);

		/// Expand the convex hull to ensure that it contains \a points
		void				ensureContainment(const hkArray<hkVector4>& points);

		/// Decimate vertices of a convex hull.
		/// If \a ensureContainment is true, ensureContainment() is called automatically
		bool				decimateVertices(int numVerticesToKeep, bool ensureContainment);

		/// Expand convex hull with margin using face normals
		bool				expandByPlanarMargin(const hkReal margin);
		
		/// Assign plane,vertex and triangle indices.
		/// This data is needed for e.g. getIndex(), getPlaneIndex() and generateIndexedFaces()
		/// Planes are merged if PAxyz.PBxyz > minCosAngle
		/// Note: this is automatically called by build is BuildConfig.m_buildIndices is set (default)
		void				buildIndices(hkReal	minCosAngle = HKGPCONVEXHULL_DEFAULT_MIN_COS_ANGLE, bool forceRebuild=false);

		/// Calculate and store mass properties. See getVolume(), getLocalInertia()
		hkResult			buildMassProperties( );		

		/// Check whatever a point is a vertex of the convex hull
		bool				isVertexUsed(const hkVector4& point) const;

		/// Check whatever a point is inside or on the convex hull
		/// If you want to get maximum performance, call buildIndices() and set \a usePlanes to true.
		/// All planes will be expanded by \a planeThickness.
		bool				isPointInside( const hkVector4& point, bool usePlanes = false, hkReal planeThickness=0) const;

		/// Clip in lines against planes, return false in the line is completely outside the convex hull
		/// All planes will be expanded by \a planeThickness.
		bool				clipLine(const hkVector4& a, const hkVector4& b, hkReal& t0, hkReal& t1, hkReal planeThickness=0) const;

		/// Retrieve the maximum distance to the planes of a given point
		hkReal				maximumDistanceToPlanes(const hkVector4& point) const;

		/// Retrieve configuration used to build the convex hull.
		const BuildConfig&	getConfiguration() const;

		/// Get the number of dimensions of the convex hull, -1 for invalid.
		int					getDimensions() const;

		/// Get the axis-aligned bounding box of the convex hull.
		hkAabb				getBoundingBox(Inputs inputs, hkReal scale=1, hkReal radialScale=0) const;

		/// Get the oriented bounding box of the convex hull		
		void				getOrientedBoundingBox(hkVector4& halfExtents, hkTransform& worldTransform) const;

		/// Get the oriented rectangle of the convex hull given a projection axis
		void				getOrientedRectangle(const hkVector4& projectionPlane, hkVector4& halfExtents, hkTransform& worldTransform) const;

		/// Get the projection axis, have meaning only for convex hull of dimension 1.
		hkVector4			getProjectionAxis() const;
		
		/// Get the projection plane, have meaning only for convex hull of dimension 2.
		hkVector4			getProjectionPlane() const;
		
		/// Retrieve the total number of planes which is the number of faces.
		int					getNumPlanes() const;

		/// Retrieve a plane by index.
		const hkVector4&	getPlane(int index) const;
		
		/// Retrieve the total number of vertices
		int					getNumVertices() const;

		/// Retrieve the first vertex of the convex hull
		Vertex*				getFirstVertex() const;

		/// Retrieve vertex index.
		int					getIndex(const Vertex* vertex) const;

		/// Retrieve the next vertex, which is part of the convex hull
		Vertex*				getNext(const Vertex* vertex) const;

		/// Retrieve the world position of a vertex
		hkVector4			getPosition(const Vertex* vertex) const;

		/// Retrieve the source position (used for building) of a vertex
		const hkVector4&	getSourcePosition(const Vertex* vertex) const;

		/// Retrieve the total number of triangles.
		int					getNumTriangles() const;

		/// Retrieve the next triangle.
		Triangle*			getFirstTriangle() const;

		/// Retrieve triangle index.
		int					getIndex(const Triangle* triangle) const;

		/// Retrieve the next triangle.
		Triangle*			getNext(const Triangle* triangle) const;

		/// Retrieve the plane index of a triangle.
		int					getPlaneIndex(const Triangle* triangle) const;

		/// Retrieve the plane equation of a triangle.
		hkVector4			getPlaneEquation(const Triangle* triangle) const;
		
		/// Retrieve a vertex of triangle.
		Vertex*				getVertex(const Triangle* triangle, int index) const;

		/// Retrieve the neighbor triangle given an edge index.
		Triangle*			getNeighbour(Triangle* triangle, int index) const;

		/// Get 'width*|direction|' of the convex hull.
		hkReal				getWidth(const hkVector4& direction) const;

		/// Get the minimum distance from a plane.
		hkReal				getMinimumDistanceFromPlane(const hkVector4& plane) const;

		/// Get the maximum distance from a plane
		hkReal				getMaximumDistanceFromPlane(const hkVector4& plane) const;
		
		/// Support mapping
		void				getSupportingVertex(const hkVector4& direction,hkVector4& vertexOut) const;

		/// Get the centroid of the convex hull
		const hkVector4&	getCentroid() const;

		/// Return the sum of all edge lengths
		hkReal				getEdgeLengths() const;

		/// Return the Sphericity of the convex hull
		hkReal				getSphericity() const;

		/// Return the volume of the convex hull
		/// You need to call buildMassProperties() before you call this.
		hkReal				getVolume() const;

		/// Return the surface area of the convex hull
		/// You need to call buildMassProperties() before you call this.
		hkReal				getSurfaceArea() const;

		/// Retrieve inertia space and inertia diagonal
		/// You need to call buildMassProperties() before you call this.
		void				getLocalInertia(hkTransform& inertiaSpace, hkVector4& inertiaDiagonal) const;

		/// Retrieve world inertia matrix
		/// You need to call buildMassProperties() before you call this.
		const hkMatrix3&	getWorldInertia() const;

		/// Retrieve the center of mass
		/// You need to call buildMassProperties() before you call this.
		const hkVector4&	getCenterOfMass() const;

		/// Retrieve major (world) plane/axis from inertia
		/// You need to call buildMassProperties() before you call this.
		hkVector4			getMajorPlane() const;

		/// Retrieve widths sorted planes/axis from inertia
		void				getMajorPlanes(hkVector4 planesOut[3]) const;
		
		/// Classify relative to a plane and retrieve distance to plane bounds
		Side				classify(const hkVector4& plane, hkReal thickness, hkReal* bounds=0) const;

		/// Split convex hull by a plane
		/// Note: Even in the case where the return value is CROSSING, pos, neg or both can be null is they are degenerated.
		/// Handling of those cases are application dependent.
		Side				splitByPlane(const hkVector4& plane, hkgpConvexHull*& posOut, hkgpConvexHull*& negOut, hkReal cuttingPlaneThickness=HKGPCONVEXHULL_DEFAULT_SPLIT_THICKNESS) const;

		/// Recursively subdivide the convex hull along its major axis
		void				subdivide(int recursions, hkArray<hkgpConvexHull*>& output, hkReal thickness=HKGPCONVEXHULL_DEFAULT_SPLIT_THICKNESS) const;

		/// Fetch all vertex positions.
		void				fetchPositions(Inputs inputs, hkArray<hkVector4>& positionsOut) const;

		/// Fetch all planes
		void				fetchPlanes(hkArray<hkVector4>& planes) const;

		/// Fetch bevel planes
		void				fetchBevelPlanes( hkReal maxCosAngle, hkArray<hkVector4>& bevelPlanesOut ) const;
		
		/// Generate indexed faces, return the number of faces/planes. This data is what is needed by hkpConvexVerticesConnectivity
		/// Note: if inputs == SOURCES, indices will be the one stored in source position with hkVector4::setInt24W
		int					generateIndexedFaces(Inputs inputs, hkArray<hkUint8>& verticesPerFace, hkArray<hkUint16>& indices) const;

		/// Generate indexed faces, return the number of faces/planes.
		/// Note: if inputs == SOURCES, is set, indices will be the one stored in source position with hkVector4::setInt24W
		int					generateIndexedFaces(Inputs inputs, hkArray<int>& verticesPerFace, hkArray<int>& indices) const;

		/// Generate vertex adjacency graph as COUNT,EDGES for each vertex
		void				generateVertexAdjacencyGraph(hkArray<int>& edgesOut, hkArray<int>& vertexOffsetsOut) const;

		/// Generate hkGeometry
		void				generateGeometry(Inputs inputs, struct hkGeometry& geometryOut) const;
		
		//
		// Boolean operations
		//

		/// Boolean split of A with B
		static void HK_CALL				booleanSplit(const hkgpConvexHull* operandA, const hkgpConvexHull* operandB, hkgpConvexHull** inside, hkArray<hkgpConvexHull*>* outside, hkReal thickness=HKGPCONVEXHULL_DEFAULT_SPLIT_THICKNESS);
		
		/// Boolean intersection of two convex hull (A & B)
		static bool HK_CALL 			booleanIntersection(const hkgpConvexHull* operandA, const hkgpConvexHull* operandB, hkgpConvexHull*& output, hkReal thickness=HKGPCONVEXHULL_DEFAULT_SPLIT_THICKNESS);

		/// Boolean subtraction of two convex hull (A - B)
		static bool HK_CALL 			booleanSubtraction(const hkgpConvexHull* operandA, const hkgpConvexHull* operandB, hkArray<hkgpConvexHull*>& output, hkReal thickness=HKGPCONVEXHULL_DEFAULT_SPLIT_THICKNESS);

		//
		// Utilities
		//		
		
		/// Create compound convex hull from a given set
		static hkgpConvexHull* HK_CALL	createCompound(Inputs inputs, const hkgpConvexHull* const* hulls, int numHulls);

		/// Create compound convex hull from a given set
		static hkgpConvexHull* HK_CALL	createCompound(Inputs inputs, const hkgpConvexHull* a, const hkgpConvexHull* b);

	private:
		hkgpConvexHull(const hkgpConvexHull& ch)	: hkReferencedObject(ch) {}
		void			operator=(const hkgpConvexHull&)		{}

	private:

		friend class hkgpConvexHullImpl;
		class hkgpConvexHullImpl*	m_data;
		mutable void*				m_userData;
		ByteArray					m_properties;
};

#endif // HKGP_CONVEX_HULL_H

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
