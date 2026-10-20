#pragma once
#include "AChallenge.h"


//-----------------------------------------------------------------------------
// Purpose: a challenge painting targets
//-----------------------------------------------------------------------------
class APaintChallenge
    : public AChallenge
{
public:
	// One target (8 bytes)
	struct Target
	{
		union
		{
			TINT  m_iInstance; // 0x00 the terrain instance in the TRB, -1: none...
			void* m_pInstance; // 0x00 ...and the instance (Create)
		};
		TINT16 m_iColour; // 0x04
		TINT16 m_iDone;   // 0x06
	};

	// The data in the TRB (0x40 bytes)
	struct TypeData : public Data
	{
		Target* m_pTargets;     // 0x38
		TUINT32 m_uiNumTargets; // 0x3C
	};

public:
	APaintChallenge();
	// $deBlob: FUNCTION 0059efe0
	// $deBlob: FUNCTION 0059cce0
	virtual ~APaintChallenge() override; // +0x00

	virtual void  Start() override;                       // +0x04
	virtual void  OnEnd() override;                       // +0x0C
	virtual void  OnPaintEvent( void* a_pBuilding ) override; // +0x10
	virtual void  OnPaintEvent2( void* a_pTree ) override;    // +0x2C
	virtual void  Update( TFLOAT a_fDeltaTime ) override;     // +0x30
	virtual void  Reset( TBOOL a_bUnk ) override;             // +0x3C
	virtual TINT  GetScore() override;                        // +0x40
	virtual TBOOL IsAlreadyCompleted() override;              // +0x44
	virtual TBOOL Create( TypeData* a_pData );                // +0x4C

	// Switches the targets' projections off (removed) or on again (debug)
	void ToggleProjections();

	// The index of target a_iInstance, -1 if it isn't one
	TINT GetTargetIndex( TINT a_iInstance ) const
	{
		const TypeData* pData = TSTATICCAST( const TypeData, m_pData );

		for ( TUINT32 i = 0; i < pData->m_uiNumTargets; i++ )
		{
			if ( pData->m_pTargets[ i ].m_iInstance == a_iInstance )
				return pData->m_pTargets[ i ].m_iColour;
		}

		return -1;
	}

private:
	TBOOL m_bPaintingStarted; // 0x174
};

TSTATICASSERT( sizeof( APaintChallenge ) == 0x178 );
