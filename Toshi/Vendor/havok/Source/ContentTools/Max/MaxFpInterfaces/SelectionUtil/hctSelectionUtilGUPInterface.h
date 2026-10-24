/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_SELECTIONUTILGUPINTERFACE
#define INC_SELECTIONUTILGUPINTERFACE

#include <iFnPub.h>

// Class ID
#define HK_SELECTION_UTIL_GUP_CLASS_ID	Class_ID(0x1623201a, 0x367c0374)

/*
** FUNCTION PUBLISHING
*/

#define HK_SELECTION_UTIL_GUP_FPINTERFACE_ID	Interface_ID(0x29130c9, 0x30cd04ab)

enum 
{
	FPI_GetVertSelectionsNames, 
	FPI_GetFaceSelectionsNames, 
	FPI_CreateNamedSelectionFromCurrent,
	FPI_OverwriteNamedVertSelectionFromCurrent,
	FPI_OverwriteNamedFaceSelectionFromCurrent,
	FPI_RenameVertSelection,
	FPI_RenameFaceSelection,
	FPI_DeleteVertSelection,
	FPI_DeleteFaceSelection,
	FPI_LoadVertSelection,
	FPI_LoadFaceSelection
};

/*
** C++ Interface to the Havok Content Tools Selection Utilities.
**
** Exposed to MAXScript as hctSelectionUtilGUP
**
** The methods here correspond to the methods exposed to MaxScript.
** 
*/
class hctSelectionUtilGUPInterface : public FPStaticInterface 
{
	public:

		virtual Tab<TCHAR*>* iGetVertSelectionsNames( INode* selectedNode ) = 0;
		virtual Tab<TCHAR*>* iGetFaceSelectionsNames( INode* selectedNode ) = 0;
		virtual BOOL iCreateNamedSelectionFromCurrent( INode* selectedNode, TCHAR* selectionName ) = 0;
		virtual BOOL iOverwriteNamedVertSelectionFromCurrent( INode* selectedNode, TCHAR* selectionName ) = 0;
		virtual BOOL iOverwriteNamedFaceSelectionFromCurrent( INode* selectedNode, TCHAR* selectionName ) = 0;
		virtual BOOL iRenameVertSelection( INode* selectedNode, TCHAR* oldName, TCHAR* newName ) = 0;
		virtual BOOL iRenameFaceSelection( INode* selectedNode, TCHAR* oldName, TCHAR* newName ) = 0;
		virtual BOOL iDeleteVertSelection( INode* selectedNode, TCHAR* name ) = 0;
		virtual BOOL iDeleteFaceSelection( INode* selectedNode, TCHAR* name ) = 0;
		virtual BOOL iLoadVertSelection( INode* selectedNode, TCHAR* name ) = 0;
		virtual BOOL iLoadFaceSelection( INode* selectedNode, TCHAR* name ) = 0;
};

#endif //INC_SELECTIONUTILGUPINTERFACE

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
