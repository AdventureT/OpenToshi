/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HAVOK_MODELESS_FILTER_H
#define HAVOK_MODELESS_FILTER_H

/// This class provides a default implementation for a modeless filter - it handles the creation of threads for processing.
/// Filters inheriting from this class must implement modalProcess() instead of process() - inside modalProcess() they can/should
/// operate modally (since a new thread is created for that call)
/// The other abstract methods are required in order to properly handle "shutdown" requests from the filter manager.
class hctModelessFilter : public hctFilterInterface
{
	public:
		
			/// Constructor. It takes a pointer to the filter manager that owns it.
		hctModelessFilter (const class hctFilterManagerInterface* owner);

			/// Destructor - waits for thread, etc
		/*virtual*/ ~hctModelessFilter ();

			/// Filters subclassing from hctModelessFilter should implement their processing here instead of in process().
			/// This will be called from a new thread, so processing here can be modal.
		virtual void modalProcess ( class hkRootLevelContainer& contents ) = 0;

			/// Access to the descriptor.
		virtual class hctModelessFilterDescriptor& getDescriptor() const = 0;

			/// Notifies the (modeless) filter that it should close asap. Usually the filter will set a flag that is checked 
			/// by the modal processing method.
		virtual void tryToClose () = 0;

			/// Should return true if the filter is closing down (not blocked). Useful for filters that may take some time to close down,
			/// so the user/filter manager can distinguish between blocked filters and slow filters. By default it returns false - slow
			/// filters should implement this so it returns true when closing down.
		virtual bool isClosingDown () const;

			/// Modeless filters can be treated as modal if desired. Override this method in order to do so.
			/// Default implementation returns false;
		virtual bool behaveAsModal () const ;

			/// Inherited from hctFilterInterface, this implementation copies the contents and creates a new thread to process it,
			/// through the use of "modalProcess()".
		/*virtual*/ void process ( class hkRootLevelContainer& contents ); /* sealed */

			/// Returns true if the thread created for the modeless filter is still active
		bool isThreadActive () const ;

	protected:

			/// Storage data used to clone the scene
		hkArray<char> m_cloneStorage;

	protected:

		/// The thread used by this
		HANDLE m_filterThread;

};


#endif // HAVOK_MODELESS_FILTER_H

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
