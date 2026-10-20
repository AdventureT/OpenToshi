/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKSERIALZE_VERSION_CHECKING_UTILS_H
#define HKSERIALZE_VERSION_CHECKING_UTILS_H

#include <Common/Serialize/Version/hkVersionUtil.h>

class hkOstream;
struct hkVersioningExceptionsArray;
class hkVersionPatchManager;
class hkDataWorld;

/// Utility routine to help maintain version compatability
/// We use these routines to help ensure we don't break backward compatability
namespace hkVersionCheckingUtils
{
	struct VersionDifference
	{
		VersionDifference(const hkClass* oldClass, const hkClass* newClass) : m_oldClass(oldClass), m_newClass(newClass) {}
		bool operator==(const VersionDifference& other) { return (other.m_oldClass == m_oldClass) && (other.m_newClass == m_newClass); }
		
		const hkClass* m_oldClass;	// If null then newClass has been added
		const hkClass* m_newClass;	// If null then oldClass has been removed
	private:
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_SERIALIZE, VersionDifference );
	};

		/// Compares two null terminated sets of classes - usually these sets represent different versions
		/// Ignore all differences specified in the exceptions array
	void HK_CALL checkVersions( const hkClass* const * oldVersions, const hkClass* const * newVersions, hkArray<VersionDifference>& differences, const hkVersioningExceptionsArray* exceptions = HK_NULL );

		/// Write a human readable description of the difference between these 2 versions
		/// For this to be useful the classes should be different versions of the same hkClass
		/// If detailed is true then this reports member offset and class size changes too.
	void HK_CALL summarizeChanges(hkOstream& output, const hkClass& oldVersion, const hkClass& newVersion, bool detailed = false);

	enum Flags
	{
		NONE			= 0,
		VERBOSE			= 1,
		IGNORE_REMOVED	= 2
	};

		/// Verify that an update description is complete.
		/// For each class in oldClasses, verify that it is identical to the new version
		/// or that a version function exists to update it. A human readable report is
		/// written to the report stream.
	hkResult HK_CALL verifyUpdateDescription( hkOstream& report,
		const hkClassNameRegistry& oldClassReg,
		const hkClassNameRegistry& newClassReg,
		const hkVersionRegistry::UpdateDescription& updateDescriptionHead,
		Flags flags = NONE );

	hkResult HK_CALL verifyClassPatches(hkOstream& report, const hkDataWorld& world, const hkClassNameRegistry& classReg, const hkVersionPatchManager& patchManager, Flags flags = NONE);
}


#endif // HKSERIALZE_VERSION_CHECKING_UTILS_H

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
