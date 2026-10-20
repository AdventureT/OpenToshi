/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_VISUALIZE_VERSION_REPORTER
#define HK_VISUALIZE_VERSION_REPORTER

class hkStreamWriter;

/// This class manages version information between the server and client.
/// Currently the proto call version is the same as the build number and
/// for a client to work with a server they must have the same version
/// number.  Please always use the visual debugger client that shipped
/// with the packages you received.
class hkVersionReporter 
{
	public:

		enum Command // for serialization
		{
			HK_VERSION_INFORMATION = 0x90
		};

		
			/// Sends a data 'chunk' which contains the version information
			/// of the Havok libs that the senders process/executable is
			/// linked with.
		static hkResult HK_CALL sendVersionInformation( hkStreamWriter* connection );

	public:

		static int m_protocalVersion;
		static int m_protocalMinimumCompatible;
};

#endif // HK_VISUALIZE_VERSION_REPORTER



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
