/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_SCENEDATA_HKX_ANIMATED_STRING_H
#define HK_SCENEDATA_HKX_ANIMATED_STRING_H

extern const hkClass hkxSparselyAnimatedStringClass;

	/// An hkxSparselyAnimatedString stores the values associated with an hkxAttribute of type string. The animation of values is stored as
	/// pairs (time, value).
struct hkxSparselyAnimatedString
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxSparselyAnimatedString );
	HK_DECLARE_REFLECTION();

		/// We put the string (char*) in a struct since char** would not be interpreted by the serialization as an array of strings.
	struct StringType
	{
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxSparselyAnimatedString::StringType );
		HK_DECLARE_REFLECTION();

		char* m_string;

		char* operator=(char* str)
		{
			m_string = str; 
			return m_string;
		}
	};

		/// The array of string values. We put those string (char*) in a struct since char** would not be interpreted by the 
		/// serialization as an array of strings.
	struct StringType* m_strings;
	int m_numStrings;

		/// The array of times associated with teach of the values in m_strings.
	float* m_times;
	int m_numTimes; 
};


#endif // HK_SCENEDATA_HKX_ANIMATED_STRING_H

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
