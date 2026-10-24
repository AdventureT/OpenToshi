/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKBASE_HKSTRINGREFARRAY_H
#define HKBASE_HKSTRINGREFARRAY_H

#include <Common/Base/System/Io/OStream/hkOStream.h>
#include <Common/Base/Container/String/hkStringPtr.h>

/// Array of hkStringPtr objects, based on hkStringPtrArray.
class hkStringPtrArray
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_ARRAY, hkStringPtrArray);

			/// Creates a zero length array.
		HK_FORCE_INLINE hkStringPtrArray();

			/// Creates an array of size n elements, with their default constructor.
		HK_FORCE_INLINE hkStringPtrArray(int initialSize);

		HK_FORCE_INLINE hkStringPtrArray( hkFinishLoadedObjectFlag f );

			/// Creates an array of n elements initialized to 'fill'.
		HK_FORCE_INLINE hkStringPtrArray(int initialSize, const hkStringPtr& fill);

			/// Copies the array a.
		HK_FORCE_INLINE hkStringPtrArray(const hkStringPtrArray& a);

			/// Copies the array a.
		HK_FORCE_INLINE hkStringPtrArray& operator= (const hkStringPtrArray& a);

			/// Deallocates array memory.
		HK_FORCE_INLINE ~hkStringPtrArray();

			/// Read/write access to the i'th element.
		HK_FORCE_INLINE hkStringPtr& operator[] (int i);

			/// Read only access to the i'th element.
		HK_FORCE_INLINE const hkStringPtr& operator[] (int i) const;

			/// Read/write access to the last element.
		HK_FORCE_INLINE hkStringPtr& back();

			/// Read only access to the last element.
		HK_FORCE_INLINE const hkStringPtr& back() const;

			/// Returns the size of the array.
		HK_FORCE_INLINE int getSize() const;

			/// Returns the capacity of the array.
		HK_FORCE_INLINE int getCapacity() const;

			/// Checks if the size is zero.
		HK_FORCE_INLINE hkBool isEmpty() const;
		
			/// Sets the size to zero.
		HK_FORCE_INLINE void clear();

			/// Sets the size to zero and deallocate storage.
		HK_FORCE_INLINE void clearAndDeallocate();

			/// Removes the element at the specified index.
			/// This is done by moving the last element into index i
			/// and resizing by -1. This is very fast, but note that
			/// the order of elements is changed.
		HK_FORCE_INLINE void removeAt(int index);

			/// Removes the element at the specified index, copying the elements down one slot.
			/// Slower than removeAt(), but the order is unchanged.
		HK_FORCE_INLINE void removeAtAndCopy(int index);

			/// Returns the index of the first occurrence of t, or -1 if not found.
		HK_FORCE_INLINE int indexOf(const hkStringPtr& t) const;

			/// Returns the index of the last occurrence of t, or -1 if not found.
		HK_FORCE_INLINE int lastIndexOf(const hkStringPtr& t) const;
		
			/// Removes the last element.
		HK_FORCE_INLINE void popBack();

			/// Adds an element to the end.
		HK_FORCE_INLINE void pushBack(const hkStringPtr& e);

			/// Sets the minimum capacity to n.
		HK_FORCE_INLINE void reserve(int minCapacity);

			/// Sets the size to n.
		HK_FORCE_INLINE void setSize(int n);

			/// Sets the size to n.
			/// If the array is expanded, new elements are initialized with 'fill'.
		HK_FORCE_INLINE void setSize(int n, const hkStringPtr& fill);

			/// Increments the size by 1 and returns a reference to the first element created.
		HK_FORCE_INLINE hkStringPtr& expandOne( );

			/// Increments the size by n and returns a reference to the first element created.
		HK_FORCE_INLINE hkStringPtr* expandBy( int n );

			/// Swaps this array's internal storage with 'a'.
		HK_FORCE_INLINE void swap(hkStringPtrArray& a);

		typedef hkStringPtr* iterator;
		typedef const hkStringPtr* const_iterator;

		HK_FORCE_INLINE iterator begin();
		HK_FORCE_INLINE iterator end();
		HK_FORCE_INLINE const_iterator begin() const;
		HK_FORCE_INLINE const_iterator end() const;

	private:

		HK_FORCE_INLINE void construct(hkStringPtr* t, int n);
		HK_FORCE_INLINE void destruct(hkStringPtr* t, int n);
		HK_FORCE_INLINE void construct(hkStringPtr* t, int n, const hkStringPtr& tinit);

		hkArray<hkStringPtr> m_array;
};

HK_FORCE_INLINE hkOstream& operator<< (hkOstream& os, const hkStringPtrArray& a);

#include <Common/Base/Container/Array/hkStringPtrArray.inl>

#endif // HKBASE_HKSTRINGREFARRAY_H

/*
* Havok SDK - NO SOURCE PC DOWNLOAD, BUILD(#20090704)
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
