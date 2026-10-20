/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_SKELETON_MAPPER_UTILS_H
#define HK_SKELETON_MAPPER_UTILS_H

class hkaSkeleton;

#include <Animation/Animation/Rig/hkaSkeletonUtils.h>
#include <Animation/Animation/Mapper/hkaSkeletonMapper.h>
#include <Animation/Animation/Mapper/hkaSkeletonMapperData.h>
#include <Common/Base/Container/Array/hkObjectArray.h>

/// This utility class matches two skeletons (A and B) using their reference pose in world and the (filtered) names of their bones
/// and, through the "createMapping" static method, creates the mapping data that can be used at run-time (by an hkaSkeletonMapper) to
/// map poses from A into B, and vice-versa.
/// One-to-one mappings and chains are autodetected, but can also be specified by the user
class hkaSkeletonMapperUtils
{
	public:

			/// A chain, specified by the names of the start and end bones - used in Params::m_userChains.
			/// These names will be matched with the bone names (after filtering the names) of both skeletons.
		struct UserChain
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_RUNTIME, hkaSkeletonMapperUtils::UserChain );

				/// Start name.
			const char* m_start;
				/// End name.
			const char* m_end;
		};

			/// A explicit mapping (one-way only) - used in Params::m_userMappingsAtoB and Params::m_userMappingsBtoA.
			/// The names will be matched with the bones names (after filtering the names) of the skeleton (A or B).
		struct UserMapping
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_RUNTIME, hkaSkeletonMapperUtils::UserMapping );

				/// Bone in name.
			const char* m_boneIn;
				/// Bone out name.
			const char* m_boneOut;
		};

			/// These struct contains all the information used by "createMapping" in order to construct mapping data
		struct Params
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_RUNTIME, hkaSkeletonMapperUtils::Params );

				/// The first of the two skeletons involved
			const hkaSkeleton* m_skeletonA;
				/// The second of the two skeletons involved
			const hkaSkeleton* m_skeletonB;

				/// These filters are used to match names of bones from each skeleton
				/// It will use case insensive compare if NULL and not using positional tolerance
			hkStringCompareFunc m_compareNames;

				/// If no string compare function is given and this tolerance is non zero,
				/// positions are used to to match the bones in model space (with this tol)
			hkReal m_positionMatchTolerance;
			
				/// Users can specify specific chains they want to enforce
			hkArray<UserChain> m_userChains;

				/// You can also force specific one-to-one mappings between bones.
				/// For example, you can map a "twist forearm" in A to a "forearm" in B.
			hkArray<UserMapping> m_userMappingsAtoB;

				/// You can also force specific one-to-one mappings between bones.
				/// For example, you can map a "twist forearm" in B to a "forearm" in A.
			hkArray<UserMapping> m_userMappingsBtoA;

				/// If true, the system will auto detect simple mappings
				/// This is normally desired unless you have completely specified
				/// your mapping using user maps.
			hkBool m_autodetectSimple; 

				/// If true, the system will try to autodetect chains by looking at unmapped bones
				/// located in between mapped bones. The chain has to be of length >=3 in at least one of the skeletons.
			hkBool m_autodetectChains;	
			
				/// Copy constructor
			Params ( const Params& other) : 
				m_skeletonA(other.m_skeletonA), m_skeletonB(other.m_skeletonB), 
				m_compareNames(other.m_compareNames), 
				m_positionMatchTolerance (other.m_positionMatchTolerance),
				m_autodetectSimple (other.m_autodetectSimple), m_autodetectChains (other.m_autodetectChains)
				{
					m_userChains = other.m_userChains;
					m_userMappingsAtoB = other.m_userMappingsAtoB;
					m_userMappingsBtoA = other.m_userMappingsBtoA;
				}

			Params () :
				m_skeletonA(HK_NULL), m_skeletonB(HK_NULL), m_compareNames(HK_NULL), 
				m_positionMatchTolerance(0.0f), m_autodetectSimple(true), m_autodetectChains(true)
			{}
		};


			/// Creates the mapping data (used by hkaSkeletonMapper) used to map poses from skeleton A to skeleton B
			/// and vice-versa.
		static void HK_CALL createMapping (const Params& params, hkaSkeletonMapperData& aToB, hkaSkeletonMapperData& bToA);

			/// Searches and returns a pointer to the simple mapping associated with the given bone (of skeleton A) inside
			/// the given mapping data.
		static const struct hkaSkeletonMapperData::SimpleMapping* HK_CALL findSimpleMapping(hkInt16 boneA, const hkaSkeletonMapperData& mapperdata );

			/// Given mapping data between two skeletons, it automatically locks translations on the appropiate bones of both skeletons.
			/// The bones at the top-most simple mapping (and their ancestors) are left unlocked - the rest of bones are locked.  
		static hkResult HK_CALL lockTranslationsAutomatically (const hkaSkeletonMapperData& mapperData);

			/// Given a skeleton mapping, scales the translations of all transforms.
			/// Useful when scaling ragdolls or animations
		static void HK_CALL scaleMapping( hkaSkeletonMapper& mapper, hkReal scale );
};



#endif // HK_SKELETON_MAPPER_UTILS_H

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
