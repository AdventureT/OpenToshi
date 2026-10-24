/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_CONSTRAINT_MODIFIER_INTERFACE_H
#define INC_CONSTRAINT_MODIFIER_INTERFACE_H

#include <iFnPub.h>

/*
** PARAMETERS AND PARAMETER BLOCKS
*/

// Common Parameter Block
enum
{
	PB_CONSTRAINT_MOD_COMMON_SPACES_PARAMS, // (Parent To, Constraint Spaces, Breakable rollouts)

	PB_CONSTRAINT_MOD_FIRST_FREE_PARAMBLOCK, // Actual constraints reuse this id for their properties
};

// Common Space Parameters
enum
{
	// HIDDEN PARAMETERS
	PA_CONSTRAINT_MOD_CHILD_SPACE_TRANSLATION,
	PA_CONSTRAINT_MOD_CHILD_SPACE_ROTATION,
	PA_CONSTRAINT_MOD_PARENT_SPACE_TRANSLATION,
	PA_CONSTRAINT_MOD_PARENT_SPACE_ROTATION,

	// "CONSTRAINT TO" ROLLOUT
	PA_CONSTRAINT_MOD_WORLD_OR_PARENT = 10,
	PA_CONSTRAINT_MOD_PARENT_NODE,

	// "SPACES" ROLLOUT
	PA_CONSTRAINT_MOD_CHILD_TRANSLATION_LOCK = 20,
	PA_CONSTRAINT_MOD_CHILD_ROTATION_LOCK,
	PA_CONSTRAINT_MOD_PARENT_TRANSLATION_LOCK,
	PA_CONSTRAINT_MOD_PARENT_ROTATION_LOCK,

	// "BREAKABLE" ROLLOUT
	PA_CONSTRAINT_MOD_IS_BREAKABLE = 30,
	PA_CONSTRAINT_MOD_BREAK_THRESHOLD
};



/*
** FUNCTION PUBLISHING
*/

enum
{
	FPI_GetChildSpaceInWorld,
	FPI_SetChildSpaceInWorld,
	FPI_GetParentSpaceInWorld,
	FPI_SetParentSpaceInWorld,

	FPI_ResetChildSpaceRotation,
	FPI_ResetChildSpaceTranslation,
	FPI_ResetParentSpaceRotation,
	FPI_ResetParentSpaceTranslation,
};

#define HK_CONSTRAINT_FPINTERFACE_ID Interface_ID(0x564702bc, 0x267f287f)

/*
** All constraint interfaces inherit from this interface
** This interface defines methods to manipulate constraint spaces
**
** Individual constraints usually just add specific parameters for limits.
**
** Check the Havok Content Tools > Integration section for details on how to use the C++/MaxScript interfaces to the 3ds max tools.
*/

class hctConstraintModifierFPInterface : public FPMixinInterface
{

	public:

	BEGIN_FUNCTION_MAP

		PROP_TFNS(FPI_GetChildSpaceInWorld, iGetChildSpaceInWorld, FPI_SetChildSpaceInWorld, iSetChildSpaceInWorld, TYPE_MATRIX3_BV);
		PROP_TFNS(FPI_GetParentSpaceInWorld, iGetParentSpaceInWorld, FPI_SetParentSpaceInWorld, iSetParentSpaceInWorld, TYPE_MATRIX3_BV);

		FN_0 (FPI_ResetChildSpaceRotation, TYPE_BOOL, iResetChildSpaceRotation)
		FN_0 (FPI_ResetChildSpaceTranslation, TYPE_BOOL, iResetChildSpaceTranslation )
		FN_0 (FPI_ResetParentSpaceRotation, TYPE_BOOL, iResetParentSpaceRotation)
		FN_0 (FPI_ResetParentSpaceTranslation, TYPE_BOOL, iResetParentSpaceTranslation)

	END_FUNCTION_MAP

		/*virtual*/ FPInterfaceDesc* GetDesc() {return getCommonFPInterfaceDescriptor();}

		virtual FPInterfaceDesc* getCommonFPInterfaceDescriptor() = 0;

		
			// Access to the Child Space
		virtual Matrix3 iGetChildSpaceInWorld (TimeValue t)=0;
		virtual void iSetChildSpaceInWorld (Matrix3 childSpaceWorld, TimeValue t) = 0;

			// Access to the parent space
		virtual Matrix3 iGetParentSpaceInWorld (TimeValue t) = 0;
		virtual void iSetParentSpaceInWorld (Matrix3 parentSpaceWorld, TimeValue t) = 0;

			// Access to the 4 Reset buttons
		virtual BOOL iResetChildSpaceRotation () = 0;
		virtual BOOL iResetChildSpaceTranslation () = 0;
		virtual BOOL iResetParentSpaceRotation () = 0;
		virtual BOOL iResetParentSpaceTranslation () = 0;
};	

#endif //INC_CONSTRAINT_MODIFIER_INTERFACE_H

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
