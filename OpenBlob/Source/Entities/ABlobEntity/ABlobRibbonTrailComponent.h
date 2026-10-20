#pragma once
#include <Math/TVector2.h>
#include <Math/TVector3.h>
#include <Math/TVector4.h>

class ABlobEntity;
class ARibbonTrailMaterial;
class ARibbonTrailMeshDynamic;
class ARibbonTrailMeshStatic;

//-----------------------------------------------------------------------------
// Purpose: Blob's paint trail (ABlobRibbonTrailComponent.cpp, ABlobEntity+0x3D0): a ribbon of up to 128 segments
// ("textaribbon.tga") left under Blob in its paint colour. A Hermite spline runs through the last three control points
// (every BlobRibbonProperties segment length), segments are added every m_fTesselation along it and the tail shrinks
// back when Blob slows down. A short arrow trail ("textaarrow.tga") and two static arrow meshes (the
// "BlobRibbonProperties" meshes) can be drawn too. On a hard landing (ABlobCollisionEventHandler) the trail follows one
// of three splat patterns for a while (state 1).
//-----------------------------------------------------------------------------
class ABlobRibbonTrailComponent
{
	friend class ABlobNavRingComponent;
	friend class ABlobChallengeScene;

public:
	static TCONSTEXPR_VAR TINT MAX_SEGMENTS     = 128;
	static TCONSTEXPR_VAR TINT NUM_ARROW_POINTS = 10;
	static TCONSTEXPR_VAR TINT MAX_SPLAT_POINTS = 10;

	enum STATE : TINT
	{
		STATE_TRAIL = 0,
		STATE_SPLAT = 1,
	};

	using FLAGS = TUINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_STARTED = BITFLAG( 0 ), // a control point was added
		FLAGS_VISIBLE = BITFLAG( 3 ),
		FLAGS_ARROW   = BITFLAG( 4 ), // the spinning arrow meshes
	};

	// A static mesh of BlobRibbonProperties (0x14 bytes)
	struct MeshData
	{
		Toshi::TVector3* pPositions;   // 0x00
		Toshi::TVector2* pUVs;         // 0x04
		TINT             iNumVertices; // 0x08
		TUINT16*         pIndices;     // 0x0C 3 TUINT16 per index
		TINT             iNumIndices;  // 0x10
	};

	// BlobProperties.trb "BlobRibbonProperties" (ABlobEntity::ms_pBlobRibbonProperties)
	struct Properties
	{
		TFLOAT   m_fSegmentLength;   // 0x00 the distance between the control points
		TFLOAT   m_fMaxSubdivisions; // 0x04 the segments per control point on straight lines
		TFLOAT   m_fMinSubdivisions; // 0x08 the segments per control point in turns
		TFLOAT   m_fShrinkSpeed;     // 0x0C
		TFLOAT   m_fTrailWidth;      // 0x10 times the size's m_fUnk04
		TFLOAT   m_fArrowWidth;      // 0x14 times the size's m_fUnk04
		TFLOAT   m_fHeight;          // 0x18 the distance to Blob's centre, plus its radius
		TFLOAT   m_fWobbleFrequency; // 0x1C
		TFLOAT   m_fWobbleSpeedMin;  // 0x20
		TFLOAT   m_fWobbleSpeedMax;  // 0x24
		TFLOAT   m_fSplatTime;       // 0x28
		MeshData m_oArrowBase;       // 0x2C
		MeshData m_oArrowHead;       // 0x40
	};                               // 0x54

	using t_ControlPointFunc = void ( ABlobRibbonTrailComponent::* )();

public:
	ABlobRibbonTrailComponent( ABlobEntity* a_pBlob );
	~ABlobRibbonTrailComponent();

	void Tick( TFLOAT a_fDeltaTime );
	void Render();

	// Sets the segments the trail keeps (0..MAX_SEGMENTS)
	TINT SetWantedSegments( TINT16 a_iNumSegments );

	// Moves every control point to a_rvPosition
	void SetPosition( const Toshi::TVector4& a_rvPosition );

	// Clears the trail (Blob respawns)
	void Reset();

	// Starts a splat pattern (a hard landing)
	void OnLanding( const Toshi::TVector4& a_rvDirection );

	// $deBlob: FUNCTION 007bdfa0
	TBOOL IsArrowVisible() const { return ( m_uiFlags & FLAGS_ARROW ) != 0; }
	// $deBlob: FUNCTION 007bdfb0
	TBOOL IsVisible() const { return ( m_uiFlags & FLAGS_VISIBLE ) != 0; }
	void  SetVisible( TBOOL a_bVisible )
	{
		if ( a_bVisible )
			m_uiFlags |= FLAGS_VISIBLE;
		else
			m_uiFlags &= ~FLAGS( FLAGS_VISIBLE );
	}

	TINT16 GetMinSegments() const { return m_iMinSegments; }

	static const Properties& GetProperties();

private:
	// Clears the segments, the trail starts at m_vLastPos
	void ResetTrail();

	// Adds a segment at the end of the trail
	void AddSegment();

	// The control point callbacks of Advance: the next point under Blob / the next splat point
	void AddTrailControlPoint();
	void AddSplatControlPoint();

	// Shrinks the tail of the trail
	void UpdateLength( TFLOAT a_fDeltaTime, TFLOAT a_fMoveDist );

	// Moves the head of the trail a_fDist along the spline
	void UpdateSpline( TFLOAT a_fLerpT );

	// Moves along the spline by a_fDist, adding segments and control points
	void Advance( TFLOAT a_fDist, t_ControlPointFunc a_fnAddControlPoint );

	// Turns the down vector around (the wobble of the trail)
	void UpdateWobble( TFLOAT a_fDeltaTime, TFLOAT a_fSpeed );

	// The splat patterns
	void CreateSplatSpiral( const Toshi::TVector4& a_rvDirection );
	void CreateSplatWobble( const Toshi::TVector4& a_rvDirection );
	void CreateSplatStar( const Toshi::TVector4& a_rvDirection );

private:
	ABlobEntity*             m_pBlob;                               // 0x000
	STATE                    m_eState;                              // 0x004
	FLAGS                    m_uiFlags;                             // 0x008
	Toshi::TVector4          m_avPoints[ 3 ];                       // 0x00C the control points, newest first
	Toshi::TVector4          m_avTangents[ 3 ];                     // 0x03C
	Toshi::TVector4          m_avSplatPoints[ MAX_SPLAT_POINTS ];   // 0x06C
	Toshi::TVector4          m_avSplatTangents[ MAX_SPLAT_POINTS ]; // 0x10C
	Toshi::TVector4          m_vSplinePos;                          // 0x1AC the head of the trail
	Toshi::TVector4          m_vSplineTangent;                      // 0x1BC
	Toshi::TVector4          m_vControlPos;                         // 0x1CC Blob's point on the spline
	Toshi::TVector4          m_vControlTangent;                     // 0x1DC
	Toshi::TVector4          m_vTailLeft;                           // 0x1EC the end of the tail
	Toshi::TVector4          m_vTailRight;                          // 0x1FC
	Toshi::TVector4          m_vLastPos;                            // 0x20C the point under Blob last frame
	Toshi::TVector4          m_vSide;                               // 0x21C the width direction of the trail
	Toshi::TVector4          m_vSideTarget;                         // 0x22C
	Toshi::TVector4          m_vLastDir;                            // 0x23C the direction of the head segment
	Toshi::TVector4          m_vArrowScale;                         // 0x24C
	Toshi::TVector4          m_vDown;                               // 0x25C from Blob's centre to the trail
	Toshi::TVector4          m_vMove;                               // 0x26C Blob's last move direction
	TFLOAT                   m_fWobbleAngle;                        // 0x27C
	TFLOAT                   m_fControlDist;                        // 0x280 the distance from the last control point
	TFLOAT                   m_fDist;                               // 0x284 the distance from the last segment
	TFLOAT                   m_fTesselation;                        // 0x288 the distance between the segments
	TFLOAT                   m_fShrink;                             // 0x28C the length to cut from the tail
	TFLOAT                   m_fArrowAngle;                         // 0x290
	TFLOAT                   m_fV;                                  // 0x294 the V of the head vertices (always 0)
	TFLOAT                   m_fLength;                             // 0x298 the length of the arrow trail
	TFLOAT                   m_fAlphaPulse;                         // 0x29C
	TFLOAT                   m_fAlphaPulseDir;                      // 0x2A0
	TUINT32                  m_uiColour;                            // 0x2A4 ARGB
	TINT16                   m_iNumSegments;                        // 0x2A8
	TINT16                   m_iMinSegments;                        // 0x2AA
	TINT16                   m_iTargetSegments;                     // 0x2AC
	TINT16                   m_iWantedSegments;                     // 0x2AE
	TINT16                   m_iSplatIndex;                         // 0x2B0
	TINT16                   m_iNumSplatPoints;                     // 0x2B2
	TUINT16                  m_uiLastVert;                          // 0x2B4 the first vertex of the head segment
	ARibbonTrailMaterial*    m_pTrailMaterial;                      // 0x2B8
	ARibbonTrailMeshDynamic* m_pTrailMesh;                          // 0x2BC
	ARibbonTrailMaterial*    m_pArrowMaterial;                      // 0x2C0
	ARibbonTrailMeshDynamic* m_pArrowMesh;                          // 0x2C4
	ARibbonTrailMeshStatic*  m_pArrowHeadMesh;                      // 0x2C8
	ARibbonTrailMeshStatic*  m_pArrowBaseMesh;                      // 0x2CC
};                                                                  // 0x2D0
