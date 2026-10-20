/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_FPU_SKINNING_DEFORMER_H
#define HK_FPU_SKINNING_DEFORMER_H

#include <Animation/Animation/Deform/Skinning/hkaSkinningDeformer.h>
class hkxVertexBuffer;

/// The derived reference counted class for a FPU based implementation of weighted vertex deformation. 
/// Applies to both indexed and non indexed skinning.
/// This is a pure, normal floating point only deformer. If neither your input
/// or you output is hkVector4 aligned and you are not using a SIMD enabled
/// hkMath library, then this relatively slow float version is at least better
/// than using the simd ones when there is no simd ops enabled.
/// N.B. It is important to note that these deformers are here to be used by Havok's demos but are not production quality. 
/// It is assumed that deforming will be done most commonly by your graphics engine, usually in hardware on GPUs or VUs.
/// That hardware deformation is usually performed at the same time as per vertex lighting operations, so Havok cannot 
/// provide optimized deformers for all such game specific usage.
class hkaFPUSkinningDeformer : public hkReferencedObject, public hkaSkinningDeformer
{
	public:

			/// Constructs an unbound deformer
		hkaFPUSkinningDeformer();

			/// Bind this deformer to input and output buffers.
			/// The input format is assumed to have (at least) vertex weights.
			/// The output buffer should be preallocated.
			/// Returns false if the deformer does not support the input or output buffer format.
		hkBool bind( const hkaVertexDeformerInput& input, const hkxVertexBuffer* inputBuffer,  hkxVertexBuffer* outputBuffer );

			/// Deform the input buffer into the output buffer using the array of matrices specified.
			/// The deformer must first be bound and the output buffer locked before deforming.
		virtual void deform( const hkTransform* m_worldCompositeMatrices );

		struct hkaFloatBinding
		{
			HK_DECLARE_NONVIRTUAL_CLASS_ALLOCATOR( HK_MEMORY_CLASS_ANIM_RUNTIME, hkaFPUSkinningDeformer::hkaFloatBinding );

			// Input buffer
			const float* m_iPosBase;
			const float* m_iNormBase;
			const float* m_iBinormBase;
			const float* m_iTangentBase;
			const hkUint8* m_iWeightBase;
			const hkUint8* m_iIndexBase;
			hkUint8 m_iPosStride; // ALL STRIDES IN NUM FLOATS
			hkUint8 m_iNormStride;
			hkUint8 m_iBinormStride;
			hkUint8 m_iTangentStride;
			hkUint8 m_iWeightStride;
			hkUint8 m_iIndexStride;
			hkUint8 m_bonesPerVertex;
			
			// Output Buffer
			float* m_oPosBase;
			float* m_oNormBase;
			float* m_oBinormBase;
			float* m_oTangentBase;
			hkUint8 m_oPosStride; // ALL STRIDES IN NUM FLOATS
			hkUint8 m_oNormStride;
			hkUint8 m_oBinormStride;
			hkUint8 m_oTangentStride;

			hkUint32 m_numVerts;
		};

			/// Static version of the virtual deform, takes an explicit binding
			/// so you can use this if you don't have hkxVertexBuffer style data
		static void HK_CALL deform( const hkTransform* m_worldCompositeMatrices, const hkaFloatBinding& binding );
	
		
	protected:

		struct hkaFloatBinding m_binding;
};

#endif // HK_FLOAT_SKINNING_DEFORMER_H

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
