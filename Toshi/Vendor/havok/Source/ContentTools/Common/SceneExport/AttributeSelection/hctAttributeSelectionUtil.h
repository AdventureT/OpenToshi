/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKSCENEEXPORT_ATTRIBUTE_SELECTION_UTIL_H
#define HKSCENEEXPORT_ATTRIBUTE_SELECTION_UTIL_H

#include <ContentTools/Common/SceneExport/AttributeSelection/hctAttributeSelection.h>

#include <Common/Serialize/Packfile/Xml/hkXmlPackfileReader.h>
#include <Common/Base/hkBase.h>

	/// This class loads user-defined attribute selection files and provides the exporters with utilities to 
	/// filter attributes based on the user selections
class hctAttributeSelectionUtil
{
	public:

		~hctAttributeSelectionUtil();

			/// Initializes an instance of this class. Usually a single instance is used to select attributes before export.
			/// A memory trackes is passed - it will be used for any allocations done during processing.
			hkBool init ( class hctFilterMemoryTracker* memoryTracker);

			/// Loads all attribute descriptions in the given path.
		hkBool loadAttributeSelections(const char* attributeSelectionPath );	

			/// Returned by "filterAttribute", it specifies the user's desire regarding that attribute.
		enum UserAction
		{
				/// The attribute should be exported
			USER_ADD,

				/// The attribute should not be exported
			USER_REMOVE,

				/// No user-specified behaviour
			USER_UNDEFINED,
		};

			/// Returns what was the choice of the user for the given attribute/attribute holder can possibly be selected for export.
			/// Matches will be checked for all the non-null values.
		UserAction filterAttribute (const char* typeName, const char* subTypeName = HK_NULL, const char* attributeName = HK_NULL) const;

	private:

		hctAttributeSelectionDatabase m_database;
		hkArray<hkXmlPackfileReader*> m_packfileReaders;

		// To keep track of what we allocate.
		hctFilterMemoryTracker* m_trackedMemory; 

		/*
		** Merging
		*/
		void mergeAttributeSelectionDatabase (const hctAttributeSelectionDatabase& newDatabase);

		// Reused for each othe addition/removal arrays
		void mergeAttributeSelectionArray (const int& newArraySize, const hctAttributeSelection* newArray, int& currentArraySizeRef, hctAttributeSelection* &currentArrayRef );
	
		// During merge we may need to create new arrays - keep track of them so we can delete them
		hkArray<hctAttributeSelection*> m_newAttributeSelections;

		/*
		** Matching
		*/

		hkBool matchAttributeSelection (const char* typeName, const char* subTypeName, const char* attributeName, const hctAttributeSelection& selection ) const;
};

#endif //HKSCENEEXPORT_ATTRIBUTE_SELECTION_UTIL_H

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
