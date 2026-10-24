/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */




void HK_CALL hkSweptTransformUtil::_lerp2( const hkSweptTransform& sweptTrans, hkReal t, hkQuaternion& quatOut )
{
	hkVector4 rot0 = sweptTrans.m_rotation0.m_vec;
	hkVector4 rot1 = sweptTrans.m_rotation1.m_vec;
	{
		quatOut.m_vec.setAdd4( rot0, rot1 );
#ifdef HK_PS2
		quatOut.normalize();
#else
		hkReal len2 = quatOut.m_vec.lengthSquared4();
		hkReal r = 0.75f - 0.125f * len2;
		r = r * (1.5f - 0.5f * len2 * r * r);
		quatOut.m_vec.mul4(r);
#endif
	}

	if ( t < 0.5f )
	{
		quatOut.m_vec.setInterpolate4( rot0, quatOut.m_vec, 2.0f * t);
	}
	else
	{
		quatOut.m_vec.setInterpolate4( quatOut.m_vec, rot1, 2.0f * t - 1.0f);
	}
	quatOut.m_vec.normalize4();
}



void HK_CALL hkSweptTransformUtil::_lerp2( const hkSweptTransform& sweptTrans, hkReal t, hkTransform& transformOut )
{
	hkQuaternion qt;
	_lerp2( sweptTrans, t, qt );

	transformOut.setRotation( qt );
	transformOut.getTranslation().setInterpolate4( sweptTrans.m_centerOfMass0, sweptTrans.m_centerOfMass1, t);

	hkVector4 centerShift;
	centerShift._setRotatedDir( transformOut.getRotation(), sweptTrans.m_centerOfMassLocal);
	transformOut.getTranslation().sub4( centerShift );
}


void HK_CALL hkSweptTransformUtil::calcTransAtT0( const hkSweptTransform& sweptTrans, hkTransform& transformOut )
{
	const hkQuaternion& qt = sweptTrans.m_rotation0;

	transformOut.setRotation( qt );

	hkVector4 centerShift;
	centerShift._setRotatedDir( transformOut.getRotation(), sweptTrans.m_centerOfMassLocal);
	transformOut.getTranslation().setSub4( sweptTrans.m_centerOfMass0, centerShift );
}


void HK_CALL hkSweptTransformUtil::calcTransAtT1( const hkSweptTransform& sweptTrans, hkTransform& transformOut )
{
	const hkQuaternion& qt = sweptTrans.m_rotation1;

	transformOut.setRotation( qt );

	hkVector4 centerShift;
	centerShift._setRotatedDir( transformOut.getRotation(), sweptTrans.m_centerOfMassLocal);
	transformOut.getTranslation().setSub4( sweptTrans.m_centerOfMass1, centerShift );
}

void HK_CALL hkSweptTransformUtil::_clipVelocities( const hkMotionState& motionState, hkVector4& linearVelocity, hkVector4& angularVelocity )
{
	hkReal linVelSq = linearVelocity.lengthSquared3();
	hkReal angVelSq = angularVelocity.lengthSquared3();
	const hkReal maxLinear = motionState.m_maxLinearVelocity * motionState.m_maxLinearVelocity;
	const hkReal maxAngular = motionState.m_maxAngularVelocity * motionState.m_maxAngularVelocity;
	if ( (linVelSq > maxLinear) || (linVelSq!=linVelSq) )
	{
		hkReal f = motionState.m_maxLinearVelocity * hkMath::sqrtInverse( linVelSq );
		linearVelocity.mul4( f );
		if ( linVelSq!=linVelSq )
		{
			linearVelocity = hkTransform::getIdentity().getColumn(0);
			HK_WARN(0xf0124242, "Nan velocity detected, something is seriously wrong (probably bad inertia tensors)");
		}
	}
	if ( (angVelSq > maxAngular) || ( angVelSq != angVelSq ))
	{
		hkReal f = motionState.m_maxAngularVelocity * hkMath::sqrtInverse( angVelSq );
		angularVelocity.mul4( f );
		if ( angVelSq!=angVelSq )
		{
			angularVelocity = hkTransform::getIdentity().getColumn(0);
			HK_WARN(0xf0143243, "Nan velocity detected, something is seriously wrong (probably bad inertia tensors ");
		}
	}
}


// Has to be outside of inline function as gcc won't inline functions with statics in them.
static hkQuadReal _stepMotionStateMaxVelf = HK_QUADREAL_CONSTANT(1e6f,1e6f,1e6f,1e6f);

void HK_CALL hkSweptTransformUtil::_stepMotionState( const hkStepInfo& info,
											 hkVector4& linearVelocity, hkVector4& angularVelocity,
											 hkMotionState& motionState )
{
#ifdef HK_DEBUG
	{
		if ( motionState.getSweptTransform().getInvDeltaTime() != 0.0f)
		{
			hkReal motionEndTime = motionState.getSweptTransform().getBaseTime() + 1.0f / motionState.getSweptTransform().getInvDeltaTime();
			HK_ASSERT(0xf0f0083, hkMath::equal(info.m_startTime, motionEndTime ) ) ;
		}
	}
#endif
	// check for nans in velocities
	// fix nans to 100
	{
		hkVector4 absLin; absLin.setAbs4( linearVelocity );
		hkVector4 absAng; absAng.setAbs4( angularVelocity );

		hkVector4 maxVel; maxVel.getQuad() = _stepMotionStateMaxVelf;
		int mask = absLin.compareLessThan4( maxVel ).getMask() & absAng.compareLessThan4( maxVel ).getMask();
		mask    &= linearVelocity.compareEqual4( linearVelocity ).getMask() & angularVelocity.compareEqual4( angularVelocity ).getMask();
		if ( (mask & hkVector4Comparison::MASK_XYZ) != hkVector4Comparison::MASK_XYZ )
		{
			// velocity to a 'random' non zero velocity
			linearVelocity  = hkTransform::getIdentity().getColumn(0);
			angularVelocity = hkTransform::getIdentity().getColumn(0);
			HK_WARN(0xf0123244, "Nan velocity or velocity > 1e6f detected, something is seriously wrong (probably bad inertia tensors ");
		}
	}

	motionState.getSweptTransform().m_centerOfMass0 = motionState.getSweptTransform().m_centerOfMass1;
	motionState.getSweptTransform().m_centerOfMass0(3) = info.m_startTime;

	hkReal linVelSq = linearVelocity.lengthSquared3();
	if ( linVelSq > motionState.m_maxLinearVelocity * motionState.m_maxLinearVelocity )
	{
		//HK_WARN_ONCE(0xf0327683, "Object exceeding maximum velocity, velocity clipped" );
		hkReal f = motionState.m_maxLinearVelocity * hkMath::sqrtInverse( linVelSq );
		linearVelocity.mul4( f );
	}

	motionState.getSweptTransform().m_centerOfMass1.addMul4(info.m_deltaTime.val(), linearVelocity);
	motionState.getSweptTransform().m_centerOfMass1(3) = info.m_invDeltaTime;

	hkQuaternion newRotation = motionState.getSweptTransform().m_rotation1;
	motionState.getSweptTransform().m_rotation0 = newRotation;

	//
	//	Calculate a new rotation, the fabs angle and angle squared
	//
	hkReal angle;
	{
		hkQuaternion dq;	dq.m_vec.setMul4( info.m_deltaTime * 0.5f, angularVelocity );
		const hkReal pi = HK_REAL_PI;
		hkReal dx2 = hkReal(dq.m_vec.lengthSquared3()) * (4.0f/ (pi*pi));

		// do a little sin(alpha * 0.5f) approximation
		const hkReal a = 0.822948f;
		const hkReal b = 0.130529f;
		const hkReal c = 0.044408f;

		const hkReal maxAnglePerStep = hkMath::min2( 0.9f, info.m_deltaTime.val() * motionState.m_maxAngularVelocity );

		// clipping angular velocity to be between [0, PI*0.9/dt]
		// info: was "<", is "<=" -- works ok for zero dt now.
		if ( dx2 <= maxAnglePerStep * maxAnglePerStep )
		{
			const hkReal dx4 = dx2 * dx2;
			const hkReal w = 1.0f - a*dx2 - b*dx4 - c*dx2*dx4;
			dq.m_vec(3) = w;
		}
		else
		{
			hkReal factor = maxAnglePerStep * hkMath::sqrtInverse( dx2 );
			angularVelocity.mul4( factor );
			dq.m_vec.mul4( factor );

			             dx2  = maxAnglePerStep*maxAnglePerStep;
			const hkReal dx4  = dx2 * dx2;
			const hkReal w    = 1.0f - a*dx2 - b*dx4 - c*dx2*dx4;
			dq.m_vec(3) = w;
		}

		newRotation.setMul( dq, newRotation );
		newRotation.normalize();

		motionState.m_deltaAngle.setAdd4( dq.m_vec, dq.m_vec );
		angle = hkMath::sqrt(dx2) * pi;
		motionState.m_deltaAngle(3) = angle;
	}
	motionState.getSweptTransform().m_rotation1 = newRotation;
	calcTransAtT1( motionState.getSweptTransform(), motionState.getTransform());
}







void HK_CALL hkSweptTransformUtil::deactivate( hkMotionState& ms )
{
	hkSweptTransform& sweptTransform = ms.getSweptTransform();
	ms.m_deltaAngle.setZero4();

	sweptTransform.m_rotation0 = sweptTransform.m_rotation1;
	sweptTransform.m_centerOfMass0 = sweptTransform.m_centerOfMass1;
	sweptTransform.m_centerOfMass1(3) = 0.0f;
}



//void HK_CALL hkSweptTransformUtil::calcTimInfo( const hkMotionState& ms0, const hkMotionState& ms1, hkVector4& timOut)
//{
//	HK_ASSERT2(0xad44d321, st0.getInvDeltaTime() == st1.getInvDeltaTime(), "Internal error: hkSweptTransform's must correspond to the same deltaTime in order to use void HK_CALL hkSweptTransformUtil::calcTimInfo( const hkMotionState& ms0, const hkMotionState& ms1, hkVector4& timOut)");
//
//	const hkSweptTransform& st0 = ms0.getSweptTransform();
//	const hkSweptTransform& st1 = ms1.getSweptTransform();
//
//	hkVector4 diff0; diff0.setSub4( st0.m_centerOfMass0, st0.m_centerOfMass1 );
//	hkVector4 diff1; diff1.setSub4( st1.m_centerOfMass1, st1.m_centerOfMass0 );
//
//	timOut.setAdd4( diff0, diff1 );
//
//	timOut(3) = ms0.m_deltaAngle(3) * ms0.m_objectRadius + ms1.m_deltaAngle(3) * ms1.m_objectRadius;
//
//}

void HK_CALL hkSweptTransformUtil::calcTimInfo( const hkMotionState& ms0, const hkMotionState& ms1, hkReal deltaTime, hkVector4& timOut)
{
	const hkSweptTransform& st0 = ms0.getSweptTransform();
	const hkSweptTransform& st1 = ms1.getSweptTransform();

	hkVector4 diff0; diff0.setSub4( st0.m_centerOfMass0, st0.m_centerOfMass1 );
	hkVector4 diff1; diff1.setSub4( st1.m_centerOfMass1, st1.m_centerOfMass0 );

	hkReal f0 = deltaTime * st0.getInvDeltaTime();
	hkReal f1 = deltaTime * st1.getInvDeltaTime();

	HK_ASSERT2(0xad56daaa, f0 <= 1.01f && f1 <= 1.01f, "Internal error: input for TIM calculation may be corrupted.");

	timOut.setMul4( f0, diff0 );
	timOut.addMul4( f1, diff1 );
	timOut(3) = f0 * ms0.m_deltaAngle(3) * ms0.m_objectRadius  + f1 * ms1.m_deltaAngle(3) * ms1.m_objectRadius;

	// we don't project angular velocity just to keep it simple ( no cross products)
}

	// Calculates angular distance (angVelocity * dt) travelled by the bodies. 
void HK_CALL hkSweptTransformUtil::calcAngularTimInfo( const hkMotionState& ms0, const hkMotionState& ms1, hkReal deltaTime, hkVector4* HK_RESTRICT deltaAngleOut0, hkVector4* HK_RESTRICT deltaAngleOut1)
{
	const hkSweptTransform& st0 = ms0.getSweptTransform();
	const hkSweptTransform& st1 = ms1.getSweptTransform();
	hkReal f0 = deltaTime * st0.getInvDeltaTime();
	hkReal f1 = deltaTime * st1.getInvDeltaTime();

	hkVector4 ang0; ang0.setMul4( f0, ms0.m_deltaAngle );
	hkVector4 ang1; ang1.setMul4( f1, ms1.m_deltaAngle );

	deltaAngleOut0[0] = ang0;
	deltaAngleOut1[0] = ang1;
}

void HK_CALL hkSweptTransformUtil::calcCenterOfMassAt( const hkMotionState& ms, hkTime t, hkVector4& centerOut )
{
	hkReal iv = ms.getSweptTransform().getInterpolationValue( t );
	centerOut.setInterpolate4( ms.getSweptTransform().m_centerOfMass0, ms.getSweptTransform().m_centerOfMass1, iv );
}

void HK_CALL hkSweptTransformUtil::getVelocity( const hkMotionState& ms, hkVector4& linearVelOut, hkVector4& angularVelOut )
{
	linearVelOut.setSub4 (ms.getSweptTransform().m_centerOfMass1, ms.getSweptTransform().m_centerOfMass0);
	linearVelOut.mul4( ms.getSweptTransform().getInvDeltaTime() );
	angularVelOut.setMul4( ms.getSweptTransform().getInvDeltaTime(), ms.m_deltaAngle );
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
