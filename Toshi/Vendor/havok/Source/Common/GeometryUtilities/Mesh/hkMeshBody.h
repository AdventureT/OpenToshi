/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HK_MESH_BODY_H
#define HK_MESH_BODY_H

class hkTransformSet;
class hkMeshShape;
class hkMeshVertexBuffer;

extern const hkClass hkMeshBodyClass;

/// The mesh body is a transformed instance of a mesh shape
///
/// This interface is implemented to allow Havok libraries such as destruction to communicate with a graphics implementation.
///
/// Generally speaking mesh bodies are constructed using the 'createBody' factory method on an hkMeshSystem instance.
/// A mesh body created on a hkMeshSystem in general can only be used on the instance it was constructed on.
///
/// The hkMeshBody is just a renderable instantiation of a hkMeshShape. The only additional data the body has over the shape
/// is where to draw the mesh shape. Each hkMeshBody has a transform which locates the body in the scene.
/// If the mesh shape associated with the mesh body has skinning information or per mesh section transform indices, additional
/// position information comes in the form of the indexed transform. Each transform is controls a bone and/or a mesh section
/// transform index.
///
/// To update the position of a hkMeshBody - the methods setTransform() and setIndexedTransforms() can be used. In and of
/// themselves they do not necessarily change the rendering position, as the application has to call 'completeUpdate' to
/// commit the updates. This allows an implementation to potentially optimize how the positional information of the moved
/// body is calculated. Thus correct usage if updating both the main transform and indexed transforms is to call both methods
/// and then finally call 'completeUpdate' so both changes can be processed in one go. If transform updates are made, and
/// 'completeUpdate' is not called behavior is undefined.
///
/// When implementing the hkMeshBody the implementation should honor how the vertex elements are shared. The most simple way to
/// do this for the hkMeshBody to clone the unique hkVertexBuffers specified in the hkMeshSections of the associated hkMeshShape.
/// If the hkVertexBuffer has been implemented correctly the clone should correctly create deep copies of the unshared members.
/// The hkMeshBody provides a way to gain access to body specific vertex buffers through the 'getVertexBuffer' method. This
/// provides per body access to per body vertex data (ie FLAG_NOT_SHARED elements).
///
/// If all of the elements vertex buffer are shared an implementation can return HK_NULL to the getVertexBuffer
/// method - in this situation there is no per body vertex information. Also note that an implementation is free to return a
/// vertex buffer with only the non-shared elements as necessary. Having the hkMeshBody return per instance vertex buffers
/// allows the application to modify vertex attributes on a per body level. For example if color was specified to be not
/// shared, on the vertex buffer, then each mesh body could have different colors by modifying their vertex buffers.
///
/// There are two implementations of hkMeshBody in the standard SDK distribution - firstly the hkgMeshBody, which
/// implements the interface for Havok demos \p hkg graphics library. Secondly there is hkMemoryMeshBody - which is an implementation used by
/// the hkMemoryMeshSystem, and isn't designed for rendering but for allowing processing of hkMesh based classes in
/// memory.
///
/// \sa hkMeshShape

class hkMeshBody: public hkReferencedObject
{
	public:
		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_SCENE_DATA);

			/// Helper enum to specify what type of pointer is used for mouse picking in the Havok demos
        enum PickDataIdentifier
        {
            PICK_BREAKABLE_BODY = 1,            ///
            PICK_USER = 0x1000,                 ///
        };

	
			/// Get the mesh shape
		virtual const hkMeshShape* getMeshShape() const = 0;

			/// Get the currently set model to world transform
		virtual void getTransform( hkMatrix4& transform ) const = 0;

			/// Set the current model to world transform
		virtual void setTransform(const hkMatrix4& matrix) = 0;

			/// Get the per body per mesh section vertex buffer/s - one per section (note they can be shared)
			/// If there is no per body vertex data, this method can return HK_NULL.
            /// The format of this buffer must minimally contain the elements specified as instancable (FLAG_NOT_SHARED elements) in the
			/// shapes vertex buffer. It may or may not contained the shared members - reading or writing to non instance
			/// members produces undefined results.
		virtual hkMeshVertexBuffer* getVertexBuffer(int sectionIndex) = 0;

            /// Get the number of indexed transforms
        virtual int getNumIndexedTransforms() = 0;

            /// Set the indexed transforms - when all changes are made, make a call to 'completeUpdate' to complete the update
        virtual void setIndexedTransforms(int startIndex, const hkMatrix4* matrices, int numMatrices) = 0;
            /// Get the indexed transforms
        virtual void getIndexedTransforms(int startIndex, hkMatrix4* matrices, int numMatrices) = 0;
            /// Get the indexed inverse transforms
        virtual void getIndexedInverseTransforms(int startIndex, hkMatrix4* matrices, int numMatrices) = 0;

			/// Set data that is used for picking. Id is used to identify the type of data
		virtual hkResult setPickingData(int id, void* data) { return HK_SUCCESS; }

			/// If changes are made to the transform set, or the transform, this method
            /// must be called to ensure correct updating of the graphics representation.
		virtual void completeUpdate() = 0;

		virtual const char* getName() const { return HK_NULL; }
		virtual void setName(const char* n) const { }
};

#endif // HK_MESH_BODY_H

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
