/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HKBASE_STACKTRACER_H
#define HKBASE_STACKTRACER_H

/// An object which can generate stack traces.
/// Some platforms may also be able to associate addresses to 
/// function and source file information.
class hkStackTracer : public hkReferencedObject
{
	public:
		HK_DECLARE_CLASS_ALLOCATOR(HK_MEMORY_CLASS_BASE_CLASS);

		hkStackTracer();

		~hkStackTracer();

		typedef void (HK_CALL *printFunc)(const char*, void* context);

			/// Print the stack trace with pfunc.
			/// pfunc is called may be called multiple times.
			/// The output format is platform specific.
		void dumpStackTrace( const hkUlong* trace, int numtrace, printFunc pfunc, void* context=HK_NULL );

			/// Write at most maxtrace stack entries into 'trace'.
			/// Return the number of entries written.
		int getStackTrace( hkUlong* trace, int maxtrace );
};

#endif // HKBASE_STACKTRACER_H

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
