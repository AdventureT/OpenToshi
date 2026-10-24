#include "pch.h"
#include "AFreeCamera.h"

#include <Input/TInputInterface.h>
#include <Input/TInputDeviceKeyboard.h>
#include <Math/TVector2.h>
#include <File/TFile.h>
#include "Memory/AMemory.h"
#include "ALevelInformation.h"
#include "GameController/AGameStatus.h"
#include "Console/AStaticCVars.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 005901d0
// $deBlob: FUNCTION 005901e0
// $deBlob: FUNCTION 00590250
// $deBlob: FUNCTION 005902b0
// $deBlob: FUNCTION 005902c0
// $deBlob: FUNCTION 0053b970
// $deBlob: FUNCTION 00857f80
TDEFINE_CLASS( AFreeCamera );

// $deBlob: FUNCTION 005902d0
AFreeCamera::AFreeCamera()
    : ACamera( CT_FREE )
{
	m_fFreeCamSpeed   = 60.0f;
	m_oController     = AInputManager2::GetSingleton()->GetControllerHandle( 0 );
	m_pFollowedObject = TNULL;
	m_bLocked         = TFALSE;
	m_bInputEnabled   = TFALSE;
	m_bUnk116         = TTRUE;
	m_iProfileCam     = 0;
	m_iNumProfileCams = 0;
	m_pProfileCams    = TNULL;
}

// $deBlob: FUNCTION 00591740
// $deBlob: FUNCTION 00590340
AFreeCamera::~AFreeCamera()
{
}

// $deBlob: FUNCTION 00590350
void AFreeCamera::Update( TFLOAT a_fDeltaTime )
{
	if ( m_bLocked || m_bInputEnabled )
		return;

	// Undoes slow_time (AApplication scales the frame time by it), so the free camera keeps its speed
	a_fDeltaTime /= AStaticCVars::cvSlowTime.GetFloat();

	if ( m_pFollowedObject != TNULL )
	{
		const TMatrix44& rObjectMatrix = *TREINTERPRETCAST( const TMatrix44*, TREINTERPRETCAST( const TUINT8*, m_pFollowedObject ) + 0x10 );
		m_Matrix.m_mMatrix.SetTranslation( rObjectMatrix.GetTranslation() );

		TVector4 vDir = rObjectMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
		vDir.w        = 1.0f;
		LookAtDirection( vDir );
		return;
	}

	AInputManager2::AInputDeviceHandle oController;

	TVector2 vAim;
	GetController( oController );
	AInputManager2::GetSingleton()->GetCameraAimAxis( vAim, oController );

	TVector2 vMove;
	GetController( oController );
	AInputManager2::GetSingleton()->GetCameraMoveAxis( vMove, oController );

	if ( TMath::Abs( vAim.x ) > TMath::TFLOAT_EPSILON )
		RotateAroundWorldUp( -( vAim.x * a_fDeltaTime ) );

	if ( TMath::Abs( vAim.y ) > TMath::TFLOAT_EPSILON )
		RotateAroundRight( -( vAim.y * a_fDeltaTime ), 0.01f );

	TInputDeviceKeyboard* pKeyboard = TInputInterface::GetSingleton()->GetDeviceByIndex<TInputDeviceKeyboard>( 0 );

	if ( pKeyboard != TNULL )
	{
		if ( pKeyboard->IsShiftDown() )
			a_fDeltaTime *= 10.0f;

		if ( pKeyboard->IsControlDown() )
			a_fDeltaTime /= 10.0f;
	}

	if ( vMove.x > TMath::TFLOAT_EPSILON )
		PushRight( vMove.x * a_fDeltaTime );
	else if ( vMove.x < 0.0f )
		PushLeft( -( vMove.x * a_fDeltaTime ) );

	if ( vMove.y > TMath::TFLOAT_EPSILON )
		PushForward( vMove.y * a_fDeltaTime );
	else if ( vMove.y < 0.0f )
		PushBackward( -( vMove.y * a_fDeltaTime ) );
}

// $deBlob: FUNCTION 005905a0
void AFreeCamera::SetInputEnabled( TBOOL a_bEnabled )
{
	if ( a_bEnabled == m_bInputEnabled )
		return;

	if ( a_bEnabled )
	{
		LoadProfileCams();
		m_bInputEnabled = a_bEnabled;
		return;
	}

	ClearProfileCams();
	m_bInputEnabled = TFALSE;
}

// $deBlob: FUNCTION 005905e0
void AFreeCamera::SelectProfileCam( TINT a_iOffset )
{
	if ( m_pProfileCams == TNULL )
		return;

	m_iProfileCam += a_iOffset;
	TMath::Clip( m_iProfileCam, 0, m_iNumProfileCams - 1 );

	TINT i   = 0;
	auto it  = m_pProfileCams->Begin();
	auto end = m_pProfileCams->End();

	if ( it == end )
		return;

	while ( i != m_iProfileCam )
	{
		it++;
		i++;

		if ( it == end )
			return;
	}

	TMatrix44 oMatrix;
	oMatrix.SetFromQuaternion( it->m_qRotation );
	oMatrix.m_f41 = it->m_vPosition.x;
	oMatrix.m_f42 = it->m_vPosition.y;
	oMatrix.m_f43 = it->m_vPosition.z;
	oMatrix.m_f44 = it->m_vPosition.w;
	SetMatrix( oMatrix );
}

// $deBlob: FUNCTION 00590740
void AFreeCamera::SetUnk116( TBOOL a_bUnk )
{
	if ( m_bUnk116 == a_bUnk )
		return;

	m_bUnk116 = a_bUnk;

	if ( m_bInputEnabled )
	{
		ClearProfileCams();
		LoadProfileCams();
		SelectProfileCam( 0 );
	}
}

// $deBlob: FUNCTION 00590780
void AFreeCamera::SetFreeCameraSpeed( TFLOAT a_fFreeCamSpeed )
{
	// NOTE: retail asserts on the old speed
	TASSERT( m_fFreeCamSpeed > 0.0f && "Free Camera Speed must be larger than zero\n" );
	m_fFreeCamSpeed = a_fFreeCamSpeed;
}

// $deBlob: FUNCTION 005907e0
void AFreeCamera::PushBackward( TFLOAT a_fDistance )
{
	TVector4 vForward = m_Matrix.m_mMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	vForward.w        = 1.0f;

	TVector4 vSpeed;
	vSpeed.Multiply( vForward, m_fFreeCamSpeed );
	TVector4 vMove;
	vMove.Multiply( vSpeed, a_fDistance );

	TVector4& rTranslation = m_Matrix.m_mMatrix.GetTranslation();
	rTranslation.x -= vMove.x;
	rTranslation.y -= vMove.y;
	rTranslation.z -= vMove.z;
}

// $deBlob: FUNCTION 00590890
void AFreeCamera::PushForward( TFLOAT a_fDistance )
{
	TVector4 vForward = m_Matrix.m_mMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	vForward.w        = 1.0f;

	TVector4 vSpeed;
	vSpeed.Multiply( vForward, m_fFreeCamSpeed );
	TVector4 vMove;
	vMove.Multiply( vSpeed, a_fDistance );

	TVector4& rTranslation = m_Matrix.m_mMatrix.GetTranslation();
	rTranslation.x += vMove.x;
	rTranslation.y += vMove.y;
	rTranslation.z += vMove.z;
}

// $deBlob: FUNCTION 00590940
void AFreeCamera::PushLeft( TFLOAT a_fDistance )
{
	TVector4 vRight = m_Matrix.m_mMatrix.AsBasisVector4( BASISVECTOR_RIGHT );
	vRight.w        = 1.0f;

	TVector4 vSpeed;
	vSpeed.Multiply( vRight, m_fFreeCamSpeed );
	TVector4 vMove;
	vMove.Multiply( vSpeed, a_fDistance );

	TVector4& rTranslation = m_Matrix.m_mMatrix.GetTranslation();
	rTranslation.x -= vMove.x;
	rTranslation.y -= vMove.y;
	rTranslation.z -= vMove.z;
}

// $deBlob: FUNCTION 005909f0
void AFreeCamera::PushRight( TFLOAT a_fDistance )
{
	TVector4 vRight = m_Matrix.m_mMatrix.AsBasisVector4( BASISVECTOR_RIGHT );
	vRight.w        = 1.0f;

	TVector4 vSpeed;
	vSpeed.Multiply( vRight, m_fFreeCamSpeed );
	TVector4 vMove;
	vMove.Multiply( vSpeed, a_fDistance );

	TVector4& rTranslation = m_Matrix.m_mMatrix.GetTranslation();
	rTranslation.x += vMove.x;
	rTranslation.y += vMove.y;
	rTranslation.z += vMove.z;
}

// $deBlob: FUNCTION 00590aa0
void AFreeCamera::Yaw( TFLOAT a_fAngle )
{
	RotateAroundWorldUp( -a_fAngle );
}

// $deBlob: FUNCTION 00590ac0
void AFreeCamera::Pitch( TFLOAT a_fAngle )
{
	RotateAroundRight( -a_fAngle, 0.01f );
}

// $deBlob: FUNCTION 00590af0
void AFreeCamera::ClearProfileCams()
{
	if ( m_pProfileCams == TNULL )
		return;

	while ( !m_pProfileCams->IsEmpty() )
	{
		ProfileCamData* pCam = m_pProfileCams->Head();
		pCam->Remove();

		if ( pCam != TNULL )
			delete pCam;
	}

	m_pProfileCams->Clear();

	if ( m_pProfileCams != TNULL )
		delete m_pProfileCams;

	m_pProfileCams    = TNULL;
	m_iNumProfileCams = 0;
	m_iProfileCam     = 0;
}

// LoadProfileCams' parsing helpers

static TBOOL IsSpace( TCHAR a_cChar )
{
	return a_cChar == ' ' || a_cChar == '\t';
}

static TBOOL IsWhiteSpace( TCHAR a_cChar )
{
	return a_cChar == ' ' || a_cChar == '\t' || a_cChar == '\r' || a_cChar == '\n';
}

// Copies the word at a_riPos (up to white space, a_cEnd or the end of the buffer, 0x200 characters at most) into
// a_szWord and moves a_riPos past it; a_cEnd 0: white space only
static void ReadWord( const TCHAR* a_pBuffer, TINT a_iSize, TINT& a_riPos, TCHAR* a_szWord, TCHAR a_cEnd )
{
	TINT iLength = 0;

	do
	{
		if ( a_riPos >= a_iSize || IsWhiteSpace( a_pBuffer[ a_riPos ] ) || ( a_cEnd != 0 && a_pBuffer[ a_riPos ] == a_cEnd ) )
			break;

		a_szWord[ iLength ] = a_pBuffer[ a_riPos ];
		a_riPos++;
		iLength++;
	} while ( iLength < 0x200 );

	a_szWord[ iLength ] = 0;
}

// Moves a_riPos to a_iStart (the end of the buffer if it's past it) and past the spaces and tabs there
static void SkipTo( const TCHAR* a_pBuffer, TINT a_iSize, TINT& a_riPos, TINT a_iStart )
{
	if ( a_iStart > a_iSize )
	{
		a_riPos = a_iSize;
		return;
	}

	for ( a_riPos = a_iStart; a_riPos < a_iSize && IsSpace( a_pBuffer[ a_riPos ] ); a_riPos++ )
		;
}

// Moves a_riPos to the start of the next line; TFALSE at the end of the buffer
static TBOOL SkipLine( const TCHAR* a_pBuffer, TINT a_iSize, TINT& a_riPos )
{
	while ( TTRUE )
	{
		if ( a_riPos >= a_iSize )
			return TFALSE;

		if ( a_pBuffer[ a_riPos ] == '\r' || a_pBuffer[ a_riPos ] == '\n' )
			break;

		a_riPos++;
	}

	while ( a_pBuffer[ a_riPos ] == '\r' || a_pBuffer[ a_riPos ] == '\n' )
	{
		a_riPos++;

		if ( a_riPos >= a_iSize )
			return TFALSE;
	}

	return TTRUE;
}

// Reads the comma separated floats of "x, y, ...)" (the last one ends at ')')
static void ReadFloats( const TCHAR* a_pBuffer, TINT a_iSize, TINT& a_riPos, TFLOAT* a_pValues, TINT a_iNumValues )
{
	TCHAR szWord[ 0x201 ];

	for ( TINT i = 0; i < a_iNumValues; i++ )
	{
		if ( i == a_iNumValues - 1 )
		{
			ReadWord( a_pBuffer, a_iSize, a_riPos, szWord, ')' );
			a_pValues[ i ] = TStringManager::String8ToFloat( szWord );
			return;
		}

		ReadWord( a_pBuffer, a_iSize, a_riPos, szWord, ',' );

		TINT iNext = a_riPos + TINT( TStringManager::String8Length( "," ) );

		if ( iNext > a_iSize )
			iNext = a_iSize;

		a_pValues[ i ] = TStringManager::String8ToFloat( szWord );

		for ( a_riPos = iNext; a_riPos < a_iSize && IsSpace( a_pBuffer[ a_riPos ] ); a_riPos++ )
			;
	}
}

// $deBlob: FUNCTION 00590bc0
void AFreeCamera::LoadProfileCams()
{
	if ( m_pProfileCams != TNULL )
		return;

	m_pProfileCams    = new ( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) T2DList<ProfileCamData>();
	m_iNumProfileCams = 0;
	m_iProfileCam     = 0;

	TFile* pFile = TFile::Create( TString8( "StatsLog.txt" ), TFILEMODE_READ );

	if ( pFile == TNULL )
		return;

	TINT   iSize   = TINT( pFile->GetSize() );
	TCHAR* pBuffer = TSTATICCAST( TCHAR, TMemalign( 16, iSize + 1 ) );
	TINT   iRead   = TINT( pFile->Read( pBuffer, iSize ) );

	if ( iSize == iRead )
	{
		pBuffer[ iRead ] = 0;
	}
	else
	{
		iSize = 0;
		TFree( pBuffer );
		pBuffer = TNULL;
	}

	pFile->Destroy();

	if ( iSize > 0 )
	{
		const TCHAR* const* ppLevelName = &ALevelInformation::GetSingleton()->GetLevelHeader()->m_pLevelProps[ AGameStatus::GetSingleton()->GetCurrentLevel() ].m_szName;

		ProfileCamData* pCam   = new ( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) ProfileCamData();
		TINT            iState = 0;
		TINT            iPos   = 0;
		TCHAR           szWord[ 0x201 ];

		// 0: looking for this level's "Level:" line, 1: its "Cam Pos:" line, 2: its "Cam Rot:" line, 3: a camera is read
		while ( iPos < iSize )
		{
			switch ( iState )
			{
				case 0:
					for ( ; iPos < iSize && IsWhiteSpace( pBuffer[ iPos ] ); iPos++ )
						;

					if ( TStringManager::String8CompareNoCase( "Level:", pBuffer + iPos, TStringManager::String8Length( "Level:" ) ) == 0 )
					{
						SkipTo( pBuffer, iSize, iPos, iPos + TINT( TStringManager::String8Length( "Level:" ) ) );
						ReadWord( pBuffer, iSize, iPos, szWord, 0 );

						if ( TStringManager::String8CompareNoCase( *ppLevelName, szWord ) == 0 )
							iState = 1;
					}

					if ( !SkipLine( pBuffer, iSize, iPos ) )
						goto done;

					break;
				case 1:
				case 2:
				{
					const TCHAR* szKey = ( iState == 1 ) ? "Cam Pos: (" : "Cam Rot: (";

					for ( ; iPos < iSize && IsWhiteSpace( pBuffer[ iPos ] ); iPos++ )
						;

					if ( TStringManager::String8CompareNoCase( "Level:", pBuffer + iPos, TStringManager::String8Length( "Level:" ) ) == 0 )
					{
						// Another level (read again in state 0)
						iState = 0;
						break;
					}

					if ( TStringManager::String8CompareNoCase( szKey, pBuffer + iPos, TStringManager::String8Length( szKey ) ) == 0 )
					{
						SkipTo( pBuffer, iSize, iPos, iPos + TINT( TStringManager::String8Length( szKey ) ) );

						if ( iState == 1 )
						{
							TFLOAT afPosition[ 3 ];
							ReadFloats( pBuffer, iSize, iPos, afPosition, 3 );
							pCam->m_vPosition.Set( afPosition[ 0 ], afPosition[ 1 ], afPosition[ 2 ], 1.0f );
							iState = 2;
						}
						else
						{
							TFLOAT afRotation[ 4 ];
							ReadFloats( pBuffer, iSize, iPos, afRotation, 4 );
							pCam->m_qRotation.Set( afRotation[ 0 ], afRotation[ 1 ], afRotation[ 2 ], afRotation[ 3 ] );
							iState = 3;
						}
					}

					// NOTE: the camera is added in state 3, so the last one of the file is never added (the file ends
					// after its "Cam Rot:" line)
					if ( !SkipLine( pBuffer, iSize, iPos ) )
						goto done;

					break;
				}
				case 3:
					m_pProfileCams->PushBack( pCam );
					m_iNumProfileCams++;

					pCam   = new ( AMemory::GetPool( AMemory::POOL_FrequentAllocations ) ) ProfileCamData();
					iState = 0;
					break;
			}
		}

	done:
		if ( pCam != TNULL )
			delete pCam;
	}

	if ( pBuffer != TNULL )
		TFree( pBuffer );
}

// $deBlob: FUNCTION 00591800
void AFreeCamera::GetController( AInputManager2::AInputDeviceHandle& a_rController )
{
	a_rController = m_oController;
}

// $deBlob: FUNCTION 00591890
TBOOL AFreeCamera::IsLocked()
{
	return m_bLocked;
}

// $deBlob: FUNCTION 005918a0
void AFreeCamera::Lock( TBOOL a_bLock )
{
	m_bLocked = a_bLock;
}
