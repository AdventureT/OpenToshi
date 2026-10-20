/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKANIMATION_RIG_HKBONEATTACHMENT_HKCLASS_H
#define HKANIMATION_RIG_HKBONEATTACHMENT_HKCLASS_H

#include <Animation/Animation/Rig/hkaBone.h>
#include <Animation/Animation/Rig/hkaSkeleton.h>

/// hkaBoneAttachment meta information
extern const class hkClass hkaBoneAttachmentClass;

/// A link between a bone and a reflected object (mesh, light, camera, etc.).
class hkaBoneAttachment
{
	public:

	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_RIG, hkaBoneAttachment );
	HK_DECLARE_REFLECTION();
		
		//
		// Members
		//
	public:

		/// A constructor for null initialization 
		inline hkaBoneAttachment()
			:	m_originalSkeletonName(HK_NULL),
				m_name(HK_NULL),
				m_boneIndex(-1)
		{
		}

			/// (Optional) The name of the skeleton from which the binding was constructed
		const char* m_originalSkeletonName; 
		
			/// The transform from the local space of the mesh to the space of the attach point
		hkMatrix4 m_boneFromAttachment;

			/// The object attached. Check the variant class and object pointer to find out what it is.
		hkVariant m_attachment;

			/// A name that can be used to recognize the attachment
		const char* m_name;

			/// The bone in that skeleton.
		hkInt16 m_boneIndex;

	public:
		// Constructor for initialisation of vtable fixup
		HK_FORCE_INLINE hkaBoneAttachment( hkFinishLoadedObjectFlag flag ) {}
};

#endif // HKANIMATION_RIG_HKBONEATTACHMENT_HKCLASS_H

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
