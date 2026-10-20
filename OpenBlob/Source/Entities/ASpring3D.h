#pragma once
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: a damped mass spring in 3D (the x, y and z axes separately), used by ABlobEntity's jelly (the wobble of the
// ball's model around the body, 0x440). The 3D counterpart of ASpring.
//-----------------------------------------------------------------------------
class ASpring3D
{
public:
	ASpring3D( const Toshi::TVector4& a_rcPosition, TFLOAT a_fStiffness, TFLOAT a_fDamping );
	ASpring3D( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcStiffness, const Toshi::TVector4& a_rcDamping );

	// Restart the spring at a_rcPosition (at rest, no velocity); the scalar version keeps the force
	void Init( const Toshi::TVector4& a_rcPosition, TFLOAT a_fStiffness, TFLOAT a_fDamping );
	void Init( const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcStiffness, const Toshi::TVector4& a_rcDamping );

	// The critical damping of the three axes (SetDampingRatio( 1.0f))
	void SetCriticalDamping();
	~ASpring3D();

	// Adds a_rcForce to the force of the next Tick
	void AddForce( const Toshi::TVector4& a_rcForce );

	// Adds a_rcVelocity to the velocity
	void AddVelocity( const Toshi::TVector4& a_rcVelocity );

	// Integrates the spring over a_fDeltaTime and clears the force
	void Tick( TFLOAT a_fDeltaTime );

	// Sets the stiffness of the three axes
	void SetStiffness( TFLOAT a_fStiffness );

	// Sets the damping of the three axes as a ratio of their critical damping
	void SetDampingRatio( TFLOAT a_fRatio );

public:
	Toshi::TVector4 m_vPosition;    // 0x00
	Toshi::TVector4 m_vRest;        // 0x10
	Toshi::TVector4 m_vVelocity;    // 0x20
	Toshi::TVector4 m_vForce;       // 0x30
	Toshi::TVector4 m_vStiffness;   // 0x40
	Toshi::TVector4 m_vDamping;     // 0x50
	TFLOAT          m_fMass;        // 0x60 1.0f
	TFLOAT          m_fInverseMass; // 0x64 1.0f
};

//-----------------------------------------------------------------------------
// Purpose: a damped spring of a quaternion: the four components are integrated like ASpring3D's axes with a scalar
// stiffness and damping, then normalised. Nothing uses it in retail.
//-----------------------------------------------------------------------------
class ASpringQuaternion
{
public:
	ASpringQuaternion( const Toshi::TVector4& a_rcOrientation, TFLOAT a_fStiffness, TFLOAT a_fDamping );

	// Restarts the spring at a_rcOrientation (at rest, no velocity)
	void Init( const Toshi::TVector4& a_rcOrientation, TFLOAT a_fStiffness, TFLOAT a_fDamping );

	// Integrates the spring over a_fDeltaTime
	void Tick( TFLOAT a_fDeltaTime );

public:
	Toshi::TVector4 m_vOrientation; // 0x00
	Toshi::TVector4 m_vRest;        // 0x10
	Toshi::TVector4 m_vVelocity;    // 0x20
	TFLOAT          m_fStiffness;   // 0x30
	TFLOAT          m_fDamping;     // 0x34
	TFLOAT          m_fMass;        // 0x38 1.0f
	TFLOAT          m_fInverseMass; // 0x3C 1.0f
};
