/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKA_SIMD_SKINNING_DEFORMER_H
#define HKA_SIMD_SKINNING_DEFORMER_H

#include <Animation/Animation/Deform/Skinning/hkaSkinningDeformer.h>

/// The derived reference counted class for a SIMD based implementation of weighted vertex deformation.
/// Applies to both indexed and non indexed skinning.
/// This deformer requires that the input buffers' deformable
/// members (pos, normals) be aligned as a hkVector4 (16byte aligned as it is 128bit)
/// and for faster operation make sure that the outputs are aligned as well.
/// N.B. It is important to note that these deformers are here to be used by Havok's demos but are not production quality.
/// It is assumed that deforming will be done most commonly by your graphics engine, usually in hardware on GPUs or VUs.
/// That hardware deformation is usually performed at the same time as per vertex lighting operations, so Havok cannot
/// provide optimized deformers for all such game specific usage.
class hkaSimdSkinningDeformer : public hkReferencedObject, public hkaSkinningDeformer
{
	public:

			/// Constructs an unbound deformer
		hkaSimdSkinningDeformer();

			/// Bind this deformer to input and output buffers.
			/// The input format is assumed to have (at least) vertex weights.
			/// The output buffer should be preallocated.
			/// Returns false if the deformer does not support the input or output buffer format.
		hkBool bind( const hkaVertexDeformerInput& input, const hkxVertexBuffer* inputBuffer,  hkxVertexBuffer* outputBuffer );

			/// Deform the input buffer into the output buffer using the array of matrices specified.
			/// The deformer must first be bound and the output buffer locked before deforming.
		virtual void deform(  const hkTransform* m_worldCompositeMatrices );

		struct hkaSimdBinding
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_RUNTIME, hkaSimdSkinningDeformer::hkaSimdBinding );

			// Input buffer
			const hkVector4* m_iPosBase;
			const hkVector4* m_iNormBase;
			const hkVector4* m_iBinormBase;
			const hkVector4* m_iTangentBase;
			const hkUint8*   m_iWeightBase;
			const hkUint8*   m_iIndexBase;
			hkUint8 m_iPosVectorStride; // STRIDE IN VECTOR4s
			hkUint8 m_iNormVectorStride;
			hkUint8 m_iBinormVectorStride;
			hkUint8 m_iTangentVectorStride;
			hkUint8 m_iIndexByteStride; // STRIDE IN BYTES
			hkUint8 m_iWeightByteStride; // STRIDE IN BYTES
			hkUint8 m_bonesPerVertex;

			// Output buffer
			hkReal* m_oPosBase;
			hkReal* m_oNormBase;
			hkReal* m_oBinormBase;
			hkReal* m_oTangentBase;
			hkUint8 m_oPosFloatStride;
			hkUint8 m_oNormFloatStride;
			hkUint8 m_oBinormFloatStride;
			hkUint8 m_oTangentFloatStride;

			hkUint32 m_numVerts;
			hkBool m_outputAligned;
		};

			/// Will call either one of the aligned static funcs, depending on m_outputAligned in binding.
		static void HK_CALL deform( const hkTransform* m_worldCompositeMatrices, const hkaSimdBinding& binding  );

			/// Input and Output are both 16 byte aligned
		static void HK_CALL deformAligned( const hkTransform* m_worldCompositeMatrices, const hkaSimdBinding& binding  );

			/// Only Input is 16 byte aligned, requires internal loop copy to unaligned float output buffers so slower
		static void HK_CALL deformAlignedInput( const hkTransform* m_worldCompositeMatrices, const hkaSimdBinding& binding );

	protected:

		struct hkaSimdBinding m_binding;
};

#endif // HKA_SIMD_SKINNING_DEFORMER_H

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
