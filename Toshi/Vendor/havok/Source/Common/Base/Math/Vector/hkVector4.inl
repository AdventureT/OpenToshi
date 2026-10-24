/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

inline void hkVector4::operator=(const hkQuadReal& r)
{
	getQuad() = r;
}

#ifndef HK_VECTOR4_setRotatedDir
HK_FORCE_INLINE void hkVector4::setRotatedDir(const hkQuaternion& quat, const hkVector4& direction)
{
#if HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED
	hkVector4 minusHalf; minusHalf.getQuad() = hkQuadRealMinusHalf;
	hkVector4 qreal;
	qreal.setBroadcast(quat.m_vec, 3);
	hkVector4 q2minus1;
	q2minus1.setAddMul4( minusHalf, qreal, qreal );

	hkVector4 ret;
	ret.setMul4( direction, q2minus1 );

	hkVector4 imagDotDir; 
	imagDotDir.getQuad() = quat.getImag().dot3( direction ).getQuad();
	imagDotDir.broadcast(0);
#else
	hkReal qreal = quat.m_vec(3);
	hkReal q2minus1 = qreal*qreal - 0.5f;

	hkVector4 ret;
	ret.setMul4( q2minus1, direction );

	hkReal imagDotDir = quat.getImag().dot3( direction );
#endif
	ret.addMul4( imagDotDir, quat.getImag() ); 

	hkVector4 imagCrossDir;
	imagCrossDir.setCross( quat.getImag(), direction );
	ret.addMul4( qreal, imagCrossDir );

	this->setAdd4( ret, ret );
}
#endif

inline void hkVector4::setRotatedInverseDir(const hkRotation& t, const hkVector4& v)
{
	_setRotatedInverseDir(t,v);
}

#ifndef HK_VECTOR4_setRotatedInverseDir
HK_FORCE_INLINE void hkVector4::setRotatedInverseDir(const hkQuaternion& quat, const hkVector4& direction)
{
#if HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED
	hkVector4 minusHalf; minusHalf.getQuad() = hkQuadRealMinusHalf;
	hkVector4 qreal;
	qreal.setBroadcast(quat.m_vec, 3);
	hkVector4 q2minus1;
	q2minus1.setAddMul4( minusHalf, qreal, qreal );

	hkVector4 ret;
	ret.setMul4( direction, q2minus1 );

	hkVector4 imagDotDir; 
	imagDotDir.getQuad() = quat.getImag().dot3( direction ).getQuad();
	imagDotDir.broadcast(0);
#else
	hkReal qreal = quat.m_vec(3);
	hkReal q2minus1 = qreal*qreal - 0.5f;

	hkVector4 ret;
	ret.setMul4( q2minus1, direction );

	hkReal imagDotDir = quat.getImag().dot3( direction );
#endif
	ret.addMul4( imagDotDir, quat.getImag() ); 

	hkVector4 imagCrossDir;
	imagCrossDir.setCross( direction, quat.getImag() );
	ret.addMul4( qreal, imagCrossDir );

	this->setAdd4( ret, ret );
}
#endif

inline hkVector4Comparison hkVector4::compareGreaterThan4(hkVector4Parameter a) const
{
	return a.compareLessThan4(*this);
}

inline hkVector4Comparison hkVector4::compareGreaterThanEqual4(hkVector4Parameter a) const
{
	return a.compareLessThanEqual4(*this);
}

#ifndef HK_VECTOR4_setReciprocal3
inline void hkVector4::setReciprocal3(const hkVector4& v)
{
	hkVector4 v1111; v1111.getQuad() = hkQuadReal1111;
	hkVector4 tmp; tmp.setXYZW( v, v1111 );
	setReciprocal4(tmp);
}
#endif

inline hkBool32 hkVector4::equals3(const hkVector4 &v, hkReal epsilon) const
{
	//non_euclidean, manhattan based
	hkVector4 t;
	t.setSub4(*this, v);
	t.setAbs4( t );
	hkVector4 epsilon_v;
	epsilon_v.setAll3( epsilon );
	return t.compareLessThanEqual4( epsilon_v ).allAreSet( hkVector4Comparison::MASK_XYZ );
}

inline hkBool32 hkVector4::equals4(const hkVector4 &v, hkReal epsilon) const
{
	//non_euclidean, manhattan based
	hkVector4 t;
	t.setSub4(*this, v);
	t.setAbs4( t );
	hkVector4 epsilon_v;
	epsilon_v.setAll(epsilon);
	return t.compareLessThanEqual4( epsilon_v ).allAreSet( hkVector4Comparison::MASK_XYZW );
}

inline int hkVector4::getMajorAxis() const
{
	hkVector4 tmp;
	tmp.setAbs4(*this);
	return tmp(0) < tmp(1) ? (tmp(1) < tmp(2) ? 2 : 1) : (tmp(0) < tmp(2) ? 2 : 0);
}

inline const hkVector4& HK_CALL hkVector4::getZero()
{
	extern hkVector4 hkVector4Zero;
	return hkVector4Zero;
}

#ifndef HK_VECTOR4_distanceTo3
inline hkSimdReal hkVector4::distanceTo3(hkVector4Parameter p) const
{
	hkVector4 d; d.setSub4(*this, p);
	return d.length3();
}
#endif

inline hkResult hkVector4::normalize3IfNotZero ()
{
	const hkReal lengthSqrd3 = this->lengthSquared3();
	if (lengthSqrd3 != 0.0f)
	{
		this->mul4( hkMath::sqrtInverse( lengthSqrd3 ) );
		return HK_SUCCESS;
	}

	return HK_FAILURE;
}

inline void hkVector4::load3(const hkReal* p)
{
	hkVector4& v = *this;
	v(0) = p[0];
	v(1) = p[1];
	v(2) = p[2];
}

#ifndef HK_VECTOR4_load4
inline void hkVector4::load4(const hkReal* p)
{
	hkVector4& v = *this;
	v(0) = p[0];
	v(1) = p[1];
	v(2) = p[2];
	v(3) = p[3];
}
#endif

inline void hkVector4::store3(hkReal* p) const
{
	const hkVector4& v = *this;
	p[0] = v(0);
	p[1] = v(1);
	p[2] = v(2);
}

inline void hkVector4::store4(hkReal* p) const
{
	const hkVector4& v = *this;
	p[0] = v(0);
	p[1] = v(1);
	p[2] = v(2);
	p[3] = v(3);
}

#ifndef HK_VECTOR4_load4a
inline void hkVector4::load4a(const hkReal* p)
{
	hkVector4& v = *this;
	v(0) = p[0];
	v(1) = p[1];
	v(2) = p[2];
	v(3) = p[3];
}
#endif
#ifndef HK_VECTOR4_store4a
inline void hkVector4::store4a(hkReal* p) const
{
	const hkVector4& v = *this;
	p[0] = v(0);
	p[1] = v(1);
	p[2] = v(2);
	p[3] = v(3);
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
