#pragma once
#include "Math/TVector4.h"

class hkpConstraintData;
class hkpConstraintInstance;

TOSHI_NAMESPACE_START

class TPhysicsObjectContainer;

// A constraint between two bodies, owned by TPhysicsManager (m_pConstraints, 0x74 bytes)
// NOTE: names of the members are guesses from their use (TPhysicsManager's constraint functions aren't ported yet)
class TPhysicsConstraintContainer
{
public:
	// m_eType; NOTE: names from the Havok constraint data TPhysicsManager::AddConstraint creates
	enum Type : TUINT32
	{
		TYPE_NONE          = 0,
		TYPE_BALLANDSOCKET = 1, // hkpBallAndSocketConstraintData: m_vPivotA
		TYPE_STIFFSPRING   = 2, // hkpStiffSpringConstraintData: m_vPivotA, m_vPivotB
		TYPE_HINGE         = 3, // hkpHingeConstraintData: m_vPivotA, m_vAxis
		TYPE_LIMITEDHINGE  = 4, // hkpLimitedHingeConstraintData: m_vPivotA, m_vAxis, m_fFriction/m_fMinAngle/m_fMaxAngle
		TYPE_PRISMATIC     = 5, // hkpPrismaticConstraintData
		TYPE_WHEEL         = 6, // hkpWheelConstraintData: m_vPivotA, the axle m_vPivotB, the steering axis m_vSteeringAxis, the suspension axis m_vSuspensionAxis, m_vLimits the suspension
	};

	// m_uiFlags
	enum Flags : TUINT32
	{
		PCCF_MALLEABLE = 1 << 0, // wrapped in a hkpMalleableConstraintData, m_pBreakData->m_fValue is the strength
		PCCF_BREAKABLE = 1 << 1, // wrapped in a hkpBreakableConstraintData, m_pBreakData->m_fValue is the threshold
	};

	// What m_pBreakableData points to (0x38 bytes, or 8 for a malleable constraint)
	struct BreakData
	{
		TFLOAT m_fValue;      // 0x00 the threshold (breakable) or the strength (malleable)
		TFLOAT m_fAngularDampingBrokenA;    // 0x04 body A after it broke: copied to the body's m_fAngularDamping
		TFLOAT m_fLinearDampingBrokenA;     // 0x08 body A after it broke: copied to the body's m_fLinearDamping
		TFLOAT m_fMaxLinearVelocityBrokenA; // 0x0C body A after it broke: copied to the body's m_fMaxLinearVelocity
		TFLOAT m_fAngularDampingBrokenB;    // 0x10 the same for body B
		TFLOAT m_fLinearDampingBrokenB;     // 0x14
		TFLOAT m_fMaxLinearVelocityBrokenB; // 0x18
		TFLOAT m_fAngularDampingA;          // 0x1C the Havok motion values of body A when the constraint was added
		TFLOAT m_fLinearDampingA;           // 0x20
		TFLOAT m_fMaxLinearVelocityA;       // 0x24
		TFLOAT m_fAngularDampingB;          // 0x28 the same of body B
		TFLOAT m_fLinearDampingB;           // 0x2C
		TFLOAT m_fMaxLinearVelocityB;       // 0x30
		TBOOL  m_bBodyA;      // 0x34 body A changes when it broke
		TBOOL  m_bBodyB;      // 0x35
		TUINT8 m_uiBodyAFlag; // 0x36
		TUINT8 m_uiBodyBFlag; // 0x37
	};

public:
	TPhysicsConstraintContainer();

	// Makes the constraint breakable with a new, cleared m_pBreakableData (threshold 0; TPhysicsManager's
	// SetConstraintBreakable does the same and sets the threshold)
	// $deBlob: FUNCTION 005a6220
	void SetBreakable()
	{
		m_uiFlags = ( m_uiFlags & ~( PCCF_MALLEABLE | PCCF_BREAKABLE ) ) | PCCF_BREAKABLE;

		if ( m_pBreakableData != TNULL )
			TFree( m_pBreakableData );

		BreakData* pData = TSTATICCAST( BreakData, TMalloc( sizeof( BreakData ) ) );

		if ( pData != TNULL )
		{
			pData->m_fValue  = 0.0f;
			pData->m_bBodyA  = TFALSE;
			pData->m_bBodyB  = TFALSE;
			m_pBreakableData = pData;
		}
		else
		{
			m_pBreakableData = TNULL;
		}
	}

public:
	TUINT32                  m_eType;          // 0x00 Type
	TUINT32                  m_uiFlags;        // 0x04 Flags
	TINT                     m_iWorld;         // 0x08
	TBOOL                    m_bInWorld;       // 0x0c
	TBOOL                    m_bWantRuntime;   // 0x0d
	TPhysicsObjectContainer* m_pBodyA;         // 0x10
	TPhysicsObjectContainer* m_pBodyB;         // 0x14
	hkpConstraintData*       m_pData;          // 0x18
	hkpConstraintInstance*   m_pInstance;      // 0x1c
	TVector4                 m_vPivotA;        // 0x20 in world space (set up by TPhysicsManager's SetConstraint* functions)
	TVector4                 m_vPivotB;        // 0x30 the second pivot (stiff spring) or the axis (hinges)
	TVector4                 m_vSteeringAxis;  // 0x40
	TVector4                 m_vSuspensionAxis; // 0x50
	TVector4                 m_vLimits;         // 0x60 x: the friction, y/z: the angle limits of the limited hinge
	BreakData*               m_pBreakableData; // 0x70 (TFree'd)
};

TOSHI_NAMESPACE_END
