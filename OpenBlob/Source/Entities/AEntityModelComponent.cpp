#include "pch.h"
#include "AEntityModelComponent.h"
#include "Animation/AModelFactory.h"
#include "Animation/AModelInstance2.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Turns the entity's matrix into the model's (x and z negated)
static const TMatrix44 s_mModelCorrection(
    -1.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    -1.0f,
    0.0f,
    0.0f,
    0.0f,
    0.0f,
    1.0f
);

// $deBlob: FUNCTION 007c7e80
AEntityModelComponent::AEntityModelComponent( ABaseEntity* a_pEntity, const TCHAR* a_szModelName )
    : AEntityComponent2( a_pEntity )
{
	m_pModelInstance = AModelFactory::GetSingleton()->CreateInstance( a_szModelName, TTRUE, TTRUE );
	m_bModelLoaded   = m_pModelInstance->GetModel()->IsLoaded();
}

// $deBlob: FUNCTION 007c7f00
// $deBlob: FUNCTION 007c7fc0
AEntityModelComponent::~AEntityModelComponent()
{
	AModelFactory::GetSingleton()->DestroyInstance( m_pModelInstance );
}

// $deBlob: FUNCTION 007c7f70
void AEntityModelComponent::Tick( TFLOAT a_fDeltaTime )
{
	m_pModelInstance->UpdateImp( a_fDeltaTime );
}

// $deBlob: FUNCTION 007c7f90
void AEntityModelComponent::SetTransform( const TMatrix44& a_rMatrix )
{
	TMatrix44 mTransform;
	mTransform.Multiply( a_rMatrix, s_mModelCorrection );
	m_pModelInstance->SetTransform( mTransform );
}
