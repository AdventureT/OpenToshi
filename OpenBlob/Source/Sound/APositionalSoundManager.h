#pragma once
#include <Toshi/TSingleton.h>
#include <Toshi/T2DList.h>
#include <Math/TVector4.h>

namespace FMOD
{
class Event;
}

class ACellBucket;

namespace TerrainTypes
{
struct Cell;
}

//-----------------------------------------------------------------------------
// Purpose: the positional (3D) sounds of the game: looping FMOD events attached to entities, terrain instances or
// positions and the level's sound emitters (the "SoundEmitterHeader" of the level's sound TRB). The sounds are grouped
// by event (EventDesc); each update the ones closest to the camera are played, up to the event's "max playbacks", and
// follow their target.
//-----------------------------------------------------------------------------
class APositionalSoundManager
    : public Toshi::TSingleton<APositionalSoundManager>
{
public:
	static TCONSTEXPR_VAR TINT MAX_POSITIONALS = 256;
	static TCONSTEXPR_VAR TINT MAX_EVENTDESCS  = 128;

	enum FLAGS : TUINT16
	{
		FLAG_Playing        = 0x0001,
		FLAG_Stopped        = 0x0002,
		FLAG_WantsToPlay    = 0x0004,
		FLAG_Disabled       = 0x0020, // never played (a disabled emitter)
		FLAG_Restart        = 0x0040,
		FLAG_EntityPosition = 0x0080, // the target is an entity: its matrix translation (0x40)
		FLAG_OwnPosition    = 0x0100, // m_vPosition
		FLAG_MatrixPtr      = 0x0200, // the target points to a matrix (a terrain cell instance)
		FLAG_Static         = 0x0400, // a level emitter: the data is the TRB's, not returned to the free list
		FLAG_Unk800         = 0x0800,
		FLAG_Pitched        = 0x1000, // stopped by StopPitchedSounds (AMusicManager's pitch changes)
		FLAG_TargetPosition = 0x2000, // the target's position at 0x18 (velocity: 0x28 * 2)
		FLAG_EntityVelocity = 0x4000, // the target's velocity at 0x134
		FLAG_Emitter8000    = 0x8000, // set in the TRB: the emitter starts disabled
	};

	// The data of a positional sound (0x30 bytes; the level emitters use the TRB's records of the same layout)
	struct SoundData
	{
		Toshi::TVector4 m_vPosition;   // 0x00 FLAG_OwnPosition
		void*           m_pTarget;     // 0x10
		FMOD::Event*    m_pEvent;      // 0x14
		TUINT32         m_uiUnk18;     // 0x18
		TFLOAT          m_fDistanceSq; // 0x1C to the camera
		TINT16          m_iEventID;    // 0x20
		TINT16          m_iGroupID;    // 0x22
		TUINT16         m_uiFlags;     // 0x24 FLAGS
		TUINT16         m_uiUnk26;     // 0x26
		TINT            m_iUnk28;      // 0x28 -1
		TUINT32         m_uiUnk2C;     // 0x2C
	};

	// A positional sound (0xC bytes)
	struct Positional
	    : public Toshi::T2DList<Positional>::Node
	{
		// $deBlob: FUNCTION 0060e930
		Positional()
		    : m_pData( TNULL )
		{}

		// $deBlob: FUNCTION 0060e940
		// $deBlob: FUNCTION 00610f30
		~Positional()
		{
			TFree( m_pData );
			m_pData = TNULL;
		}

		SoundData* m_pData; // 0x08
	};

	// The positional sounds of one event (0x18 bytes)
	struct EventDesc
	    : public Toshi::T2DList<EventDesc>::Node
	{
		// $deBlob: FUNCTION 00610b40
		// $deBlob: FUNCTION 00610ca0
		// $deBlob: FUNCTION 00610ed0
		EventDesc() = default;

		TBOOL HasTarget( void* a_pTarget, TINT a_iGroupID, TINT a_iEventID );
		TBOOL StopInstance( Positional* a_pPositional );
		TBOOL StartInstance( TINT a_iGroupID, TINT a_iEventID, Positional* a_pPositional );
		TBOOL SetPitch( void* a_pTarget, TFLOAT a_fPitch );
		TBOOL Remove( void* a_pTarget, Toshi::T2DList<Positional>* a_pFreeList );
		void  Update();
		// Starts again the sounds with FLAG_Restart and any of a_uiFlags
		void RestartSounds( TUINT16 a_uiFlags );

		Toshi::T2DList<Positional> m_oPositionalList; // 0x08
		TINT16                     m_iGroupID;        // 0x10
		TINT16                     m_iEventID;        // 0x12
		TINT16                     m_iMaxInstances;   // 0x14 the event's max playbacks
		TINT16                     m_iNumInstaces;    // 0x16 playing
	};

	// The "SoundEmitterHeader" of the level's sound TRB
	struct SoundEmitterHeader
	{
		SoundData* m_pEmitters;    // 0x00
		TINT       m_iNumEmitters; // 0x04
	};

public:
	APositionalSoundManager();
	~APositionalSoundManager();

	// Attach a looping event to a target (see FLAGS for what each kind follows)
	TBOOL AddEntitySound( void* a_pEntity, TINT a_iGroupID, TINT a_iEventID );
	TBOOL AddMovingEntitySound( void* a_pEntity, TINT a_iGroupID, TINT a_iEventID );
	TBOOL AddMatrixSound( void* a_pTarget, TINT a_iGroupID, TINT a_iEventID );
	TBOOL AddTransformSound( void* a_pTarget, TINT a_iGroupID, TINT a_iEventID );
	TBOOL AddTargetSound( void* a_pTarget, TINT a_iGroupID, TINT a_iEventID );
	TBOOL AddPositionSound( const Toshi::TVector4* a_pPosition, TINT a_iGroupID, TINT a_iEventID );

	// The pitched variants (FLAG_Pitched: stopped by StopPitchedSounds)
	TBOOL AddPitchedPositionSound( const Toshi::TVector4* a_pPosition, TINT a_iGroupID, TINT a_iEventID, Positional** a_ppOutPositional );
	TBOOL AddPitchedEntitySound( void* a_pEntity, TINT a_iGroupID, TINT a_iEventID );

	// Stops and removes the sound of the event attached to a_pTarget
	TBOOL RemoveSound( void* a_pTarget, TINT a_iGroupID, TINT a_iEventID );

	// NOTE: retail passes ALoadLevelState's level properties, unused
	TBOOL LoadEmitters( void* a_pUnused );
	void  Reset();
	void  Update();
	void  StopPitchedSounds();
	// Starts again the sounds with FLAG_Restart and any of a_uiFlags (EventDesc::RestartSounds)
	void RestartSounds( TUINT16 a_uiFlags );
	// Asserts that no event has positional sounds left
	void CheckNoPositionals();
	TINT GetNumSounds();
	TINT GetNumFree();
	void SetPaused( TBOOL a_bPaused );
	void ResetEmitters();
	void SetEmitterDisabled( TINT a_iIndex, TBOOL a_bDisabled );
	void SetPitch( void* a_pTarget, TINT a_iGroupID, TINT a_iEventID, TFLOAT a_fPitch );

	// Called by AInGameState::OnInsertion; empty in retail.
	// $deBlob: FUNCTION 006100b0
	void OnLevelStarted() {}

	void OnCellLoaded( TerrainTypes::Cell* a_pCell );
	void OnCellUnloaded( TerrainTypes::Cell* a_pCell );

	// The terrain's cell unloaded callback
	static void OnCellUnloadedCallback( ACellBucket* a_pBucket );

private:
	TBOOL       AddEmitter( Positional* a_pPositional, SoundData* a_pData );
	TBOOL       AllocPositional( Positional*& a_rpPositional, TINT a_iGroupID, TINT a_iEventID, void* a_pTarget );
	Positional* PopFreePositional();
	EventDesc*  GetEventDesc( TINT a_iGroupID, TINT a_iEventID );
	void        UpdatePositions();

private:
	Toshi::T2DList<EventDesc>  m_oActivePosDesc;   // 0x00
	Toshi::T2DList<EventDesc>  m_oFreeDescs;       // 0x08
	Toshi::T2DList<Positional> m_oFreePositionals; // 0x10
	Positional*                m_pPositionals;     // 0x18 MAX_POSITIONALS
	Positional*                m_pEmitters;        // 0x1C one per emitter of m_pSoundTRBData
	SoundEmitterHeader*        m_pSoundTRBData;    // 0x20
	EventDesc*                 m_pEventDescs;      // 0x24 MAX_EVENTDESCS
	TUINT32                    m_uiFlags;          // 0x28 1: paused
};
