#include "pch.h"
#include "ACritter.h"
#include "AEntityParams.h"
#include "AEntityIOPacket.h"
#include "AEntityModelComponent.h"
#include "ABlobEntity/ABlobColourComponent.h"
#include "ABlobEntity/ABlobColourMaps.h"
#include "Animation/AModelInstance2.h"
#include "Animation/AModel2.h"
#include "Collision/ACollisionObject.h"
#include "GameController/AGameStatus.h"
#include "AEntityHash.h"
#include "ABlobEntity/ABlobEntity.h"

#include <Render/TModel.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

const TINT   ACritter::ms_aNormalStates[ Max ]   = { 0, 2, 4 };
const TINT   ACritter::ms_aSquashedStates[ Max ] = { 1, 3, 5 };
const TFLOAT ACritter::ms_fScale                 = 1.25f;

TSTATICASSERT( sizeof( ACritter ) == 0xA4 );

// The critters around Blob (SquashCritters' local AEntityHashFilterFunctorCritterOnly)
class ACritterHashFilter : public AEntityHashFilter
{
public:
	// $deBlob: FUNCTION 007abfc0
	virtual TBOOL operator()( ABaseEntity* a_pEntity ) override
	{
		return a_pEntity->GetClass() == &TGetClass( ACritter );
	}
};

// $deBlob: FUNCTION 007ab6e0
// $deBlob: FUNCTION 007ab6f0
// $deBlob: FUNCTION 007ab790
// $deBlob: FUNCTION 007ab830
// $deBlob: FUNCTION 007ab840
// $deBlob: FUNCTION 005412e0
// $deBlob: FUNCTION 00859f40
TDEFINE_CLASS( ACritter );

// $deBlob: FUNCTION 007ab850
ACritter::ACritter()
{
	m_pCollision     = TNULL;
	m_uiCritterFlags = CRITTERFLAG_StartInvisible;
	SetVisible( TFALSE );
	SetHash( TFALSE );
	m_Flags |= 0x80000;
}

// $deBlob: FUNCTION 007ac000
ACritter::~ACritter()
{
}

// $deBlob: FUNCTION 007ab8d0
void ACritter::ProcessParams( const AEntityParams& a_rParams )
{
	const TCHAR* szMesh = TNULL;
	a_rParams.GetValue( szMesh, "Mesh" );

	TCHAR iType = 0;
	a_rParams.GetChar( iType, "CritterType" );
	TASSERT( iType >= 0 && iType < Max );

	m_uiCritterFlags |= iType;
	SetState( ms_aNormalStates[ GetType() ] );

	TBOOL bStartInvisible = TTRUE;
	a_rParams.GetValue( bStartInvisible, "Start Invisible" );

	if ( !bStartInvisible )
		m_uiCritterFlags &= ~CRITTERFLAG_StartInvisible;

	m_Flags &= ~0x80000;
	CreateModelComponent( szMesh );
	CreatePoseController();
	CreateStateComponent();
	CreateFXBreakpointComponent( szMesh );
	AVisibleEntity::ProcessParams( a_rParams );
	SetVisible( !bStartInvisible );
	SetHash( ( m_Flags & FLAG_Visible ) != 0 );
}

// $deBlob: FUNCTION 007ab9e0
void ACritter::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_iState == ms_aSquashedStates[ GetType() ] && HasStateFinished() )
		SetState( ms_aNormalStates[ GetType() ] );

	AVisibleEntity::Tick( a_fDeltaTime );

	TMatrix44 mTransform = m_oMatrix;
	mTransform.Scale( ms_fScale, ms_fScale, ms_fScale );
	TSTATICCAST( AEntityModelComponent, GetModelComponent() )->SetTransform( mTransform );
}

// $deBlob: FUNCTION 007abaa0
void ACritter::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	AVisibleEntity::HandleEntityIO( a_pCaller, a_rPacket );

	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_TRIGGER:
		case AEntityIOPacket::MESSAGE_TOGGLE:
			if ( !( TUINT8( m_Flags >> 2 ) & 1 ) )
			{
				// The score of a squashed critter (BlobScoringProperties+0x7C, not ported as a struct yet)
				TVector4 vPosition = m_oMatrix.GetTranslation();
				vPosition.y -= m_vBBox.y * 2.0f;
				AGameStatus::GetSingleton()->AddBonusScore( 0, TREINTERPRETCAST( TINT*, AGameStatus::GetBlobScoringProperties() )[ 0x7C / 4 ], &vPosition );
			}

			SetVisible( TTRUE );
			SetHash( ( TUINT8( m_Flags >> 2 ) & 1 ) != 0 );
			SetState( ms_aSquashedStates[ GetType() ] );
			return;
		case AEntityIOPacket::MESSAGE_TOUCH:
		case AEntityIOPacket::MESSAGE_TOUCHEDBY:
			if ( m_iState != ms_aSquashedStates[ GetType() ] )
				SetState( ms_aSquashedStates[ GetType() ] );
	}
}

// $deBlob: FUNCTION 007abbc0
void ACritter::Reset( ABaseEntity* a_pEntity )
{
	AVisibleEntity::Reset( a_pEntity );
	SetVisible( !( m_uiCritterFlags & CRITTERFLAG_StartInvisible ) );
	SetHash( ( m_Flags & FLAG_Visible ) != 0 );
	SetState( ms_aNormalStates[ GetType() ] );
	SetupModel();
}

// $deBlob: FUNCTION 007abc20
void ACritter::PostCreate()
{
	CreateCollision();
	SetupModel();
}

// $deBlob: FUNCTION 007abc30
void ACritter::Destroy()
{
	if ( m_pCollision )
		delete m_pCollision;

	m_pCollision = TNULL;
	ABaseEntity::Destroy();
}

// $deBlob: FUNCTION 007abc60
void ACritter::CreateCollision()
{
	TASSERT( m_pCollision == TNULL );
	m_pCollision = new ACollisionObject( this, 1, 0xFF );
	m_pCollision->SetFlags( ACollisionObject::FLAG_AABBDirty | ACollisionObject::FLAG_SphereDirty );
	m_pCollision->SetFlags( ACollisionObject::FLAG_Created );
	*TREINTERPRETCAST( TUINT8*, &m_iCollisionGroupAndType ) = 0x20;
}

// Blob squashes the critters it lands on
// $deBlob: FUNCTION 007abd20
void ACritter::SquashCritters( ABlobEntity* a_pBlob )
{
	const TVector4& rvPosition = a_pBlob->GetBodyPosition();
	TFLOAT          fRadius    = ABlobEntity::GetProperties( a_pBlob->GetSize() )->m_fRadius;
	TFLOAT          fRangeSq   = ( fRadius + 1.0f ) * ( fRadius + 1.0f );

	ACritterHashFilter oFilter;
	TINT               iCount;
	ABaseEntity**      ppEntities = AEntityHash::GetSingleton()->Query( rvPosition, fRadius - 1.0f, &iCount, &oFilter, TTRUE, 500, TFALSE, TTRUE, TFALSE );

	// The assert is once per process (retail's dword flag)
	static TBOOL s_bAssertedExactly = TFALSE;

	for ( TUINT i = 0; i < TUINT( iCount ); i++ )
	{
		ABaseEntity* pEntity = ppEntities[ i ];

		if ( !s_bAssertedExactly && !pEntity->IsExactly( &TGetClass( ACritter ) ) )
		{
			TASSERT( pEntity->IsExactly( &TGetClass( ACritter ) ) );
			s_bAssertedExactly = TTRUE;
		}

		if ( TVector4::DistanceSq( rvPosition, pEntity->GetMatrix().GetTranslation() ) < fRangeSq )
		{
			AEntityIOPacket oPacket;
			oPacket.m_eMessage = AEntityIOPacket::MESSAGE_TOUCHEDBY;
			oPacket.m_iData    = 0;
			pEntity->HandleEntityIO( a_pBlob, oPacket );
		}
	}
}

// $deBlob: FUNCTION 007abe30
void ACritter::SetupModel()
{
	AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();

	pModelInstance->GetModel()->GetModelPtr().GetData()->GetLOD( 0 );
	TASSERT( 1 == pModelInstance->GetModel()->GetModelPtr().GetData()->GetLODCount() );

	pModelInstance->SetFlag( 0x1000, TTRUE );
	pModelInstance->SetMeshFlags( 1, 1, TFALSE );
	pModelInstance->UseLocalAmbientLight( TTRUE );

	TINT iColour = ABlobColourComponent::GetRandomColour();
	pModelInstance->SetAmbientColour( ABlobColourMaps::ms_pEnemyLightColourMap[ iColour ] );

	pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
	pModelInstance->SetMeshFlags( 0, 1, TTRUE );
	pModelInstance->SetMeshColour( 0, ABlobColourMaps::ms_pEnemyBodyColourMap[ iColour ] );
}

// $deBlob: FUNCTION 007ac030
ACollisionObject* ACritter::GetCollisionObject()
{
	return m_pCollision;
}

// $deBlob: FUNCTION 007ac050
const TCHAR* ACritter::GetModelName()
{
	return TNULL;
}
