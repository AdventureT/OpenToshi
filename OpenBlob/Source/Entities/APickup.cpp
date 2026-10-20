#include "pch.h"
#include "APickup.h"
#include "AEntityParams.h"
#include "AApplication.h"
#include "Collision/ACollisionObject.h"
#include "Collision/ACollisionManager2.h"
#include "APickupManager.h"
#include "APickupData.h"
#include "APooledEntityManager.h"
#include "ABlobEntity/ABlobEntity.h"
#include "Render/AInstanceRenderer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TINT   APickup::ms_iNextRotationIndex;
TFLOAT APickup::ms_fUnk9866D4  = -1.0f;
TFLOAT APickup::ms_fShadowSize = 8.0f;

TSTATICASSERT( sizeof( APickup ) == 0x15C );

// $deBlob: FUNCTION 007b8970
// $deBlob: FUNCTION 007b8980
// $deBlob: FUNCTION 007b8990
// $deBlob: FUNCTION 007b89a0
// $deBlob: FUNCTION 007b89b0
// $deBlob: FUNCTION 00541770
// $deBlob: FUNCTION 0085a050
TDEFINE_CLASS_NORUNTIME( APickup );

// $deBlob: FUNCTION 007b89c0
APickup::APickup( TUINT8 a_uiPickupType, TINT a_iUnk2, TFLOAT a_fUnk3, TBOOL a_bUnk4 )
    : m_oShadow( ms_fShadowSize, 0 )
{
	m_vVelocity          = TVector4::VEC_ZERO;
	m_vSpawnPosition     = TVector4::VEC_ZERO;
	m_fRespawnTime       = -1.0f;
	m_fRespawnTimer      = 0.0f;
	m_fLifeTime          = -1.0f;
	m_fAlpha             = 1.0f;
	m_oListNode.m_pOwner = this;
	m_uiPickupType       = a_uiPickupType;
	m_pIgnoredEntity     = TNULL;
	m_pCollision         = TNULL;
	m_uiUnk134           = 0;
	m_uiUnk138           = 0;
	m_fDropDuration      = 0.0f;
	m_fHiddenTime        = 0.0f;
	m_pPooled            = TNULL;
	m_fnCallback         = TNULL;
	m_pCallbackUserData  = TNULL;
	m_iMesh              = -1;
	m_iMesh2             = -1;
	m_eState             = PICKUPSTATE_Idle;
	m_uiPickupFlags      = 0;
	m_oMatrix            = TMatrix44::IDENTITY;
	m_Flags |= FLAG_UpdateMatrix;

	m_uiRotationIndex = TUINT8( ms_iNextRotationIndex );
	ms_iNextRotationIndex++;
	if ( ms_iNextRotationIndex > 3 )
		ms_iNextRotationIndex = 0;

	m_oShadow.Init( ms_fShadowSize, 0 );
	m_oShadow.SetAlpha( 0.5f );
}

// An ARGB colour from 0-1 components
// $deBlob: FUNCTION 007b9d70
static TUINT32 MakeColour( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue, TFLOAT a_fAlpha )
{
	return TUINT32( TINT( a_fBlue * 255.0f ) ) |
	    ( ( TUINT32( TINT( a_fGreen * 255.0f ) ) | ( ( ( TUINT32( TINT( a_fAlpha * 255.0f ) ) << 8 ) | TUINT32( TINT( a_fRed * 255.0f ) ) ) << 8 ) ) << 8 );
}

// $deBlob: FUNCTION 007b8ba0
// $deBlob: FUNCTION 007b9ec0
// $deBlob: FUNCTION 007b9ea6
APickup::~APickup()
{
	if ( m_oListNode.IsLinked() && APickupManager::GetSingleton() != TNULL )
		APickupManager::GetSingleton()->RemovePickup( *this );

	if ( m_pCollision )
		delete m_pCollision;

	if ( m_iMesh != -1 )
	{
		AInstanceRenderer::GetInstance()->UnregisterModel( m_iMesh );
		m_iMesh = -1;
	}

	if ( m_iMesh2 != -1 )
	{
		AInstanceRenderer::GetInstance()->UnregisterModel( m_iMesh2 );
		m_iMesh2 = -1;
	}
}

// $deBlob: FUNCTION 007b8cc0
// $deBlob: FUNCTION 007b9f7c
void APickup::ProcessParams( const AEntityParams& a_rParams )
{
	ABaseEntity::ProcessParams( a_rParams );
	ATriggerUtil::ProcessParams( a_rParams );
	a_rParams.GetValue( m_fRespawnTime, "RespawnTime" );

	if ( !( m_uiPickupFlags & PICKUPFLAG_NoMesh ) )
	{
		const TCHAR* szMesh = TNULL;

		if ( m_iMesh == -1 && a_rParams.GetValue( szMesh, "Mesh" ) )
			m_iMesh = AInstanceRenderer::GetInstance()->RegisterModel( szMesh );

		const TCHAR* szMesh2 = TNULL;

		if ( m_iMesh2 == -1 && a_rParams.GetValue( szMesh2, "Mesh2" ) )
			m_iMesh2 = AInstanceRenderer::GetInstance()->RegisterModel( szMesh2 );
	}

	SetBBox( TVector4( 2.0f, 2.0f, 2.0f, 1.0f ) );
	GetCollisionObject()->SetFlags( ACollisionObject::FLAG_AABBDirty | ACollisionObject::FLAG_SphereDirty );

	TASSERT( APickupManager::IsSingletonCreated() );

	if ( APickupManager::IsSingletonCreated() )
		APickupManager::GetSingleton()->AddPickup( *this );
}

// $deBlob: FUNCTION 007b8df0
void APickup::InitPooled( TINT a_eType )
{
	m_fRespawnTime = -1.0f;
	SetBBox( TVector4( 2.0f, 2.0f, 2.0f, 1.0f ) );
	GetCollisionObject()->SetFlags( ACollisionObject::FLAG_AABBDirty | ACollisionObject::FLAG_SphereDirty );
}

// $deBlob: FUNCTION 007b8e40
void APickup::Create()
{
	ABaseEntity::Create();

	if ( GetCollisionObject() == TNULL )
		m_pCollision = new ACollisionObject( this, 1, 0xFF );

	// NOTE: the byte after the collision object's flags
	TREINTERPRETCAST( TUINT8*, m_pCollision )
	[0x11] = 0;
	SetVisible( TTRUE );
}

// $deBlob: FUNCTION 007b8ee0
// $deBlob: FUNCTION 0079ca73
void APickup::PostCreate()
{
	ATriggerUtil::PostCreate();
	m_Flags |= FLAG_Pickup;
	SetBBox( TVector4( 2.0f, 2.0f, 2.0f, 1.0f ) );
	m_vSpawnPosition = m_oMatrix.GetTranslation();
}

// $deBlob: FUNCTION 007b8f30
void APickup::OnDataLoad()
{
	TFLOAT fDistance = APickupData::GetSingleton()->GetData()->m_fUnk04;
	ms_fUnk9866D4    = fDistance * fDistance;
}

// $deBlob: FUNCTION 007b8f50
void APickup::PreRenderUpdate()
{
	if ( TUINT8( m_Flags >> 2 ) & 1 )
		m_oShadow.Update( m_oMatrix.GetTranslation(), m_oMatrix.AsBasisVector4( BASISVECTOR_FORWARD ), ( m_Flags & FLAG_UpdateMatrix ) != 0 );
}

// $deBlob: FUNCTION 007b8f80
void APickup::SetPosition( const TVector4& a_rPosition )
{
	m_Flags |= FLAG_UpdateMatrix;
	m_oMatrix.GetTranslation() = a_rPosition;
}

// $deBlob: FUNCTION 007b8fa0
void APickup::OnCollected( ABaseEntity* a_pCollector )
{
	TASSERT( ( TUINT8( m_Flags >> 2 ) & 1 ) != 0 );
	TASSERT( ( m_eState == PICKUPSTATE_Idle ) || ( m_eState == PICKUPSTATE_Fading ) || ( m_eState == PICKUPSTATE_Flashing ) );

	TriggerAllTargets( this );
	Hide();

	if ( m_fRespawnTime > 0.0f )
	{
		m_fRespawnTimer = m_fRespawnTime;
		m_eState        = PICKUPSTATE_Respawning;
		SetPosition( m_vSpawnPosition );
		return;
	}

	RemoveFromPhysics();
}

// Create without ABaseEntity::Create
// $deBlob: FUNCTION 007b9b20
void APickup::CreateCollision()
{
	if ( GetCollisionObject() == TNULL )
		m_pCollision = new ACollisionObject( this, 1, 0xFF );

	// NOTE: the byte after the collision object's flags
	TREINTERPRETCAST( TUINT8*, m_pCollision )
	[0x11] = 0;
	SetVisible( TTRUE );
}

// OnCollected without the asserts and the triggers
// $deBlob: FUNCTION 007b9bd0
void APickup::Despawn()
{
	Hide();

	if ( m_fRespawnTime > 0.0f )
	{
		m_fRespawnTimer = m_fRespawnTime;
		m_eState        = PICKUPSTATE_Respawning;
		SetPosition( m_vSpawnPosition );
		return;
	}

	RemoveFromPhysics();
}

// $deBlob: FUNCTION 007b90b0
void APickup::Tick( TFLOAT a_fDeltaTime )
{
	if ( !g_oTheApp.m_bGamePaused )
	{
		if ( m_eState == PICKUPSTATE_Dropping )
		{
			UpdateDrop( a_fDeltaTime );
		}
		else if ( m_eState == PICKUPSTATE_Respawning )
		{
			m_fRespawnTimer -= a_fDeltaTime;

			if ( m_fRespawnTimer < 0.0f )
				Show();

			goto TICK_BASE;
		}
		else if ( m_eState == PICKUPSTATE_Hidden )
		{
			m_fHiddenTime -= a_fDeltaTime;

			if ( m_fHiddenTime <= 0.0f )
				Show();

			goto TICK_BASE;
		}

		UpdateLifeTime( a_fDeltaTime );
	}

TICK_BASE:
	if ( GetLOD() == FLAG_HighLOD && ( m_Flags & FLAG_Visible ) )
		Render();

	ABaseEntity::Tick( a_fDeltaTime );
}

// $deBlob: FUNCTION 007b91e0
void APickup::Show()
{
	m_Flags &= ~FLAG_DontTick;
	m_vVelocity      = TVector4::VEC_ZERO;
	m_eState         = PICKUPSTATE_Idle;
	m_fRespawnTimer  = 0.0f;
	m_pIgnoredEntity = TNULL;
	m_uiUnk134       = 0;

	m_uiRotationIndex = TUINT8( ms_iNextRotationIndex );
	ms_iNextRotationIndex++;
	if ( ms_iNextRotationIndex > 3 )
		ms_iNextRotationIndex = 0;

	SetHash( TTRUE );
	SetVisible( TTRUE );
}

// $deBlob: FUNCTION 007b9260
void APickup::Hide()
{
	m_eState = PICKUPSTATE_Collected;

	if ( m_fnCallback != TNULL )
	{
		m_fnCallback( this, m_pCallbackUserData );
		m_fnCallback        = TNULL;
		m_pCallbackUserData = TNULL;
	}

	if ( m_fRespawnTime <= 0.0f )
	{
		m_Flags |= FLAG_DontTick;

		if ( TUINT8( m_Flags >> 8 ) & 1 )
			SetTicking( TFALSE );
	}

	SetHash( TFALSE );
	SetVisible( TFALSE );

	if ( TUINT8( m_Flags >> 23 ) & 1 )
	{
		APooledEntityManager::GetSingleton()->Release( TSTATICCAST( APooledEntity, m_pPooled ) );
		m_pPooled = TNULL;
		m_fAlpha  = 1.0f;
	}
}

// $deBlob: FUNCTION 007b9300
void APickup::SetBBox( TVector4 a_vBBox )
{
	m_pCollision->SetFlags( ACollisionObject::FLAG_AABBDirty | ACollisionObject::FLAG_SphereDirty );
	m_fBoundingSphereRadius = 6.0f;
	ABaseEntity::SetBBox( a_vBBox );
}

// $deBlob: FUNCTION 007b9330
TVector4 APickup::GetBasePosition() const
{
	TFLOAT          fHeight = -m_vBBox.y;
	const TVector4& vUp     = m_oMatrix.AsBasisVector4( BASISVECTOR_UP );
	const TVector4& vPos    = m_oMatrix.GetTranslation();

	return TVector4( vUp.x * fHeight + vPos.x, vUp.y * fHeight + vPos.y, vUp.z * fHeight + vPos.z, vPos.w );
}

// $deBlob: FUNCTION 007b9390
void APickup::SetVisible( TBOOL a_bVisible )
{

	if ( a_bVisible )
		m_Flags |= FLAG_Visible;
	else
		m_Flags &= ~FLAG_Visible;

	m_oShadow.SetHidden( !a_bVisible );
}

// $deBlob: FUNCTION 007b9400
void APickup::OnLODChanging( TINT a_iLOD )
{
	if ( a_iLOD & FLAG_LOD0 )
	{
		SetVisible( TFALSE );
		return;
	}

	switch ( m_eState )
	{
		case PICKUPSTATE_Idle:
		case PICKUPSTATE_Dropping:
		case PICKUPSTATE_Fading:
		case PICKUPSTATE_Flashing:
			SetVisible( TTRUE );
			return;
		default:
			return;
	}
}

// $deBlob: FUNCTION 007b9460
void APickup::Spawn( const TVector4& a_rPosition, TFLOAT a_fHiddenTime, TFLOAT a_fLifeTime )
{
	SetPosition( a_rPosition );
	m_fLifeTime   = a_fLifeTime;
	m_fAlpha      = 1.0f;
	m_fHiddenTime = a_fHiddenTime;

	if ( a_fHiddenTime > 0.0f )
	{
		m_eState = PICKUPSTATE_Hidden;
		SetVisible( TFALSE );
	}
	else
	{
		Show();
	}

	if ( m_fHiddenTime > 0.0f || m_fLifeTime > 0.0f )
	{
		m_Flags &= ~FLAG_DontTick;
		SetTicking( TTRUE );
	}

	APickupManager::GetSingleton()->AddPickup( *this );
}

// $deBlob: FUNCTION 007b94f0
void APickup::Reset( ABaseEntity* a_pEntity )
{
	m_fAlpha = 1.0f;
	m_Flags  = ( m_Flags & ~( FLAG_MediumLOD | FLAG_HighLOD | 0x1000 ) ) | FLAG_LOD0;
	Show();
}

// $deBlob: FUNCTION 007b9520
void APickup::Drop( ABaseEntity* a_pDropper, const TVector4& a_rVelocity )
{
	Show();
	m_pIgnoredEntity = a_pDropper;
	m_vVelocity      = a_rVelocity;
	m_fDropTime      = -1.0f;
	TFLOAT fDropTime = APickupData::GetSingleton()->GetData()->m_fDropTime;
	m_uiPickupFlags &= ~PICKUPFLAG_DropTimeout;
	m_Flags &= ~FLAG_DontTick;
	m_fDropDuration = fDropTime;
	m_eState        = PICKUPSTATE_Dropping;
	SetTicking( TTRUE );
}

// $deBlob: FUNCTION 007b9590
void APickup::DropTimed( const TVector4& a_rVelocity, TFLOAT a_fDropTime, ABaseEntity* a_pIgnored )
{
	Show();
	m_pIgnoredEntity = a_pIgnored;
	m_vVelocity      = a_rVelocity;
	m_eState         = PICKUPSTATE_Dropping;
	m_fDropTime      = a_fDropTime;

	if ( a_fDropTime <= 0.0f )
	{
		m_uiPickupFlags &= ~PICKUPFLAG_DropTimeout;
		m_fDropDuration = APickupData::GetSingleton()->GetData()->m_fDropTime;
	}
	else
	{
		m_uiPickupFlags |= PICKUPFLAG_DropTimeout;
		m_fDropDuration = a_fDropTime;
	}

	m_Flags &= ~FLAG_DontTick;
	SetTicking( TTRUE );
}

// $deBlob: FUNCTION 007b9620
void APickup::Fly( const TVector4& a_rFrom, const TVector4& a_rTo, TFLOAT a_fAngle, TINT a_iUnk )
{
	if ( TVector4::Distance( a_rFrom, a_rTo ) < 0.25f )
		return;

	TFLOAT fAngle = a_fAngle;

	if ( a_fAngle < 0.0f || a_fAngle > 1.5707964f )
	{
		// Half way between the elevation of the target (y is down) and straight up
		TVector4 vHorizontal( a_rTo.x - a_rFrom.x, 0.0f, a_rTo.z - a_rFrom.z, a_rTo.w );
		TFLOAT   fElevation = TMath::ATan2( -( a_rTo.y - a_rFrom.y ), vHorizontal.Magnitude() );
		fElevation          = TMath::Max( fElevation, 0.0f );
		fAngle              = ( 1.5707964f - fElevation ) * 0.5f + fElevation;
	}

	TVector4 vVelocity;
	TFLOAT   fFlightTime = ABlobEntity::CalculateJumpVelocity( fAngle, a_rFrom, a_rTo, 80.0f, vVelocity );
	DropTimed( vVelocity, fFlightTime, TNULL );
	SetPosition( a_rFrom );
}

// $deBlob: FUNCTION 007b9760
void APickup::OnPoolRelease()
{

	if ( m_fnCallback != TNULL )
	{
		m_fnCallback( this, m_pCallbackUserData );
		m_fnCallback        = TNULL;
		m_pCallbackUserData = TNULL;
	}

	APickupManager::GetSingleton()->RemovePickup( *this );
}

// $deBlob: FUNCTION 007b97a0
void APickup::UpdateDrop( TFLOAT a_fDeltaTime )
{
	// A timed drop doesn't fly past its end
	if ( ( m_uiPickupFlags & PICKUPFLAG_DropTimeout ) && m_fDropTime <= a_fDeltaTime )
		a_fDeltaTime = m_fDropTime;

	// The new position (the gravity, 80 per second squared, is +y)
	TVector4 vPosition;
	vPosition.Multiply( m_vVelocity, a_fDeltaTime );
	vPosition.x = m_oMatrix.m_f41 + vPosition.x;
	vPosition.z = m_oMatrix.m_f43 + vPosition.z;
	vPosition.y = 80.0f * 0.5f * a_fDeltaTime * a_fDeltaTime + m_oMatrix.m_f42 + vPosition.y;

	if ( !( m_uiPickupFlags & PICKUPFLAG_DropTimeout ) )
	{
		// Swept to the new position: bounces (or lands when slow)
		TVector4 vMove( vPosition.x - m_oMatrix.m_f41, vPosition.y - m_oMatrix.m_f42, vPosition.z - m_oMatrix.m_f43, vPosition.w );
		TVector4 vNormal   = TVector4::VEC_ZERO;
		TFLOAT   fDistance = vMove.NormaliseMag();
		TFLOAT   fStep;

		APickupCollisionFunctor oFunctor( this );
		ACollisionObject*       pIgnore = ( m_pIgnoredEntity != TNULL ) ? m_pIgnoredEntity->GetCollisionObject() : TNULL;

		if ( ACollisionManager2::GetSingleton()->CollideSweptAABB( GetBasePosition(), m_vBBox, vMove, fDistance, vNormal, 0.0f, fStep, pIgnore, &oFunctor, 0x83, TNULL ) != TNULL )
		{
			if ( m_vVelocity.Magnitude() > 1.5f )
			{
				TASSERT( vNormal.Magnitude() > 0 );

				TVector4 vReflect = vNormal;
				vReflect.Multiply( ( m_vVelocity.y * vNormal.y + m_vVelocity.x * vNormal.x + m_vVelocity.z * vNormal.z ) * 2.0f );
				m_vVelocity.x -= vReflect.x;
				m_vVelocity.y -= vReflect.y;
				m_vVelocity.z -= vReflect.z;
				m_vVelocity.Multiply( 0.4f );
			}
			else
			{
				m_uiPickupFlags &= ~PICKUPFLAG_DropTimeout;
				m_vVelocity = TVector4::VEC_ZERO;

				if ( m_fHiddenTime <= 0.0f && m_fLifeTime <= 0.0f )
					SetTicking( TFALSE );

				OnDropFinished();
				m_eState = PICKUPSTATE_Idle;
			}
		}
	}

	m_oMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) = vPosition;
	m_Flags |= FLAG_UpdateMatrix;
	m_vVelocity.y = 80.0f * a_fDeltaTime + m_vVelocity.y;

	if ( m_uiPickupFlags & PICKUPFLAG_DropTimeout )
	{
		m_fDropTime -= a_fDeltaTime;

		if ( m_fDropTime <= 0.0f )
		{
			m_uiPickupFlags &= ~PICKUPFLAG_DropTimeout;
			m_vVelocity = TVector4::VEC_ZERO;

			if ( m_fHiddenTime <= 0.0f && m_fLifeTime <= 0.0f )
				SetTicking( TFALSE );

			OnDropFinished();
			m_eState = PICKUPSTATE_Idle;
		}
	}
}

// $deBlob: FUNCTION 007b9f40
TBOOL APickupCollisionFunctor::HitIgnorableObject( ACollisionObject* a_pObject )
{
	return a_pObject->GetOwner() == m_pPickup->GetIgnoredEntity();
}

// $deBlob: FUNCTION 007b9c20
void APickup::UpdateLifeTime( TFLOAT a_fDeltaTime )
{
	switch ( m_eState )
	{
		case PICKUPSTATE_Idle:
			if ( m_fLifeTime > 0.0f )
			{
				m_fLifeTime -= a_fDeltaTime;

				if ( m_fLifeTime <= 1.0f )
					m_eState = PICKUPSTATE_Flashing;
			}
			break;
		case PICKUPSTATE_Flashing:
			m_fLifeTime -= a_fDeltaTime;

			if ( m_fLifeTime <= 0.0f )
			{
				Hide();
				return;
			}

			m_fAlpha = m_fLifeTime;
			break;
	}
}

// $deBlob: FUNCTION 007b9cd0
void APickup::Render()
{
	if ( m_iMesh != -1 )
		AInstanceRenderer::GetInstance()->SubmitForBatchRender( m_iMesh, APickupManager::GetSingleton()->GetPickupRotation( m_uiRotationIndex, TFALSE ), &m_oMatrix.GetTranslation(), m_fAlpha, 0xFFFFFFFF );

	if ( m_iMesh2 != -1 )
		AInstanceRenderer::GetInstance()->SubmitForBatchRender( m_iMesh2, APickupManager::GetSingleton()->GetPickupRotation( m_uiRotationIndex, TTRUE ), &m_oMatrix.GetTranslation(), m_fAlpha, 0xFFFFFFFF );
}

// $deBlob: FUNCTION 0079ca60
ACollisionObject* APickup::GetCollisionObject()
{
	return m_pCollision;
}

// $deBlob: FUNCTION 0079ca70
void* APickup::GetModelComponent()
{
	return TNULL;
}

// $deBlob: FUNCTION 0079ca80
void APickup::OnDropFinished()
{
}
