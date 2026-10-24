#include "pch.h"
#include "AZJumpData.h"
#include "ABaseEntity.h"
#include "ParticleSystem/AEnvParticles.h"
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffectManager.h>

#include <File/TTRB.h>
#include <Math/TMatrix44.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TSTATICASSERT( sizeof( AZJumpData ) == 0x10 );
TSTATICASSERT( sizeof( AZJumpData::Node ) == 0x18 );
TSTATICASSERT( sizeof( AZJumpData::Set ) == 0x10 );

// $deBlob: FUNCTION 00782e00
AZJumpData::AZJumpData( TTRB* a_pTRB )
{
	m_pData = TNULL;
	m_apEmitters = TNULL;
	m_pData = a_pTRB->GetSymbolAddress( "ZJumpData" );
}

// $deBlob: FUNCTION 005a06e0
AZJumpData::Set* AZJumpData::GetSet( TINT a_iSet ) const
{
	Block* pBlock = TREINTERPRETCAST( Block*, m_pData );

	TASSERT( -1 < a_iSet );
	TASSERT( a_iSet < pBlock->m_iSetCount );

	return pBlock->m_pSets + a_iSet;
}

// $deBlob: FUNCTION 0076a780
AZJumpData::Point* AZJumpData::Set::FindPoint( const Node* a_pNode ) const
{
	for ( TINT i = 0; i < m_iPointCount; i++ )
	{
		if ( m_pPoints[ i ].m_pNode == a_pNode )
			return &m_pPoints[ i ];
	}

	return TNULL;
}

// $deBlob: FUNCTION 00782dd0
const TVector4& AZJumpData::Node::GetPosition() const
{
	switch ( m_eType )
	{
		case NodeType_Instance:
			return ( *TREINTERPRETCAST( TMatrix44**, m_pInstance ) )->GetTranslation();
		case NodeType_Entity:
			return TSTATICCAST( ABaseEntity, m_pInstance )->GetMatrix().GetTranslation();
		default:
			return TVector4::VEC_ZERO; // X0Y0Z0W1
	}
}

// $deBlob: FUNCTION 00782ea0
void AZJumpData::SetPointEffect( TINT a_iIndex, TBOOL a_bOn )
{
	TASSERT( a_iIndex < GetPointCount() );

	if ( !a_bOn )
	{
		if ( m_apEmitters[ a_iIndex ] != TNULL )
		{
			AEnvParticles::GetSingleton()->RemoveEmitter( m_apEmitters[ a_iIndex ], TTRUE );
			m_apEmitters[ a_iIndex ] = TNULL;
		}
	}
	else if ( m_apEmitters[ a_iIndex ] == TNULL )
	{
		TMatrix44 mTransform;
		mTransform.Identity();
		mTransform.SetTranslation( GetPoint( a_iIndex )->m_pNode->GetPosition() );

		m_apEmitters[ a_iIndex ] = AEnvParticles::GetSingleton()->AddEmitter( m_pEffect, mTransform, 0.0f, AEnvEmitter::FLAGS_Unk1 );
	}
}

// $deBlob: FUNCTION 00782f80
void AZJumpData::UpdatePointEffect( TINT a_iIndex )
{
	if ( m_apEmitters[ a_iIndex ] != TNULL )
	{
		TMatrix44 mTransform;
		mTransform.Identity();
		mTransform.SetTranslation( GetPoint( a_iIndex )->m_pNode->GetPosition() );

		AEnvParticles::GetSingleton()->SetEmitterTransform( m_apEmitters[ a_iIndex ], mTransform );
	}
}

// $deBlob: FUNCTION 00782ff0
void AZJumpData::CreatePointEffects()
{
	m_apEmitters = TSTATICCAST( AEnvEmitter*, TMemalign( 16, GetPointCount() * sizeof( AEnvEmitter* ) ) );
	m_pEffect    = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( "Blob_Env_ZJump_Active" );
	TASSERT( m_pEffect, "Couldn't find particle effect for zjump markers: Blob_ZJump_WarmingGlow" );

	for ( TINT i = 0; i < GetPointCount(); i++ )
	{
		TASSERT( NodeType_Instance == GetPoint( i )->m_pNode->m_eType );

		TMatrix44 mTransform;
		mTransform.Identity();
		mTransform.SetTranslation( GetPoint( i )->m_pNode->GetPosition() );

		m_apEmitters[ i ] = AEnvParticles::GetSingleton()->AddEmitter( m_pEffect, mTransform, 0.0f, AEnvEmitter::FLAGS_Unk1 );
	}
}

// $deBlob: FUNCTION 00783200
void AZJumpData::DestroyPointEffects()
{
	if ( AEnvParticles::GetSingleton() != TNULL )
	{
		for ( TINT i = 0; i < GetPointCount(); i++ )
		{
			if ( m_apEmitters[ i ] != TNULL )
			{
				AEnvParticles::GetSingleton()->RemoveEmitter( m_apEmitters[ i ], TTRUE );
				m_apEmitters[ i ] = TNULL;
			}
		}
	}

	::operator delete( m_apEmitters );
	m_apEmitters = TNULL;
	m_pEffect    = TNULL;
}
