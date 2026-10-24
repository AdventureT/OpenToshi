/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_FILTER_SCENE_REMOVE_SCALE__H
#define HAVOK_FILTER_SCENE_REMOVE_SCALE__H

class hctRemoveScaleFilter : public hctFilterInterface
{
	public: 

		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_EXPORT);

		hctRemoveScaleFilter(const hctFilterManagerInterface* owner);
		/*virtual*/ ~hctRemoveScaleFilter();
		
		/*virtual*/ void process( class hkRootLevelContainer& data );
		
};

class hctRemoveScaleFilterDesc : public hctFilterDescriptor
{
	public:

		/*virtual*/ unsigned int getID() const { return 0x9d42b3ae; }
		/*virtual*/ FilterCategory getCategory() const { return HK_CATEGORY_CORE; }
		/*virtual*/ FilterBehaviour getFilterBehaviour() const { return HK_DATA_MUTATES_INPLACE; }
		/*virtual*/ const char* getShortName() const { return "Bake Scale"; }
		/*virtual*/ const char* getLongName() const { return "Remove scale from all nodes and bake it into attached objects."; }
		/*virtual*/ unsigned int getFilterVersion() const { return HCT_FILTER_VERSION(1,1,0); }
		/*virtual*/ hctFilterInterface* createFilter(const hctFilterManagerInterface* owner) const { return new hctRemoveScaleFilter(owner); }
		
		/*virtual*/ HavokComponentMask getRequiredHavokComponents () const { return HK_COMPONENT_COMMON; }
};

extern hctRemoveScaleFilterDesc g_removeScaleDesc;

#endif // HAVOK_FILTER_SCENE_REMOVE_SCALE__H

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
