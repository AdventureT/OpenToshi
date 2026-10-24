/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKBASE_ARRAY_STREAMWRITER_H
#define HKBASE_ARRAY_STREAMWRITER_H

#include <Common/Base/System/Io/Writer/hkStreamWriter.h>

/// Writer which uses an hkArray as its storage.
/// The written buffer area is from [0, hkArray.getSize()]
/// This class maintains a single null byte directly after
/// the buffer area so the the buffer may be interpreted
/// as a c style string.
class hkArrayStreamWriter : public hkStreamWriter
{
	public:

		enum ArrayOwnership
		{
			ARRAY_TAKE,
			ARRAY_BORROW
		};

		void nullTerminate()
		{
			m_arr->reserve(m_arr->getSize()+1);
			m_arr->begin()[ m_arr->getSize() ] = 0;
		}

		void* getData() { return m_arr->begin(); }
		int getDataSize() { return m_arr->getSize(); }

			/// Create an hkStreamWriter which writes in to the array *arr.
			/// The write position is initially set to the end of the array.
			/// If o is ARRAY_TAKE, this object owns the array and will destroy
			/// it in this objects destructor. If o is ARRAY_BORROW, the array
			/// will not be deleted.
		hkArrayStreamWriter(hkArray<char>* arr, ArrayOwnership o)
			: m_arr(arr), m_offset(arr->getSize()), m_ownerShip(o)
		{
			nullTerminate();
		}


		~hkArrayStreamWriter()
		{
			if( m_ownerShip == ARRAY_TAKE )
			{
				delete m_arr;
			}
		}

		virtual void clear()
		{
			m_arr->clear();
			m_offset = 0;
			nullTerminate();
		}

		virtual int write(const void* mem, int size)
		{
			HK_ASSERT2( 0x170ce358, m_offset <= m_arr->getSize(),
				"Array size has changed without a call to seek" );
			int spaceLeft = m_arr->getSize() - m_offset;
			if( size > spaceLeft )
			{
				int newSize = size + m_arr->getSize() - spaceLeft;
				m_arr->reserve( 1 + newSize );
				m_arr->setSizeUnchecked( newSize );
				m_arr->begin()[ newSize ] = 0;
			}
			else if( m_arr->getCapacity() > m_arr->getSize() )
			{
				m_arr->begin()[ m_arr->getSize() ] = 0;
			}
			char* p = m_arr->begin() + m_offset;
			hkString::memCpy(p, mem, size);
			m_offset += size;

			return size;
		}

		virtual hkBool isOk() const
		{
			return true;
		}

		virtual hkBool seekTellSupported() const
		{
			return true;
		}

		virtual hkResult seek(int offset, SeekWhence whence)
		{
			int absOffset = m_offset;
			switch( whence )
			{
				case STREAM_SET:
					absOffset = offset;
					break;
				case STREAM_CUR:
					absOffset = m_offset + offset;
					break;
				case STREAM_END:
					absOffset = m_arr->getSize() - offset;
					break;
				default:
					HK_ASSERT2(0x55f1b803, 0, "Bad 'whence' passed to seek()");
					break;
			}
			if( absOffset >= 0 )
			{
				if( absOffset > m_arr->getSize() )
				{
					m_arr->setSize( absOffset+1, 0 ); // zero filled space, null terminated
					m_arr->setSizeUnchecked( absOffset );
				}
				m_offset = absOffset;
				return HK_SUCCESS;
			}
			return HK_FAILURE;
		}

		virtual int tell() const
		{
			return m_offset;
		}

	protected:

		hkArray<char>* m_arr; // written chars always in area 0,getSize()
		int m_offset; // invariant: m_offset <= m_arr.getSize()
		ArrayOwnership m_ownerShip;
};

#endif // HKBASE_ARRAY_STREAMWRITER_H


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
