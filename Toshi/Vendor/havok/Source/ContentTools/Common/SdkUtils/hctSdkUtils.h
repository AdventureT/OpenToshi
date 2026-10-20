/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_COMMON_UTILS_H
#define INC_COMMON_UTILS_H

#include <Common/Base/hkBase.h>
#include <Common/Base/Reflection/hkClass.h>
#include <Common/Base/Reflection/hkClassMemberAccessor.h>

#include <Common/Base/Types/Geometry/hkGeometry.h>
#include <Common/Base/Container/Array/hkObjectArray.h>

#include <Common/Base/Fwd/hkwindows.h>

#define HCT_MAX_NUM_CLASS_ENTRIES 128

struct hctModelerNodeIdToClassMap
{
	int            m_3dsMaxClassId;
	int            m_mayaNodeId;
	const hkClass* m_class;
	const char*    m_nodeName;
	const char*    m_displayName;
};


class hctSdkUtils
{
	public:

		/// A modeler independent abstraction of a "progress bar", to monitor the utility's progress and terminate it if necessary
		struct ProgressUpdater
		{
			virtual void progression (float percentage) =0;
			virtual bool didUserCancel () =0;

			const char* m_currentObjectName;
		};


			/// loads all available class dlls and retrieves the modeler node information
		static void loadAllClasses();

			/// unloads the available class dlls and releases all memory
		static void unloadAllClasses();

			/// Finds all the classes that either match the class pointed to by targetAttributeName or inherit
			/// from it. If the target class isn't currently loaded, no matching classes are returned.
			/// Also, if the the targetAttributeName attribute is not a member of the specified class, no
			/// matching classes will be returned.
			/// Returns the number of matching classes
		static int getMatchingClasses(const char* targetNodeName, const char* targetAttributeName, hkObjectArray<hkString>& matchingClassesOut);

			/// Returns either the class or HK_NULL if not registered/loaded.
		static const hkClass* getClassByName(const char* className);

			/// Fills the supplied array with those class map entries whose classes are derived from the supplied base class.
		static void getClassesByBaseName(const char* baseName, hkArray<hctModelerNodeIdToClassMap>& classesOut);

		static int getIndexByNodeName(const char* className);

		static const hkArray<hctModelerNodeIdToClassMap>& getNodeIdToClassMap() {return m_modelerNodeIdToClassMap;}

			/// Returns the number of all registered classes.
		static int getNumUiClasses();

			/// Returns the i'th registered class from the table.
		static hctModelerNodeIdToClassMap& getUiClass(int i);

			// Sometimes there can be conflicts with certain modeler-internal objects/names.
			// This function helps in getting a non-colliding name for any given Havok name.
		static const char* getValidName(const char* name, hkString& buffer);

			// This function will 'revert' getValidName() and will return the original name.
		static const char* revertValidName(const char* validModelerName, hkString& buffer);

			// Get the name of the HCT .chm help file.
		static void getHelpFileName(hkString& helpFile);

			// Get the path to the HCT documentation .CHM file.
		static void getPathToAndNameOfDocumentationChmFile(hkString& docsPathAndFileName);

	private:

			/// Returns a module pointer if it is able to load the class dll specified in the path
		static HMODULE loadClassDll(const char* dllName);

		static hkArray<hctModelerNodeIdToClassMap> m_modelerNodeIdToClassMap;

};

/// Prototype for the DLL-exported method
typedef void (__cdecl *hkGetClassEntriesFunc) (hkArray<hctModelerNodeIdToClassMap>& classMap);


#endif // INC_COMMON_UTILS_H


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
