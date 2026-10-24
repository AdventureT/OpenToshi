#pragma once
#include <Render/TModel.h>

class AModelInstance2;
class AModel2;

namespace FMOD
{
class Event;
}

//-----------------------------------------------------------------------------
// Purpose: the breakpoint sounds of a model instance. The instance's animation names of the level's sound TRB ("Main"
// symbol) are mapped to the skeleton's sequence IDs (PatchAnimIDs), the sounds play while the animation runs (TickImp).
// The vtable has only the destructor.
//-----------------------------------------------------------------------------
class ASoundBreakpointComponent
{
public:
	static TCONSTEXPR_VAR TUINT16 FLAGS_MUTED = 1 << 0; // m_uiFlags, set by SetMuted

	// A record of a sub entry (0x10 bytes): a sound that plays while the sub entry's sequence runs
	struct BreakpointRecord
	{
		TFLOAT  m_fMaxDistSq; // 0x00 squared camera distance limit (unless FLAG_NO_DISTANCE)
		TFLOAT  m_fNormTime;  // 0x04 the play time inside the sequence, as a fraction of its duration
		TUINT16 m_uiFlags;    // 0x08 bit 0x01 plays with the total time, 0x02/0x08/0x10 tracked, 0x10 moves with the
		                      // instance, 0x20 no distance check and no position
		TINT16 m_iEventName;  // 0x0A index into the table's names (event)
		TINT16 m_iGroupName;  // 0x0C index into the table's names (group)
		                      // 0x0E padding
	};
	TSTATICASSERT( sizeof( BreakpointRecord ) == 0x10 );

	// A sound playing for a breakpoint (0x10 bytes), the empty one is (-1, -1, TNULL, -1, -1)
	struct BreakpointSound
	{
		BreakpointSound()
		    : m_iRecordIndex( -1 )
		    , m_iEntryIndex( -1 )
		    , m_pEvent( TNULL )
		    , m_iGroupName( -1 )
		    , m_iEventName( -1 )
		{
		}

		BreakpointSound( TINT a_iRecordIndex, TINT a_iEntryIndex, FMOD::Event* a_pEvent, TINT16 a_iGroupName, TINT16 a_iEventName )
		    : m_iRecordIndex( a_iRecordIndex )
		    , m_iEntryIndex( a_iEntryIndex )
		    , m_pEvent( a_pEvent )
		    , m_iGroupName( a_iGroupName )
		    , m_iEventName( a_iEventName )
		{
		}

		TINT         m_iRecordIndex; // 0x00 record of the sub entry
		TINT         m_iEntryIndex;  // 0x04 sub entry index
		FMOD::Event* m_pEvent;       // 0x08
		TINT16       m_iGroupName;   // 0x0C the record's group name (the names are compared by CheckSound)
		TINT16       m_iEventName;   // 0x0E the record's event name
	};

	// An entry of the breakpoint table (0x08 bytes): the animation name and its sub-entries
	struct BreakpointEntry
	{
		// A sub-entry (0x0C bytes): the sequence ID is looked up from the name by PatchAnimIDs
		struct SubEntry
		{
			TINT              m_iNumRecords; // 0x00
			BreakpointRecord* m_pRecords;    // 0x04
			TINT16            m_iSequenceID; // 0x08 set by PatchAnimIDs
			TINT16            m_iNameIndex;  // 0x0A into the table's names
		};

		SubEntry* m_pSubEntries;    // 0x00
		TINT16    m_iNumSubEntries; // 0x04
		TINT16    m_iNameIndex;     // 0x06 into the table's names
	};

	// The "Main" symbol of the level's sound breakpoint TRB (0x0C bytes)
	struct BreakpointTable
	{
		TINT16           m_iNumEntries; // 0x00
		TINT16           m_iNumNames;   // 0x02
		BreakpointEntry* m_pEntries;    // 0x04
		const TCHAR**    m_ppNames;     // 0x08
	};

public:
	ASoundBreakpointComponent( AModelInstance2* a_pModelInstance );
	ASoundBreakpointComponent( Toshi::TModelInstance* a_pModelInstance, const TCHAR* a_szName );
	virtual ~ASoundBreakpointComponent(); // +0x00

	// Plays and tracks the sounds of the entry's running sequences (the frame update of the instance)
	void TickImp();

	// Maps the animation names of the entry to sequence IDs of the model's skeleton; a non-null instance replaces
	// m_pModelInstance.
	void PatchAnimIDs( Toshi::TModelInstance* a_pModelInstance );

	// TTRUE stops the sounds and clears them, and sets FLAGS_MUTED; TFALSE clears FLAGS_MUTED.
	void SetMuted( TBOOL a_bMuted );

	// The type of the ground the owner stands on (ACharEntity::Tick)
	void SetSurfaceType( TUINT16 a_uiSurfaceType ) { m_uiSurfaceType = a_uiSurfaceType; }

	// Stops every sound and clears the vector (the destructor and SetMuted).
	void StopAndClearSounds();

	// Clears the vector (the elements are reset to empty).
	void ClearSounds();

	// TRUE when the model's name has a breakpoint entry in the table
	TBOOL HasEntry() const { return m_pEntry != TNULL; }

	// Finds the breakpoint entry of a name (case-insensitive, the name may be up to 4 characters longer), TNULL when
	// there is none. Also looks up the instance animations of ATerrain2Interface (the same "Main" table).
	static BreakpointEntry* FindEntry( const BreakpointTable* a_pTable, const TCHAR* a_szName );

private:
	// Resets the vector to empty.
	void InitSounds();

	// The name of an index of the table's names (TNULL when the index is out of the table).
	const TCHAR* GetName( TINT16 a_iIndex ) const;

	// The instance's position: m_pPosition when set, otherwise the row 3 of the instance's matrix.
	Toshi::TVector4* GetPosition();

	// TRUE when no sound with the names (a_iGroupName, a_iEventName) is playing.
	TBOOL CheckSound( TINT16 a_iGroupName, TINT16 a_iEventName ) const;

	// Moves the sounds of the record (a_iEntryIndex, a_iRecordIndex) to the position of the instance (the record's
	// flags 0x10). Second overload: the one sound.
	void UpdateTrackingSounds( TINT a_iEntryIndex, TINT a_iRecordIndex );
	void UpdateTrackingSounds( TINT a_iEntryIndex, TINT a_iRecordIndex, const BreakpointSound* a_pSound );

	// Removes the sound at the index; the sounds after it move down (the T2Vector erase).
	void EraseSound( TINT a_iIndex );

	// Appends a sound (the vector is never full here: the callers check the count).
	void PushBackSound( const BreakpointSound& a_rSound );

private:
	Toshi::TModelInstance* m_pModelInstance; // 0x04

public:
	TUINT8* m_pPosition; // 0x08 the position (a matrix row 3, set by ATerrain2Interface); TNULL: the instance's own

private:
	const TCHAR*     m_szName;        // 0x0C the model's name
	TUINT16          m_uiSurfaceType; // 0x10 1 after the constructors, the ground's surface type (ACharEntity::Tick)
	TUINT16          m_uiFlags;       // 0x12 FLAGS_MUTED
	BreakpointTable* m_pTable;        // 0x14 the "Main" symbol of the level's sound TRB
	BreakpointEntry* m_pEntry;        // 0x18 the entry of m_szName (TNULL: none)
	TINT             m_iNumSounds;    // 0x1C the vector's count
	BreakpointSound  m_aSounds[ 5 ];  // 0x20 T2Vector<BreakpointSound,5> storage
};                                    // 0x70
