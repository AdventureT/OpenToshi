/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_SKELETON_MAPPER_H
#define HK_SKELETON_MAPPER_H

#include <Common/Base/hkBase.h>

#include <Animation/Animation/Mapper/hkaSkeletonMapperData.h>

class hkaPose;

extern const hkClass hkaSkeletonMapperClass;

/// This run-time class converts a pose from one skeleton (A) to another (B). The poses can be specified by either hkaPose objects or arrays of transforms.
/// It uses mapping data (hkaSkeletonMapperData), which can be created using the hkaSkeletonMapperUtils utility class.
class hkaSkeletonMapper : public hkReferencedObject
{
	public:

		HK_DECLARE_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_RIG );
		HK_DECLARE_REFLECTION();

			/// Constructor - Takes mapping data as a parameter; this data can be created using the hkaSkeletonMapperUtils class.
		hkaSkeletonMapper( const hkaSkeletonMapperData& mapping );

			/// Destructor
		virtual ~hkaSkeletonMapper();

			/// Constraints source.
		enum ConstraintSource
		{
				/// No constraints
			NO_CONSTRAINTS,
				/// Reference pose
			REFERENCE_POSE,
				/// Current pose
			CURRENT_POSE
		};

			/// Maps a pose from an skeleton to another.
			/// \param poseAModelSpace      The poseA in model space. The number of elements must be equal to mapping.m_skeletonA->m_numBones
			/// \param originalPoseBLocalSpace
			///                             The poseB in local space. The number of elements must be equal to mapping.m_skeletonB->m_numBones.
			///								This pose is used as an additional input to calculate output bones which do
			///								not map simply to an input bone 
			///								like bones in the middle of a chain or unmapped bones.
			/// \param poseBModelSpaceInOut The output pose. The number of elements must be equal to mapping.m_skeletonB->m_numBones.
			///                             Typically for all mapped bones, this array is a simply output parameter. For unmapped bones
			///								(especially the root bone), the output may be untouched
			/// \param source				Chooses between no adjustment, using the reference pose or the current pose when adjusting bone lengths. Usually the current pose is the best choice.
		void HK_CALL mapPose(const hkQsTransform* poseAModelSpace, const hkQsTransform* originalPoseBLocalSpace, hkQsTransform* poseBModelSpaceInOut, ConstraintSource source ) const;


			/// Maps a pose from an skeleton to another, using hkaPose objects.
			/// \param poseAIn				The input pose. The skeleton of poseAIn should be the same skeleton as m_mapping.m_skeletonA
			/// \param poseBInOut			The output pose. This pose should be initialized, as the transform of some of the bones in B may be used.
			///								The skeleton of poseBInOut should be the same skeleton as in m_mapping.m_skeletonB		
			/// \param source				Chooses between no adjustment, using the reference pose or the current pose when adjusting bone lengths. Usually the current pose is the best choice.
		void HK_CALL mapPose(const hkaPose& poseAIn, hkaPose& poseBInOut, ConstraintSource source ) const;


	public:

			/// The mapping data
		class hkaSkeletonMapperData m_mapping;

	public:

		/// For DEBUG builds only - Checks the mapping data and generates HK_WARN errors for troublesome mappings; called by the constructor in DEBUG.
		static void HK_CALL checkMapping(const hkaSkeletonMapperData& mapping);	

	public:

		hkaSkeletonMapper( hkFinishLoadedObjectFlag f ) : hkReferencedObject(f), m_mapping(f) { }
};


#endif // HK_SKELETON_MAPPER_H

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
