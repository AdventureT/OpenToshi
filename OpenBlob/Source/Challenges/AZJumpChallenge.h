#pragma once
#include "AChallenge.h"


//-----------------------------------------------------------------------------
// Purpose: a Z-jump challenge
//-----------------------------------------------------------------------------
class AZJumpChallenge
    : public AChallenge
{
public:
	// One Z-jump set (8 bytes)
	struct ZJumpSet
	{
		union
		{
			TINT  m_iWPSetID; // 0x00 in the TRB...
			void* m_pSet;     // 0x00 ...and the AZJumpData set (0x10 bytes, Create)
		};
		TBOOL m_bDone; // 0x04
	};

	// The data in the TRB (0x40 bytes)
	struct TypeData : public Data
	{
		ZJumpSet* m_aiZJumpSets; // 0x38
		TUINT32   m_uiNumSets;   // 0x3C
	};

public:
	AZJumpChallenge();
	// $deBlob: FUNCTION 005a06b0
	// $deBlob: FUNCTION 0059ce00
	virtual ~AZJumpChallenge() override; // +0x00

	virtual void  Start() override;                       // +0x04
	virtual void  OnZJumpEvent( void* a_pSet ) override;  // +0x14
	virtual void  Update( TFLOAT a_fDeltaTime ) override; // +0x30
	virtual void  Reset( TBOOL a_bUnk ) override;         // +0x3C
	virtual TBOOL Create( TypeData* a_pData );            // +0x4C

	// Whether the challenge uses the waypoint set a_iWPSetID
	TBOOL UsesWPSet( TINT a_iWPSetID ) const
	{
		const TypeData* pData = TSTATICCAST( const TypeData, m_pData );

		for ( TUINT32 i = 0; i < pData->m_uiNumSets; i++ )
		{
			if ( pData->m_aiZJumpSets[ i ].m_iWPSetID == a_iWPSetID )
				return TTRUE;
		}

		return TFALSE;
	}

private:
};

TSTATICASSERT( sizeof( AZJumpChallenge ) == 0x174 );
