/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#include <xmmintrin.h>
#define HK_TRANSPOSE4(A,B,C,D) _MM_TRANSPOSE4_PS(A,B,C,D)

#define HK_TRANSPOSE3(_a,_b,_c) { \
	hkQuadReal _tmp, _ctmp;	\
	_tmp = _mm_unpacklo_ps(_a,_b);	\
	HK_VECTOR4_SHUF(_ctmp,_mm_unpackhi_ps(_a,_b),_c,HK_VECTOR4_SHUFFLE(0,1,2,3));	\
	_a = _mm_movelh_ps(_tmp,_c);	\
	HK_VECTOR4_SHUF(_b,_mm_movehl_ps(_a,_tmp),_c,HK_VECTOR4_SHUFFLE(0,1,1,3));	\
	_c = _ctmp; }

#define HK_VECTOR4_SHUFFLE(_a,_b,_c,_d) _MM_SHUFFLE(_d,_c,_b,_a)
#define HK_VECTOR4_SHUF( tgt, src0, src1, shuf ) tgt = _mm_shuffle_ps(src0, src1, shuf)

#define HK_VECTOR4_PERM1ARG(_a,_b,_c,_d) _MM_SHUFFLE(_d,_c,_b,_a)
#define HK_VECTOR4_PERM1( tgt, src, shuf ) tgt = _mm_shuffle_ps(src, src, shuf)

typedef __m128 hkQuadReal;

class hkVector4;
typedef const hkVector4& hkVector4Parameter;

union hkQuadRealUnion
{
	hkReal r[4];
	hkQuadReal q;
};

#define HK_QUADREAL_CONSTANT(a,b,c,d) {a,b,c,d}
#define HK_SIMD_REAL(a) hkSimdReal(a)
class hkSimdReal
{
	public:

		hkSimdReal(const hkQuadReal& x)
			: m_real(x)
		{
		}

		hkSimdReal(hkReal x)
		{
			m_real = _mm_load_ss(&x);
		}

		hkSimdReal(){}

		operator hkReal() const
		{
			hkReal s;
			_mm_store_ss(&s, m_real);
			return s;
		}

		HK_FORCE_INLINE hkQuadReal broadcast() const
		{
			return _mm_shuffle_ps(m_real, m_real, 0);
		}

		HK_FORCE_INLINE const hkQuadReal& getQuad() const
		{
			return m_real;
		}

	private:

		hkQuadReal m_real;
};

typedef const hkSimdReal& hkSimdRealParameter;

HK_FORCE_INLINE void hkSimdReal_setMax(  hkSimdRealParameter a, hkSimdRealParameter b, hkSimdReal& result ) 
{
	result = hkSimdReal(_mm_max_ps( a.getQuad(), b.getQuad() ));
}

HK_FORCE_INLINE void hkSimdReal_setMin(  hkSimdRealParameter a, hkSimdRealParameter b, hkSimdReal& result ) 
{
	result = hkSimdReal(_mm_min_ps( a.getQuad(), b.getQuad() ));
}


HK_FORCE_INLINE void hkSimdReal_store(  hkSimdRealParameter a, float *f) 
{
	_mm_store_ss( f, a.getQuad() );
}

inline hkSimdReal HK_CALL operator* (hkSimdRealParameter r, hkSimdRealParameter s)
{
	return _mm_mul_ss(r.getQuad(),s.getQuad());
}

inline hkSimdReal HK_CALL operator- (hkSimdRealParameter r, hkSimdRealParameter s)
{
	return _mm_sub_ss(r.getQuad(),s.getQuad());
}

inline hkSimdReal HK_CALL operator+ (hkSimdRealParameter r, hkSimdRealParameter s)
{
	return _mm_add_ss(r.getQuad(),s.getQuad());
}

inline hkSimdReal HK_CALL operator/ (hkSimdRealParameter r, hkSimdRealParameter s)
{
	return _mm_div_ss(r.getQuad(),s.getQuad());
}

inline hkSimdReal HK_CALL operator- (hkSimdRealParameter r)
{
    extern const hkQuadReal hkQuadReal0000;
	return _mm_sub_ss(hkQuadReal0000,r.getQuad());
}

class hkVector4Comparison;
typedef const hkVector4Comparison& hkVector4ComparisonParameter;

	/// Result of a hkVector4 comparison.
class hkVector4Comparison;
typedef const hkVector4Comparison& hkVector4ComparisonParameter;

class hkVector4Comparison
{
	public:

		enum Mask
		{
			MASK_NONE	= 0,
			MASK_X		= 1,
			MASK_Y		= 2,
			MASK_XY		= 3,

			MASK_Z		= 4,
			MASK_XZ		= 5,
			MASK_YZ		= 6,
			MASK_XYZ	= 7,

			MASK_W		= 8,
			MASK_XW		= 9,
			MASK_YW		= 10,
			MASK_XYW	= 11,

			MASK_ZW		= 12,
			MASK_XZW	= 13,
			MASK_YZW	= 14,
			MASK_XYZW	= 15
		};

		static const Mask s_components[4];

		HK_FORCE_INLINE void setAnd( hkVector4ComparisonParameter a, hkVector4ComparisonParameter b ) { m_mask = _mm_and_ps( a.m_mask,b.m_mask ); }
		HK_FORCE_INLINE void setOr( hkVector4ComparisonParameter a, hkVector4ComparisonParameter b ) { m_mask = _mm_or_ps( a.m_mask,b.m_mask ); }
		HK_FORCE_INLINE void setNot( hkVector4ComparisonParameter a )
		{ 
			static HK_ALIGN16( const hkUint32 allBits[4] ) = { 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF };		
			m_mask = _mm_andnot_ps( a.m_mask, *(const hkQuadReal*)&allBits); 
		}

		HK_FORCE_INLINE void setSelect( hkVector4ComparisonParameter a, hkVector4ComparisonParameter b, hkVector4ComparisonParameter comp) 
		{
			m_mask = _mm_or_ps( _mm_and_ps(comp.m_mask, b.m_mask), _mm_andnot_ps(comp.m_mask, a.m_mask) );
		}

		static const hkQuadReal s_maskFromBits[MASK_XYZW+1];
		static const hkQuadReal s_invMaskFromBits[MASK_XYZW+1];

		HK_FORCE_INLINE void set( Mask m) { m_mask = s_maskFromBits[m]; }

		HK_FORCE_INLINE hkBool32 allAreSet( Mask m ) const { return (_mm_movemask_ps(m_mask) & m) == m; }
		HK_FORCE_INLINE hkBool32 anyIsSet( Mask m ) const { return _mm_movemask_ps(m_mask) & m; }

		HK_FORCE_INLINE hkBool32 allAreSet() const { return _mm_movemask_ps(m_mask) == MASK_XYZW; }
		HK_FORCE_INLINE hkBool32 anyIsSet() const { return _mm_movemask_ps(m_mask); }

		HK_FORCE_INLINE int getMask() const { return _mm_movemask_ps(m_mask); }
		HK_FORCE_INLINE int getMask(Mask m) const { return _mm_movemask_ps(m_mask) & m; }

	private:

		hkQuadReal m_mask;
		friend class hkVector4;
};

#define HK_SIMD_COMPARE_MASK_X 1

extern const hkQuadReal hkQuadRealHalf;
extern const hkQuadReal hkQuadReal3333;

namespace hkMath
{
	inline int HK_CALL isNegative(const hkSimdReal& r0)
	{
		return _mm_movemask_ps(r0.getQuad()) & hkVector4Comparison::MASK_X;
	}

	inline hkSimdReal HK_CALL sqrt(hkSimdRealParameter r)
	{
		return _mm_sqrt_ss(r.getQuad());
	}

#	define HK_MATH_hkToIntFast
		// Fast rounding, however last bit might be wrong
	inline int HK_CALL hkToIntFast( hkReal r ){
		int i;
		_asm {
			fld r
			fistp i
		}
		return i;
	}

#	define HK_MATH_prefetch128
	inline void prefetch128( const void* p)
	{
		_mm_prefetch( (const char*)p, _MM_HINT_NTA );
	}

#	define HK_MATH_forcePrefetch
	template<int SIZE>
	inline void forcePrefetch( const void* p )
	{
		const char* q = (const char*)p;
		_mm_prefetch( q, _MM_HINT_NTA );
		if ( SIZE > 64){  _mm_prefetch( q + 64, _MM_HINT_NTA ); }
		if ( SIZE > 128){ _mm_prefetch( q + 128, _MM_HINT_NTA ); }
		if ( SIZE > 192){ _mm_prefetch( q + 192, _MM_HINT_NTA ); }
	}

    inline hkQuadReal quadReciprocal( hkQuadReal r )
    {
		hkQuadReal e = _mm_rcp_ps( r );
		//One round of Newton-Raphson refinement
        return _mm_sub_ps(_mm_add_ps(e,e), _mm_mul_ps(_mm_mul_ps(e, r), e));
    }

    inline hkQuadReal quadReciprocalSquareRoot( hkQuadReal r )
    {
		hkQuadReal e = _mm_rsqrt_ps(r);
		hkQuadReal he = _mm_mul_ps(hkQuadRealHalf,e);
		hkQuadReal ree = _mm_mul_ps(_mm_mul_ps(r,e),e);
		return _mm_mul_ps(he, _mm_sub_ps(hkQuadReal3333, ree) );
    }
}

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
