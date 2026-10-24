/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef __hkRadixSort_H__
#define __hkRadixSort_H__

class hkRadixSort
{
	public:

		typedef HK_CLASSALIGN(struct,4)
		{
			union 
			{
				hkUint16 m_key;	
				hkUint8 m_keys[2];	
			};
			hkUint16 m_userData;
		} SortData16;


		typedef HK_CLASSALIGN(struct,4)
		{
			union 
			{
				hkUint32 m_key;	
				hkUint8 m_keys[4];	
			};
			hkUint32 m_userData;
		} SortData32;


			/// sort data according to m_key
			/// buffer needs to be of same size as data
		static void HK_CALL sort16( SortData16* data, int numData, SortData16* buffer );

			/// sort data according to m_key
			/// buffer needs to be of same size as data
		static void HK_CALL sort32( SortData32* data, int numData, SortData32* buffer );


};

#endif // __hkRadixSort_H__

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
