/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
HK_FORCE_INLINE hkReal hkVector2::signedDistanceToLine( const hkVector2& p0, const hkVector2& p1 ) const
{
	return ((p1.x - p0.x)*(y - p0.y) - (x - p0.x)*(p1.y - p0.y));
}

inline hkBool32 hkVector2::rightOfLine( const hkVector2& p0, const hkVector2& p1 ) const
{
	return signedDistanceToLine(p0,p1) < 0;
}

inline hkBool32 hkVector2::leftOfLine( const hkVector2& p0, const hkVector2& p1 ) const
{
	return signedDistanceToLine(p0,p1) > 0;
}

inline hkBool32 hkVector2::inCircumcircle( const hkVector2& p0, const hkVector2& p1, const hkVector2& p2, hkReal tolerance ) const
{
	hkVector4 a; a.set( p0.x - x, p1.x - x, p2.x - x );
	hkVector4 b; b.set( p0.y - y, p1.y - y, p2.y - y );
	hkVector4 temp1; temp1.setMul4( a, a );
	hkVector4 temp2; temp2.setMul4( b, b );
	hkVector4 c; c.setAdd4( temp1, temp2 );
	hkVector4 cross; cross.setCross( b, c );
	return a.dot3( cross ) > -tolerance; // XXX
}

inline hkReal hkVector2::dot( const hkVector2& p ) const
{
	return x*p.x + y*p.y;
}

inline void hkVector2::setMax( const hkVector2& a, const hkVector2& b )
{
	x = hkMath::max2(a.x,b.x);
	y = hkMath::max2(a.y,b.y);
}

inline void hkVector2::setMin( const hkVector2& a, const hkVector2& b )
{
	x = hkMath::min2(a.x,b.x);
	y = hkMath::min2(a.y,b.y);
}

inline void hkVector2::setAll( hkReal a )
{
	x = a;
	y = a;
}

inline void hkVector2::set( hkReal a, hkReal b )
{
	x = a;
	y = b;
}

inline void hkVector2::setPerp( const hkVector2& a )
{
	x = -a.y;
	y =  a.x;
}

inline void hkVector2::setAdd( const hkVector2& a, const hkVector2& b )
{
	x = a.x + b.x;
	y = a.y + b.y;
}

inline void hkVector2::setMul( const hkVector2& v, hkReal r )
{
	x = v.x * r;
	y = v.y * r;
}

inline void hkVector2::setMul( const hkVector2& v, const hkVector2& w )
{
	x = v.x * w.x;
	y = v.y * w.y;
}

inline void hkVector2::mul( hkReal r )
{
	x *= r;
	y *= r;
}

inline void hkVector2::setAddMul( const hkVector2& a, const hkVector2& b, hkReal r )
{
	x = a.x + b.x * r;
	y = a.y + b.y * r;
}

inline void hkVector2::setSub( const hkVector2& a, const hkVector2& b )
{
	x = a.x - b.x;
	y = a.y - b.y;
}

inline void hkVector2::setInterpolate( const hkVector2& a, const hkVector2& b, hkReal t)
{
	x = a.x + t * (b.x-a.x);
	y = a.y + t * (b.y-a.y);
}

inline hkReal hkVector2::distanceTo( const hkVector2& p ) const
{
	hkReal dx = x - p.x;
	hkReal dy = y - p.y;
	return hkMath::sqrt( dx*dx + dy*dy );
}

inline void hkVector2::setProject( hkVector4Parameter p, hkVector4Parameter ax, hkVector4Parameter ay )
{
	x = ax.dot3(p);
	y = ay.dot3(p);
}

inline void hkVector2::normalize()
{
	hkReal n = hkMath::sqrtInverse(x*x + y*y);
	x *= n;
	y *= n;
}

inline hkReal hkVector2::normalizeWithLength()
{
	hkReal n = hkMath::sqrtInverse(x*x + y*y);
	x *= n;
	y *= n;
	return 1.0f / n;
}

inline void hkVector2::setZero()
{
	x = y = 0;
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
