/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKANIMATION_ANIMATION_HKANIMATIONBINDING_HKCLASS_H
#define HKANIMATION_ANIMATION_HKANIMATIONBINDING_HKCLASS_H

/// hkaAnimationBinding meta information
extern const class hkClass hkaAnimationBindingClass;

/// Describes how an animation is bound to a skeleton
/// This class is useful when we have partial animations which only animate a subset of the skeleton
/// or when we have have reordered the tracks to aid in LOD. All hkaAnimationControl objects
/// (added to a hkaAnimatedSkeleton) require a hkaAnimationBinding to map from tracks to bones/slots
/// for blending of multiple animations.
class hkaAnimationBinding
{
	public:
	
		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_DATA, hkaAnimationBinding);
		HK_DECLARE_REFLECTION();
		
		//
		// Members
		//
	public:

			/// Default constructor.
		hkaAnimationBinding();

			/// (Optional) The name of the skeleton from which the binding was constructed.
		const char* m_originalSkeletonName; 

			/// A handle to the animation bound to the skeleton.
		class hkaAnimation* m_animation;
		
			/// A mapping from animation track indices to bone indices.
		hkInt16* m_transformTrackToBoneIndices;
			/// Size of m_transformTrackToBoneIndices.
		hkInt32 m_numTransformTrackToBoneIndices;

			/// A mapping from animation float track indices to float slot indices.
		hkInt16* m_floatTrackToFloatSlotIndices;
			/// Size of m_numFloatTrackToFloatSlotIndices.
		hkInt32 m_numFloatTrackToFloatSlotIndices;

			/// A hint to indicate how this animation should be blended - 
			/// See hkaAnimatedSkeleton for a description of how this affects animation blending.
		enum BlendHint
		{
				/// Normal
			NORMAL = 0,
				/// Additive
			ADDITIVE = 1
		};

			/// The blend hint.
		hkEnum<BlendHint, hkInt8> m_blendHint; //+default(0/*hkaAnimationBinding::NORMAL*/)

			// Helper functions to find tracks given bones or slots
		hkInt16 findTrackIndexFromBoneIndex( hkInt16 boneIndex );
		hkInt16 findTrackIndexFromSlotIndex( hkInt16 slotIndex );
		static hkInt16 HK_CALL findTrackIndexFromBoneIndex( hkInt16 boneIndex, hkInt32 numTracks, hkInt16* trackToBoneIndices );

	public:

			// Constructor for initialisation of vtable fixup
		hkaAnimationBinding( hkFinishLoadedObjectFlag flag ) {}

};

#include <Animation/Animation/Animation/hkaAnimationBinding.inl>

#endif // HKANIMATION_ANIMATION_HKANIMATIONBINDING_HKCLASS_H

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
