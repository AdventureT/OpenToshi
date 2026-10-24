/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_DATA_COPIER_H
#define HK_DATA_COPIER_H

#include <Common/Serialize/Data/hkDataObject.h>

class hkResource;
class hkClassNameRegistry;

	/// Utilities for copying between hkDataObject and native types.
namespace hkDataObjectUtil
{
		/// Recursively copy the object into an hkResource.
	hkResource* HK_CALL toResource( const hkDataObject& src);

		/// Recursively copy the object into an hkResource using classes from the supplied
		/// 'classReg' registry. If class is not found then the object is not copied.
		/// Usually you should use hkDataObjectUtil::toResource() instead of this function.
	hkResource* toResourceWithRegistry(const hkDataObject& src, const hkClassNameRegistry* classReg);

		/// Convert hkClassMember::Type to hkDataObject::Type.
	hkDataObject::Type HK_CALL getTypeFromMemberType(hkClassMember::Type mtype, hkClassMember::Type stype, const hkClass* klass, int count);
}

#endif // HK_DATA_COPIER_H

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
