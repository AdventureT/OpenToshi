/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef INC_HK_MAX_SCENE_EXPORT_UTILITY_INTERFACE__H
#define INC_HK_MAX_SCENE_EXPORT_UTILITY_INTERFACE__H

#include <iFnPub.h>

// Class ID
#define HK_SCENE_EXPORT_UTILITY_CLASS_ID	Class_ID(0x16052618, 0x2e0770f6)

/*
** FUNCTION PUBLISHING
*/

#define HK_SCENE_EXPORT_UTILITY_FPINTERFACE_ID Interface_ID(0x47713dcf, 0x9125fade)

enum
{
	FPI_ExportScene,
	FPI_SetUseOptionsFile,
	FPI_GetUseOptionsFile,
	FPI_SetOptionsFile,
	FPI_GetOptionsFile,
	FPI_SetExportVisibleOnly,
	FPI_GetExportVisibleOnly,
	FPI_SetExportSelectedOnly,
	FPI_GetExportSelectedOnly,
	FPI_SetExportMeshes,
	FPI_GetExportMeshes,
	FPI_SetExportMaterials,
	FPI_GetExportMaterials,
	FPI_SetExportAttributes,
	FPI_GetExportAttributes,
	FPI_SetExportAnnotations,
	FPI_GetExportAnnotations,
	FPI_SetExportLights,
	FPI_GetExportLights,
	FPI_SetExportCameras,
	FPI_GetExportCameras,
	FPI_GetAddDefaultCamera,
	FPI_SetAddDefaultCamera,
	FPI_GetUseOldPhysics,
	FPI_SetUseOldPhysics,
	FPI_GetAnimatedDataExport,
	FPI_SetAnimatedDataExport,
	FPI_GetAnimationStart,
	FPI_SetAnimationStart,
	FPI_GetAnimationEnd,
	FPI_SetAnimationEnd,
	FPI_GetVersionString,
	FPI_GetVersionNumber,
	FPI_GetEnvironmentVariables,
	FPI_SetEnvironmentVariables,
	FPI_GetDoNotSplitVertices,
	FPI_SetDoNotSplitVertices,
	FPI_ImportScene,
	FPI_WaitForFilterLoad
};

enum
{
	FPI_ENUM_AnimatedDataExport
};

enum
{
	ADE_CURRENT_FRAME,
	ADE_CURRENT_RANGE,
	ADE_SPECIFIC_RANGE
};

/*
** C++ Interface to the Havok Exporter Utility.
**
** Exposed to MAXScript as hctSceneExportUtility
**
** The methods here correspond to the methods and properties (get/set) exposed to MaxScript.
**
** Check the Havok Content Tools > Integration section for details on how to use the C++/MaxScript interfaces to the 3ds max tools.
**
*/

class hctSceneExportUtilityFPInterface : public FPStaticInterface
{
	public:

		// hctSceneExportUtility.exportScene batchMode  -> exports the scene. If batchMode is true, no UI is shown.
		virtual BOOL iExportScene ( BOOL batchMode ) = 0;

		// hctSceneExportUtility.importScene, given a HKX file to load
		virtual BOOL iImportScene ( TCHAR* pathToHKX ) = 0;
		
		// cross exporter sync point to wait on the thread that is caching the filters in the background
		virtual BOOL iWaitForFilterLoad() = 0;

		/*
		** UI Fields
		** 
		** Parameter blocks in utilities are not automatically exposed to maxscript, so we need to expose them using FPI
		**
		** Please check the documentation of the 3ds max tools for details on the meaning of each field.
		*/

		virtual void iSetExportVisibleOnly( BOOL exportHidden ) = 0;
		virtual BOOL iGetExportVisibleOnly() = 0;

		virtual void iSetExportSelectedOnly( BOOL selectedOnly ) = 0;
		virtual BOOL iGetExportSelectedOnly() = 0;

		virtual void iSetExportMeshes( BOOL exportMeshes ) = 0;
		virtual BOOL iGetExportMeshes() = 0;

		virtual void iSetExportMaterials( BOOL exportMaterials ) = 0;
		virtual BOOL iGetExportMaterials() = 0;

		virtual void iSetExportAttributes( BOOL exportAttributes ) = 0;
		virtual BOOL iGetExportAttributes() = 0;

		virtual void iSetExportAnnotations( BOOL exportAnnotations ) = 0;
		virtual BOOL iGetExportAnnotations() = 0;

		virtual void iSetExportLights( BOOL exportLights ) = 0;
		virtual BOOL iGetExportLights() = 0;

		virtual void iSetExportCameras( BOOL exportCameras ) = 0;
		virtual BOOL iGetExportCameras() = 0;

		virtual void iSetAddDefaultCamera (BOOL addDefaultCamera) = 0;
		virtual BOOL iGetAddDefaultCamera() = 0;


		virtual void iSetAnimatedDataExport (int animatedDataExport) = 0;
		virtual int iGetAnimatedDataExport () = 0;

		virtual void iSetAnimationStart (TimeValue startTick) =0;
		virtual TimeValue iGetAnimationStart () = 0;

		virtual void iSetAnimationEnd (TimeValue endTick) =0;
		virtual TimeValue iGetAnimationEnd () = 0;


		virtual void iSetUseOptionsFile( BOOL useOptionsFile ) = 0;
		virtual BOOL iGetUseOptionsFile() = 0;

		virtual void iSetOptionsFile( TCHAR* optionsFile ) = 0;
		virtual TCHAR* iGetOptionsFile() = 0;

		virtual void iSetEnvironmentVariables( TCHAR* environmentVars ) = 0;
		virtual TCHAR* iGetEnvironmentVariables() = 0;

		virtual void iSetDoNotSplitVertices (BOOL doNotSplitVertices) = 0;
		virtual BOOL iGetDoNotSplitVertices () = 0;


		// Returns a string representing the version of the exporter (for example "4.0.0.3 Beta")
		virtual const TCHAR* iGetVersionString () = 0;

		// Returns an integer representation of the version (for example 0x04000003)
		virtual int iGetVersionNumber () = 0;
};

/*
** ACTION PUBLISHING
*/

#define HK_SCENE_EXPORT_UTILITY_ACTIONS_ID Interface_ID(0x5f35035, 0x69ba187a)

enum
{
	ACT_Export,
	ACT_Export_Enabled,
};

// The Action interface exposes buttons in the utility. In this case, the only button is "Export".
class hctSceneExportUtilityActions : public FPStaticInterface
{
	public:

		// Press the Export button
		virtual FPStatus doExport() = 0;

		// Whether the button should be enabled or not
		virtual BOOL isExportEnabled() = 0;
};

#endif //INC_HK_MAX_SCENE_EXPORT_UTILITY_INTERFACE__H

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
