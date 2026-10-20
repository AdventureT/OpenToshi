#include "pch.h"
#include "ARevolutionary.h"
#include "Animation/AModel2.h"
#include "Animation/AModelFactory.h"
#include "Animation/AModelInstance2.h"
#include "Cameras/ACameraManager.h"
#include "Challenges/AChallenge.h"
#include "Challenges/AChallengeManager.h"
#include "Entities/AEntityFactory.h"
#include "Entities/AEntityModelComponent.h"
#include "Fx/Shadows/AShadow.h"
#include "GameInterface/APlayerManager.h"
#include "Platform/DX11/ASkinShader_DX11.h"
#include "Sound/ASoundBreakpointComponent.h"

#include <Render/TMesh.h>
#include <Render/TModel.h>
#include <Toshi/T2String8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// The ambient colour of m_pChallengeModel per marker (dword_985220...)
static TCONSTEXPR_VAR TUINT32 s_uiColourRace     = 0xff0e697b;
static TCONSTEXPR_VAR TUINT32 s_uiColourPaint    = 0xff668527;
static TCONSTEXPR_VAR TUINT32 s_uiColourCombat   = 0xfff38701;
static TCONSTEXPR_VAR TUINT32 s_uiColourLandmark = 0xff706741;

// The mesh colour of the Revolutionaries' skins (dword_9852D0...)
static TCONSTEXPR_VAR TUINT32 s_uiColourZip  = 0xffe6e6e6;
static TCONSTEXPR_VAR TUINT32 s_uiColourProf = 0xffb4b4b4;
static TCONSTEXPR_VAR TUINT32 s_uiColourArty = 0xffc8c8c8;
static TCONSTEXPR_VAR TUINT32 s_uiColourBiff = 0xff7f7f7f;

// $deBlob: FUNCTION 00779940
// $deBlob: FUNCTION 00779950
// $deBlob: FUNCTION 007799c0
// $deBlob: FUNCTION 00779a20
// $deBlob: FUNCTION 00779a30
// $deBlob: FUNCTION 005409e0
// $deBlob: FUNCTION 00859cf0
TDEFINE_CLASS( ARevolutionary );

// $deBlob: FUNCTION 00779a40
void ARevolutionary::MoveTowardsWrapped( TFLOAT& a_rfStart, TFLOAT a_fDestination, TFLOAT a_fMin, TFLOAT a_fMax, TFLOAT a_fStep )
{
	TASSERT( a_fMin < a_fMax );
	TASSERT( a_rfStart >= a_fMin && a_rfStart <= a_fMax );
	TASSERT( a_fDestination >= a_fMin && a_fDestination <= a_fMax );

	TFLOAT fStart = a_rfStart;

	if ( a_fDestination <= fStart )
	{
		TFLOAT fToMax   = a_fMax - fStart;
		TFLOAT fWrapped = ( a_fDestination - a_fMin ) + fToMax;

		if ( fStart - a_fDestination <= a_fStep || fWrapped <= a_fStep )
		{
			a_rfStart = a_fDestination;
			return;
		}

		if ( fWrapped <= fStart - a_fDestination )
		{
			// Up, through a_fMax
			if ( fToMax <= a_fStep )
				a_rfStart = ( a_fStep - fToMax ) + a_fMin;
			else
				a_rfStart = fStart + a_fStep;

			TMath::Clip( a_rfStart, a_fMin, a_fMax );
			return;
		}
	}
	else
	{
		TFLOAT fToMin   = fStart - a_fMin;
		TFLOAT fWrapped = ( a_fMax - a_fDestination ) + fToMin;

		if ( a_fDestination - fStart <= a_fStep || fWrapped <= a_fStep )
		{
			a_rfStart = a_fDestination;
			return;
		}

		if ( a_fDestination - fStart < fWrapped )
		{
			a_rfStart = fStart + a_fStep;
			TMath::Clip( a_rfStart, a_fMin, a_fMax );
			return;
		}

		// Down, through a_fMin
		if ( fToMin <= a_fStep )
		{
			a_fStep = a_fStep - fToMin;
			fStart  = a_fMax;
		}
	}

	a_rfStart = fStart - a_fStep;
	TMath::Clip( a_rfStart, a_fMin, a_fMax );
}

// $deBlob: FUNCTION 00779be0
ARevolutionary::ARevolutionary()
{
	m_uiMesh               = 1;
	m_uiMeshA              = 0;
	m_uiMeshB              = 2;
	m_bShowMarkerNextFrame = TFALSE;
	m_pChallenge           = TNULL;
	m_fTime                = 0.0f;
	m_fHeight              = 0.0f;
	m_fAmbientScale        = 1.0f;
	// NOTE: retail leaves m_pChallengeModel uninitialised until Create
	CreateShadow( 3.0f, 0.75f );
}

// $deBlob: FUNCTION 00779c80
// $deBlob: FUNCTION 0077a850
ARevolutionary::~ARevolutionary()
{
}

// $deBlob: FUNCTION 00779c90
void ARevolutionary::Create()
{
	AVisibleEntity::Create();

	const RevolutionaryData* pData = TREINTERPRETCAST( const RevolutionaryData*, m_pTypeData->m_pTRBData );
	SetBBox( *pData->m_pBBox );
	SetState( 0 );
	m_pShadow->SetHeight( 0.05f );

	m_pChallengeModel = AModelFactory::GetSingleton()->CreateInstance( "data/pickups/pickup_challengelocation.trb", TTRUE, TTRUE );
	m_pChallengeModel->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
	m_pChallengeModel->SetFlag( 0x2000, TFALSE );

	PostCreate();
}

// $deBlob: FUNCTION 00779d10
void ARevolutionary::Destroy()
{
	AModelFactory::GetSingleton()->DestroyInstance( m_pChallengeModel );
	ABaseEntity::Destroy();
}

// $deBlob: FUNCTION 00779d30
void ARevolutionary::Tick( TFLOAT a_fDeltaTime )
{
	AVisibleEntity::Tick( a_fDeltaTime );

	if ( m_bShowMarkerNextFrame )
	{
		ShowMarker();
		m_bShowMarkerNextFrame = TFALSE;
	}

	switch ( GetState() )
	{
		case 0:
			// The marker bobs and spins
			if ( m_pChallengeModel->GetFlags() & AModelInstance2::FLAG_Visible )
			{
				m_fTime += a_fDeltaTime;

				TMatrix44 mTransform;
				m_pChallengeModel->GetTransform( mTransform );
				TVector4 vPosition = mTransform.GetTranslation();
				vPosition.y        = TFLOAT( sin( m_fTime * 3.0f ) ) * 0.5f + m_fHeight;
				mTransform.RotateY( a_fDeltaTime * 2.0f );
				mTransform.GetTranslation() = vPosition;
				m_pChallengeModel->SetTransform( mTransform );
			}
			break;
		case 1:
			if ( HasStateFinished() )
				SetState( 3 );
			break;
		case 3:
			if ( HasStateFinished() )
				SetState( 4 );
			break;
		case 7:
		case 8:
			if ( HasStateFinished() )
				Refresh( TFALSE );
			break;
	}

	// Faces the camera (or the player) while busy
	if ( ( m_Flags & FLAG_Visible ) && GetState() != 0 )
	{
		TVector4           vPosition = m_oMatrix.GetTranslation();
		TVector4           vTarget;
		AChallengeManager* pChallengeManager = AChallengeManager::GetSingleton();

		if ( !pChallengeManager->IsWaitingForPrompt() || pChallengeManager->GetNearbyChallenge() == TNULL )
			vTarget = APlayerManager::GetSingleton()->GetCurrentPlayerBlob()->GetMatrix().GetTranslation();
		else
			vTarget = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix().GetTranslation();

		TVector4 vFacing( vTarget.x - vPosition.x, vTarget.y - vPosition.y, vTarget.z - vPosition.z, vTarget.w );
		vFacing.NormaliseXZ();
		SetFacing( vFacing );
	}
}

// $deBlob: FUNCTION 00779f40
void ARevolutionary::SetVisible( TBOOL a_bVisible )
{
	AVisibleEntity::SetVisible( a_bVisible );

	if ( a_bVisible && GetState() == 0 )
		m_pShadow->SetHidden( TTRUE );
}

// $deBlob: FUNCTION 00779f70
void ARevolutionary::OnLODChanging( TINT a_iLOD )
{
	AVisibleEntity::OnLODChanging( a_iLOD );

	if ( ( a_iLOD & FLAG_HighLOD ) && GetState() == 0 )
		m_pShadow->SetHidden( TTRUE );
}

// $deBlob: FUNCTION 00779fa0
void ARevolutionary::Reset( ABaseEntity* a_pEntity )
{
	AVisibleEntity::Reset( a_pEntity );
	m_bShowMarkerNextFrame = TFALSE;
	Refresh( TTRUE );
}

// $deBlob: FUNCTION 00779fd0
void ARevolutionary::SetChallenge( AChallenge* a_pChallenge )
{
	m_pChallenge = a_pChallenge;

	AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
	pModelInstance->SetFlag( 0x1000, TTRUE );

	TASSERT( m_uiMeshA < AModelInstance2::MAX_MESH_SETTINGS );
	TASSERT( m_uiMeshB < AModelInstance2::MAX_MESH_SETTINGS );
	pModelInstance->SetMeshFlags( m_uiMeshA, 1, TTRUE );
	pModelInstance->SetMeshFlags( m_uiMeshB, 1, TTRUE );

	if ( m_pChallenge->GetData()->m_uiFlags & 0x80 )
	{
		pModelInstance->SetMeshFlags( m_uiMeshA, 2, TFALSE );
		pModelInstance->SetMeshFlags( m_uiMeshB, 2, TTRUE );
	}
	else
	{
		pModelInstance->SetMeshFlags( m_uiMeshA, 2, TTRUE );
		pModelInstance->SetMeshFlags( m_uiMeshB, 2, TFALSE );
	}
}

// Called by AChallenge::Start when less than 10 seconds are left
// $deBlob: FUNCTION 0077a0c0
void ARevolutionary::OnChallengeStartShort()
{
}

// $deBlob: FUNCTION 0077a0d0
void ARevolutionary::OnChallengeStart()
{
	SetState( 4 );
}

// $deBlob: FUNCTION 0077a0e0
void ARevolutionary::OnChallengeComplete()
{
	AChallenge::Data* pData = m_pChallenge->GetData();

	for ( TUINT i = 0; i < pData->m_uiNumLinkedChallenges; i++ )
		m_pChallenge->GetData()->m_ppLinkedChallenges[ i ]->SetAvailable( TTRUE, TFALSE );

	m_bShowMarkerNextFrame = TFALSE;
	m_pChallengeModel->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );

	if ( GetState() != 8 && GetState() != 0 )
		SetState( 8 );
}

// $deBlob: FUNCTION 0077a150
void ARevolutionary::OnChallengeFail()
{
	if ( GetState() != 8 && GetState() != 0 )
		SetState( 8 );
}

// $deBlob: FUNCTION 0077a170
void ARevolutionary::OnChallengeStarting()
{
	SetState( 3 );
}

// $deBlob: FUNCTION 0077a190
void ARevolutionary::ShowStartPrompt()
{
	m_bShowMarkerNextFrame = TFALSE;
	m_pChallengeModel->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
	SetState( 1 );
}

// $deBlob: FUNCTION 0077a1c0
void ARevolutionary::Refresh( TBOOL a_bReset )
{
	SetState( 0 );

	if ( !a_bReset && AChallengeManager::GetSingleton()->GetCurrentChallenge() == m_pChallenge )
	{
		m_bShowMarkerNextFrame = TFALSE;
		m_pChallengeModel->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
	}
	else if ( !( m_pChallenge->GetFlags() & AChallenge::Flags_Completed ) && !( m_pChallenge->GetData()->m_uiFlags & AChallenge::Data::Flags_SubChallenge ) )
	{
		m_bShowMarkerNextFrame = TTRUE;
	}

	TASSERT( !( m_pChallengeModel->GetFlags() & AModelInstance2::FLAG_Visible ) || ( ( m_pChallengeModel->GetFlags() & AModelInstance2::FLAG_Visible ) && m_bShowMarkerNextFrame ) );
}

// $deBlob: FUNCTION 0077a270
void ARevolutionary::HideStartPrompt()
{
	if ( GetState() != 8 && GetState() != 0 )
		SetState( 8 );
}

// $deBlob: FUNCTION 0077a290
TBOOL ARevolutionary::IsIdle()
{
	return GetState() == 0;
}

// $deBlob: FUNCTION 0077a2a0
TBOOL ARevolutionary::IsStarting()
{
	return GetState() == 3;
}

// $deBlob: FUNCTION 0077a2b0
void ARevolutionary::ShowMarker()
{
	if ( !( m_pChallenge->GetFlags() & AChallenge::Flags_Available ) && AChallengeManager::GetSingleton()->GetCurrentChallenge() != m_pChallenge )
	{
		m_bShowMarkerNextFrame = TFALSE;
		m_pChallengeModel->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
		return;
	}

	m_pChallengeModel->SetFlag( AModelInstance2::FLAG_Visible, TTRUE );
	m_pChallengeModel->UseLocalAmbientColour( TTRUE );
	m_pChallengeModel->UseLocalAmbientLight( TTRUE );

	TUINT32 uiColour = 0;

	switch ( m_pChallenge->GetType() )
	{
		case AChallenge::Type_Paint:
		case AChallenge::Type_Civilian:
		case AChallenge::Type_Car:
			uiColour = s_uiColourPaint;
			break;
		case AChallenge::Type_Landmark:
			uiColour = s_uiColourLandmark;
			break;
		case AChallenge::Type_Combat:
			uiColour = s_uiColourCombat;
			break;
		case AChallenge::Type_Race:
			uiColour = s_uiColourRace;
			break;
		case AChallenge::Type_Null:
			// NOTE: ANullChallenge's data (0x38) says which marker it shows
			switch ( *TREINTERPRETCAST( TINT*, m_pChallenge->GetData() + 1 ) )
			{
				case 0:
					uiColour = s_uiColourRace;
					break;
				case 1:
					uiColour = s_uiColourPaint;
					break;
				case 2:
					uiColour = s_uiColourLandmark;
					break;
				case 3:
					uiColour = s_uiColourCombat;
					break;
			}
			break;
		default:
			break;
	}

	m_pChallengeModel->SetLocalAmbientColour( uiColour );
	m_pChallengeModel->SetAmbientColour( 0 );

	TMatrix44 mTransform = m_oMatrix;
	m_fTime              = 0.0f;
	mTransform.m_f42     = mTransform.m_f42 - 2.0f;
	m_fHeight            = mTransform.m_f42;
	m_pChallengeModel->SetTransform( mTransform );
}

// $deBlob: FUNCTION 0077a430
void ARevolutionary::HideMarker()
{
	m_bShowMarkerNextFrame = TFALSE;
	m_pChallengeModel->SetFlag( AModelInstance2::FLAG_Visible, TFALSE );
}

// $deBlob: FUNCTION 0077a450
void ARevolutionary::PostCreate()
{
	AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
	pModelInstance->SetFlag( 0x2000, TFALSE );

	T2ModelPtr pModel = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModel()->GetModelPtr();
	TModelLOD& rLOD   = pModel->GetLOD( 0 );

	TASSERT( 1 == TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->GetModel()->GetModelPtr()->GetLODCount() );

	for ( TINT i = 0; i < rLOD.GetMeshCount(); i++ )
	{
		ASkinMaterialHAL* pMaterial = TSTATICCAST( ASkinMaterialHAL, rLOD.GetMeshes()[ i ]->GetMaterial() );
		AModelInstance2*  pInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
		TASSERT( TUINT8( i ) < AModelInstance2::MAX_MESH_SETTINGS );

		const TCHAR* szName = pMaterial->GetName();

		// The rim light, except on the manhole and skylight meshes
		if ( szName == TNULL || ( !T2String8::FindString( szName, "manhole" ) && !T2String8::FindString( szName, "skylight" ) ) )
		{
			TTexture* pRimTexture = TTextureManager::GetSingleton()->FindTexture( "rim_add_revs.tga" );
			pRimTexture->SetWrap( D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_TEXTURE_ADDRESS_CLAMP );
			pMaterial->SetTextureNum( 2 );
			pMaterial->SetTevMode( 0, 0x11 );
			pMaterial->SetTevMode( 1, 2 );
			pMaterial->SetTexture( 1, pRimTexture );
			pMaterial->SetPassMode( 1, ASkinMaterialHAL::PASSMODE_RIMLIGHT );

			// The skin colour of each Revolutionary
			if ( szName != TNULL )
			{
				if ( T2String8::FindString( szName, "mat_revbiff_old" ) )
					pInstance->SetMeshColour( TUINT8( i ), s_uiColourBiff );
				else if ( T2String8::FindString( szName, "rev_prof_skin" ) )
					pInstance->SetMeshColour( TUINT8( i ), s_uiColourProf );
				else if ( T2String8::FindString( szName, "rev_zip_skin" ) )
					pInstance->SetMeshColour( TUINT8( i ), s_uiColourZip );
				else if ( T2String8::FindString( szName, "rev_arty_skin" ) )
					pInstance->SetMeshColour( TUINT8( i ), s_uiColourArty );
			}

			TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->UseLocalAmbientLight( TTRUE );
			TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance()->SetAmbientColour( 0xffffff );
		}
	}
}

// $deBlob: FUNCTION 0077a720
void ARevolutionary::OnEnterState( TINT a_iState )
{
	switch ( a_iState )
	{
		case 0:
			ShowMesh();
			break;
		case 1:
		case 2:
			HideMesh();
			break;
	}
}

// $deBlob: FUNCTION 0077a750
const TCHAR* ARevolutionary::GetModelName()
{
	return m_pTypeData->m_pTRBData->m_szModelName;
}

// $deBlob: FUNCTION 0077a760
void ARevolutionary::ShowMesh()
{
	AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
	TASSERT( m_uiMesh < AModelInstance2::MAX_MESH_SETTINGS );
	pModelInstance->SetMeshFlags( m_uiMesh, 2, TFALSE );
	pModelInstance->SetMeshFlags( m_uiMesh, 1, TTRUE );
	m_pShadow->SetHidden( TTRUE );

	if ( pModelInstance->GetSoundBreakpointComponent() != TNULL )
		TSTATICCAST( ASoundBreakpointComponent, pModelInstance->GetSoundBreakpointComponent() )->SetMuted( TTRUE );
}

// $deBlob: FUNCTION 0077a7e0
void ARevolutionary::HideMesh()
{
	AModelInstance2* pModelInstance = TSTATICCAST( AEntityModelComponent, GetModelComponent() )->GetModelInstance();
	pModelInstance->GetMeshSettings( m_uiMesh )->uiFlags |= 3;
	m_pShadow->SetHidden( TFALSE );

	if ( pModelInstance->GetSoundBreakpointComponent() != TNULL )
		TSTATICCAST( ASoundBreakpointComponent, pModelInstance->GetSoundBreakpointComponent() )->SetMuted( TFALSE );
}
