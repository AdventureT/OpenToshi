/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_FILTER_MANAGER_INTERFACE__H
#define HAVOK_FILTER_MANAGER_INTERFACE__H


	/// The interface to the filter manager.
	/// Use the function createFilterManager() exposed in hctFilterManagerDll to do that.
	/// We make this pure virtual so that the actual implementation can be havok memory managed etc..
	/// and this interface is still clean. It also avoids static linkage with the implementation.
	/// Some methods are used by applications calling the filter manager (exporters for example), while other
	/// will mostly be used by filters during processing.
class hctFilterManagerInterface
{
	public:

			/// Virtual destructor
		virtual ~hctFilterManagerInterface() {} 

			/// Returns a version associated with the filter manager.
			/// This version is used to check compatibility between configurations.
		virtual unsigned int getFilterManagerVersion() const = 0;

			/*
			* Configuration options : Called by application (exporter)
			*/

			/// Get the current configuration set, including all the filters for each configuration.
			/// This data is usually saved with the original asset, or saved explicitly by the user to file.
			/// Pass in a NULL to retrieve the required size of the buffer.
		virtual int getConfigurationSet(void* optionData) const = 0;

			/// Initialize a configuration set.
		virtual void setConfigurationSet(const void* optionData, int optionDataSize) = 0;

			/*
			* Interaction and processing, called by the application (exporter)
			*/

			/// Bring up a dialog box to allow the user to organize which filters to use.
			/// The selected filters and configuration are remembered between runs.
			/// It returns whether the configuration should be executed and whether it should be saved.
		virtual void openFilterManager ( HWND owner, const class hkRootLevelContainer& data, hkBool& shouldSaveConfigOut ) = 0;

			/// Process a scene in batch mode. All configurations will be executed.
		    /// It will alter a copy of the data (one per config) in place as much as possible and all allocations into the hkRootLevelContainer
			/// will be through the memory tracker. As such the contents of the scene post process will be 
			/// valid until you delete the memory tracker (will deallocate all tracked mem) or you shut down
			/// this manager instance (it will unload all the filter DLLs and hence all the hkClasses from
			/// those DLLs will be unloaded too and the pointers in any registry etc ).
		virtual void processBatch ( HWND owner, const class hkRootLevelContainer& data, class hctFilterMemoryTracker& sceneMemory, int configToRun = -1, bool allowModlessFilters = false  ) = 0;

			/// Process a scene in batch mode. Only specific configuration will be executed.
			/// It will alter a copy of the data in place as much as possible and all allocations into the hkRootLevelContainer
			/// will be through the memory tracker. As such the contents of the scene post process will be 
			/// valid until you delete the memory tracker (will deallocate all tracked mem) or you shut down
			/// this manager instance (it will unload all the filter DLLs and hence all the hkClasses from
			/// those DLLs will be unloaded too and the pointers in any registry etc ).
			/// It will only return the modified (copied) data if you provide a sceneCopyStorage array to hold the original deep copy
		virtual hkRootLevelContainer* processBatchReturnData( HWND owner, const class hkRootLevelContainer& data, class hctFilterMemoryTracker& sceneMemory, int configToRun, bool allowModelessFilters, hkArray<char>* sceneCopyStorage ) = 0;


			/// Accesses (non-const access) the global registry of classes.
			/// Allows for registration of classes not added by filter DLLS (added by exporters for example).
		virtual hctFilterClassRegistry& accessFilterClassRegistry() = 0;

			/*
			** Methods useful for filters
			*/

			/// Some filters may want to access the contents that is going to be processed in order to show their options
			/// (for example in order to present picking dialogs). Notice that this is not necessarily the contents that
			/// will reach the filter (as it may be modified by any filter preceding this one)
		virtual const class hkRootLevelContainer* getOriginalContents() const = 0;

			/// Get the filter manager's error handler.
			/// Allows for merging with other error handlers (from the exporters for example).
		virtual class hctSceneExportError* getErrorHandler() = 0;

			/// Retrieves (const access) the global registry of classes (gathered from all filter DLLs).
			/// Filter DLLs register their classes implementing hkFilterDLL::registerClasses().
		virtual const hctFilterClassRegistry& getFilterClassRegistry () const = 0;

			/// Some complicated filter setups require some knowledge on what 
			/// data they will get upon process(), so this will process up
			/// to the given filter, in the current filter configuration. 
			/// It's up to the filter to manage the memory of this contents, hence the storageTempMem parameter.
		virtual bool getInputContentsToCurrentFilter ( hkRootLevelContainer** outputData, hctFilterMemoryTracker& storageTempMem ) const = 0;

			/// Returns the HWND of the filter manager main window (or HK_NULL if no UI is shown).
		virtual HWND getMainWindowHandle () const = 0;

			/// Returns the HWND of the application (owner) that called the filter manager
		virtual HWND getOwnerWindowHandle () const = 0;

		enum ProcessMode
		{
			PROCESS_NONE = 0, // not in process of any type
			PROCESS_INTERACTIVE, // full interactive run
			PROCESS_BATCH, // batch (so no ui) and do full writes etc
			PROCESS_BATCH_UI, // batch (but with ui / modeless filters) and do full writes etc
			OPTIONS_BATCH  // batch mode for options to current dialog etc, write any scene mutating data, but do not alter external files 
		};

		virtual ProcessMode getProcessMode() const = 0;

			/* 
			* Thread utils
			*/

			/// Register a thread callback. DLLs creating objects with virtual methods need to register themselves
			/// so their threadback data is updated when a new thread is created. The filter manager automatically
			/// registers filter DLLS, but if you have another module taking part (a non-filter DLL, like hkpreview)
			/// you can register a callback to ensure thread information is registered in that module. The scene exporters
			/// register themselves using this mechanism.
		virtual void registerThreadCallback (const class hctFilterThreadCallback* cb)= 0;

			/// If you create a thread that will use code from a another filter (by means
			/// of vtable calls for instance) you must propagate your thread data (esp thread mem)
			/// It will set the thread data for all filters, but also whatever callback
			/// is registered too (just one)
		virtual void setThreadData (hkThreadMemory* threadMemory) const = 0;

			/* 
			* Utility dialogs, called by filters
			*/

			/// Bring up a modal dialog to select a given node, mesh, whatever from the scene.
			/// A tree view manager may be implemented to limit the selection to a specific root node
			/// and/or specific class types. If the user successfully selected an object, the
			/// function returns true and the selected object is returned as the variant.
		virtual bool selectObjectFromTree( HWND owner, const char* title, class hctTreeViewManager* tvManager, struct hkVariant& selected ) const = 0;

			/// Bring up a modal dialog to select one item from a list of items.
			/// Returns the index of the selected item, or -1 if the user hit cancel or escape.
		virtual int selectItemFromList( HWND owner, const char* title, const hkArray<const char*>& items ) const = 0;

			/// Bring up a modal dialog to show a list of item names.
			/// Only action available to the user is to close the dialog.
		virtual void showItems( HWND owner, const char* title, const hkArray<const char*>& items ) const = 0;
};

	/// Thread callback : Registered through hctFilterManagerInterface::registerThreadCallback, these objects will
	/// be called each time a new thread is created by any filter.
class hctFilterThreadCallback
{
	public:

			/// This method will be called each time a new thread is created by any filter.
		virtual void newThreadCreated (hkThreadMemory* threadMemory) const = 0;

};


#endif // HAVOK_FILTER_MANAGER_INTERFACE__H

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
