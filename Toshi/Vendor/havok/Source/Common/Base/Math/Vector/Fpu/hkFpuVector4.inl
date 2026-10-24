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
	return reinterpret_cast<hkQuadReal&>(x);
}

inline const hkQuadReal& hkVector4::getQuad() const
{
	return reinterpret_cast<const hkQuadReal&>(x);
}

/* operator (),[] here for inlining */

inline hkReal& hkVector4::operator() (int i)
{
	HK_ASSERT(0x3d62369d, i>=0 && i<4);
	return (&x)[i];
}

inline const hkReal& hkVector4::operator() (int i) const
{
	HK_ASSERT(0x269e424a, i>=0 && i<4);
	return (&x)[i];
}

HK_FORCE_INLINE void hkVector4::setW(const hkVector4& v)
{
	w = v.w;
}

HK_FORCE_INLINE void hkVector4::setXYZ(const hkVector4& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
}

HK_FORCE_INLINE void hkVector4::setXYZ0(const hkVector4& xyz)
{
	x = xyz.x;
	y = xyz.y;
	z = xyz.z;
	w = 0;
}

HK_FORCE_INLINE void hkVector4::setXYZW(const hkVector4& xyz, const hkVector4& vw)
{
	x = xyz.x;
	y = xyz.y;
	z = xyz.z;
	w = vw.w;
}

HK_FORCE_INLINE void hkVector4::setReciprocal4(const hkVector4& v)
{
	x = 1.0f / v.x;
	y = 1.0f / v.y;
	z = 1.0f / v.z;
	w = 1.0f / v.w;
}

#define HK_VECTOR4_setReciprocal3
inline void hkVector4::setReciprocal3(const hkVector4& v)
{
	x = 1.0f / v.x;
	y = 1.0f / v.y;
	z = 1.0f / v.z;
	w = 1.0f;
}

HK_FORCE_INLINE void hkVector4::setSqrtInverse4(const hkVector4& v)
{
	x = hkMath::sqrtInverse(v.x);
	y = hkMath::sqrtInverse(v.y);
	z = hkMath::sqrtInverse(v.z);
	w = hkMath::sqrtInverse(v.w);
}

inline hkSimdReal hkVector4::getSimdAt(int i) const
{
	HK_ASSERT(0x5e605c8e, i>=0 && i<4);
	return (&x)[i];
}

inline void hkVector4::operator= (const hkVector4& v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	w = v.w;
}

#ifndef HK_DISABLE_VECTOR4_CONSTRUCTORS

/* construct, assign, zero */
inline hkVector4::hkVector4(hkReal a, hkReal b, hkReal c, hkReal d)
	: x(a), y(b), z(c), w(d)
{
}

inline hkVector4::hkVector4(const hkQuadReal& q)
	: x(q.x), y(q.y), z(q.z), w(q.w)
{
}

inline hkVector4::hkVector4( const hkVector4& v)
{
	*this = v;
}
#endif

inline void hkVector4::set(hkReal a, hkReal b, hkReal c, hkReal d)
{
	x = a;
	y = b;
	z = c;
	w = d;
}

inline void hkVector4::setAll(hkReal a)
{
	x = a;
	y = a;
	z = a;
	w = a;
}

inline void hkVector4::setAll3(hkReal a)
{
	x = a;
	y = a;
	z = a;
	w = a;
}

inline void hkVector4::setZero4()
{
	x = y = z = w = 0;
}

inline void hkVector4::zeroElement( int i )
{
	(*this)(i) = 0.0f;
}

inline void hkVector4::setInt24W( int value )
{
	(reinterpret_cast<int*>(this))[3] = (value | 0x3f000000);
}

// Gets the .w component as an integer with 24 bit resolution (for real experts only)
inline int hkVector4::getInt24W( ) const 
{
	return (reinterpret_cast<const int*>(this))[3] & ~0x3f000000;
}

inline void hkVector4::storeUncached( void* dest) const
{
	*static_cast<hkVector4*>(dest) = *this;
}

/* vector methods */

inline void hkVector4::add4(const hkVector4& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	w += v.w;
}

inline void hkVector4::add3clobberW(const hkVector4& v)
{
	x += v.x;
	y += v.y;
	z += v.z;
}

inline void hkVector4::setAdd4(const hkVector4& v0, const hkVector4& v1)
{
	x = v0.x + v1.x;
	y = v0.y + v1.y;
	z = v0.z + v1.z;
	w = v0.w + v1.w;
}

inline void hkVector4::sub4(const hkVector4& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	w -= v.w;
}

inline void hkVector4::sub3clobberW(const hkVector4& v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
}

inline void hkVector4::setSub4(const hkVector4& v0, const hkVector4& v1)
{
	x = v0.x - v1.x;
	y = v0.y - v1.y;
	z = v0.z - v1.z;
	w = v0.w - v1.w;
}

inline void hkVector4::mul4(const hkVector4& a)
{
	x *= a.x;
	y *= a.y;
	z *= a.z;
	w *= a.w;
}
inline void hkVector4::setMul4(const hkVector4& a, const hkVector4& b)
{
	x = a.x * b.x;
	y = a.y * b.y;
	z = a.z * b.z;
	w = a.w * b.w;
}

inline void hkVector4::mul4(hkSimdRealParameter rs)
{  
	hkReal s = hkReal(rs);
	x *= s;
	y *= s;
	z *= s;
	w *= s;
}
inline void hkVector4::setMul4(hkSimdRealParameter rs, const hkVector4& a)
{
	hkReal r = hkReal(rs);
	x = r * a.x;
	y = r * a.y;
	z = r * a.z;
	w = r * a.w;
}
inline void hkVector4::addMul4(hkSimdRealParameter rs, const hkVector4& a)
{
	hkReal r = hkReal(rs);
	x += r * a.x;
	y += r * a.y;
	z += r * a.z;
	w += r * a.w;
}
inline void hkVector4::addMul4(const hkVector4& a, const hkVector4& b)
{
	x += a.x * b.x;
	y += a.y * b.y;
	z += a.z * b.z;
	w += a.w * b.w;
}

inline void hkVector4::subMul4(hkSimdRealParameter rs, const hkVector4& a)
{
	hkReal r = hkReal(rs);
	x -= r * a.x;
	y -= r * a.y;
	z -= r * a.z;
	w -= r * a.w;
}

inline void hkVector4::subMul4(const hkVector4& a, const hkVector4& b)
{
	x -= a.x * b.x;
	y -= a.y * b.y;
	z -= a.z * b.z;
	w -= a.w * b.w;
}

inline void hkVector4::setAddMul4(const hkVector4& a, const hkVector4& b, hkSimdRealParameter r)
{
	x = a.x + r * b.x;
	y = a.y + r * b.y;
	z = a.z + r * b.z;
	w = a.w + r * b.w;
}

inline void hkVector4::setAddMul4(const hkVector4& a, const hkVector4& m0, const hkVector4& m1)
{
	x = a.x + m0.x * m1.x;
	y = a.y + m0.y * m1.y;
	z = a.z + m0.z * m1.z;
	w = a.w + m0.w * m1.w;
}


#if !defined( HK_VECTOR4_setCross)
	inline void hkVector4::setCross(const hkVector4& v1, const hkVector4& v2)
	{
		const hkReal nx = v1(1)*v2(2) - v1(2)*v2(1);
		const hkReal ny = v1(2)*v2(0) - v1(0)*v2(2);
		const hkReal nz = v1(0)*v2(1) - v1(1)*v2(0);
		this->set( nx, ny, nz , 0.0f );
	}
#endif // HK_VECTOR4_SET_CROSS


inline void hkVector4::setInterpolate4(const hkVector4& v0, const hkVector4& v1, hkSimdRealParameter rt)
{
	hkReal t = hkReal(rt);
	hkReal s = 1.0f - t;
	hkVector4& d = *this;
	d(0) = s * v0(0) + t * v1(0);
	d(1) = s * v0(1) + t * v1(1);
	d(2) = s * v0(2) + t * v1(2);
	d(3) = s * v0(3) + t * v1(3);
}

inline hkVector4Comparison hkVector4::compareLessThan4(const hkVector4& a) const
{
	hkVector4Comparison ret;
	ret.m_mask =
		((x<a.x) ? hkVector4Comparison::MASK_X : 0) |
		((y<a.y) ? hkVector4Comparison::MASK_Y : 0) |
		((z<a.z) ? hkVector4Comparison::MASK_Z : 0) |
		((w<a.w) ? hkVector4Comparison::MASK_W : 0);
	return ret;
}

inline hkVector4Comparison hkVector4::compareEqual4(const hkVector4& a) const
{
	hkVector4Comparison ret;
	ret.m_mask =
		((x==a.x) ? hkVector4Comparison::MASK_X : 0) |
		((y==a.y) ? hkVector4Comparison::MASK_Y : 0) |
		((z==a.z) ? hkVector4Comparison::MASK_Z : 0) |
		((w==a.w) ? hkVector4Comparison::MASK_W : 0);
	return ret;
}

inline hkVector4Comparison hkVector4::compareLessThanEqual4(const hkVector4& a) const
{
	hkVector4Comparison ret;
	ret.m_mask =
		((x<=a.x) ? hkVector4Comparison::MASK_X : 0) |
		((y<=a.y) ? hkVector4Comparison::MASK_Y : 0) |
		((z<=a.z) ? hkVector4Comparison::MASK_Z : 0) |
		((w<=a.w) ? hkVector4Comparison::MASK_W : 0);
	return ret;
}

inline hkVector4Comparison hkVector4::compareLessThanZero4() const
{
	hkVector4Comparison ret;
	ret.m_mask =
		((x<0) ? hkVector4Comparison::MASK_X : 0) |
		((y<0) ? hkVector4Comparison::MASK_Y : 0) |
		((z<0) ? hkVector4Comparison::MASK_Z : 0) |
		((w<0) ? hkVector4Comparison::MASK_W : 0);
	return ret;
}

inline hkBool32 hkVector4::allLessThan3(hkVector4Parameter a) const
{
	return x<a.x && y<a.y && z<a.z;
}

inline hkBool32 hkVector4::allLessThan4(hkVector4Parameter a) const
{
	return x<a.x && y<a.y && z<a.z && w<a.w;
}

inline void hkVector4::select32( hkVector4Parameter a, hkVector4Parameter b, const hkVector4Comparison& comp)
{
	int m = comp.getMask();
	x = (m & hkVector4Comparison::MASK_X) ? b.x : a.x;
	y = (m & hkVector4Comparison::MASK_Y) ? b.y : a.y;
	z = (m & hkVector4Comparison::MASK_Z) ? b.z : a.z;
	w = (m & hkVector4Comparison::MASK_W) ? b.w : a.w;
}

inline void hkVector4::setNeg4(const hkVector4& v)
{
	x = -v.x;
	y = -v.y;
	z = -v.z;
	w = -v.w;
}

inline void hkVector4::setNeg3(const hkVector4& v)
{
	x = -v.x;
	y = -v.y;
	z = -v.z;
	w =  v.w;
}

inline void hkVector4::setAbs4(const hkVector4& v)
{
	x = hkMath::fabs(v.x);
	y = hkMath::fabs(v.y);
	z = hkMath::fabs(v.z);
	w = hkMath::fabs(v.w);
}

#if !defined(HK_VECTOR4_setMin4)
	inline void hkVector4::setMin4(const hkVector4& a, const hkVector4& b)
	{
		x = a.x < b.x ? a.x : b.x;
		y = a.y < b.y ? a.y : b.y;
		z = a.z < b.z ? a.z : b.z;
		w = a.w < b.w ? a.w : b.w;
	}
#endif

#if !defined( HK_VECTOR4_setMax4)
	inline void hkVector4::setMax4(const hkVector4& a, const hkVector4& b)
	{
		x = a.x > b.x ? a.x : b.x;
		y = a.y > b.y ? a.y : b.y;
		z = a.z > b.z ? a.z : b.z;
		w = a.w > b.w ? a.w : b.w;
	}
#endif

#if !defined( HK_VECTOR4__setMul3)
	inline void hkVector4::_setMul3( const hkMatrix3& r, const hkVector4& v )
	{
		//HK_ASSERT(0x28487ddd, hkMath::isAliased(r,this) == false );
		hkReal v0 = v(0);
		hkReal v1 = v(1);
		hkReal v2 = v(2);
		hkVector4& t = *this;
		t(0) = r(0,0)*v0 + r(0,1)*v1 + r(0,2)*v2;
		t(1) = r(1,0)*v0 + r(1,1)*v1 + r(1,2)*v2;
		t(2) = r(2,0)*v0 + r(2,1)*v1 + r(2,2)*v2;
		t(3) = 0;
	}
#endif

#if !defined( HK_VECTOR4__setMul4)
	inline void hkVector4::_setMul4( const hkMatrix3& t, const hkVector4& v )
	{
		//HK_ASSERT(0x28487ddd, hkMath::isAliased(t,this) == false );
		hkReal v0 = v(0);
		hkReal v1 = v(1);
		hkReal v2 = v(2);
		hkVector4& d = *this;
		d(0) = t(0,0)*v0 + t(0,1)*v1 + t(0,2)*v2;
		d(1) = t(1,0)*v0 + t(1,1)*v1 + t(1,2)*v2;
		d(2) = t(2,0)*v0 + t(2,1)*v1 + t(2,2)*v2;
		d(3) = t(3,0)*v0 + t(3,1)*v1 + t(3,2)*v2;
	}
#endif

#if !defined( HK_VECTOR4__setMul4xyz1)
	inline void hkVector4::_setMul4xyz1( const hkTransform& t, const hkVector4& v )
	{
		//HK_ASSERT(0x28487ddd, hkMath::isAliased(t,this) == false );
		hkReal v0 = v(0);
		hkReal v1 = v(1);
		hkReal v2 = v(2);
		hkVector4& d = *this;
		d(0) = t(0,0)*v0 + t(0,1)*v1 + t(0,2)*v2 + t(0,3);
		d(1) = t(1,0)*v0 + t(1,1)*v1 + t(1,2)*v2 + t(1,3);
		d(2) = t(2,0)*v0 + t(2,1)*v1 + t(2,2)*v2 + t(2,3);
		d(3) = t(3,0)*v0 + t(3,1)*v1 + t(3,2)*v2 + t(3,3);
	}
#endif

#if !defined( HK_VECTOR4__setRotatedDir)
	inline void hkVector4::_setRotatedDir(const hkRotation& r, const hkVector4& v)
	{
		_setMul3(r,v);	
	}
#endif

#if !defined( HK_VECTOR4__setRotatedInverseDir)
	inline void hkVector4::_setRotatedInverseDir(const hkRotation& r, const hkVector4& v)
	{
		//HK_ASSERT(0x28487ddd, hkMath::isAliased(r,this) == false );
		hkReal v0 = v(0);
		hkReal v1 = v(1);
		hkReal v2 = v(2);
		hkVector4& t = *this;
		t(0) = r(0,0)*v0 + r(1,0)*v1 + r(2,0)*v2;
		t(1) = r(0,1)*v0 + r(1,1)*v1 + r(2,1)*v2;
		t(2) = r(0,2)*v0 + r(1,2)*v1 + r(2,2)*v2;
		t(3) = 0;
	}
#endif

#if !defined( HK_VECTOR4__setTransformedPos)
	inline void hkVector4::_setTransformedPos(const hkTransform& t, const hkVector4& v)
	{
		//HK_ASSERT(0x28487ddd, hkMath::isAliased(t,this) == false );
		hkReal v0 = v(0);
		hkReal v1 = v(1);
		hkReal v2 = v(2);
		hkVector4& d = *this;
		d(0) = t(0,0)*v0 + t(0,1)*v1 + t(0,2)*v2 + t(0,3);
		d(1) = t(1,0)*v0 + t(1,1)*v1 + t(1,2)*v2 + t(1,3);
		d(2) = t(2,0)*v0 + t(2,1)*v1 + t(2,2)*v2 + t(2,3);
		d(3) = 0;
	}
#endif

#if !defined( HK_VECTOR4__setTransformedInversePos)
	inline void hkVector4::_setTransformedInversePos(const hkTransform& t, const hkVector4& v)
	{
		//HK_ASSERT(0x28487ddd, hkMath::isAliased(t,this) == false );
		hkReal v0 = v(0) - t(0,3);
		hkReal v1 = v(1) - t(1,3);
		hkReal v2 = v(2) - t(2,3);
	
		hkVector4& d = *this;
		d(0) = t(0,0)*v0 + t(1,0)*v1 + t(2,0)*v2;
		d(1) = t(0,1)*v0 + t(1,1)*v1 + t(2,1)*v2;
		d(2) = t(0,2)*v0 + t(1,2)*v1 + t(2,2)*v2;
		d(3) = 0.0f;
	}
#endif
	
/* length and distance */

inline hkSimdReal hkVector4::dot3(const hkVector4& a) const
{
	return (x * a.x) + ( y * a.y ) + ( z * a.z );
}

inline void hkVector4::setDot3(hkVector4Parameter a, hkVector4Parameter b)
{
	x = ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z );
}

inline hkReal hkVector4::dot3fpu(const hkVector4& a) const
{
	return (x * a.x) + ( y * a.y ) + ( z * a.z );
}

inline hkSimdReal hkVector4::dot4(const hkVector4& a) const
{
	return (x * a.x) + ( y * a.y ) + ( z * a.z ) + ( w * a.w );
}

inline void hkVector4::setDot4(hkVector4Parameter a, hkVector4Parameter b)
{
	x = y = z = w = ( a.x * b.x ) + ( a.y * b.y ) + ( a.z * b.z) + ( a.w * b.w );
}

inline hkSimdReal hkVector4::dot4xyz1(const hkVector4& a) const
{
	return (x * a.x) + ( y * a.y ) + ( z * a.z ) + ( w  );
}

inline hkSimdReal hkVector4::horizontalAdd3() const
{
	return x+y+z;
}

inline hkSimdReal hkVector4::horizontalAdd4() const
{
	return x+y+z+w;
}

#if !defined( HK_VECTOR4_length3)
	inline hkSimdReal hkVector4::length3() const
	{
		return hkMath::sqrt((x * x) + ( y * y ) + ( z * z ));
	}
#endif
	
	
inline hkSimdReal hkVector4::lengthSquared3() const
{
	return this->dot3(*this);
}

#if !defined( HK_VECTOR4_lengthInverse3)
	inline hkSimdReal hkVector4::lengthInverse3() const
	{
		hkReal l2 = (x*x) + (y*y) + (z*z);
		return l2 ? hkMath::sqrtInverse(l2) : 0;
	}
#endif

#if !defined( HK_VECTOR4_length4)	
	inline hkSimdReal hkVector4::length4() const
	{
		return hkMath::sqrt( (x * x) + (y * y) + (z * z) + (w * w) );
	}
#endif

inline hkSimdReal hkVector4::lengthSquared4() const
{
	return this->dot4(*this);
}

#if !defined( HK_VECTOR4_lengthInverse4)
	inline hkSimdReal hkVector4::lengthInverse4() const
	{
		hkReal l2 = (x*x) + (y*y) + (z*z) + (w*w);
		return l2 ? hkMath::sqrtInverse(l2) : 0;
	}
#endif

#if !defined( HK_VECTOR4_normalize3 )
	inline void hkVector4::normalize3()
	{
		this->mul4( this->lengthInverse3() );
		HK_ASSERT2(0x475d86b1, isNormalized3(), "hkVector4 too small for normalize3");
	}
#endif
	
#if !defined( HK_VECTOR4_normalizeWithLength3)	
	inline hkSimdReal hkVector4::normalizeWithLength3()
	{
		hkReal len = this->length3();
		this->mul4( (1.0f)/len );
		HK_ASSERT2(0x6fe84a9b, isNormalized3(), "hkVector4 too small for normalize3");
		return len;
	}
#endif

#if !defined( HK_VECTOR4_normalize4)
	inline void hkVector4::normalize4()
	{
		this->mul4( this->lengthInverse4() );
		HK_ASSERT2(0x21c8ab2a,  isNormalized4(), "hkVector4 too small for normalize4");
	}
#endif

#if !defined( HK_VECTOR4_normalizeWithLength4)
	inline hkSimdReal hkVector4::normalizeWithLength4()
	{
		
		hkReal len = this->length4();
		this->mul4( (1.0f)/len );
		HK_ASSERT2(0x309314d9,  isNormalized4(), "hkVector4 too small for normalize4");
		return len;
	}
#endif
	
inline void hkVector4::fastNormalize3()
{
	normalize3();
}

inline void hkVector4::fastNormalize3NonZero()
{
	hkReal l2 = (x*x) + (y*y) + (z*z);
	l2 = hkMath::sqrtInverse(l2);
	this->mul4( l2 );
}

inline hkSimdReal hkVector4::fastNormalizeWithLength3()
{
	return normalizeWithLength3();
}


inline void hkVector4::broadcast(int i)
{
	HK_ASSERT2(0x4a4d61ec, i>=0 && i<4, "index error in broadcast");
	hkReal r = (&x)[i];
	x = r;
	y = r;
	z = r;
	w = r;
}

inline void hkVector4::setBroadcast(const hkVector4& v, int i)
{
	HK_ASSERT2(0x294fb42b, i>=0 && i<4, "index error in broadcast");
	hkReal r = (&v.x)[i];
	x = r;
	y = r;
	z = r;
	w = r;
}

inline void hkVector4::setBroadcast3clobberW(const hkVector4& v, int i)
{
	HK_ASSERT2(0x294fb42b, i>=0 && i<4, "index error in broadcast");
	hkReal r = (&v.x)[i];
	x = r;
	y = r;
	z = r;
	w = r;
}

inline void hkVector4::_setTransformedPos(const hkQsTransform& a, const hkVector4& b)
{
	hkVector4 scaled; scaled.setMul4(b, a.m_scale);
	hkVector4 rotated; rotated.setRotatedDir( a.m_rotation, scaled );
	setAdd4( rotated, a.m_translation );
}

inline void hkVector4::_setTransformedInversePos(const hkQsTransform& a, const hkVector4& b)
{
	//HK_ASSERT(0x28487ddd, hkMath::isAliased(a,this) == false );
	hkVector4 temp(b);
	temp.sub4(a.m_translation);
	setRotatedInverseDir(a.m_rotation, temp);

	hkVector4 invScale; invScale.set(1.0f/a.m_scale(0), 1.0f/a.m_scale(1), 1.0f/a.m_scale(2));
	mul4(invScale);
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
