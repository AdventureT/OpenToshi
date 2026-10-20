#include "pch.h"
#include "AModelsAttachedComponent.h"
#include "ABaseEntity.h"
#include "AEntityModelComponent.h"
#include "Animation/AModelFactory.h"
#include "Animation/AModelInstance2.h"
#include <Render/TModel.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AModelsAttachedComponent ) == 0x68 );

// $deBlob: FUNCTION 007c8040
AModelsAttachedComponent::AModelsAttachedComponent( ABaseEntity* a_pEntity )
    : AEntityComponent2( a_pEntity )
{
	m_iNumModelsAttached = 0;
}

// $deBlob: FUNCTION 007c8070
// $deBlob: FUNCTION 007c8410
AModelsAttachedComponent::~AModelsAttachedComponent()
{
	for ( TINT i = 0; i < m_iNumModelsAttached; i++ )
		AModelFactory::GetSingleton()->DestroyInstance( m_aModels[ i ].pInstance );

	m_iNumModelsAttached = 0;
}

// $deBlob: FUNCTION 007c80f0
void AModelsAttachedComponent::Tick( TFLOAT a_fDeltaTime )
{
	TUINT32 uiFlags = m_pEntity->GetFlags();

	if ( !( uiFlags & ABaseEntity::FLAG_Visible ) )
		return;

	if ( uiFlags & ABaseEntity::FLAG_HighLOD )
	{
		AModelInstance2* pEntityModel = TSTATICCAST( AEntityModelComponent, m_pEntity->GetModelComponent() )->GetModelInstance();

		for ( TINT i = 0; i < m_iNumModelsAttached; i++ )
		{
			if ( m_aModels[ i ].bVisible )
			{
				TMatrix44 mBone;
				pEntityModel->GetBoneTransform( m_aModels[ i ].iBone, mBone );
				m_aModels[ i ].pInstance->GetModelInstance()->GetTransform().SetMatrix( mBone );
				m_aModels[ i ].pInstance->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
			}
		}
	}

	for ( TINT i = 0; i < m_iNumModelsAttached; i++ )
	{
		if ( m_aModels[ i ].bAnimated )
			m_aModels[ i ].pInstance->UpdateImp( a_fDeltaTime );
	}
}

// $deBlob: FUNCTION 007c81a0
void AModelsAttachedComponent::AttachModel( const TCHAR* a_szModel, const TCHAR* a_szBone, TBOOL a_bVisible, TBOOL a_bAnimated, TBOOL a_bSoundBreakpoints )
{
	TASSERT( MAXMODELSATTACHED > ( m_iNumModelsAttached + 1 ) );

	m_aModels[ m_iNumModelsAttached ].pInstance = AModelFactory::GetSingleton()->CreateInstance( a_szModel, a_bSoundBreakpoints, a_bAnimated );

	AModelInstance2* pEntityModel           = TSTATICCAST( AEntityModelComponent, m_pEntity->GetModelComponent() )->GetModelInstance();
	m_aModels[ m_iNumModelsAttached ].iBone = TINT16( pEntityModel->FindBone( a_szBone ) );

	m_aModels[ m_iNumModelsAttached ].pInstance->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
	m_aModels[ m_iNumModelsAttached ].bVisible  = a_bVisible;
	m_aModels[ m_iNumModelsAttached ].bAnimated = a_bAnimated;
	m_iNumModelsAttached++;
}

// $deBlob: FUNCTION 007c8240
void AModelsAttachedComponent::SetBoneModelVisible( const TCHAR* a_szBone, TBOOL a_bVisible )
{
	AModelInstance2* pEntityModel = TSTATICCAST( AEntityModelComponent, m_pEntity->GetModelComponent() )->GetModelInstance();
	TINT16           iBone        = TINT16( pEntityModel->FindBone( a_szBone ) );

	for ( TINT i = 0; i < m_iNumModelsAttached; i++ )
	{
		if ( m_aModels[ i ].iBone == iBone )
		{
			if ( !a_bVisible )
				m_aModels[ i ].pInstance->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );

			m_aModels[ i ].bVisible = a_bVisible;
		}
	}
}

// $deBlob: FUNCTION 007c82b0
void AModelsAttachedComponent::SetModelVisible( TINT a_iNum, TBOOL a_bVisible )
{
	TASSERT( a_iNum >= 0 && a_iNum < m_iNumModelsAttached );

	if ( !a_bVisible )
		m_aModels[ a_iNum ].pInstance->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );

	m_aModels[ a_iNum ].bVisible = a_bVisible;
}

// $deBlob: FUNCTION 007c8310
void AModelsAttachedComponent::DestroyModels()
{
	for ( TINT i = 0; i < m_iNumModelsAttached; i++ )
		AModelFactory::GetSingleton()->DestroyInstance( m_aModels[ i ].pInstance );

	m_iNumModelsAttached = 0;
}

// $deBlob: FUNCTION 007c8440
void AModelsAttachedComponent::ResetComponent()
{
	for ( TINT i = 0; i < m_iNumModelsAttached; i++ )
		AModelFactory::GetSingleton()->DestroyInstance( m_aModels[ i ].pInstance );

	m_iNumModelsAttached = 0;
}

// $deBlob: FUNCTION 007c8350
void AModelsAttachedComponent::ShowAllModels()
{
	for ( TINT i = 0; i < m_iNumModelsAttached; i++ )
		m_aModels[ i ].bVisible = TTRUE;
}

// $deBlob: FUNCTION 007c8370
AModelInstance2* AModelsAttachedComponent::GetModelInstance( TINT a_iNum )
{
	TASSERT( a_iNum >= 0 && a_iNum < m_iNumModelsAttached );
	return m_aModels[ a_iNum ].pInstance;
}

// $deBlob: FUNCTION 007c83d0
void AModelsAttachedComponent::SetVisible( TBOOL a_bVisible )
{
	if ( !a_bVisible )
	{
		for ( TINT i = 0; i < m_iNumModelsAttached; i++ )
			m_aModels[ i ].pInstance->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
	}
}
