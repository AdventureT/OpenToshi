/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKSERIALZE_VERSIONING_EXCEPTIONS_ARRAY_H
#define HKSERIALZE_VERSIONING_EXCEPTIONS_ARRAY_H

extern const hkClass hkVersioningExceptionsArrayClass;

// This class is used to help us maintain version compatability
// When the demos are run we check the differences between the 
// class list from the previous version against the current builtin registry
// for any classes that differ an entry must be made in the exception file
// which states that the change has been catered for (either manually or automatically)
struct hkVersioningExceptionsArray
{
	struct VersioningException
	{
		VersioningException(const char* classname = HK_NULL, unsigned oldSignature = 0, unsigned newSignature = 0) 
			: m_className(classname), m_oldSignature(oldSignature), m_newSignature(newSignature) {}

		bool operator==(const VersioningException& other)
		{
			return ((m_oldSignature == other.m_oldSignature) &&
					(m_newSignature == other.m_newSignature) &&
					(hkString::strCmp( m_className, other.m_className ) == 0));
		}

		const char* m_className;
		unsigned m_oldSignature;
		unsigned m_newSignature;

	private:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SERIALIZE, hkVersioningExceptionsArray::VersioningException );
	};

	hkArray<struct VersioningException> m_exceptions;

	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SERIALIZE, hkVersioningExceptionsArray );
};


#endif // HKSERIALZE_VERSIONING_EXCEPTIONS_ARRAY_H

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
