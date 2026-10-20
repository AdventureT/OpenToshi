/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_HK_MAX_CONVEX_HULL_UTILITY_INTERFACE__H
#define INC_HK_MAX_CONVEX_HULL_UTILITY_INTERFACE__H

#include <iFnPub.h>

/*
** C++ Interface to the Create Convex Hull Utility.
**
** Exposed to MAXScript as hctConvexHullUtility
**
** The methods here correspond to the methods and properties (get/set) exposed to MaxScript.
** Those, in turn, correspond to the fields exposed to the 3ds max UI
**
*/

// Class ID
#define HK_CONVEX_HULL_UTILITY_CLASS_ID Class_ID(0x2c7f91ed, 0xa114e3b5)

// Reference number of the PBlock
#define PBLOCK_ConvexHullUtility	0

// Param block IDs
enum
{
	PB_CONVEX_HULL_UTILITY_PBLOCK,
};


// Normal Parameters
enum
{
	PA_CONVEX_HULL_UTILITY_MAXVERTS,
	PA_CONVEX_HULL_UTILITY_SAMPLES,
	PA_CONVEX_HULL_UTILITY_STEPS,
	PA_CONVEX_HULL_UTILITY_MIN_WEIGHT,
};



//
// Function publishing
//

#define HK_CONVEX_HULL_UTILITY_FPINTERFACE_ID Interface_ID(0x62ed5ad1, 0xdf34ffa1)

enum
{
	FPI_CanCreateHull,
	FPI_CreateHull,
	FPI_CreateMultiHull,
	FPI_CreateSkinHull,
	FPI_CreateSkinMultiHull,
	FPI_GetMaxVerts,
	FPI_SetMaxVerts,
	FPI_GetSamples,
	FPI_SetSamples,
	FPI_GetSteps,
	FPI_SetSteps,

	FPI_GetMinWeight,
	FPI_SetMinWeight,
};

class hctConvexHullUtilityFPInterface : public FPStaticInterface
{
	public:

		virtual BOOL iCanCreateHull() = 0;
		virtual INode* iCreateHull (INode* node) = 0;
		virtual INode* iCreateMultiHull (Tab<INode*>& nodes) = 0;
		virtual INode* iCreateSkinHull (INode* node) = 0;
		virtual INode* iCreateSkinMultiHull (Tab<INode*>& nodes) = 0;


		virtual void iSetMaxVerts (int maxVerts) = 0;
		virtual int iGetMaxVerts () = 0;

		virtual void iSetSamples (int samples) = 0;
		virtual int iGetSamples () = 0;

		virtual void iSetSteps (int steps ) = 0;
		virtual int iGetSteps () = 0;

		virtual void iSetMinWeight (float minWeight) = 0;
		virtual float iGetMinWeight () = 0;
};


//
// Action publishing
//

#define HK_CONVEX_HULL_UTILITY_ACTIONS_ID Interface_ID(0xf1ed017a, 0xca3ad12f)

enum
{
	ACT_GenerateHullEnabled,
	ACT_GenerateHull,
	ACT_GenerateSkinHullEnabled,
	ACT_GenerateSkinHull,
};

class hctConvexHullUtilityActions : public FPStaticInterface
{
	public:

		virtual BOOL iGenerateHullEnabled() = 0;
		virtual BOOL iGenerateSkinHullEnabled() = 0;

		virtual FPStatus iGenerateHull() = 0;
		virtual FPStatus iGenerateSkinHull() = 0;

};


#endif //INC_HK_MAX_CONVEX_HULL_UTILITY_INTERFACE__H

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
