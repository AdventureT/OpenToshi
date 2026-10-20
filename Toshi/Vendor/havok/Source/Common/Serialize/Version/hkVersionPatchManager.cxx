/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_PATCHES_FILE
#	error You must define HK_PATCHES_FILE before including this file
#endif

#include <Common/Base/Config/hkConfigBranches.h>
{
#define HK_CLASS_ADDED -1
#define HK_CLASS_REMOVED -1

//
// Defaults
//
// - NONE
#define HK_DEFAULT_VALUES_NONE HK_NULL
#define HK_DEFAULT_STORAGE_NONE void*
#define HK_DEFAULT_ARRAY_NONE
#define HK_COUNT_OF_OLD_NONE 0
#define HK_COUNT_OF_NEW_NONE 0

// - SUPPORTED TYPES

#define HK_DEFAULT_STORAGE_TYPE_INT(X) hkInt64
#define HK_DEFAULT_STORAGE_TYPE_REAL(X) hkReal
#define HK_DEFAULT_STORAGE_TYPE_CSTRING(X) char*
#define HK_DEFAULT_STORAGE_hkVector4(X,Y,Z,W) hkReal
#define HK_DEFAULT_STORAGE_hkQuaternion(X,Y,Z,W) hkReal
#define HK_DEFAULT_STORAGE_hkMatrix3(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2) hkReal
#define HK_DEFAULT_STORAGE_hkRotation(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2) hkReal
#define HK_DEFAULT_STORAGE_hkQsTransform(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2) hkReal
#define HK_DEFAULT_STORAGE_hkMatrix4(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2,X3,Y3,Z3,W3) hkReal
#define HK_DEFAULT_STORAGE_hkTransform(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2,X3,Y3,Z3,W3) hkReal

#define HK_DEFAULT_ARRAY_TYPE_INT(X) []
#define HK_DEFAULT_ARRAY_TYPE_REAL(X) []
#define HK_DEFAULT_ARRAY_TYPE_CSTRING(X) []
#define HK_DEFAULT_ARRAY_hkVector4(X,Y,Z,W) []
#define HK_DEFAULT_ARRAY_hkQuaternion(X,Y,Z,W) []
#define HK_DEFAULT_ARRAY_hkMatrix3(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2) []
#define HK_DEFAULT_ARRAY_hkRotation(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2) []
#define HK_DEFAULT_ARRAY_hkQsTransform(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2) []
#define HK_DEFAULT_ARRAY_hkMatrix4(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2,X3,Y3,Z3,W3) []
#define HK_DEFAULT_ARRAY_hkTransform(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2,X3,Y3,Z3,W3) []

#define HK_DEFAULT_VALUES_TYPE_INT(X) {X}
#define HK_DEFAULT_VALUES_TYPE_REAL(X) {X}
#define HK_DEFAULT_VALUES_TYPE_CSTRING(X) {X}
#define HK_DEFAULT_VALUES_hkVector4(X,Y,Z,W) {X,Y,Z,W}
#define HK_DEFAULT_VALUES_hkQuaternion(X,Y,Z,W) {X,Y,Z,W}
#define HK_DEFAULT_VALUES_hkMatrix3(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2) {X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2}
#define HK_DEFAULT_VALUES_hkRotation(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2) {X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2}
#define HK_DEFAULT_VALUES_hkQsTransform(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2) {X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2}
#define HK_DEFAULT_VALUES_hkMatrix4(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2,X3,Y3,Z3,W3) {X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2,X3,Y3,Z3,W3}
#define HK_DEFAULT_VALUES_hkTransform(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2,X3,Y3,Z3,W3) {X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2,X3,Y3,Z3,W3}

#define HK_COUNT_OF_OLD_TYPE_INT(X) HK_COUNT_OF(HK_PREPROCESSOR_JOIN_TOKEN(oldDefaults,__LINE__))
#define HK_COUNT_OF_OLD_TYPE_REAL(X) HK_COUNT_OF(HK_PREPROCESSOR_JOIN_TOKEN(oldDefaults,__LINE__))
#define HK_COUNT_OF_OLD_TYPE_CSTRING(X) HK_COUNT_OF(HK_PREPROCESSOR_JOIN_TOKEN(oldDefaults,__LINE__))
#define HK_COUNT_OF_OLD_hkVector4(X,Y,Z,W) HK_COUNT_OF(HK_PREPROCESSOR_JOIN_TOKEN(oldDefaults,__LINE__))
#define HK_COUNT_OF_OLD_hkQuaternion(X,Y,Z,W) HK_COUNT_OF(HK_PREPROCESSOR_JOIN_TOKEN(oldDefaults,__LINE__))
#define HK_COUNT_OF_OLD_hkMatrix3(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2) HK_COUNT_OF(HK_PREPROCESSOR_JOIN_TOKEN(oldDefaults,__LINE__))
#define HK_COUNT_OF_OLD_hkRotation(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2) HK_COUNT_OF(HK_PREPROCESSOR_JOIN_TOKEN(oldDefaults,__LINE__))
#define HK_COUNT_OF_OLD_hkQsTransform(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2) HK_COUNT_OF(HK_PREPROCESSOR_JOIN_TOKEN(oldDefaults,__LINE__))
#define HK_COUNT_OF_OLD_hkMatrix4(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2,X3,Y3,Z3,W3) HK_COUNT_OF(HK_PREPROCESSOR_JOIN_TOKEN(oldDefaults,__LINE__))
#define HK_COUNT_OF_OLD_hkTransform(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2,X3,Y3,Z3,W3) HK_COUNT_OF(HK_PREPROCESSOR_JOIN_TOKEN(oldDefaults,__LINE__))

#define HK_COUNT_OF_NEW_TYPE_INT(X) HK_COUNT_OF(HK_PREPROCESSOR_JOIN_TOKEN(newDefaults,__LINE__))
#define HK_COUNT_OF_NEW_TYPE_REAL(X) HK_COUNT_OF(HK_PREPROCESSOR_JOIN_TOKEN(newDefaults,__LINE__))
#define HK_COUNT_OF_NEW_TYPE_CSTRING(X) HK_COUNT_OF(HK_PREPROCESSOR_JOIN_TOKEN(newDefaults,__LINE__))
#define HK_COUNT_OF_NEW_hkVector4(X,Y,Z,W) HK_COUNT_OF(HK_PREPROCESSOR_JOIN_TOKEN(newDefaults,__LINE__))
#define HK_COUNT_OF_NEW_hkQuaternion(X,Y,Z,W) HK_COUNT_OF(HK_PREPROCESSOR_JOIN_TOKEN(newDefaults,__LINE__))
#define HK_COUNT_OF_NEW_hkMatrix3(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2) HK_COUNT_OF(HK_PREPROCESSOR_JOIN_TOKEN(newDefaults,__LINE__))
#define HK_COUNT_OF_NEW_hkRotation(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2) HK_COUNT_OF(HK_PREPROCESSOR_JOIN_TOKEN(newDefaults,__LINE__))
#define HK_COUNT_OF_NEW_hkQsTransform(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2) HK_COUNT_OF(HK_PREPROCESSOR_JOIN_TOKEN(newDefaults,__LINE__))
#define HK_COUNT_OF_NEW_hkMatrix4(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2,X3,Y3,Z3,W3) HK_COUNT_OF(HK_PREPROCESSOR_JOIN_TOKEN(newDefaults,__LINE__))
#define HK_COUNT_OF_NEW_hkTransform(X0,Y0,Z0,W0,X1,Y1,Z1,W1,X2,Y2,Z2,W2,X3,Y3,Z3,W3) HK_COUNT_OF(HK_PREPROCESSOR_JOIN_TOKEN(newDefaults,__LINE__))

//
// Patches
//

#define HK_PATCH_BEGIN(CLASSNAME, OLDVER, NEWNAME, NEWVER)
#define		HK_PATCH_GROUP(CNAME, VERSION) static const hkVersionPatchManager::GroupPatch HK_PREPROCESSOR_JOIN_TOKEN(p,__LINE__) = {CNAME, VERSION};
#define		HK_PATCH_CAST(CNAME) static const hkVersionPatchManager::CastPatch HK_PREPROCESSOR_JOIN_TOKEN(p,__LINE__) = {CNAME};
#define		HK_PATCH_DEPENDS(CNAME, VERSION) static const hkVersionPatchManager::DependsPatch HK_PREPROCESSOR_JOIN_TOKEN(p,__LINE__) = {CNAME, VERSION};
#define		HK_PATCH_PARENT_SET(OLDNAME, NEWNAME) static const hkVersionPatchManager::SetParentPatch HK_PREPROCESSOR_JOIN_TOKEN(p,__LINE__) = {OLDNAME, NEWNAME};
#define		HK_PATCH_ENUM_ADDED(NAME) static const hkVersionPatchManager::EnumAddedPatch HK_PREPROCESSOR_JOIN_TOKEN(p,__LINE__) = {NAME};
#define		HK_PATCH_ENUM_REMOVED(NAME) static const hkVersionPatchManager::EnumRemovedPatch HK_PREPROCESSOR_JOIN_TOKEN(p,__LINE__) = {NAME};
#define		HK_PATCH_ENUM_RENAMED(OLD, NEW) static const hkVersionPatchManager::EnumRenamedPatch HK_PREPROCESSOR_JOIN_TOKEN(p,__LINE__) = {OLD, NEW};
#define		HK_PATCH_ENUM_VALUE_ADDED(NAME, VNAME, VAL) static const hkVersionPatchManager::EnumValueAddedPatch HK_PREPROCESSOR_JOIN_TOKEN(p,__LINE__) = {NAME, VNAME, VAL};
#define		HK_PATCH_ENUM_VALUE_REMOVED(NAME, VNAME, VAL) static const hkVersionPatchManager::EnumValueRemovedPatch HK_PREPROCESSOR_JOIN_TOKEN(p,__LINE__) = {NAME, VNAME, VAL};
#define		HK_PATCH_ENUM_VALUE_CHANGED(NAME, VNAME, OLD, NEW) static const hkVersionPatchManager::EnumValueChangedPatch HK_PREPROCESSOR_JOIN_TOKEN(p,__LINE__) = {NAME, VNAME, OLD, NEW};
#define		HK_PATCH_MEMBER_ADDED(NAME, TYPE, TYPE_NAME, TUPLES) static const hkVersionPatchManager::MemberAddedPatch HK_PREPROCESSOR_JOIN_TOKEN(p,__LINE__) = {NAME,hkDataObject::TYPE,TYPE_NAME,TUPLES};
#define		HK_PATCH_MEMBER_RENAMED(OLDNAME,NEWNAME) static const hkVersionPatchManager::MemberRenamedPatch HK_PREPROCESSOR_JOIN_TOKEN(p,__LINE__) = {OLDNAME,NEWNAME};
#define		HK_PATCH_MEMBER_DEFAULT(NAME,OLD,NEW) \
	static const HK_DEFAULT_STORAGE_##OLD HK_PREPROCESSOR_JOIN_TOKEN(oldDefaults,__LINE__) HK_DEFAULT_ARRAY_##OLD = HK_DEFAULT_VALUES_##OLD;\
	static const HK_DEFAULT_STORAGE_##NEW HK_PREPROCESSOR_JOIN_TOKEN(newDefaults,__LINE__) HK_DEFAULT_ARRAY_##NEW = HK_DEFAULT_VALUES_##NEW;\
	static const hkVersionPatchManager::MemberDefaultPatch HK_PREPROCESSOR_JOIN_TOKEN(p,__LINE__) = {NAME,HK_PREPROCESSOR_JOIN_TOKEN(oldDefaults,__LINE__),HK_COUNT_OF_OLD_##OLD,HK_PREPROCESSOR_JOIN_TOKEN(newDefaults,__LINE__),HK_COUNT_OF_NEW_##NEW};
#define		HK_PATCH_FUNCTION(NAME) static const hkVersionPatchManager::FunctionPatch HK_PREPROCESSOR_JOIN_TOKEN(p,__LINE__) = { #NAME, NAME };
#define		HK_PATCH_MEMBER_REMOVED(NAME, TYPE, TYPE_NAME, TUPLES) static const hkVersionPatchManager::MemberRemovedPatch HK_PREPROCESSOR_JOIN_TOKEN(p,__LINE__) = {NAME,hkDataObject::TYPE,TYPE_NAME,TUPLES};
#define HK_PATCH_END()
#		include HK_PATCHES_FILE
#undef	HK_PATCH_BEGIN
#undef		HK_PATCH_GROUP
#undef		HK_PATCH_CAST
#undef		HK_PATCH_DEPENDS
#undef		HK_PATCH_PARENT_SET
#undef		HK_PATCH_ENUM_ADDED
#undef		HK_PATCH_ENUM_REMOVED
#undef		HK_PATCH_ENUM_RENAMED
#undef		HK_PATCH_ENUM_VALUE_ADDED
#undef		HK_PATCH_ENUM_VALUE_REMOVED
#undef		HK_PATCH_ENUM_VALUE_CHANGED
#undef		HK_PATCH_MEMBER_ADDED
#undef		HK_PATCH_MEMBER_RENAMED
#undef		HK_PATCH_MEMBER_DEFAULT
#undef		HK_PATCH_FUNCTION
#undef		HK_PATCH_MEMBER_REMOVED
#undef	HK_PATCH_END

#define	HK_PATCH_BEGIN(CLASSNAME, OLDVER, NEWCLASSNAME, NEWVER) { const char* classname = CLASSNAME; const char* newclassname = NEWCLASSNAME; const int oldver = OLDVER; const int newver = NEWVER; static const hkVersionPatchManager::PatchInfo::Component patch[] = {
#define		HK_PATCH_GROUP(CNAME, VERSION) {hkVersionPatchManager::PATCH_GROUP, HK_PREPROCESSOR_JOIN_TOKEN(&p,__LINE__)},
#define		HK_PATCH_CAST(CNAME) {hkVersionPatchManager::PATCH_CAST, HK_PREPROCESSOR_JOIN_TOKEN(&p,__LINE__)},
#define		HK_PATCH_DEPENDS(CNAME, VERSION) {hkVersionPatchManager::PATCH_DEPENDS, HK_PREPROCESSOR_JOIN_TOKEN(&p,__LINE__)},
#define		HK_PATCH_PARENT_SET(OLDNAME, NEWNAME) {hkVersionPatchManager::PATCH_PARENT_SET, HK_PREPROCESSOR_JOIN_TOKEN(&p,__LINE__)},
#define		HK_PATCH_ENUM_ADDED(NAME) {hkVersionPatchManager::PATCH_ENUM_ADDED, HK_PREPROCESSOR_JOIN_TOKEN(&p,__LINE__)},
#define		HK_PATCH_ENUM_REMOVED(NAME) {hkVersionPatchManager::PATCH_ENUM_REMOVED, HK_PREPROCESSOR_JOIN_TOKEN(&p,__LINE__)},
#define		HK_PATCH_ENUM_RENAMED(OLD, NEW) {hkVersionPatchManager::PATCH_ENUM_RENAMED, HK_PREPROCESSOR_JOIN_TOKEN(&p,__LINE__)},
#define		HK_PATCH_ENUM_VALUE_ADDED(NAME, VNAME, VAL) {hkVersionPatchManager::PATCH_ENUM_VALUE_ADDED, HK_PREPROCESSOR_JOIN_TOKEN(&p,__LINE__)},
#define		HK_PATCH_ENUM_VALUE_REMOVED(NAME, VNAME, VAL) {hkVersionPatchManager::PATCH_ENUM_VALUE_RENAMED, HK_PREPROCESSOR_JOIN_TOKEN(&p,__LINE__)},
#define		HK_PATCH_ENUM_VALUE_CHANGED(NAME, VNAME, OLD, NEW) {hkVersionPatchManager::PATCH_ENUM_VALUE_CHANGED, HK_PREPROCESSOR_JOIN_TOKEN(&p,__LINE__)},
#define		HK_PATCH_MEMBER_ADDED(NAME, TYPE, TYPE_NAME, TUPLES) {hkVersionPatchManager::PATCH_MEMBER_ADDED, HK_PREPROCESSOR_JOIN_TOKEN(&p,__LINE__)},
#define		HK_PATCH_MEMBER_RENAMED(OLDNAME,NEWNAME) {hkVersionPatchManager::PATCH_MEMBER_RENAMED, HK_PREPROCESSOR_JOIN_TOKEN(&p,__LINE__)},
#define		HK_PATCH_MEMBER_DEFAULT(NAME,OLD,NEW) {hkVersionPatchManager::PATCH_MEMBER_DEFAULT, HK_PREPROCESSOR_JOIN_TOKEN(&p,__LINE__)},
#define		HK_PATCH_FUNCTION(NAME) {hkVersionPatchManager::PATCH_FUNCTION, HK_PREPROCESSOR_JOIN_TOKEN(&p,__LINE__)},
#define		HK_PATCH_MEMBER_REMOVED(NAME, TYPE, TYPE_NAME, TUPLES) {hkVersionPatchManager::PATCH_MEMBER_REMOVED, HK_PREPROCESSOR_JOIN_TOKEN(&p,__LINE__)},
#define	HK_PATCH_END() {hkVersionPatchManager::PATCH_INVALID, HK_NULL} }; static const hkVersionPatchManager::PatchInfo pinfo = {classname, newclassname, oldver, newver, patch, HK_COUNT_OF(patch) - 1}; man.addPatch(&pinfo); }
#		include HK_PATCHES_FILE
#undef	HK_PATCH_BEGIN
#undef		HK_PATCH_GROUP
#undef		HK_PATCH_CAST
#undef		HK_PATCH_DEPENDS
#undef		HK_PATCH_PARENT_SET
#undef		HK_PATCH_ENUM_ADDED
#undef		HK_PATCH_ENUM_REMOVED
#undef		HK_PATCH_ENUM_RENAMED
#undef		HK_PATCH_ENUM_VALUE_ADDED
#undef		HK_PATCH_ENUM_VALUE_REMOVED
#undef		HK_PATCH_ENUM_VALUE_CHANGED
#undef		HK_PATCH_MEMBER_ADDED
#undef		HK_PATCH_MEMBER_RENAMED
#undef		HK_PATCH_MEMBER_DEFAULT
#undef		HK_PATCH_FUNCTION
#undef		HK_PATCH_MEMBER_REMOVED
#undef	HK_PATCH_END

#undef HK_CLASS_ADDED
#undef HK_CLASS_REMOVED

#undef HK_DEFAULT_VALUES_NONE
#undef HK_DEFAULT_STORAGE_NONE
#undef HK_DEFAULT_ARRAY_NONE
#undef HK_COUNT_OF_OLD_NONE
#undef HK_COUNT_OF_NEW_NONE

// - SUPPORTED TYPES
#undef HK_DEFAULT_ARRAY_TYPE_INT
#undef HK_DEFAULT_ARRAY_TYPE_REAL
#undef HK_DEFAULT_ARRAY_TYPE_CSTRING
#undef HK_DEFAULT_ARRAY_hkVector4
#undef HK_DEFAULT_ARRAY_hkQuaternion
#undef HK_DEFAULT_ARRAY_hkMatrix3
#undef HK_DEFAULT_ARRAY_hkRotation
#undef HK_DEFAULT_ARRAY_hkQsTransform
#undef HK_DEFAULT_ARRAY_hkMatrix4
#undef HK_DEFAULT_ARRAY_hkTransform

#undef HK_COUNT_OF_OLD_TYPE_INT
#undef HK_COUNT_OF_OLD_TYPE_REAL
#undef HK_COUNT_OF_OLD_TYPE_CSTRING
#undef HK_COUNT_OF_OLD_hkVector4
#undef HK_COUNT_OF_OLD_hkQuaternion
#undef HK_COUNT_OF_OLD_hkMatrix3
#undef HK_COUNT_OF_OLD_hkRotation
#undef HK_COUNT_OF_OLD_hkQsTransform
#undef HK_COUNT_OF_OLD_hkMatrix4
#undef HK_COUNT_OF_OLD_hkTransform

#undef HK_COUNT_OF_NEW_TYPE_INT
#undef HK_COUNT_OF_NEW_TYPE_REAL
#undef HK_COUNT_OF_NEW_TYPE_CSTRING
#undef HK_COUNT_OF_NEW_hkVector4
#undef HK_COUNT_OF_NEW_hkQuaternion
#undef HK_COUNT_OF_NEW_hkMatrix3
#undef HK_COUNT_OF_NEW_hkRotation
#undef HK_COUNT_OF_NEW_hkQsTransform
#undef HK_COUNT_OF_NEW_hkMatrix4
#undef HK_COUNT_OF_NEW_hkTransform

#undef HK_DEFAULT_STORAGE_TYPE_INT
#undef HK_DEFAULT_STORAGE_TYPE_REAL
#undef HK_DEFAULT_STORAGE_TYPE_CSTRING
#undef HK_DEFAULT_STORAGE_hkVector4
#undef HK_DEFAULT_STORAGE_hkQuaternion
#undef HK_DEFAULT_STORAGE_hkMatrix3
#undef HK_DEFAULT_STORAGE_hkRotation
#undef HK_DEFAULT_STORAGE_hkQsTransform
#undef HK_DEFAULT_STORAGE_hkMatrix4
#undef HK_DEFAULT_STORAGE_hkTransform

#undef HK_DEFAULT_VALUES_TYPE_INT
#undef HK_DEFAULT_VALUES_TYPE_REAL
#undef HK_DEFAULT_VALUES_TYPE_CSTRING
#undef HK_DEFAULT_VALUES_hkVector4
#undef HK_DEFAULT_VALUES_hkQuaternion
#undef HK_DEFAULT_VALUES_hkMatrix3
#undef HK_DEFAULT_VALUES_hkRotation
#undef HK_DEFAULT_VALUES_hkQsTransform
#undef HK_DEFAULT_VALUES_hkMatrix4
#undef HK_DEFAULT_VALUES_hkTransform
}

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
