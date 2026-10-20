#include "pch.h"
#include "AIOMeshComponent.h"
#include "IO/AIOEntity.h"
#include "Entities/AEntityModelComponent.h"
#include "Animation/AModelInstance2.h"

#include <Toshi/TString8.h>
#include <Render/TModel.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 007ccd50
AIOMeshComponent::AIOMeshComponent( AIOEntity* a_pOwner )
    : AIOEntityComponent( a_pOwner, TYPE )
{
	m_oLocalMatrix    = TMatrix44::IDENTITY;
	m_pModelComponent = TNULL;
}

// $deBlob: FUNCTION 007cd1d0
// $deBlob: FUNCTION 007ccdb0
AIOMeshComponent::~AIOMeshComponent()
{
	if ( m_pModelComponent != TNULL )
		delete m_pModelComponent;
}

// $deBlob: FUNCTION 007cce20
void AIOMeshComponent::SetIOEntityData( void* a_pData, TTRB* a_pTRB )
{
	IOEntityData* pIOData = TSTATICCAST( IOEntityData, a_pData );

	TBOOL bCorrectType = pIOData->iType == TYPE;
	TASSERT( TTRUE == bCorrectType );

	Data* pData   = pIOData->pData;
	m_szModelName = pData->szModelName;
	m_pPosition   = pData->pPosition;
	m_pRotation   = pData->pRotation;
	m_pScale      = pData->pScale;

	if ( m_szModelName != TNULL )
	{
		TASSERT( TNULL == m_pModelComponent );

		TString8 strFileName;
		strFileName.Format( "%s.trb", m_szModelName );

		m_pModelComponent = new AEntityModelComponent( m_pOwner, strFileName.GetString() );

		// Z up to Y up
		m_pModelComponent->GetModelInstance()->SetScale( TVector4( m_pScale->x, m_pScale->z, m_pScale->y, 1.0f ) );
		m_uiFlags |= 1;
	}

	TQuaternion quatRotation( m_pRotation->x, m_pRotation->y, m_pRotation->z, m_pRotation->w );
	m_oLocalMatrix.SetFromQuaternion( quatRotation );

	// Z up to Y up
	m_oLocalMatrix.m_f41 = m_pPosition->x;
	m_oLocalMatrix.m_f42 = -m_pPosition->z;
	m_oLocalMatrix.m_f43 = m_pPosition->y;
	m_oLocalMatrix.m_f44 = m_pPosition->w;
}

// $deBlob: FUNCTION 007cd000
TBOOL AIOMeshComponent::ExecuteActionInvoker( TINT* a_pMessage, AIOEntity* a_pEntity, ABaseEntity* a_pOther )
{
	if ( a_pMessage[ 0 ] == 0x19 )
	{
		if ( m_pModelComponent != TNULL )
		{
			TINT* pParams = *TREINTERPRETCAST( TINT**, a_pMessage + 2 );
			m_pModelComponent->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, pParams[ 1 ] == 1 );
		}

		return TTRUE;
	}

	return TFALSE;
}

// $deBlob: FUNCTION 007cd040
void AIOMeshComponent::Tick( TFLOAT a_fDeltaTime )
{
	if ( m_pModelComponent != TNULL )
	{
		if ( m_pOwner->IsIOFlagSet( 0x100 ) )
			m_pModelComponent->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, ( m_pOwner->GetFlags() & ABaseEntity::FLAG_Visible ) != 0 );

		m_pModelComponent->Tick( a_fDeltaTime );
	}
}

// $deBlob: FUNCTION 007cd090
void AIOMeshComponent::UpdateWorldTransform( const TMatrix44& a_rEntityMatrix )
{
	if ( m_pModelComponent != TNULL )
	{
		TMatrix44 matWorld;
		matWorld.Multiply( a_rEntityMatrix, m_oLocalMatrix );
		m_pModelComponent->GetModelInstance()->SetTransform( matWorld );
	}
}

// $deBlob: FUNCTION 007cd0d0
void AIOMeshComponent::GetMatrix( TMatrix44& a_rOutMatrix )
{
	TMatrix44 matTransform;

	if ( m_pModelComponent == TNULL )
	{
		a_rOutMatrix.m_f11 = m_oLocalMatrix.m_f11;
		a_rOutMatrix.m_f12 = m_oLocalMatrix.m_f12;
		a_rOutMatrix.m_f13 = m_oLocalMatrix.m_f13;
		a_rOutMatrix.m_f14 = m_oLocalMatrix.m_f14;
		a_rOutMatrix.m_f21 = m_oLocalMatrix.m_f21;
		a_rOutMatrix.m_f22 = m_oLocalMatrix.m_f22;
		a_rOutMatrix.m_f23 = m_oLocalMatrix.m_f23;
		a_rOutMatrix.m_f24 = m_oLocalMatrix.m_f24;
		a_rOutMatrix.m_f31 = m_oLocalMatrix.m_f31;
		a_rOutMatrix.m_f32 = m_oLocalMatrix.m_f32;
		a_rOutMatrix.m_f33 = m_oLocalMatrix.m_f33;
		a_rOutMatrix.m_f34 = m_oLocalMatrix.m_f34;
		matTransform.m_f41 = m_oLocalMatrix.m_f41;
		matTransform.m_f42 = m_oLocalMatrix.m_f42;
		matTransform.m_f43 = m_oLocalMatrix.m_f43;
		matTransform.m_f44 = m_oLocalMatrix.m_f44;
	}
	else
	{
		m_pModelComponent->GetModelInstance()->GetTransform( matTransform );

		// NOTE: retail copies the TModelInstance's model reference and reads its data without using it
		T2ModelPtr pModel = m_pModelComponent->GetModelInstance()->GetModelInstance()->GetModel();
		pModel.GetData();

		a_rOutMatrix.m_f11 = matTransform.m_f11;
		a_rOutMatrix.m_f12 = matTransform.m_f12;
		a_rOutMatrix.m_f13 = matTransform.m_f13;
		a_rOutMatrix.m_f14 = matTransform.m_f14;
		a_rOutMatrix.m_f21 = matTransform.m_f21;
		a_rOutMatrix.m_f22 = matTransform.m_f22;
		a_rOutMatrix.m_f23 = matTransform.m_f23;
		a_rOutMatrix.m_f24 = matTransform.m_f24;
		a_rOutMatrix.m_f31 = matTransform.m_f31;
		a_rOutMatrix.m_f32 = matTransform.m_f32;
		a_rOutMatrix.m_f33 = matTransform.m_f33;
		a_rOutMatrix.m_f34 = matTransform.m_f34;
	}

	a_rOutMatrix.m_f41 = matTransform.m_f41;
	a_rOutMatrix.m_f42 = matTransform.m_f42;
	a_rOutMatrix.m_f43 = matTransform.m_f43;
	a_rOutMatrix.m_f44 = matTransform.m_f44;
}

// $deBlob: FUNCTION 007cd1b0
void AIOMeshComponent::SetVisible( TBOOL a_bVisible )
{
	if ( m_pModelComponent != TNULL )
		m_pModelComponent->GetModelInstance()->SetFlag( AModelInstance2::FLAG_Visible, a_bVisible );
}
