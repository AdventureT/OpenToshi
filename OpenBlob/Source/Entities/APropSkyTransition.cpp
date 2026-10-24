#include "pch.h"
#include "APropSkyTransition.h"
#include "AEntityIOPacket.h"
#include "Fx/AEnvironmentFX.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( APropSkyTransition ) == 0x80 );

// $deBlob: FUNCTION 007a22b0
// $deBlob: FUNCTION 007a22c0
// $deBlob: FUNCTION 007a2340
// $deBlob: FUNCTION 007a23b0
// $deBlob: FUNCTION 007a23c0
// $deBlob: FUNCTION 00540fe0
// $deBlob: FUNCTION 00859e80
TDEFINE_CLASS( APropSkyTransition );

// $deBlob: FUNCTION 007a23d0
APropSkyTransition::APropSkyTransition()
{
}

// $deBlob: FUNCTION 007a23f0
// $deBlob: FUNCTION 007a2440
APropSkyTransition::~APropSkyTransition()
{
}

// $deBlob: FUNCTION 007a2400
void APropSkyTransition::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	switch ( a_rPacket.m_eMessage )
	{
		case AEntityIOPacket::MESSAGE_TRIGGER:
			if ( AEnvironmentFX::GetSingleton() != TNULL )
				AEnvironmentFX::GetSingleton()->StartSkySpace();
			break;
	}
}

// $deBlob: FUNCTION 007a2420
void APropSkyTransition::Reset( ABaseEntity* a_pEntity )
{
	// NOTE: the same as ABaseEntity::Reset
	ABaseEntity::Reset( a_pEntity );
}
