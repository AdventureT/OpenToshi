/* 
 * 
 * Confidential Information of Telekinesys Research Limited (t/a Havok). Not for disclosure or distribution without Havok's
 * prior written consent. This software contains code, techniques and know-how which is confidential and proprietary to Havok.
 * Level 2 and Level 3 source code contains trade secrets of Havok. Havok Software (C) Copyright 1999-2009 Telekinesys Research Limited t/a Havok. All Rights Reserved. Use of this software is subject to the terms of an end user license agreement.
 * 
 */

#ifndef HK_UTILITIES2_JOB_THREAD_POOL_H
#define HK_UTILITIES2_JOB_THREAD_POOL_H

#include <Common/Base/Object/hkReferencedObject.h>
#include <Common/Base/Thread/JobQueue/hkJobQueue.h>

class hkJobQueue;
struct hkTimerData;

class hkJobThreadPool : public hkReferencedObject
{
	public:

		virtual ~hkJobThreadPool() {}

		virtual void processAllJobs( hkJobQueue* queue, hkJobType firstJobType = HK_JOB_TYPE_MAX ) = 0;
		virtual void waitForCompletion( ) = 0;

		virtual bool isProcessing() = 0;

		virtual int getNumThreads() = 0;
		virtual void setNumThreads( int numThreads ) = 0;

		virtual void appendTimerData( hkArray<hkTimerData>& timerDataOut ) = 0;
		virtual void clearTimerData() = 0;
};


#endif // HK_UTILITIES2_JOB_THREAD_POOL_H

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
