/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_INSPECT_PROCESS_H
#define HK_INSPECT_PROCESS_H

#include <Common/Visualize/hkProcess.h>
#include <Common/Visualize/hkProcessContext.h>

/// Given a set of tracked objects (as given by the list in hkVisualDebugger)
/// give the attached client the list, and upon request drill down and supply the 
/// actual data.
#define HK_INSPECT_FLAG_AUTOUPDATE 0x01
#define HK_INSPECT_FLAG_RECURSE    0x02

class hkInspectProcess : public hkReferencedObject, public hkProcess
{
	public:

		enum Command // for serialization
		{
			HK_SETUP = 0x20,
			HK_ADD_TOPLEVEL, // Add a top level object (two ids, one for object and one for the class) (server->client)
			HK_REMOVE_TOPLEVEL, // Remove a top level object (one id, just the object) (server->client)
			HK_REQUEST_OBJECT, // Request the data for a given object id (client->server)
			HK_ADD_OBJECT, // New object (can be a class) data or just refreshed. (bidirectional potentially)
			HK_UPDATE_MEMBER // Update the data for a given member. Sent over is the object id, along with enoygh nformation to odentify the exact member item (down to a data item)
		};
	
		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_VDB);
			
		static hkProcess* HK_CALL create(const hkArray<hkProcessContext*>& contexts);
			
		static void HK_CALL registerProcess();

		virtual void getConsumableCommands( hkUint8*& commands, int& numCommands );

		virtual void consumeCommand( hkUint8 command );

			/// Gets the tag associated with this viewer type
		virtual int getProcessTag() { return m_tag; }

		static inline const char* HK_CALL getName() { return "ObjectInspection"; }

			// step only preforms work if auto update is turned on for any objects
			// ie: m_autoUpdateList is not empyt
		virtual void step(hkReal frameTimeInMs);

		virtual void init();

	// internal stuff (public for the callbacks)
	public:

		struct ObjectPair
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_UTILITIES, hkInspectProcess::ObjectPair );

			const void* obj;
			const hkClass* klass;
		};

		int writeObject( const void* ptr, const hkClass& klass, hkBool recurse, hkArray<ObjectPair>& recursionStack );
		int addTopLevelObject( void* ptr, const hkClass& klass );
		int removeTopLevelObject( void* ptr );
				
		hkArray<ObjectPair> m_autoUpdateList;		
		class hkVisualDebugger* m_vdb;

		hkInspectProcess(const hkArray<hkProcessContext*>& contexts);
		virtual ~hkInspectProcess();

	protected:

		static int m_tag;

};

#endif	// HK_INSPECT_PROCESS_H

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
