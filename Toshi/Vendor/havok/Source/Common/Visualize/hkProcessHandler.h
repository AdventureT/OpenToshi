/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_VISUALIZE_PROCESS_HANDLER
#define HK_VISUALIZE_PROCESS_HANDLER

/// This class defines the interface to control the state of viewers 
/// in the visual debugger.  Implementations exist on both the client 
/// and server side.
class hkProcessHandler
{
	public:
		
		enum Command // for serialization
		{
			HK_REGISTER_PROCESS = 0xC0, 
			HK_SELECT_PROCESS,
			HK_CREATE_PROCESS,
			HK_DELETE_PROCESS
		};

		virtual ~hkProcessHandler() {}

			/// Informs the plugin handler about a viewer.  It and then be created and
			/// destroyed by calling the createViewer and deleteViewer methods.
		virtual hkResult registerProcess(const char* name, int id) = 0;

			/// Query for the id of a named process type.
		virtual int getProcessId(const char* name) = 0;
	
			/// Query for the name of a given id.
		virtual const char* getProcessName( int id ) = 0;

			/// Instructs the plugin handler to create a viewer by default when a
			/// a connection is made to the visual debugger.  When the server (or
			/// simulation) calls selectViewer(id) this will cause the client to send
			/// back a createViewer(id) without intervention from the user.  This is
			/// used to make sure that geometry data and debug points and lines are
			/// always displayed by default when a new connection is made to a visual
			/// debugger server.
		virtual hkResult selectProcess(int id) = 0;

			/// Creates an instance of the viewer identified by id in the server (or simulation).
			/// The viewer is only created if one does not already exist.
		virtual hkResult createProcess(int id) = 0;

			/// Deletes a viewer, identified by id, from the server.
		virtual hkResult deleteProcess(int id) = 0;
};

#endif // HK_VISUALIZE_PROCESS_HANDLER





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
