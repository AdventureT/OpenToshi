/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_COLLIDE2_TRIANGLEUTIL_H
#define HK_COLLIDE2_TRIANGLEUTIL_H

extern hkReal hkDefaultTriangleDegeneracyTolerance;

/// Triangle utilities (ray intersection, normal calculation, point-in-triangle tests etc.) 
class hkpTriangleUtil
{
	public:

		

		// Utilities...

			/// Method to calculate the (non-normalized) triangle normal.
			/// Given the vertices, set normal to be the non-unit normal.
			/// \param normal   
			/// \param a   
			/// \param b   
			/// \param c   
		static inline void HK_CALL calcNormal( hkVector4& normal, const hkVector4& a, const hkVector4& b, const hkVector4& c );


			/// Method to calculate the triangle centroid.
			/// Given the vertices list, set the centroid to be the centroid of the three vertices.
			/// \param centroid   
			/// \param a   
			/// \param b   
			/// \param c 
		static inline void HK_CALL calcCentroid( hkVector4& centroid, const hkVector4& a, const hkVector4& b, const hkVector4& c );


			/// Returns true if the point is in front of the Triangle.
			/// Given the plane in which the triangle is embedded, the point is in front if (and only if) 
			/// the point is in the half space (defined by the plane) that the normal points into.
			/// \param point   
			/// \param a   
			/// \param b   
			/// \param c   
		static inline hkBool HK_CALL inFront( const hkVector4& point, const hkVector4& a, const hkVector4& b, const hkVector4& c );


			/// Return true if the point is contained within the Triangle.
			/// Assumes that the point is on the plane containing the Triangle.
			/// \param pt   
			/// \param a   
			/// \param b   
			/// \param c  
		static inline hkBool HK_CALL containsPoint( const hkVector4 &pt, const hkVector4& a, const hkVector4& b, const hkVector4& c );


			/// Returns the plane constant (d) for this Triangle
		static inline hkReal HK_CALL calcPlaneConstant( const hkVector4& a, const hkVector4& b, const hkVector4& c );


			/// Performs a ray-triangle intersection test.
			/// Returns true if the given ray r=pos+t*dir intersects the triangle  Returns t,
			/// and u,v (planar coordinates  on the triangle).
			/// If perform_culling=true, back-face culling is performed, i.e. if t gets negative, the
			/// method returns false.
			///
			/// \param t               (output) distance to triangle if hit.
			/// \param u               (output) barycentric coordinate on triangle if hit.
			/// \param v               (output) barycentric coordinate on triangle if hit.
			/// \param pos             (input) origin of ray.
			/// \param dir             (input) direction of ray.
			/// \param a               First vertex.
			/// \param b               Second vertex.
			/// \param c               Third vertex.
			/// \param perform_culling If true backface culling will be performed.
		static hkBool HK_CALL rayIntersect( hkReal &t, hkReal &u, hkReal &v, const hkVector4 &pos, const hkVector4 &dir,
								  const hkVector4& a, const hkVector4& b, const hkVector4& c, hkBool perform_culling=false );

			/// Returns true if the triangle is degenerate.
			/// Degenerate is assumed to be:
			///  * it has very small area (cross product of edges all squared less than given tolerance).
			///  * it has a aspect ratio which will cause collision detection algorithms to fail.
			/// \param a               First vertex.
			/// \param b               Second vertex.
			/// \param c               Third vertex.
			/// \param tolerance	   Minimal acceptable area and squared edge length
		static hkBool HK_CALL isDegenerate(const hkVector4& a, const hkVector4& b, const hkVector4& c , hkReal tolerance = hkDefaultTriangleDegeneracyTolerance);

	protected:

		hkpTriangleUtil();

};

#include <Physics/Collide/Util/hkpTriangleUtil.inl>

#endif // HK_COLLIDE2_TRIANGLEUTIL_H

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
