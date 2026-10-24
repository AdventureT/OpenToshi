/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_VISUALIZE__VISUAL_DEBUGGER_DEBUG_OUTPUT_H
#define HK_VISUALIZE__VISUAL_DEBUGGER_DEBUG_OUTPUT_H

extern int hkVisualDebuggerDebugLevel;

/// Setting the level to zero will result in most information being suppressed
/// only error messages will be displayed at this level
/// Setting it to 1 and higher will enable more detail levels of debug output...
#define HK_VISUAL_DEBUGGER_INFO_SET_LEVEL(LEVEL) \
	do { hkVisualDebuggerDebugLevel = LEVEL; } while(0)

/// Displays debug information regarding the visual debugger.
/// These messages can be placed on both the client and server side
/// This information is sent to the standard output and does not
/// effect the network payload in anyway.
/// The debug levels start at 1 where this is the highest level
/// information.  2 is more detailed and so on...
#define HK_VISUAL_DEBUGGER_INFO(LEVEL, INFO) \
	if(LEVEL <= hkVisualDebuggerDebugLevel) \
	{ \
		hkcout << "VisualDebugger: ["; \
		if(LEVEL == 0) \
		{ \
			hkcout << "ERROR: "; \
		} \
		hkcout << INFO << ']' << hkendl; \
	}

#endif // HK_VISUALIZE__VISUAL_DEBUGGER_DEBUG_OUTPUT_H

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
