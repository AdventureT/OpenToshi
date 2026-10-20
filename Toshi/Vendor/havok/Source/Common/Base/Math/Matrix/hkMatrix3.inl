/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

inline hkMatrix3::hkMatrix3( const hkMatrix3& other )
{
	m_col0 = other.m_col0;
	m_col1 = other.m_col1;
	m_col2 = other.m_col2;
}

#ifndef HK_MATRIX3_getColumn
#define HK_MATRIX3_getColumn

inline hkVector4& hkMatrix3::getColumn(int x)
{
	return (&m_col0)[x];
}

inline const hkVector4& hkMatrix3::getColumn(int x) const
{
	return (&m_col0)[x];
}

#endif

inline hkReal& hkMatrix3::operator() (int r, int c)
{
	return getColumn(c)(r);
}

inline const hkReal& hkMatrix3::operator() (int r, int c) const
{
	return getColumn(c)(r);
}


inline void hkMatrix3::setRows( const hkVector4& r0,
								const hkVector4& r1,
								const hkVector4& r2)
{
	getColumn(0).set( r0(0), r1(0), r2(0) );
	getColumn(1).set( r0(1), r1(1), r2(1) );
	getColumn(2).set( r0(2), r1(2), r2(2) );
}


inline void hkMatrix3::getRows( hkVector4& r0,
								hkVector4& r1,
								hkVector4& r2) const
{
	r0.set( getColumn(0)(0), getColumn(1)(0), getColumn(2)(0) );
	r1.set( getColumn(0)(1), getColumn(1)(1), getColumn(2)(1) );
	r2.set( getColumn(0)(2), getColumn(1)(2), getColumn(2)(2) );
}

inline void hkMatrix3::setCols( const hkVector4& r0,
								const hkVector4& r1,
								const hkVector4& r2)
{
	m_col0 = r0;
	m_col1 = r1;
	m_col2 = r2;
}

inline void hkMatrix3::operator= ( const hkMatrix3& a )
{
	m_col0 = a.getColumn(0);
	m_col1 = a.getColumn(1);
	m_col2 = a.getColumn(2);
}

inline void hkMatrix3::getCols( hkVector4& r0,
								hkVector4& r1,
								hkVector4& r2) const
{
	r0 = getColumn(0);
	r1 = getColumn(1);
	r2 = getColumn(2);
}

inline void hkMatrix3::getRow( int row, hkVector4& r) const
{
	r.set( getColumn(0)(row), getColumn(1)(row), getColumn(2)(row) );
}

	/// copy and transpose 
inline void hkMatrix3::_setTranspose( const hkMatrix3& s )
{
	hkMatrix3& d = *this;
	d(0,0) = s(0,0);
	d(1,1) = s(1,1);
	d(2,2) = s(2,2);
	d(3,0) = 0.0f;
	d(3,1) = 0.0f;
	d(3,2) = 0.0f;
	d(1,0) = s(0,1);	
	d(0,1) = s(1,0);	
	d(2,0) = s(0,2);	
	d(0,2) = s(2,0);	
	d(2,1) = s(1,2);	
	d(1,2) = s(2,1);	
}

/// aTc = aTb * bTc
inline void hkMatrix3::_setMul( const hkMatrix3& aTb, const hkMatrix3& bTc )
{
	hkMatrix3_setMulMat3Mat3( this, aTb, bTc );
}

inline void hkMatrix3::_setMul( hkSimdRealParameter scale, const hkMatrix3& a)
{
	getColumn(0).setMul4(scale, a.getColumn(0));
	getColumn(1).setMul4(scale, a.getColumn(1));
	getColumn(2).setMul4(scale, a.getColumn(2));
}

inline void hkMatrix3::_add( const hkMatrix3& a )
{
	getColumn(0).add4( a.getColumn(0) );
	getColumn(1).add4( a.getColumn(1) );
	getColumn(2).add4( a.getColumn(2) );
}

inline void hkMatrix3::_sub( const hkMatrix3& a )
{
	getColumn(0).sub4( a.getColumn(0) );
	getColumn(1).sub4( a.getColumn(1) );
	getColumn(2).sub4( a.getColumn(2) );
}

inline void hkMatrix3::_invertSymmetric()
{
	hkVector4 r0; r0.setCross( getColumn(1), getColumn(2) );
    hkVector4 r1; r1.setCross( getColumn(2), getColumn(0) );
    hkVector4 r2; r2.setCross( getColumn(0), getColumn(1) );

    hkSimdReal determinant = getColumn(0).dot3(r0);
	HK_ASSERT2( 0xf0140200, hkReal(determinant) > 0.0f, "You cannot invert this matrix");
    
	hkSimdReal dinv = hkSimdReal(1.0f) / determinant;

	getColumn(0).setMul4(dinv, r0);
	getColumn(1).setMul4(dinv, r1);
	getColumn(2).setMul4(dinv, r2);
}

inline void hkMatrix3::multiplyVector (const hkVector4& v, hkVector4& resultOut) const
{
	hkVector4 xb; xb.setBroadcast(v, 0);
	hkVector4 yb; yb.setBroadcast(v, 1);
	hkVector4 zb; zb.setBroadcast(v, 2);

	hkVector4 t0;
	t0.setMul4( xb, m_col0 );
	t0.addMul4( yb, m_col1 );
	t0.addMul4( zb, m_col2 );

	resultOut = t0;
}

// has the default implementation be overridden?
#if !defined( HK_MATRIX3_setZero )
inline void hkMatrix3::setZero()
{
	hkVector4 zero; zero.setZero4();
	getColumn(0) = zero;
	getColumn(1) = zero;
	getColumn(2) = zero;
}
#endif

#if !defined( HK_MATRIX3_setDiagonal )
inline void hkMatrix3::setDiagonal( hkReal m00, hkReal m11, hkReal m22 )
{
	setZero();
	(*this)(0,0) = m00;
	(*this)(1,1) = m11;
	(*this)(2,2) = m22;
}
#endif

#if !defined( HK_MATRIX3_setIdentity )
inline void hkMatrix3::setIdentity()
{
	setZero();
	hkReal one = 1.0f;
	(*this)(0,0) = one;
	(*this)(1,1) = one;
	(*this)(2,2) = one;
}
#endif


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
