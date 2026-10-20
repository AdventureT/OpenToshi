#include "pch.h"
#include "AEnvParticles.h"
#include "Cameras/ACamera.h"
#include "Render/ARenderer.h"
#include "Render/AXYZViewportManager.h"
#include <TParticleSystem/TParticleSystem.h>
#include <TParticleSystem/TParticleEffect.h>
#include <TParticleSystem/TParticleEffectInstance.h>
#include <TParticleSystem/TParticleEffectManager.h>
#include <TParticleSystem/TGenericParticlesSettings.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TINT                         AEnvParticles::s_iMaxActiveEmitters  = 50;
TINT                         AEnvParticles::sm_iNumActiveEmitters = 0;
Toshi::T2GenericDList::Node* AEnvParticles::sm_pInactiveList      = TNULL;

TSTATICASSERT( sizeof( AEnvEmitter ) == 0x64 );
TSTATICASSERT( sizeof( AEnvParticles ) == 0x3C );

// The emitter of a list node (the node is A2DHashItem::m_Node, at 0x0C of the emitter)
static AEnvEmitter* GetEmitterOfNode( T2GenericDList::Node* a_pNode )
{
	return TREINTERPRETCAST( AEnvEmitter*, TREINTERPRETCAST( TUINT8*, a_pNode ) - 0x0C );
}

static TBOOL IsListEmpty( const T2GenericDList::Node* a_pList )
{
	return a_pList->Next() == a_pList;
}

// Moves all the nodes of a_pFrom before a_pInsertBefore (T2DList::Splice)
static void SpliceList( T2GenericDList::Node* a_pInsertBefore, T2GenericDList::Node* a_pFrom )
{
	TASSERT( a_pFrom != a_pInsertBefore );
	TASSERT( !IsListEmpty( a_pFrom ), "Don't splice from an empty list or the Root node will be moved!" );
	T2GenericDList::Splice( a_pInsertBefore, a_pFrom->Next(), a_pFrom->Prev() );
}

// $deBlob: FUNCTION 005e89a0
AEnvParticles::AEnvParticles( TINT a_iMaxEmitters )
{
	m_iMaxEmitters = a_iMaxEmitters;
	m_iNumEmitters = 0;

	TFLOAT fMinX, fMinY, fMaxX, fMaxY;
	TINT   iXDiv, iYDiv;
	A2DHash::GetLevelBounds( 50.0f, &fMinX, &fMinY, &fMaxX, &fMaxY, &iXDiv, &iYDiv );

	m_pHash = new A2DHash();
	m_pHash->Init( TMemory_GetGlobalHeap(), &AEnvParticles::SetEmitterStamps, fMinX, fMinY, fMaxX, fMaxY, iXDiv, iYDiv, 200, 2048, 1024 );

	m_paEmitters = new AEnvEmitter[ m_iMaxEmitters ];

	for ( TINT i = 0; i < m_iMaxEmitters; i++ )
		m_paEmitters[ i ].m_Node.InsertBefore( &m_oFree );

	sm_iNumActiveEmitters = 0;
	m_apQueued            = TSTATICCAST( AEnvEmitter*, TMalloc( s_iMaxActiveEmitters * sizeof( AEnvEmitter* ) ) );
	m_apQueued1           = TSTATICCAST( AEnvEmitter*, TMalloc( s_iMaxActiveEmitters * sizeof( AEnvEmitter* ) ) );
	sm_pInactiveList      = &m_oInactive;
}

// $deBlob: FUNCTION 005e8c00
// $deBlob: FUNCTION 005ea140
AEnvParticles::~AEnvParticles()
{
	RemoveAllEmitters();

	// T2DList::Clear of the free list
	while ( !IsListEmpty( &m_oFree ) )
		m_oFree.Next()->Remove();

	delete[] m_paEmitters;

	TFree( m_apQueued );
	TFree( m_apQueued1 );

	if ( m_pHash != TNULL )
		delete m_pHash;

	sm_pInactiveList = TNULL;
}

// $deBlob: FUNCTION 005e8d50
void AEnvParticles::Update( TFLOAT a_fDeltaTime )
{
	// The emitters that had an effect: the ones still near a camera are moved back to the active lists
	T2GenericDList::Node oOldActive;
	T2GenericDList::Node oOldActive1;

	if ( !IsListEmpty( &m_oActive ) )
		SpliceList( &oOldActive, &m_oActive );

	if ( !IsListEmpty( &m_oActive1 ) )
		SpliceList( &oOldActive1, &m_oActive1 );

	TINT iNumQueued1 = 0;
	TINT iNumQueued  = 0;

	AXYZViewportManager* pViewportManager = ARenderer::GetSingleton()->GetViewportManager();

	for ( TINT i = 0; i < AXYZViewportManager::VIEWPORT_Count; i++ )
	{
		AXYZViewportManager::VIEWPORT eViewport = AXYZViewportManager::VIEWPORT( i );

		if ( !pViewportManager->IsViewportUsed( eViewport ) )
			continue;

		ACamera* pCamera = pViewportManager->GetCamera( eViewport );

		if ( pCamera == TNULL )
			continue;

		TVector4 vDirection = pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_FORWARD );
		vDirection.w        = 1.0f;

		TINT             iNumResults = 0;
		A2DHash::Result* pResults    = m_pHash->Query( pCamera->GetMatrix().GetTranslation(), vDirection, 250.0f, &iNumResults, 200 );

		for ( TINT j = 0; j < iNumResults; j++ )
		{
			AEnvEmitter* pEmitter = TSTATICCAST( AEnvEmitter, pResults[ j ].m_pItem );

			if ( pEmitter->m_pParticleEffectInstance != TNULL )
			{
				pEmitter->m_Node.Remove();
				pEmitter->m_Node.InsertBefore( ( pEmitter->m_uiFlags & AEnvEmitter::FLAGS_Unk1 ) ? &m_oActive1 : &m_oActive );
			}
			else if ( !( pEmitter->m_uiFlags & AEnvEmitter::FLAGS_Queued ) )
			{
				if ( pEmitter->m_uiFlags & AEnvEmitter::FLAGS_Unk1 )
				{
					if ( iNumQueued1 < s_iMaxActiveEmitters )
					{
						pEmitter->m_uiFlags |= AEnvEmitter::FLAGS_Queued;
						m_apQueued1[ iNumQueued1++ ] = pEmitter;
					}
				}
				else if ( iNumQueued < s_iMaxActiveEmitters )
				{
					pEmitter->m_uiFlags |= AEnvEmitter::FLAGS_Queued;
					m_apQueued[ iNumQueued++ ] = pEmitter;
				}
			}
		}
	}

	// The emitters that are no longer near a camera lose their effect
	while ( !IsListEmpty( &oOldActive ) )
	{
		AEnvEmitter* pEmitter = GetEmitterOfNode( oOldActive.Prev() );

		if ( pEmitter->m_pParticleEffectInstance != TNULL )
		{
			pEmitter->m_pParticleEffectInstance->SetFinishedCallback( TNULL, TNULL );
			pEmitter->m_pParticleEffectInstance->Stop();
			pEmitter->m_pParticleEffectInstance = TNULL;
			sm_iNumActiveEmitters--;
		}

		pEmitter->m_Node.Remove();
		pEmitter->m_Node.InsertBefore( &m_oInactive );
	}

	while ( !IsListEmpty( &oOldActive1 ) )
	{
		AEnvEmitter* pEmitter = GetEmitterOfNode( oOldActive1.Prev() );

		if ( pEmitter->m_pParticleEffectInstance != TNULL )
		{
			pEmitter->m_pParticleEffectInstance->SetFinishedCallback( TNULL, TNULL );
			pEmitter->m_pParticleEffectInstance->Stop();
			pEmitter->m_pParticleEffectInstance = TNULL;
			sm_iNumActiveEmitters--;
		}

		pEmitter->m_Node.Remove();
		pEmitter->m_Node.InsertBefore( &m_oInactive );
	}

	// The queued FLAGS_Unk1 emitters start first: when all the instances are used, the first emitter of m_oActive
	// makes room (once per emitter); with m_oActive empty the rest stays unstarted
	TINT i = 0;

	for ( ; i < iNumQueued1; i++ )
	{
		if ( sm_iNumActiveEmitters >= s_iMaxActiveEmitters )
		{
			if ( IsListEmpty( &m_oActive ) )
				break;

			AEnvEmitter* pEmitter = GetEmitterOfNode( m_oActive.Next() );

			if ( pEmitter->m_pParticleEffectInstance != TNULL )
			{
				pEmitter->m_pParticleEffectInstance->SetFinishedCallback( TNULL, TNULL );
				pEmitter->m_pParticleEffectInstance->Stop();
				pEmitter->m_pParticleEffectInstance = TNULL;
				sm_iNumActiveEmitters--;
			}

			pEmitter->m_Node.Remove();
			pEmitter->m_Node.InsertBefore( &m_oInactive );
		}

		StartEmitter( m_apQueued1[ i ], &m_oActive1 );
		m_apQueued1[ i ]->m_uiFlags &= ~AEnvEmitter::FLAGS_Queued;
	}

	for ( ; i < iNumQueued1; i++ )
		m_apQueued1[ i ]->m_uiFlags &= ~AEnvEmitter::FLAGS_Queued;

	// The others while instances are left
	i = 0;

	for ( ; i < iNumQueued; i++ )
	{
		if ( sm_iNumActiveEmitters >= s_iMaxActiveEmitters )
			break;

		StartEmitter( m_apQueued[ i ], &m_oActive );
		m_apQueued[ i ]->m_uiFlags &= ~AEnvEmitter::FLAGS_Queued;
	}

	for ( ; i < iNumQueued; i++ )
		m_apQueued[ i ]->m_uiFlags &= ~AEnvEmitter::FLAGS_Queued;
}

// $deBlob: FUNCTION 005e9c90
TBOOL __stdcall AEnvParticles::StartEmitter( AEnvEmitter* a_pEmitter, T2GenericDList::Node* a_pList )
{
	TASSERT( a_pEmitter->m_pParticleEffectInstance == TNULL );

	TParticleEffectInstance* pInstance = TParticleSystem::GetSingleton()->GetEffectManager()->StartEffect( a_pEmitter->m_pEffect );

	if ( pInstance == TNULL )
		return TFALSE;

	pInstance->m_oTransform = a_pEmitter->m_mTransform;
	pInstance->SetFinishedCallback( &AEnvParticles::OnEffectFinished, a_pEmitter );
	a_pEmitter->m_pParticleEffectInstance = pInstance;

	a_pEmitter->m_Node.Remove();
	a_pEmitter->m_Node.InsertBefore( a_pList );
	sm_iNumActiveEmitters++;

	return TTRUE;
}

// $deBlob: FUNCTION 005e93b0
AEnvEmitter* AEnvParticles::AddEmitter( TParticleEffect* a_pEffect, const TMatrix44& a_rTransform, TFLOAT a_fRadius, TUINT32 a_uiFlags )
{
	if ( IsListEmpty( &m_oFree ) )
		return TNULL;

	TFLOAT fRadius = a_fRadius;

	if ( fRadius <= 0.0f )
	{
		for ( TINT i = 0; i < a_pEffect->GetNumSystems(); i++ )
		{
			TGenericParticlesSettings* pSettings = a_pEffect->GetSystem( i ).m_pSettings;

			if ( !pSettings->m_bDynamicRadius && pSettings->m_fRadius > fRadius )
				fRadius = pSettings->m_fRadius;
		}
	}

	TASSERT( fRadius > 0.0f );
	TASSERT( fRadius < 50.0f );
	TASSERT( !IsListEmpty( &m_oFree ) );

	// The last free emitter
	AEnvEmitter* pEmitter = GetEmitterOfNode( m_oFree.Prev() );
	pEmitter->m_Node.Remove();

	pEmitter->m_pEffect                 = a_pEffect;
	pEmitter->m_pParticleEffectInstance = TNULL;
	pEmitter->m_mTransform              = a_rTransform;
	pEmitter->m_uiFlags                 = a_uiFlags;
	pEmitter->m_fRadius                 = fRadius;

	pEmitter->m_Node.InsertBefore( &m_oInactive );
	m_pHash->Insert( pEmitter );
	m_iNumEmitters++;

	return pEmitter;
}

// $deBlob: FUNCTION 005e9370
AEnvEmitter* AEnvParticles::AddEmitter( const TCHAR* a_szEffect, const TMatrix44& a_rTransform, TFLOAT a_fRadius, TUINT32 a_uiFlags )
{
	TParticleEffect* pEffect = TParticleSystem::GetSingleton()->GetEffectManager()->FindEffect( a_szEffect );

	if ( pEffect == TNULL )
		return TNULL;

	return AddEmitter( pEffect, a_rTransform, a_fRadius, a_uiFlags );
}

// $deBlob: FUNCTION 005e95a0
void AEnvParticles::RemoveEmitter( AEnvEmitter* a_pEmitter, TBOOL a_bInstant )
{
	TASSERT( a_pEmitter != TNULL );

	StopEmitter( a_pEmitter, a_bInstant );
	m_pHash->Remove( a_pEmitter );

	a_pEmitter->m_Node.Remove();
	a_pEmitter->m_Node.InsertBefore( &m_oFree );
	m_iNumEmitters--;
}

// $deBlob: FUNCTION 005e9640
void AEnvParticles::SetEmitterTransform( AEnvEmitter* a_pEmitter, const TMatrix44& a_rTransform )
{
	m_pHash->Remove( a_pEmitter );

	a_pEmitter->m_mTransform = a_rTransform;

	if ( a_pEmitter->m_pParticleEffectInstance != TNULL )
		a_pEmitter->m_pParticleEffectInstance->m_oTransform = a_rTransform;

	m_pHash->Insert( a_pEmitter );
}

// $deBlob: FUNCTION 005e8d20
// $deBlob: FUNCTION 005e99c0
void AEnvParticles::Reset()
{
	while ( !IsListEmpty( &m_oActive ) )
	{
		AEnvEmitter* pEmitter = GetEmitterOfNode( m_oActive.Prev() );

		if ( pEmitter->m_pParticleEffectInstance != TNULL )
		{
			pEmitter->m_pParticleEffectInstance->SetFinishedCallback( TNULL, TNULL );
			pEmitter->m_pParticleEffectInstance->StopInstant();
			pEmitter->m_pParticleEffectInstance = TNULL;
			sm_iNumActiveEmitters--;
		}

		pEmitter->m_Node.Remove();
		pEmitter->m_Node.InsertBefore( &m_oInactive );
	}

	while ( !IsListEmpty( &m_oActive1 ) )
	{
		AEnvEmitter* pEmitter = GetEmitterOfNode( m_oActive1.Prev() );

		if ( pEmitter->m_pParticleEffectInstance != TNULL )
		{
			pEmitter->m_pParticleEffectInstance->SetFinishedCallback( TNULL, TNULL );
			pEmitter->m_pParticleEffectInstance->StopInstant();
			pEmitter->m_pParticleEffectInstance = TNULL;
			sm_iNumActiveEmitters--;
		}

		pEmitter->m_Node.Remove();
		pEmitter->m_Node.InsertBefore( &m_oInactive );
	}

	TASSERT( sm_iNumActiveEmitters == 0 );
}

// $deBlob: FUNCTION 005e96f0
void AEnvParticles::OnEffectFinished( TParticleEffectInstance* a_pEffectInstance, void* a_pUserData )
{
	AEnvEmitter* pEmitter = TSTATICCAST( AEnvEmitter, a_pUserData );
	TASSERT( a_pEffectInstance == pEmitter->m_pParticleEffectInstance );

	pEmitter->m_pParticleEffectInstance = TNULL;
	pEmitter->m_Node.Remove();
	pEmitter->m_Node.InsertBefore( sm_pInactiveList );
	sm_iNumActiveEmitters--;
}

// $deBlob: FUNCTION 005e9780
void AEnvParticles::SetEmitterStamps( TUINT32 a_uiStamp )
{
	AEnvParticles* pThis = GetSingleton();

	for ( T2GenericDList::Node* pNode = pThis->m_oActive.Next(); pNode != &pThis->m_oActive; pNode = pNode->Next() )
		GetEmitterOfNode( pNode )->m_uiStamp = a_uiStamp;

	for ( T2GenericDList::Node* pNode = pThis->m_oActive1.Next(); pNode != &pThis->m_oActive1; pNode = pNode->Next() )
		GetEmitterOfNode( pNode )->m_uiStamp = a_uiStamp;

	for ( T2GenericDList::Node* pNode = pThis->m_oInactive.Next(); pNode != &pThis->m_oInactive; pNode = pNode->Next() )
		GetEmitterOfNode( pNode )->m_uiStamp = a_uiStamp;
}

// $deBlob: FUNCTION 005e9b70
void AEnvParticles::RemoveAllEmitters()
{
	while ( !IsListEmpty( &m_oActive ) )
		RemoveEmitter( GetEmitterOfNode( m_oActive.Prev() ), TTRUE );

	while ( !IsListEmpty( &m_oActive1 ) )
		RemoveEmitter( GetEmitterOfNode( m_oActive1.Prev() ), TTRUE );

	while ( !IsListEmpty( &m_oInactive ) )
		RemoveEmitter( GetEmitterOfNode( m_oInactive.Prev() ), TTRUE );
}

// $deBlob: FUNCTION 005e9d50
void AEnvParticles::StopEmitter( AEnvEmitter* a_pEmitter, TBOOL a_bInstant )
{
	if ( a_pEmitter->m_pParticleEffectInstance != TNULL )
	{
		a_pEmitter->m_pParticleEffectInstance->SetFinishedCallback( TNULL, TNULL );

		if ( a_bInstant )
			a_pEmitter->m_pParticleEffectInstance->StopInstant();
		else
			a_pEmitter->m_pParticleEffectInstance->Stop();

		a_pEmitter->m_pParticleEffectInstance = TNULL;
		sm_iNumActiveEmitters--;
	}

	a_pEmitter->m_Node.Remove();
	a_pEmitter->m_Node.InsertBefore( &m_oInactive );
}
