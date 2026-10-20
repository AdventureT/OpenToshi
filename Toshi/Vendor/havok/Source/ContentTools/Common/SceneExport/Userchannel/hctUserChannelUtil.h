/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_USER_CHANNEL_UTIL_H
#define HK_USER_CHANNEL_UTIL_H

#include <Common/Base/Container/Array/hkObjectArray.h>

	/// This class stores component data (vertex, edge, triangle selection and data) using global indices for the components.
	/// Then, with the help of maps between section indices and global indices, it creates and stores that data in the sections
	/// of an hkxMesh.
	/// This is used by all exporters to easily map between data in the modeller to user channels in the hkxMesh sections
class hctUserChannelUtil
{
	public:

			/// Defined the type of global component data. Currently only "CT_VERTEX_SELECTION" is supported.
			/// Note that FACE channels will become TRIANGLE channels.
		enum ChannelType
		{
			CT_INVALID = -1,

			CT_VERTEX_SELECTION =0,
			CT_VERTEX_FLOAT,            // floats per vertex counting duplicated vertices as identical
			CT_SAMPLE_FLOAT,            // floats per vertex counting duplicated vertices separately
			CT_VERTEX_INT,				// not implemented yet
			CT_VERTEX_VECTOR,			// not implemented yet

			CT_EDGE_SELECTION = 10 ,	// not implemented yet

			CT_FACE_SELECTION = 20	
		};

		enum ChannelDimensions
		{
			CD_INVALID = -1,
			
			CD_FLOAT = 0,
			CD_DISTANCE,
			CD_ANGLE
		};

			/// Union of index, int, float and vector elements (depending on the type of component data)
		union ChannelDataItem
		{
			int m_index;
			int m_integer;
			float m_float;
			float m_vector[4];
		};

			/// Data describing a channel for component data. Indices are global.
		struct GlobalChannel
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_EXPORT, GlobalChannel );
			hkString m_channelName;
			ChannelType m_channelType;
			ChannelDimensions m_channelDimensions;
			hkArray<ChannelDataItem> m_channelData;

			GlobalChannel (const GlobalChannel& other)
			{
				m_channelName = other.m_channelName;
				m_channelType = other.m_channelType;
				m_channelDimensions = other.m_channelDimensions;
				m_channelData = other.m_channelData;
			}

			GlobalChannel () : m_channelName (""), m_channelType (CT_INVALID) {}
		};

			/// A map between the indices of this section and the original indices of the global mesh
		struct SectionToGlobalMap
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_EXPORT, SectionToGlobalMap );

			// Maps vertices in the section to vertices in the mesh, counting duplicated vertices as identical
			hkArray<int> m_sectionVertexIdToGlobalVertexId; 

			// Maps vertices in the section to vertices in the mesh, counting duplicated vertices separately
			hkArray<int> m_sectionVertexIdToGlobalSampleId; 
			
			hkArray<int> m_sectionTriangleIdToGlobalFaceId;

			SectionToGlobalMap (const SectionToGlobalMap& other)
			{
				m_sectionVertexIdToGlobalVertexId = other.m_sectionVertexIdToGlobalVertexId;
				m_sectionVertexIdToGlobalSampleId = other.m_sectionVertexIdToGlobalSampleId;
				m_sectionTriangleIdToGlobalFaceId = other.m_sectionTriangleIdToGlobalFaceId;
			}

			SectionToGlobalMap () {}
		};

			/// Add a new user channel to the mesh
		void addGlobalChannel (const GlobalChannel& globalChannel);

			/// Register a new section in the mesh and provide a map between indices in the section and indices in the original mesh
		void registerSection (const SectionToGlobalMap& sectionMap);

			/// Converts the stored global channels to individual per-section channels, which are added to the hkxMeshSections of the
			/// given mesh. Allocations are done using the given hctFilterMemoryTracker object.
		void storeChannelsInMesh (hkxMesh* theMesh,  class hctFilterMemoryTracker* memoryTracker);

			/// Clears all the stored data. Useful if the object is reused for multiple meshes
		void clear();
		
	private:

		hkObjectArray<GlobalChannel> m_globalChannels;

		hkObjectArray<SectionToGlobalMap> m_sectionMaps;

		void processVertexSelection (class hctFilterMemoryTracker* memory, int channelNumber, int sectionNumber, hkxMesh* theMesh) const;
		void processFaceSelection (class hctFilterMemoryTracker* memory, int channelNumber, int sectionNumber, hkxMesh* theMesh) const;
		void processFloatData (class hctFilterMemoryTracker* memory, int ChannelNumber, int sectionNumber, hkxMesh* theMesh) const;

};



#endif // HK_USER_CHANNEL_UTIL_H


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
