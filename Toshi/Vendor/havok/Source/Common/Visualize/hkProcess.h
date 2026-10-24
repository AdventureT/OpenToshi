/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_VISUALIZE_REMOTE_PROCESS_H
#define HK_VISUALIZE_REMOTE_PROCESS_H

#include <Common/Visualize/Serialize/hkDisplaySerializeOStream.h>

class hkDisplaySerializeIStream;
class hkDebugDisplayHandler;
class hkProcessHandler;

class hkProcess 
{
	public:

		/// Virtual destructor for base class.
		hkProcess( hkBool selectable ) 
			: m_selectable(selectable) {}

		virtual ~hkProcess() {}

		virtual int getProcessTag() = 0;
		
		//
		// General state and settings
		//
		hkBool m_selectable; // can it be selected by the client or is just always on on the server
	
		//
		// The streams and interfaces the process can use to communticate.
		//
		hkDisplaySerializeIStream* m_inStream; 
		hkDisplaySerializeOStream* m_outStream; 
		hkDebugDisplayHandler* m_displayHandler;
		hkProcessHandler* m_processHandler;

		//
		// Command Interface (How it processes incoming data)
		//

			/// Get the commands this handler understands so 
			/// that this handler can be mapped correctly and process only 
			/// called when appropriate
		virtual void getConsumableCommands( hkUint8*& commands, int& numCommands ) { commands = HK_NULL; numCommands = 0; }
		
			/// given a input stream and a command (that has already been parsed from
			/// the stream), consume the data.
		virtual void consumeCommand( hkUint8 command ) { }
			
			/// We can say, for any current process we have, that it is 
			/// ok if its output stream is still ok ( alot don't use input, but all use output)
		virtual bool isOk() { return m_outStream && m_outStream->isOk(); }

		//
		// Process step interface
		//
		virtual void init() { } // after the process handler has created the process, it sets up the stream ptrs etc, then calls this init func

			/// step (a lot don't bother to impl this as driven by callbacks)
		virtual void step( hkReal frameTimeInMs  ) { }
};

#endif // HK_VISUALIZE_REMOTE_PROCESS_H

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
