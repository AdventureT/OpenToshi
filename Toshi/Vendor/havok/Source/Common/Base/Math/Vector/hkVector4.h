/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_MATH_VECTOR4_H
#define HK_MATH_VECTOR4_H

#ifndef HK_MATH_MATH_H
#	error Please include Common/Base/hkBase.h instead of this file.
#endif

/// Stores four hkReals in a SIMD friendly format.
/// Although an hkVector4 - as its name suggests - actually stores four hkReals, 
/// it can be used to represent either a point or vector (x,y,z) in 3-space, ie. a 3-vector.
/// The final, or w, value of the hkVector4 defaults to zero.
/// Note that some methods have the suffix '3' or '4' - for instance, there is dot3()
/// method and a dot4() method. This suffix indicates whether the method operates
/// on the first three, or all four, elements of the hkVector4. 
/// As you can see, some methods have a '4' version but no '3' version because
/// there is no generic hardware support for that operation. Other methods,
/// such as setMul(), have no suffixes because there is no ambiguity regarding
/// the elements on which they operate. 
class hkVector4
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_MATH, hkVector4);

#ifndef HK_DISABLE_VECTOR4_CONSTRUCTORS
			/// Empty constructor.
		HK_FORCE_INLINE hkVector4() { }

			/// Creates a new hkVector4 from the specified three or four reals, with the obvious definition.
		HK_FORCE_INLINE hkVector4(hkReal x, hkReal y, hkReal z, hkReal w=0);

			/// Creates a new hkVector4, copying all 4 values from q.
		HK_FORCE_INLINE hkVector4(const hkQuadReal& q);

#ifndef HK_PLATFORM_PS3_SPU // This constructor causes the compiler NOT to pass const hkVector4's by register.
			/// Creates a new hkVector4, copying all 4 values from v.
		HK_FORCE_INLINE hkVector4( const hkVector4& v);
#endif
#endif
			/// Copies all 4 values from v.
		HK_ALWAYS_INLINE void operator= ( const hkVector4& v );

			/// Copies all 4 values from v.
		HK_ALWAYS_INLINE void operator= ( const hkQuadReal& v ); 
		
			/// Sets the hkVector4's components using the specified three or four reals, with the obvious definition.
		HK_ALWAYS_INLINE void set(hkReal x, hkReal y, hkReal z, hkReal w=0);

#if HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED
			/// Sets all the hkVector4's components to the same value x.
		HK_FORCE_INLINE void setAll(hkSimdRealParameter x);
#endif

			/// Sets all the hkVector4's components to the same value x.
		HK_ALWAYS_INLINE void setAll(hkReal x);

			/// Sets the xyz hkVector4's components to the same value x. The w component will be destroyed.
		HK_ALWAYS_INLINE void setAll3(hkReal x);

			/// Sets all values to zero.
		HK_ALWAYS_INLINE void setZero4();

			/// Sets the .w component as an integer with 24 bit resolution (for real experts only).
		HK_FORCE_INLINE void setInt24W( int value );

			/// Gets the .w component as an integer with 24 bit resolution (for real experts only).
		HK_FORCE_INLINE int getInt24W( ) const ;

			/// Sets this to be a, bypassing the processors cache.
		HK_FORCE_INLINE void storeUncached( void* dest) const;


		/* vector methods */

			/// Modifies this hkVector4 by adding the value of a to it.  All four components are modified.
		HK_ALWAYS_INLINE void add4(const hkVector4& a);

			/// Modifies this hkVector4 by adding the value of a to it.  Three components are modified, the w component is unspecified.
		HK_ALWAYS_INLINE void add3clobberW(const hkVector4& a);

			/// Sets this hkVector4 to be the sum of a and b. All four components are modified.
		HK_ALWAYS_INLINE void setAdd4(const hkVector4& a, const hkVector4& b);

			/// Modifies this hkVector4 by subtracting the value of a from it.  All four components are modified.
		HK_ALWAYS_INLINE void sub4(const hkVector4& a);

			/// Modifies this hkVector4 by subtracting the value of a from it.  All three components are modified, the .w component is unspecified.
		HK_FORCE_INLINE void sub3clobberW(const hkVector4& a);

			/// Sets this hkVector4 to be the difference of a and b. All four components are modified.
		HK_ALWAYS_INLINE void setSub4(const hkVector4& a, const hkVector4& b);

			/// Modifies this hkVector4 by setting it to be the product of itself and the vector a.  All four components are multiplied by a.
		HK_ALWAYS_INLINE void mul4(const hkVector4& a);

			/// Sets this hkVector4 to be the product of a and b. All four components are modified.
		HK_ALWAYS_INLINE void setMul4(hkVector4Parameter a, hkVector4Parameter b);

			/// Modifies this hkVector4 by setting it to be the product of itself with the real value a.  All four components are multiplied by a.
		HK_ALWAYS_INLINE void mul4(hkSimdRealParameter a);

			/// Sets this hkVector4 to be the product of the real value r and the vector b. All four components are modified.
		HK_ALWAYS_INLINE void setMul4(hkSimdRealParameter r, const hkVector4& a);

			/// Modifies this hkVector4 by adding the product of real r and vector a to it.  All four components are modified.
		HK_ALWAYS_INLINE void addMul4(hkSimdRealParameter r, const hkVector4& a);

			/// Modifies this hkVector4 by adding the component-wise product of a and b to it.  All four components are modified.
		HK_ALWAYS_INLINE void addMul4(hkVector4Parameter a, hkVector4Parameter b);

			/// Modifies this hkVector4 by subtracting the product of real r and vector a from it.  All four components are modified.
		HK_ALWAYS_INLINE void subMul4(hkSimdRealParameter r, const hkVector4& a);

			/// Modifies this hkVector4 by subtracting the component-wise product of a and b from it.  All four components are modified.
		HK_ALWAYS_INLINE void subMul4(hkVector4Parameter a, hkVector4Parameter b);

			/// Shortcut for *this = a, this->addMul(r,b).  All four components are modified.
		HK_ALWAYS_INLINE void setAddMul4(const hkVector4& a, const hkVector4& b, hkSimdRealParameter r);

			/// Shortcut for *this = a, this->addMul(x,y).  All four components are modified.
		HK_ALWAYS_INLINE void setAddMul4(const hkVector4& a, const hkVector4& x, const hkVector4& y);

			/// Sets this hkVector4 to be the cross product of a and b. (this = a cross b).
		HK_FORCE_INLINE void setCross( const hkVector4& a, const hkVector4& b );

			/// Sets this hkVector4 to a linear interpolation of the vectors a and b.
			/// The parameter t specifies the relative position along the line segment defined by
			/// a and b.  With t = 0, the start of the line segment - the point a - will be returned.
			/// The interpolation is calculated using the following formula: (1 - t) * a + t * b.
		HK_ALWAYS_INLINE void setInterpolate4( const hkVector4& a, const hkVector4& b, hkSimdRealParameter t);


		/* equality */

			/// Compares the x, y, z elements of the vectors and returns nonzero if equal to given epsilon.
		HK_FORCE_INLINE hkBool32 equals3(const hkVector4 &a, hkReal epsilon = 1e-3f ) const;

			/// Compares all elements of the vectors, including the w component, and returns nonzero if to given epsilon.
		HK_FORCE_INLINE hkBool32 equals4(const hkVector4 &a, hkReal epsilon = 1e-3f ) const;


		/// Elementwise compare.  Use the mask methods to examine the result.
		HK_FORCE_INLINE hkVector4Comparison compareLessThan4(hkVector4Parameter a) const;

		/// Elementwise compare.  Use the mask methods to examine the result.
		HK_FORCE_INLINE hkVector4Comparison compareEqual4(hkVector4Parameter a) const;

			/// Elementwise compare.  Use the mask methods to examine the result.
		HK_FORCE_INLINE hkVector4Comparison compareLessThanEqual4(hkVector4Parameter a) const;

			/// Elementwise compare.  Use the mask methods to examine the result.
		HK_FORCE_INLINE hkVector4Comparison compareLessThanZero4() const;

			/// Elementwise compare.  Use the mask methods to examine the result.
		HK_FORCE_INLINE hkVector4Comparison compareGreaterThan4(hkVector4Parameter a) const;

			/// Elementwise compare.  Use the mask methods to examine the result.
		HK_FORCE_INLINE hkVector4Comparison compareGreaterThanEqual4(hkVector4Parameter a) const;

			/// Return nonzero if x,y,z components of this are less than the corresponding elements of a.
		HK_FORCE_INLINE hkBool32 allLessThan3(hkVector4Parameter a) const;

			/// Return nonzero if all components of this are less than the corresponding elements of a.
		HK_FORCE_INLINE hkBool32 allLessThan4(hkVector4Parameter a) const;

			/// Each 32 bit component is set to the component of b if the corresponding bit in comp is set, otherwise a. i.e.
			/// this.x = comp.getMask( hkVector4Comparison::MASK_X ) ? b.x : a.x; and so on for each component.
		HK_FORCE_INLINE void select32( hkVector4Parameter a, hkVector4Parameter b, const hkVector4Comparison& comp);

		/* neg, abs, min, max */

			/// Sets this vectors components to -v.x,-v.y,-v.z,+v.w
		HK_FORCE_INLINE void setNeg3(const hkVector4& v);

			/// Sets this vector to the value of v with the signs of all the elements flipped.
		HK_FORCE_INLINE void setNeg4(const hkVector4& v);

			/// Sets this vector to the value of v with the signs of all the elements flipped if cond is true.
		HK_FORCE_INLINE void setNeg4If( int cond );

			/// Gets the absolute value of all elements in v and stores the result in this vector.
		HK_FORCE_INLINE void setAbs4(const hkVector4& v);


			/// Finds the element wise minimum of the elements in a and b and stores the result in this vector.
		HK_FORCE_INLINE void setMin4(const hkVector4& a, const hkVector4& b);

			/// Finds the element wise maximum of the elements in a and b and stores the result in this vector.
		HK_FORCE_INLINE void setMax4(const hkVector4& a, const hkVector4& b);

		/* matrix3, rotation, quaternion, transform */

			/// Sets this object to be the product of the matrix a and the vector b.
			/// The w component of the result is undefined.  (this = a * b)
		void setMul3(const hkMatrix3& a, const hkVector4& b ); 

			/// Sets this vector to the vector b rotated by the rotation matrix a. (this = a * b)
		void setRotatedDir(const hkRotation& a, const hkVector4& b);
			/// Sets this vector to the vector b rotated by the inverse of the rotation matrix a. (this = a^-1 * b)
			/// This operation depends on a being orthonormal.
		void setRotatedInverseDir(const hkRotation& a, const hkVector4& b);		
		
			/// Sets this vector to the vector b rotated by the quaternion a. (this = q b q^-1)
		HK_FORCE_INLINE void setRotatedDir(const hkQuaternion& q, const hkVector4& b);
			/// Sets this vector to the vector b rotated by the inverse of quaternion a. (this = q^-1 b q)
		HK_FORCE_INLINE void setRotatedInverseDir(const hkQuaternion& q, const hkVector4& b);

			/// Sets this vector to the vector b transformed by a. (this = a * b)
		void setTransformedPos(const hkTransform& a, const hkVector4& b);

			/// Sets this vector to the vector b transformed by the inverse of the transform a. 
			/// (this = a^-1 * b)
		void setTransformedInversePos(const hkTransform& a, const hkVector4& b);

			/// Sets this vector to the vector b transformed by a. (this = a * b): use _setTransformedPos if you want the inline version
		void setTransformedPos(const hkQsTransform& a, const hkVector4& b);

			/// Sets this vector to the vector b "inversely transformed" by the Qs transform a. 
			/// Notice that this is not the same as this := Inv(a) * b whenever scale is present
			/// Note:  use _setTransformedPos if you want the inline version
		void setTransformedInversePos(const hkQsTransform& a, const hkVector4& b);

		/* HK_FORCE_INLINE versions of above */

			/// Sets this object to be the product of the 3x3 matrix a and the 3 vector b.
			/// The w component of the result is undefined.  (this = a * b)
		HK_FORCE_INLINE void _setMul3(const hkMatrix3& a, const hkVector4& b ); 

			/// Sets this object to be the product of the 4x3 matrix a and the 3 vector.
			/// The w component of the result is defined.  (this = a * b)
		void _setMul4(const hkMatrix3& a, const hkVector4& b ); 

			/// Sets this object to be the product of the matrix a and the vector b.  
			/// (this = a * (bx,by,bz,1.f))
		HK_FORCE_INLINE void _setMul4xyz1(const hkTransform& a, const hkVector4& b ); 

			/// Inlined.  Sets this vector to the vector b rotated by the rotation matrix a. (this = a * b)
		HK_FORCE_INLINE void _setRotatedDir(const hkRotation& a, const hkVector4& b);

			/// Inlined.  Sets this vector to the vector b rotated by the inverse of the rotation matrix a. (this = a^-1 * b)
		HK_FORCE_INLINE void _setRotatedInverseDir(const hkRotation& a, const hkVector4& b);


			/// Inlined.  Sets this vector to the vector b transformed by a. (this = a * b)
		HK_FORCE_INLINE void _setTransformedPos(const hkTransform& a, const hkVector4& b);
			/// Inlined.  Sets this vector to the vector b transformed by the inverse of the transform a. 
			/// (this = a^-1 * b)
		HK_FORCE_INLINE void _setTransformedInversePos(const hkTransform& a, const hkVector4& b);

			/// Sets this vector to the vector b transformed by a. (this = a * b)
		HK_FORCE_INLINE void _setTransformedPos(const hkQsTransform& a, const hkVector4& b);

			/// Sets this vector to the vector b "inversely transformed" by the Qs transform a. 
			/// Notice that this is not the same as this := Inv(a) * b whenever scale is present
		HK_FORCE_INLINE void _setTransformedInversePos(const hkQsTransform& a, const hkVector4& b);

		/* length and distance */

			/// Returns the dot product of the 3-vector represented by the x, y and z components.
		HK_FORCE_INLINE hkSimdReal dot3(const hkVector4& a) const;

			/// Sets the x component only to the 3 element (x,y,z) dot product of a and b.
			/// The x,y,z values are destroyed.
		HK_FORCE_INLINE void setDot3(hkVector4Parameter a, hkVector4Parameter b);

			/// Returns the dot product of the 3-vector represented by the x, y and z components, is only using the FPU
		HK_FORCE_INLINE hkReal dot3fpu(const hkVector4& a) const;

			/// Returns the dot product of the 4-vector represented by the x, y, z and w components.
		HK_FORCE_INLINE hkSimdReal dot4(const hkVector4& a) const;

			/// Sets all 4 elements to the 4 element dot product of a and b.
		HK_FORCE_INLINE void setDot4(hkVector4Parameter a, hkVector4Parameter b);

			/// Returns this.x * a.x + this.y * a.y + this.z * a.z + this.w * 1.0f
			/// Note: the w component of a is replaced by 1.0f
		HK_FORCE_INLINE hkSimdReal dot4xyz1(const hkVector4& a) const;

			/// Returns x + y + z.
		HK_FORCE_INLINE hkSimdReal horizontalAdd3() const;

			/// Returns x + y + z + w.
		HK_FORCE_INLINE hkSimdReal horizontalAdd4() const;

			/// Returns the length of the 3-vector represented by the x, y, and z components
		HK_FORCE_INLINE hkSimdReal length3() const;
			/// Returns the squared length of the 3-vector represented by the x, y, and z components.
			/// This avoids the square root calculation.
		HK_FORCE_INLINE hkSimdReal lengthSquared3() const;
			/// Returns the inverse length of the 3-vector represented by the x, y, and z components.
		HK_FORCE_INLINE hkSimdReal lengthInverse3() const;
			/// Returns the length of the distance between this point and the point p.
		HK_FORCE_INLINE hkSimdReal distanceTo3( hkVector4Parameter p ) const;

			/// Returns the length of the 4-vector represented by the x, y, z and w components.
		HK_FORCE_INLINE hkSimdReal length4() const;
			/// Returns the squared length of the 4-vector represented by the x, y, z and w components.
			/// This avoids the square root calculation.
		HK_FORCE_INLINE hkSimdReal lengthSquared4() const;
			/// Returns the inverse length of the 4-vector represented by the x, y, z and w components.
		HK_FORCE_INLINE hkSimdReal lengthInverse4() const;

			/// Normalizes the 3-vector represented by the x, y, and z components.
			/// The w component of the result is undefined.
		HK_FORCE_INLINE void normalize3();
			/// Normalizes the 3-vector represented by the x, y, and z components, returning the original length.
			/// The w component of the result is undefined.
		HK_FORCE_INLINE hkSimdReal normalizeWithLength3();

			/// Normalizes the 4-vector represented by the x, y, z and w components.
		HK_FORCE_INLINE void normalize4();
			/// Normalizes the 4-vector represented by the x, y, z and w components, returning the original length.
		HK_FORCE_INLINE hkSimdReal normalizeWithLength4();

			/// Fast/low accuracy normalize of the 3-vector represented by the x, y, and z components. 
			/// The w component of the result is undefined.
		HK_FORCE_INLINE void fastNormalize3();

		/// Fast/low accuracy normalize of the 3-vector represented by the x, y, z components.
		/// Returns the original length.  The w component of the result is undefined.
		HK_FORCE_INLINE hkSimdReal fastNormalizeWithLength3();

		/// Fast/low accuracy normalize of the 3-vector represented by the x, y, z components.
		/// The w component of the result is undefined. Assumes that the input is non zero.
		HK_FORCE_INLINE void fastNormalize3NonZero();

		//HK_FORCE_INLINE hkSimdReal fastLengthInverse3();

			/// Normalizes the vector if the length of the 3-vector is not exactly zero. 
			/// Returns HK_SUCCESS if normalization occurred (length not zero)
			/// or HK_FAILURE if the 3-vector's length was exactly zero.
		HK_FORCE_INLINE hkResult normalize3IfNotZero ();

		/* element access */

			/// Gives read/write access to element i.  (0,1,2,3) correspond to the (x,y,z,w) components respectively.
		HK_FORCE_INLINE hkReal& operator() (int i);
			/// Gives read only access to element i.  (0,1,2,3) correspond to the (x,y,z,w) components respectively.
		HK_FORCE_INLINE const hkReal& operator() (int i) const;

			/// Set the xyz,w components.
		HK_FORCE_INLINE void setXYZW(const hkVector4& xyz, const hkVector4& w);

			/// Set the w component to be v.w
		HK_FORCE_INLINE void setW(const hkVector4& v);

			/// Set the xyz components. W component is unchanged.
		HK_FORCE_INLINE void setXYZ(const hkVector4& v);

			/// Set the xyz components. W component is set to zero.
		HK_FORCE_INLINE void setXYZ0(const hkVector4& xyz);

			/// Sets each element to the reciprocal of the corresponding element of v.
		HK_FORCE_INLINE void setReciprocal4(const hkVector4& v);

			/// Sets the xyz components to the reciprocal of the corresponding element of v, w set to 1.
		HK_FORCE_INLINE void setReciprocal3(const hkVector4& v);

			/// Sets each element to its inverse square root.
		HK_FORCE_INLINE void setSqrtInverse4(const hkVector4& v);

			/// Gives read only access to element i.  (0,1,2,3) correspond to the (x,y,z,w) components respectively.
			/// Using this method instead of operator() can give the compiler greater opportunity for optimization.
		HK_ALWAYS_INLINE hkSimdReal getSimdAt(int i) const;

			/// Faster version of setAll( this->getSimdAt(i) )
		HK_ALWAYS_INLINE void broadcast(int i);

			/// this.xyzw = v(i)
		HK_ALWAYS_INLINE void setBroadcast(hkVector4Parameter v, int i);

			/// this.xyz = v(i), w is unspecified, faster, especially on PlayStation(R)2
		HK_ALWAYS_INLINE void setBroadcast3clobberW(hkVector4Parameter v, int i);

			/// returns the index of the axis that is closest to this vector.
		HK_FORCE_INLINE	int getMajorAxis() const;

			/// Gives read/write access to the internal 128bit quad representation of the hkVector4.
		HK_ALWAYS_INLINE hkQuadReal& getQuad();

			/// Gives read only access to the internal 128bit quad representation of the hkVector4.
		HK_ALWAYS_INLINE const hkQuadReal& getQuad() const;

			/// Checks that the first 3 elements are valid numbers.
		hkBool isOk3() const;

			/// Checks that all elements are valid numbers.
		hkBool isOk4() const;

			/// Checks that the length of the vector is one
		hkBool isNormalized3(hkReal eps = 1e-4f) const;

			/// Checks that the length of the vector is one
		hkBool isNormalized4(hkReal eps = 1e-4f) const;

			/// Sets element i of this vector to zero.
			/// Calling this->zeroElement(0) zeroes the x component, 
			/// this->zeroElement(1) the y, this->zeroElement(2) the z, and this->zeroElement(3) the w.
		HK_FORCE_INLINE void zeroElement( int i );

			///Gets an hkVector4 with all components set to zero.
		HK_ALWAYS_INLINE static const hkVector4& HK_CALL getZero();

			/// Load from 4 byte aligned address. Last element is undefined.
		void load3(const hkReal* p);

			/// Load from 4 byte aligned address.
		void load4(const hkReal* p);

			/// Load from 16 byte aligned address.
		void load4a(const hkReal* p);

			/// Store to 4 byte aligned address. Last element is undefined.
		void store3(hkReal* p) const;

			/// Store to 4 byte aligned address.
		void store4(hkReal* p) const;

			/// Store to 16 byte aligned address.
		void store4a(hkReal* p) const;

	private:

#if HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED || defined(HK_PLATFORM_RVL)
		hkQuadReal m_quad;

	public:
		operator hkQuadReal() const
		{
			return m_quad;
		}

#else
		HK_ALIGN16( hkReal x );
		hkReal y;
		hkReal z;
		hkReal w;
#endif
};


#endif //HK_MATH_VECTOR4_H


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
