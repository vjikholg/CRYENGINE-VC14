#define NOMINMAX
#define WIN32_LEAN_AND_MEAN

#include "PhysicsRegressionTests.h"
#include "TestRunner.h"

#pragma warning(push, 0)
#include <ISystem.h>
#include <IConsole.h>
#include <primitives.h>
#include <physinterface.h>
#pragma warning(pop)

#include <windows.h>

#include <algorithm>
#include <array>
#include <atomic>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <iterator>
#include <string>
#include <thread>
#include <utility>
#include <vector>

struct PhysicsRegressionSystemCallback : ISystemUserCallback
{
	bool OnError(const char* errorString) override
	{
		std::fprintf(
			stderr,
			"CrySystem error: %s\n",
			errorString != nullptr ? errorString : "<null>");
		return false;
	}

	void OnSaveDocument() override {}
	void OnProcessSwitch() override {}
	void OnInitProgress(const char* progressMessage) override
	{
		if (progressMessage != nullptr)
			std::printf("CrySystem: %s\n", progressMessage);
	}
	void GetMemoryUsage(ICrySizer*) override {}
};

struct PhysicsRegressionRuntime
{
	using CreateSystemFunction = ISystem* (__cdecl*)(const SSystemInitParams& params);

	HMODULE crySystemModule = nullptr;
	ISystem* system = nullptr;
	IPhysicalWorld* world = nullptr;
	std::filesystem::path previousWorkingDirectory;
	PhysicsRegressionSystemCallback systemCallback;
	HWND testWindow = nullptr;
};

struct PhysicsRegressionSnapshot
{
	Vec3 position = Vec3(ZERO);
	quaternionf orientation = quaternionf(IDENTITY);
	Vec3 velocity = Vec3(ZERO);
	Vec3 angularVelocity = Vec3(ZERO);
	float energy = 0.0f;
	int contactCount = 0;
	int simulationClass = 0;
	int entityId = -1;
	int awake = 0;
	bool hasPosition = false;
	bool hasDynamics = false;
};

struct PhysicsRegressionContext
{
	IPhysicalWorld* world = nullptr;
	std::vector<IPhysicalEntity*> entities;
	std::ofstream* report = nullptr;
	const char* caseName = nullptr;
	size_t snapshotSequence = 0;
};

using PhysicsRegressionTestFunction = void (*)(
	CryTestRunner& runner,
	PhysicsRegressionContext& context);

struct PhysicsRegressionCase
{
	const char* name;
	const char* category;
	PhysicsRegressionTestFunction function;
};

struct PhysicsRegressionCollisionProbe
{
	IPhysicalWorld* world = nullptr;
	IPhysicalEntity* first = nullptr;
	IPhysicalEntity* second = nullptr;
	IPhysicalEntity* destroyTarget = nullptr;
	int collisionCount = 0;
	int validOwnershipCount = 0;
	bool destroyedTarget = false;
};

struct PhysicsRegressionRenderer final : IPhysRenderer
{
	size_t geometryCount = 0;
	size_t lineCount = 0;
	size_t textCount = 0;
	std::array<size_t, 8> lineCountBySimulationClass{};
	std::uint64_t lineChecksum = 1469598103934665603ULL;
	IPhysicalWorld* world = nullptr;
	IPhysicalEntity* destroyTarget = nullptr;
	bool destructionAttempted = false;
	bool destroyedTarget = false;

	void HashBytes(const void* data, size_t size)
	{
		const auto* bytes = static_cast<const unsigned char*>(data);
		for (size_t index = 0; index < size; ++index)
		{
			lineChecksum ^= bytes[index];
			lineChecksum *= 1099511628211ULL;
		}
	}

	void DrawGeometry(
		IGeometry*,
		geom_world_data*,
		int,
		int,
		const Vec3&) override
	{
		++geometryCount;
	}

	void DrawLine(const Vec3& start, const Vec3& end, int color, int flags) override
	{
		if (!destructionAttempted && world != nullptr && destroyTarget != nullptr)
		{
			destructionAttempted = true;
			destroyedTarget = world->DestroyPhysicalEntity(destroyTarget) != 0;
			world->PurgeDeletedEntities();
		}

		++lineCount;
		if (color >= 0 && static_cast<size_t>(color) < lineCountBySimulationClass.size())
			++lineCountBySimulationClass[static_cast<size_t>(color)];
		HashBytes(&start, sizeof(start));
		HashBytes(&end, sizeof(end));
		HashBytes(&color, sizeof(color));
		HashBytes(&flags, sizeof(flags));
	}

	const char* GetForeignName(void*, int, int) override
	{
		return "";
	}

	void DrawText(const Vec3&, const char*, int, float) override
	{
		++textCount;
	}
};

static PhysicsRegressionCollisionProbe* g_physicsRegressionCollisionProbe = nullptr;

static int PhysicsRegressionOnCollision(const EventPhys* event)
{
	PhysicsRegressionCollisionProbe* probe = g_physicsRegressionCollisionProbe;
	if (probe == nullptr || event == nullptr || event->idval != EventPhysCollision::id)
		return 1;

	const EventPhysCollision* collision = static_cast<const EventPhysCollision*>(event);
	++probe->collisionCount;
	const bool expectedPair =
		(collision->pEntity[0] == probe->first && collision->pEntity[1] == probe->second) ||
		(collision->pEntity[0] == probe->second && collision->pEntity[1] == probe->first);
	if (expectedPair)
	{
		const int firstId = probe->world->GetPhysicalEntityId(collision->pEntity[0]);
		const int secondId = probe->world->GetPhysicalEntityId(collision->pEntity[1]);
		if (firstId >= 0 && secondId >= 0 &&
			probe->world->GetPhysicalEntityById(firstId) == collision->pEntity[0] &&
			probe->world->GetPhysicalEntityById(secondId) == collision->pEntity[1])
		{
			++probe->validOwnershipCount;
		}

		if (probe->destroyTarget != nullptr && !probe->destroyedTarget)
		{
			probe->destroyedTarget =
				probe->world->DestroyPhysicalEntity(probe->destroyTarget, 0, 1) != 0;
		}
	}
	return 1;
}

static bool PhysicsRegressionSetDllDirectory(const wchar_t* directory)
{
	using SetDllDirectoryFunction = BOOL (WINAPI*)(LPCWSTR pathName);
	HMODULE kernel32 = GetModuleHandleW(L"kernel32.dll");
	if (kernel32 == nullptr)
		return false;
	const auto setDllDirectory = reinterpret_cast<SetDllDirectoryFunction>(
		GetProcAddress(kernel32, "SetDllDirectoryW"));
	return setDllDirectory != nullptr && setDllDirectory(directory) != FALSE;
}

static void PhysicsRegressionShutdownRuntime(PhysicsRegressionRuntime& state)
{
	state.world = nullptr;
	if (state.system != nullptr)
	{
		state.system->Release();
		state.system = nullptr;
	}
	if (state.crySystemModule != nullptr)
	{
		FreeLibrary(state.crySystemModule);
		state.crySystemModule = nullptr;
	}
	if (state.testWindow != nullptr)
	{
		DestroyWindow(state.testWindow);
		state.testWindow = nullptr;
	}
	PhysicsRegressionSetDllDirectory(nullptr);
	if (!state.previousWorkingDirectory.empty())
		SetCurrentDirectoryW(state.previousWorkingDirectory.c_str());
}

static bool PhysicsRegressionInitializeRuntime(
	const std::filesystem::path& runtimeDirectory,
	const std::filesystem::path& gameRoot,
	PhysicsRegressionRuntime& state,
	std::string& error)
{
	std::error_code statusError;
	if (!std::filesystem::is_directory(runtimeDirectory, statusError))
	{
		error = "Runtime directory is not accessible";
		return false;
	}
	if (!std::filesystem::is_directory(gameRoot, statusError))
	{
		error = "Game root is not accessible";
		return false;
	}

	state.previousWorkingDirectory = std::filesystem::current_path(statusError);
	if (statusError || !SetCurrentDirectoryW(gameRoot.c_str()))
	{
		error = "Unable to set the game working directory";
		return false;
	}
	if (!PhysicsRegressionSetDllDirectory(runtimeDirectory.c_str()))
	{
		error = "Unable to set the runtime DLL directory";
		return false;
	}

	const std::filesystem::path crySystemPath = runtimeDirectory / L"CrySystem.dll";
	state.crySystemModule = LoadLibraryExW(
		crySystemPath.c_str(),
		nullptr,
		LOAD_WITH_ALTERED_SEARCH_PATH);
	if (state.crySystemModule == nullptr)
	{
		error = "Unable to load CrySystem.dll, Windows error " +
			std::to_string(GetLastError());
		return false;
	}

	const auto createSystem = reinterpret_cast<PhysicsRegressionRuntime::CreateSystemFunction>(
		GetProcAddress(state.crySystemModule, "CreateSystemInterface"));
	if (createSystem == nullptr)
	{
		error = "CrySystem.dll does not export CreateSystemInterface";
		return false;
	}

	state.testWindow = CreateWindowExW(
		0,
		L"STATIC",
		L"CryCommonTests physics regression",
		WS_POPUP,
		0,
		0,
		64,
		64,
		nullptr,
		nullptr,
		GetModuleHandleW(nullptr),
		nullptr);
	if (state.testWindow == nullptr)
	{
		error = "Unable to create the hidden renderer test window";
		return false;
	}

	SSystemInitParams startupParams;
	std::memset(&startupParams, 0, sizeof(startupParams));
	startupParams.hInstance = GetModuleHandleW(nullptr);
	startupParams.hWnd = state.testWindow;
	startupParams.sLogFileName = "PhysicsRegression.log";
	startupParams.pUserCallback = &state.systemCallback;
	startupParams.bPreview = true;
	startupParams.bTestMode = true;
	startupParams.bDedicatedServer = false;
	strncpy_s(
		startupParams.szSystemCmdLine,
		sizeof(startupParams.szSystemCmdLine),
		"-devmode -DX10 -PhysicsRegressionTest",
		_TRUNCATE);

	state.system = createSystem(startupParams);
	if (state.system == nullptr)
	{
		error = "CreateSystemInterface failed; inspect PhysicsRegression.log";
		return false;
	}
	if (IConsole* console = state.system->GetIConsole())
	{
		if (ICVar* exitOnQuit = console->GetCVar("ExitOnQuit"))
			exitOnQuit->Set(0);
	}
	state.world = state.system->GetIPhysicalWorld();
	if (state.world == nullptr)
	{
		error = "CrySystem did not initialize CryPhysics";
		return false;
	}

	state.world->SetupEntityGrid(
		2,
		Vec3(-128.0f, -128.0f, -64.0f),
		16,
		16,
		16.0f,
		16.0f);
	state.world->SetPhysicsTime(0.0f);
	state.world->SetiPhysicsTime(0);
	state.world->SetSurfaceParameters(0, 0.0f, 0.5f);

	PhysicsVars* variables = state.world->GetPhysVars();
	if (variables == nullptr)
	{
		error = "CryPhysics did not expose PhysicsVars";
		return false;
	}
	variables->bSingleStepMode = 0;
	variables->bDoStep = 0;
	variables->fixedTimestep = 0.0f;
	variables->timeGranularity = 0.0001f;
	variables->maxWorldStep = 0.1f;
	variables->nMaxSubsteps = 8;
	variables->gravity = Vec3(0.0f, 0.0f, -9.81f);
	variables->bMultiplayer = 0;
	variables->timeScalePlayers = 1.0f;
	return true;
}

static void PhysicsRegressionDestroyEntities(PhysicsRegressionContext& context)
{
	for (auto entity = context.entities.rbegin(); entity != context.entities.rend(); ++entity)
	{
		if (*entity != nullptr)
			context.world->DestroyPhysicalEntity(*entity);
	}
	context.entities.clear();
	context.world->PurgeDeletedEntities();
}

static void PhysicsRegressionDestroyEntity(
	PhysicsRegressionContext& context,
	IPhysicalEntity* entity)
{
	const auto found = std::find(context.entities.begin(), context.entities.end(), entity);
	if (found != context.entities.end())
		context.entities.erase(found);
	if (entity != nullptr)
		context.world->DestroyPhysicalEntity(entity);
	context.world->PurgeDeletedEntities();
}

static IPhysicalEntity* PhysicsRegressionCreateBox(
	PhysicsRegressionContext& context,
	pe_type entityType,
	const Vec3& position,
	const Vec3& halfExtents,
	float mass)
{
	IGeomManager* geometryManager = context.world->GetGeomManager();
	if (geometryManager == nullptr)
		return nullptr;

	primitives::box box;
	box.Basis.SetIdentity();
	box.bOriented = 0;
	box.center = Vec3(ZERO);
	box.size = halfExtents;
	IGeometry* geometry = geometryManager->CreatePrimitive(primitives::box::type, &box);
	if (geometry == nullptr)
		return nullptr;

	phys_geometry* registeredGeometry = geometryManager->RegisterGeometry(geometry, 0);
	geometry->Release();
	if (registeredGeometry == nullptr)
		return nullptr;

	pe_params_pos positionParams;
	positionParams.pos = position;
	positionParams.q.SetIdentity();
	positionParams.scale = 1.0f;
	IPhysicalEntity* entity = context.world->CreatePhysicalEntity(
		entityType,
		&positionParams);
	if (entity == nullptr)
	{
		geometryManager->UnregisterGeometry(registeredGeometry);
		return nullptr;
	}

	pe_geomparams geometryParams;
	geometryParams.mass = mass;
	const int geometryId = entity->AddGeometry(registeredGeometry, &geometryParams);
	geometryManager->UnregisterGeometry(registeredGeometry);
	if (geometryId < 0)
	{
		context.world->DestroyPhysicalEntity(entity);
		context.world->PurgeDeletedEntities();
		return nullptr;
	}

	context.entities.push_back(entity);
	return entity;
}

static IPhysicalEntity* PhysicsRegressionCreateBareEntity(
	PhysicsRegressionContext& context,
	pe_type entityType,
	const Vec3& position)
{
	pe_params_pos positionParams;
	positionParams.pos = position;
	positionParams.q.SetIdentity();
	positionParams.scale = 1.0f;
	IPhysicalEntity* entity = context.world->CreatePhysicalEntity(
		entityType,
		&positionParams);
	if (entity != nullptr)
		context.entities.push_back(entity);
	return entity;
}

static IPhysicalEntity* PhysicsRegressionCreateLiving(
	PhysicsRegressionContext& context,
	const Vec3& position)
{
	pe_params_pos positionParams;
	positionParams.pos = position;
	positionParams.q.SetIdentity();
	positionParams.scale = 1.0f;
	IPhysicalEntity* entity = context.world->CreatePhysicalEntity(
		PE_LIVING,
		&positionParams);
	if (entity == nullptr)
		return nullptr;

	pe_player_dimensions dimensions;
	dimensions.heightPivot = 0.0f;
	dimensions.heightEye = 1.6f;
	dimensions.sizeCollider = Vec3(0.4f, 0.4f, 0.5f);
	dimensions.heightCollider = 0.5f;
	dimensions.headRadius = 0.0f;
	dimensions.heightHead = 1.6f;
	dimensions.bUseCapsule = 0;
	if (entity->SetParams(&dimensions) == 0)
	{
		context.world->DestroyPhysicalEntity(entity);
		context.world->PurgeDeletedEntities();
		return nullptr;
	}

	pe_player_dynamics dynamics;
	dynamics.kInertia = 0.0f;
	dynamics.kInertiaAccel = 0.0f;
	dynamics.kAirControl = 1.0f;
	dynamics.kAirResistance = 0.0f;
	dynamics.gravity = Vec3(0.0f, 0.0f, -9.81f);
	dynamics.mass = 80.0f;
	dynamics.collTypes = ent_static | ent_sleeping_rigid | ent_rigid | ent_living;
	dynamics.bActive = 1;
	if (entity->SetParams(&dynamics) == 0)
	{
		context.world->DestroyPhysicalEntity(entity);
		context.world->PurgeDeletedEntities();
		return nullptr;
	}

	context.entities.push_back(entity);
	return entity;
}

static void PhysicsRegressionEnableCollisionEvents(IPhysicalEntity* entity)
{
	pe_params_flags flags;
	flags.flagsOR = pef_monitor_collisions;
	entity->SetParams(&flags);
}

static void PhysicsRegressionConfigureRigidBody(
	IPhysicalEntity* entity,
	const Vec3& gravity,
	float damping,
	float minimumEnergy)
{
	pe_simulation_params simulationParams;
	simulationParams.maxTimeStep = 1.0f / 60.0f;
	simulationParams.gravity = gravity;
	simulationParams.gravityFreefall = gravity;
	simulationParams.damping = damping;
	simulationParams.dampingFreefall = damping;
	simulationParams.minEnergy = minimumEnergy;
	entity->SetParams(&simulationParams);
}

static void PhysicsRegressionStep(
	PhysicsRegressionContext& context,
	size_t stepCount,
	float timeStep = 1.0f / 60.0f)
{
	for (size_t step = 0; step < stepCount; ++step)
		context.world->TimeStep(timeStep);
}

static PhysicsRegressionSnapshot PhysicsRegressionReadSnapshot(
	PhysicsRegressionContext& context,
	IPhysicalEntity* entity)
{
	PhysicsRegressionSnapshot snapshot;
	snapshot.entityId = context.world->GetPhysicalEntityId(entity);

	pe_status_pos positionStatus;
	snapshot.hasPosition = entity->GetStatus(&positionStatus) != 0;
	if (snapshot.hasPosition)
	{
		snapshot.position = positionStatus.pos;
		snapshot.orientation = positionStatus.q;
		snapshot.simulationClass = positionStatus.iSimClass;
	}

	pe_status_dynamics dynamicsStatus;
	snapshot.hasDynamics = entity->GetStatus(&dynamicsStatus) != 0;
	if (snapshot.hasDynamics)
	{
		snapshot.velocity = dynamicsStatus.v;
		snapshot.angularVelocity = dynamicsStatus.w;
		snapshot.energy = dynamicsStatus.energy;
		snapshot.contactCount = dynamicsStatus.nContacts;
	}

	pe_status_awake awakeStatus;
	snapshot.awake = entity->GetStatus(&awakeStatus);
	return snapshot;
}

static void PhysicsRegressionWriteSnapshot(
	PhysicsRegressionContext& context,
	const char* label,
	size_t simulationStep,
	const PhysicsRegressionSnapshot& snapshot)
{
	if (context.report == nullptr || !*context.report)
		return;

	++context.snapshotSequence;
	*context.report
		<< "snapshot\t"
		<< context.caseName << '\t'
		<< label << '\t'
		<< context.snapshotSequence << '\t'
		<< simulationStep << '\t'
		<< snapshot.entityId << '\t'
		<< snapshot.simulationClass << '\t'
		<< snapshot.awake << '\t'
		<< snapshot.position.x << '\t'
		<< snapshot.position.y << '\t'
		<< snapshot.position.z << '\t'
		<< snapshot.orientation.v.x << '\t'
		<< snapshot.orientation.v.y << '\t'
		<< snapshot.orientation.v.z << '\t'
		<< snapshot.orientation.w << '\t'
		<< snapshot.velocity.x << '\t'
		<< snapshot.velocity.y << '\t'
		<< snapshot.velocity.z << '\t'
		<< snapshot.angularVelocity.x << '\t'
		<< snapshot.angularVelocity.y << '\t'
		<< snapshot.angularVelocity.z << '\t'
		<< snapshot.energy << '\t'
		<< snapshot.contactCount << "\n";
	context.report->flush();
}

static bool PhysicsRegressionIsFinite(const Vec3& value)
{
	return std::isfinite(value.x) &&
		std::isfinite(value.y) &&
		std::isfinite(value.z);
}

static bool PhysicsRegressionIsFinite(const quaternionf& value)
{
	return PhysicsRegressionIsFinite(value.v) && std::isfinite(value.w);
}

static bool PhysicsRegressionNear(float left, float right, float tolerance)
{
	return std::fabs(left - right) <= tolerance;
}

static void PhysicsRegressionCheckSnapshot(
	CryTestRunner& runner,
	const PhysicsRegressionSnapshot& snapshot)
{
	CRY_TEST_CHECK(runner, snapshot.hasPosition);
	CRY_TEST_CHECK(runner, PhysicsRegressionIsFinite(snapshot.position));
	CRY_TEST_CHECK(runner, PhysicsRegressionIsFinite(snapshot.orientation));
}

static void PhysicsRegressionCheckDynamicSnapshot(
	CryTestRunner& runner,
	const PhysicsRegressionSnapshot& snapshot)
{
	PhysicsRegressionCheckSnapshot(runner, snapshot);
	CRY_TEST_CHECK(runner, snapshot.hasDynamics);
	CRY_TEST_CHECK(runner, PhysicsRegressionIsFinite(snapshot.velocity));
	CRY_TEST_CHECK(runner, PhysicsRegressionIsFinite(snapshot.angularVelocity));
	CRY_TEST_CHECK(runner, std::isfinite(snapshot.energy));
}

static void PhysicsRegressionTestWorldInterface(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	CRY_TEST_CHECK(runner, context.world != nullptr);
	CRY_TEST_CHECK(runner, context.world->GetGeomManager() != nullptr);
	CRY_TEST_CHECK(runner, context.world->GetPhysVars() != nullptr);
	CRY_TEST_CHECK(runner, std::isfinite(context.world->GetPhysicsTime()));
}

static void PhysicsRegressionTestDebugEntityEnumeration(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	constexpr size_t entityCount = 32;
	for (size_t index = 0; index < entityCount; ++index)
	{
		IPhysicalEntity* entity = PhysicsRegressionCreateBox(
			context,
			PE_STATIC,
			Vec3(static_cast<float>(index) * 2.0f, 0.0f, 0.0f),
			Vec3(0.5f),
			0.0f);
		CRY_TEST_CHECK(runner, entity != nullptr);
		if (entity != nullptr)
		{
			const int entityId = context.world->GetPhysicalEntityId(entity);
			CRY_TEST_CHECK(runner, entityId >= 0);
			CRY_TEST_CHECK(runner, context.world->GetPhysicalEntityById(entityId) == entity);
		}
	}

	PhysicsVars* variables = context.world->GetPhysVars();
	CRY_TEST_CHECK(runner, variables != nullptr);
	if (variables == nullptr)
		return;

	const int previousDrawHelpers = variables->iDrawHelpers;
	constexpr int staticSimulationClass = 0;
	variables->iDrawHelpers = pe_helper_bbox | (0x100 << staticSimulationClass);

	PhysicsRegressionRenderer renderer;
	context.world->DrawPhysicsHelperInformation(&renderer, 1);

	constexpr size_t boundingBoxEdges = 12;
	CRY_TEST_CHECK(runner, renderer.lineCount >= entityCount * boundingBoxEdges);

	variables->iDrawHelpers = 0;
	context.world->DrawPhysicsHelperInformation(nullptr, 1);
	variables->iDrawHelpers = previousDrawHelpers;
}

static void PhysicsRegressionTestDebugEntityIdStorage(
	CryTestRunner& runner,
	PhysicsRegressionContext&)
{
	const std::filesystem::path sourcePath =
		std::filesystem::current_path() / "CryEngine" / "CryPhysics" / "physicalworld.cpp";
	std::ifstream sourceFile(sourcePath, std::ios::binary);
	CRY_TEST_CHECK(runner, sourceFile.is_open());
	if (!sourceFile)
		return;

	const std::string sourceText{
		std::istreambuf_iterator<char>(sourceFile),
		std::istreambuf_iterator<char>()};
	std::string compactSource;
	compactSource.reserve(sourceText.size());
	for (const char character : sourceText)
	{
		if (character != ' ' && character != '\t' && character != '\r' && character != '\n')
			compactSource.push_back(character);
	}

	const bool storesEntityIdInPointer =
		compactSource.find("pEntList[n++]=(CPhysicalEntity*)pent->m_id;") != std::string::npos;
	const bool readsEntityIdThroughPointerStorage =
		compactSource.find("intid=*(int*)(pEntList+i);") != std::string::npos;
	CRY_TEST_CHECK(runner, !storesEntityIdInPointer);
	CRY_TEST_CHECK(runner, !readsEntityIdThroughPointerStorage);
}

static void PhysicsRegressionTestDebugEntityFiltering(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	constexpr size_t staticEntityCount = 3;
	constexpr size_t rigidEntityCount = 2;
	constexpr size_t boundingBoxEdges = 12;
	constexpr int staticSimulationClass = 0;
	constexpr int rigidSimulationClass = 2;

	for (size_t index = 0; index < staticEntityCount; ++index)
	{
		CRY_TEST_CHECK(runner, PhysicsRegressionCreateBox(
			context,
			PE_STATIC,
			Vec3(static_cast<float>(index) * 3.0f, 0.0f, 0.0f),
			Vec3(0.5f),
			0.0f) != nullptr);
	}
	for (size_t index = 0; index < rigidEntityCount; ++index)
	{
		CRY_TEST_CHECK(runner, PhysicsRegressionCreateBox(
			context,
			PE_RIGID,
			Vec3(static_cast<float>(index) * 3.0f, 10.0f, 0.0f),
			Vec3(0.5f),
			1.0f) != nullptr);
	}

	PhysicsVars* variables = context.world->GetPhysVars();
	CRY_TEST_CHECK(runner, variables != nullptr);
	if (variables == nullptr)
		return;
	const int previousDrawHelpers = variables->iDrawHelpers;

	variables->iDrawHelpers = pe_helper_bbox | (0x100 << staticSimulationClass);
	PhysicsRegressionRenderer firstStaticRenderer;
	context.world->DrawPhysicsHelperInformation(&firstStaticRenderer, 1);
	CRY_TEST_CHECK(
		runner,
		firstStaticRenderer.lineCountBySimulationClass[staticSimulationClass] ==
			staticEntityCount * boundingBoxEdges);
	CRY_TEST_CHECK(runner, firstStaticRenderer.lineCountBySimulationClass[rigidSimulationClass] == 0);

	PhysicsRegressionRenderer secondStaticRenderer;
	context.world->DrawPhysicsHelperInformation(&secondStaticRenderer, 1);
	CRY_TEST_CHECK(runner, secondStaticRenderer.lineCount == firstStaticRenderer.lineCount);
	CRY_TEST_CHECK(runner, secondStaticRenderer.lineChecksum == firstStaticRenderer.lineChecksum);

	variables->iDrawHelpers = pe_helper_bbox | (0x100 << rigidSimulationClass);
	PhysicsRegressionRenderer rigidRenderer;
	context.world->DrawPhysicsHelperInformation(&rigidRenderer, 1);
	CRY_TEST_CHECK(runner, rigidRenderer.lineCountBySimulationClass[staticSimulationClass] == 0);
	CRY_TEST_CHECK(
		runner,
		rigidRenderer.lineCountBySimulationClass[rigidSimulationClass] ==
			rigidEntityCount * boundingBoxEdges);

	variables->iDrawHelpers = pe_helper_bbox |
		(0x100 << staticSimulationClass) |
		(0x100 << rigidSimulationClass);
	PhysicsRegressionRenderer combinedRenderer;
	context.world->DrawPhysicsHelperInformation(&combinedRenderer, 1);
	CRY_TEST_CHECK(
		runner,
		combinedRenderer.lineCount ==
			(staticEntityCount + rigidEntityCount) * boundingBoxEdges);

	variables->iDrawHelpers = 0;
	context.world->DrawPhysicsHelperInformation(nullptr, 1);
	variables->iDrawHelpers = previousDrawHelpers;
}

static void PhysicsRegressionTestDebugEntityStaleId(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	constexpr size_t boundingBoxEdges = 12;
	constexpr int staticSimulationClass = 0;
	constexpr int rigidSimulationClass = 2;
	IPhysicalEntity* destroyTarget = PhysicsRegressionCreateBox(
		context,
		PE_RIGID,
		Vec3(10.0f, 0.0f, 0.0f),
		Vec3(0.5f),
		1.0f);
	IPhysicalEntity* drawTrigger = PhysicsRegressionCreateBox(
		context,
		PE_STATIC,
		Vec3(ZERO),
		Vec3(0.5f),
		0.0f);
	CRY_TEST_CHECK(runner, destroyTarget != nullptr);
	CRY_TEST_CHECK(runner, drawTrigger != nullptr);
	if (destroyTarget == nullptr || drawTrigger == nullptr)
		return;
	const int destroyTargetId = context.world->GetPhysicalEntityId(destroyTarget);

	PhysicsVars* variables = context.world->GetPhysVars();
	CRY_TEST_CHECK(runner, variables != nullptr);
	if (variables == nullptr)
		return;
	const int previousDrawHelpers = variables->iDrawHelpers;
	variables->iDrawHelpers = pe_helper_bbox |
		(0x100 << staticSimulationClass) |
		(0x100 << rigidSimulationClass);

	PhysicsRegressionRenderer renderer;
	renderer.world = context.world;
	renderer.destroyTarget = destroyTarget;
	context.world->DrawPhysicsHelperInformation(&renderer, 1);

	const auto trackedTarget = std::find(
		context.entities.begin(),
		context.entities.end(),
		destroyTarget);
	if (trackedTarget != context.entities.end())
		context.entities.erase(trackedTarget);
	CRY_TEST_CHECK(runner, renderer.destructionAttempted);
	CRY_TEST_CHECK(runner, renderer.destroyedTarget);
	CRY_TEST_CHECK(runner, context.world->GetPhysicalEntityById(destroyTargetId) == nullptr);
	CRY_TEST_CHECK(
		runner,
		renderer.lineCountBySimulationClass[staticSimulationClass] == boundingBoxEdges);
	CRY_TEST_CHECK(runner, renderer.lineCountBySimulationClass[rigidSimulationClass] == 0);

	variables->iDrawHelpers = 0;
	context.world->DrawPhysicsHelperInformation(nullptr, 1);
	variables->iDrawHelpers = previousDrawHelpers;
}

static void PhysicsRegressionTestDebugEntityBufferReuse(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	constexpr size_t largeEntityCount = 4097;
	constexpr size_t smallEntityCount = 3;
	constexpr size_t boundingBoxEdges = 12;
	constexpr int staticSimulationClass = 0;
	for (size_t index = 0; index < largeEntityCount; ++index)
	{
		CRY_TEST_CHECK(runner, PhysicsRegressionCreateBareEntity(
			context,
			PE_STATIC,
			Vec3(static_cast<float>(index), 0.0f, 0.0f)) != nullptr);
	}

	PhysicsVars* variables = context.world->GetPhysVars();
	CRY_TEST_CHECK(runner, variables != nullptr);
	if (variables == nullptr)
		return;
	const int previousDrawHelpers = variables->iDrawHelpers;
	variables->iDrawHelpers = pe_helper_bbox | (0x100 << staticSimulationClass);

	PhysicsRegressionRenderer largeRenderer;
	context.world->DrawPhysicsHelperInformation(&largeRenderer, 1);
	CRY_TEST_CHECK(runner, largeRenderer.lineCount == largeEntityCount * boundingBoxEdges);

	PhysicsRegressionDestroyEntities(context);
	for (size_t index = 0; index < smallEntityCount; ++index)
	{
		CRY_TEST_CHECK(runner, PhysicsRegressionCreateBareEntity(
			context,
			PE_STATIC,
			Vec3(static_cast<float>(index), 0.0f, 0.0f)) != nullptr);
	}
	PhysicsRegressionRenderer smallRenderer;
	context.world->DrawPhysicsHelperInformation(&smallRenderer, 1);
	CRY_TEST_CHECK(runner, smallRenderer.lineCount == smallEntityCount * boundingBoxEdges);

	variables->iDrawHelpers = 0;
	context.world->DrawPhysicsHelperInformation(nullptr, 1);
	variables->iDrawHelpers = previousDrawHelpers;
}

static void PhysicsRegressionTestDebugEntityConcurrentCallers(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	constexpr size_t entityCount = 128;
	constexpr size_t boundingBoxEdges = 12;
	constexpr int staticSimulationClass = 0;
	for (size_t index = 0; index < entityCount; ++index)
	{
		CRY_TEST_CHECK(runner, PhysicsRegressionCreateBareEntity(
			context,
			PE_STATIC,
			Vec3(static_cast<float>(index), 0.0f, 0.0f)) != nullptr);
	}

	PhysicsVars* variables = context.world->GetPhysVars();
	CRY_TEST_CHECK(runner, variables != nullptr);
	if (variables == nullptr)
		return;
	const int previousDrawHelpers = variables->iDrawHelpers;
	variables->iDrawHelpers = pe_helper_bbox | (0x100 << staticSimulationClass);

	PhysicsRegressionRenderer callerZeroRenderer;
	PhysicsRegressionRenderer callerOneRenderer;
	std::atomic<int> readyCallers = 0;
	std::atomic<bool> startDrawing = false;
	auto drawForCaller = [&](PhysicsRegressionRenderer& renderer, int callerIndex)
	{
		readyCallers.fetch_add(1, std::memory_order_release);
		while (!startDrawing.load(std::memory_order_acquire))
			std::this_thread::yield();
		context.world->DrawPhysicsHelperInformation(&renderer, callerIndex);
	};

	std::thread callerZero(drawForCaller, std::ref(callerZeroRenderer), 0);
	std::thread callerOne(drawForCaller, std::ref(callerOneRenderer), 1);
	while (readyCallers.load(std::memory_order_acquire) != 2)
		std::this_thread::yield();
	startDrawing.store(true, std::memory_order_release);
	callerZero.join();
	callerOne.join();

	const size_t expectedLineCount = entityCount * boundingBoxEdges;
	CRY_TEST_CHECK(runner, callerZeroRenderer.lineCount == expectedLineCount);
	CRY_TEST_CHECK(runner, callerOneRenderer.lineCount == expectedLineCount);
	CRY_TEST_CHECK(runner, callerZeroRenderer.lineChecksum == callerOneRenderer.lineChecksum);

	variables->iDrawHelpers = 0;
	context.world->DrawPhysicsHelperInformation(nullptr, 1);
	variables->iDrawHelpers = previousDrawHelpers;
}

static void PhysicsRegressionTestConvexHullBufferGrowth(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	constexpr int pointCount = 1536;
	constexpr float radius = 64.0f;
	constexpr double goldenAngle = 2.39996322972865332;
	std::vector<Vec3> points;
	points.reserve(pointCount);
	for (int index = 0; index < pointCount; ++index)
	{
		const double y = 1.0 - 2.0 * (index + 0.5) / pointCount;
		const double ringRadius = std::sqrt(1.0 - y * y);
		const double angle = goldenAngle * index;
		points.emplace_back(
			radius * static_cast<float>(ringRadius * std::cos(angle)),
			radius * static_cast<float>(y),
			radius * static_cast<float>(ringRadius * std::sin(angle)));
	}

	IPhysUtils* utils = context.world->GetPhysUtils();
	CRY_TEST_CHECK(runner, utils != nullptr);
	if (!utils)
		return;

	std::vector<index_t> firstHull;
	for (int pass = 0; pass < 2; ++pass)
	{
		index_t* triangles = nullptr;
		const int triangleCount = utils->qhull(points.data(), pointCount, triangles);
		CRY_TEST_CHECK(runner, triangles != nullptr);
		// More than 1,280 live faces forces growth beyond the initial 1,024 slots twice.
		CRY_TEST_CHECK(runner, triangleCount > 1280);
		CRY_TEST_CHECK(runner, triangleCount <= 2 * pointCount - 4);
		if (!triangles || triangleCount <= 1280 || triangleCount > 2 * pointCount - 4)
		{
			utils->DeletePointer(triangles);
			return;
		}

		std::vector<unsigned char> usedPoints(pointCount, 0);
		std::vector<std::pair<int, int>> edges;
		float maximumOutsideDistance = 0.0f;
		for (int triangle = 0; triangle < triangleCount; ++triangle)
		{
			const int a = triangles[triangle * 3];
			const int b = triangles[triangle * 3 + 1];
			const int c = triangles[triangle * 3 + 2];
			const bool validIndices = a >= 0 && a < pointCount && b >= 0 && b < pointCount &&
				c >= 0 && c < pointCount && a != b && b != c && c != a;
			CRY_TEST_CHECK(runner, validIndices);
			if (!validIndices)
				continue;
			usedPoints[a] = usedPoints[b] = usedPoints[c] = 1;
			for (const auto edge : { std::pair{a, b}, std::pair{b, c}, std::pair{c, a} })
				edges.emplace_back(std::min(edge.first, edge.second), std::max(edge.first, edge.second));

			const Vec3 normal = (points[b] - points[a]) ^ (points[c] - points[a]);
			const float normalLength = normal.len();
			CRY_TEST_CHECK(runner, normalLength > 0.0001f);
			CRY_TEST_CHECK(runner, normal * points[a] > 0.0f);
			if (normalLength > 0.0001f)
			{
				for (const Vec3& point : points)
					maximumOutsideDistance = std::max(maximumOutsideDistance,
						(normal * (point - points[a])) / normalLength);
			}
		}
		// e_cansee uses a 0.002 angular tolerance; bound distance by the input diameter.
		CRY_TEST_CHECK(runner, maximumOutsideDistance <= 2.0f * radius * 0.002f);
		// The legacy angular tolerance can omit nearly coplanar input points.
		const auto usedPointCount = std::count(usedPoints.begin(), usedPoints.end(), 1);
		CRY_TEST_CHECK(runner, triangleCount == 2 * usedPointCount - 4);
		std::sort(edges.begin(), edges.end());
		CRY_TEST_CHECK(runner, edges.size() % 2 == 0);
		for (size_t index = 0; index + 1 < edges.size(); index += 2)
		{
			CRY_TEST_CHECK(runner, edges[index] == edges[index + 1]);
			CRY_TEST_CHECK(runner, index + 2 == edges.size() || edges[index] != edges[index + 2]);
		}
		const std::vector<index_t> hull(triangles, triangles + triangleCount * 3);
		if (pass == 0)
			firstHull = hull;
		else
			CRY_TEST_CHECK(runner, hull == firstHull);
		utils->DeletePointer(triangles);
	}
}

static void PhysicsRegressionTestStaticBodyInvariance(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	IPhysicalEntity* body = PhysicsRegressionCreateBox(
		context,
		PE_STATIC,
		Vec3(2.0f, -3.0f, 4.0f),
		Vec3(0.5f, 1.0f, 1.5f),
		0.0f);
	CRY_TEST_CHECK(runner, body != nullptr);
	if (body == nullptr)
		return;

	const PhysicsRegressionSnapshot before = PhysicsRegressionReadSnapshot(context, body);
	PhysicsRegressionWriteSnapshot(context, "before", 0, before);
	PhysicsRegressionStep(context, 120);
	const PhysicsRegressionSnapshot after = PhysicsRegressionReadSnapshot(context, body);
	PhysicsRegressionWriteSnapshot(context, "after", 120, after);
	PhysicsRegressionCheckSnapshot(runner, before);
	PhysicsRegressionCheckSnapshot(runner, after);
	CRY_TEST_CHECK(runner, PhysicsRegressionNear(before.position.x, after.position.x, 0.0001f));
	CRY_TEST_CHECK(runner, PhysicsRegressionNear(before.position.y, after.position.y, 0.0001f));
	CRY_TEST_CHECK(runner, PhysicsRegressionNear(before.position.z, after.position.z, 0.0001f));
}

static void PhysicsRegressionTestRigidFreeFall(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	IPhysicalEntity* body = PhysicsRegressionCreateBox(
		context,
		PE_RIGID,
		Vec3(0.0f, 0.0f, 20.0f),
		Vec3(0.5f),
		2.0f);
	CRY_TEST_CHECK(runner, body != nullptr);
	if (body == nullptr)
		return;

	PhysicsRegressionConfigureRigidBody(body, Vec3(0.0f, 0.0f, -9.81f), 0.0f, 0.0f);
	const PhysicsRegressionSnapshot before = PhysicsRegressionReadSnapshot(context, body);
	PhysicsRegressionWriteSnapshot(context, "before", 0, before);
	PhysicsRegressionStep(context, 60);
	const PhysicsRegressionSnapshot after = PhysicsRegressionReadSnapshot(context, body);
	PhysicsRegressionWriteSnapshot(context, "after", 60, after);
	PhysicsRegressionCheckDynamicSnapshot(runner, after);
	CRY_TEST_CHECK(runner, PhysicsRegressionNear(after.position.x, 0.0f, 0.001f));
	CRY_TEST_CHECK(runner, PhysicsRegressionNear(after.position.y, 0.0f, 0.001f));
	CRY_TEST_CHECK(runner, after.position.z < before.position.z - 3.0f);
	CRY_TEST_CHECK(runner, after.position.z > 12.0f);
	CRY_TEST_CHECK(runner, after.velocity.z < -5.0f);
}

static void PhysicsRegressionTestImpulseResponse(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	IPhysicalEntity* body = PhysicsRegressionCreateBox(
		context,
		PE_RIGID,
		Vec3(ZERO),
		Vec3(0.5f),
		2.0f);
	CRY_TEST_CHECK(runner, body != nullptr);
	if (body == nullptr)
		return;

	PhysicsRegressionConfigureRigidBody(body, Vec3(ZERO), 0.0f, 0.0f);
	pe_action_impulse impulse;
	impulse.impulse = Vec3(4.0f, 0.0f, 0.0f);
	CRY_TEST_CHECK(runner, body->Action(&impulse) != 0);
	PhysicsRegressionStep(context, 15);
	const PhysicsRegressionSnapshot after = PhysicsRegressionReadSnapshot(context, body);
	PhysicsRegressionWriteSnapshot(context, "after-impulse", 15, after);
	PhysicsRegressionCheckDynamicSnapshot(runner, after);
	CRY_TEST_CHECK(runner, after.position.x > 0.25f);
	CRY_TEST_CHECK(runner, after.velocity.x > 1.0f);
	CRY_TEST_CHECK(runner, PhysicsRegressionNear(after.position.y, 0.0f, 0.001f));
	CRY_TEST_CHECK(runner, PhysicsRegressionNear(after.position.z, 0.0f, 0.001f));
}

static void PhysicsRegressionTestFloorContactAndSettle(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	IPhysicalEntity* floor = PhysicsRegressionCreateBox(
		context,
		PE_STATIC,
		Vec3(0.0f, 0.0f, -0.5f),
		Vec3(10.0f, 10.0f, 0.5f),
		0.0f);
	IPhysicalEntity* body = PhysicsRegressionCreateBox(
		context,
		PE_RIGID,
		Vec3(0.0f, 0.0f, 4.0f),
		Vec3(0.5f),
		1.0f);
	CRY_TEST_CHECK(runner, floor != nullptr);
	CRY_TEST_CHECK(runner, body != nullptr);
	if (floor == nullptr || body == nullptr)
		return;

	const PhysicsRegressionSnapshot before = PhysicsRegressionReadSnapshot(context, body);
	PhysicsRegressionWriteSnapshot(context, "before", 0, before);
	PhysicsRegressionStep(context, 360);
	const PhysicsRegressionSnapshot after = PhysicsRegressionReadSnapshot(context, body);
	PhysicsRegressionWriteSnapshot(context, "settled", 360, after);
	PhysicsRegressionCheckDynamicSnapshot(runner, after);
	CRY_TEST_CHECK(runner, after.position.z > 0.35f);
	CRY_TEST_CHECK(runner, after.position.z < 0.75f);
	CRY_TEST_CHECK(runner, std::fabs(after.position.x) < 0.1f);
	CRY_TEST_CHECK(runner, std::fabs(after.position.y) < 0.1f);
	CRY_TEST_CHECK(runner, std::fabs(after.velocity.z) < 0.25f);
}

static void PhysicsRegressionTestStackStability(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	IPhysicalEntity* floor = PhysicsRegressionCreateBox(
		context,
		PE_STATIC,
		Vec3(0.0f, 0.0f, -0.5f),
		Vec3(10.0f, 10.0f, 0.5f),
		0.0f);
	std::array<IPhysicalEntity*, 3> bodies = {
		PhysicsRegressionCreateBox(context, PE_RIGID, Vec3(0.0f, 0.0f, 0.55f), Vec3(0.5f), 1.0f),
		PhysicsRegressionCreateBox(context, PE_RIGID, Vec3(0.0f, 0.0f, 1.60f), Vec3(0.5f), 1.0f),
		PhysicsRegressionCreateBox(context, PE_RIGID, Vec3(0.0f, 0.0f, 2.65f), Vec3(0.5f), 1.0f)
	};
	CRY_TEST_CHECK(runner, floor != nullptr);
	for (IPhysicalEntity* body : bodies)
		CRY_TEST_CHECK(runner, body != nullptr);
	if (floor == nullptr ||
		std::find(bodies.begin(), bodies.end(), nullptr) != bodies.end())
	{
		return;
	}

	PhysicsRegressionStep(context, 600);
	std::array<PhysicsRegressionSnapshot, 3> snapshots;
	for (size_t index = 0; index < bodies.size(); ++index)
	{
		snapshots[index] = PhysicsRegressionReadSnapshot(context, bodies[index]);
		PhysicsRegressionWriteSnapshot(context, "settled-stack", 600, snapshots[index]);
		PhysicsRegressionCheckDynamicSnapshot(runner, snapshots[index]);
		CRY_TEST_CHECK(runner, std::fabs(snapshots[index].position.x) < 0.5f);
		CRY_TEST_CHECK(runner, std::fabs(snapshots[index].position.y) < 0.5f);
		CRY_TEST_CHECK(runner, snapshots[index].velocity.len() < 0.5f);
	}
	CRY_TEST_CHECK(runner, snapshots[0].position.z > 0.30f);
	CRY_TEST_CHECK(runner, snapshots[0].position.z < 0.80f);
	CRY_TEST_CHECK(runner, snapshots[1].position.z > snapshots[0].position.z + 0.70f);
	CRY_TEST_CHECK(runner, snapshots[2].position.z > snapshots[1].position.z + 0.70f);
	CRY_TEST_CHECK(runner, snapshots[2].position.z < 3.0f);
}

static void PhysicsRegressionTestSpatialQueriesAndEntityLifecycle(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	IPhysicalEntity* body = PhysicsRegressionCreateBox(
		context,
		PE_STATIC,
		Vec3(5.0f, 6.0f, 2.0f),
		Vec3(1.0f),
		0.0f);
	CRY_TEST_CHECK(runner, body != nullptr);
	if (body == nullptr)
		return;

	const int entityId = context.world->GetPhysicalEntityId(body);
	CRY_TEST_CHECK(runner, entityId >= 0);
	CRY_TEST_CHECK(runner, context.world->GetPhysicalEntityById(entityId) == body);

	IPhysicalEntity** entitiesInBox = nullptr;
	const int entityCount = context.world->GetEntitiesInBox(
		Vec3(3.0f, 4.0f, 0.0f),
		Vec3(7.0f, 8.0f, 4.0f),
		entitiesInBox,
		ent_static);
	CRY_TEST_CHECK(runner, entityCount > 0);
	CRY_TEST_CHECK(
		runner,
		entitiesInBox != nullptr &&
			std::find(entitiesInBox, entitiesInBox + entityCount, body) != entitiesInBox + entityCount);

	ray_hit hit{};
	const int hitCount = context.world->RayWorldIntersection(
		Vec3(5.0f, 6.0f, 8.0f),
		Vec3(0.0f, 0.0f, -12.0f),
		ent_static,
		rwi_stop_at_pierceable,
		&hit,
		1);
	CRY_TEST_CHECK(runner, hitCount == 1);
	CRY_TEST_CHECK(runner, hit.pCollider == body);
	CRY_TEST_CHECK(runner, hit.dist > 4.5f && hit.dist < 5.5f);

	PhysicsRegressionDestroyEntity(context, body);
	CRY_TEST_CHECK(runner, context.world->GetPhysicalEntityById(entityId) == nullptr);
}

static PhysicsRegressionSnapshot PhysicsRegressionRunRepeatabilitySample(
	CryTestRunner& runner,
	PhysicsRegressionContext& context,
	const char* label)
{
	IPhysicalEntity* body = PhysicsRegressionCreateBox(
		context,
		PE_RIGID,
		Vec3(1.0f, -2.0f, 20.0f),
		Vec3(0.5f),
		2.0f);
	CRY_TEST_CHECK(runner, body != nullptr);
	if (body == nullptr)
		return PhysicsRegressionSnapshot{};

	PhysicsRegressionConfigureRigidBody(body, Vec3(0.0f, 0.0f, -9.81f), 0.0f, 0.0f);
	PhysicsRegressionStep(context, 90);
	const PhysicsRegressionSnapshot snapshot = PhysicsRegressionReadSnapshot(context, body);
	PhysicsRegressionWriteSnapshot(context, label, 90, snapshot);
	PhysicsRegressionDestroyEntity(context, body);
	context.world->SetPhysicsTime(0.0f);
	context.world->SetiPhysicsTime(0);
	return snapshot;
}

static void PhysicsRegressionTestDeterministicReplay(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	const PhysicsRegressionSnapshot first = PhysicsRegressionRunRepeatabilitySample(
		runner,
		context,
		"run-a");
	const PhysicsRegressionSnapshot second = PhysicsRegressionRunRepeatabilitySample(
		runner,
		context,
		"run-b");
	PhysicsRegressionCheckDynamicSnapshot(runner, first);
	PhysicsRegressionCheckDynamicSnapshot(runner, second);
	CRY_TEST_CHECK(runner, PhysicsRegressionNear(first.position.x, second.position.x, 0.0001f));
	CRY_TEST_CHECK(runner, PhysicsRegressionNear(first.position.y, second.position.y, 0.0001f));
	CRY_TEST_CHECK(runner, PhysicsRegressionNear(first.position.z, second.position.z, 0.0001f));
	CRY_TEST_CHECK(runner, PhysicsRegressionNear(first.velocity.x, second.velocity.x, 0.0001f));
	CRY_TEST_CHECK(runner, PhysicsRegressionNear(first.velocity.y, second.velocity.y, 0.0001f));
	CRY_TEST_CHECK(runner, PhysicsRegressionNear(first.velocity.z, second.velocity.z, 0.0001f));
}

static void PhysicsRegressionTestImpulseCollisionPair(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	IPhysicalEntity* left = PhysicsRegressionCreateBox(
		context,
		PE_RIGID,
		Vec3(-2.0f, 0.0f, 0.0f),
		Vec3(0.5f),
		1.0f);
	IPhysicalEntity* right = PhysicsRegressionCreateBox(
		context,
		PE_RIGID,
		Vec3(2.0f, 0.0f, 0.0f),
		Vec3(0.5f),
		1.0f);
	CRY_TEST_CHECK(runner, left != nullptr);
	CRY_TEST_CHECK(runner, right != nullptr);
	if (left == nullptr || right == nullptr)
		return;

	PhysicsRegressionConfigureRigidBody(left, Vec3(ZERO), 0.0f, 0.0f);
	PhysicsRegressionConfigureRigidBody(right, Vec3(ZERO), 0.0f, 0.0f);
	pe_action_set_velocity leftVelocity;
	leftVelocity.v = Vec3(2.0f, 0.0f, 0.0f);
	leftVelocity.w = Vec3(ZERO);
	pe_action_set_velocity rightVelocity;
	rightVelocity.v = Vec3(-2.0f, 0.0f, 0.0f);
	rightVelocity.w = Vec3(ZERO);
	CRY_TEST_CHECK(runner, left->Action(&leftVelocity) != 0);
	CRY_TEST_CHECK(runner, right->Action(&rightVelocity) != 0);

	PhysicsRegressionStep(context, 120);
	const PhysicsRegressionSnapshot leftAfter = PhysicsRegressionReadSnapshot(context, left);
	const PhysicsRegressionSnapshot rightAfter = PhysicsRegressionReadSnapshot(context, right);
	PhysicsRegressionWriteSnapshot(context, "left-after", 120, leftAfter);
	PhysicsRegressionWriteSnapshot(context, "right-after", 120, rightAfter);
	PhysicsRegressionCheckDynamicSnapshot(runner, leftAfter);
	PhysicsRegressionCheckDynamicSnapshot(runner, rightAfter);
	CRY_TEST_CHECK(runner, leftAfter.position.x < rightAfter.position.x);
	CRY_TEST_CHECK(runner, std::fabs(leftAfter.velocity.x + rightAfter.velocity.x) < 0.25f);
	CRY_TEST_CHECK(runner, std::fabs(leftAfter.position.y) < 0.05f);
	CRY_TEST_CHECK(runner, std::fabs(rightAfter.position.y) < 0.05f);
}

static void PhysicsRegressionTestLivingMovementAndContact(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	IPhysicalEntity* floor = PhysicsRegressionCreateBox(
		context,
		PE_STATIC,
		Vec3(0.0f, 0.0f, -0.5f),
		Vec3(12.0f, 12.0f, 0.5f),
		0.0f);
	IPhysicalEntity* obstacle = PhysicsRegressionCreateBox(
		context,
		PE_RIGID,
		Vec3(0.0f, 0.0f, 0.5f),
		Vec3(0.5f),
		25.0f);
	IPhysicalEntity* living = PhysicsRegressionCreateLiving(
		context,
		Vec3(-2.0f, 0.0f, 0.02f));
	CRY_TEST_CHECK(runner, floor != nullptr);
	CRY_TEST_CHECK(runner, obstacle != nullptr);
	CRY_TEST_CHECK(runner, living != nullptr);
	if (floor == nullptr || obstacle == nullptr || living == nullptr)
		return;

	PhysicsRegressionConfigureRigidBody(obstacle, Vec3(0.0f, 0.0f, -9.81f), 0.1f, 0.0f);
	PhysicsRegressionStep(context, 90);

	pe_status_living settledStatus;
	CRY_TEST_CHECK(runner, living->GetStatus(&settledStatus) != 0);
	CRY_TEST_CHECK(runner, settledStatus.bFlying == 0);
	CRY_TEST_CHECK(runner, std::isfinite(settledStatus.groundHeight));
	CRY_TEST_CHECK(runner, PhysicsRegressionIsFinite(settledStatus.groundSlope));

	const PhysicsRegressionSnapshot obstacleBefore = PhysicsRegressionReadSnapshot(context, obstacle);
	pe_action_move movement;
	movement.dir = Vec3(2.5f, 0.0f, 0.0f);
	movement.dt = 0.0f;
	CRY_TEST_CHECK(runner, living->Action(&movement) != 0);
	PhysicsRegressionStep(context, 120);

	pe_status_living movingStatus;
	CRY_TEST_CHECK(runner, living->GetStatus(&movingStatus) != 0);
	const PhysicsRegressionSnapshot livingAfter = PhysicsRegressionReadSnapshot(context, living);
	const PhysicsRegressionSnapshot obstacleAfter = PhysicsRegressionReadSnapshot(context, obstacle);
	PhysicsRegressionWriteSnapshot(context, "living-after", 210, livingAfter);
	PhysicsRegressionWriteSnapshot(context, "obstacle-after", 210, obstacleAfter);
	PhysicsRegressionCheckSnapshot(runner, livingAfter);
	PhysicsRegressionCheckDynamicSnapshot(runner, obstacleAfter);
	CRY_TEST_CHECK(runner, livingAfter.position.x > -1.0f);
	CRY_TEST_CHECK(runner, livingAfter.position.z > -0.05f && livingAfter.position.z < 0.25f);
	CRY_TEST_CHECK(runner, std::isfinite(movingStatus.groundHeight));
	CRY_TEST_CHECK(runner, PhysicsRegressionIsFinite(movingStatus.groundSlope));
	CRY_TEST_CHECK(runner, obstacleAfter.position.x > obstacleBefore.position.x + 0.02f);
}

static PhysicsRegressionSnapshot PhysicsRegressionRunPartitionSample(
	CryTestRunner& runner,
	PhysicsRegressionContext& context,
	const char* label,
	size_t stepCount,
	float timeStep)
{
	IPhysicalEntity* body = PhysicsRegressionCreateBox(
		context,
		PE_RIGID,
		Vec3(0.0f, 0.0f, 10.0f),
		Vec3(0.5f),
		2.0f);
	CRY_TEST_CHECK(runner, body != nullptr);
	if (body == nullptr)
		return PhysicsRegressionSnapshot{};

	PhysicsRegressionConfigureRigidBody(body, Vec3(ZERO), 0.0f, 0.0f);
	pe_action_set_velocity velocity;
	velocity.v = Vec3(3.0f, -1.0f, 0.5f);
	velocity.w = Vec3(ZERO);
	CRY_TEST_CHECK(runner, body->Action(&velocity) != 0);
	PhysicsRegressionStep(context, stepCount, timeStep);
	const PhysicsRegressionSnapshot snapshot = PhysicsRegressionReadSnapshot(context, body);
	PhysicsRegressionWriteSnapshot(context, label, stepCount, snapshot);
	PhysicsRegressionDestroyEntity(context, body);
	context.world->SetPhysicsTime(0.0f);
	context.world->SetiPhysicsTime(0);
	return snapshot;
}

static void PhysicsRegressionTestPartitionedTimeSteps(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	const PhysicsRegressionSnapshot at30Hz = PhysicsRegressionRunPartitionSample(
		runner, context, "30-hz", 30, 1.0f / 30.0f);
	const PhysicsRegressionSnapshot at60Hz = PhysicsRegressionRunPartitionSample(
		runner, context, "60-hz", 60, 1.0f / 60.0f);
	const PhysicsRegressionSnapshot at120Hz = PhysicsRegressionRunPartitionSample(
		runner, context, "120-hz", 120, 1.0f / 120.0f);
	PhysicsRegressionCheckDynamicSnapshot(runner, at30Hz);
	PhysicsRegressionCheckDynamicSnapshot(runner, at60Hz);
	PhysicsRegressionCheckDynamicSnapshot(runner, at120Hz);

	for (int axis = 0; axis < 3; ++axis)
	{
		CRY_TEST_CHECK(runner, PhysicsRegressionNear(at30Hz.position[axis], at60Hz.position[axis], 0.02f));
		CRY_TEST_CHECK(runner, PhysicsRegressionNear(at60Hz.position[axis], at120Hz.position[axis], 0.02f));
		CRY_TEST_CHECK(runner, PhysicsRegressionNear(at30Hz.velocity[axis], at120Hz.velocity[axis], 0.01f));
	}
}

static void PhysicsRegressionTestCollisionEventOwnership(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	IPhysicalEntity* first = PhysicsRegressionCreateBox(
		context, PE_RIGID, Vec3(-1.5f, 0.0f, 2.0f), Vec3(0.5f), 1.0f);
	IPhysicalEntity* second = PhysicsRegressionCreateBox(
		context, PE_RIGID, Vec3(1.5f, 0.0f, 2.0f), Vec3(0.5f), 1.0f);
	CRY_TEST_CHECK(runner, first != nullptr);
	CRY_TEST_CHECK(runner, second != nullptr);
	if (first == nullptr || second == nullptr)
		return;

	PhysicsRegressionConfigureRigidBody(first, Vec3(ZERO), 0.0f, 0.0f);
	PhysicsRegressionConfigureRigidBody(second, Vec3(ZERO), 0.0f, 0.0f);
	PhysicsRegressionEnableCollisionEvents(first);
	PhysicsRegressionEnableCollisionEvents(second);

	PhysicsRegressionCollisionProbe probe;
	probe.world = context.world;
	probe.first = first;
	probe.second = second;
	g_physicsRegressionCollisionProbe = &probe;
	context.world->AddEventClient(EventPhysCollision::id, &PhysicsRegressionOnCollision, 0);

	pe_action_set_velocity firstVelocity;
	firstVelocity.v = Vec3(3.0f, 0.0f, 0.0f);
	firstVelocity.w = Vec3(ZERO);
	pe_action_set_velocity secondVelocity;
	secondVelocity.v = Vec3(-3.0f, 0.0f, 0.0f);
	secondVelocity.w = Vec3(ZERO);
	first->Action(&firstVelocity);
	second->Action(&secondVelocity);
	PhysicsRegressionStep(context, 90);

	context.world->RemoveEventClient(EventPhysCollision::id, &PhysicsRegressionOnCollision, 0);
	g_physicsRegressionCollisionProbe = nullptr;
	CRY_TEST_CHECK(runner, probe.collisionCount > 0);
	CRY_TEST_CHECK(runner, probe.validOwnershipCount > 0);
}

static void PhysicsRegressionTestDestroyDuringCollisionCallback(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	IPhysicalEntity* first = PhysicsRegressionCreateBox(
		context, PE_RIGID, Vec3(-1.5f, 0.0f, 2.0f), Vec3(0.5f), 1.0f);
	IPhysicalEntity* second = PhysicsRegressionCreateBox(
		context, PE_RIGID, Vec3(1.5f, 0.0f, 2.0f), Vec3(0.5f), 1.0f);
	CRY_TEST_CHECK(runner, first != nullptr);
	CRY_TEST_CHECK(runner, second != nullptr);
	if (first == nullptr || second == nullptr)
		return;

	PhysicsRegressionConfigureRigidBody(first, Vec3(ZERO), 0.0f, 0.0f);
	PhysicsRegressionConfigureRigidBody(second, Vec3(ZERO), 0.0f, 0.0f);
	PhysicsRegressionEnableCollisionEvents(first);
	PhysicsRegressionEnableCollisionEvents(second);
	const int secondId = context.world->GetPhysicalEntityId(second);

	PhysicsRegressionCollisionProbe probe;
	probe.world = context.world;
	probe.first = first;
	probe.second = second;
	probe.destroyTarget = second;
	g_physicsRegressionCollisionProbe = &probe;
	context.world->AddEventClient(EventPhysCollision::id, &PhysicsRegressionOnCollision, 0);

	pe_action_set_velocity firstVelocity;
	firstVelocity.v = Vec3(3.0f, 0.0f, 0.0f);
	firstVelocity.w = Vec3(ZERO);
	pe_action_set_velocity secondVelocity;
	secondVelocity.v = Vec3(-3.0f, 0.0f, 0.0f);
	secondVelocity.w = Vec3(ZERO);
	first->Action(&firstVelocity);
	second->Action(&secondVelocity);
	PhysicsRegressionStep(context, 90);

	context.world->RemoveEventClient(EventPhysCollision::id, &PhysicsRegressionOnCollision, 0);
	g_physicsRegressionCollisionProbe = nullptr;
	const auto destroyedEntity = std::find(context.entities.begin(), context.entities.end(), second);
	if (destroyedEntity != context.entities.end())
		context.entities.erase(destroyedEntity);
	context.world->PurgeDeletedEntities();
	CRY_TEST_CHECK(runner, probe.collisionCount > 0);
	CRY_TEST_CHECK(runner, probe.validOwnershipCount > 0);
	CRY_TEST_CHECK(runner, probe.destroyedTarget);
	CRY_TEST_CHECK(runner, context.world->GetPhysicalEntityById(secondId) == nullptr);
}

static void PhysicsRegressionTestContactIslandIsolation(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	IPhysicalEntity* activeLeft = PhysicsRegressionCreateBox(
		context, PE_RIGID, Vec3(-2.0f, 0.0f, 2.0f), Vec3(0.5f), 1.0f);
	IPhysicalEntity* activeRight = PhysicsRegressionCreateBox(
		context, PE_RIGID, Vec3(2.0f, 0.0f, 2.0f), Vec3(0.5f), 1.0f);
	IPhysicalEntity* isolatedLeft = PhysicsRegressionCreateBox(
		context, PE_RIGID, Vec3(-0.5f, 12.0f, 2.0f), Vec3(0.5f), 1.0f);
	IPhysicalEntity* isolatedRight = PhysicsRegressionCreateBox(
		context, PE_RIGID, Vec3(0.5f, 12.0f, 2.0f), Vec3(0.5f), 1.0f);
	const std::array<IPhysicalEntity*, 4> bodies = {
		activeLeft, activeRight, isolatedLeft, isolatedRight
	};
	for (IPhysicalEntity* body : bodies)
		CRY_TEST_CHECK(runner, body != nullptr);
	if (std::find(bodies.begin(), bodies.end(), nullptr) != bodies.end())
		return;

	for (IPhysicalEntity* body : bodies)
		PhysicsRegressionConfigureRigidBody(body, Vec3(ZERO), 0.0f, 0.0f);
	const PhysicsRegressionSnapshot isolatedLeftBefore =
		PhysicsRegressionReadSnapshot(context, isolatedLeft);
	const PhysicsRegressionSnapshot isolatedRightBefore =
		PhysicsRegressionReadSnapshot(context, isolatedRight);

	pe_action_set_velocity leftVelocity;
	leftVelocity.v = Vec3(4.0f, 0.0f, 0.0f);
	leftVelocity.w = Vec3(ZERO);
	pe_action_set_velocity rightVelocity;
	rightVelocity.v = Vec3(-4.0f, 0.0f, 0.0f);
	rightVelocity.w = Vec3(ZERO);
	activeLeft->Action(&leftVelocity);
	activeRight->Action(&rightVelocity);
	PhysicsRegressionStep(context, 120);

	const PhysicsRegressionSnapshot activeLeftAfter =
		PhysicsRegressionReadSnapshot(context, activeLeft);
	const PhysicsRegressionSnapshot activeRightAfter =
		PhysicsRegressionReadSnapshot(context, activeRight);
	const PhysicsRegressionSnapshot isolatedLeftAfter =
		PhysicsRegressionReadSnapshot(context, isolatedLeft);
	const PhysicsRegressionSnapshot isolatedRightAfter =
		PhysicsRegressionReadSnapshot(context, isolatedRight);
	PhysicsRegressionCheckDynamicSnapshot(runner, activeLeftAfter);
	PhysicsRegressionCheckDynamicSnapshot(runner, activeRightAfter);
	PhysicsRegressionCheckDynamicSnapshot(runner, isolatedLeftAfter);
	PhysicsRegressionCheckDynamicSnapshot(runner, isolatedRightAfter);
	CRY_TEST_CHECK(runner, activeLeftAfter.velocity.x < 4.0f);
	CRY_TEST_CHECK(runner, activeRightAfter.velocity.x > -4.0f);
	CRY_TEST_CHECK(runner, PhysicsRegressionNear(
		isolatedLeftAfter.position.x, isolatedLeftBefore.position.x, 0.01f));
	CRY_TEST_CHECK(runner, PhysicsRegressionNear(
		isolatedRightAfter.position.x, isolatedRightBefore.position.x, 0.01f));
	CRY_TEST_CHECK(runner, isolatedLeftAfter.velocity.len() < 0.01f);
	CRY_TEST_CHECK(runner, isolatedRightAfter.velocity.len() < 0.01f);
}

static void PhysicsRegressionTestTextSnapshotBounds(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	IPhysicalEntity* entity = PhysicsRegressionCreateLiving(context, Vec3(0.0f, 0.0f, 10.0f));
	CRY_TEST_CHECK(runner, entity != nullptr);
	if (!entity)
		return;

	std::array<char, 8> output;
	output.fill('!');
	CRY_TEST_CHECK(runner, entity->GetStateSnapshotTxt(nullptr, 8) == 0);
	CRY_TEST_CHECK(runner, entity->GetStateSnapshotTxt(output.data(), 0) == 0);
	CRY_TEST_CHECK(runner, entity->GetStateSnapshotTxt(output.data(), -1) == 0);
	CRY_TEST_CHECK(runner, entity->GetStateSnapshotTxt(output.data(), 1) == 0);
	CRY_TEST_CHECK(runner, std::all_of(output.begin(), output.end(), [](char value) { return value == '!'; }));
}

static void PhysicsRegressionTestWorldDumpRoundTrip(
	CryTestRunner& runner,
	PhysicsRegressionContext& context)
{
	const Vec3 savedPosition(31.0f, -23.0f, 17.0f);
	IPhysicalEntity* living = PhysicsRegressionCreateLiving(context, savedPosition);
	CRY_TEST_CHECK(runner, living != nullptr);
	if (living == nullptr)
		return;

	const int savedId = context.world->GetPhysicalEntityId(living);
	const std::filesystem::path dumpPath =
		std::filesystem::current_path() / "BinTemp" /
		("PhysicsRegressionWorldDump-" + std::to_string(GetCurrentProcessId()) + ".txt");
	std::error_code fileError;
	std::filesystem::create_directories(dumpPath.parent_path(), fileError);
	CRY_TEST_CHECK(runner, !fileError);
	const std::string dumpPathString = dumpPath.string();
	CRY_TEST_CHECK(runner, context.world->SerializeWorld(dumpPathString.c_str(), 1) != 0);
	const uintmax_t savedSize = std::filesystem::file_size(dumpPath, fileError);
	CRY_TEST_CHECK(runner, !fileError);
	CRY_TEST_CHECK(runner, savedSize > 0);

	PhysicsRegressionDestroyEntity(context, living);
	CRY_TEST_CHECK(runner, context.world->GetPhysicalEntityById(savedId) == nullptr);
	CRY_TEST_CHECK(runner, context.world->SerializeWorld(dumpPathString.c_str(), 0) != 0);

	IPhysicalEntity* loaded = context.world->GetPhysicalEntityById(savedId);
	CRY_TEST_CHECK(runner, loaded != nullptr);
	if (loaded != nullptr)
	{
		context.entities.push_back(loaded);
		CRY_TEST_CHECK(runner, loaded->GetType() == PE_LIVING);
		const PhysicsRegressionSnapshot loadedSnapshot =
			PhysicsRegressionReadSnapshot(context, loaded);
		PhysicsRegressionWriteSnapshot(context, "loaded", 0, loadedSnapshot);
		PhysicsRegressionCheckSnapshot(runner, loadedSnapshot);
		CRY_TEST_CHECK(runner, PhysicsRegressionNear(loadedSnapshot.position.x, savedPosition.x, 0.001f));
		CRY_TEST_CHECK(runner, PhysicsRegressionNear(loadedSnapshot.position.y, savedPosition.y, 0.001f));
		CRY_TEST_CHECK(runner, PhysicsRegressionNear(loadedSnapshot.position.z, savedPosition.z, 0.001f));
	}
	std::filesystem::remove(dumpPath, fileError);
}

static const std::array<PhysicsRegressionCase, 23> PhysicsRegressionCases = {{
	{ "world-interface", "lifecycle", &PhysicsRegressionTestWorldInterface },
	{ "convex-hull-buffer-growth", "geometry", &PhysicsRegressionTestConvexHullBufferGrowth },
	{ "debug-entity-enumeration", "diagnostics", &PhysicsRegressionTestDebugEntityEnumeration },
	{ "debug-entity-id-storage", "diagnostics", &PhysicsRegressionTestDebugEntityIdStorage },
	{ "debug-entity-filtering", "diagnostics", &PhysicsRegressionTestDebugEntityFiltering },
	{ "debug-entity-stale-id", "diagnostics", &PhysicsRegressionTestDebugEntityStaleId },
	{ "debug-entity-buffer-reuse", "diagnostics", &PhysicsRegressionTestDebugEntityBufferReuse },
	{ "debug-entity-concurrent-callers", "diagnostics", &PhysicsRegressionTestDebugEntityConcurrentCallers },
	{ "static-body-invariance", "integration", &PhysicsRegressionTestStaticBodyInvariance },
	{ "rigid-free-fall", "integration", &PhysicsRegressionTestRigidFreeFall },
	{ "impulse-response", "actions", &PhysicsRegressionTestImpulseResponse },
	{ "floor-contact-settle", "solver", &PhysicsRegressionTestFloorContactAndSettle },
	{ "stack-stability", "solver", &PhysicsRegressionTestStackStability },
	{ "impulse-collision-pair", "solver", &PhysicsRegressionTestImpulseCollisionPair },
	{ "spatial-query-lifecycle", "broadphase", &PhysicsRegressionTestSpatialQueriesAndEntityLifecycle },
	{ "deterministic-replay", "determinism", &PhysicsRegressionTestDeterministicReplay },
	{ "living-movement-contact", "living", &PhysicsRegressionTestLivingMovementAndContact },
	{ "partitioned-timesteps", "integration", &PhysicsRegressionTestPartitionedTimeSteps },
	{ "collision-event-ownership", "events", &PhysicsRegressionTestCollisionEventOwnership },
	{ "destroy-during-callback", "lifetime", &PhysicsRegressionTestDestroyDuringCollisionCallback },
	{ "contact-island-isolation", "solver", &PhysicsRegressionTestContactIslandIsolation },
	{ "text-snapshot-bounds", "persistence", &PhysicsRegressionTestTextSnapshotBounds },
	{ "world-dump-roundtrip", "persistence", &PhysicsRegressionTestWorldDumpRoundTrip }
}};

static const PhysicsRegressionCase* PhysicsRegressionFindCase(const char* caseName)
{
	const auto found = std::find_if(
		PhysicsRegressionCases.begin(),
		PhysicsRegressionCases.end(),
		[caseName](const PhysicsRegressionCase& testCase)
		{
			return std::strcmp(testCase.name, caseName) == 0;
		});
	return found != PhysicsRegressionCases.end() ? &*found : nullptr;
}

static std::wstring PhysicsRegressionQuoteArgument(const std::wstring& argument)
{
	return L"\"" + argument + L"\"";
}

static std::filesystem::path PhysicsRegressionExecutablePath()
{
	std::vector<wchar_t> pathBuffer(32768);
	const DWORD length = GetModuleFileNameW(
		nullptr,
		pathBuffer.data(),
		static_cast<DWORD>(pathBuffer.size()));
	if (length == 0 || length == pathBuffer.size())
		return {};
	return std::filesystem::path(pathBuffer.data(), pathBuffer.data() + length);
}

static bool PhysicsRegressionRunChild(
	const std::filesystem::path& executablePath,
	const std::filesystem::path& runtimeDirectory,
	const std::filesystem::path& gameRoot,
	const PhysicsRegressionCase& testCase,
	const std::filesystem::path& reportPath,
	DWORD& exitCode,
	bool& timedOut)
{
	const std::wstring wideCaseName(testCase.name, testCase.name + std::strlen(testCase.name));
	std::wstring commandLine =
		PhysicsRegressionQuoteArgument(executablePath.wstring()) +
		L" --physics-regression-case " +
		PhysicsRegressionQuoteArgument(runtimeDirectory.wstring()) + L" " +
		PhysicsRegressionQuoteArgument(gameRoot.wstring()) + L" " +
		PhysicsRegressionQuoteArgument(wideCaseName) + L" " +
		PhysicsRegressionQuoteArgument(reportPath.wstring());

	STARTUPINFOW startupInfo{};
	startupInfo.cb = sizeof(startupInfo);
	PROCESS_INFORMATION processInfo{};
	if (!CreateProcessW(
		nullptr,
		commandLine.data(),
		nullptr,
		nullptr,
		FALSE,
		0,
		nullptr,
		nullptr,
		&startupInfo,
		&processInfo))
	{
		exitCode = GetLastError();
		timedOut = false;
		return false;
	}

	const DWORD waitResult = WaitForSingleObject(processInfo.hProcess, 120000);
	timedOut = waitResult == WAIT_TIMEOUT;
	if (timedOut)
	{
		TerminateProcess(processInfo.hProcess, 3);
		WaitForSingleObject(processInfo.hProcess, 5000);
	}
	if (!GetExitCodeProcess(processInfo.hProcess, &exitCode))
		exitCode = GetLastError();
	CloseHandle(processInfo.hThread);
	CloseHandle(processInfo.hProcess);
	return waitResult == WAIT_OBJECT_0;
}

static void PhysicsRegressionAppendProcessResult(
	const std::filesystem::path& reportPath,
	const PhysicsRegressionCase& testCase,
	const char* status,
	DWORD exitCode)
{
	std::ofstream report(reportPath, std::ios::app);
	if (!report)
		return;
	report
		<< "result\t"
		<< testCase.name << '\t'
		<< status
		<< "\t0\t0\t"
		<< exitCode
		<< "\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\t0\n";
}

int RunPhysicsRegressionCase(
	const wchar_t* runtimeDirectory,
	const wchar_t* gameRoot,
	const char* caseName,
	const wchar_t* reportPath)
{
	std::error_code pathError;
	const std::filesystem::path absoluteRuntimeDirectory =
		std::filesystem::absolute(runtimeDirectory, pathError);
	if (pathError)
	{
		std::fprintf(stderr, "Unable to resolve the physics runtime directory\n");
		return 2;
	}
	const std::filesystem::path absoluteGameRoot =
		std::filesystem::absolute(gameRoot, pathError);
	if (pathError)
	{
		std::fprintf(stderr, "Unable to resolve the physics game root\n");
		return 2;
	}

	const PhysicsRegressionCase* testCase = PhysicsRegressionFindCase(caseName);
	if (testCase == nullptr)
	{
		std::fprintf(stderr, "Unknown physics regression case: %s\n", caseName);
		return 2;
	}

	PhysicsRegressionRuntime runtime;
	std::string initializationError;
	if (!PhysicsRegressionInitializeRuntime(
		absoluteRuntimeDirectory,
		absoluteGameRoot,
		runtime,
		initializationError))
	{
		std::fprintf(stderr, "Physics runtime initialization failed: %s\n", initializationError.c_str());
		PhysicsRegressionShutdownRuntime(runtime);
		return 2;
	}

	std::ofstream report(reportPath, std::ios::app);
	PhysicsRegressionContext context;
	context.world = runtime.world;
	context.report = report ? &report : nullptr;
	context.caseName = testCase->name;

	CryTestRunner runner;
	runner.Run(testCase->name, testCase->function, context);
	PhysicsRegressionDestroyEntities(context);
	const int exitCode = runner.ExitCode();
	report.close();
	PhysicsRegressionShutdownRuntime(runtime);
	return exitCode;
}

int RunPhysicsRegressionPipeline(
	const wchar_t* runtimeDirectory,
	const wchar_t* gameRoot,
	const wchar_t* reportPath)
{
	std::error_code pathError;
	const std::filesystem::path absoluteRuntimeDirectory =
		std::filesystem::absolute(runtimeDirectory, pathError);
	if (pathError)
	{
		std::fprintf(stderr, "Unable to resolve the physics runtime directory\n");
		return 2;
	}
	const std::filesystem::path absoluteGameRoot =
		std::filesystem::absolute(gameRoot, pathError);
	if (pathError)
	{
		std::fprintf(stderr, "Unable to resolve the physics game root\n");
		return 2;
	}
	const std::filesystem::path absoluteReportPath =
		std::filesystem::absolute(reportPath, pathError);
	if (pathError)
	{
		std::fprintf(stderr, "Unable to resolve the physics report path\n");
		return 2;
	}

	const std::filesystem::path executablePath = PhysicsRegressionExecutablePath();
	if (executablePath.empty())
	{
		std::fprintf(stderr, "Unable to resolve the CryCommonTests executable path\n");
		return 2;
	}

	std::ofstream report(absoluteReportPath, std::ios::trunc);
	if (!report)
	{
		std::fprintf(stderr, "Unable to create the physics regression report\n");
		return 2;
	}
	report << "record\tcase\tlabel\tsequence\tstep\tentity_id\tsim_class\tawake\t"
		"position_x\tposition_y\tposition_z\torientation_x\torientation_y\t"
		"orientation_z\torientation_w\tvelocity_x\tvelocity_y\tvelocity_z\t"
		"angular_velocity_x\tangular_velocity_y\tangular_velocity_z\tenergy\tcontacts\n";
	report.close();

	int failureCount = 0;
	for (const PhysicsRegressionCase& testCase : PhysicsRegressionCases)
	{
		std::printf("\n[RUN ] %s (%s)\n", testCase.name, testCase.category);
		std::fflush(stdout);
		DWORD exitCode = 0;
		bool timedOut = false;
		const bool completed = PhysicsRegressionRunChild(
			executablePath,
			absoluteRuntimeDirectory,
			absoluteGameRoot,
			testCase,
			absoluteReportPath,
			exitCode,
			timedOut);
		const char* status = "PASS";
		if (timedOut)
			status = "TIMEOUT";
		else if (!completed)
			status = "LAUNCH_ERROR";
		else if (exitCode == 1)
			status = "FAIL";
		else if (exitCode == 2)
			status = "SETUP_ERROR";
		else if (exitCode != 0)
			status = "CRASH";

		PhysicsRegressionAppendProcessResult(absoluteReportPath, testCase, status, exitCode);
		std::printf("[%s] %s (exit %lu)\n", status, testCase.name, exitCode);
		if (exitCode != 0 || !completed)
			++failureCount;
	}

	std::printf(
		"\nPhysics regression: %zu case(s), %d failure(s)\nReport: %ls\n",
		PhysicsRegressionCases.size(),
		failureCount,
		absoluteReportPath.c_str());
	return failureCount == 0 ? 0 : 1;
}
