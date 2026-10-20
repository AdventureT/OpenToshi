#pragma once
#include <Toshi/TSingleton.h>
#include <Math/TVector4.h>

namespace Toshi
{
class TTRB;
}

//-----------------------------------------------------------------------------
// Purpose: the waypoint sets of the level (Projects\Entities\AWaypointData.h, 4 bytes; the "WaypointsMain" symbol of
// the Waypoints data block): the patrol routes of the enemies, the paths of the platforms, the routes of the
// civilians and the race markers. A set is a graph of nodes, a node links to other nodes of its set.
// Node
//-----------------------------------------------------------------------------
class AWaypointData
    : public Toshi::TSingleton<AWaypointData>
{
public:
	struct Link
	{
		enum FLAG_ : TUINT16
		{
			FLAG_Toggled = 0x1, // the link to follow (first one wins)
		};

		TINT16  iNode;   // 0x0 the linked node of the same set
		TUINT16 uiFlags; // 0x2 FLAG_
	};

	struct Set;

	struct Node
	{
		enum FLAG_ : TUINT16
		{
			FLAG_Toggled       = 0x1,  // toggled by the triggers (APropTrigger::ToggleWaypointSets)
			FLAG_Start         = 0x2,  // where a route starts
			FLAG_NavSets       = 0x4,  // reaching it toggles the platform's nav sets (APropPlatform)
			FLAG_EnableNavSets = 0x8,  // ...on (else off)
			FLAG_Trigger       = 0x10, // reaching it triggers the platform's targets
			FLAG_TurretChange  = 0x20, // reaching it calls APropPlatform::SetTurretActive( FLAG_TurretOn )
			FLAG_TurretOn      = 0x40,
		};

		Link& GetLink( TINT a_iLinkIndex )
		{
			TASSERT( a_iLinkIndex >= 0 && a_iLinkIndex < uiLinkCount );
			return pLinks[ a_iLinkIndex ];
		}

		// $deBlob: FUNCTION 0059ff90
		TUINT16 GetLinkCount() const { return uiLinkCount; }
		// The node index of a link
		// $deBlob: FUNCTION 0059fda0
		TINT16 GetLinkNode( TINT a_iLinkIndex ) const
		{
			TASSERT( a_iLinkIndex >= 0 && a_iLinkIndex < uiLinkCount );
			return pLinks[ a_iLinkIndex ].iNode;
		}

		// Whether a link has Link::FLAG_Toggled
		// $deBlob: FUNCTION 0059ff20
		TBOOL IsLinkFlagged( TINT a_iLinkIndex ) const
		{
			TASSERT( a_iLinkIndex >= 0 && a_iLinkIndex < uiLinkCount );
			return pLinks[ a_iLinkIndex ].uiFlags & Link::FLAG_Toggled;
		}

		// Returns the node a_iLinkIndex links to
		Node* GetLinkedNode( TINT a_iLinkIndex, Set* a_pSet );

		// Toggles FLAG_Toggled
		void ToggleFlag()
		{
			if ( !( uiFlags & FLAG_Toggled ) )
				uiFlags |= FLAG_Toggled;
			else
				uiFlags &= ~FLAG_Toggled;
		}

		Toshi::TVector4 vPosition;   // 0x00
		TFLOAT          fDelay;      // 0x10 the delay of a platform at the node (< 0: stops)
		TUINT16         uiFlags;     // 0x14 FLAG_
		TUINT16         uiLinkCount; // 0x16
		Link*           pLinks;      // 0x18
		TUINT32         uiUnk1C;     // 0x1C
	};

	struct Set
	{
		// $deBlob: FUNCTION 0059fc80
		Node& GetNode( TINT16 a_Index )
		{
			TASSERT( a_Index >= 0 && a_Index < iNodeCount );
			return pNodes[ a_Index ];
		}

		// Returns the first node with Node::FLAG_Start (0 if there's none)
		// $deBlob: FUNCTION 0059feb0
		TINT GetStartNode() const
		{
			for ( TINT i = 0; i < iNodeCount; i++ )
			{
				if ( pNodes[ i ].uiFlags & Node::FLAG_Start )
					return i;
			}

			return 0;
		}

		TINT16 iNodeCount; // 0x0
		TUINT8 uiUnk02;    // 0x2 bit 0x1: the set can be the route of a respawned platform (APropPlatform::SaveRespawnData)
		Node*  pNodes;     // 0x4
	};

	struct Data
	{
		TINT iSetCount; // 0x0
		Set* pSets;     // 0x4
	};

public:
	AWaypointData( Toshi::TTRB* a_pTRB );
	// $deBlob: FUNCTION 0078db20
	~AWaypointData() = default;

	// $deBlob: FUNCTION 0059fce0
	Set& GetSet( TINT a_Index )
	{
		TASSERT( a_Index >= 0 && a_Index < m_pData->iSetCount );
		return m_pData->pSets[ a_Index ];
	}

	TINT GetSetCount() const { return m_pData->iSetCount; }

private:
	Data* m_pData; // 0x0
};
