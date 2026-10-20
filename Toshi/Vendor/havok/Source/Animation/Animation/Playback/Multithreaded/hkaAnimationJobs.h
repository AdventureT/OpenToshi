/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_ANIMATION_JOBS_H
#define HK_ANIMATION_JOBS_H

#include <Common/Base/hkBase.h>

#include <Common/Base/Thread/Semaphore/hkSemaphoreBusyWait.h>
#include <Common/Base/Thread/JobQueue/hkJobQueue.h>
#include <Animation/Animation/Animation/hkaAnimationBinding.h>
#include <Animation/Animation/Animation/hkaAnimation.h>

class hkaAnimatedSkeleton;


/// The base class for all animation jobs

// CONSERVATIVE 70k max space for animation buffer. 
// Use either:
// hkaAnimation::getMaxSizeOfCombinedDataChunks() for sample-only jobs , or
// hkaMultithreadedAnimationUtils::getMaxSizeRequiredForSampleAndCombineJobBuffer() and
// getConservativeMaxSizeRequiredForSampleAndCombineJobBuffer() for sample-and-combine jobs
// to get a better estimate of how large this actually needs to be.
const int DEFAULT_MAX_COMPRESSED_ANIM_DATA = 70 * 1024 ; 

const int HK_MAX_NUM_DATA_CHUNKS = 10;

/// Base class for all 'animation' jobs.
class hkaAnimationJob : public hkJob
{
	public:

			/// Types enum.
		enum JobSubType
		{
				/// Sample only (no blending)
			ANIMATION_JOB_SAMPLE_ANIMATION,
				/// Sample and combine (blending)
			ANIMATION_JOB_SAMPLE_AND_COMBINE,
				//
			ANIMATION_JOB_END		// just a marker marking the end of the animation job id range
		};

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIMATION, hkaAnimationJob );

	protected:

		HK_FORCE_INLINE hkaAnimationJob( JobSubType type, hkUint16 size );

	public:

			/// The buffer allowable/available for decompressing data. You can use getMaxSizeOfCombinedDataChunks() to ensure this is large enough.
		hkUint32	m_bufferSize;      

};

/// A job which samples a set of animations - See Animation\Animation\Playback\Multithreaded\hkaAnimationJobs.h for details.
/// If more than one animation is specified then DMA cost is hidden through double buffering.
class hkaSampleAnimationJob : public hkaAnimationJob
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIMATION, hkaSampleAnimationJob);

		HK_FORCE_INLINE hkaSampleAnimationJob();

		HK_FORCE_INLINE ~hkaSampleAnimationJob() {}

		inline hkBool isValid() const;	// Only exists on PPU for error checking

			/// Helper structure for data needed to sample a single animation
		struct AnimationData
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIMATION, hkaSampleAnimationJob::AnimationData);

				/// Constructor.
			HK_FORCE_INLINE AnimationData();        // Only exists on PPU for error checking
			HK_FORCE_INLINE ~AnimationData() {}
				/// Init all members.
			HK_FORCE_INLINE void init();

			/// Test if all members are valid (alignment, size etc.).
			inline hkBool isValid() const; 	// Only exists on PPU for error checking
			/* Sample inputs */
				/// The frame number such that local time for the sample is between m_frameIndex and m_frameIndex+1.
			HK_ALIGN16(hkUint32				m_frameIndex);
				/// The delta between frames, in range [0,1). You can use getFrameAndDelta() to calculate this and m_frameIndex for a given time.
			hkReal							m_frameDelta;
				/// The highest transfrom track to sample to - used for LOD sampling.
			hkUint32						m_maxTransformTrack;
				/// The highest floattrack to sample to - used for LOD sampling.
			hkUint32						m_maxFloatTrack;

			/* Animation Data */
				/// Pointer to original animated skeleton - used only in shared memory architectures, otherwise ignored.
			const hkaAnimation* m_animationOnCPU;
				/// The type of animation data stored in the chunks
			hkaAnimation::AnimationType	m_animationType;  
				/// The static animation data required to sample at 'localTime', given by m_frameIndex and m_frameDelta.
			hkaAnimation::DataChunk		m_chunks[HK_MAX_NUM_DATA_CHUNKS]; 
				/// The number of chunks that contain valid data.
			int									m_numValidChunks;

			/* Output Data */
				/// The transfrom/bone results are placed in this buffer in main memory.
			hkQsTransform*					m_poseOut;
				/// The float results are placed in this buffer in main memory.
			hkReal*					m_floatSlotsOut;
		};

		/* Animation Data */
			/// The array of animations to sample.
		AnimationData*					m_animData;	
			/// The number of animations to sample.
		int								m_numAnims;	

			/// Optional cache - used only in shared memory architectures, otherwise ignored.
		hkaChunkCache*			m_cache;

			/// This semaphore is released when the work is finished.
			/// Can be set to HK_NULL if you don't need to wait on a specific job
		hkSemaphoreBusyWait*			m_jobDoneSemaphore;

			/// This flag is incremented when the work is finished.
			/// Can be set to HK_NULL if you don't need to wait on a specific job
		hkUint32*						m_jobDoneFlag;
};

/// A job which does all the decompression and blending for an animated skeleton - See Animation\Animation\Playback\Multithreaded\hkaAnimationJobs.h for details.
class hkaAnimationSampleAndCombineJob : public hkaAnimationJob
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIMATION, hkaAnimationSampleAndCombineJob);

		HK_FORCE_INLINE hkaAnimationSampleAndCombineJob();

		HK_FORCE_INLINE ~hkaAnimationSampleAndCombineJob() {}

		inline hkBool isValid() const;

	public:

			/// Helper structure for data needed to sample and blend a single animation control.
		struct ControlData
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIMATION, hkaAnimationSampleAndCombineJob::ControlData);

			HK_FORCE_INLINE ControlData();	// Only exists on PPU for error checking
			HK_FORCE_INLINE ~ControlData() {}

			HK_FORCE_INLINE void init();

			inline hkBool isValid() const;	// Only exists on PPU for error checking

				/// The frame number such that local time for the sample is between m_frameIndex and m_frameIndex+1.
			HK_ALIGN16(hkUint32					m_frameIndex);
				/// The delta between frames, in range [0,1). You can use getFrameAndDelta() to calculate this and m_frameIndex for a given time.
			hkReal								m_frameDelta;
				/// The master weight for blending this animation.
			hkReal								m_weight;	
				/// The binding for this animation.
			hkaAnimationBinding					m_binding;
				/// The type of animation data stored in the chunks.
			hkaAnimation::AnimationType	m_animationType;
				 /// The animation data required to sample at 'localTime', given by m_frameIndex and m_frameDelta
			hkaAnimation::DataChunk		m_chunks[HK_MAX_NUM_DATA_CHUNKS];
				/// The number of chunks that contain valid data.
			hkInt32								m_numValidChunks;
				/// The per track weights for this control.
			const hkUint8*						m_transformTrackWeights;
				/// Set to 0 if no per track weight are required.
			hkInt32								m_numTransformTrackWeights;
				/// The per track weights for this control
			const hkUint8*						m_floatTrackWeights;  
				/// Set to 0 if no per track weight are required.
			hkInt32								m_numFloatTrackWeights; 
		};

		/* Sample inputs */
			/// Pointer to original animated skeleton - used only in shared memory architectures, otherwise ignored.
		const hkaAnimatedSkeleton* m_animatedSkeletonOnCPU;
			/// Point to the reference pose or HK_NULL for no filling.
		class hkQsTransform*	m_referencePose;
			/// The fill threshold (see hkaAnimatedSkeleton).
		hkReal					m_referencePoseWeightThreshold;
			/// The total number of bones in the output skeleton - used only if local-to-model space conversion is required.
		hkUint32				m_numSkeletonBones;
			/// The parent indices array - used only if local-to-model space conversion is required.
		const hkInt16*			m_parentIndices;
			/// The highest number of bones to sample.
		hkUint32				m_maxBone;
			/// The highest number of float slots to sample.
		hkUint32				m_maxFloatSlot;

		/* Animation Control Data */
			/// Note: The control data here is expected to be ordered by blend hint. Normal blending comes first followed by additive.
		ControlData*			m_animationControls;
			/// Number of controls.
		hkUint32				m_numAnimationControls;
			/// Required to aid reserving of stack space for track sample data.
		hkUint16				m_maxTransformTracksInAllAnimations;
			/// Required to aid reserving of stack space for track sample data.
		hkUint16				m_maxFloatTracksInAllAnimations; 
			/// Optional cache - used only in shared memory architectures, otherwise ignored.
		hkaChunkCache*			m_cache;

		/* Output Data */
			/// The transform/bone results are placed in this buffer in main memory. If m_parentIndices == HK_NULL, these are in local space, otherwise they are in model space.
		hkQsTransform*			m_poseOut;
			/// The float results are placed in this buffer in main memory.
		hkReal*					m_floatSlotsOut;
			/// This semaphore is released when the work is finished.
			/// Can be set to HK_NULL if you don't need to wait on a specific job
		hkSemaphoreBusyWait*	m_jobDoneSemaphore;
			/// This flag is incremented when the work is finished.
			/// Can be set to HK_NULL if you don't need to wait on a specific job
		hkUint32*						m_jobDoneFlag;
};

#include <Animation/Animation/Playback/Multithreaded/hkaAnimationJobs.inl>

#endif // HK_ANIMATION_JOBS_H

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
