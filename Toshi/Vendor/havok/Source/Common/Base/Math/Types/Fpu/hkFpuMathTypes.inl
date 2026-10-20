/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

typedef hkReal hkSimdReal;
typedef hkReal hkSimdRealParameter;

class hkVector4;
typedef const hkVector4& hkVector4Parameter;

#define HK_SIMD_REAL(a) hkSimdReal(a)

HK_FORCE_INLINE void hkSimdReal_setMax(  hkSimdRealParameter a, hkSimdRealParameter b, hkSimdReal& result ) 
{
	result = (a>b)?a:b;
}

HK_FORCE_INLINE void hkSimdReal_setMin(  hkSimdRealParameter a, hkSimdRealParameter b, hkSimdReal& result ) 
{
	result = (a<b)?a:b;
}

HK_FORCE_INLINE void hkSimdReal_store(  hkSimdRealParameter a, float *f) 
{
	*f = a;
}

#define HK_TRANSPOSE4_SWAP(a,b) t = a; a = b; b = t
#define HK_TRANSPOSE4(v0,v1,v2,v3) { hkReal t; \
	HK_TRANSPOSE4_SWAP( v0(1), v1(0) ); \
	HK_TRANSPOSE4_SWAP( v0(2), v2(0) ); \
	HK_TRANSPOSE4_SWAP( v0(3), v3(0) ); \
	HK_TRANSPOSE4_SWAP( v1(2), v2(1) ); \
	HK_TRANSPOSE4_SWAP( v1(3), v3(1) ); \
	HK_TRANSPOSE4_SWAP( v2(3), v3(2) ); }

#define HK_TRANSPOSE3_SWAP(a,b) t = a; a = b; b = t
#define HK_TRANSPOSE3(v0,v1,v2) { hkReal t; \
	HK_TRANSPOSE3_SWAP( v0(1), v1(0) ); \
	HK_TRANSPOSE3_SWAP( v0(2), v2(0) ); \
	HK_TRANSPOSE3_SWAP( v1(2), v2(1) ); }

struct hkQuadReal
{
	HK_ALIGN16( hkReal x );
	hkReal y;
	hkReal z;
	hkReal w;
};

#define HK_QUADREAL_CONSTANT(a,b,c,d) {a,b,c,d}

union hkQuadRealUnion
{
	hkReal r[4];
	hkQuadReal q;
};

#define HK_SIMD_COMPARE_MASK_X 8

	/// Result of a hkVector4 comparison.
class hkVector4Comparison
{
	public:

		enum Mask
		{
			MASK_NONE	= 0,
			MASK_W		= 1,
			MASK_Z		= 2,
			MASK_ZW		= 3,

			MASK_Y		= 4,
			MASK_YW		= 5,
			MASK_YZ		= 6,
			MASK_YZW	= 7,

			MASK_X		= 8,
			MASK_XW		= 9,
			MASK_XZ		= 10,
			MASK_XZW	= 11,

			MASK_XY		= 12,
			MASK_XYW	= 13,
			MASK_XYZ	= 14,
			MASK_XYZW	= 15
		};

		static const Mask s_components[4];

		void setAnd( hkVector4Comparison a, hkVector4Comparison b ) { m_mask = a.m_mask & b.m_mask; }
		void set( Mask m ) { m_mask = m; }
		void setOr( hkVector4Comparison a, hkVector4Comparison b ) { m_mask = a.m_mask | b.m_mask; }
		void setNot( hkVector4Comparison a ) { m_mask = (~a.m_mask) & MASK_XYZW ; }

		HK_FORCE_INLINE void setSelect( hkVector4Comparison a, hkVector4Comparison b, hkVector4Comparison comp) 
		{
			m_mask = (comp.m_mask & b.m_mask) | (~comp.m_mask & a.m_mask);
		}

		hkBool32 allAreSet( Mask m ) const { return (m_mask & m) == m; }
		hkBool32 anyIsSet( Mask m ) const { return m_mask & m; }

		hkBool32 allAreSet() const { return m_mask == MASK_XYZW; }
		hkBool32 anyIsSet() const { return m_mask; }

		int getMask() const { return m_mask; }
		int getMask(Mask m) const { return m_mask & m; }

	private:

		int m_mask;
		friend class hkVector4;
};

typedef hkVector4Comparison hkVector4ComparisonParameter;

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
