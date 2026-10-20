/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKANIMATION_RIG_HKSKELETON_HKCLASS_H
#define HKANIMATION_RIG_HKSKELETON_HKCLASS_H

#include <Animation/Animation/Rig/hkaBone.h>

class hkLocalFrame;

/// hkaSkeleton meta information
extern const class hkClass hkaSkeletonClass;

/// The class that represents a combination of a character rig and arbitrary float data.
/// The bone/transform data is designed to represent a skeletal animation rig - bone(transform) names, hierachy and reference pose.
/// The float slot data is designed to store arbitrary floating point data.
/// Either bones or float slots may be empty (but not both). For example: <br>
/// A character rig might have only bones. <br>
/// A character rig with auxiliary animated data might have both bones and float slots. <br>
/// A morph target 'rig' might have only float slots. <br>
/// Note that m_numBones==m_numParentIndices==m_numReferencePose. This redundancy
/// is required in order to properly serialize m_bones, m_parentIndices and m_referencePose
/// as arrays. Check the Serialization documentation for details.
class hkaSkeleton
{
	public:

	HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_RIG, hkaSkeleton );
	HK_DECLARE_REFLECTION();

		//
		// Members
		//
	public:

			/// A constructor for null initialization
		inline hkaSkeleton();

			/// Get the local frame that is attached to a bone.  Returns HK_NULL if there isn't one.
		hkLocalFrame* getLocalFrameForBone( int boneIndex ) const;

	public:

			/// A user name to aid in identifying the skeleton
		const char* m_name;

			/// Parent relationship
		hkInt16* m_parentIndices;

			/// Number of parent indices, has to be identical to m_numBones
		hkInt32 m_numParentIndices;

			/// Bones for this skeleton
		hkaBone** m_bones;

			/// Number of bones in the skeleton
		hkInt32 m_numBones;

			/// The reference pose for this skeleton. This pose is stored in local space.
		hkQsTransform* m_referencePose;

			/// Number of elements in the reference pose array
		hkInt32 m_numReferencePose;

			/// Floating point track slots. Often used for auxiliary float data or morph target parameters etc.
			/// This defines the target when binding animations to a particular rig.
		char**	m_floatSlots;

			/// Number of floating point slots in the skeleton
		hkInt32 m_numFloatSlots;

			/// A local frame and the index of the bone it is attached to.
		struct LocalFrameOnBone
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_RIG, LocalFrameOnBone );
			HK_DECLARE_REFLECTION();

				/// The local frame attached to a bone.
			hkLocalFrame* m_localFrame;

				/// The index of the bone that the frame is attached to.
			int m_boneIndex;
		};

			/// The local frames that are attached to the bones of this skeleton.  These must be ordered
			/// in order of increasing bone index.
		struct LocalFrameOnBone* m_localFrames;

			/// The number of local frames attached to the bones of this skeleton.
		hkInt32 m_numLocalFrames;

	public:

			// Constructor for initialisation of vtable fixup
		HK_FORCE_INLINE hkaSkeleton( hkFinishLoadedObjectFlag flag ) {}
};

#include <Animation/Animation/Rig/hkaSkeleton.inl>

#endif // HKANIMATION_RIG_HKSKELETON_HKCLASS_H

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
