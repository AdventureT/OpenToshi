/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_ANIMATION_PREVIEW_COLOR_H
#define HK_ANIMATION_PREVIEW_COLOR_H

/// hkaAnimationBinding meta information
extern const class hkClass hkaAnimationPreviewColorClass;
extern const class hkClass hkaAnimationPreviewColorContainerClass;

/// The hkaAnimationPreviewColor class stores visulazation
/// information for footstep analysis.  It is indended for use by
/// the content tool chain.
class hkaAnimationPreviewColor
{
public:

	HK_DECLARE_REFLECTION();

	hkaAnimationPreviewColor( hkUint32 color );

	hkUint32 m_color;
};


// Utility class to store hkaAnimationPreviewColor objects
class hkaAnimationPreviewColorContainer
{
public:

	HK_DECLARE_REFLECTION();

	hkaAnimationPreviewColorContainer();
	~hkaAnimationPreviewColorContainer();

	// Creates a new hkaAnimationPreviewColor object and adds it to this container
	hkaAnimationPreviewColor* newPreviewColor( hkUint32 color );

	// Storage for hkaAnimationPreviewColor objects
	class hkaAnimationPreviewColor** m_previewColor;
	hkInt32 m_numPreviewColor;
};

#endif // HK_ANIMATION_PREVIEW_COLOR_H

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
