/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKBASE_HKARRAY_H
#define HKBASE_HKARRAY_H

#ifndef hkArrayAllocator
#	define hkArrayAllocator hkThreadMemory::getInstance()
#endif

/// Common functionality for all hkArray types.
/// These are out of line functions to avoid code bloat.
namespace hkArrayUtil
{
	void HK_CALL _reserve(void*, int numElem, int sizeElem);
	void HK_CALL _reserveMore(void* array, int sizeElem);
	void HK_CALL _reduce(void* array, int sizeElem, char* inplaceMem, int requestedCapacity);
}

template <typename T> class hkObjectArray;

/// The default Havok array.
/// Note that, for performance reasons, order may not be preserved when deleting elements.<br>
///	<br>
/// This is <i>not</i> a complete replacement for the STL array,
/// and should be used only with plain old data types
/// (or objects that have a trivial constructor, destructor,
/// copy constructor and assignment operator that are equivalent
/// to memcpy), as constructors, destructors, etc. are not called.
/// If you need to create an array of objects, you should use an hkObjectArray.
template <typename T>
class hkArray
{
		friend void HK_CALL hkArrayUtil::_reserve(void*, int numElem, int sizeElem);
		friend void HK_CALL hkArrayUtil::_reserveMore(void* array, int sizeElem);
		friend void HK_CALL hkArrayUtil::_reduce(void* array, int sizeElem, char* inplaceMem, int requestedCapacity);
		friend class hkObjectArray< hkArray<T> >;
		friend class hkArraySpu;

	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_ARRAY, hkArray<T>);

			/// Creates a zero length array.
		HK_FORCE_INLINE hkArray();

			/// Creates an array of size n. All elements are uninitialized.
		explicit HK_FORCE_INLINE hkArray(int size);

			/// Creates an array of n elements initialized to 'fill'.
		HK_FORCE_INLINE hkArray(int size, const T& fill);

			/// Noncopying initialization from an existing external buffer.
			/// This does not copy the array but uses it in place until its capacity
			/// is exceeded at which point a reallocation occurs and the array behaves
			/// like a normal hkArray.
			/// The caller must ensure that the buffer is valid for the lifetime
			/// of this array and for deallocation of the buffer.
		HK_FORCE_INLINE hkArray(T* buffer, int size, int capacity);

	private:
			/// Not publicly accessible, too easy to call accidentally.
		hkArray(const hkArray& a);

	public:

			/// Use a user supplied array as storage.
			/// The external array must be valid for the lifetime of this array.
		void useExternalArray(T*, int numElements);

			/// Use a user supplied buffer as storage.
			/// The external buffer must be valid for the lifetime of this array.
		void useExternalBuffer(void*, int numBytes);

			/// Copies the array a.
		hkArray& operator= (const hkArray& a);

			/// Deallocates array memory.
		HK_FORCE_INLINE ~hkArray();

			/// Read/write access to the i'th element.
		HK_FORCE_INLINE T& operator[] (int i);

			/// Read only access to the i'th element.
		HK_FORCE_INLINE const T& operator[] (int i) const;

			/// Read/write access to the last element.
		HK_FORCE_INLINE T& back();

			/// Read only access to the last element.
		HK_FORCE_INLINE const T& back() const;

			/// Returns the size.
		HK_FORCE_INLINE int getSize() const;

			/// Returns the capacity.
		HK_FORCE_INLINE int getCapacity() const;

			/// Checks if the size is zero.
		HK_FORCE_INLINE hkBool isEmpty() const;

			/// Sets the size to zero.
		HK_FORCE_INLINE void clear();

			/// Sets the size to zero and deallocates storage.
		void clearAndDeallocate();

			/// Tries to reduce the capacity to avoid wasting storage. If shrinkExact is true the resulting capacity
			/// is size+numElementsLeft
		HK_FORCE_INLINE void optimizeCapacity( int numFreeElemsLeft, hkBool32 shrinkExact=false );

			/// Removes the element at the specified index. The last array element is used to replace the removed element, and the size is reduced by 1.
			/// This is very fast, but note that the order of elements is changed.
		void removeAt(int index);

			/// Removes the element at the specified index, copying elements down one slot as in the STL array.
			/// Slower than removeAt(), but the order is unchanged.
		void removeAtAndCopy(int index);

			/// Removes several elements at the specified index, copying elements down as in the STL array.
		void removeAtAndCopy(int index, int numToRemove);

			/// Returns the index of the first occurrence of t, or -1 if not found.
		int indexOf(const T& t, int start=0, int end=-1) const;

			/// Returns index of the last occurrence of t, or -1 if not found.
		int lastIndexOf(const T& t) const;

			/// Removes the last element.
		HK_FORCE_INLINE void popBack( int numElemsToRemove = 1 );

			/// Returns the specified subarray of this array.
			///
			/// begin and end are inclusive and exclusive respectively.
			/// i.e. {0,1,2,3}.getSubarray(1,3) == { 1, 2 }
		hkArray<T> getSubarray(int begin, int end);

			/// Adds an element to the end.
		HK_FORCE_INLINE void pushBack(const T& e);

			/// Adds an element to the end. No check for resize.
		HK_FORCE_INLINE void pushBackUnchecked(const T& e);

			/// Ensures no reallocation occurs until at least size n.
		HK_FORCE_INLINE void reserve(int n);

			/// Ensures no reallocation occurs until size n.
		HK_FORCE_INLINE void reserveExactly(int n);

			/// Sets the size.
			/// If the array is expanded, new elements are uninitialized.
		HK_FORCE_INLINE void setSize(int size);

			/// Sets the size to n.
			/// If the array is expanded, new elements initialized with 'fill'.
		void setSize(int n, const T& fill);

			/// Sets the size assuming the capacity to be sufficient.
			/// If the array is expanded, new elements are uninitialized.
		HK_FORCE_INLINE void setSizeUnchecked(int size);

			/// Checks if the locked flag is set.
			/// If an array is locked, the storage has come from file
			/// (so the do not deallocate flag is set) but it also
			/// means that the destructor will never be called so if you resize
			/// the array you will have to make sure and call clearAndDeallocate()
			/// yourself.
		HK_FORCE_INLINE hkBool isLocked();

			/// Overrides the lock state. If you unlock and then resize
			/// the array then the destructor will never be called, so
			/// you will have to make sure and call clearAndDeallocate()
			/// yourself.
		HK_FORCE_INLINE void setLocked( bool locked );

			/// Increments the size by 1 and returns a reference to the first element created.
		HK_FORCE_INLINE T& expandOne( );

			/// Increments the size by n and returns a pointer to the first element created.
		HK_FORCE_INLINE T* expandBy( int n );

			/// Increments the size by n and returns a pointer to the first element created.  No check for resize!
		HK_FORCE_INLINE T* expandByUnchecked( int n );

			/// Expands the array by numToInsert at the specified index.
			/// See also getSubarray() and the constructor which uses an existing
			/// C style array in place.
		HK_FORCE_INLINE T* expandAt( int index, int numToInsert );

			/// Inserts the array a at index i.
			/// See also getSubarray() and the constructor, which uses an existing
			/// C style array in place.
		void insertAt(int i, const T* p, int numElems );

			/// Replaces elements [i,i+ndel) with the supplied array.
			/// This method avoids redundant copying associated with separate remove & insert steps.
		void spliceInto(int i, int ndel, const T* p, int numElems );

			/// Inserts t at index i.
			/// Elements from i to the end are copied up one place.
		void insertAt(int i, const T& t);

			/// Swaps this array's internal storage with 'a'.
		void swap(hkArray<T>& a);

			///
		typedef T* iterator;
			///
		typedef const T* const_iterator;

			/// Returns an STL-like iterator to the first element.
		HK_FORCE_INLINE iterator begin();
			/// Returns an STL-like iterator to the 'one past the last' element.
		HK_FORCE_INLINE iterator end();
			/// Returns an STL-like const iterator to the first element.
		HK_FORCE_INLINE const_iterator begin() const;
			/// Returns an STL-like const iterator to the 'one past the last' element.
		HK_FORCE_INLINE const_iterator end() const;

		static void HK_CALL copy(T* dst, const T* src, int n);

		static void HK_CALL copyBackwards(T* dst, const T* src, int n);

			/// Advanced use only.
		HK_FORCE_INLINE void setOwnedData(T* ptr, int size, int capacity);

            /// Get the capacity and the flags - advanced use
        HK_FORCE_INLINE int getCapacityAndFlags() const;
	public:

		// Public so that the serialization can access it.
		enum
		{
			CAPACITY_MASK = int(0x3FFFFFFF),
			FLAG_MASK = int(0xC0000000),
			DONT_DEALLOCATE_FLAG = int(0x80000000), // Indicates that the storage is not the array's to delete
			LOCKED_FLAG = int(0x40000000),  // Indicates that the array will never have its dtor called (read in from packfile for instance)
			FORCE_SIGNED = -1
		};

	protected:

		friend class hkStatisticsCollector;

		void releaseMemory();

		T* m_data;
		int m_size;
		int m_capacityAndFlags; // highest 2 bits indicate any special considerations about the allocation for the array

	public:

			/// For serialization, we want to initialize the vtables
			/// in classes post data load, and NOT call the default constructor
			/// for the arrays (as the data has already been set).
		hkArray(hkFinishLoadedObjectFlag f) { }
};

/// Array that has an internal storage capacity within the class itself.
/// Originally hkArray::m_data points to hkInplaceArray::m_storage.
/// It is safe to expand the capacity beyond the internal capacity. In this
/// case the array behaves like a normal hkArray (i.e. m_data points to heap
/// memory instead of to &m_storage[0]).
/// Note that once the builtin capacity has been exceeded,
/// the inplace elements are unused even if subsequently resized smaller
/// than the original capacity.
template <typename T, unsigned N>
class hkInplaceArray : public hkArray<T>
{
	public:

		typedef hkInplaceArray<T,N> mytype;

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_ARRAY, mytype );

			/// Creates an array with the specified initial size.
		HK_FORCE_INLINE hkInplaceArray(int size = 0);

		HK_FORCE_INLINE hkInplaceArray(const hkInplaceArray<T,N>& a);

		HK_FORCE_INLINE ~hkInplaceArray(){}

			/// Copies the array a.
		hkArray<T>& operator= (const hkArray<T>& a);

			/// Copies the array a.
		hkArray<T>& operator= (const hkInplaceArray<T,N>& a);

			/// Tries to reduce the capacity to avoid wasting storage
		HK_FORCE_INLINE void optimizeCapacity( int numFreeElemsLeft, hkBool32 shrinkExact=false );

		inline hkBool wasReallocated() const;

			/// returns true if the array is still using its inplace buffer.
			/// This check is done using only the mask field, so do not use
			/// this function for arrays which can be serialized
		inline int stillInplaceUsingMask() const;

	public:

		T m_storage[N];

	public:

		/// For serialization, we want to initialize the vtables
		/// in classes post data load, and NOT call the default constructor
		/// for the arrays (as the data has already been set).
		hkInplaceArray(hkFinishLoadedObjectFlag f) : hkArray<T>(f) { }
};


/// An array that has a small internal storage capacity, aligned to 16 bytes within the class itself.
template <typename T, unsigned N>
class hkInplaceArrayAligned16 : public hkArray<T>
{
	public:

		typedef hkInplaceArray<T,N> mytype;

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_ARRAY, mytype);

			/// Creates an array with the specified initial size.
		HK_FORCE_INLINE hkInplaceArrayAligned16(int size = 0);

		HK_FORCE_INLINE ~hkInplaceArrayAligned16(){}

			/// Copies the array a.
		hkArray<T>& operator= (const hkArray<T>& a);

			/// Copies the array a.
		hkArray<T>& operator= (const hkInplaceArrayAligned16<T,N>& a);

		inline hkBool wasReallocated() const;

	protected:

		int m_padding; // sizeof(base class) + padding == 16 bytes
		HK_ALIGN16( hkUint8 m_storage[sizeof(T) * N] );

	public:

		/// For serialization, we want to initialize the vtables
		/// in classes post data load, and NOT call the default constructor
		/// for the arrays (as the data has already been set).
		hkInplaceArrayAligned16(hkFinishLoadedObjectFlag f) : hkArray<T>(f) { }
};

#include <Common/Base/Container/Array/hkArray.inl>
#undef hkArrayAllocator

#endif // HKBASE_HKARRAY_H


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
