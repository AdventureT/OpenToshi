/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_MULTITHREADING_UTIL_NEW_H
#define HK_UTILITIES2_MULTITHREADING_UTIL_NEW_H

#include <Common/Base/Thread/Job/ThreadPool/hkJobThreadPool.h>
#include <Common/Base/Thread/Semaphore/hkSemaphoreBusyWait.h>

class hkSpuUtil;
class hkJobQueue;


struct hkSpuJobThreadPoolCinfo
{
	hkSpuJobThreadPoolCinfo();

	hkSpuUtil* m_spuUtil;
	int m_numSpus;

		/// Defaults to 1024*100 (100K)
	int m_perSpuMontiorBufferSize;

	const char* m_tasksetName;
};


class hkSpuJobThreadPool : public hkJobThreadPool
{
	public:

		hkSpuJobThreadPool( hkSpuJobThreadPoolCinfo& cinfo );
		~hkSpuJobThreadPool();

		virtual void processAllJobs( hkJobQueue* queue, hkJobType firstJobType = HK_JOB_TYPE_MAX );
		virtual void waitForCompletion( );

		virtual bool isProcessing();

		virtual void appendTimerData( hkArray<hkTimerData>& timerData );
		virtual void clearTimerData();

		virtual int getNumThreads();
		virtual void setNumThreads( int numThreads );


		hkSpuUtil* getSpuUtil() { return m_spuUtil; }
		class hkSpuMonitorCache* getMonitorCache(int i) { return m_monitorCaches[i]; }

		enum { MAX_NUM_SPUS = 6 };

	protected:

		hkSpuUtil* m_spuUtil;
		int m_maxNumSpus;
		int m_numSpus;

		bool m_isRunning;

		const char* m_tasksetName;

		hkSemaphoreBusyWait* m_taskCompletionSemaphore;

		hkJobQueue* m_jobQueue;
		hkSpuMonitorCache* m_monitorCaches[MAX_NUM_SPUS];
};





#endif // HK_UTILITIES2_MULTITHREADING_UTIL_H

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
