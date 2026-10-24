/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


/* quad, here for inlining */

inline hkQuadReal& hkVector4::getQuad()
{
	return m_quad;
}

inline const hkQuadReal& hkVector4::getQuad() const
{
	return m_quad;
}

#ifndef HK_DISABLE_VECTOR4_CONSTRUCTORS
/* construct, assign, zero */
inline hkVector4::hkVector4(hkReal a, hkReal b, hkReal c, hkReal d)
{
	m_quad = _mm_setr_ps(a,b,c,d);
}

inline hkVector4::hkVector4(const hkQuadReal& q)
{
	m_quad = q;
}

inline hkVector4::hkVector4(const hkVector4& v)
{
	m_quad = v.m_quad;
}
#endif

inline void hkVector4::operator= (const hkVector4& v)
{
	m_quad = v.m_quad;
}


inline void hkVector4::set(hkReal a, hkReal b, hkReal c, hkReal d)
{
	m_quad = _mm_setr_ps(a,b,c,d);
}

inline void hkVector4::setAll(hkSimdRealParameter a)
{
	m_quad = _mm_shuffle_ps( a.getQuad(), a.getQuad(), _MM_SHUFFLE(0,0,0,0));
}

inline void hkVector4::setAll(hkReal a)
{
	m_quad = _mm_set_ps1(a);
}

inline void hkVector4::setAll3(hkReal a)
{
	m_quad = _mm_set_ps1(a);
}

inline void hkVector4::setZero4()
{
	m_quad = _mm_setzero_ps();
}

void hkVector4::setInt24W( int value )
{
	(reinterpret_cast<hkUint32*>(this))[3] = value |= 0x3f000000;
}

// Gets the .w component as an integer with 24 bit resolution (for real experts only)
int hkVector4::getInt24W( ) const 
{
	return (reinterpret_cast<const hkUint32*>(this))[3] & ~0x3f000000;
}

inline void hkVector4::storeUncached( void* dest) const
{
	_mm_stream_ps( (float*)dest, this->m_quad );
}


inline void hkVector4::zeroElement( int i )
{
	HK_ASSERT(0x3bc36625, i>=0 && i<4);
	static HK_ALIGN16( const hkUint32 cx[4][4] ) ={ 
	{ 0x00000000, 0xffffffff, 0xffffffff, 0xffffffff },
	{ 0xffffffff, 0x00000000, 0xffffffff, 0xffffffff },
	{ 0xffffffff, 0xffffffff, 0x00000000, 0xffffffff },
	{ 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000 } };
	m_quad = _mm_and_ps( m_quad, reinterpret_cast<const hkQuadReal*>(cx)[i] );
}


/* vector methods */

inline void hkVector4::add4(const hkVector4& v)
{
	m_quad = _mm_add_ps( m_quad, v.m_quad );
}
inline void hkVector4::add3clobberW(const hkVector4& v)
{
	m_quad = _mm_add_ps( m_quad, v.m_quad );
}

inline void hkVector4::setAdd4(const hkVector4& v0, const hkVector4& v1)
{
	m_quad = _mm_add_ps(v0.m_quad, v1.m_quad);
}

inline void hkVector4::sub4(const hkVector4& v)
{
	m_quad = _mm_sub_ps( m_quad, v.m_quad );
}

inline void hkVector4::sub3clobberW(const hkVector4& v)
{
	m_quad = _mm_sub_ps( m_quad, v.m_quad );
}

inline void hkVector4::setSub4(const hkVector4& v0, const hkVector4& v1)
{
	m_quad = _mm_sub_ps(v0.m_quad, v1.m_quad);
}

inline void hkVector4::mul4(const hkVector4& v)
{              
	m_quad = _mm_mul_ps( m_quad, v.m_quad );
}

inline void hkVector4::setNeg3(const hkVector4& v)
{
	static HK_ALIGN16( const hkUint32 negateMask[4] ) = { 0x80000000, 0x80000000, 0x80000000, 0x00000000 };
	m_quad = _mm_xor_ps(v.m_quad, *(const hkQuadReal*)&negateMask);
}


inline void hkVector4::setMul4(const hkVector4& a, const hkVector4& b)
{
	m_quad = _mm_mul_ps( a.m_quad, b.m_quad );
}

inline void hkVector4::mul4(hkSimdRealParameter s)
{              
	m_quad = _mm_mul_ps( s.broadcast(), m_quad );
}

inline void hkVector4::setMul4(hkSimdRealParameter r, const hkVector4& v1)
{
	m_quad = _mm_mul_ps( r.broadcast(), v1.m_quad);
}

inline void hkVector4::addMul4(hkSimdRealParameter r, const hkVector4& v1)
{
	m_quad = _mm_add_ps( m_quad,
			_mm_mul_ps( r.broadcast(), v1.m_quad) );
}

inline void hkVector4::addMul4(const hkVector4& x, const hkVector4& y)
{
	m_quad = _mm_add_ps( m_quad,
				_mm_mul_ps( x.m_quad, y.m_quad) );
}

inline void hkVector4::subMul4(hkSimdRealParameter r, const hkVector4& v1)
{
	m_quad = _mm_sub_ps( m_quad,
			_mm_mul_ps( r.broadcast(), v1.m_quad) );
}

inline void hkVector4::subMul4(const hkVector4& x, const hkVector4& y)
{
	m_quad = _mm_sub_ps( m_quad,
		_mm_mul_ps( x.m_quad, y.m_quad) );
}

inline void hkVector4::setAddMul4(const hkVector4& a, const hkVector4& b, hkSimdRealParameter r)
{
	m_quad = _mm_add_ps( a.m_quad,
			 _mm_mul_ps( r.broadcast(), b.m_quad) );
}

inline void hkVector4::setAddMul4(const hkVector4& a, const hkVector4& x, const hkVector4& y)
{
	m_quad = _mm_add_ps( a.m_quad,
			 _mm_mul_ps( x.m_quad, y.m_quad) );
}

inline void hkVector4::setCross( const hkVector4& v0, const hkVector4& v1 )
{
	hkQuadReal cross0;
	cross0 = _mm_mul_ps(
			_mm_shuffle_ps(v0.m_quad, v0.m_quad, _MM_SHUFFLE(3,0,2,1)),
			_mm_shuffle_ps(v1.m_quad, v1.m_quad, _MM_SHUFFLE(3,1,0,2)) );
	hkQuadReal cross1;
	cross1 = _mm_mul_ps(
			_mm_shuffle_ps(v0.m_quad, v0.m_quad, _MM_SHUFFLE(3,1,0,2)),
			_mm_shuffle_ps(v1.m_quad, v1.m_quad, _MM_SHUFFLE(3,0,2,1)) );
	m_quad = _mm_sub_ps(cross0, cross1);
}

inline void hkVector4::setInterpolate4(const hkVector4& v0, const hkVector4& v1, hkSimdRealParameter _t)
{
	static HK_ALIGN16( const float cw[4] ) = { 1.0f, 1.0f, 1.0f, 1.0f };
	hkQuadReal t = _t.broadcast();
	hkQuadReal s = _mm_sub_ps( *(const hkQuadReal*)&cw, t);
	m_quad = _mm_add_ps(
			_mm_mul_ps(s, v0.m_quad),
			_mm_mul_ps(t, v1.m_quad) );
}

inline hkVector4Comparison hkVector4::compareLessThan4(hkVector4Parameter a) const
{
	hkVector4Comparison m; m.m_mask =_mm_cmplt_ps(m_quad, a.m_quad);
	return m;
}

inline hkVector4Comparison hkVector4::compareEqual4(hkVector4Parameter a) const
{
	hkVector4Comparison m; m.m_mask =_mm_cmpeq_ps(m_quad, a.m_quad);
	return m;
}

inline hkVector4Comparison hkVector4::compareLessThanEqual4(const hkVector4 &a) const
{
	hkVector4Comparison m; m.m_mask =_mm_cmple_ps(m_quad, a.m_quad);
	return m;
}

inline hkVector4Comparison hkVector4::compareLessThanZero4() const
{
	hkVector4 zero; zero.setZero4();
	hkVector4Comparison m; m.m_mask =_mm_cmplt_ps(m_quad, zero);
	return m;
}

inline hkBool32 hkVector4::allLessThan3(hkVector4Parameter a) const
{
	return (_mm_movemask_ps( _mm_cmplt_ps(m_quad, a.m_quad) ) & hkVector4Comparison::MASK_XYZ) == hkVector4Comparison::MASK_XYZ;
}

inline hkBool32 hkVector4::allLessThan4(hkVector4Parameter a) const
{
	return _mm_movemask_ps( _mm_cmplt_ps(m_quad, a.m_quad) ) == hkVector4Comparison::MASK_XYZW;
}

inline void hkVector4::select32( hkVector4Parameter a, hkVector4Parameter b, const hkVector4Comparison& comp)
{
	m_quad = _mm_or_ps( _mm_and_ps(comp.m_mask, b.m_quad), _mm_andnot_ps(comp.m_mask, a.m_quad) );
}

inline void hkVector4::setNeg4(const hkVector4& v)
{
	static HK_ALIGN16( const hkUint32 negateMask[4] ) = { 0x80000000, 0x80000000, 0x80000000, 0x80000000 };
	m_quad = _mm_xor_ps(v.m_quad, *(const hkQuadReal*)&negateMask);
}

inline void hkVector4::setAbs4(const hkVector4& v)
{
	static HK_ALIGN16( const hkUint32 absMask[4] ) = { 0x7fffffff, 0x7fffffff, 0x7fffffff, 0x7fffffff };
	m_quad = _mm_and_ps(v.m_quad, *(const hkQuadReal*)&absMask);
}

inline void hkVector4::setMin4(const hkVector4& a, const hkVector4& b)
{
	m_quad = _mm_min_ps(a.getQuad(), b.getQuad());
}

inline void hkVector4::setMax4(const hkVector4& a, const hkVector4& b)
{
	m_quad = _mm_max_ps(a.getQuad(), b.getQuad());
}

/* matrix3, rotation, quaternion, transform */

inline void hkVector4::_setMul3(const hkMatrix3& a, const hkVector4& b )
{
	const hkQuadReal b0 = _mm_shuffle_ps( b.m_quad, b.m_quad, _MM_SHUFFLE(0,0,0,0));
	const hkQuadReal r0 = _mm_mul_ps( a.getColumn(0).m_quad, b0 );
	const hkQuadReal b1 = _mm_shuffle_ps( b.m_quad, b.m_quad, _MM_SHUFFLE(1,1,1,1));
	const hkQuadReal r1 = _mm_mul_ps( a.getColumn(1).m_quad, b1 );
	const hkQuadReal b2 = _mm_shuffle_ps( b.m_quad, b.m_quad, _MM_SHUFFLE(2,2,2,2));	
	const hkQuadReal r2 = _mm_mul_ps( a.getColumn(2).m_quad, b2 );

	m_quad = _mm_add_ps( _mm_add_ps(r0, r1), r2 );
}

inline void hkVector4::_setMul4(const hkMatrix3& r, const hkVector4& v)
{
	_setMul3(r,v);
}


inline void hkVector4::_setRotatedDir (const hkRotation& r, const hkVector4& v)
{
	_setMul3(r,v);
}

inline void hkVector4::_setRotatedInverseDir(const hkRotation& r, const hkVector4& b)
{
	
	
	
	// - horizontal add is 15+a few more shuffles.
	hkQuadReal c0 = r.getColumn(0).m_quad;
	hkQuadReal c1 = r.getColumn(1).m_quad;
	hkQuadReal c2 = r.getColumn(2).m_quad;
	HK_TRANSPOSE3(c0,c1,c2);

	const hkQuadReal b0 = _mm_shuffle_ps( b.m_quad, b.m_quad, _MM_SHUFFLE(0,0,0,0));
	const hkQuadReal r0 = _mm_mul_ps( c0, b0 );
	const hkQuadReal b1 = _mm_shuffle_ps( b.m_quad, b.m_quad, _MM_SHUFFLE(1,1,1,1));
	const hkQuadReal r1 = _mm_mul_ps( c1, b1 );
	const hkQuadReal b2 = _mm_shuffle_ps( b.m_quad, b.m_quad, _MM_SHUFFLE(2,2,2,2));
	const hkQuadReal r2 = _mm_mul_ps( c2, b2 );

	m_quad = _mm_add_ps( _mm_add_ps(r0, r1), r2 );
}


inline void hkVector4::_setTransformedPos(const hkTransform& t, const hkVector4& b)
{
	const hkRotation& r = t.getRotation();
	hkQuadReal b0 = _mm_shuffle_ps( b.m_quad, b.m_quad, _MM_SHUFFLE(0,0,0,0));
	hkQuadReal r0 = _mm_mul_ps( r.getColumn(0).m_quad, b0 );
	hkQuadReal b1 = _mm_shuffle_ps( b.m_quad, b.m_quad, _MM_SHUFFLE(1,1,1,1) );
	hkQuadReal r1 = _mm_mul_ps( r.getColumn(1).m_quad, b1 );
	hkQuadReal b2 = _mm_shuffle_ps( b.m_quad, b.m_quad, _MM_SHUFFLE(2,2,2,2));
	hkQuadReal r2 = _mm_mul_ps( r.getColumn(2).m_quad, b2 );
	m_quad = _mm_add_ps( _mm_add_ps(r0, r1), _mm_add_ps(r2, t.getTranslation().m_quad ) );
}

inline void hkVector4::_setMul4xyz1(const hkTransform& t, const hkVector4& v)
{
	_setTransformedPos( t, v );
}



inline void hkVector4::_setTransformedInversePos(const hkTransform& a, const hkVector4& b)
{
	hkVector4 t0; t0.setSub4( b, a.getTranslation() );
	_setRotatedInverseDir(a.getRotation(), t0);
}

inline hkSimdReal hkVector4::dot3(const hkVector4& a) const
{
	hkQuadReal x2 = _mm_mul_ps(m_quad,a.m_quad);
	hkQuadReal xySum = _mm_add_ss( _mm_shuffle_ps(x2,x2,_MM_SHUFFLE(1,1,1,1)), x2);
	return _mm_add_ss( _mm_shuffle_ps(x2,x2,_MM_SHUFFLE(2,2,2,2)), xySum);
}

inline void hkVector4::setDot3(hkVector4Parameter a, hkVector4Parameter b)
{
	hkQuadReal x2 = _mm_mul_ps(a.m_quad, b.m_quad);
	hkQuadReal xySum = _mm_add_ss( _mm_shuffle_ps(x2,x2,_MM_SHUFFLE(1,1,1,1)), x2);
	m_quad = _mm_add_ss( _mm_shuffle_ps(x2,x2,_MM_SHUFFLE(2,2,2,2)), xySum);
}

inline hkReal hkVector4::dot3fpu(const hkVector4& a) const
{
	const hkVector4& t = *this;
	return (t(0) * a(0)) + ( t(1) * a(1)) + ( t(2) * a(2) );
}

inline hkSimdReal hkVector4::dot4(const hkVector4& a) const
{
	hkQuadReal x2 = _mm_mul_ps(m_quad,a.m_quad);
	hkQuadReal sum0 = _mm_add_ps( _mm_shuffle_ps(x2,x2,_MM_SHUFFLE(1,0,3,2)), x2); // w+z w+z x+y x+y
	hkQuadReal sum1 = _mm_shuffle_ps(sum0,sum0, _MM_SHUFFLE(2,3,0,1)); // x+y x+y w+z w+z
	return _mm_add_ps( sum0, sum1 ); // x+y+w+z all 4
}
inline void hkVector4::setDot4(hkVector4Parameter a, hkVector4Parameter b)
{
	hkQuadReal x2 = _mm_mul_ps(a.m_quad, b.m_quad);
	hkQuadReal sum0 = _mm_add_ps( _mm_shuffle_ps(x2,x2,_MM_SHUFFLE(1,0,3,2)), x2); // w+z w+z x+y x+y
	hkQuadReal sum1 = _mm_shuffle_ps(sum0,sum0, _MM_SHUFFLE(2,3,0,1)); // x+y x+y w+z w+z
	m_quad = _mm_add_ps( sum0, sum1 ); // x+y+w+z all 4
}

inline hkSimdReal hkVector4::dot4xyz1(const hkVector4& a) const
{
	hkQuadReal x2 = _mm_mul_ps(m_quad,a.m_quad);
	hkQuadReal xySum = _mm_add_ss( _mm_shuffle_ps(x2,x2,_MM_SHUFFLE(1,1,1,1)), x2);
	hkQuadReal zwSum = _mm_add_ss( _mm_shuffle_ps(x2,x2,_MM_SHUFFLE(2,2,2,2)), _mm_shuffle_ps(m_quad, m_quad, _MM_SHUFFLE(3,3,3,3)));
	return _mm_add_ps( xySum, zwSum ); // x+y+w+z all 4
}

inline hkSimdReal hkVector4::horizontalAdd3() const
{
	hkQuadReal xySum = _mm_add_ss( _mm_shuffle_ps(m_quad,m_quad,_MM_SHUFFLE(1,1,1,1)), m_quad);
	return _mm_add_ss( _mm_shuffle_ps(m_quad,m_quad,_MM_SHUFFLE(2,2,2,2)), xySum);
}

inline hkSimdReal hkVector4::horizontalAdd4() const
{
	hkQuadReal sum0 = _mm_add_ps( _mm_shuffle_ps(m_quad,m_quad,_MM_SHUFFLE(1,0,3,2)), m_quad); // w+z w+z x+y x+y
	hkQuadReal sum1 = _mm_shuffle_ps(sum0,sum0, _MM_SHUFFLE(2,3,0,1)); // x+y x+y w+z w+z
	return _mm_add_ps( sum0, sum1 ); // x+y+w+z all 4
}

inline hkSimdReal hkVector4::length3() const
{
	return _mm_sqrt_ss( this->dot3(*this).getQuad() );
}

inline hkSimdReal hkVector4::lengthSquared3() const
{
	return this->dot3(*this);
}

inline hkSimdReal hkVector4::lengthInverse3() const
{
	//HK_ASSERT2(0xf0435654, this->lengthSquared3() > 0, "hkVector4 too small for lengthInverse3");

	const hkQuadReal half  = _mm_set_ss(0.5f);
	const hkQuadReal three = _mm_set_ss(3.0f);
	const hkQuadReal zero  = _mm_set_ss(0.0f);

	const hkQuadReal len2 = this->dot3(*this).getQuad();

	const hkQuadReal len2EqualsZero = _mm_cmpeq_ss(len2, zero);

	const hkQuadReal approx = _mm_rsqrt_ss(len2);
	const hkQuadReal refined = _mm_mul_ss(
			_mm_mul_ss(half, approx),
			_mm_sub_ss(three, _mm_mul_ss( _mm_mul_ss(len2, approx), approx ) ) );


	// Ensures that we return 0 if the length is 0
	return _mm_andnot_ps(len2EqualsZero, refined);
}

inline hkSimdReal hkVector4::length4() const
{
	return _mm_sqrt_ss( this->dot4(*this).getQuad() );
}

inline hkSimdReal hkVector4::lengthSquared4() const
{
	return this->dot4(*this);
}

inline hkSimdReal hkVector4::lengthInverse4() const
{
	const hkQuadReal half  = _mm_set_ss(0.5f);
	const hkQuadReal three = _mm_set_ss(3.0f);
	const hkQuadReal zero  = _mm_set_ss(0.0f);

	const hkQuadReal len2 = this->dot4(*this).getQuad();
	const hkQuadReal len2EqualsZero = _mm_cmpeq_ss(len2, zero);

	const hkQuadReal approx = _mm_rsqrt_ss(len2);
	const hkQuadReal refined = _mm_mul_ss(
			_mm_mul_ss(half, approx),
			_mm_sub_ss(three, _mm_mul_ss( _mm_mul_ss(len2, approx), approx ) ) );

	// Ensures that we return 0 if the length is 0
	return _mm_andnot_ps(len2EqualsZero, refined);
}

inline void hkVector4::normalize3()
{
	hkQuadReal lenInv = lengthInverse3().broadcast();
	m_quad = _mm_mul_ps(lenInv, m_quad);
	//HK_ASSERT2(0xf0435f54, this->lengthSquared3() > 0.5f, "hkVector4 cannot be normalized");
}

inline hkSimdReal hkVector4::normalizeWithLength3()
{
	HK_ASSERT2(0xf043f654, this->lengthSquared3() > 0, "hkVector4 too small for lengthInverse3");

	const hkQuadReal half  = _mm_set_ss(0.5f);
	const hkQuadReal three = _mm_set_ss(3.0f);

	hkQuadReal len2 = this->dot3(*this).getQuad();
	hkQuadReal approx = _mm_rsqrt_ss(len2);
	hkQuadReal lenInv = _mm_mul_ss(
			_mm_mul_ss(half, approx),
			_mm_sub_ss(three, _mm_mul_ss( _mm_mul_ss(len2, approx), approx ) ) );
	lenInv = _mm_shuffle_ps(lenInv, lenInv, _MM_SHUFFLE(0,0,0,0) );
	m_quad = _mm_mul_ps(lenInv, m_quad);
	return _mm_mul_ss(len2, lenInv); // quicker to return x^2/x
}

inline void hkVector4::normalize4()
{
	hkQuadReal lenInv = lengthInverse4().broadcast();
	m_quad = _mm_mul_ps(lenInv, m_quad);
}

inline hkSimdReal hkVector4::normalizeWithLength4()
{
	const hkQuadReal half  = _mm_set_ss(0.5f);
	const hkQuadReal three = _mm_set_ss(3.0f);

	hkQuadReal len2 = this->dot4(*this).getQuad();
	hkQuadReal approx = _mm_rsqrt_ss(len2);
	hkQuadReal lenInv = _mm_mul_ss(
			_mm_mul_ss(half, approx),
			_mm_sub_ss(three, _mm_mul_ss( _mm_mul_ss(len2, approx), approx ) ) );
	lenInv = _mm_shuffle_ps(lenInv, lenInv, _MM_SHUFFLE(0,0,0,0) );
	m_quad = _mm_mul_ps(lenInv, m_quad);
	return _mm_mul_ss(len2, lenInv); // quicker to return x^2/x
}


inline void hkVector4::fastNormalize3()
{
	hkQuadReal len2 = this->dot3(*this).getQuad();
	hkSimdReal approx = _mm_rsqrt_ss(len2);
	m_quad = _mm_mul_ps(m_quad, approx.broadcast());
}

inline void hkVector4::fastNormalize3NonZero()
{
	hkQuadReal len2 = this->dot3(*this).getQuad();
	hkSimdReal approx = _mm_rsqrt_ss(len2);
	m_quad = _mm_mul_ps(m_quad, approx.broadcast());
}

inline hkSimdReal hkVector4::fastNormalizeWithLength3()
{
	hkQuadReal len2 = this->dot3(*this).getQuad();
	hkQuadReal approx = _mm_rsqrt_ss(len2);
	approx = _mm_shuffle_ps(approx, approx, _MM_SHUFFLE(0,0,0,0) );
	m_quad = _mm_mul_ps(m_quad, approx);
	return _mm_mul_ss(len2, approx);
}

/* operator () */

inline hkReal& hkVector4::operator() (int a)
{
	return reinterpret_cast<hkReal*>(&m_quad)[a];
}
inline const hkReal& hkVector4::operator() (int a) const
{
	return reinterpret_cast<const hkReal*>(&m_quad)[a];
}

#define HK_VECTOR4_COMBINE_XYZ_W(xyz, w) \
	_mm_shuffle_ps( xyz, _mm_unpackhi_ps(xyz, w), _MM_SHUFFLE(3,0,1,0))

inline void hkVector4::setXYZW(const hkVector4& xyz, const hkVector4& w)
{
	m_quad = HK_VECTOR4_COMBINE_XYZ_W(xyz.m_quad, w.m_quad);
}

inline void hkVector4::setW(const hkVector4& w)
{
	m_quad = HK_VECTOR4_COMBINE_XYZ_W(m_quad, w.m_quad);
}

inline void hkVector4::setXYZ(const hkVector4& xyz)
{
	m_quad = HK_VECTOR4_COMBINE_XYZ_W(xyz.m_quad, m_quad);
}

inline void hkVector4::setXYZ0(const hkVector4& xyz)
{
	static HK_ALIGN16( const hkUint32 cw[4] ) = { 0xffffffff, 0xffffffff, 0xffffffff, 0x00000000 };
	m_quad = _mm_and_ps( xyz, *(const hkQuadReal*)&cw );
}

inline void hkVector4::setReciprocal4(const hkVector4& vec)
{
	m_quad = hkMath::quadReciprocal(vec.m_quad);
}

inline void hkVector4::setSqrtInverse4(const hkVector4& vec)
{
	m_quad = hkMath::quadReciprocalSquareRoot(vec.m_quad);
}

inline hkSimdReal hkVector4::getSimdAt(int i) const
{
	HK_ASSERT2(0x6d0c31d7, i>=0 && i<4, "index out of bounds for element access");
	switch(i)
	{
		case 0:
			return m_quad;
		case 1:
			return _mm_shuffle_ps(m_quad, m_quad, _MM_SHUFFLE(1,1,1,1));
		case 2:
			return _mm_shuffle_ps(m_quad, m_quad, _MM_SHUFFLE(2,2,2,2));
		case 3:
		default:
			return _mm_shuffle_ps(m_quad, m_quad, _MM_SHUFFLE(3,3,3,3));
	}
}

inline void hkVector4::broadcast(int i)
{
	HK_ASSERT2(0x32b298db, i>=0 && i<4, "index out of bounds for broadcast");
	if( i == 0 )
	{
		m_quad = _mm_shuffle_ps(m_quad, m_quad, _MM_SHUFFLE(0,0,0,0));
	}
	else if( i == 1 )
	{
		m_quad = _mm_shuffle_ps(m_quad, m_quad, _MM_SHUFFLE(1,1,1,1));
	}
	else if( i == 2 )
	{
		m_quad = _mm_shuffle_ps(m_quad, m_quad, _MM_SHUFFLE(2,2,2,2));
	}
	else if( i == 3 )
	{
		m_quad = _mm_shuffle_ps(m_quad, m_quad, _MM_SHUFFLE(3,3,3,3));
	}
}

inline void hkVector4::setBroadcast(const hkVector4& v, int i)
{
	HK_ASSERT2(0x7d6992c5, i>=0 && i<4, "index out of bounds for broadcast");
	if( i == 0 )
	{
		m_quad = _mm_shuffle_ps(v.m_quad, v.m_quad, _MM_SHUFFLE(0,0,0,0));
	}
	else if( i == 1 )
	{
		m_quad = _mm_shuffle_ps(v.m_quad, v.m_quad, _MM_SHUFFLE(1,1,1,1));
	}
	else if( i == 2 )
	{
		m_quad = _mm_shuffle_ps(v.m_quad, v.m_quad, _MM_SHUFFLE(2,2,2,2));
	}
	else if( i == 3 )
	{
		m_quad = _mm_shuffle_ps(v.m_quad, v.m_quad, _MM_SHUFFLE(3,3,3,3));
	}
}

inline void hkVector4::setBroadcast3clobberW(const hkVector4& v, int i)
{
	setBroadcast( v, i );
}

inline void hkVector4::_setTransformedPos(const hkQsTransform& a, const hkVector4& b)
{
	hkVector4 temp(b);
	temp.mul4(a.m_scale);
	setRotatedDir(a.m_rotation, temp);
	add4(a.m_translation);
}

inline void hkVector4::_setTransformedInversePos(const hkQsTransform& a, const hkVector4& b)
{
	hkVector4 temp(b);
	temp.sub4(a.m_translation);
	setRotatedInverseDir(a.m_rotation, temp);

	hkVector4 invScale; invScale.setReciprocal4(a.m_scale);
	mul4(invScale);
}

#define HK_VECTOR4_load4a
inline void hkVector4::load4a(const hkReal* p)
{
	HK_ASSERT2(0x64211c2f, ( ((hkUlong)p) & 0xf ) == 0, "p must be 16-byte aligned.");
	m_quad = _mm_load_ps(p);
}

#define HK_VECTOR4_store4a
inline void hkVector4::store4a(hkReal* p) const
{
	HK_ASSERT2(0x6b865439, ( ((hkUlong)p) & 0xf ) == 0, "p must be 16-byte aligned.");
	_mm_store_ps(p, m_quad);
}

#define HK_VECTOR4_load4
inline void hkVector4::load4(const hkReal* p)
{
	m_quad = _mm_loadu_ps(p);
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
