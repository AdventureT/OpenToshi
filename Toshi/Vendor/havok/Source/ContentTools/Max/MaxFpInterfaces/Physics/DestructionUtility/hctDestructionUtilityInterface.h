/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_HK_MAX_DESTRUCTION_UTILITY_INTERFACE__H
#define INC_HK_MAX_DESTRUCTION_UTILITY_INTERFACE__H

#include <iFnPub.h>

/*
** C++ Interface to the Destruction Utility.
**
** Exposed to MAXScript as hctDestructionUtility
**
** The methods here correspond to the methods and properties (get/set) exposed to MaxScript.
** Those, in turn, correspond to the fields exposed to the 3ds max UI
**
*/

// Class ID
#define HK_DESTRUCTION_UTILITY_CLASS_ID Class_ID(0x3fe80d6c, 0x298208cd)

// Reference number of the PBlock
#define PBLOCK_DestructionUtility 0

// Param block IDs
enum
{
	PB_DESTRUCTION_UTILITY_PBLOCK = 0,
};


//
// Function publishing
//

#define HK_DESTRUCTION_UTILITY_FPINTERFACE_ID Interface_ID(0x1eed4694, 0x21ae42a2)

enum
{
	FPI_CanDoDestruction = 0,
	FPI_Destruct,
	FPI_DestructionPreview,
	FPI_DestructionOptions,
	FPI_DestructionDelete,
	FPI_DestructionMessageLog
};

class hctDestructionUtilityFPInterface : public FPStaticInterface
{
	public:

		virtual BOOL iCanDoDestruction() = 0;
		virtual BOOL iDestruct(Tab<INode*>& nodes, Tab<INode*>& newNodes) = 0;
		virtual BOOL iDestructionPreview(Tab<INode*>& nodes) = 0;
		virtual BOOL iDestructionOptions(Tab<INode*>& nodes, Tab<INode*>& newNodes) = 0;
		virtual BOOL iDestructionDelete(Tab<INode*>& nodes) = 0;
		virtual BOOL iDestructionMessageLog() = 0;
};


//
// Action publishing
//

#define HK_DESTRUCTION_UTILITY_ACTIONS_ID Interface_ID(0x7c255fb6, 0x2a9a36d7)

enum
{
	ACT_DestructionPossible = 0,

	ACT_Destruct,
	ACT_DestructionPreview,
	ACT_DestructionOptions,
	ACT_DestructionDelete,
	ACT_DestructionMessageLog
};


class hctDestructionUtilityActions : public FPStaticInterface
{
	public:

		virtual BOOL     iDestructionPossible() = 0;

		virtual FPStatus iDestruct() = 0;
		virtual FPStatus iDestructionPreview() = 0;
		virtual FPStatus iDestructionOptions() = 0;
		virtual FPStatus iDestructionDelete() = 0;
		virtual FPStatus iDestructionMessageLog() = 0;
};


#endif //INC_HK_MAX_DESTRUCTION_UTILITY_INTERFACE__H

/*
* Havok SDK - NO SOURCE PC DOWNLOAD, BUILD(#20090216)
* 
* Confidential Information of Havok.  (C) Copyright 1999-2009
* Telekinesys Research Limited t/a Havok. All Rights Reserved. The Havok
* Logo, and the Havok buzzsaw logo are trademarks of Havok.  Title, ownership
* rights, and intellectual property rights in the Havok software remain in
* Havok and/or its suppliers.
* 
* Use of this software for evaluation purposes is subject to and indicates
* acceptance of the End User licence Agreement for this product. A copy of
* the license is included with this software and is also available at www.havok.com/tryhavok.
* 
*/
