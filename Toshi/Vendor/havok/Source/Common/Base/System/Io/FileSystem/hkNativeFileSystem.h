/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_DEFAULTFILESYSTEM
#define HK_DEFAULTFILESYSTEM

#include <Common/Base/hkBase.h>
#include <Common/Base/System/Io/Reader/hkStreamReader.h>
#include <Common/Base/System/Io/Writer/hkStreamWriter.h>
#include <Common/Base/System/Io/FileSystem/hkFileSystem.h>
#include <Common/Base/System/Io/Reader/Buffered/hkBufferedStreamReader.h>
#include <Common/Base/System/Io/Writer/Buffered/hkBufferedStreamWriter.h>

#	include <Common/Base/System/Io/Reader/Stdio/hkStdioStreamReader.h>
#	include <Common/Base/System/Io/Writer/Stdio/hkStdioStreamWriter.h>
#	include <Common/Base/System/Io/Directory/Win32/hkWin32ListDirectory.h>


// FileSystem class implementation for native file systems.
// Uses default reader, writer and file system browsers for each platform
// All the paths and file names must be in Havok standard format i.e. the
// only character accepted as a separator is '/'.
// e.g. A valid path is Dir/SubDir/SubSubDir/Filename
class hkNativeFileSystem : public hkFileSystem
{
public:
		// Function pointer type for directory listing.
	typedef hkResult (HK_CALL *ListDirectoryFunType)( const char* pathIn, hkFileSystem::DirectoryListing& directoryListingOut );
		// Function pointer type for converting Havok paths to platform paths.
	typedef const char* (HK_CALL *HavokToPlatformConvertPathFunType)( const char* pathIn, hkArray<char>& buffer );
		// Function pointer type for converting platform paths to Havok paths.
	typedef const char* (HK_CALL *PlatformToHavokConvertPathFunType)( const char* pathIn, hkArray<char>& buffer );

		// Function pointer for directory listing.
		// Replace the function to modify listDirectory behavior.
	static ListDirectoryFunType s_listDirectory;
		// Pointer to function for converting Havok paths to platform paths.
		// Replace the function to modify havokToPlatformConvertPath behavior.
	static HavokToPlatformConvertPathFunType s_havokToPlatformConvert;
		// Pointer to function for converting platform paths to Havok paths.
		// Replace the function to modify platformToHavokConvertPath behavior.
	static PlatformToHavokConvertPathFunType s_platformToHavokConvert;

		// Default conversions functions
	static const char* HK_CALL nativeHavokToPlatformConvertPath( const char* pathIn, hkArray<char>& buffer );
	static const char* HK_CALL nativePlatformToHavokConvertPath( const char* pathIn, hkArray<char>& buffer );


	virtual hkStreamReader* openReader( const char* name )
	{
		hkArray<char> buffer;
		hkStreamReader* s = new DefaultFileReader( havokToPlatformConvertPath( name, buffer ) );
		if( s->markSupported() == false )
		{
			hkStreamReader* b = new hkBufferedStreamReader(s);
			s->removeReference();
			return b;
		}
		return s;
	}

	virtual hkStreamWriter* openWriter( const char* name )
	{
		hkArray<char> buffer;
		hkStreamWriter* s = new DefaultFileWriter( havokToPlatformConvertPath( name, buffer ) );
		hkStreamWriter* b = new hkBufferedStreamWriter(s);
		s->removeReference();
		return b;
	}

	static hkReferencedObject* create()
	{
		return new hkNativeFileSystem();
	}


		// Converts a Havok path to platform path. Set s_havokToPlatformConvert
	HK_FORCE_INLINE static const char* HK_CALL havokToPlatformConvertPath( const char* pathIn, hkArray<char>& buffer )
	{
		return s_havokToPlatformConvert( pathIn, buffer );
	}
		// Converts a platform path to Havok path
	HK_FORCE_INLINE static const char* HK_CALL platformToHavokConvertPath( const char* pathIn, hkArray<char>& buffer )
	{
		return s_platformToHavokConvert( pathIn, buffer );
	}

		/// list all the directories and files in the "basePath" directory,
		/// returns HK_FAILURE if the path is not valid
		/// basePath must be in Havok format, all the generated paths and file names
		/// in listingOut will be in Havok format too.
	virtual hkResult listDirectory(const char* basePath, DirectoryListing& listingOut)
	{
		return s_listDirectory( basePath, listingOut );
	}
};

#endif //HK_DEFAULTFILESYSTEM


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
