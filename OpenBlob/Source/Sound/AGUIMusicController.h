#pragma once
#include <Toshi/T2DList.h>
#include <XUI/TXUIScene.h>
#include <XUI/TXUITimelineControl.h>

class ABlobMenu;

//-----------------------------------------------------------------------------
// AGUIMusicController (0x468): plays the named frames of the menu scenes in time with the music (created by
// AMusicManager's ctor). A registered scene's named frames "beat_*", "bar_*", "melody_*" and "scratch_*" are played (up
// to their "end_*" frame) one after the other on the beats, bars, melody changes and scratches of the music stream. The
// beat scene (an ABlobMenu, set by the menus) gets a callback on each beat.
//-----------------------------------------------------------------------------
class AGUIMusicController
{
public:
	// A named frame of a registered scene (0x14)
	struct Animation : public Toshi::T2DList<Animation>::Node
	{
		// $deBlob: FUNCTION 00603f90
		Animation()
		{
			m_pScene      = TNULL;
			m_pStartFrame = TNULL;
			m_pEndFrame   = TNULL;
		}

		// $deBlob: FUNCTION 00603fb0
		void Reset()
		{
			m_pScene      = TNULL;
			m_pStartFrame = TNULL;
			m_pEndFrame   = TNULL;
		}

		Toshi::TXUIScene*                       m_pScene;      // 0x08
		Toshi::TXUITimelineControl::NamedFrame* m_pStartFrame; // 0x0C
		Toshi::TXUITimelineControl::NamedFrame* m_pEndFrame;   // 0x10 the "end_" frame, TNULL: none
	};

	// The animation lists of the scene
	enum ANIMTYPE
	{
		ANIMTYPE_BEAT,
		ANIMTYPE_BAR,
		ANIMTYPE_MELODY,
		ANIMTYPE_SCRATCH,
		ANIMTYPE_NUMOF
	};

	// A registered scene (0x3C)
	struct Scene : public Toshi::T2DList<Scene>::Node
	{
		// $deBlob: FUNCTION 00603fd0
		Scene()
		{
			m_pScene = TNULL;

			for ( TINT i = 0; i < ANIMTYPE_NUMOF; i++ )
				m_aiNumAnimations[ i ] = 0;

			for ( TINT i = 0; i < ANIMTYPE_NUMOF; i++ )
				m_aiCurrentAnimation[ i ] = 0;
		}

		// $deBlob: FUNCTION 00604a50
		~Scene() = default;

		// $deBlob: FUNCTION 00604060
		TINT Reset()
		{
			m_pScene = TNULL;

			for ( TINT i = 0; i < ANIMTYPE_NUMOF; i++ )
				m_aiNumAnimations[ i ] = 0;

			for ( TINT i = 0; i < ANIMTYPE_NUMOF; i++ )
				m_aiCurrentAnimation[ i ] = 0;

			return 0;
		}

		Toshi::T2DList<Animation> m_aAnimations[ ANIMTYPE_NUMOF ];        // 0x08 beat, bar, melody, scratch
		Toshi::TXUIScene*         m_pScene;                               // 0x28
		TINT16                    m_aiNumAnimations[ ANIMTYPE_NUMOF ];    // 0x2C
		TINT16                    m_aiCurrentAnimation[ ANIMTYPE_NUMOF ]; // 0x34
	};

	static TCONSTEXPR_VAR TINT MAX_ANIMATIONS = 25;
	static TCONSTEXPR_VAR TINT MAX_SCENES     = 10;

public:
	AGUIMusicController();
	~AGUIMusicController();

	// Adds the "beat_", "bar_", "melody_" and "scratch_" named frames of a_pScene (once)
	void RegisterScene( Toshi::TXUIScene* a_pScene );
	void UnregisterScene( Toshi::TXUIScene* a_pScene );
	void UnregisterAllScenes();

	// Plays the next animation of each registered scene
	void OnBeat();
	void OnBar();
	void OnScratch();
	void OnMelodyChange();

	// $deBlob: FUNCTION 006047f0
	static void PlayAnimation( Animation* a_pAnimation )
	{
		const TWCHAR* wszEnd = TNULL;

		if ( a_pAnimation->m_pEndFrame != TNULL )
			wszEnd = a_pAnimation->m_pEndFrame->m_wszName;

		a_pAnimation->m_pScene->GetTimelineControl().PlayAnimation( a_pAnimation->m_pStartFrame->m_wszName, wszEnd );
	}

	// $deBlob: FUNCTION 0062aef0
	ABlobMenu* GetBeatScene() const { return m_pBeatScene; }
	// $deBlob: FUNCTION 0062afd0
	void SetBeatScene( ABlobMenu* a_pScene ) { m_pBeatScene = a_pScene; }

private:
	// The "end_<name>" frame of the named frame a_szName (up to its '#') in a_rNamedFrames
	static Toshi::TXUITimelineControl::NamedFrame* FindEndFrame( const TCHAR* a_szName, Toshi::T2DList<Toshi::TXUITimelineControl::NamedFrame>& a_rNamedFrames );

	// Moves the animations of a_rList to a_rFreeList
	static void FreeAnimations( Toshi::T2DList<Animation>& a_rList, Toshi::T2DList<Animation>& a_rFreeList );

	// Plays the a_rCurrent-th animation of a_rList and advances a_rCurrent (modulo a_iNumAnimations)
	static void PlayNextAnimation( Toshi::T2DList<Animation>& a_rList, TINT16& a_rCurrent, TINT16 a_iNumAnimations );

private:
	Animation                 m_aAnimations[ MAX_ANIMATIONS ]; // 0x000
	Scene                     m_aScenes[ MAX_SCENES ];         // 0x1F4
	Toshi::T2DList<Animation> m_oFreeAnimations;               // 0x44C
	Toshi::T2DList<Scene>     m_oFreeScenes;                   // 0x454
	Toshi::T2DList<Scene>     m_oScenes;                       // 0x45C
	ABlobMenu*                m_pBeatScene;                    // 0x464
};
