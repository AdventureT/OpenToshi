/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifdef HK_INT_VECTOR_USE_TEMPLATE
#define HK_INT_VECTOR_COMPILE_TIME_ASSERT(X) HK_COMPILE_TIME_ASSERT(X)
#else
#define HK_INT_VECTOR_COMPILE_TIME_ASSERT(X)
#endif


//
// Clearing, loading, and storing
//

inline void hkIntVector::setZero()
{
	hkQuadUintUnion qu;
	qu.u[0] = 0;
	qu.u[1] = 0;
	qu.u[2] = 0;
	qu.u[3] = 0;
	m_quad = qu.q;
}

inline void hkIntVector::loadUnaligned4(const hkUint32* p)
{
	HK_ASSERT2(0x612f84c7, ((hkUlong)p & 3) == 0, "pointer for hkIntVector::loadUnaligned4 must be 4-byte aligned");
	hkQuadUintUnion qu;
	qu.u[0] = p[0];
	qu.u[1] = p[1];
	qu.u[2] = p[2];
	qu.u[3] = p[3];
	this->m_quad = qu.q;
}

//
// Logical operations
//

inline void hkIntVector::setNot(hkIntVectorParameter a)
{
	hkQuadUintUnion qu, qa;
	qa.q = a;

	qu.u[0] = ~qa.u[0];
	qu.u[1] = ~qa.u[1];
	qu.u[2] = ~qa.u[2];
	qu.u[3] = ~qa.u[3];

	this->m_quad = qu.q;
}

inline void hkIntVector::setOr(hkIntVectorParameter a, hkIntVectorParameter b)
{
	hkQuadUintUnion qu, qa, qb;
	qa.q = a;
	qb.q = b;

	qu.u[0] = qa.u[0] | qb.u[0];
	qu.u[1] = qa.u[1] | qb.u[1];
	qu.u[2] = qa.u[2] | qb.u[2];
	qu.u[3] = qa.u[3] | qb.u[3];

	this->m_quad = qu.q;
}

inline void hkIntVector::setAnd(hkIntVectorParameter a, hkIntVectorParameter b)
{
	hkQuadUintUnion qu, qa, qb;
	qa.q = a;
	qb.q = b;

	qu.u[0] = qa.u[0] & qb.u[0];
	qu.u[1] = qa.u[1] & qb.u[1];
	qu.u[2] = qa.u[2] & qb.u[2];
	qu.u[3] = qa.u[3] & qb.u[3];

	this->m_quad = qu.q;
}

inline void hkIntVector::setXor(hkIntVectorParameter a, hkIntVectorParameter b)
{
	hkQuadUintUnion qu, qa, qb;
	qa.q = a;
	qb.q = b;

	qu.u[0] = qa.u[0] ^ qb.u[0];
	qu.u[1] = qa.u[1] ^ qb.u[1];
	qu.u[2] = qa.u[2] ^ qb.u[2];
	qu.u[3] = qa.u[3] ^ qb.u[3];

	this->m_quad = qu.q;
}

inline void hkIntVector::setAndC(hkIntVectorParameter a, hkIntVectorParameter b)
{
	hkQuadUintUnion qu, qa, qb;
	qa.q = a;
	qb.q = b;

	qu.u[0] = qa.u[0] & ~qb.u[0];
	qu.u[1] = qa.u[1] & ~qb.u[1];
	qu.u[2] = qa.u[2] & ~qb.u[2];
	qu.u[3] = qa.u[3] & ~qb.u[3];

	this->m_quad = qu.q;
}

//
// Arithmetic operations
//

inline void hkIntVector::setAddSaturateU32( hkIntVectorParameter a, hkIntVectorParameter b )
{
	hkQuadUintUnion qu, qa, qb;
	qa.q = a; qb.q = b;
	
	const hkUint32 max = hkUint32(-1);
	qu.u[0] = (qa.u[0] < max - qb.u[0]) ? (qa.u[0] + qb.u[0]) : max; 
	qu.u[1] = (qa.u[1] < max - qb.u[1]) ? (qa.u[1] + qb.u[1]) : max; 
	qu.u[2] = (qa.u[2] < max - qb.u[2]) ? (qa.u[2] + qb.u[2]) : max; 
	qu.u[3] = (qa.u[3] < max - qb.u[3]) ? (qa.u[3] + qb.u[3]) : max; 

	m_quad = qu.q;
}


inline void hkIntVector::setAddSaturateU16( hkIntVectorParameter a, hkIntVectorParameter b )
{
	hkQuadShortUnion qu, qa, qb;
	qa.q = a; qb.q = b; qu.q = a;

	const hkUint16 max = hkUint16(-1);
	for (int i=0; i<8; i++)
	{
		qu.u[i] = (qa.u[i] < max - qb.u[i]) ? (qa.u[i] + qb.u[i]) : max; 
	}

	m_quad = qu.q;
}


inline void hkIntVector::setSubSaturateU32( hkIntVectorParameter a, hkIntVectorParameter b )
{
	hkQuadUintUnion qu, qa, qb;
	qa.q = a; qb.q = b;

	qu.u[0] = (qa.u[0] > qb.u[0]) ? (qa.u[0] - qb.u[0]) : 0; 
	qu.u[1] = (qa.u[1] > qb.u[1]) ? (qa.u[1] - qb.u[1]) : 0; 
	qu.u[2] = (qa.u[2] > qb.u[2]) ? (qa.u[2] - qb.u[2]) : 0; 
	qu.u[3] = (qa.u[3] > qb.u[3]) ? (qa.u[3] - qb.u[3]) : 0; 

	m_quad = qu.q;
}


inline void hkIntVector::setSubSaturateU16( hkIntVectorParameter a, hkIntVectorParameter b )
{
	hkQuadShortUnion qu, qa, qb;
	qa.q = a; qb.q = b; qu.q = a;

	for (int i=0; i<8; i++)
	{
		qu.u[i] = (qa.u[i] > qb.u[i]) ? (qa.u[i] - qb.u[i]) : 0; 
	}

	m_quad = qu.q;
}


//
// Shift operations
//

#ifdef HK_INT_VECTOR_USE_TEMPLATE
template<int shift> inline void hkIntVector::setShiftLeft32( hkIntVectorParameter a)
#else
inline void hkIntVector::setShiftLeft32( hkIntVectorParameter a, int shift )
#endif
{
	hkQuadUintUnion qu, qa;
	qa.q = a;

	qu.u[0] = qa.u[0] << shift;
	qu.u[1] = qa.u[1] << shift;
	qu.u[2] = qa.u[2] << shift;
	qu.u[3] = qa.u[3] << shift;

	m_quad = qu.q;
}

#ifdef HK_INT_VECTOR_USE_TEMPLATE
template<int shift> inline void hkIntVector::setShiftRight32( hkIntVectorParameter a)
#else
inline void hkIntVector::setShiftRight32( hkIntVectorParameter a, int shift )
#endif
{

	hkQuadUintUnion qu, qa;
	qa.q = a;

	qu.u[0] = qa.u[0] >> shift;
	qu.u[1] = qa.u[1] >> shift;
	qu.u[2] = qa.u[2] >> shift;
	qu.u[3] = qa.u[3] >> shift;

	m_quad = qu.q;
}

inline void hkIntVector::setShiftLeft32( hkIntVectorParameter a, hkIntVectorParameter shift )
{

	hkQuadUintUnion qu, qa, qshift;
	qa.q = a;
	qshift.q = shift;

	qu.u[0] = qa.u[0] << qshift.u[0];
	qu.u[1] = qa.u[1] << qshift.u[1];
	qu.u[2] = qa.u[2] << qshift.u[2];
	qu.u[3] = qa.u[3] << qshift.u[3];

	m_quad = qu.q;
}

inline void hkIntVector::setShiftRight32( hkIntVectorParameter a, hkIntVectorParameter shift )
{

	hkQuadUintUnion qu, qa, qshift;
	qa.q = a;
	qshift.q = shift;

	qu.u[0] = qa.u[0] >> qshift.u[0];
	qu.u[1] = qa.u[1] >> qshift.u[1];
	qu.u[2] = qa.u[2] >> qshift.u[2];
	qu.u[3] = qa.u[3] >> qshift.u[3];

	m_quad = qu.q;
}

//
// Merge operations
//
inline void hkIntVector::setMergeHigh32(hkIntVectorParameter a, hkIntVectorParameter b)
{
	hkQuadUintUnion qu, qa, qb;
	qa.q = a; qb.q = b;

	qu.u[0] = qa.u[0]; 
	qu.u[1] = qb.u[0];
	qu.u[2] = qa.u[1];
	qu.u[3] = qb.u[1];
	m_quad = qu.q;
}

inline void hkIntVector::setMergeLow32(hkIntVectorParameter a, hkIntVectorParameter b)
{
	hkQuadUintUnion qu, qa, qb;
	qa.q = a; qb.q = b;

	qu.u[0] = qa.u[2];
	qu.u[1] = qb.u[2];
	qu.u[2] = qa.u[3];
	qu.u[3] = qb.u[3];
	m_quad = qu.q;
}

inline void hkIntVector::setMergeHigh16(hkIntVectorParameter a, hkIntVectorParameter b)
{
	hkQuadShortUnion qu, qa, qb;
	qa.q = a; qb.q = b; qu.q = a;

	for (int i=0, j=0; i<8; i+=2, j++)
	{
		qu.u[ i ] = qa.u[j];
		qu.u[i+1] = qb.u[j];
	}

	m_quad = qu.q;
}

inline void hkIntVector::setMergeLow16(hkIntVectorParameter a, hkIntVectorParameter b)
{
	hkQuadShortUnion qu, qa, qb;
	qa.q = a; qb.q = b; qu.q = a;

	for (int i=0, j=4; i<8; i+=2, j++)
	{
		qu.u[ i ] = qa.u[j];
		qu.u[i+1] = qb.u[j];
	}

	m_quad = qu.q;
}

inline void hkIntVector::setMergeHigh8(hkIntVectorParameter a, hkIntVectorParameter b)
{
	hkQuadCharUnion qu, qa, qb;
	qa.q = a; qb.q = b; qu.q = a;

	for (int i=0, j=0; i<16; i+=2, j++)
	{
		qu.u[ i ] = qa.u[j];
		qu.u[i+1] = qb.u[j];
	}

	m_quad = qu.q;
}

inline void hkIntVector::setMergeLow8(hkIntVectorParameter a, hkIntVectorParameter b)
{
	hkQuadCharUnion qu, qa, qb;
	qa.q = a; qb.q = b; qu.q = a;

	for (int i=0, j=8; i<16; i+=2, j++)
	{
		qu.u[ i ] = qa.u[j];
		qu.u[i+1] = qb.u[j];
	}

	m_quad = qu.q;
}


//
// Pack operations
//
inline void hkIntVector::setPackModuloU32( hkIntVectorParameter a, hkIntVectorParameter b )
{

	hkQuadShortUnion qu, qa, qb;
	qa.q = a; qb.q = b;

	qu.u[0] = qa.u[1];
	qu.u[1] = qa.u[3];
	qu.u[2] = qa.u[5];
	qu.u[3] = qa.u[7];

	qu.u[4] = qb.u[1];
	qu.u[5] = qb.u[3];
	qu.u[6] = qb.u[5];
	qu.u[7] = qb.u[7];
	
	m_quad = qu.q;
}

//
// Shuffle
//
#ifdef HK_INT_VECTOR_USE_TEMPLATE
template<int A, int B, int C, int D> HK_FORCE_INLINE void hkIntVector::setShuffle( hkIntVectorParameter a )
#else
inline void hkIntVector::setShuffle( hkIntVectorParameter a, int A, int B, int C, int D )
#endif
{
	hkQuadUintUnion qu, qa;
	qa.q = a;

	qu.u[0] = qa.u[A];
	qu.u[1] = qa.u[B];
	qu.u[2] = qa.u[C];
	qu.u[3] = qa.u[D];

	m_quad = qu.q;
}

//
// Splat
//
#ifdef HK_INT_VECTOR_USE_TEMPLATE
template<int val> inline void hkIntVector::splatImmediate32()
#else
inline void hkIntVector::splatImmediate32(int val)
#endif
{
	HK_INT_VECTOR_COMPILE_TIME_ASSERT( val <=  15 );
	HK_INT_VECTOR_COMPILE_TIME_ASSERT( val >= -16 );

	hkQuadIntUnion qu;
	qu.u[0] = val;
	qu.u[1] = val;
	qu.u[2] = val;
	qu.u[3] = val;
	m_quad = qu.q;
}

#ifdef HK_INT_VECTOR_USE_TEMPLATE
template<int val> inline void hkIntVector::splatImmediate16()
#else
inline void hkIntVector::splatImmediate16(int val)
#endif
{
	HK_INT_VECTOR_COMPILE_TIME_ASSERT( val <=  15 );
	HK_INT_VECTOR_COMPILE_TIME_ASSERT( val >= -16 );

	hkQuadShortUnion qu; 
	qu.q = m_quad; // avoid uninitialized variable warning
	for (int i=0; i<8; i++)
	{
		qu.u[i] = (hkUint16) val;
	}
	m_quad = qu.q;
}

#ifdef HK_INT_VECTOR_USE_TEMPLATE
template<int val> inline void hkIntVector::splatImmediate8()
#else
inline void hkIntVector::splatImmediate8(int val)
#endif
{
	HK_INT_VECTOR_COMPILE_TIME_ASSERT( val <=  15 );
	HK_INT_VECTOR_COMPILE_TIME_ASSERT( val >= -16 );

	hkQuadCharUnion qu; 
	qu.q = m_quad; // avoid uninitialized variable warning
	for (int i=0; i<16; i++)
	{
		qu.u[i] = (hkUint8) val;
	}
	m_quad = qu.q;
}

#ifdef HK_INT_VECTOR_USE_TEMPLATE
template<int idx> inline void hkIntVector::setBroadcast(hkIntVectorParameter a)
#else
inline void hkIntVector::setBroadcast(hkIntVectorParameter a, int idx)
#endif
{
	hkQuadUintUnion qu, qa;
	qa.q = a; 

	qu.u[0] = qa.u[idx];
	qu.u[1] = qa.u[idx];
	qu.u[2] = qa.u[idx];
	qu.u[3] = qa.u[idx];
	
	m_quad = qu.q;
}


//
// int <-> float conversion
//
inline void hkIntVector::convertToF32fromU32(hkVector4& vOut) const
{
	hkQuadUintUnion qu;
	qu.q = m_quad;

	vOut(0) = (hkReal) qu.u[0];
	vOut(1) = (hkReal) qu.u[1];
	vOut(2) = (hkReal) qu.u[2];
	vOut(3) = (hkReal) qu.u[3];
}

inline void hkIntVector::convertToF32fromS32(hkVector4& vOut) const
{
	hkQuadIntUnion qu;
	qu.q = m_quad;

	vOut(0) = (hkReal) qu.u[0];
	vOut(1) = (hkReal) qu.u[1];
	vOut(2) = (hkReal) qu.u[2];
	vOut(3) = (hkReal) qu.u[3];
}

inline void hkIntVector::convertFromF32toU32(hkVector4Parameter vIn)
{
	hkQuadUintUnion qu;
	
	qu.u[0] = (hkUint32) vIn(0);
	qu.u[1] = (hkUint32) vIn(1);
	qu.u[2] = (hkUint32) vIn(2);
	qu.u[3] = (hkUint32) vIn(3);

	m_quad = qu.q;
}

// The following functions are valid for all platforms (there wont' be many of these)
inline void hkIntVector::loadAligned(const hkUint32* p)
{
	HK_ASSERT2(0x3a8619dc, ((hkUlong)p & 0xf) == 0, "pointer for hkIntVector::loadAligned must be 16-byte aligned");
	m_quad = *reinterpret_cast<const hkQuadUint*>(p);
}

inline hkQuadUint& hkIntVector::getQuad()
{
	return m_quad;
}

inline const hkQuadUint& hkIntVector::getQuad() const
{
	return m_quad;
}

inline void hkIntVector::select32(hkIntVectorParameter a, hkIntVectorParameter b, hkVector4ComparisonParameter selectMask)
{
	((hkVector4*)(this))->select32( *(hkVector4*)(&a), *(hkVector4*)(&b), selectMask );
}

//

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
