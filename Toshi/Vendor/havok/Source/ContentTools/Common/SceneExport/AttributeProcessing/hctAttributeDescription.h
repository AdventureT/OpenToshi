/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKSCENEEXPORT_ATTRIBUTE_DESCRIPTION__H
#define HKSCENEEXPORT_ATTRIBUTE_DESCRIPTION__H

extern const hkClass hctAttributeDescriptionClass;
extern const hkClass hctAttributeGroupDescriptionClass;
extern const hkClass hctAttributeDescriptionDatabaseClass;
extern const hkTypeInfo hctAttributeDescriptionTypeInfo;
extern const hkTypeInfo hctAttributeGroupDescriptionTypeInfo;
extern const hkTypeInfo hctAttributeDescriptionDatabaseTypeInfo;

	/// An attribute description object contains information about an attribute that can be used by
	/// an exporter to complement the data given by the modeler regarding that attribute. This is useful
	/// in order to support attribute types and hints not natively supported by the modeler.
struct hctAttributeDescription
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_EXPORT, hctAttributeDescription );
	HK_DECLARE_REFLECTION();

		/// Name of the attribute we are describing
	const char* m_name;

		/// If a boolean attribute with the given name is not set to true, then this attribute is ignored.
		/// Note : From 4.0, attributes are usually automatically enabled/disabled if another bool/int attribute named
		/// "changeXXX" (where XXX is the name of this attribute) is found. This is done in hctAttributeProcessingUtil::processAttributeGroup().
	const char* m_enabledBy;

		/// Sometimes the modeler may use a type that doesn't match the expected final type.
		/// We can force a conversion by specifying one of these enums in m_forceType.
	enum ForcedType
	{
		LEAVE,
		FORCE_BOOL,
		FORCE_INT,
		FORCE_ENUM,
		FORCE_FLOAT,
		FORCE_STRING,
		FORCE_VECTOR,
		FORCE_MATRIX,
		FORCE_QUATERNION,
	};

		/// (Possibly) force this attribute to be of the given type.
	hkEnum<ForcedType,hkInt8> m_forcedType;

		/// For attributes with FORCE_ENUM, this describes the enum used for the given attribute.
	class hkClassEnum* m_enum;

		// Note : There should be a one to one mapping between this enum and the hkxattrib enum (serialization
		// doesn't like referencing it from here)

		/// This hint specifies extra information for the attribute; in particular, for numeric (float, vector) attributes
		/// it describes how it should be transformed if the scene is transformed.
	enum Hint 
	{
			/// No hint
		HINT_NONE = 0, 
			/// This attribute should be ignored
		HINT_IGNORE = 1, 
			/// This attribute should be fully transformed (rotated) but not scaled: it is a direction
		HINT_TRANSFORM = 2, 
			/// This attribute should be scaled : it is a distance
		HINT_SCALE = 4, 
			/// This attribute should be fully fully transformed and scaled : it is a position
		HINT_TRANSFORM_AND_SCALE= 6, 
			/// This attribute should be flipped upon handness flip : it is an angle
		HINT_FLIP = 8 
	};

		/// Want to know if it has a special meaning or usage
	hkEnum<Hint,hkInt8> m_hint;

		/// If true, the above hint(s) will override any previous hints - otherwise they will be OR-ed
	hkBool m_clearHints;

		/// Float attributes will be scaled by this value. It used to transform degrees into radians (in XSI)
	hkReal m_floatScale; 

		/// Constructor, sets defaults.
	hctAttributeDescription();

};

	/// hctAttributeDescription objects are grouped in hctAttributeGroupDescription objects, 
	/// just as hkxAttribute objects are grouped in hkxAttributeGroup objects.
struct hctAttributeGroupDescription
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_EXPORT, hctAttributeGroupDescription );
	HK_DECLARE_REFLECTION();

		/// Finds an attribute description by name in this attribute group description.
	hctAttributeDescription* findAttributeDescriptionByName (const char* name) const;

		/// The name of the attribute group description (should match the name of the hkxAttributeGroup it describes).
		/// For example: "hkRigidBody"
	const char* m_name;

		/// The array of hctAttributeDescription objects in this group.
	struct hctAttributeDescription* m_attributeDescriptions;

		/// The number of hctAttributeDescription objects in this group.
	int m_numAttributeDescriptions;
};

	/// An hctAttributeDescriptionDatabase object stores descriptions for all attributes and attribute groups.
	/// It contains an array of hctAttributeGroupDescription objects.
struct hctAttributeDescriptionDatabase
{
	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_EXPORT, hctAttributeDescriptionDatabase );
	HK_DECLARE_REFLECTION();

		/// Searches for a description for a group by name.
		/// If found (not NULL) then use hctAttributeGroupDescription::findAttributeDescriptionByName
		/// to look for specific attribute data.
	struct hctAttributeGroupDescription* findAttributeDescriptionGroupByName (const char* name) const;

		/// The array of hctAttributeGroupDescription objects in the database.
	struct hctAttributeGroupDescription* m_groupDescriptions;

		/// The number of hctAttributeGroupDescription objects in the database.
	int m_numGroupDescriptions;
};


#endif // HKSCENEEXPORT_ATTRIBUTE_DESCRIPTION__H

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
