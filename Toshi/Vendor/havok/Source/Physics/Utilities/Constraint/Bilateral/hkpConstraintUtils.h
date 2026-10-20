/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_UTILITIES_CONSTRAINTUTILS_H
#define INC_UTILITIES_CONSTRAINTUTILS_H

#include <Common/Base/hkBase.h>

class hkpConstraintInstance;
class hkpConstraintMotor;
class hkpPositionConstraintMotor;
class hkpConstraintChainInstance;
class hkpBallSocketChainData;
class hkpEntity;
class hkVector4;


class hkpConstraintInstance;
class hkpConstraintMotor;
class hkpPositionConstraintMotor;
class hkpConstraintChainInstance;
class hkpBallSocketChainData;
class hkpEntity;
class hkVector4;
class hkpConstraintData;

	/// Constraint utility functions.
class hkpConstraintUtils
{
public:

		/// Converts the given constraint to its powered counterpart, using the given motor. If the constraint cannot be powered or it's powered already it returns HK_NULL
		/// and raises a warning.
	static hkpConstraintInstance* HK_CALL convertToPowered (const hkpConstraintInstance* originalConstraint, hkpConstraintMotor* constraintMotor, hkBool enableMotors);

		/// Converts the given constraint to its corresponding 'limits' counterpart. This only works for powered/non-powered hinge and ragdoll constraints.
		/// It returns the original constraint if it already is of a 'limits' type.
		/// The function returns HK_NULL and asserts for all other constraints.
	static hkpConstraintInstance* HK_CALL convertToLimits (hkpConstraintInstance* originalConstraint);

		/// Checks whether the linear component of a constraint is valid. If large errors (distance between pivots > maxAllowedError) are detected,
		/// the constraint's child object (A) is moved towards the position of the parent object (B) by the amount specified by "relativeDisplacementTowardsParent".
		/// This can be called for the constraints of a rag doll to avoid the situation of the ragdoll getting stuck with an arm / leg in the landscape (by moving the child
		/// rigid body towards the parent, the rag doll is actually "pushed" away from the landscape.
		/// Notice that this method expects the pivots of rigid bodies to match the constraints representing the joints.
		/// The method return true if any fix up is done, false if no fix up is done.
	static hkBool HK_CALL checkAndFixConstraint (const hkpConstraintInstance* constraint, hkReal maxAllowedError = 0.05f, hkReal relativeDisplacementTowardsParent = 0.8f);


		/// Extracts pivot vectors from the constraint.
	static hkResult HK_CALL getConstraintPivots(const hkpConstraintInstance* constraint, hkVector4& pivotA, hkVector4& pivotB);

		/// Extracts motors from the constraints.
	static hkResult HK_CALL getConstraintMotors(const hkpConstraintData* constraintData, hkpConstraintMotor*& motor0, hkpConstraintMotor*& motor1, hkpConstraintMotor*& motor2 );

};

#endif // INC_UTILITIES_CONSTRAINTUTILS_H

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
