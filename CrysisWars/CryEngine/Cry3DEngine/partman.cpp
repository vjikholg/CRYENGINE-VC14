////////////////////////////////////////////////////////////////////////////
//
//  Crytek Engine Source File.
//  Copyright (C), Crytek Studios, 2002.
// -------------------------------------------------------------------------
//  File name:   partman.cpp
//  Version:     v1.00
//  Created:     28/5/2001 by Vladimir Kajalin
//  Compilers:   Visual Studio.NET
//  Description: manage particles and sprites
// -------------------------------------------------------------------------
//  History:
//	- 03:2006				 : Modified by Jan Muller (Serialization)
//
////////////////////////////////////////////////////////////////////////////

#include "StdAfx.h"
#include "partman.h"
#include "ParticleEmitter.h"
#include "3dEngine.h"
#include "ISerialize.h"
#include "ISound.h"

#define EFFECTS_FOLDER "Libs/Particles"
#define bUSE_CRY_THREAD	1

static constexpr char LEVEL_PARTICLE_LIBRARY[] = "Level";
static constexpr char LEVEL_PARTICLE_EFFECT_PREFIX[] = "Level.";

/*
	Particle threading strategy:

		MAIN THREAD														[DATA]									PARTICLE THREAD
			
			***
			PartMgr.Update:
				Halt threaded emitter update															
				#: [remaining]							
					Emitters.Update:								<=> Emitters
						evolve/kill, UpdateBounds,		=> UpdateQ: remove dead
						PlaySound
					register for rendering					=> 3DEngine
				transfer UpdateQ to NextQ					move NextQ => UpdateQ		----
																					UpdateQ: pop =>					pop, (queued current frame first)
																					Containers <=>					# Containers.UpdateParticles
			***																													
																					
			3DEngine visibility traversal:
				#: Emitter.Render:																			
					#: Container.Render:		
						create/submit to renderer			=> RenderObject
						queue for threaded update			=> UpdateQ: push
							(hi pri)
																																
																																
			Renderer preparation																			
				#: Container:
					SetVertices																						
						UpdateParticles								<=> Containers
						add to next frame's queue			=> NextQ: push
						compute/fill vertex buffer		=> Verts/Indices

*/

//////////////////////////////////////////////////////////////////////////
// Thread management.

#if bUSE_CRY_THREAD

#include "CryThread.h"

struct CParticleThread: CrySimpleThread<>
{
	CParticleThread(CPartManager* pPartMan)
		: m_pPartMan(pPartMan)
	{	
		Start();
	}

	virtual ~CParticleThread()
	{
		// Push 1 ptr to queue to signal canceling.
		m_pPartMan->QueueUpdate(reinterpret_cast<CParticleContainer*>(1), false);
		m_pPartMan->WakeParticleThread(true);

		// Wait for thread to quit.
		Join();
	}

protected:

	virtual void Run()
	{
		CryThreadSetName( -1, "ParticleUpdate" );		
		m_pPartMan->OnThreadUpdate();
	}

	CPartManager*		m_pPartMan;
};

#else

#include "IThreadTask.h"

struct CParticleThread: IThreadTask
{
	CParticleThread(CPartManager* pPartMan)
		: m_pPartMan(pPartMan)
	{
		SThreadTaskParams thread_params;
		thread_params.nPreferedThread = 1;
		thread_params.sName = "ParticleUpdate";
		m_pPartMan->GetSystem()->GetIThreadTaskManager()->RegisterTask( this, thread_params );
	}

	virtual ~CParticleThread()
	{
		m_pPartMan->GetSystem()->GetIThreadTaskManager()->UnregisterTask( this );
	}

protected:

	virtual void OnUpdate()
	{
		m_pPartMan->OnThreadUpdate();
	}

	CPartManager*	m_pPartMan;
};

#endif

#if defined(PS3)

inline CPartManager::Event::Event()
{
}

inline CPartManager::Event::~Event()
{
}

inline void CPartManager::Event::Wait()
{
	m_lockNotify.Lock();
	if (!m_flag)
		m_cond.Wait(m_lockNotify);
	m_lockNotify.Unlock();
}

inline void CPartManager::Event::Notify()
{
	m_lockNotify.Lock();
	m_flag = true;
	m_cond.Notify();
	m_lockNotify.Unlock();
}

#else //defined(PS3)

inline CPartManager::Event::Event()
{
	m_hEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
}

inline CPartManager::Event::~Event()
{
	CloseHandle(m_hEvent);
}

inline void CPartManager::Event::Wait()
{
	WaitForSingleObject(m_hEvent, INFINITE);
}

inline void CPartManager::Event::Notify()
{
	SetEvent(m_hEvent);
}

EVENT_HANDLE m_hEvent;

#endif //defined(PS3)

#if defined(CRY_PARTICLE_TEARDOWN_REGRESSION)
struct CPartManager::STeardownRegression
{
	CParticleContainer* pContainer;
	HANDLE dequeued = CreateEvent(NULL, TRUE, FALSE, NULL);
	HANDLE resume = CreateEvent(NULL, TRUE, FALSE, NULL);
	volatile LONG workerFinished = 0;
	unsigned int workerDelayMs;
	bool destroyedWhileRunning = false;
	bool destroyed = false;

	STeardownRegression(CParticleContainer* pTarget, unsigned int delayMs)
		: pContainer(pTarget), workerDelayMs(delayMs) {}

	~STeardownRegression()
	{
		if (resume)
			CloseHandle(resume);
		if (dequeued)
			CloseHandle(dequeued);
	}
};

void CPartManager::TestParticleTeardown(IConsoleCmdArgs* args)
{
	const int delayMs = args->GetArgCount() > 1 ? atoi(args->GetArg(1)) : 0;
	const bool passed = m_pPartManager && m_pPartManager->RunTeardownRegression(clamp_tpl(delayMs, 0, 100));
	CryLogAlways("Particle teardown regression %s", passed ? "PASS" : "FAIL");
}

bool CPartManager::RunTeardownRegression(unsigned int workerDelayMs)
{
	if (!IsThreading())
	{
		CryLogAlways("Particle teardown regression requires e_particles_thread and at least two cores");
		return false;
	}
	if (!m_pThreadTask)
		m_pThreadTask = new CParticleThread(this);

	ParticleParams params;
	params.bEnabled = true;
	CParticleEmitter* pEmitter = static_cast<CParticleEmitter*>(CreateEmitter(false, Matrix34(IDENTITY), nullptr, &params));
	if (pEmitter->m_SubEmitters.size() != 1)
	{
		DeleteEmitter(pEmitter);
		return false;
	}

	STeardownRegression test(&pEmitter->m_SubEmitters[0].GetContainer(), workerDelayMs);
	if (!test.dequeued || !test.resume)
	{
		DeleteEmitter(pEmitter);
		return false;
	}

	{
		AUTO_LOCK(m_ThreadLock);
		m_pTeardownRegression = &test;
		QueueUpdate(test.pContainer, false);
	}
	WakeParticleThread(true);

	const bool dequeued = WaitForSingleObject(test.dequeued, 5000) == WAIT_OBJECT_0;
	if (!dequeued)
		SetEvent(test.resume);
	DeleteEmitter(pEmitter);
	{
		AUTO_LOCK(m_ThreadLock);
		m_pTeardownRegression = nullptr;
	}

	CryLogAlways("Particle teardown regression dequeued=%d destroyed=%d early_destroy=%d delay_ms=%u",
		dequeued, test.destroyed, test.destroyedWhileRunning, workerDelayMs);
	return dequeued && test.destroyed && !test.destroyedWhileRunning;
}

void CPartManager::ObserveContainerDestruction(const CParticleContainer* pCont)
{
	if (m_pTeardownRegression && pCont == m_pTeardownRegression->pContainer)
	{
		m_pTeardownRegression->destroyed = true;
		m_pTeardownRegression->destroyedWhileRunning =
			InterlockedCompareExchange(&m_pTeardownRegression->workerFinished, 0, 0) == 0;
		SetEvent(m_pTeardownRegression->resume);
	}
}
#endif

void CPartManager::UnqueueUpdate(CParticleContainer* pCont)
{
	if (m_pThreadTask)
	{
		m_UpdateQueue.Replace(pCont, 0);
		m_NextQueue.Replace(pCont, 0);
	}
}

CPartManager::CTeardownFence::CTeardownFence(CPartManager& manager, const char* operation, bool needed)
	: m_pManager(needed ? &manager : nullptr), m_operation(operation)
{
	if (!m_pManager)
		return;

	m_profileWait = (GetCVars()->e_particles_debug & AlphaBit('w')) != 0;
	if (manager.m_ThreadLock.TryLock())
		return;

	const CTimeValue start = m_profileWait ? GetTimer()->GetAsyncTime() : CTimeValue();
#if defined(CRY_PARTICLE_TEARDOWN_REGRESSION)
	if (manager.m_pTeardownRegression)
		SetEvent(manager.m_pTeardownRegression->resume);
#endif
	manager.m_ThreadLock.Lock();
	if (m_profileWait)
		m_waitMs = (GetTimer()->GetAsyncTime() - start).GetMilliSeconds();
}

CPartManager::CTeardownFence::~CTeardownFence()
{
	if (!m_pManager)
		return;

	m_pManager->m_ThreadLock.Unlock();
	if (m_profileWait)
		CryLogAlways("Particle teardown fence %s wait_ms=%.4f", m_operation, m_waitMs);
}

void CPartManager::OnThreadUpdate()
{
	for (;;)
	{
		{
			// Indicate thread is processing queue data.
			AUTO_LOCK(m_ThreadLock);

			// Process everything in the queue.
			CParticleContainer* pCont;
			while (m_UpdateQueue.Pop(pCont))
			{
				// A 1-ptr is a signal from the main thread to exit.
				if (pCont == reinterpret_cast<CParticleContainer*>(1))
					return;

#if defined(CRY_PARTICLE_TEARDOWN_REGRESSION)
				if (m_pTeardownRegression && pCont == m_pTeardownRegression->pContainer)
				{
					SetEvent(m_pTeardownRegression->dequeued);
					if (WaitForSingleObject(m_pTeardownRegression->resume, 5000) == WAIT_OBJECT_0)
					{
						if (m_pTeardownRegression->workerDelayMs)
							Sleep(m_pTeardownRegression->workerDelayMs);
						InterlockedExchange(&m_pTeardownRegression->workerFinished, 1);
					}
					// The test observes premature destruction without dereferencing a freed container.
					continue;
				}
#endif
				if (pCont)
					pCont->OnThreadUpdate();
			}
		}

		// Wait until there is work to do. Note that it is slightly possible
		// that we could miss an event between checking the queue and this point.
		// However, this is not fatal, and will result only in a slight delay in
		// processing the update. We prefer the greatly reduced overhead of not
		// having to lock on the main thread.
		m_UpdateQueueEvent.Wait();
	}
}

void CPartManager::WakeParticleThread(bool force)
{
	if (m_pThreadTask)
	{
		// This check here can potentially cause some signals to be lost. However, this is not a fatal
		// situation, since the thread will be awakened next time this function is called, which is
		// many times per frame. The idea is to avoid using the locks inside the semaphore too much.
		if (force || (++m_EventFrequencyCounter) % 8 == 0)
		{
			FUNCTION_PROFILER_SYS(PARTICLE);
			m_UpdateQueueEvent.Notify();
		}
	}
}

//////////////////////////////////////////////////////////////////////////

AABB CPartManager::m_bbAreasChanged(AABB::RESET);

void ListParticles( IConsoleCmdArgs* args )
{
	ICVar* pVar = gEnv->pConsole->GetCVar("e_particles_debug");
	if (pVar)
		pVar->Set( int(pVar->GetIVal() | AlphaBit('s')) );
}

CPartManager::CPartManager( ) 
{
	m_bRuntime = false;
  m_pPartLightShader = MakeSystemMaterialFromShader("ParticlesNoMat");
	m_pThreadTask = 0;
	m_iEventSwitch = 1;
	m_EventFrequencyCounter = 0;

	GetPhysicalWorld()->AddEventClient(EventPhysAreaChange::id, &OnPhysAreaChange, 0);

	gEnv->pConsole->AddCommand("e_list_particles", &ListParticles, 0, "writes all effects used and counts to TestResults/particle_list.csv");
#if defined(CRY_PARTICLE_TEARDOWN_REGRESSION)
	gEnv->pConsole->AddCommand("e_test_particle_teardown", &TestParticleTeardown, VF_CHEAT,
		"Tests whether emitter deletion waits for a dequeued particle update. Optional worker delay in ms.");
#endif
}

CPartManager::~CPartManager() 
{ 
#if defined(CRY_PARTICLE_TEARDOWN_REGRESSION)
	gEnv->pConsole->RemoveCommand("e_test_particle_teardown");
#endif
	if (Get3DEngine()->GetIVisAreaManager())
		Get3DEngine()->GetIVisAreaManager()->RemoveListener(this);
	GetPhysicalWorld()->RemoveEventClient(EventPhysAreaChange::id, &OnPhysAreaChange, 0);
	Reset(false);
	delete m_pThreadTask;
}

bool CPartManager::IsActive( ParticleParams const& params ) const
{
	if (!params.bEnabled)
		return false;

	int quality = GetCVars()->e_particles_quality;
	if (quality == 0)
		quality = CONFIG_VERYHIGH_SPEC;
	EConfigSpecBrief eSysConfig = EConfigSpecBrief(quality - CONFIG_LOW_SPEC + ConfigSpec_Low);
	if (eSysConfig < params.eConfigMin || eSysConfig > params.eConfigMax)
		return false;
	if (!TrinaryMatch(params.tDX10, GetRenderer()->GetRenderType() == eRT_DX10))
		return false;
	if (!TrinaryMatch(params.tMultiThread, m_pThreadTask != 0))
		return false;
	return true;
}

struct SortParticleCount
{
	bool operator()(CParticleEffect* a, CParticleEffect* b)
	{
		return a->m_statsAll.ParticlesActive > b->m_statsAll.ParticlesActive;
	}
};

void CPartManager::Render(CObjManager* pObjManager, CTerrain* pTerrain) 
{
}

void CPartManager::GetCounts( SParticleCounts& counts )
{
  FUNCTION_PROFILER_SYS(PARTICLE);

	counts.Clear();
	bool bEffectStats = (GetCVars()->e_particles_debug & AlphaBit('s')) != 0;
	if (bEffectStats)
	{
		for_all (m_effects).ClearStats();
	}

	for_array (i, m_allEmitters)
	{
		// Count stats when debug flags indicate, but not for ignore-location emitters (in preview window etc).
		if (!m_allEmitters[i].m_SpawnParams.bIgnoreLocation)
			m_allEmitters[i].GetCounts( counts, bEffectStats );
	}

	if (bEffectStats)
	{
		// Print effect stats.
		DynArray<CParticleEffect*> aSortedEffects;
		aSortedEffects.resize(m_effects.size());
		for_array (i, aSortedEffects)
		{
			m_effects[i].SumStats();
			aSortedEffects[i] = &m_effects[i];
		}
		std::sort(aSortedEffects.begin(), aSortedEffects.end(), SortParticleCount());

		// Header for CSV-formatted emitter list.
		for_array (i, aSortedEffects)
			aSortedEffects[i]->PrintStats(i==0);
		GetCVars()->e_particles_debug &= ~AlphaBit('s');
	}
}

void CPartManager::GetMemoryUsage(ICrySizer* pSizer)
{
	pSizer->Add(*this);
	{
	  SIZER_COMPONENT_NAME(pSizer, "Effects");
		AddPtrContainer(pSizer, m_effects);
		AddContainer(pSizer, m_effectsMap);
		AddContainer(pSizer, m_loadedLibs);
	}
	{
	  SIZER_COMPONENT_NAME(pSizer, "Emitters");
		AddPtrContainer(pSizer, m_allEmitters);
	}
}

void CPartManager::Reset(bool bIndependentOnly)
{
	CTeardownFence fence(*this, "Reset");
	m_bRuntime = false;

	if (bIndependentOnly)
		for_array (i, m_allEmitters)
		{
			if (m_allEmitters[i].IsIndependent())
				i = DeleteEmitter(i);
		}
	else
	{
		for_all (m_allEmitters).UnqueueUpdates();
		m_allEmitters.clear();
	}

	ReleaseLocks();
}

//////////////////////////////////////////////////////////////////////////
void CPartManager::ClearRenderResources( bool bForceClear )
{
	CTeardownFence fence(*this, "ClearRenderResources");
	for_all (m_allEmitters).UnqueueUpdates();
	m_allEmitters.clear();

	if (!GetCVars()->e_particles_preload || bForceClear)
	{
		m_effectsMap.clear();
		m_effects.clear();
		m_loadedLibs.clear();
	}
	else if (!m_pSystem->IsEditor())
	{
		ClearLevelLibrary();
	}
	m_bRuntime = false;
	ReleaseLocks();
}

//////////////////////////////////////////////////////////////////////////
// Particle Effects.
//////////////////////////////////////////////////////////////////////////
IParticleEffect* CPartManager::CreateEffect()
{
	CParticleEffect* pEffect = new CParticleEffect();
	m_effects.push_back( pEffect );
	return pEffect;
}

//////////////////////////////////////////////////////////////////////////
void CPartManager::RenameEffect( IParticleEffect *pEffect,const char *sNewName )
{
	assert( pEffect );
	if (!pEffect->GetParent())
	{
		const char *sOldName = pEffect->GetName();
		if (*sOldName)
		{
			// Delete old name.
			m_effectsMap[sOldName] = 0;
		}
		if (*sNewName)
		{
			// Add new name.
			if (m_effectsMap[sNewName])
 				Warning( "Loading duplicate particle effect %s", sNewName);
			m_effectsMap[sNewName] = pEffect;
		}
	}
}

//////////////////////////////////////////////////////////////////////////
void CPartManager::RemoveEffect( IParticleEffect *pEffect )
{
	assert( pEffect );

	if (!pEffect->GetParent())
	{
		const char *sOldName = pEffect->GetName();
		pEffect->Release();
		stl::find_and_erase( m_effects,pEffect );
		if (*sOldName)
			// Delete old name.
			m_effectsMap[sOldName] = 0;
	}
}

//////////////////////////////////////////////////////////////////////////
IParticleEffect* CPartManager::FindEffect( const char* sEffectName, const char* sCallerType, const char* sCallerName, bool bLoad )
{
	if (!sEffectName || !*sEffectName)
		return NULL;

	EffectsMap::iterator it = m_effectsMap.find(CONST_TEMP_STRING(sEffectName));
	if (it == m_effectsMap.end())
	{
		if (bLoad)
		{
			// Try to load the lib.
			const char* dot = strchr(sEffectName,'.');
			if (dot)
			{
				string sLibraryName(sEffectName,dot-sEffectName);
				LoadLibrary(sLibraryName);

				// Try it again.
				it = m_effectsMap.find(CONST_TEMP_STRING(sEffectName));
			}
			if (it == m_effectsMap.end())
			{
				// Not found. Add to map anyway to avoid duplicate warnings.
				m_effectsMap[sEffectName] = NULL;
				Warning( "Particle effect not found: '%s', from %s %s", 
					sEffectName, *sCallerType ? sCallerType : "unknown", sCallerName );
				return NULL;
			}
		}
	}
	if (it != m_effectsMap.end())
	{
		IParticleEffect* pEffect = it->second;
		if (pEffect)
			it->second->AddRef();

		if (pEffect && bLoad)
			if (pEffect->LoadResources() && m_bRuntime)
				CryLog( "Particle effect loaded at runtime: '%s', from %s %s",
					sEffectName, *sCallerType ? sCallerType : "unknown", sCallerName );
		return pEffect;
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////

void CPartManager::LogEvents()
{
#if bEVENT_TIMINGS
	AUTO_LOCK(m_Lock);

	static float timeThreshold = 0.f;						// Dump events when critical funcs exceed threshold.

	if (GetCVars()->e_particles_debug & AlphaBits('ed'))
	{
		if (!m_aEvents.size())
		{
			// Start timing this frame.
			AddEventTiming("*Frame", 0);
		}
		else
		{
			// Finish and log frame.
			m_aEvents[0].timeEnd = max(GetTimer()->GetAsyncCurTime(), m_aEvents[0].timeStart + GetTimer()->GetRealFrameTime());
			float timeFrameStart = m_aEvents[0].timeStart,
						timeFrameTime = m_aEvents[0].timeEnd - m_aEvents[0].timeStart;

			if (timeFrameTime > 0.f)
			{
				static const int nWidth = 50;
				int nSumStart = m_aEvents.size();
				float timeTotal = 0.f;

				// Add to summary entries at end.
				for (int e = 0; e < nSumStart; e++)
				{
					SEventTiming* pEvent = &m_aEvents[e];
					if (pEvent->nContainerId)
					{
						timeTotal += pEvent->timeEnd - pEvent->timeStart;
						int s;
						for (s = nSumStart; s < m_aEvents.size(); s++)
						{
							SEventTiming* pSum = &m_aEvents[s];
							if (pSum->sEvent == pEvent->sEvent && pSum->nThread == pEvent->nThread)
								break;
						}
						if (s == m_aEvents.size())
						{
							// Add to summary.
							SEventTiming* pSum = m_aEvents.push_back();
							*pSum = m_aEvents[e];
							pSum->nContainerId = 0;
						}
					}
				}

				if (timeThreshold == 0.f)
					// Default time threshold is 8 ms
					timeThreshold = 0.008f;

				// Check against time threshold.
				if ((GetCVars()->e_particles_debug & AlphaBit('e')) || timeTotal >= timeThreshold)
				{
					// Increase threshold for next time.
					timeThreshold *= 2.f;
					for_array (c, m_aEvents)
					{
						SEventTiming* pEventC = &m_aEvents[c];
						if (!pEventC->sEvent)
							continue;

						// Iterate unique threads.
						for (int t = c; t < m_aEvents.size(); t++)
						{
							SEventTiming* pEventT = &m_aEvents[t];
							if (!pEventT->sEvent)
								continue;
							if (pEventT->nContainerId != pEventC->nContainerId)
								continue;

							const char* sThread = CryThreadGetName(pEventT->nThread);
							if (pEventT == pEventC)			// Main thread.
								GetLog()->LogToFile( "%*s %s(%p) @%s",
									nWidth, "", pEventC->pEffect ? pEventC->pEffect->GetName() : "",
									reinterpret_cast<void*>(pEventC->nContainerId), sThread );
							else
								GetLog()->LogToFile( "%*s   @%s", nWidth, "", sThread );

							// Log event times.
							for (int e = t; e < m_aEvents.size(); e++)
							{
								SEventTiming* pEvent = &m_aEvents[e];
								if (!pEvent->sEvent)
									continue;
								if (pEvent->nContainerId != pEventT->nContainerId || pEvent->nThread != pEventT->nThread)
									continue;

								// Construct thread timeline.
								char sGraph[nWidth+1];
								memset(sGraph, ' ', nWidth);
								sGraph[nWidth] = 0;

								int start_iter = pEventC->nContainerId ? e : 0;
								int end_iter = pEventC->nContainerId ? e+1 : nSumStart;
								float timeStart = m_aEvents[e].timeStart, 
											timeEnd = m_aEvents[e].timeEnd,
											timeTotal = 0.f;
								for (int i = start_iter; i < end_iter; i++)
								{
									SEventTiming* pEventI = &m_aEvents[i];
									if (pEventI->sEvent != pEvent->sEvent || pEventI->nThread != pEvent->nThread)
										continue;
 
									timeStart = min(timeStart, pEventI->timeStart);
									timeEnd = max(timeEnd, pEventI->timeEnd);
									timeTotal += pEventI->timeEnd - pEventI->timeStart;

									int nEventStart = int_round((pEventI->timeStart - timeFrameStart) * nWidth / timeFrameTime);
									int nEventEnd = int_round((pEventI->timeEnd - timeFrameStart) * nWidth / timeFrameTime);

									nEventStart = min(nEventStart, nWidth-1);
									nEventEnd = min(max(nEventEnd, nEventStart+1), nWidth);

									const char* sEvent = strrchr(pEventI->sEvent, ':');
									char cEvent = sEvent ? sEvent[1] : *pEventI->sEvent;
									memset(sGraph+nEventStart, cEvent, nEventEnd-nEventStart);
								}

								GetLog()->LogToFile( "%s     %.3f-%.3f [%.3f] %s",
									sGraph,
									(timeStart - timeFrameStart)*1000.f, (timeEnd - timeFrameStart)*1000.f, timeTotal*1000.f,
									pEvent->sEvent );

								pEvent->sEvent = 0;
							}
						}
					}
				}
			}

			// Clear log.
			GetCVars()->e_particles_debug &= ~AlphaBit('e');
			m_aEvents.resize(0);
			if (GetCVars()->e_particles_debug & AlphaBit('d'))
				// Keep timing every frame.
				AddEventTiming("*Frame", 0);
		}
	}
	else
	{
		m_aEvents.resize(0);
		timeThreshold = 0.f;
	}
	m_iEventSwitch *= -1;
#endif
}

void CPartManager::OnFrameStart()
{
	LogEvents();

	// Manage threading.
	if (IsThreading())
	{
		if (!m_pThreadTask)
			m_pThreadTask = new CParticleThread(this);

		m_UpdateQueue.Clear();
	}
	else
	{
		if (m_pThreadTask)
		{
			delete m_pThreadTask;
			m_pThreadTask = 0;
			m_UpdateQueue.Clear();
			m_NextQueue.Clear();
		}
	}
}

void CPartManager::Update()
{
	FUNCTION_PROFILER_CONTAINER(0);

	static const float fMAX_WORLD = 1e6f;

	if (IsThreading())
	{
		// Wait for thread to really finish.
		AUTO_LOCK(m_ThreadLock);
	}

	if (GetSystem()->IsPaused() || (GetCVars()->e_particles_debug & AlphaBit('z')))
	{
		for_array (i, m_allEmitters)
			m_allEmitters[i].ClearCounts();
		return;
	}
	
	m_bRuntime = !m_pSystem->IsEditorMode();
	m_tLastUpdate = GetTimer()->GetFrameStartTime();
	m_RenderCam = m_pSystem->GetViewCamera();

	if(I3DEngine* pEngine = Get3DEngine())
		if(IVisAreaManager* pVisMan = pEngine->GetIVisAreaManager())
			pVisMan->AddListener(this);

	// Initial pass to update particle-generated forces.
	for_array (i, m_allEmitters)
	{
		m_allEmitters[i].UpdateForce();
	}

	// Main pass to update state.
	for_array (i, m_allEmitters)
	{
		CParticleEmitter* pEmitter = &m_allEmitters[i];

		// Evolve emitters (and particles of dynamic-bounds emitters).
		pEmitter->Update();

		// Cull dead emitters.
		if (!pEmitter->IsAlive())
			i = DeleteEmitter(i);
		else
			pEmitter->Register(true);
	}

	m_bbAreasChanged.Reset();

	if (IsThreading())
	{
		// Transfer update queue from last frame to current.
		m_UpdateQueue.Swap(m_NextQueue);
	}
}

//////////////////////////////////////////////////////////////////////////
IParticleEmitter* CPartManager::CreateEmitter( bool bIndependent, Matrix34 const& mLoc, const IParticleEffect* pEffect, const ParticleParams* pParams )
{
	assert(pEffect || pParams);
	FUNCTION_PROFILER_SYS(PARTICLE);

	CParticleEmitter *pEmitter = new CParticleEmitter(bIndependent);
	if (pEmitter)
	{
		pEmitter->SetEffect(pEffect, pParams);
		pEmitter->SetMatrix(mLoc);
		RegisterEmitter(pEmitter);
	}
	return pEmitter;
}

//////////////////////////////////////////////////////////////////////////
void CPartManager::DeleteEmitter( IParticleEmitter *pEmitter)
{
	if (pEmitter)
	{
		for_array (i, m_allEmitters)
		{
			if (&m_allEmitters[i] == pEmitter)
			{
				DeleteEmitter(i);
				break;
			}
		}
	}
}

int CPartManager::DeleteEmitter( int i )
{
	CTeardownFence fence(*this, "DeleteEmitter");
	m_allEmitters[i].Register(false);
	return m_allEmitters.erase(i);
}

void CPartManager::RegisterEmitter( CParticleEmitter* pEmitter )
{
	if (!pEmitter->IsRegistered())
	{
		pEmitter->Register(true);
		m_allEmitters.push_back(pEmitter);
	}
}

void CPartManager::UpdateEmitters( IParticleEffect *pEffect )
{
	// Update all emitters with this effect tree.
	while (pEffect->GetParent())
		pEffect = pEffect->GetParent();
	for_array (i, m_allEmitters)
	{
		if (m_allEmitters[i].GetEffect() == pEffect)
			m_allEmitters[i].SetEffect(pEffect);
	}
}

int CPartManager::OnPhysAreaChange(const EventPhys *pEvent)
{
	EventPhysAreaChange const& epac = (EventPhysAreaChange const&)*pEvent;
	m_bbAreasChanged.Add( AABB(epac.boxAffected[0], epac.boxAffected[1]) );
	return 0;
}

void CPartManager::OnVisAreaDeleted( IVisArea* pVisArea )
{
	AUTO_LOCK(m_Lock);
	for_all(m_allEmitters).OnVisAreaDeleted(pVisArea);
}

//////////////////////////////////////////////////////////////////////////
CryLock<CRYLOCK_RECURSIVE>* CPartManager::AllocLock()
{
	AUTO_LOCK(m_Lock);
	if (m_aFreeLocks.size() > 0)
	{
		CryLock<CRYLOCK_RECURSIVE>* pLock = m_aFreeLocks.back();
		assert(!pLock->IsLocked());
		m_aFreeLocks.pop_back();
		return pLock;
	}
	return new(m_poolLocks) CryLock<CRYLOCK_RECURSIVE>;
}

void CPartManager::FreeLock(CryLock<CRYLOCK_RECURSIVE>* pLock)
{
	assert(!pLock->IsLocked());
	AUTO_LOCK(m_Lock);
 	m_aFreeLocks.push_back(pLock);
}

void CPartManager::ReleaseLocks()
{
	AUTO_LOCK(m_Lock);
	for (int i = m_aFreeLocks.size()-1; i >= 0; i--)
		m_poolLocks.Delete(m_aFreeLocks[i]);
	m_aFreeLocks.clear();
}

//////////////////////////////////////////////////////////////////////////
bool CPartManager::LoadLevelLibrary(const char* libraryFile)
{
	{
		CTeardownFence fence(*this, "LoadLevelLibrary");
		ClearLevelLibrary();
	}
	return LoadLibrary(LEVEL_PARTICLE_LIBRARY, libraryFile, true);
}

void CPartManager::ClearLevelLibrary()
{
	m_loadedLibs.erase(LEVEL_PARTICLE_LIBRARY);

	// This also clears cached misses for names from the outgoing level.
	auto entry = m_effectsMap.lower_bound(LEVEL_PARTICLE_EFFECT_PREFIX);
	while (entry != m_effectsMap.end() &&
		strnicmp(entry->first.c_str(), LEVEL_PARTICLE_EFFECT_PREFIX, sizeof(LEVEL_PARTICLE_EFFECT_PREFIX) - 1) == 0)
	{
		entry = m_effectsMap.erase(entry);
	}

	const auto isLevelEffect = [](const _smart_ptr<CParticleEffect>& effect)
	{
		return strnicmp(effect->GetName(), LEVEL_PARTICLE_EFFECT_PREFIX, sizeof(LEVEL_PARTICLE_EFFECT_PREFIX) - 1) == 0;
	};
	m_effects.erase(std::remove_if(m_effects.begin(), m_effects.end(), isLevelEffect), m_effects.end());
}

//////////////////////////////////////////////////////////////////////////
bool CPartManager::LoadLibrary( const char *sParticlesLibrary, const char *sParticlesLibraryFile, bool bLoadResources )
{
	if (strchr(sParticlesLibrary, '*'))
	{
		// Wildcard load.
		string sLibPath = PathUtil::Make( EFFECTS_FOLDER, sParticlesLibrary, "xml" );
		ICryPak *pack = gEnv->pCryPak;
		_finddata_t fd;
		intptr_t handle = pack->FindFirst( sLibPath, &fd );
		int nCount = 0;
		if (handle >= 0)
		{
			do {
				if (LoadLibrary( PathUtil::GetFileName(fd.name), NULL, bLoadResources ))
					nCount++;
			} while (pack->FindNext( handle, &fd ) >= 0);
			pack->FindClose(handle);
		}
		return nCount > 0;
	}
	else if (sParticlesLibrary[0] == '@')
	{
		int nCount = 0;
		string sFilename = PathUtil::Make( EFFECTS_FOLDER, sParticlesLibrary+1, "txt" );
		CCryFile file;
		if (file.Open( sFilename, "r" ))
		{
			const size_t fileLength = file.GetLength();
			string sAllText;
			sAllText.assign(fileLength, '\n');
			file.ReadRaw(sAllText.begin(), fileLength);

			int tokenOffset = 0;
			string sLine;
			while (!(sLine = sAllText.Tokenize("\r\n", tokenOffset)).empty())
			{
				if (m_pPartManager->LoadLibrary( sLine, NULL, bLoadResources ))
					nCount++;
			}
		}
		return nCount > 0;
	}

	// The runtime Level library belongs to the active map and has an explicit path.
	if (!sParticlesLibraryFile && stricmp(sParticlesLibrary, LEVEL_PARTICLE_LIBRARY) == 0 && !m_pSystem->IsEditor())
		return false;

	if (m_loadedLibs.find(sParticlesLibrary) != m_loadedLibs.end())
	{
		// Already loaded.
		if (bLoadResources)
		{
			// Iterate all fx in lib and load their resources.
			string sPrefix = sParticlesLibrary;
			sPrefix += ".";
			for_array (i, m_effects)
			{
				if (strnicmp(m_effects[i].GetName(), sPrefix, sPrefix.size()) == 0)
					m_effects[i].LoadResources();
			}
		}
		return true;
	}

	string sLibFilename;
	if (sParticlesLibraryFile)
		sLibFilename = sParticlesLibraryFile;
	else
		sLibFilename = PathUtil::Make( EFFECTS_FOLDER,sParticlesLibrary,"xml" );

	if (m_bRuntime)
		Warning( "Particle library loaded at runtime: %s", sParticlesLibrary );

	XmlNodeRef libNode = GetISystem()->LoadXmlFile( sLibFilename );
	if (!libNode)
		return false;

	m_loadedLibs.insert(sParticlesLibrary);
	gEnv->pLog->LogToFile( "Loading Particle Library: %s",sParticlesLibrary );	// to file only so console is cleaner
	gEnv->pLog->UpdateLoadingScreen(0);

	// Load all nodes from the particle libaray.
	string sLibName = sParticlesLibrary;
	const string::size_type dot = sLibName.find('.');
	if (dot != string::npos)
		sLibName.erase(dot+1);
	else
		sLibName += ".";

	for (int i = 0; i < libNode->getChildCount(); i++)
	{
		XmlNodeRef effectNode = libNode->getChild(i);
		if (effectNode->isTag("Particles"))
		{
			// Skip existing effects.
			string sEffectName = sParticlesLibrary;
			sEffectName += ".";
			sEffectName += effectNode->getAttr("Name");
			IParticleEffect* pEffect = m_effectsMap[sEffectName];
			if (!pEffect)
			{
				pEffect = CreateEffect();
				pEffect->Serialize( effectNode, true, true );
			}

			if (bLoadResources)
				pEffect->LoadResources();

			if (GetCVars()->e_particles_debug & AlphaBit('t'))
				PrintTextures(pEffect);
		}
	}

	return true;
}

//////////////////////////////////////////////////////////////////////////
void CPartManager::PrintTextures( IParticleEffect* pEffect )
{
	PrintMessage( string("Particle ") + pEffect->GetName() + ", Texture " + pEffect->GetParticleParams().sTexture );
	for (int i = 0; i < pEffect->GetChildCount(); i++)
		PrintTextures( pEffect->GetChild(i) );
}

void CPartManager::RenderDebugInfo()
{
  if ((GetCVars()->e_particles_debug & AlphaBit('b')) || GetSystem()->IsEditorMode())
  {
		// Debug particle BBs.
		for_array (i, m_allEmitters)
			m_allEmitters[i].RenderDebugInfo();
	}
}

#if bEVENT_TIMINGS

int CPartManager::AddEventTiming( const char* sEvent, const CParticleContainer* pCont )
{
	AUTO_LOCK(m_Lock);

	if (!m_aEvents.size() && *sEvent != '*')
		return -1;

	SEventTiming* pEvent = m_aEvents.push_back();
	pEvent->nContainerId = reinterpret_cast<uintptr_t>(pCont);
	pEvent->pEffect = pCont ? pCont->GetEffect() : 0;
	pEvent->nThread = GetCurrentThreadId();
	pEvent->sEvent = sEvent;
	pEvent->timeStart = GetTimer()->GetAsyncCurTime();

	return m_aEvents.size()-1;
}

#endif

//////////////////////////////////////////////////////////////////////////
void CPartManager::Serialize(TSerialize ser)
{
	ser.BeginGroup("ParticleEmitters");

	if (ser.IsWriting())
	{
		ListEmitters("before save");

		int nCount = 0;
		for_array (i, m_allEmitters)
		{
			if (m_allEmitters[i].NeedSerialize())
				nCount++;
		}
		ser.Value("Emitters", nCount);

		for_array (i, m_allEmitters)
		{
			if (m_allEmitters[i].NeedSerialize())
				m_allEmitters[i].Serialize(ser);
		}
	}
	else
	{
		ListEmitters("before load");
		m_bRuntime = false;

		// Clean up existing emitters.
		for_array (i, m_allEmitters)
		{
			if (m_allEmitters[i].IsIndependent())
				// Would be serialized.
				i = DeleteEmitter(i);
			else if (m_allEmitters[i].GetAge() < 0.f ||
			m_allEmitters[i].GetAge() > m_allEmitters[i].GetStopAge())
				// No longer exists at new time.
				i = DeleteEmitter(i);
			else
				// Restart at age 0, in case expired.
				m_allEmitters[i].Activate(eAct_Reactivate);
		}

		int nCount = 0;
		ser.Value("Emitters", nCount);
		while (nCount-- > 0)
		{
			CParticleEmitter *pEmitter = new CParticleEmitter(true);
			pEmitter->Serialize(ser);
			if (pEmitter->GetEffect())
				RegisterEmitter(pEmitter);
			else
				delete pEmitter;
		}
	}
	ser.EndGroup();
}

void CPartManager::ListEmitters( const char* sDesc )
{
	if (GetCVars()->e_particles_debug & AlphaBit('l'))
	{
		// Count emitters.
		int anEmitters[eEmitter_Active+1] = {0};
		for_array (i, m_allEmitters)
			anEmitters[min(m_allEmitters[i].GetState(), eEmitter_Active)]++;

		// Log summary, and state of each emitter.
		CryLog( "Emitters %s: time %.3f (prev %.3f), %d active, %d inactive, %d dormant, %d dead", 
			sDesc, GetTimer()->GetFrameStartTime().GetSeconds(), m_tLastUpdate.GetSeconds(), 
			anEmitters[eEmitter_Active], anEmitters[eEmitter_Particles], anEmitters[eEmitter_Dormant], anEmitters[eEmitter_Dead] );
		for_array (i, m_allEmitters)
			CryLog( " Effect %s", m_allEmitters[i].GetDebugString('s').c_str() );
	}
}
