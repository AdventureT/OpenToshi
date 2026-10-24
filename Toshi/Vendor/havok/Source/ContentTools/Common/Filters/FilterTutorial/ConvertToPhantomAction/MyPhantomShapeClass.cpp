/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

// WARNING: THIS FILE IS GENERATED. EDITS WILL BE LOST.
// Generated from 'ContentTools/Common/Filters/FilterTutorial/ConvertToPhantomAction/MyPhantomShape.h'
#include <ContentTools/Common/Filters/FilterTutorial/hctFilterTutorial.h>
#include <Common/Base/Reflection/hkClass.h>
#include <Common/Base/Reflection/hkInternalClassMember.h>
#include <Common/Base/Reflection/hkTypeInfo.h>
#include <ContentTools/Common/Filters/FilterTutorial/ConvertToPhantomAction/MyPhantomShape.h>



//
// Enum MyPhantomShape::ActionType
//
static const hkInternalClassEnumItem MyPhantomShapeActionTypeEnumItems[] =
{
	{0, "ACTION_WIND"},
	{1, "ACTION_ATTRACT"},
	{2, "ACTION_DEFLECT"},
};
static const hkInternalClassEnum MyPhantomShapeEnums[] = {
	{"ActionType", MyPhantomShapeActionTypeEnumItems, 3, HK_NULL, 0 }
};
extern const hkClassEnum* MyPhantomShapeActionTypeEnum = reinterpret_cast<const hkClassEnum*>(&MyPhantomShapeEnums[0]);

//
// Class MyPhantomShape
//
static const hkInternalClassMember MyPhantomShapeClass_Members[] =
{
	{ "actionType", HK_NULL, MyPhantomShapeActionTypeEnum, hkClassMember::TYPE_ENUM, hkClassMember::TYPE_INT8, 0, 0, HK_OFFSET_OF(MyPhantomShape,m_actionType), HK_NULL },
	{ "direction", HK_NULL, HK_NULL, hkClassMember::TYPE_VECTOR4, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(MyPhantomShape,m_direction), HK_NULL },
	{ "strength", HK_NULL, HK_NULL, hkClassMember::TYPE_REAL, hkClassMember::TYPE_VOID, 0, 0, HK_OFFSET_OF(MyPhantomShape,m_strength), HK_NULL }
};
extern const hkClass hkpPhantomCallbackShapeClass;

extern const hkClass MyPhantomShapeClass;
const hkClass MyPhantomShapeClass(
	"MyPhantomShape",
	&hkpPhantomCallbackShapeClass, // parent
	sizeof(MyPhantomShape),
	HK_NULL,
	0, // interfaces
	reinterpret_cast<const hkClassEnum*>(MyPhantomShapeEnums),
	1, // enums
	reinterpret_cast<const hkClassMember*>(MyPhantomShapeClass_Members),
	HK_COUNT_OF(MyPhantomShapeClass_Members),
	HK_NULL, // defaults
	HK_NULL, // attributes
	0, // flags
	0 // version
	);
HK_REFLECTION_DEFINE_VIRTUAL(MyPhantomShape);

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
