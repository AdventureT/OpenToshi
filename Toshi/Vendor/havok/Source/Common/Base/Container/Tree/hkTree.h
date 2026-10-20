/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_BASE_TREE_H
#define HK_BASE_TREE_H

// Dont doc this class, users won't use it directly.
class hkTreeBase
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_TREE, hkTreeBase);

		struct Node;
		typedef Node* Iter;
		typedef void (HK_CALL *destructFunc)(void*);
		
		hkTreeBase(int nodesize);
		~hkTreeBase();

		// insert/remove
		Iter append(Iter iter, const void* value, int size);
		Iter remove(Iter iter, destructFunc destroy);

		// values
		void setValue(Iter iter, const void* value, int size );
		const void* getValue(Iter i) const;

		int getDepth(Iter i) const;
		int getNumChildren(Iter i) const;

		// iterator
		Iter iterGetRoot() const;
		Iter iterNext(Iter i) const;
		Iter iterNextPreOrder(Iter i) const;
		Iter iterParent(Iter i) const;
		Iter iterChildren(Iter i) const;

	private:

		Iter m_firstRoot;
		Iter m_lastRoot;
		int m_nodeSize;
};

/// N-ary tree with homogeneous values.
/// The values must be memcpy-able.
template <typename T>
class hkTree
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR(HK_MEMORY_CLASS_TREE, hkTree<T>);

		typedef hkTreeBase::Iter Iter;
		typedef hkTreeBase::destructFunc destructFunc;
		static void HK_CALL defaultDestruct(void* p);

			/// Create an empty tree.
			/// If your values need special destruction, pass in a destruct function.
		inline hkTree(destructFunc destruct=defaultDestruct);

			/// Destroy a tree and each of its nodes.
		~hkTree();

			// 
			// Insert/remove
			//

			/// Remove all values from the tree.
		inline void clear();

			/// Add a new node as the last child of iter.
			/// If iter is null, add it at root level.
		inline Iter append(Iter i, const T& t);

			/// Remove the subtree at iterator i. Iterator i is then invalid.
			/// For each node that is destroyed, destructFunc is called.
			/// Returns an iterator to the next element at the same level.
		inline Iter remove(Iter i);

			//
			// Value
        	//

			/// Set the value of a node.
		inline void setValue(Iter i, const T& t );

			/// Get the value at iter i.
		inline const T& getValue(Iter i) const;

			/// Get the depth at iter i. Root nodes have depth 0. Returns -1 if the tree is empty;
		inline int getDepth(Iter i) const;

			/// Return the number of children of node at i.
		inline int getNumChildren(Iter i) const;

			//
			// Iterators	
			//

			/// Root iterator.
		inline Iter iterGetRoot() const;

			/// Next node in pre-order (children before siblings).
		inline Iter iterNextPreOrder(Iter i) const;

			/// Next sibling.
		inline Iter iterNext(Iter i) const;

			/// Parent.
		inline Iter iterParent(Iter i) const;

			/// First child.
		inline Iter iterChildren(Iter i) const;

	private:

		destructFunc m_destruct;
		hkTreeBase m_tree;
};

#include <Common/Base/Container/Tree/hkTree.inl>

#endif //HK_BASE_TREE_H

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
