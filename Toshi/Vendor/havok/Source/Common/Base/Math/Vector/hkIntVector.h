/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_MATH_INTVECTOR_H
#define HK_MATH_INTVECTOR_H

#ifndef HK_MATH_MATH_H
#	error Please include Common/Base/hkBase.h instead of this file.
#endif

class hkIntVector;

// Not all platforms support method declarations like
//		template<int idx> HK_FORCE_INLINE void foo();
// But we need certain methods to use this because the parameters need to be known at compile to.
// To get around this, we declare methods with both versions, but only one is allowed per platform
#	define HK_INT_VECTOR_USE_TEMPLATE


// <ce.todo> Move to hkXxxMathTypes.inl
#if defined (HK_PLATFORM_PS3_PPU) || defined (HK_PLATFORM_PS3_SPU)
	//typedef vector unsigned int hkQuadUint; // already taken care of in MathTypes
	typedef const hkIntVector hkIntVectorParameter;
	typedef vector unsigned char hkPermuteMask;
#elif HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED && defined(HK_COMPILER_HAS_INTRINSICS_IA32)
	// SSE2 header
#	include <emmintrin.h>	
	typedef __m128i hkQuadUint;
	typedef const hkIntVector& hkIntVectorParameter;
	typedef int hkPermuteMask;
#else
	struct hkQuadUint { HK_ALIGN16(hkUint32 u32[4]); };
	typedef const hkIntVector& hkIntVectorParameter;
	typedef int hkPermuteMask;
#endif

// handy for fallback implementations
union hkQuadUintUnion
{
	hkUint32 u[4];
	hkQuadUint q;
};

union hkQuadIntUnion
{
	hkInt32 u[4];
	hkQuadUint q;
};

union hkQuadShortUnion
{
	hkUint16 u[8];
	hkQuadUint q;
};

union hkQuadCharUnion
{
	hkUint8 u[16];
	hkQuadUint q;
};

/// Stores four hkUint32 in a SIMD friendly format.
class hkIntVector
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_MATH, hkIntVector);

			/// Clear
		HK_FORCE_INLINE void setZero();

			/// Load from 16 byte aligned address.
		HK_FORCE_INLINE void loadAligned(const hkUint32* p);

			/// Load from 4 byte aligned address.
		HK_FORCE_INLINE void loadUnaligned4(const hkUint32* p);

			/// Store to an aligned address
		HK_FORCE_INLINE void store(void* p) const; // not yet implemented

			/// Store to an unaligned address
		HK_FORCE_INLINE void storeUnaligned(void* p) const; // not yet implemented

#ifdef HK_INT_VECTOR_USE_TEMPLATE
		template<int idx> HK_FORCE_INLINE void setBroadcast(hkIntVectorParameter v);
#else
		HK_FORCE_INLINE void setBroadcast(hkIntVectorParameter v, int idx);
#endif

		const hkQuadUint& getQuad() const;
		hkQuadUint& getQuad();
		
		
			/// Logical Ops
		HK_FORCE_INLINE void setNot( hkIntVectorParameter a );
		HK_FORCE_INLINE void setOr( hkIntVectorParameter a, hkIntVectorParameter b );
		HK_FORCE_INLINE void setAnd( hkIntVectorParameter a, hkIntVectorParameter b ); 
		HK_FORCE_INLINE void setXor( hkIntVectorParameter a, hkIntVectorParameter b );
		HK_FORCE_INLINE void setAndC( hkIntVectorParameter a, hkIntVectorParameter b );

			/// Comparison 
		// Will implement these as needed. There are potentially a LOT of functions to implement here
		//HK_FORCE_INLINE void setCompareEqual(32/16/8)( hkIntVectorParameter a, hkIntVectorParameter b );
		//HK_FORCE_INLINE void setCompareLessThan(S/U)(32/16/8)( hkIntVectorParameter a, hkIntVectorParameter b );
		//HK_FORCE_INLINE void setCompareLessThanEqual(S/U)(32/16/8)( hkIntVectorParameter a, hkIntVectorParameter b );
		//HK_FORCE_INLINE void setCompareGreaterThan(S/U)(32/16/8)( hkIntVectorParameter a, hkIntVectorParameter b );
		//HK_FORCE_INLINE void setCompareGreaterThanEqual(S/U)(32/16/8)( hkIntVectorParameter a, hkIntVectorParameter b );
			// Need to convert between comparison results and hkVector4Comparison masks easily

			/// Integer arithmetic
		// Will implement these as needed. 
		//HK_FORCE_INLINE void setAddSaturate(S/U)(32/16/8)( hkIntVectorParameter a, hkIntVectorParameter b ) 
		//HK_FORCE_INLINE void setSubSaturate(S/U)(32/16/8)( hkIntVectorParameter a, hkIntVectorParameter b ) 
		HK_FORCE_INLINE void setAddSaturateU32( hkIntVectorParameter a, hkIntVectorParameter b );
		HK_FORCE_INLINE void setAddSaturateU16( hkIntVectorParameter a, hkIntVectorParameter b );
		HK_FORCE_INLINE void setSubSaturateU32( hkIntVectorParameter a, hkIntVectorParameter b );
		HK_FORCE_INLINE void setSubSaturateU16( hkIntVectorParameter a, hkIntVectorParameter b );
		//HK_FORCE_INLINE void setAddS32( hkIntVectorParameter a, hkIntVectorParameter b );
		//HK_FORCE_INLINE void setSubS32( hkIntVectorParameter a, hkIntVectorParameter b );



			/// Min / Max
		//HK_FORCE_INLINE void setMin(S/U)(32/16/8)( hkIntVectorParameter a, hkIntVectorParameter b ) 
		//HK_FORCE_INLINE void setMax(S/U)(32/16/8)( hkIntVectorParameter a, hkIntVectorParameter b ) 

			/// Logical Rotate / Shift
		//HK_FORCE_INLINE void setShiftLeft(32/16/8)( hkIntVectorParameter a, int shift );
		//HK_FORCE_INLINE void setShiftRight(32/16/8)( hkIntVectorParameter a, int shift );
		//HK_FORCE_INLINE void setRotateLeft(32/16/8)( hkIntVectorParameter a, int shift );
		//HK_FORCE_INLINE void setRotateRight(32/16/8)( hkIntVectorParameter a, int shift );

#ifdef HK_INT_VECTOR_USE_TEMPLATE
		template<int shitf> HK_FORCE_INLINE void setShiftLeft32( hkIntVectorParameter a );
		template<int shitf> HK_FORCE_INLINE void setShiftRight32( hkIntVectorParameter a );
#else
		HK_FORCE_INLINE void setShiftLeft32( hkIntVectorParameter a, int shift);
		HK_FORCE_INLINE void setShiftRight32( hkIntVectorParameter a, int shift );
#endif
		HK_FORCE_INLINE void setShiftLeft32( hkIntVectorParameter a, hkIntVectorParameter shift );
		HK_FORCE_INLINE void setShiftRight32( hkIntVectorParameter a, hkIntVectorParameter shift );


		// Merge 
		HK_FORCE_INLINE void setMergeHigh32( hkIntVectorParameter a, hkIntVectorParameter b );
		HK_FORCE_INLINE void setMergeLow32( hkIntVectorParameter a, hkIntVectorParameter b );
		HK_FORCE_INLINE void setMergeHigh16( hkIntVectorParameter a, hkIntVectorParameter b );
		HK_FORCE_INLINE void setMergeLow16( hkIntVectorParameter a, hkIntVectorParameter b );
		HK_FORCE_INLINE void setMergeHigh8( hkIntVectorParameter a, hkIntVectorParameter b ); 
		HK_FORCE_INLINE void setMergeLow8( hkIntVectorParameter a, hkIntVectorParameter b ); 

		// Pack (unmerge)
		HK_FORCE_INLINE void setPackModuloU32( hkIntVectorParameter a, hkIntVectorParameter b );

			/// Select, Shuffle
		HK_FORCE_INLINE void setShuffle( hkPermuteMask perm, hkIntVectorParameter b ); // not yet implemented

#ifdef HK_INT_VECTOR_USE_TEMPLATE		
		template<int A, int B, int C, int D> HK_FORCE_INLINE void setShuffle( hkIntVectorParameter a );
#else
		HK_FORCE_INLINE void setShuffle( hkIntVectorParameter a, int A, int B, int C, int D );
#endif

			///  (~select & a) | (select & b)
		HK_FORCE_INLINE void setSelect( hkIntVectorParameter select, hkIntVectorParameter a, hkIntVectorParameter b ); // not yet implemented
		HK_FORCE_INLINE void select32(hkIntVectorParameter a, hkIntVectorParameter b, hkVector4ComparisonParameter selectMask);

			/// Splat Immediate
#ifdef HK_INT_VECTOR_USE_TEMPLATE
		template<int val> HK_FORCE_INLINE void splatImmediate32();
		template<int val> HK_FORCE_INLINE void splatImmediate16();
		template<int val> HK_FORCE_INLINE void splatImmediate8(); 
#else
		HK_FORCE_INLINE void splatImmediate32( int val );
		HK_FORCE_INLINE void splatImmediate16( int val );
		HK_FORCE_INLINE void splatImmediate8 ( int val ); 
#endif

			/// Convert to from fixed point
		HK_FORCE_INLINE void convertToF32fromS32( hkVector4& vOut ) const; 
		HK_FORCE_INLINE void convertToF32fromU32( hkVector4& vOut ) const;
		HK_FORCE_INLINE void convertFromF32toU32( hkVector4Parameter vIn );
		HK_FORCE_INLINE void convertFromF32toS32( hkVector4Parameter vIn ); // not yet implemented
			// Add versions with scaling if needed?

	private:

		hkQuadUint m_quad;

	public:
		operator hkQuadUint() const
		{
			return m_quad;
		}

};

#if HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED
#	if defined(HK_COMPILER_HAS_INTRINSICS_IA32)
#		include <Common/Base/Math/Vector/Sse/hkSseIntVector.inl>
#	elif defined(HK_PLATFORM_PS3_PPU) || defined(HK_PLATFORM_PS3_SPU)
#		include <Common/Base/Math/Vector/Ps3/hkPs3IntVector.inl>
#	elif defined(HK_PLATFORM_XBOX360)
#		include <Common/Base/Math/Vector/Xbox360/hkXbox360IntVector.inl>
#	else
#		include <Common/Base/Math/Vector/hkIntVector.inl>
#	endif
#else
#	include <Common/Base/Math/Vector/hkIntVector.inl>
#endif

#endif //HK_MATH_INTVECTOR_H


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
