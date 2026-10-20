#include "pch.h"
#include "Sound/APositionalSoundManager.h"
#include "ASoundAction.h"
#include "AEntityParams.h"
#include "Sound/ASoundInterface.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( ASoundAction ) == 0x84 );

// $deBlob: FUNCTION 007a9980
// $deBlob: FUNCTION 007a9990
// $deBlob: FUNCTION 007a9a20
// $deBlob: FUNCTION 007a9ab0
// $deBlob: FUNCTION 007a9ac0
// $deBlob: FUNCTION 005411e0
// $deBlob: FUNCTION 00859f00
TDEFINE_CLASS( ASoundAction );

// $deBlob: FUNCTION 007a9ad0
ASoundAction::ASoundAction()
{
	m_pData = TNULL;
	SetTicking( TFALSE );
	m_Flags |= FLAG_DontTick;
}

// $deBlob: FUNCTION 007a9b40
// $deBlob: FUNCTION 007a9e00
ASoundAction::~ASoundAction()
{
	if ( m_pData != TNULL )
	{
		::operator delete( m_pData->m_pEmitters );
		::operator delete( m_pData );
	}
}

// $deBlob: FUNCTION 007a9b80
void ASoundAction::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	if ( m_pData == TNULL )
		return;

	TUINT16 uiFlags = m_pData->m_uiFlags;

	if ( uiFlags & Data::FLAG_PlayEvent )
	{
		if ( ASoundInterface::IsEventLooping( ASoundMap::GROUPID( m_pData->m_iGroupID ), ASoundMap::EVENTID( m_pData->m_iEventID ) ) )
			Start();
		else
			Play();
	}
	else if ( uiFlags & Data::FLAG_EnableEmitters )
		SetEmittersActive( TTRUE );
	else if ( uiFlags & Data::FLAG_DisableEmitters )
		SetEmittersActive( TFALSE );
}

// $deBlob: FUNCTION 007a9be0
void ASoundAction::ProcessParams( const AEntityParams& a_rParams )
{
	void* pParamData = this;
	ABaseEntity::ProcessParams( a_rParams );
	a_rParams.GetSoundAction( pParamData );
	Data* pSource = TSTATICCAST( Data, pParamData );

	m_pData = TSTATICCAST( Data, TMalloc( sizeof( Data ) ) );

	if ( pSource->m_iNumEmitters <= 0 )
	{
		m_pData->m_pEmitters = TNULL;
	}
	else
	{
		m_pData->m_pEmitters = TSTATICCAST( TUINT32, TMemalign( 16, pSource->m_iNumEmitters * sizeof( TUINT32 ) ) );
		TUtil::MemCopy( m_pData->m_pEmitters, pSource->m_pEmitters, pSource->m_iNumEmitters * sizeof( TUINT32 ) );
	}

	m_pData->m_uiFlags      = pSource->m_uiFlags;
	m_pData->m_iEventID     = pSource->m_iEventID;
	m_pData->m_iGroupID     = pSource->m_iGroupID;
	m_pData->m_iNumEmitters = pSource->m_iNumEmitters;
}

// $deBlob: FUNCTION 007a9cb0
void ASoundAction::Reset( ABaseEntity* a_pEntity )
{
	if ( m_pData != TNULL && ( m_pData->m_uiFlags & Data::FLAG_Started ) )
	{
		APositionalSoundManager::GetSingleton()->RemoveSound( this, m_pData->m_iGroupID, m_pData->m_iEventID );
		m_pData->m_uiFlags &= ~Data::FLAG_Started;
	}
}

// $deBlob: FUNCTION 007a9cf0
void ASoundAction::Start()
{
	if ( m_pData != TNULL && !( m_pData->m_uiFlags & Data::FLAG_Started ) )
	{
		if ( APositionalSoundManager::GetSingleton()->AddEntitySound( this, m_pData->m_iGroupID, m_pData->m_iEventID ) )
			m_pData->m_uiFlags |= Data::FLAG_Started;
	}
}

// $deBlob: FUNCTION 007a9d70
void ASoundAction::Play()
{
	if ( m_pData == TNULL )
		return;

	if ( m_pData->m_uiFlags & Data::FLAG_NotPositional )
		ASoundInterface::PlayEvent( ASoundMap::GROUPID( m_pData->m_iGroupID ), ASoundMap::EVENTID( m_pData->m_iEventID ), 0, TTRUE );
	else
		ASoundInterface::PlayEventAtPosition( ASoundMap::GROUPID( m_pData->m_iGroupID ), ASoundMap::EVENTID( m_pData->m_iEventID ), m_oMatrix.GetTranslation(), 0 );
}

// $deBlob: FUNCTION 007a9db0
void ASoundAction::SetEmittersActive( TBOOL a_bActive )
{
	if ( m_pData != TNULL )
	{
		for ( TINT i = 0; i < m_pData->m_iNumEmitters; i++ )
			APositionalSoundManager::GetSingleton()->SetEmitterDisabled( m_pData->m_pEmitters[ i ], a_bActive );
	}
}
