#include "ToshiPCH.h"
#include "TParticleAffector.h"
#include "TGenericParticles.h"
#include "Math/TRandom.h"
#include "Math/TMathInline.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// The data of the affectors (floats after TParticleAffector::TRBHeader)
struct AffectorFloatData : TParticleAffector::TRBHeader
{
	TFLOAT m_afValues[ 6 ]; // 0x0C
};

static AffectorFloatData* GetFloatData( TParticleAffector::TRBHeader* a_pData )
{
	return TSTATICCAST( AffectorFloatData, a_pData );
}

// TParticleAffectorRandomForce's random (0x009b3858, constructed by the file's static initialiser)
// $deBlob: FUNCTION 005400f0
static TRandom s_oRandomForceRandom;

//-----------------------------------------------------------------------------
// TParticleAffector
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 0071c600
TParticleAffector::TParticleAffector( const TParticleAffector& a_rcOrig )
{
	TASSERT( a_rcOrig.m_pData->m_uiSize >= sizeof( TParticleAffector::TRBHeader ) && "a_pOrig->m_pData->m_uiSize >= sizeof(TParticleAffector::TRBHeader)" );

	m_pData = TSTATICCAST( TRBHeader, TMalloc( a_rcOrig.m_pData->m_uiSize ) );
	TUtil::MemCopy( m_pData, a_rcOrig.m_pData, a_rcOrig.m_pData->m_uiSize );
	m_pNext    = TNULL;
	m_fnUpdate = a_rcOrig.m_fnUpdate;
}

// $deBlob: FUNCTION 0071c670
TParticleAffector::TParticleAffector( TRBHeader* a_pData )
{
	m_pNext    = TNULL;
	m_fnUpdate = TNULL;
	m_pData    = TSTATICCAST( TRBHeader, TMalloc( a_pData->m_uiSize ) );
	TUtil::MemCopy( m_pData, a_pData, a_pData->m_uiSize );
}

// $deBlob: FUNCTION 0071c6b0
// $deBlob: FUNCTION 0071af60
TParticleAffector::~TParticleAffector()
{
	TFree( m_pData );
}

// $deBlob: FUNCTION 0071c6c0
void TParticleAffector::Create()
{
	switch ( m_pData->m_eType )
	{
		case TYPE_PHYSICS:
			m_fnUpdate = TParticleAffectorPhysics::Update;
			return;
		case TYPE_DRAG:
			m_fnUpdate = TParticleAffectorDrag::Update;
			return;
		case TYPE_CAPPEDSPEEDDRAG:
			m_fnUpdate = TParticleAffectorCappedSpeedDrag::Update;
			return;
		case TYPE_FORCE:
			m_fnUpdate = TParticleAffectorForce::Update;
			return;
		case TYPE_RANDOMFORCE:
			m_fnUpdate = TParticleAffectorRandomForce::Update;
			return;
		case TYPE_VELOCITYROTATE:
			m_fnUpdate = TParticleAffectorVelocityRotate::Update;
			return;
		case TYPE_PUSHPULL:
			m_fnUpdate = TParticleAffectorPushPull::Update;
			return;
		case TYPE_ROTATEAROUNDEMITTERX:
			m_fnUpdate = TParticleAffectorRotateAroundEmitter::UpdateX;
			return;
		case TYPE_ROTATEAROUNDEMITTERY:
			m_fnUpdate = TParticleAffectorRotateAroundEmitter::UpdateY;
			return;
		case TYPE_ROTATEAROUNDEMITTERZ:
			m_fnUpdate = TParticleAffectorRotateAroundEmitter::UpdateZ;
			return;
		case TYPE_CUSTOM:
			m_fnUpdate = TParticleAffectorRotateAroundEmitter::UpdateCustom;
			return;
	}

	TASSERT( 0 );
}

//-----------------------------------------------------------------------------
// The affectors
//-----------------------------------------------------------------------------

// $deBlob: FUNCTION 0071edc0
void __stdcall TParticleAffectorPhysics::Update( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData )
{
	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVelocity = a_pParticles->m_avVelocity[ i ];
		TVector4& rPosition = a_pParticles->m_avPosition[ i ];

		TVector4 vMove;
		vMove.Multiply( rVelocity, a_fDeltaTime );
		rPosition.x = rPosition.x + vMove.x;
		rPosition.y = rPosition.y + vMove.y;
		rPosition.z = rPosition.z + vMove.z;
	}
}

// $deBlob: FUNCTION 0071ee50
void __stdcall TParticleAffectorDrag::Update( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData )
{
	TFLOAT fDrag = GetFloatData( a_pData )->m_afValues[ 0 ] * a_fDeltaTime;
	TMath::Clip( fDrag, -1.0f, 1.0f );

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVelocity = a_pParticles->m_avVelocity[ i ];

		TVector4 vDrag( rVelocity.x * fDrag, rVelocity.y * fDrag, rVelocity.z * fDrag, rVelocity.w * 0.0f );
		rVelocity.x = rVelocity.x - vDrag.x;
		rVelocity.y = rVelocity.y - vDrag.y;
		rVelocity.z = rVelocity.z - vDrag.z;
	}
}

// $deBlob: FUNCTION 0071ef10
void __stdcall TParticleAffectorCappedSpeedDrag::Update( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData )
{
	AffectorFloatData* pData   = GetFloatData( a_pData );
	TFLOAT             fScale  = 1.0f - pData->m_afValues[ 0 ] * a_fDeltaTime;
	TFLOAT             fMinSpeed = pData->m_afValues[ 1 ];

	if ( fMinSpeed <= 0.0f )
	{
		for ( TINT i = a_iStart; i < a_iEnd; i++ )
			a_pParticles->m_avVelocity[ i ].Multiply( fScale );
	}
	else
	{
		TFLOAT fMinSpeedSq = fMinSpeed * fMinSpeed;

		for ( TINT i = a_iStart; i < a_iEnd; i++ )
		{
			TVector4& rVelocity = a_pParticles->m_avVelocity[ i ];
			TFLOAT    fSpeedSq  = rVelocity.MagnitudeSq();

			if ( fMinSpeedSq < fSpeedSq )
			{
				TFLOAT fSpeed    = TFLOAT( sqrt( TDOUBLE( fSpeedSq ) ) );
				TFLOAT fNewSpeed = fSpeed * fScale;
				rVelocity.Multiply( TMath::Max( fNewSpeed, pData->m_afValues[ 1 ] ) / fSpeed );
			}
		}
	}
}

// $deBlob: FUNCTION 0071f200
void __stdcall TParticleAffectorForce::Update( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData )
{
	AffectorFloatData* pData = GetFloatData( a_pData );

	TVector4 vForce( pData->m_afValues[ 0 ], pData->m_afValues[ 1 ], pData->m_afValues[ 2 ], 0.0f );
	vForce.Multiply( pData->m_afValues[ 3 ] * a_fDeltaTime );
	TFLOAT fGravity = pData->m_afValues[ 4 ] * a_fDeltaTime;

	if ( 0.0001f < vForce.x || 0.0001f < vForce.y || 0.0001f < vForce.z || vForce.x < -0.0001f || vForce.y < -0.0001f || vForce.z < -0.0001f )
	{
		for ( TINT i = a_iStart; i < a_iEnd; i++ )
		{
			TVector4& rVelocity = a_pParticles->m_avVelocity[ i ];

			TVector4 vAccel;
			vAccel.Divide( vForce, a_pParticles->m_afMass[ i ] );
			vAccel.y = vAccel.y + fGravity;
			rVelocity.x = rVelocity.x + vAccel.x;
			rVelocity.y = rVelocity.y + vAccel.y;
			rVelocity.z = rVelocity.z + vAccel.z;
		}
	}
	else
	{
		for ( TINT i = a_iStart; i < a_iEnd; i++ )
		{
			TVector4& rVelocity = a_pParticles->m_avVelocity[ i ];
			rVelocity.x         = rVelocity.x + 0.0f;
			rVelocity.y         = fGravity + rVelocity.y;
			rVelocity.z         = rVelocity.z + 0.0f;
		}
	}
}

// $deBlob: FUNCTION 0071f380
void __stdcall TParticleAffectorRandomForce::Update( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData )
{
	AffectorFloatData* pData = GetFloatData( a_pData );
	TFLOAT             fMinX = pData->m_afValues[ 0 ];
	TFLOAT             fMinY = pData->m_afValues[ 1 ];
	TFLOAT             fMinZ = pData->m_afValues[ 2 ];

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVelocity = a_pParticles->m_avVelocity[ i ];

		TVector4 vForce;
		vForce.x = s_oRandomForceRandom.GetFloatMinMax( fMinX, pData->m_afValues[ 3 ] );
		vForce.y = s_oRandomForceRandom.GetFloatMinMax( fMinY, pData->m_afValues[ 4 ] );
		vForce.z = s_oRandomForceRandom.GetFloatMinMax( fMinZ, pData->m_afValues[ 5 ] );
		vForce.Multiply( a_fDeltaTime / a_pParticles->m_afMass[ i ] );

		rVelocity.x = rVelocity.x + vForce.x;
		rVelocity.y = vForce.y + rVelocity.y;
		rVelocity.z = vForce.z + rVelocity.z;
	}
}

// $deBlob: FUNCTION 0071f550
void __stdcall TParticleAffectorVelocityRotate::Update( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData )
{
	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rVelocity = a_pParticles->m_avVelocity[ i ];
		rVelocity.RotateX( GetFloatData( a_pData )->m_afValues[ 0 ] );
		rVelocity.RotateY( GetFloatData( a_pData )->m_afValues[ 1 ] );
		rVelocity.RotateZ( GetFloatData( a_pData )->m_afValues[ 2 ] );
	}
}

// $deBlob: FUNCTION 0071f790
void __stdcall TParticleAffectorPushPull::Update( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData )
{
	AffectorFloatData* pData = GetFloatData( a_pData );
	TVector4           vTarget( pData->m_afValues[ 0 ], pData->m_afValues[ 1 ], pData->m_afValues[ 2 ], 1.0f );

	if ( !a_pParticles->HasFlag( TGenericParticles::FLAGS_TRANSWITHXFORM ) && !a_pParticles->HasFlag( TGenericParticles::FLAGS_TRANSROTWITHXFORM ) )
	{
		// The target is in the emitter's space
		TMatrix44 oInverse;
		oInverse.InvertOrthogonal( a_pParticles->m_oTransform );
		TMatrix44::TransformVector( vTarget, oInverse, vTarget );
	}

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rPosition = a_pParticles->m_avPosition[ i ];
		TVector4& rVelocity = a_pParticles->m_avVelocity[ i ];

		TVector4 vDir( vTarget.x - rPosition.x, vTarget.y - rPosition.y, vTarget.z - rPosition.z, vTarget.w );
		vDir.Normalise();
		vDir.Multiply( a_fDeltaTime * pData->m_afValues[ 3 ] );

		TVector4 vAccel;
		vAccel.Divide( vDir, a_pParticles->m_afMass[ i ] );
		rVelocity.x = rVelocity.x + vAccel.x;
		rVelocity.y = rVelocity.y + vAccel.y;
		rVelocity.z = rVelocity.z + vAccel.z;
	}
}

// $deBlob: FUNCTION 0071f8f0
void __stdcall TParticleAffectorRotateAroundEmitter::UpdateX( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData )
{
	if ( !a_pParticles->HasFlag( TGenericParticles::FLAGS_TRANSWITHXFORM ) && !a_pParticles->HasFlag( TGenericParticles::FLAGS_TRANSROTWITHXFORM ) )
	{
		UpdateWorldX( a_pParticles, a_iStart, a_iEnd, a_fDeltaTime, a_pData );
		return;
	}

	TFLOAT fSin, fCos;
	TMath::SinCos( GetFloatData( a_pData )->m_afValues[ 0 ] * a_fDeltaTime, fSin, fCos );

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rPosition = a_pParticles->m_avPosition[ i ];
		TFLOAT    fZ        = rPosition.z;
		rPosition.z         = fZ * fCos + rPosition.y * fSin;
		rPosition.y         = rPosition.y * fCos - fZ * fSin;
	}
}

// $deBlob: FUNCTION 0071f9b0
void __stdcall TParticleAffectorRotateAroundEmitter::UpdateY( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData )
{
	if ( !a_pParticles->HasFlag( TGenericParticles::FLAGS_TRANSWITHXFORM ) && !a_pParticles->HasFlag( TGenericParticles::FLAGS_TRANSROTWITHXFORM ) )
	{
		UpdateWorldY( a_pParticles, a_iStart, a_iEnd, a_fDeltaTime, a_pData );
		return;
	}

	TFLOAT fSin, fCos;
	TMath::SinCos( GetFloatData( a_pData )->m_afValues[ 0 ] * a_fDeltaTime, fSin, fCos );

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rPosition = a_pParticles->m_avPosition[ i ];
		TFLOAT    fZ        = rPosition.z;
		rPosition.z         = fZ * fCos - rPosition.x * fSin;
		rPosition.x         = rPosition.x * fCos + fZ * fSin;
	}
}

// $deBlob: FUNCTION 0071fa70
void __stdcall TParticleAffectorRotateAroundEmitter::UpdateZ( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData )
{
	if ( !a_pParticles->HasFlag( TGenericParticles::FLAGS_TRANSWITHXFORM ) && !a_pParticles->HasFlag( TGenericParticles::FLAGS_TRANSROTWITHXFORM ) )
	{
		UpdateWorldZ( a_pParticles, a_iStart, a_iEnd, a_fDeltaTime, a_pData );
		return;
	}

	TFLOAT fSin, fCos;
	TMath::SinCos( GetFloatData( a_pData )->m_afValues[ 0 ] * a_fDeltaTime, fSin, fCos );

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rPosition = a_pParticles->m_avPosition[ i ];
		TFLOAT    fY        = rPosition.y;
		rPosition.y         = rPosition.x * fSin + fY * fCos;
		rPosition.x         = rPosition.x * fCos - fY * fSin;
	}
}

// $deBlob: FUNCTION 0071fb30
void __stdcall TParticleAffectorRotateAroundEmitter::UpdateWorldX( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData )
{
	TFLOAT fSin, fCos;
	TMath::SinCos( GetFloatData( a_pData )->m_afValues[ 0 ] * a_fDeltaTime, fSin, fCos );

	TFLOAT fCentreY = a_pParticles->m_oTransform.m_f42;
	TFLOAT fCentreZ = a_pParticles->m_oTransform.m_f43;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rPosition = a_pParticles->m_avPosition[ i ];
		TFLOAT    fY        = rPosition.y - fCentreY;
		TFLOAT    fZ        = rPosition.z - fCentreZ;
		rPosition.y         = fY;
		rPosition.z         = fZ;
		rPosition.z         = fZ * fCos + fY * fSin + fCentreZ;
		rPosition.y         = ( fY * fCos - fZ * fSin ) + fCentreY;
	}
}

// $deBlob: FUNCTION 0071fbf0
void __stdcall TParticleAffectorRotateAroundEmitter::UpdateWorldY( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData )
{
	TFLOAT fSin, fCos;
	TMath::SinCos( GetFloatData( a_pData )->m_afValues[ 0 ] * a_fDeltaTime, fSin, fCos );

	TFLOAT fCentreX = a_pParticles->m_oTransform.m_f41;
	TFLOAT fCentreZ = a_pParticles->m_oTransform.m_f43;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rPosition = a_pParticles->m_avPosition[ i ];
		TFLOAT    fZ        = rPosition.z - fCentreZ;
		TFLOAT    fX        = rPosition.x - fCentreX;
		rPosition.z         = fZ;
		rPosition.x         = fX;
		rPosition.z         = ( fZ * fCos - fX * fSin ) + fCentreZ;
		rPosition.x         = fZ * fSin + fX * fCos + fCentreX;
	}
}

// $deBlob: FUNCTION 0071fcb0
void __stdcall TParticleAffectorRotateAroundEmitter::UpdateWorldZ( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData )
{
	TFLOAT fSin, fCos;
	TMath::SinCos( GetFloatData( a_pData )->m_afValues[ 0 ] * a_fDeltaTime, fSin, fCos );

	TFLOAT fCentreX = a_pParticles->m_oTransform.m_f41;
	TFLOAT fCentreY = a_pParticles->m_oTransform.m_f42;

	for ( TINT i = a_iStart; i < a_iEnd; i++ )
	{
		TVector4& rPosition = a_pParticles->m_avPosition[ i ];
		TFLOAT    fX        = rPosition.x - fCentreX;
		TFLOAT    fY        = rPosition.y - fCentreY;
		rPosition.x         = fX;
		rPosition.y         = fY;
		rPosition.y         = fY * fCos + fX * fSin + fCentreY;
		rPosition.x         = ( fX * fCos - fY * fSin ) + fCentreX;
	}
}

// $deBlob: FUNCTION 0071fd70
void __stdcall TParticleAffectorRotateAroundEmitter::UpdateCustom( TGenericParticles* a_pParticles, TINT a_iStart, TINT a_iEnd, TFLOAT a_fDeltaTime, TParticleAffector::TRBHeader* a_pData )
{
	TASSERT( !"not implemented yet" );
}

TOSHI_NAMESPACE_END
