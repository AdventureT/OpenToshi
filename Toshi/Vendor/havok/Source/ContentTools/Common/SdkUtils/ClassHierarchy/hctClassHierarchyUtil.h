/* 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_HCT_CLASS_HIERARCHY_UTIL_H
#define INC_HCT_CLASS_HIERARCHY_UTIL_H


#include <string>
#include <vector>

#include <Common/Base/Reflection/Attributes/hkAttributes.h>


class hctClassHierarchyUtil
{
	public:

		struct ParamGroup
		{
			public:

				struct ClassAndMember
				{
					std::string          m_memberLabel;				// member's label in string form
					std::string          m_memberInternalName;		// member's internal name in string form
					std::string          m_defaultValue;			// member's default value in string form
					const hkClass*       m_class;					// the class the current member sits in
					const hkClassMember* m_member;					// the member information
					int                  m_classMemberIndex;		// member's index inside its class
					hkBool               m_visible;					// member's visibility flag
				};

			public:

				ParamGroup(const char* groupLabel, const char* groupInternalName)
				{
					m_groupLabel        = groupLabel;
					m_groupInternalName = groupInternalName;
					m_visuallyEmpty     = false;
				}

				HK_FORCE_INLINE void addChild(ParamGroup* child)
				{
					m_subGroups.push_back(child);
				}

			public:

				const char* m_groupLabel;
				std::string m_groupInternalName;
				hkBool      m_visuallyEmpty;

				std::vector<ClassAndMember> m_members;
				std::vector<ParamGroup*>    m_subGroups;
		};

	public:

		static const hkUiAttribute*        getUiAttributes       (const hkClassMember& member);
		static const hkGizmoAttribute*     getGizmoAttributes    (const hkClassMember& member);
		static const hkLinkAttribute*      getLinkAttributes     (const hkClassMember& member);
		static const hkSemanticsAttribute* getSemanticsAttributes(const hkClassMember& member);
		static const hkArrayTypeAttribute* getArrayTypeAttributes(const hkClassMember& member);
		static const char*                 getMemberName         (const hkClassMember& member, const hkUiAttribute* uiAttributes);

		static void createDisplayHierarchyFromClass(const hkClass* klass, std::string& groupPathDelimiter, hkReal unitConversionModifier, std::vector<hctClassHierarchyUtil::ParamGroup*>& groupsArrayOut);

		static void buildTreeFromClassDescription(const hkClass& klass, const hkClassMember& member, int classMemberIndex, const std::string* namePrefix, const std::string groupPathDelimiter, hkReal unitConversionModifier, std::vector<ParamGroup*>& virtualRecursionStack, std::vector<ParamGroup*>& advancedGroupVirtualRecursionStack);

		static void pruneEmptyGroups(std::vector<ParamGroup*>& groupsArray);

		static void pruneInvisibleMembers(hkUiAttribute::HideInModeler modelerId, std::vector<ParamGroup*>& groupsArray);

		static void pruneMembersByClassType(const char* parentClassName, std::vector<hctClassHierarchyUtil::ParamGroup*>& groupsArray);

		static void markVisuallyEmptyGroup(hkUiAttribute::HideInModeler modelerId, std::vector<ParamGroup*>& groupsArray);

		static void flattenHierarchy(ParamGroup* group, std::vector<ParamGroup*>& rootLevelArray);

		static bool isParentClassOf(const char* parentClassName, const hkClass* klass);

			/// This utility function will assert that the supplied string is "more than" C-style compliant, i.e. it will:
			/// - remove all non-alpha-numeric characters (by thus shortening the string)
			/// - make the first character lowercase
			/// - make sure the first character is a letter (and no digit)
		static void verifyAlphaNumericString(std::string& inputString);

	protected:

		static hkString prettifyName(const char* nameIn);
		static void     getDefaultValueAsString(const ParamGroup::ClassAndMember& memberData, hkReal unitConversionModifier, std::string& defaultValueStringOut);
		static bool     pruneEmptyGroupsRecursively(ParamGroup* group);
		static void     pruneInvisibleMembersRecursively(hkUiAttribute::HideInModeler modelerId, ParamGroup* group);
		static void     pruneMembersByClassTypeRecursively(const char* parentClassName, hctClassHierarchyUtil::ParamGroup* group);
		static void     markVisuallyEmptyGroupRecursively(hkUiAttribute::HideInModeler modelerId, ParamGroup* group);
		static void     mergeGroupsOfSameName(std::vector<hctClassHierarchyUtil::ParamGroup*>& groupsArray);
};


#endif // INC_HCT_CLASS_HIERARCHY_UTIL_H

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
