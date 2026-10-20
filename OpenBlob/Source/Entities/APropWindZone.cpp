#include "pch.h"
#include "Sound/APositionalSoundManager.h"
#include "Sound/ASoundInterface.h"
#include "AEntityFactory.h"
#include "APropWindZone.h"
#include "AEntityLoader.h"
#include "AEntityParams.h"
#include "AEntityIOPacket.h"
#include "ABlobEntity/ABlobEntity.h"

#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TParticleEffectInstance.h>
#include <Math/TOBBox.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APropWindZone ) == 0x110 );

// The strength of the wind by m_uiType (ProcessParams sets them from the properties once)
static TFLOAT s_afWindStrength[ 4 ] = { 450000000.0f, 900000000.0f, 1350000000.0f, 0.0f };
static TBOOL  s_bUnk985FF4          = TTRUE;
static TBOOL  s_bWindStrengthSet    = TFALSE;

// The wind sent to the entities in the zone (AEntityIOPacket::MESSAGE_WIND)
struct WindForce
{
	TVector4 vForce;         // 0x00
	TUINT8   aUnk10[ 0x10 ]; // 0x10
	TINT     iType;          // 0x20 APropWindZone's m_uiType
	TBOOL    bUnk24;         // 0x24 cleared for TYPE_Target, otherwise not set
	TVector4 vDirection;     // 0x28 the -Z axis of the zone
};

// $deBlob: FUNCTION 007a2470
// $deBlob: FUNCTION 007a2480
// $deBlob: FUNCTION 007a24f0
// $deBlob: FUNCTION 00541020
// $deBlob: FUNCTION 00859e90
TDEFINE_CLASS( APropWindZone );

// $deBlob: FUNCTION 007a2570
APropWindZone::APropWindZone()
{
	m_uiWindFlags        = WINDFLAG_On | WINDFLAG_HasEntities;
	m_uiType             = 2;
	m_bWindOn            = TTRUE;
	m_bTargetOnExitFront = TTRUE;
	m_fOnDuration        = -1.0f;
	m_fOffDuration       = 0.0f;
	m_fTimer             = 0.0f;
	m_pParticleInstance  = TNULL;
	m_pParticleEffect    = TNULL;
	m_iRequiredSize      = -1;
	m_vTargetBase        = TVector4::VEC_ZERO;
	m_vTargetPos         = TVector4::VEC_ZERO;
}

// $deBlob: FUNCTION 007a2600
// $deBlob: FUNCTION 007a35a0
// $deBlob: FUNCTION 007a358b
APropWindZone::~APropWindZone()
{
	StopParticles();
	m_vecEntities.Clear();
}

// The finished callback of m_pParticleInstance
// $deBlob: FUNCTION 007a2ec0
void APropWindZone::OnParticlesFinished( TParticleEffectInstance* a_pInstance, void* a_pUserData )
{
	TSTATICCAST( APropWindZone, a_pUserData )->m_pParticleInstance = TNULL;
}

// Starts "Particle" (if it isn't playing) at the trigger's matrix, rotated to blow along it
// $deBlob: FUNCTION 007a3230
void APropWindZone::StartParticles()
{
	if ( m_pParticleEffect == TNULL || m_pParticleInstance != TNULL )
		return;

	m_pParticleInstance = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( m_pParticleEffect );

	if ( m_pParticleInstance != TNULL )
	{
		TMatrix44 oMatrix = m_pPhysicsData->m_oMatrix;
		oMatrix.RotateX( 1.5707964f );
		m_pParticleInstance->GetTransform() = oMatrix;
		m_pParticleInstance->SetFinishedCallback( OnParticlesFinished, this );
	}
}

// $deBlob: FUNCTION 007a3300
void APropWindZone::StopParticles()
{
	if ( m_pParticleInstance )
	{
		m_pParticleInstance->SetFinishedCallback( TNULL, TNULL );
		m_pParticleInstance->Stop();
		m_pParticleInstance = TNULL;
	}
}

// $deBlob: FUNCTION 007a2930
void APropWindZone::Tick( TFLOAT a_fDeltaTime )
{
	if ( ( m_uiWindFlags & WINDFLAG_On ) && GetLOD() != FLAG_LowLOD && GetLOD() == FLAG_HighLOD )
	{
		if ( !( m_uiWindFlags & WINDFLAG_AlwaysOn ) )
		{
			m_fTimer -= a_fDeltaTime;

			if ( m_fTimer < 0.0f )
			{
				if ( m_uiWindFlags & WINDFLAG_Blowing )
				{
					m_uiWindFlags &= ~WINDFLAG_Blowing;
					m_fTimer = m_fOffDuration;
					StopParticles();
				}
				else
				{
					m_uiWindFlags |= WINDFLAG_Blowing;
					m_fTimer = m_fOnDuration;
					StartParticles();
				}
			}
		}

		if ( ( m_uiWindFlags & ( WINDFLAG_HasEntities | WINDFLAG_Blowing ) ) == ( WINDFLAG_HasEntities | WINDFLAG_Blowing ) )
		{
			const TMatrix44& rcMatrix = m_pPhysicsData->m_oMatrix;

			WindForce oForce;
			oForce.vDirection = rcMatrix.AsBasisVector4( BASISVECTOR_FORWARD );

			TOBBox oBox( rcMatrix, GetBBox() );
			oForce.vDirection.Multiply( -1.0f );
			oForce.iType = m_uiType;

			AEntityIOPacket oPacket;
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE_WIND;
			oPacket.m_iData    = TREINTERPRETCAST( TINT, &oForce );

			// NOTE: an entity removed here moves the next one into its slot, which ++ then skips (as in retail)
			for ( auto it = m_vecEntities.Begin(); it != m_vecEntities.End(); ++it )
			{
				ABaseEntity* pEntity = *it;

				if ( pEntity->GetClass() == &TGetClass( ABlobEntity ) )
				{
					ABlobEntity* pBlob = TSTATICCAST( ABlobEntity, pEntity );

					if ( m_iRequiredSize > -1 && pBlob->GetSize() != m_iRequiredSize )
						continue;

					const TVector4& rcPosition = pBlob->GetBodyPosition();
					TSphere         oSphere( rcPosition.x, rcPosition.y, rcPosition.z, pBlob->GetBoundingSphereRadius() );

					if ( !TOBBox::Intersects( oBox, oSphere ) )
					{
						if ( s_bUnk985FF4 )
							UpdateEntity( pBlob, TFALSE );

						continue;
					}
				}

				if ( m_uiType == TYPE_Target )
				{
					oForce.vForce.Multiply( oForce.vDirection, ABlobEntity::GetCommonProperties()->m_afUnk38[ 3 ] ); // 0x44
					oForce.bUnk24 = TFALSE;
				}
				else
				{
					oForce.vForce = oForce.vDirection;
					oForce.vForce.Multiply( s_afWindStrength[ m_uiType ] * a_fDeltaTime );
				}

				pEntity->HandleEntityIO( this, oPacket );
			}
		}
	}

	APropTrigger::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 007a2e30
void APropWindZone::OnLODChanging( TINT a_iLOD )
{
	if ( m_uiWindFlags & WINDFLAG_On )
	{
		if ( m_uiWindFlags & WINDFLAG_Blowing )
		{
			if ( a_iLOD == FLAG_HighLOD )
			{
				StartParticles();
			}
			else
			{
				StopParticles();
			}

			return;
		}

		StopParticles();
	}
}

// $deBlob: FUNCTION 007a2ea0
void APropWindZone::Destroy()
{
	APositionalSoundManager::GetSingleton()->RemoveSound( this, 5, 0x6b );

	ABaseEntity::Destroy();
}

// $deBlob: FUNCTION 007a26c0
// $deBlob: FUNCTION 007a36e2
void APropWindZone::ProcessParams( const AEntityParams& a_rParams )
{
	APropTrigger::ProcessParams( a_rParams );

	a_rParams.GetValue( m_uiType, "Type" );

	if ( m_uiType == TYPE_Target )
	{
		a_rParams.GetValue( m_vTargetPos, "TargetPos" );

		const AEntityLoader::EntityData* pData   = TSTATICCAST( const AEntityLoader::EntityData, &a_rParams );
		const TMatrix44&                 rMatrix = *pData->m_pMatrix;
		const TVector4&                  rBBox   = *pData->m_pBBox;

		m_vTargetBase.x = rMatrix.m_f41 + rMatrix.m_f31 * -rBBox.z;
		m_vTargetBase.y = rMatrix.m_f42 + rMatrix.m_f32 * -rBBox.z;
		m_vTargetBase.z = rMatrix.m_f43 + rMatrix.m_f33 * -rBBox.z;
		m_vTargetBase.w = rMatrix.m_f44;
		m_vTargetBase.x += rMatrix.m_f21 * -rBBox.y;
		m_vTargetBase.y += rMatrix.m_f22 * -rBBox.y;
		m_vTargetBase.z += rMatrix.m_f23 * -rBBox.y;
	}

	a_rParams.GetValue( m_bWindOn, "WindOn" );

	if ( !m_bWindOn )
		m_uiWindFlags &= ~WINDFLAG_On;
	else
		m_uiWindFlags |= WINDFLAG_On;

	a_rParams.GetValue( m_bTargetOnExitFront, "TargetOnExitFront" );
	a_rParams.GetValue( m_fOnDuration, "OnDuration" );
	a_rParams.GetValue( m_fOffDuration, "OffDuration" );

	if ( m_fOnDuration < 0.0f )
		m_uiWindFlags |= WINDFLAG_Blowing | WINDFLAG_AlwaysOn;

	const TCHAR* szParticle = TNULL;
	a_rParams.GetValue( szParticle, "Particle" );

	if ( szParticle )
	{
		m_pParticleEffect = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( szParticle );
		TASSERT( m_pParticleEffect != TNULL );
	}

	a_rParams.GetValue( m_iRequiredSize, "RequiredSize" );

	// The wind strengths come from the type data (its physics fields) the first time
	if ( !s_bWindStrengthSet )
	{
		s_afWindStrength[ 3 ] = 0.0f;
		s_bWindStrengthSet    = TTRUE;

		const AEntityTypeData::TRBData* pTRBData = GetTypeData()->m_pTRBData;
		s_afWindStrength[ 0 ]                    = pTRBData->m_fLinearDamping * 9e+07f;
		s_afWindStrength[ 1 ]                    = *TREINTERPRETCAST( const TFLOAT*, &pTRBData->m_aUnk10[ 0 ] ) * 9e+07f;
		s_afWindStrength[ 2 ]                    = pTRBData->m_fMass * 9e+07f;
	}
}

// $deBlob: FUNCTION 007a2b80
void APropWindZone::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_TRIGGER:
		case AEntityIOPacket::MESSAGE_TOGGLE:
			// Toggles the wind
			SetWindOn( ( m_uiWindFlags & WINDFLAG_On ) == 0 );
			break;
		case AEntityIOPacket::MESSAGE_TOUCH:
			UpdateEntity( a_pCaller, TTRUE );
			break;
		case AEntityIOPacket::MESSAGE_UNTOUCH:
			UpdateEntity( a_pCaller, TFALSE );

			// A target zone throws the entity leaving it to "TargetPos" (a blob only at "RequiredSize")
			if ( ( m_uiWindFlags & WINDFLAG_On ) && m_uiType == TYPE_Target &&
			     ( m_iRequiredSize < 0 || a_pCaller->GetClass() != &TGetClass( ABlobEntity ) || TSTATICCAST( ABlobEntity, a_pCaller )->GetSize() == m_iRequiredSize ) )
			{
				const TVector4& rcForward = m_pPhysicsData->m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
				TVector4        vBack( -rcForward.x, -rcForward.y, -rcForward.z, rcForward.w );

				if ( m_bTargetOnExitFront )
				{
					// Only when it leaves through the front
					const TVector4& rcPosition = a_pCaller->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );
					TVector4        vToEntity( rcPosition.x - m_vTargetBase.x, rcPosition.y - m_vTargetBase.y, rcPosition.z - m_vTargetBase.z, rcPosition.w );
					vToEntity.Normalise();

					if ( vToEntity.x * vBack.x + vToEntity.y * vBack.y + vToEntity.z * vBack.z < 0.0f )
						break;
				}

				// The throw (ABlobEntity's MESSAGE_WIND): the velocity (0x00), the start (0x10), the type (0x20) and
				// whether to move there first (0x24)
				struct
				{
					TVector4 vVelocity;
					TVector4 vPosition;
					TINT     iType;
					TBOOL    bMove;
				} oThrow;

				oThrow.iType = m_uiType;
				vBack.Multiply( a_pCaller->GetBBox().Magnitude() );
				oThrow.bMove       = TTRUE;
				oThrow.vPosition.x = m_vTargetBase.x + vBack.x;
				oThrow.vPosition.y = m_vTargetBase.y + vBack.y;
				oThrow.vPosition.z = m_vTargetBase.z + vBack.z;
				oThrow.vPosition.w = m_vTargetBase.w;
				ABlobEntity::CalculateJumpVelocityAtSpeed( oThrow.vPosition, m_vTargetPos, ABlobEntity::GetCommonProperties()->m_afUnk38[ 3 ], 80.0f, oThrow.vVelocity ); // 0x44

				AEntityIOPacket oPacket;
				oPacket.m_eMessage = AEntityIOPacket::MESSAGE_WIND;
				oPacket.m_iData    = TREINTERPRETCAST( TINT, &oThrow );
				a_pCaller->HandleEntityIO( this, oPacket );
			}

			break;
		case AEntityIOPacket::MESSAGE_CHALLENGEINIT:
		case AEntityIOPacket::MESSAGE_CHALLENGERESET:
			m_uiWindFlags &= ~WINDFLAG_On;
			StopParticles();
			break;
		case AEntityIOPacket::MESSAGE_CHALLENGECOMPLETE:
			SetWindOn( TTRUE );
			break;
	}

	APropTrigger::HandleEntityIO( a_pCaller, a_rPacket );
}

// Switches the wind on (with its sound, and the particles at the high LOD) or off
// $deBlob: FUNCTION 007a2ee0
void APropWindZone::SetWindOn( TBOOL a_bOn )
{
	if ( a_bOn )
	{
		ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( 5 ), ASoundMap::EVENTID( 0x6C ), GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ), 0 );
		m_uiWindFlags |= WINDFLAG_On;

		if ( GetLOD() == FLAG_HighLOD )
			StartParticles();
	}
	else
	{
		m_uiWindFlags &= ~WINDFLAG_On;
		StopParticles();
	}
}

// While the wind is on: adds an entity (with the flag 0x10) that entered the zone to m_vecEntities (once, up to
// MAX_ENTITIES) or removes one that left it
// $deBlob: FUNCTION 007a3000
void APropWindZone::UpdateEntity( ABaseEntity* a_pEntity, TBOOL a_bEntered )
{
	if ( a_pEntity == TNULL || ( a_pEntity->GetFlags() & 0x10 ) == 0 || ( m_uiWindFlags & WINDFLAG_On ) == 0 )
		return;

	if ( a_bEntered )
	{
		if ( m_vecEntities.Size() < MAX_ENTITIES )
		{
			if ( FindEntity( a_pEntity ) != m_vecEntities.End() )
				return;

			m_vecEntities.PushBack( a_pEntity );
		}
	}
	else
	{
		auto it = FindEntity( a_pEntity );

		if ( it != m_vecEntities.End() )
			m_vecEntities.Erase( it );
	}

	if ( m_vecEntities.Size() != 0 )
		m_uiWindFlags |= WINDFLAG_HasEntities;
	else
		m_uiWindFlags &= ~WINDFLAG_HasEntities;
}

// $deBlob: FUNCTION 007a30e0
T2Vector<ABaseEntity*, APropWindZone::MAX_ENTITIES>::Iterator APropWindZone::FindEntity( ABaseEntity* a_pEntity )
{
	return m_vecEntities.Find( a_pEntity );
}

// $deBlob: FUNCTION 007a2f60
void APropWindZone::Reset( ABaseEntity* a_pEntity )
{
	StopParticles();
	m_vecEntities.Clear();

	if ( m_bWindOn )
		m_uiWindFlags |= WINDFLAG_On;
	else
		m_uiWindFlags &= ~WINDFLAG_On;

	APropTrigger::Reset( a_pEntity );
}
