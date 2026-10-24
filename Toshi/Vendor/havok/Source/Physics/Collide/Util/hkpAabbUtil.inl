/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */


#include <Common/Base/Types/Geometry/Aabb/hkAabb.h>

void HK_CALL hkpAabbUtil::calcAabb( const hkTransform& localToWorld, const hkVector4& halfExtents, float extraRadius, hkAabb& aabbOut )
{
#if defined (HK_PS2) && defined (HK_COMPILER_GCC) && (HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED)
	asm (
"		lqc2		vf8, 0x00(%3)	#	column(0)			\n"		\
"		qmtc2			%4, vf12	# extra radius \n"				\
"		lqc2		vf9, 0x10(%3)	#	column(1)			\n"		\
"		lqc2		vf10, 0x20(%3)	#	column(2)			\n"		\
"		lqc2		vf11, 0x30(%3)	#	column(3)			\n"		\
"		vaddx.xyzw	vf12, vf0, vf12 #   broadcast extra radius \n"	\
"		vmulx.xyz	vf8, vf8,  %2		# transformedX.setMul4( halfExtents(0), localToWorld.getRotation().getColumn(0))		\n"		\
"		vmuly.xyz	vf9, vf9,  %2		# transformedY.setMul4( halfExtents(1), localToWorld.getRotation().getColumn(0))		\n"		\
"		vmulz.xyz	vf10,vf10, %2		# transformedZ.setMul4( halfExtents(2), localToWorld.getRotation().getColumn(0))		\n"		\
"		vabs.xyz    vf8, vf8			# abs x \n"		\
"		vabs.xyz    vf9, vf9			# abs y \n"		\
"		vabs.xyz    vf10, vf10			# abs z \n"		\
"		vmulaw.xyzw	     ACC, vf8, vf0	# x+y+z\n"		\
"		vmaddaw.xyzw	 ACC, vf9, vf0	# \n"			\
"		vmaddaw.xyzw	 ACC, vf12,vf0	# extra radius \n"			\
"		vmaddw.xyzw		%1, vf10, vf0	# extends = all together\n"		\
"		vsub.xyz	%0, vf11, %1		# min = offset - extends  \n"	\
"		vadd.xyz	%1, vf11, %1		# max += offset	\n"
 : "=j" (aabbOut.m_min.getQuad() ), "=j" (aabbOut.m_max.getQuad() )
 :  "j"(halfExtents.getQuad()), "r"( &localToWorld.getRotation().getColumn(0) ), "r" ( extraRadius )
 : "vf8", "vf9", "vf10", "vf11", "vf12", "memory"	);
#else

	hkVector4 he0; he0.setBroadcast3clobberW( halfExtents, 0 );
	hkVector4 he1; he1.setBroadcast3clobberW( halfExtents, 1 );
	hkVector4 he2; he2.setBroadcast3clobberW( halfExtents, 2 );

	hkVector4 transformedX; transformedX.setMul4( he0, localToWorld.getRotation().getColumn(0));
	hkVector4 transformedY; transformedY.setMul4( he1, localToWorld.getRotation().getColumn(1));
	hkVector4 transformedZ; transformedZ.setMul4( he2, localToWorld.getRotation().getColumn(2));
	
	transformedX.setAbs4( transformedX );
	transformedY.setAbs4( transformedY );
	transformedZ.setAbs4( transformedZ );

	hkVector4 max = aabbOut.m_max; // copy so local on xbox etc
	hkVector4 min = aabbOut.m_min;

	hkVector4 extra; extra.setAll3( extraRadius );
	transformedZ.add4( extra );
	max.setAdd4( transformedX, transformedY );
	max.add4( transformedZ );

	min.setNeg4(max);
	max.add4( localToWorld.getTranslation() );
	min.add4( localToWorld.getTranslation()  );

	aabbOut.m_max = max;
	aabbOut.m_min = min;
#endif
}


void HK_CALL hkpAabbUtil::calcAabb( const hkTransform& localToWorld, const hkVector4& halfExtents, const hkVector4& center, float extraRadius, hkAabb& aabbOut )
{
#if defined HK_PS2 && defined (HK_COMPILER_GCC) && (HK_CONFIG_SIMD == HK_CONFIG_SIMD_ENABLED)
	asm (
"		lqc2		vf8, 0x00(%3)	#	column(0)			\n"		\
"		qmtc2			%4, vf12	# extra radius \n"				\
"		lqc2		vf9, 0x10(%3)	#	column(1)			\n"		\
"		lqc2		vf10, 0x20(%3)	#	column(2)			\n"		\
"		lqc2		vf11, 0x30(%3)	#	column(3)			\n"		\
"		vaddx.xyzw	vf12, vf0, vf12 #   broadcast extra radius \n"	\
"		vmulax.xyz	ACC,  vf8, %5	#   vf13.setTransformedPos(center)	\n" \
"		vmadday.xyz	ACC,  vf9, %5	#   vf13.setTransformedPos(center)	\n" \
"		vmaddaz.xyz	ACC,  vf10, %5	#   vf13.setTransformedPos(center)	\n" \
"		vmaddw.xyz	vf11, vf11, vf0	#   vf13.setTransformedPos(center)	\n" \
"		vmulx.xyz	vf8, vf8,  %2		# transformedX.setMul4( halfExtents(0), localToWorld.getRotation().getColumn(0))		\n"		\
"		vmuly.xyz	vf9, vf9,  %2		# transformedY.setMul4( halfExtents(1), localToWorld.getRotation().getColumn(0))		\n"		\
"		vmulz.xyz	vf10,vf10, %2		# transformedZ.setMul4( halfExtents(2), localToWorld.getRotation().getColumn(0))		\n"		\
"		vabs.xyz    vf8, vf8			# abs x \n"		\
"		vabs.xyz    vf9, vf9			# abs y \n"		\
"		vabs.xyz    vf10, vf10			# abs z \n"		\
"		vmulaw.xyzw	     ACC, vf8, vf0	# x+y+z\n"		\
"		vmaddaw.xyzw	 ACC, vf9, vf0	# \n"			\
"		vmaddaw.xyzw	 ACC, vf10,vf0	# extra radius \n"			\
"		vmaddw.xyzw		%1, vf12, vf0	# extends = all together\n"		\
"		vsub.xyz	%0, vf11, %1		# min = offset - extends  \n"	\
"		vadd.xyz	%1, vf11, %1		# max += offset	\n"
 : "=j" (aabbOut.m_min.getQuad() ), "=j" (aabbOut.m_max.getQuad() )
 :  "j"(halfExtents.getQuad()), "r"( &localToWorld.getRotation().getColumn(0) ), "r" ( extraRadius ), "j" (center.getQuad())
 : "vf8", "vf9", "vf10", "vf11", "vf12", "memory"	);
#else
	hkVector4 he0; he0.setBroadcast3clobberW( halfExtents, 0 );
	hkVector4 he1; he1.setBroadcast3clobberW( halfExtents, 1 );
	hkVector4 he2; he2.setBroadcast3clobberW( halfExtents, 2 );

	hkVector4 transformedX; transformedX.setMul4( he0, localToWorld.getRotation().getColumn(0));
	hkVector4 transformedY; transformedY.setMul4( he1, localToWorld.getRotation().getColumn(1));
	hkVector4 transformedZ; transformedZ.setMul4( he2, localToWorld.getRotation().getColumn(2));

	transformedX.setAbs4( transformedX );
	transformedY.setAbs4( transformedY );
	transformedZ.setAbs4( transformedZ );

	hkVector4 max = aabbOut.m_max; // copy so local on xbox etc
	hkVector4 min = aabbOut.m_min;

	hkVector4 extra; extra.setAll3( extraRadius );
	transformedZ.add4( extra );

	max.setAdd4( transformedX, transformedY );
	max.add4( transformedZ );

	min.setNeg4(max);

	hkVector4 temp;
	temp._setTransformedPos(localToWorld, center);

	max.add4( temp );
	min.add4( temp );

	aabbOut.m_max = max;
	aabbOut.m_min = min;
#endif
}


void hkpAabbUtil::sweepAabb(const hkMotionState* motionState, hkReal tolerance, const hkAabb& aabbIn, hkAabb& aabbOut)
{
	//
	//  Expand the aabb by the angular part
	//

	hkReal radius = motionState->m_deltaAngle(3) * motionState->m_objectRadius;
	hkVector4 rotationalGain; rotationalGain.setAll(radius);
	aabbOut.m_min.setSub4(aabbIn.m_min, rotationalGain);
	aabbOut.m_max.setAdd4(aabbIn.m_max, rotationalGain);

	//
	// restrict the size of the AABB to the worst case radius size
	//
	{
		hkVector4 radius4; radius4.setAll3( motionState->m_objectRadius + tolerance );
		hkVector4 maxR; maxR.setAdd4( motionState->getSweptTransform().m_centerOfMass1, radius4 );
		hkVector4 minR; minR.setSub4( motionState->getSweptTransform().m_centerOfMass1, radius4 );

		HK_ON_DEBUG
		(
			// I consider success when (aabbInOut.m_min <= maxR) and (minR <= aabbInOut.m_max) are true for all XYZ
			hkVector4Comparison a = aabbIn.m_min.compareLessThanEqual4( maxR );
			hkVector4Comparison b = minR.compareLessThanEqual4( aabbIn.m_max );
			hkVector4Comparison both; both.setAnd(a,b);
			HK_ASSERT2(0x366ca7b2, both.allAreSet(hkVector4Comparison::MASK_XYZ), "Invalid Radius. Did you make changes to a shape or change its center of mass while the owner was added to the world?" );
		);

		aabbOut.m_min.setMax4( aabbOut.m_min, minR );
		aabbOut.m_min.setMin4( aabbOut.m_min, aabbIn.m_min );

		aabbOut.m_max.setMin4( aabbOut.m_max, maxR );
		aabbOut.m_max.setMax4( aabbOut.m_max, aabbIn.m_max );
	}

	//
	// Sweep/expand the aabb along the motion path
	//

	hkVector4 invPath; invPath.setSub4( motionState->getSweptTransform().m_centerOfMass0, motionState->getSweptTransform().m_centerOfMass1 );
	hkVector4 zero; zero.setZero4();
	hkVector4 minPath; minPath.setMin4( zero, invPath );
	hkVector4 maxPath; maxPath.setMax4( zero, invPath );

	aabbOut.m_min.add4( minPath );
	aabbOut.m_max.add4( maxPath );

#if defined(HK_DEBUG)
	hkReal diffMinX = aabbIn.m_min(0) - aabbOut.m_min(0);
	hkReal diffMinY = aabbIn.m_min(1) - aabbOut.m_min(1);
	hkReal diffMinZ = aabbIn.m_min(2) - aabbOut.m_min(2);
	hkReal diffMaxX = aabbOut.m_max(0) - aabbIn.m_max(0);
	hkReal diffMaxY = aabbOut.m_max(1) - aabbIn.m_max(1);
	hkReal diffMaxZ = aabbOut.m_max(2) - aabbIn.m_max(2);
	HK_ASSERT2( 0xaf63e413, diffMinX >= 0.0f && diffMinY >= 0.0f && diffMinZ >= 0.0f && diffMaxX >= 0.0f && diffMaxY >= 0.0f && diffMaxZ >= 0.0f, "Expanded AABB is smaller than the unexpanded AABB." );
#endif
}


HK_ON_CPU( HK_FORCE_INLINE void HK_CALL hkpAabbUtil::sweepOffsetAabb(const hkpAabbUtil::OffsetAabbInput& input, const hkAabb& aabbIn, hkAabb& aabbOut) )
HK_ON_SPU( HK_FORCE_INLINE void HK_CALL hkAabbUtil_sweepOffsetAabb(const hkpAabbUtil::OffsetAabbInput& input, const hkAabb& aabbIn, hkAabb& aabbOut) )
{
	hkSimdReal half(0.5f);
	hkVector4 aabbHalfSize; aabbHalfSize.setSub4(aabbIn.m_max, aabbIn.m_min);  aabbHalfSize.mul4(half);
	hkVector4 aabbCenter; aabbCenter.setInterpolate4(aabbIn.m_max, aabbIn.m_min, half);
	hkVector4 arm; arm._setTransformedPos(input.m_endTransformInv, aabbCenter);


	hkVector4 min = aabbCenter;
	hkVector4 max = aabbCenter;
	{
		hkVector4 p; p._setTransformedPos(input.m_startTransform, arm);
		min.setMin4(min, p);
		max.setMax4(max, p);
	}

	// extended arm for the in-between transforms (cos(22.5deg)
	const hkVector4 centerOfMassLocal = input.m_motionState->getSweptTransform().m_centerOfMassLocal;

	for (int i = 0; i < input.m_numTransforms; i++)
	{	
		hkVector4 extendedArm;
		extendedArm.setSub4(arm, centerOfMassLocal);
		extendedArm.mul4(input.m_transforms[i].getTranslation().getSimdAt(3));
		extendedArm.add4(centerOfMassLocal);

		hkVector4 p; p._setTransformedPos(input.m_transforms[i], extendedArm);
		min.setMin4(min, p);
		max.setMax4(max, p);
	}

	//
	// Expand the aabb due to angular rotation of the shape around the aabbIn's center
	//
	{
		hkSimdReal r = aabbHalfSize.length3();
		hkVector4 radius; radius.setAll(r);

		aabbHalfSize.addMul4(input.m_motionState->m_deltaAngle.getSimdAt(3), radius);
		aabbHalfSize.setMin4(radius, aabbHalfSize);
	}

	aabbOut.m_min.setSub4(min, aabbHalfSize);
	aabbOut.m_max.setAdd4(max, aabbHalfSize);

	// We need to ensure that in order for our hkAabbUint32 compression to work
	aabbOut.m_min.setMin4(aabbIn.m_min, aabbOut.m_min);
	aabbOut.m_max.setMax4(aabbIn.m_max, aabbOut.m_max);
}

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
