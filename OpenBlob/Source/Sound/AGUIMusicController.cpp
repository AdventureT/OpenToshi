#include "pch.h"
#include "AGUIMusicController.h"
#include "XUI/ABlobMenu.h"

#include <XUI/TXUI.h>
#include <Toshi/T2String8.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $deBlob: FUNCTION 00603dd0
AGUIMusicController::AGUIMusicController()
{
	m_pBeatScene = TNULL;

	for ( TINT i = 0; i < MAX_ANIMATIONS; i++ )
		m_oFreeAnimations.PushBack( &m_aAnimations[ i ] );

	for ( TINT i = 0; i < MAX_SCENES; i++ )
		m_oFreeScenes.PushBack( &m_aScenes[ i ] );
}

// $deBlob: FUNCTION 00603ed0
// $deBlob: FUNCTION 0060c1a0
AGUIMusicController::~AGUIMusicController()
{
	m_oFreeAnimations.Clear();
	m_oFreeScenes.Clear();
}

// $deBlob: FUNCTION 00604080
void AGUIMusicController::RegisterScene( TXUIScene* a_pScene )
{
	for ( auto it = m_oScenes.Begin(); it != m_oScenes.End(); it++ )
	{
		if ( it->m_pScene == a_pScene )
			return;
	}

	if ( m_oFreeScenes.IsEmpty() )
		return;

	TINT   iNumAnimations = 0;
	Scene* pScene         = m_oFreeScenes.Head();
	pScene->Remove();
	pScene->Reset();

	T2DList<TXUITimelineControl::NamedFrame>& rNamedFrames = a_pScene->GetTimelineControl().m_oNamedFrames;

	for ( auto it = rNamedFrames.Begin(); it != rNamedFrames.End(); it++ )
	{
		if ( it.Get() == TNULL || m_oFreeAnimations.IsEmpty() )
			continue;

		Animation* pAnimation = m_oFreeAnimations.Head();
		pAnimation->Remove();
		pAnimation->Reset();

		TCHAR szName[ 256 ];
		TStringManager::StringUnicodeToChar( szName, it->m_wszName, 256 );

		T2DList<Animation>* pList;

		if ( TStringManager::String8CompareNoCase( "beat_", szName, 5 ) == 0 )
			pList = &pScene->m_aAnimations[ ANIMTYPE_BEAT ];
		else if ( TStringManager::String8CompareNoCase( "melody_", szName, 7 ) == 0 )
			pList = &pScene->m_aAnimations[ ANIMTYPE_MELODY ];
		else if ( TStringManager::String8CompareNoCase( "bar_", szName, 4 ) == 0 )
			pList = &pScene->m_aAnimations[ ANIMTYPE_BAR ];
		else if ( TStringManager::String8CompareNoCase( "scratch_", szName, 8 ) == 0 )
			pList = &pScene->m_aAnimations[ ANIMTYPE_SCRATCH ];
		else
		{
			m_oFreeAnimations.PushFront( pAnimation );
			continue;
		}

		pList->PushBack( pAnimation );
		iNumAnimations++;
		pAnimation->m_pScene      = a_pScene;
		pAnimation->m_pStartFrame = it;
		pAnimation->m_pEndFrame   = FindEndFrame( szName, rNamedFrames );
	}

	if ( iNumAnimations > 0 )
	{
		pScene->m_pScene = a_pScene;

		for ( TINT i = 0; i < ANIMTYPE_NUMOF; i++ )
			pScene->m_aiNumAnimations[ i ] = TINT16( pScene->m_aAnimations[ i ].Size() );

		m_oScenes.PushBack( pScene );
	}
	else
	{
		m_oFreeScenes.PushFront( pScene );
	}
}

// $deBlob: FUNCTION 006043c0
void AGUIMusicController::UnregisterScene( TXUIScene* a_pScene )
{
	for ( auto it = m_oScenes.Begin(); it != m_oScenes.End(); )
	{
		Scene* pScene = it;
		it++;

		if ( pScene->m_pScene == a_pScene )
		{
			for ( TINT i = 0; i < ANIMTYPE_NUMOF; i++ )
				FreeAnimations( pScene->m_aAnimations[ i ], m_oFreeAnimations );

			pScene->Remove();
			m_oFreeScenes.PushBack( pScene );
		}
	}
}

// $deBlob: FUNCTION 006044c0
void AGUIMusicController::UnregisterAllScenes()
{
	for ( auto it = m_oScenes.Begin(); it != m_oScenes.End(); )
	{
		Scene* pScene = it;

#ifdef DEBLOB_COMMUNITY_PATCH
		// NOTE: retail reads the next scene after moving the scene to the free list (so it walks the free list and
		// never gets back to the end of m_oScenes)
		it++;
#endif

		for ( TINT i = 0; i < ANIMTYPE_NUMOF; i++ )
			FreeAnimations( pScene->m_aAnimations[ i ], m_oFreeAnimations );

		pScene->Remove();
		m_oFreeScenes.PushBack( pScene );

#ifndef DEBLOB_COMMUNITY_PATCH
		it++;
#endif
	}
}

// $deBlob: FUNCTION 006045b0
void AGUIMusicController::OnBeat()
{
	for ( auto it = m_oScenes.Begin(); it != m_oScenes.End(); it++ )
		PlayNextAnimation( it->m_aAnimations[ ANIMTYPE_BEAT ], it->m_aiCurrentAnimation[ ANIMTYPE_BEAT ], it->m_aiNumAnimations[ ANIMTYPE_BEAT ] );

	if ( m_pBeatScene != TNULL )
	{
		TXUI* pXUI = TXUI::GetSingleton();

		if ( pXUI->GetTransToScene() == TNULL && pXUI->GetTransBackFromScene() == TNULL && !pXUI->IsInputLocked() )
		{
			// NOTE: vtable slot +0x134 of ABlobMenu (the beat callback of the beat scene)
			m_pBeatScene->OnAttractModeUpdate();
		}
	}
}

// $deBlob: FUNCTION 00604640
void AGUIMusicController::OnBar()
{
	for ( auto it = m_oScenes.Begin(); it != m_oScenes.End(); it++ )
		PlayNextAnimation( it->m_aAnimations[ ANIMTYPE_BAR ], it->m_aiCurrentAnimation[ ANIMTYPE_BAR ], it->m_aiNumAnimations[ ANIMTYPE_BAR ] );
}

// $deBlob: FUNCTION 006046a0
void AGUIMusicController::OnScratch()
{
	for ( auto it = m_oScenes.Begin(); it != m_oScenes.End(); it++ )
		PlayNextAnimation( it->m_aAnimations[ ANIMTYPE_SCRATCH ], it->m_aiCurrentAnimation[ ANIMTYPE_SCRATCH ], it->m_aiNumAnimations[ ANIMTYPE_SCRATCH ] );
}

// $deBlob: FUNCTION 00604700
void AGUIMusicController::OnMelodyChange()
{
	for ( auto it = m_oScenes.Begin(); it != m_oScenes.End(); it++ )
		PlayNextAnimation( it->m_aAnimations[ ANIMTYPE_MELODY ], it->m_aiCurrentAnimation[ ANIMTYPE_MELODY ], it->m_aiNumAnimations[ ANIMTYPE_MELODY ] );
}

// $deBlob: FUNCTION 00604760
void AGUIMusicController::FreeAnimations( T2DList<Animation>& a_rList, T2DList<Animation>& a_rFreeList )
{
	for ( auto it = a_rList.Begin(); it != a_rList.End(); )
	{
		Animation* pAnimation = it;
		it++;

		pAnimation->Remove();
		a_rFreeList.PushBack( pAnimation );
	}
}

// $deBlob: FUNCTION 00604820
TXUITimelineControl::NamedFrame* AGUIMusicController::FindEndFrame( const TCHAR* a_szName, T2DList<TXUITimelineControl::NamedFrame>& a_rNamedFrames )
{
	// NOTE: the length of the name up to its '#', plus "end_" (retail doesn't check that the name has a '#')
	TINT iLength = TINT( TStringManager::String8FindChar( a_szName, '#' ) - a_szName ) + 4;

	TCHAR szEndName[ 256 ];
	T2String8::Format( szEndName, 256, "end_%s", a_szName );

	for ( auto it = a_rNamedFrames.Begin(); it != a_rNamedFrames.End(); it++ )
	{
		TCHAR szName[ 256 ];
		TStringManager::StringUnicodeToChar( szName, it->m_wszName, iLength );

		if ( TStringManager::String8CompareNoCase( szName, szEndName, iLength ) == 0 )
			return it;
	}

	return TNULL;
}

// $deBlob: FUNCTION 006048f0
void AGUIMusicController::PlayNextAnimation( T2DList<Animation>& a_rList, TINT16& a_rCurrent, TINT16 a_iNumAnimations )
{
	if ( a_iNumAnimations > 0 )
	{
		a_rCurrent %= a_iNumAnimations;

		auto it = a_rList.Begin();

		if ( it == a_rList.End() )
			return;

		for ( TINT i = 0; i != a_rCurrent; i++ )
		{
			it++;

			if ( it == a_rList.End() )
				return;
		}

		PlayAnimation( it );
		a_rCurrent++;
	}
}
