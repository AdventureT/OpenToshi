/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_TOOLBARGUPINTERFACE
#define INC_TOOLBARGUPINTERFACE

#include <iFnPub.h>

// Class ID
#define HK_TOOLBAR_GUP_CLASS_ID			Class_ID(0x16285733, 0x138c2dca)

/*
** FUNCTION PUBLISHING
*/

#define HK_TOOLBAR_GUP_FPINTERFACE_ID	Interface_ID(0x292c67fd, 0x7b0a4397)

enum 
{
	FPI_Hide, 
	FPI_Show, 
	FPI_Dock, 
	FPI_Undock,
	FPI_IsVisible,
	FPI_IsDocked,
	FPI_AddMacro,
	FPI_AddSeparator
};

/*
** C++ Interface to the Havok Content Tools Toolbar.
**
** Exposed to MAXScript as hctToolbarGUP
**
** The methods here correspond to the methods exposed to MaxScript.
** 
*/
class hctToolbarGUPInterface : public FPStaticInterface 
{
	public:

			// hctToolbarGUP.hide() -> hides the toolbar (if visible)
		virtual void iHide() = 0;

			// hctToolbarGUP.show() -> shows the toolbar (if hidden)
		virtual void iShow() = 0;

			// hctToolbarGUP.dock() -> docs the toolbar
		virtual void iDock() = 0;

			// hctToolbarGUP.undock() -> undocs the toolbar
		virtual void iUndock() = 0;

			// hctToolbarGUP.isVisible() -> returns true if the toolbar is not hidden
		virtual BOOL iIsVisible() = 0;

			// hctToolbarGUP.isDocked() -> returns true if the toolbar is docked
		virtual BOOL iIsDocked() = 0;

			// hctToolbarGUP.addMacro() -> adds a button based on a macroscript (of the given category + name)
		virtual void iAddMacro( TCHAR* category, TCHAR* name) =0;

			// hctToolbarGUP.addSeparator() -> adds a separator to the toolbar
		virtual void iAddSeparator () = 0;
};

#endif //INC_TOOLBARGUPINTERFACE

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
