/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_HK_MAX_CONVEX_DECOMPOSITION_UTILITY_INTERFACE__H
#define INC_HK_MAX_CONVEX_DECOMPOSITION_UTILITY_INTERFACE__H

#include <iFnPub.h>

/*
** C++ Interface to the Create Convex Decomposition Utility.
**
** Exposed to MAXScript as hctConvexDecompositionUtility
**
** The methods here correspond to the methods and properties (get/set) exposed to MaxScript.
** Those, in turn, correspond to the fields exposed to the 3ds max UI
**
*/   

// Class ID
#define HK_CONVEX_DECOMPOSITION_UTILITY_CLASS_ID Class_ID(0x41b47421, 0x1e1f69c7)

// Reference number of the PBlock
#define PBLOCK_ConvexDecompositionUtility	0

// Param block IDs
enum
{
	PB_CONVEX_DECOMPOSITION_UTILITY_PBLOCK,
};


// Normal Parameters
enum
{
	PA_CONVEX_DECOMPOSITION_UTILITY_ACCURACY
};



//
// Function publishing
//

#define HK_CONVEX_DECOMPOSITION_UTILITY_FPINTERFACE_ID Interface_ID(0x20723700, 0x5ec7816)

enum
{
	FPI_CanCreateDecomposition,
	FPI_CreateDecomposition,
	FPI_CreateMultiDecomposition,
	FPI_GetAccuracy,
	FPI_SetAccuracy,
};

class hctConvexDecompositionUtilityFPInterface : public FPStaticInterface
{
	public:

		virtual BOOL iCanCreateDecomposition() = 0;
		virtual INode* iCreateDecomposition (INode* node) = 0;
		virtual INode* iCreateMultiDecomposition (Tab<INode*>& nodes) = 0;

		virtual void iSetAccuracy (float accuracy) = 0;
		virtual float iGetAccuracy () = 0;
};


//
// Action publishing
//

#define HK_CONVEX_DECOMPOSITION_UTILITY_ACTIONS_ID Interface_ID(0x2d1c51ea, 0x7da15607)

enum
{
	ACT_GenerateDecompositionEnabled,
	ACT_GenerateDecomposition,
};

class hctConvexDecompositionUtilityActions : public FPStaticInterface
{
	public:

		virtual BOOL iGenerateDecompositionEnabled() = 0;
		virtual FPStatus iGenerateDecomposition() = 0;
};


#endif //INC_HK_MAX_CONVEX_DECOMPOSITION_UTILITY_INTERFACE__H

/*
* Havok SDK - NO SOURCE PC DOWNLOAD, BUILD(#20090704)
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
