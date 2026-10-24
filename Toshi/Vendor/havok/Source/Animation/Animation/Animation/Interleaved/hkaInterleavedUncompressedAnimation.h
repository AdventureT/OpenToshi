/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKANIMATION_ANIMATION_INTERLEAVED_HKINTERLEAVEDANIMATION_HKCLASS_H
#define HKANIMATION_ANIMATION_INTERLEAVED_HKINTERLEAVEDANIMATION_HKCLASS_H

#include <Animation/Animation/Animation/hkaAnimation.h>

/// hkaInterleavedUncompressedAnimation meta information
extern const class hkClass hkaInterleavedUncompressedAnimationClass;

/// The information needed to construct an hkSimpleAnimation (An interleaved
/// uncompressed stream of Bone Transforms)
class hkaInterleavedUncompressedAnimation : public hkaAnimation
{
	public:
	
		HK_DECLARE_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_UNCOMPRESSED );
		HK_DECLARE_REFLECTION();

			/// Default constructor
		inline hkaInterleavedUncompressedAnimation() :
			m_transforms( HK_NULL ), m_numTransforms( 0 ),
			m_floats( HK_NULL ), m_numFloats( 0 )
		{
			m_type = hkaAnimation::HK_INTERLEAVED_ANIMATION;
		}
		
			/// Get the pose at a given time
		virtual void sampleTracks(hkReal time, hkQsTransform* transformTracksOut, hkReal* floatTracksOut, hkaChunkCache* cache) const;
		
			/// Get a subset of the the first 'maxNumTracks' tracks of a pose at a given time (all tracks from 0 to maxNumTracks-1 inclusive).
		virtual void samplePartialTracks(hkReal time,
										 hkUint32 maxNumTransformTracks, hkQsTransform* transformTracksOut,
										 hkUint32 maxNumFloatTracks, hkReal* floatTracksOut,
										 hkaChunkCache* cache) const;

			/// Sample individual animation tracks
		virtual void sampleIndividualTransformTracks( hkReal time, const hkInt16* tracks, hkUint32 numTracks, hkQsTransform* transformOut ) const;

			/// Sample a individual floating tracks
		virtual void sampleIndividualFloatTracks( hkReal time, const hkInt16* tracks, hkUint32 numTracks, hkReal* out ) const;

	
			/// Transform all samples of a given track by premultiplication with a (bone) transform 
		void transformTrack(int track, const hkQsTransform& transform);
		
			/// Returns the number of original samples / frames of animation
		virtual int getNumOriginalFrames() const;

			/// Return the number of chunks of data required to sample a pose at time t
		virtual int getNumDataChunks(hkReal time) const;

			/// Return the chunks of data required to sample the tracks at time t
		virtual void getDataChunks(hkReal time, DataChunk* dataChunks, int numDataChunks) const;

			/// Return the maximum total size of all combined chunk data which could be returned by getDataChunks fro this animation.
		virtual int getMaxSizeOfCombinedDataChunks() const;

			/// Get a subset of the tracks at a given time using data chunks. Sample is calculated using pose[frameIndex] * (1 - frameDelta) + pose[frameIndex+1] * frameDelta.
		static void HK_CALL samplePartialWithDataChunks(hkUint32 frameIndex, hkReal frameDelta, 
														hkUint32 maxNumTransformTracks, hkQsTransform* transformTracksOut,
														hkUint32 maxNumFloatTracks, hkReal* floatTracksOut,
														const DataChunk* dataChunks, int numDataChunks);

	public:
		
			/// Array of hkQsTransforms - see Userguide for interleaved format.
		hkQsTransform* m_transforms;
			/// Number of hkQsTransforms ( = #tracks * #frames)
		int			   m_numTransforms;

		hkReal*        m_floats;
		int            m_numFloats;
		
	public:
	
			// Constructor for initialisation of vtable fixup
		HK_FORCE_INLINE hkaInterleavedUncompressedAnimation( hkFinishLoadedObjectFlag flag ) : hkaAnimation(flag) {}
};

#endif // HKANIMATION_ANIMATION_INTERLEAVED_HKINTERLEAVEDANIMATION_HKCLASS_H

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
