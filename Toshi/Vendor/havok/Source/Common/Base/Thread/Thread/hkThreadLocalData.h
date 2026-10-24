/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */
#ifndef HKBASE_HK_THREAD_LOCAL_POINTER_H
#define HKBASE_HK_THREAD_LOCAL_POINTER_H

#include <Common/Base/Config/hkConfigThread.h>
//HK_THREAD_LOCAL - declare a thread local variable
//HK_THREAD_LOCAL_SET(var,value) - assign to tls var
//HK_THREAD_LOCAL_GET(var) - get from tls var

#if (HK_CONFIG_THREAD == HK_CONFIG_MULTI_THREADED) && !defined(HK_PLATFORM_SPU)
#if defined(HK_PLATFORM_PS3_PPU) || defined(HK_PLATFORM_UNIX)
#		define HK_THREAD_LOCAL(TYPE) __thread TYPE
#	elif defined(HK_PLATFORM_WIN32)
		extern "C" {
			__declspec(dllimport) unsigned long _stdcall TlsAlloc(void);
			__declspec(dllimport) void* _stdcall TlsGetValue( unsigned long dwTlsIndex );		
			__declspec(dllimport) int _stdcall TlsSetValue( unsigned long dwTlsIndex, void* lpTlsValue);
			__declspec(dllimport) int _stdcall TlsFree( unsigned long dwTlsIndex );
		}

		/// A platform independent wrapper for thread local storage
		/// We assume we always just store a pointer (or data the same size or smaller than a pointer) 
		/// ie,  sizeof(T) <= sizeof(char*)
		template<typename T>
		class hkThreadLocalData
		{
			public:

				hkThreadLocalData() { m_slotID =  TlsAlloc(); }
				~hkThreadLocalData() { TlsFree(m_slotID); }
				HK_FORCE_INLINE T getData() const
				{
					return (T)(hkUlong)TlsGetValue(m_slotID) ;
				}
				HK_FORCE_INLINE void setData(T p) 
				{
					hkUlong v = hkUlong(p);
					TlsSetValue( m_slotID, (void*)v );
				}
				unsigned long m_slotID; 
		};
		
#		define HK_THREAD_LOCAL(TYPE) hkThreadLocalData<TYPE>
#		define HK_THREAD_LOCAL_SET(var,value) var.setData(value)
#		define HK_THREAD_LOCAL_GET(var) var.getData()
#	elif defined(HK_PLATFORM_MACPPC) || defined(HK_PLATFORM_MAC386) 

#		include <pthread.h>
			template < typename T > 
			class hkThreadLocalData
			{
				public:
					hkThreadLocalData() { pthread_key_create(&m_key, HK_NULL); }
					~hkThreadLocalData() { pthread_key_delete(m_key); }

					HK_FORCE_INLINE T getData() 
					{
						return (T) pthread_getspecific(m_key);
					}

					HK_FORCE_INLINE void setData(T p) 
					{					
						pthread_setspecific(m_key, (void*)p );
					}

				protected:
					pthread_key_t m_key; 
			};

#		define HK_THREAD_LOCAL(TYPE) hkThreadLocalData<TYPE>
#		define HK_THREAD_LOCAL_SET(var,value) var.setData(value)
#		define HK_THREAD_LOCAL_GET(var) var.getData()

#	else
#		error fixme
#	endif
#else
#	define HK_THREAD_LOCAL(TYPE) TYPE
#endif

#ifndef HK_THREAD_LOCAL_SET
#	define HK_THREAD_LOCAL_SET(var,value) var = value
#endif
#ifndef HK_THREAD_LOCAL_GET
#	define HK_THREAD_LOCAL_GET(var) var
#endif

#endif // HKBASE_HK_THREAD_LOCAL_POINTER_H

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
