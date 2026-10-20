/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_TREE_VIEW_MANAGER__H
#define HAVOK_TREE_VIEW_MANAGER__H

	/// This class implements some methods used to populate and handle user interaction with a tree-view representation of the
	/// contents that is being processed. It is used by filters such as ViewXML and CreateSkeleton.
class hctTreeViewManager
{
	public:
		
			/// Constructor - takes a variant to use as the root of the tree, and an optional class registry.
		hctTreeViewManager( const hkVariant* root, const hctFilterClassRegistry* reg ) : m_root(root), m_classReg(reg) {}
		virtual ~hctTreeViewManager() {}
		
			/// Determines whether a specific class type should appear in the tree.
		virtual bool isViewable( const hkVariant& variant ) { return true; }

			/// Determines whether a specific class type should be 'selectable' in some dialogs.
		virtual bool isSelectable( const hkVariant& variant ) { return isViewable( variant ); }
		
			/// Adds the root node to the specified tree control.
		void init( HWND treeWnd );
			
			/// Handles Win32 messages, especially the expansion of items in the tree.
		BOOL handleNotification( LPNMHDR n );
			
			/// Retrieves a variant structure for the currently selected tree item, if any.
		bool getSelectedVariant( struct hkVariant& var );
		
	protected:
		
			/// Gets the index of a variant in the tree item <-> variant map,
			/// adding a new entry if if was not found.
		int getVariantIndex( const struct hkVariant& var );

			/// Retrieves the name, the children and the index into the scene nodes bitmap,
			/// for any given variant.
		void getItemData( const struct hkVariant& var, hkString& name, hkArray< hkVariant >& children, int& imageIndex );

			/// Adds a variant as a child of a tree item.
		void addItem( HTREEITEM parent, const struct hkVariant& var );
		
		const hkVariant* m_root;
		const hctFilterClassRegistry* m_classReg;
		
		HWND m_treeWnd;
		hkArray< hkVariant > m_variantMap;
};

#endif // HAVOK_TREE_VIEW_MANAGER__H

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
