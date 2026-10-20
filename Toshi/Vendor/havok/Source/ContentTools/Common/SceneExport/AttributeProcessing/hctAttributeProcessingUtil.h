/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_SCENE_EXPORT_ATTRIBUTES_H
#define HK_SCENE_EXPORT_ATTRIBUTES_H

#include <Common/Serialize/Packfile/Xml/hkXmlPackfileReader.h>

#include <Common/Base/hkBase.h>

#include <Common/SceneData/Graph/hkxNode.h>

#include <Common/SceneData/Attributes/hkxAttribute.h>

#include <ContentTools/Common/SceneExport/AttributeProcessing/hctAttributeDescription.h>

	/// An instance of this class can be used to handle the manipulation of attributes before export (the transformation
	/// of attributes based on hctAttributeDescription objects). 
class hctAttributeProcessingUtil
{
	public:

			/// Initializes an instance of this class. Usually a single instance is used to process all attributes before export.
			/// A memory trackes is passed - it will be used for any allocations done during processing.
		hkBool init ( class hctFilterMemoryTracker* memoryTracker);
		
			/// Loads all attribute descriptions in the given path.
		hkBool loadAttributeDescriptions(const char* attributeDescriptionPath );	

			/// Given an attribute holder (node/material), it merges attribute groups with the same name and then processes each group,
			/// following the instructions in the attribute descriptions stored in the database.
		void processAttributes (hkxAttributeHolder* attributeHolder);

			/// Given two attribute groups, merges them into a third one.
		void mergeTwoAttributeGroups(const hkxAttributeGroup& groupOne, const hkxAttributeGroup& groupTwo, hkxAttributeGroup& mergedGroup);

			/// Returns true if no valid attribute description data has been loaded yet.
		hkBool isEmpty () const;

			/// Destructor, frees up allocated memory.
		~hctAttributeProcessingUtil();

	private:

		hctAttributeDescriptionDatabase m_database;
		hkArray<hkXmlPackfileReader*> m_packfileReaders;

		// To keep track of what we allocate.
		hctFilterMemoryTracker* m_trackedMemory; 

	private:

		// Given an attribute group, it processes the attributes inside it, following the instructions in the attribute description.
		void processAttributeGroup (hkxAttributeGroup& attrGroup);

		// Merging (EXP-497)
		void mergeAttributeDescriptionDatabase (const hctAttributeDescriptionDatabase& newDatabase);
		void mergeAttributeGroupDescriptions (const hctAttributeGroupDescription& newGroupDesc, hctAttributeGroupDescription& currentGroupDesc);
		void mergeAttributeDescriptions(const hctAttributeDescription& newAttDesc, hctAttributeDescription& currentAttDesc);
		// When we merge we may have to create new arrays of group descriptions or attribute descriptions
		// We keep track of them so we can delete them
		hkArray<hctAttributeGroupDescription*> m_newGroupDescriptions;
		hkArray<hctAttributeDescription*> m_newAttDescriptions;

		void mergeXYZToVector (hkArray<hkxAttribute>& attributes);
		void scaleFloats (hkArray<hkxAttribute>& attributes, const struct hctAttributeGroupDescription* attrGroupDesc);
		void mergeTransAndRotToMatrix (hkArray<hkxAttribute>& attributes);
		void enforceTypes (hkArray<hkxAttribute>& attributes, const struct hctAttributeGroupDescription* attrGroupDesc);
		void enforceChangePairs (hkArray<hkxAttribute>& attributes);
		void enforceEnabledBy (hkArray<hkxAttribute>& attributes, const struct hctAttributeGroupDescription* attrGroupDesc);
		void enforceHints (hkArray<hkxAttribute>& attributes, const struct hctAttributeGroupDescription* attrGroupDesc);

		// Supported attribute conversions
		struct hkxAnimatedQuaternion* convertEulerToQuaternion (struct hkxAnimatedVector* animatedVector);
		struct hkxAnimatedQuaternion* convertMatrixToQuaternion (struct hkxAnimatedMatrix* animatedMatrix);
		struct hkxSparselyAnimatedEnum* convertIntToEnum (struct hkxSparselyAnimatedInt* animatedInt, const class hkClassEnum* enumClass);
		struct hkxSparselyAnimatedString* convertIntToString (struct hkxSparselyAnimatedInt* animatedInt, const class hkClassEnum* enumClass);
		struct hkxSparselyAnimatedString* convertEnumToString (struct hkxSparselyAnimatedEnum* animatedEnum);

};

#endif //HK_SCENE_EXPORT_ATTRIBUTES_H

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
