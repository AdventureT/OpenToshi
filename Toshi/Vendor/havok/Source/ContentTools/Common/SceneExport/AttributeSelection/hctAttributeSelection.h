/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKSCENEEXPORT_ATTRIBUTE_SELECTION__H
#define HKSCENEEXPORT_ATTRIBUTE_SELECTION__H

extern const hkClass hctAttributeSelectionClass;
extern const hkClass hctAttributeSelectionDatabaseClass;
extern const hkTypeInfo hctAttributeSelectionTypeInfo;
extern const hkTypeInfo hctAttributeSelectionDatabaseTypeInfo;

/// An hctAttributeSelectionDatabase represents a table of entries (attribute selection) specifying sets of attributes that
/// the user wants a scene exporter to sample and export.
struct hctAttributeSelectionDatabase
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_EXPORT, hctAttributeSelectionDatabase );
	HK_DECLARE_REFLECTION();

		/// The array of attribute selections (additions)
	struct hctAttributeSelection* m_attributeAdditions;
	int m_numAttributeAdditions;

	/// The array of attribute selections (removals)
	struct hctAttributeSelection* m_attributeRemovals;
	int m_numAttributeRemovals;

};

/// An hctAttributeSelection contains an entry specifying a user selection of one or multiple attributes inside particular
/// node/parameter block/property. The "*" character can be used as a wildcard for any of the strings. 
/// The interpretation of each field is specific to the individual modeller - check the Havok
/// Content Tools documentation (integration section) for details.
struct hctAttributeSelection
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_EXPORT, hctAttributeSelection );
	HK_DECLARE_REFLECTION();

		/// The typename is the higher-level name of the modeller entity that holds the attribute. This is different for each modeller.
	const char* m_typeName;

		/// Some modelers have two levels of containment of attributes, and use this "subTypeName" value.
	const char* m_subTypeName;
	
		/// A list of attribute names. Wildcards can be used here as well.
	const char** m_attributeNames;
	int m_numAttributeNames;

};


#endif //HKSCENEEXPORT_ATTRIBUTE_SELECTION__H

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
