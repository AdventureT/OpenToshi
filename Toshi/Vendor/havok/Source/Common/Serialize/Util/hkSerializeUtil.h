/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_SERIALIZE_UTIL_H
#define HK_SERIALIZE_UTIL_H

#include <Common/Serialize/Util/hkStructureLayout.h>
#include <Common/Serialize/Tagfile/hkTagfileWriter.h>
#include <Common/Serialize/Packfile/hkPackfileWriter.h>

class hkClass;
class hkResource;
class hkDataObject;
class hkStreamReader;
class hkStreamWriter;
class hkClassNameRegistry;
class hkTypeInfoRegistry;

/// hkSerializeUtil utility functions
namespace hkSerializeUtil
{
		/// Load serialized objects from stream and return pointer
		/// to hkResource object.
		/// To access hkResource objects you should call hkResource::getContents()
		/// or hkResource::getContentsWithRegistry() specifying type of top level
		/// object pointer you expect.
		/// hkResource is reference counted and users are responsible to
		/// remove reference when they do not need the hkResource content.
		/// The objects owned by hkResource are not reference counted.
	hkResource* HK_CALL load( hkStreamReader* sr );

		/// Load serialized objects from a file using name and return pointer
		/// to hkResource object.
		/// See hkSerializeUtil::load( hkStreamReader* sr ) for details.
	hkResource* HK_CALL load( const char* filename );

		/// Load serialized objects from buffer and return pointer
		/// to hkResource object. The buffer may be freed after loading.
		/// See hkSerializeUtil::load( hkStreamReader* sr ) for details.
	hkResource* HK_CALL load( const void* buf, int buflen );

		/// Error details class for loading packfile/tagfile.
	struct ErrorDetails
	{
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SERIALIZE, ErrorDetails );

			/// ErrorDetails constructor.
		ErrorDetails() : id(ERRORID_NONE) {}

			/// Error ids for loading packfile/tagfile.
		enum ErrorID
		{
				/// No errors reading from stream.
			ERRORID_NONE,
				/// Failed to read from stream.
			ERRORID_READ_FAILED,
				/// Unsupported file format.
			ERRORID_UNSUPPORTED_FORMAT,
				/// Packfile cannot be read on this platform.
			ERRORID_PACKFILE_PLATFORM,
				/// Versioning cannot be applied to provided
			ERRORID_VERSIONING_FAILED
		};
			/// Error id. Default is ErrorDetails::ERROR_NOT_FOUND.
		hkEnum<enum ErrorID, hkInt32> id;
			/// Detailed error message, if error id is not ErrorDetails::ERROR_NOT_FOUND.
		hkString defaultMessage;
	};

		/// Load serialized objects from stream and return pointer
		/// to hkResource object. If the 'classReg' is HK_NULL the
		/// class name registry from hkBuiltinTypeRegistry is used instead.
		/// The function does additional check for the stream format, versioning and finishing steps.
		/// It will return HK_NULL if the checks failed. The error details are
		/// returned in 'resultOut'.
		/// Usually you should use hkSerializeUtil::load().
	hkResource* HK_CALL loadWithDetails( hkStreamReader* stream, ErrorDetails& resultOut, const hkClassNameRegistry* classReg = HK_NULL );

		/// Save a snapshot of a given object in packfile form using provided writer.
		/// Returns HK_SUCCESS if successful.
		/// If you don't provide a target layout then the current host layout is assumed.
		/// NOTE: Some objects are inherently not serializable because they point to
		/// external memory, i.e. the vertex and index arrays of an hkpMeshShape.
		/// These objects should be converted before saving, i.e hkpMeshShape -> hkpStorageMeshShape
		/// (see hkpHavokSnapshot::ConvertListener class for details).
		/// If you want these conversions applied you should provide a hkpHavokSnapshot::ConvertListener as
		/// a value for userListener.
	hkResult HK_CALL savePackfile( const void* object, const hkClass& klass, hkStreamWriter* writer, const hkPackfileWriter::Options& options, hkBool32 binary = true, hkPackfileWriter::AddObjectListener* userListener = HK_NULL );

		/// Save a snapshot of a given object in tagfile form using provided writer.
		/// Returns HK_SUCCESS if successful.
		/// NOTE: Some objects are inherently not serializable because they point to
		/// external memory, i.e. the vertex and index arrays of an hkpMeshShape.
		/// These objects should be converted before saving, i.e hkpMeshShape -> hkpStorageMeshShape
		/// (see hkpHavokSnapshot::ConvertListener class for details).
		/// If you want these conversions applied you should provide a hkpHavokSnapshot::ConvertListener as
		/// a value for userListener.
	hkResult HK_CALL saveTagfile( const void* object, const hkClass& klass, hkStreamWriter* writer, hkBool32 binary = true, hkPackfileWriter::AddObjectListener* userListener = HK_NULL );

		/// Save a given data object in tagfile form using provided writer.
		/// Returns HK_SUCCESS if successful.
	hkResult HK_CALL saveTagfile( const hkDataObject& object, hkStreamWriter* writer, hkBool32 binary = true );
}

#endif // HK_SERIALIZE_UTIL_H


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
