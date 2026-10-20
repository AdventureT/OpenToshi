#pragma once
#include <File/TTRB.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: the points of the level ("PointDataHeader" of the point data block: start positions...), the member of
// AGameStatus at 0xA18 (4 bytes).
//-----------------------------------------------------------------------------
class APointDataFunction
{
public:
	enum POINT_TYPE : TINT
	{
		POINT_DEFAULT          = 0,
		POINT_PLAYER_1_RESPAWN = 9, // the respawn points of the multiplayer players
		POINT_PLAYER_4_RESPAWN = 12,
		POINT_PLAYER_RESPAWN   = 13, // any player's
		POINT_MAX              = 15,
	};

	struct Point
	{
		TUINT8          m_aUnk00[ 0x10 ]; // 0x00
		Toshi::TVector4 m_vPosition;      // 0x10
		TFLOAT          m_aUnk20[ 2 ];    // 0x20 [0]: the types 9-13 set it to 1.0f/-1.0f (ResetUnk20)
		TUINT8          m_uiFlags;        // 0x28 1: a start point the blob is dropped onto (not lowered by the blob's radius)
		TUINT8          m_uiUnk29;        // 0x29
		TINT16          m_iType;          // 0x2A POINT_TYPE
		TINT            m_iUnk2C;         // 0x2C -1: none
	};                                    // 0x30

	struct PointDataHeader
	{
		Point* m_pPoints;    // 0x00
		TINT   m_iNumPoints; // 0x04
	};

public:
	APointDataFunction();
	~APointDataFunction();

	TBOOL Load( Toshi::TTRB* a_pTRB );

	// The first point of the type, TNULL if none
	Point* GetPoint( TINT a_itype );

	// The a_iNth (1-based) point of the type, TNULL if none
	Point* GetNthPoint( TINT a_itype, TINT a_iNth );

	// The last point of the type, TNULL if none
	Point* GetLastPoint( TINT a_itype );

	// The a_iIndex-th (0-based) point of the type, TNULL if none / the index of a_pPoint among the points of the type, -1
	// if it isn't one
	Point* GetPointByIndex( TINT a_iIndex, TINT a_itype );
	TINT   GetPointIndex( Point* a_pPoint, TINT a_itype );

	// The first (or a_iNth, 1-based) point of the type whose m_iUnk2C is a_iUnk2C, the last one for GetLastPointWithUnk2C
	Point* GetPointWithUnk2C( TINT a_itype, TINT a_iUnk2C );
	Point* GetLastPointWithUnk2C( TINT a_itype, TINT a_iUnk2C );
	Point* GetNthPointWithUnk2C( TINT a_itype, TINT a_iUnk2C, TINT a_iNth );

	// The point of the type nearest to (farthest from) a_rPosition, TNULL if none
	Point* GetNearestPoint( TINT a_itype, const Toshi::TVector4& a_rPosition );
	Point* GetFarthestPoint( TINT a_itype, const Toshi::TVector4& a_rPosition );

	// Multiplayer: the enabled respawn point of a_itype or POINT_PLAYER_RESPAWN nearest to a_rPosition, TNULL if none
	Point* GetNearestRespawnPoint( TINT a_itype, const Toshi::TVector4& a_rPosition );

	// Multiplayer: the enabled (m_aUnk20[ 0 ] > 0) respawn point of a_itype or POINT_PLAYER_RESPAWN farthest from
	// a_rPosition, TNULL if none
	Point* GetFarthestRespawnPoint( TINT a_itype, const Toshi::TVector4& a_rPosition );

	// Sets m_aUnk20[ 0 ] of the points of the types 9-13: 1.0f if m_iUnk2C is -1, else -1.0f
	void ResetUnk20();

	// Sets m_aUnk20[ 0 ] of the points of the types 9-13 with m_iUnk2C == a_iUnk2C to 1.0f (a_bSet) or -1.0f;
	void SetUnk20( TINT a_iUnk2C, TBOOL a_bSet );

	// Forgets the point data (the level unload, AGameStatus::ResetLevelReferences)
	// $deBlob: FUNCTION 005c0c90
	void Clear() { m_pHeader = TNULL; }

private:
	PointDataHeader* m_pHeader; // 0x00
};
