/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef SERIALIZE_UTILITIES_H
#define SERIALIZE_UTILITIES_H

#include <Common/Serialize/Packfile/hkPackfileWriter.h>
#include <Common/Base/Reflection/hkTypeInfo.h>
#include <Common/Base/Reflection/Registry/hkTypeInfoRegistry.h>

extern const hkTypeInfo hkClassTypeInfo;
extern const hkTypeInfo hkClassMemberTypeInfo;
extern const hkTypeInfo hkClassEnumTypeInfo;

// Saves and reloads the object using Reader and Writer of the provided types and performs the test using TestFunc.
// TestFunc must compare two objects of type Object and return HK_SUCCESS if the objects are considered equal.
template<typename Reader, typename Writer, typename Object>
static void serializeTest(const Object& object, const hkClass& klass, const hkTypeInfo& typeInfo, hkResult (*TestFunc)(const Object& o1, const Object& o2) )
{

	hkArray<char> newBuf;

	{
		hkOstream os(newBuf);
		Writer writer;
		writer.setContents( &object, klass );
		hkPackfileWriter::Options options;
		writer.save( os.getStreamWriter(), options );
	}

	hkOstream("dump.txt").write( newBuf.begin(), newBuf.getSize() );

	{
		hkTypeInfoRegistry localTypeInfoReg;
		// default classes
		localTypeInfoReg.registerTypeInfo(&hkClassTypeInfo);
		localTypeInfoReg.registerTypeInfo(&hkClassMemberTypeInfo);
		localTypeInfoReg.registerTypeInfo(&hkClassEnumTypeInfo);
		// unit test class
		localTypeInfoReg.registerTypeInfo(&typeInfo);

		hkIstream is(newBuf.begin(), newBuf.getSize());
		Reader reader;
		reader.loadEntireFile( is.getStreamReader() );
		Object* copy = static_cast<Object*>( reader.getContentsWithRegistry( klass.getName(), &localTypeInfoReg ) );

		HK_TEST( TestFunc(object, *copy) == HK_SUCCESS );
	}
}

#endif // SERIALIZE_UTILITIES_H

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
