/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_CONVERSIONUTILGUPINTERFACE
#define INC_CONVERSIONUTILGUPINTERFACE

#include <iFnPub.h>

// Class ID
#define HK_CONVERSION_UTIL_GUP_CLASS_ID	Class_ID(0x5fcd3256, 0x46f60d66)

/* 
** FUNCTION PUBLISHING
*/

#define HK_CONVERSION_UTIL_GUP_FPINTERFACE_ID	Interface_ID(0x32881fdd, 0x34e953ee)

enum 
{
	FPI_ConvertSelectionToVertexChannel,
	FPI_ConvertVertexChannelToSelection,
	FPI_AssignVertexColors
};

/*
** C++ Interface to the Havok Content Tools Conversion Utilities.
**
** Exposed to MAXScript as hctConversionUtilGUP
**
** The methods here correspond to the methods exposed to MaxScript.
** 
*/
class hctConversionUtilGUPInterface : public FPStaticInterface 
{
	public:

		virtual void iConvertSelectionToVertexChannel( INode* selectedNode, ReferenceTarget* vpModifier, FLOAT selectedValue, FLOAT unselectedValue ) = 0;
		virtual void iConvertVertexChannelToSelection( INode* selectedNode, INT channelID, FLOAT thresholdValue, BOOL invert, TCHAR* selectionName ) = 0;
		virtual void iAssignVertexColors( INode* selectedNode, ReferenceTarget* vpModifier, INT channelID, BOOL byVertex ) = 0;
};

#endif //INC_CONVERSIONUTILGUPINTERFACE

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
