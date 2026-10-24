/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKSCENEDATA_HKX_ATTRIBUTE_HOLDER_H
#define HKSCENEDATA_HKX_ATTRIBUTE_HOLDER_H

struct hkxAttributeHolder
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SCENE_DATA, hkxAttributeHolder );
	HK_DECLARE_REFLECTION();


		/// Attributes associated with this node.
		/// You can use hkxAttribute::findObjectByName and related methods
		/// to search the array for a given attribute
		/// They are organized by groups, each group with a string type identifier
	struct hkxAttributeGroup* m_attributeGroups;
	int m_numAttributeGroups;

		/// Looks for an attribute group by name 
	const hkxAttributeGroup* findAttributeGroupByName (const char* name) const;

		/// Search an array of attributes for one by name, and optionally
		/// check the type of that attribute to make sure it matches the 
		/// given class. If no attribute matches by name (and by class if given)
		/// it will return HK_NULL.
		/// Will search all groups;
	void* findAttributeObjectByName( const char* name, const hkClass* type = HK_NULL ) const;

		/// Search the attributes for one by name and return the variant  
		/// of that attribute. Will return HK_NULL if not found.
		/// Will search all groups;
	const hkVariant* findAttributeVariantByName( const char* name ) const;

};

#endif // HKSCENEDATA_HKX_ATTRIBUTE_HOLDER_H

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
