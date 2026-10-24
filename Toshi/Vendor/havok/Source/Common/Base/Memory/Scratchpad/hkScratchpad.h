/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKBASE_HKSCRATCHPAD_H
#define HKBASE_HKSCRATCHPAD_H

/// A scratchpad is a small piece of contiguous memory used for
/// temporary calculations. On supported hardware it may map to 
/// a physically faster portion of memory, for instance PlayStation(R)2.
class hkScratchpad : public hkReferencedObject, public hkSingleton<hkScratchpad>
{
	public:
	
		hkScratchpad( void* p, int sz)
			: m_address(p), m_sizeInBytes(sz), m_inUse(false)
		{
		}

			/// Acquire the scratchpad memory exclusively.
		void* acquire()
		{
			HK_ASSERT(0x20ad62c3,  m_inUse == false );
			m_inUse = true;
			return m_address;
		}

			/// Release the memory to allow it to be acquired again.		
		void release()
		{
			HK_ASSERT(0x651498c4,  m_inUse == true );
			m_inUse = false;
		}
		
			/// Return the size of the memory block.
		int getSizeInBytes() const
		{
			return m_sizeInBytes;
		}
		
	protected:

		void* m_address;	
		int m_sizeInBytes;
		hkBool m_inUse;
};

#endif // HKBASE_HKSCRATCHPAD_H

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
