/* 
	interface definition for spu job manager
*/

#ifndef __IJOBMAN_SPU_H
#define __IJOBMAN_SPU_H
#pragma once

struct ILog;

#if defined(PS3)

namespace NPPU
{
	//bubble mode
	enum EBubbleMode
	{
		eBM_Single = 0,		//single bubble mode, job occupies as much as it takes (up to ~180 KB)
		eBM_4x8		 = 1,			//4 bubbles with 8 KB each
		eBM_4x16	 = 2,			//4 bubbles with 16 KB each
		eBM_4x32	 = 4,			//4 bubbles with 32 KB each
		eBM_NONE	 = 8			
	};
	#define BUBBLE_MODE_MASK (unsigned char)7

	//maximum cache size, 
	enum ECacheMode
	{
		eCM_4		= 8,			//max cache size 4 KB
		eCM_8		= 16,			//max cache size 8 KB
		eCM_16	= 32,			//max cache size 16 KB
		eCM_32	= 64,			//max cache size 32 KB
		eCM_64	= 128			//max cache size 64 KB, default
	};
	#define CACHE_MODE_MASK (unsigned char)(~7)
	#define CACHE_MODE_SIZE_SHIFT 1
}
#define CACHE_MODE_DEFAULT NPPU::eCM_64
#define BUBBLE_MODE_DEFAULT NPPU::eBM_Single

#include <PPU/PPU.h>

//enable to obtain stats of spu usage each frame
#define SUPP_SPU_FRAME_STATS

namespace NPPU
{
	struct SJobStringHandle;
	typedef SJobStringHandle* TJobHandle;	//handle retrieved by name for job invocation
	#define INVALID_JOB_HANDLE ((unsigned int)-1)

	//return results for AddJob
	enum EAddJobRes
	{
		eAJR_Success,						//success of adding job
		eAJR_NoElf,							//spu job is no elf file
		eAJR_NoSPUElf,					//spu job is no spu elf file
		eAJR_ElfTooBig,					//spu job elf is too big
		eAJR_NoQWAddress,				//spu job image start is not on a quadword address
		eAJR_EnqueueTimeOut,		//spu job was not added due to timeout (SPU were occupied for too long)
		eAJR_EnqueueTimeOutPushJob,		//spu job was not added due to timeout of a push job slot (SPU was occupied by one particular job for too long)
		eAJR_SPUNotInitialized,	//SPU were not initialized
		eAJR_JobTooLarge,				//spu job cannot fit into a single SPU local store
		eAJR_JobSetupViolation,	//spu job has some invalid setup for packets/depending jobs
		eAJR_InvalidJobHandle,	//spu job was invoked with an invalid job handle (job has not been found in spu repository)
		eAJR_UnknownError,			//unknown error
	};
}

#if !defined(__SPU__)

//optimization options, disable all for final usage
//make sure job queue contains never the maximum count->otherwise entries will be overwritten and crash
//switch off for performance
//#define USE_JOB_QUEUE_VERIFICATION	//performs lots of verification during job adding, save cycles by disabling it 
#if defined(USE_JOB_QUEUE_VERIFICATION)
	#define USE_JOB_DMA_VERIFICATION		//performs checks on input and output DMA mapping
#endif

namespace NSPU
{
	namespace NDriver
	{
		struct SExtJobState;
		struct SJobPerfStats;
	}
	namespace NElf
	{
		struct SElfInfo;
	}
}

namespace NPPU
{
	class CSPUJobDel;
	struct SFrameProfileData;

#if defined(SUPP_SPU_FRAME_STATS)
	struct SSPUFrameStats
	{
		float spuStatsPerc[NPPU::scMaxSPU];	//0.f..100.f
		void Reset(){for(int i=0; i<NPPU::scMaxSPU; ++i)spuStatsPerc[i] = 0.f;}
		SSPUFrameStats(){Reset();}
	};

	//per frame and job specific 
	struct SFrameProfileData
	{
		unsigned int usec;				//last frames SPU time in microseconds (written and accumulated by SPU)
		unsigned int count;				//number of calls this frame (written by PPU and SPU)
		const char* const cpName;	//job name (written by PPU)
		unsigned int usecLast;		//last but one frames SPU time in microseconds (written and accumulated by SPU)

		SFrameProfileData(const char* cpJobName) : usec(0), count(0), cpName(cpJobName), usecLast(0){}
		SFrameProfileData() : cpName("Uninitialized"), usec(0), count(0), usecLast(0){}

		void operator=(const SFrameProfileData& crFrom)
		{
			//fast copy op
			*(vec_uint4*)this = *(vec_uint4*)&crFrom;
		}

		void Reset()
		{
			usecLast = usec;
			usec = count = 0;
		}

		const bool operator<(const NPPU::SFrameProfileData& crOther) const
		{
			return usec < crOther.usec;
		}
	} _ALIGN(16);
#endif

	struct SJobStringHandle
	{
		const char *cpString;			//points into repository, must be first element
		unsigned int strLen;			//string length
		uint32 jobHandle;					//job address (corresponding to NBinBub::SJob)
		int jobId;								//index (also acts as id) of job

		const bool operator==(const SJobStringHandle& crOther) const
		{
			if(strLen == crOther.strLen)
			{
				const char* pCharSrc = cpString;
				const char* pCharDst = crOther.cpString;
				unsigned int curIndex = 0;
				while(curIndex++ < strLen && *pCharSrc++ == *pCharDst++){}
				return (curIndex == (strLen+1));
			}
			return false;
		}

		const bool operator<(const SJobStringHandle& crOther) const
		{
			if(strLen != crOther.strLen)
				return strLen < crOther.strLen;
			else
			{
				assert(strLen > 0);
				const char* pCharSrcEnd = &cpString[strLen-1];
				const char* pCharSrc = cpString;
				const char* pCharDst = crOther.cpString;
				while(*pCharSrc++ == *pCharDst++ && pCharSrc != pCharSrcEnd){}
				return *pCharSrc < *pCharDst;
			}
		}
	};
	// singleton managing the job queues and/for the SPUs
	struct IJobManSPU
	{
		//returns number of SPUs allowed for job scheduling
		virtual const unsigned int GetSPUsAllowed() const = 0;
		//sets number of SPUs allowed for job scheduling (must be called before spu initialization)
		virtual void SetSPUsAllowed(const unsigned int cNum) = 0;
		//returns spu driver size, all data must be placed behind it
		virtual const unsigned int GetDriverSize() const  = 0;
		//initializes all allowed SPUs, cpSPURepository is pointer to the SPU repository (SPU job binaries)
		virtual const bool InitSPUs(const char* cpSPURepository) = 0;
		//polls for a spu job (do not use is a callback has been registered)
		//returns false if a time out has occurred
		virtual const bool WaitSPUJob(volatile NSPU::NDriver::SExtJobState& rJobState) const = 0;
		//print performance stats
		virtual void PrintPerfStats(const volatile NSPU::NDriver::SJobPerfStats* pPerfStats, const char* cpJobName) const = 0;
		//sets the external log
		virtual void SetLog(ILog *pLog) = 0;
		//returns true if SPU jobs are still active
		virtual const bool SPUJobsActive() const = 0;
		//shuts down spu job manager
		virtual void ShutDown() = 0;
		//tests all acquired SPUs if they are running
		virtual void TestSPUs() = 0;
		//clean released memory form SPUs and refill buckets
		virtual void UpdateSPUMemMan() = 0;
		//enables spu driver debugging
		virtual void EnableSPUDriverDebugging(const bool) = 0;
		//enables spu debugging for a particular job
		virtual void EnableSPUJobDebugging(void*) = 0;
		//registers a variable to check if the profiling data should be transferred back
		virtual void RegisterProfileStatVar(int *pVar) = 0;
		//obtains and resets the SPU stats of the last frame, also adjusts SPU ID mapping
		virtual void GetAndResetSPUFrameStats(SSPUFrameStats& rStats) = 0;
		virtual void GetAndResetSPUFrameStats(SSPUFrameStats& rStats, std::vector<SFrameProfileData>& rCurFrameProfVec) = 0;
		//adds a job
		virtual const EAddJobRes AddJob
		(
			CSPUJobDel& __restrict crJob,
			const unsigned int cOpMode,
			const unsigned char cMinStackSizeKB,
			const TJobHandle cJobHandle,
			const unsigned int cIsDependentJob
		) = 0;

		//obtain job handle from name
		virtual const TJobHandle GetJobHandle(const char* cpJobName, const unsigned int cStrLen) const = 0;
	};
}//NPPU


// IJobManSPU dynamic library Export
typedef NPPU::IJobManSPU* (*PFNCREATEJOBMANINTERFACE)();

// interface of the DLL
extern "C" 
{
	NPPU::IJobManSPU* CreateJobManSPUInterface();
}

extern NPPU::IJobManSPU* GetIJobManSPU();

//headers located in Tools/PS3JobManager
//platform.h must not be included sicne it would make all memory allocation be listed under the launcher
#ifndef _ALIGN
	#define _ALIGN(num) __attribute__ ((aligned(num)))
#endif
#include <SPU/JobStructs.h>
#include <PPU/ProdConsQueue.h>

#endif //__SPU__

#else	//PS3
/*
//provide an implementation which can be called on any non ps3 platform evaluating to nothing
//avoids having nasty preprocessor defs

struct IJobManSPU
{
	const unsigned int GetSPUsAllowed() const{return 0;}
	void SetSPUsAllowed(const unsigned int)const{}
	const unsigned int GetDriverSize() const{return 0;}
	const bool InitSPUs(const char*)const{return true;}
	const bool SPUJobsActive() const{return false;}
	void ShutDown()const{}
	void TestSPUs() const{}
	void UpdateSPUMemMan()const{}
	void SetLog(ILog*)const{}
};

inline IJobManSPU *GetIJobManSPU()
{
	static IJobManSPU sJobManDummy;
	return &sJobManDummy;
}
*/
#endif //PS3
#endif //__IJOBMAN_SPU_H
	
