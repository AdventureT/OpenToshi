/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKBASE_HKOBJECTARRAY_H
#define HKBASE_HKOBJECTARRAY_H

#include <Common/Base/System/Io/OStream/hkOStream.h>

/// Array for objects with nontrivial constructors/destructors.
/// Object lifetimes: Memory at elements [0, getSize()) are live objects.
/// Memory at elements [getSize(),getCapacity()) is uninitialized.<p>
///
/// For performance reasons, order may not be preserved when deleting elements.<p>
///
/// Note: The following must be publicly accessible for each item in an object array:
/// <ul><li>default constructor, copy constructor, assignment, destructor
/// (either hand or compiler generated)</li>
/// <li>placement new, needed to construct objects in place. (You may need to use 
/// HK_DECLARE_CLASS_ALLOCATOR or HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR
/// for structs, nested classes, etc)</li></ul>
template <typename T>
class hkObjectArray
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_ARRAY, hkObjectArray<T>);

			/// Creates a zero length array.
		HK_FORCE_INLINE hkObjectArray();

			/// Creates an array of size n elements, with their default constructor.
		HK_FORCE_INLINE hkObjectArray(int initialSize);

			/// Creates an array of n elements initialized to 'fill'.
		HK_FORCE_INLINE hkObjectArray(int initialSize, const T& fill);

			/// Copies the array a.
		HK_FORCE_INLINE hkObjectArray(const hkObjectArray& a);

			/// Copies the array a.
		hkObjectArray& operator= (const hkObjectArray& a);

			/// Deallocates array memory.
		HK_FORCE_INLINE ~hkObjectArray();

			/// Read/write access to the i'th element.
		HK_FORCE_INLINE T& operator[] (int i);

			/// Read only access to the i'th element.
		HK_FORCE_INLINE const T& operator[] (int i) const;

			/// Read/write access to the last element.
		HK_FORCE_INLINE T& back();

			/// Read only access to the last element.
		HK_FORCE_INLINE const T& back() const;

			/// Returns the size of the array.
		HK_FORCE_INLINE int getSize() const;

			/// Returns the capacity of the array.
		int getCapacity() const;

			/// Checks if the size is zero.
		HK_FORCE_INLINE hkBool isEmpty() const;
		
			/// Sets the size to zero.
		HK_FORCE_INLINE void clear();

			/// Sets the size to zero and deallocate storage.
		void clearAndDeallocate();

			/// Removes the element at the specified index.
			/// This is done by moving the last element into index i
			/// and resizing by -1. This is very fast, but note that
			/// the order of elements is changed.
		void removeAt(int index);

			/// Removes the element at the specified index, copying the elements down one slot.
			/// Slower than removeAt(), but the order is unchanged.
		void removeAtAndCopy(int index);

			/// Returns the index of the first occurrence of t, or -1 if not found.
		int indexOf(const T& t) const;

			/// Returns the index of the last occurrence of t, or -1 if not found.
		int lastIndexOf(const T& t) const;
		
			/// Removes the last element.
		HK_FORCE_INLINE void popBack();

			/// Adds an element to the end.
		HK_FORCE_INLINE void pushBack(const T& e);

			/// Sets the minimum capacity to n.
		HK_FORCE_INLINE void reserve(int minCapacity);

			/// Sets the size to n.
		HK_FORCE_INLINE void setSize(int n);

			/// Sets the size to n.
			/// If the array is expanded, new elements are initialized with 'fill'.
		void setSize(int n, const T& fill);

			/// Increments the size by 1 and returns a reference to the first element created.
		HK_FORCE_INLINE T& expandOne( );

			/// Increments the size by n and returns a reference to the first element created.
		HK_FORCE_INLINE T* expandBy( int n );

			/// Insert the array a at index i.
		//void insertAt(int i, const hkObjectArray<T>& a);

			/// Insert t at index i.
		//void insertAt(int i, const T& t);

			/// Swaps this array's internal storage with 'a'.
		void swap(hkObjectArray<T>& a);


		typedef T* iterator;
		typedef const T* const_iterator;

		HK_FORCE_INLINE iterator begin();
		HK_FORCE_INLINE iterator end();
		HK_FORCE_INLINE const_iterator begin() const;
		HK_FORCE_INLINE const_iterator end() const;

	protected:

		friend class hkStatisticsCollector;

		void construct(T* t, int n);
		void destruct(T* t, int n);
		void construct(T* t, int n, const T& tinit);

		hkArray<T> m_array;
};


template <typename T>
hkOstream& operator<< (hkOstream& os, const hkObjectArray<T>& a);

#include <Common/Base/Container/Array/hkObjectArray.inl>

#endif // HKBASE_HKOBJECTARRAY_H

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
