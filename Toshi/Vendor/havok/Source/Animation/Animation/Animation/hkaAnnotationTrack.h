/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKANIMATION_ANIMATION_HKANIMATIONTRACK_HKCLASS_H
#define HKANIMATION_ANIMATION_HKANIMATIONTRACK_HKCLASS_H

/// hkaAnnotationTrack meta information
extern const class hkClass hkAnimationTrackClass;

/// Annotation information for an animated bone track.
class hkaAnnotationTrack
{
	public:

		HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_DATA, hkaAnnotationTrack);
		HK_DECLARE_REFLECTION();
	
			/// The information needed to construct an simple animation annotation.
		struct Annotation
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIMATION, hkaAnnotationTrack::Annotation );
			HK_DECLARE_REFLECTION();

				/// The animation time associated with this annotation
			hkReal m_time;
			
				/// The annotation text, null terminated.
			const char * m_text;
		};
		
		//
		// Members
		//
	public:
		
			/// The animation track name.
		char* m_trackName;
		
			/// The annotations associated with this animation track.
		struct Annotation* m_annotations;
			/// Size of m_annotations array.
		hkInt32 m_numAnnotations;
};

#endif // HKANIMATION_ANIMATION_HKANIMATIONTRACK_HKCLASS_H

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
