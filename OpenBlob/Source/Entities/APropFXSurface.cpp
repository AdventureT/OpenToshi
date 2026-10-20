#include "pch.h"
#include "APropFXSurface.h"
#include "ACharEntity.h"
#include "AEntityParams.h"
#include "AEntityIOPacket.h"

#include <Math/TCollision.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

APropFXSurface::Node* APropFXSurface::ms_pNodes;
// $deBlob: FUNCTION 005419b0
// $deBlob: FUNCTION 0085a100
Toshi::T2DList<APropFXSurface::Node> APropFXSurface::ms_oFreeNodes;

TSTATICASSERT( sizeof( APropFXSurface::Slot ) == 0x2BC );
TSTATICASSERT( sizeof( APropFXSurface ) == 0x1670 );

// NOTE: retail calls GetCollisionObject() and ignores it, then uses constants: a read-only copy of TVector4::VEC_ZERO
// as the box's centre and a unit (1, 1, 1, 1) vector as its half extents
static const TVector4 s_vSurfaceCentre = TVector4::VEC_ZERO;
static const TVector4 s_vSurfaceHalfExtents( 1.0f, 1.0f, 1.0f, 1.0f );

// $deBlob: FUNCTION 007ddc80
// $deBlob: FUNCTION 007ddc90
// $deBlob: FUNCTION 007ddd00
// $deBlob: FUNCTION 007ddd60
// $deBlob: FUNCTION 007ddd70
// $deBlob: FUNCTION 00541970
// $deBlob: FUNCTION 0085a0f0
TDEFINE_CLASS( APropFXSurface );

// $deBlob: FUNCTION 007ddd80
APropFXSurface::APropFXSurface()
{
	m_iUnk166C = -1;

	for ( TINT i = 0; i < NUM_SLOTS; i++ )
		m_aSlots[ i ].m_bUsed = TFALSE;
}

// $deBlob: FUNCTION 007dde40
// $deBlob: FUNCTION 007de9f0
APropFXSurface::~APropFXSurface()
{
	ReleaseNodes();
}

// $deBlob: FUNCTION 007de520
void APropFXSurface::InitialisePool()
{
	ms_pNodes = new Node[ NUM_NODES ];

	for ( TINT i = 0; i < NUM_NODES; i++ )
		ms_pNodes[ i ].InsertBefore( &*ms_oFreeNodes.End() );
}

// $deBlob: FUNCTION 007de5d0
void APropFXSurface::DeinitialisePool()
{
	ms_oFreeNodes.Clear();

	if ( ms_pNodes != TNULL )
		delete[] ms_pNodes;
}

// $deBlob: FUNCTION 007de610
void APropFXSurface::ReleaseNodes()
{
	while ( !m_oNodes.IsEmpty() )
	{
		Node* pNode = m_oNodes.Back();
		pNode->Remove();
		pNode->InsertBefore( &*ms_oFreeNodes.End() );
	}
}

// $deBlob: FUNCTION 007dded0
void APropFXSurface::ProcessParams( const AEntityParams& a_rParams )
{
	ABaseEntity::ProcessParams( a_rParams );

	const TCHAR* szName = TNULL;
	a_rParams.GetValue( szName, "EnterEffectName" );

	if ( szName != TNULL && TStringManager::String8Length( szName ) != 0 )
	{
		for ( TINT i = 0; i < NUM_SLOTS; i++ )
			m_aSlots[ i ].m_oEnter.Initialize( szName );
	}

	szName = TNULL;
	a_rParams.GetValue( szName, "InsideEffectName" );

	if ( szName != TNULL && TStringManager::String8Length( szName ) != 0 )
	{
		for ( TINT i = 0; i < NUM_SLOTS; i++ )
			m_aSlots[ i ].m_oInside.Initialize( szName );
	}

	szName = TNULL;
	a_rParams.GetValue( szName, "ExitEffectName" );

	if ( szName != TNULL && TStringManager::String8Length( szName ) != 0 )
	{
		for ( TINT i = 0; i < NUM_SLOTS; i++ )
			m_aSlots[ i ].m_oExit.Initialize( szName );
	}
}

TBOOL APropFXSurface::IsInSurface( ABaseEntity* a_pEntity )
{
	// NOTE: retail calls GetCollisionObject() three times and uses none (see s_vSurfaceCentre)
	GetCollisionObject();
	GetCollisionObject();
	GetCollisionObject();

	return TCollision::AABBvsOBB( a_pEntity->GetBasePosition(), a_pEntity->GetBBox(), s_vSurfaceCentre, TMatrix44::IDENTITY, s_vSurfaceHalfExtents );
}

// $deBlob: FUNCTION 007ddfd0
void APropFXSurface::HandleEntityIO( ABaseEntity* a_pCaller, const AEntityIOPacket& a_rPacket )
{
	TObject* pSender = TSTATICCAST( TObject, a_pCaller );

	AInvisibleEntity::HandleEntityIO( a_pCaller, a_rPacket );

	TASSERT( ( FLAG_LODMask_Level & m_Flags ) == FLAG_LowLOD || ( FLAG_LODMask_Level & m_Flags ) == FLAG_MediumLOD || ( FLAG_LODMask_Level & m_Flags ) == FLAG_HighLOD );

	if ( ( m_Flags & FLAG_LODMask_Level ) != FLAG_HighLOD || a_rPacket.m_eMessage != AEntityIOPacket::MESSAGE_TOUCH || ms_oFreeNodes.IsEmpty() )
		return;

	if ( !pSender->IsA( &TGetClass( ACharEntity ) ) )
		return;

	ABaseEntity* pEntity = pSender->CastTo<ABaseEntity>();

	if ( !IsInSurface( pEntity ) || !pEntity->IsVisible() )
		return;

	// Already on the surface?
	for ( auto it = m_oNodes.Begin(); it != m_oNodes.End(); it++ )
	{
		if ( it->m_pEntity == pEntity )
			return;
	}

	// A free effect set
	TINT iSlot;

	for ( iSlot = 0;; iSlot++ )
	{
		if ( iSlot >= NUM_SLOTS )
			return;

		Slot& rSlot = m_aSlots[ iSlot ];

		if ( !rSlot.m_bUsed && rSlot.m_oEnter.IsFinished() && rSlot.m_oInside.IsFinished() && rSlot.m_oExit.IsFinished() )
			break;
	}

	if ( a_rPacket.m_eMessage == AEntityIOPacket::MESSAGE_TOUCH )
	{
		Slot* pSlot = &m_aSlots[ iSlot ];

		if ( pSlot->m_oEnter.HasEffect() )
		{
			TMatrix44 oMatrix;
			GetEffectMatrix( pEntity, oMatrix );
			pSlot->m_oEnter.TriggerEffect( oMatrix );
		}

		Node* pNode = ms_oFreeNodes.Back();
		pNode->Remove();
		pNode->m_pEntity = pEntity;
		pNode->m_pSlot   = pSlot;
		pNode->InsertBefore( &*m_oNodes.End() );
	}

	SetTicking( TTRUE );
}

// $deBlob: FUNCTION 007de1e0
void APropFXSurface::Tick( TFLOAT a_fDeltaTime )
{
	ABaseEntity::Tick( a_fDeltaTime );

	TBOOL bPlaying = TFALSE;

	for ( TINT i = 0; i < NUM_SLOTS; i++ )
	{
		if ( m_aSlots[ i ].m_oEnter.Update( a_fDeltaTime ) == TTRUE )
			bPlaying = TTRUE;

		if ( m_aSlots[ i ].m_oInside.Update( a_fDeltaTime ) == TTRUE )
			bPlaying = TTRUE;

		if ( m_aSlots[ i ].m_oExit.Update( a_fDeltaTime ) == TTRUE )
			bPlaying = TTRUE;
	}

	if ( m_oNodes.IsEmpty() )
	{
		if ( !bPlaying )
			SetTicking( TFALSE );

		return;
	}

	auto it = m_oNodes.Begin();

	while ( it != m_oNodes.End() )
	{
		auto         itNext  = it.Next();
		Node*        pNode   = it;
		ABaseEntity* pEntity = pNode->m_pEntity;
		TBOOL        bLeft   = TTRUE;

		if ( ( pEntity->GetFlags() & FLAG_Hashed ) && ( pEntity->GetFlags() & FLAG_Visible ) && IsInSurface( pEntity ) )
		{
			bLeft = TFALSE;

			TFLOAT fBottom = pEntity->GetBasePosition().y - pEntity->GetBBox().y;

			// NOTE: retail calls GetCollisionObject() twice and uses none (the surface's height is s_vSurfaceCentre.y
			// - s_vSurfaceHalfExtents.y)
			GetCollisionObject();
			GetCollisionObject();

			if ( fBottom <= s_vSurfaceCentre.y - s_vSurfaceHalfExtents.y )
			{
				if ( pNode->m_pSlot->m_oInside.HasEffect() )
				{
					TMatrix44 oMatrix;
					GetEffectMatrix( pEntity, oMatrix );

					if ( !pNode->m_pSlot->m_oInside.IsFinished() )
						pNode->m_pSlot->m_oInside.SetTransform( oMatrix );
					else
						pNode->m_pSlot->m_oInside.TriggerEffect( oMatrix );
				}
			}
			else if ( !pNode->m_pSlot->m_oInside.IsFinished() )
			{
				pNode->m_pSlot->m_oInside.Stop();
			}
		}

		if ( bLeft )
		{
			// The character left the surface
			if ( pNode->m_pSlot->m_oExit.HasEffect() )
			{
				TMatrix44 oMatrix;
				GetEffectMatrix( pEntity, oMatrix );
				pNode->m_pSlot->m_oExit.TriggerEffect( oMatrix );
			}

			if ( pNode->m_pSlot->m_oInside.HasEffect() )
				pNode->m_pSlot->m_oInside.Stop();

			pNode->m_pSlot->m_bUsed = TFALSE;
			pNode->m_pSlot          = TNULL;
			pNode->Remove();
			pNode->InsertBefore( &*ms_oFreeNodes.End() );
		}

		it = itNext;
	}
}

// $deBlob: FUNCTION 007de670
void APropFXSurface::GetEffectMatrix( const TVector4& a_rPosition, TMatrix44& a_rOutMatrix )
{
	a_rOutMatrix = m_oMatrix;

	TFLOAT fX = a_rPosition.x;
	TFLOAT fZ = a_rPosition.z;
	TFLOAT fW = a_rPosition.w;

	// NOTE: retail calls GetCollisionObject() twice and uses none (see s_vSurfaceCentre)
	GetCollisionObject();
	GetCollisionObject();

	TVector4 vTranslation( fX, s_vSurfaceCentre.y - s_vSurfaceHalfExtents.y, fZ, fW );
	TASSERT( vTranslation.w == 1.0f );
	a_rOutMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) = vTranslation;
}

// $deBlob: FUNCTION 007de730
void APropFXSurface::GetEffectMatrix( ABaseEntity* a_pEntity, TMatrix44& a_rOutMatrix )
{
	GetEffectMatrix( a_pEntity->GetMatrix().GetTranslation(), a_rOutMatrix );
}
