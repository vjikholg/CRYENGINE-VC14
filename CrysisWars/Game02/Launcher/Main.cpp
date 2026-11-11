/*************************************************************************
Crytek Source File.
Copyright (C), Crytek Studios, 2001-2004.
-------------------------------------------------------------------------
$Id$
$DateTime$

-------------------------------------------------------------------------
History:
- 28:7:2004   11:41 : Created by Marco Koegler
- 30:7:2004   11:02 : Taken-over by Márcio Martins
- xx:x:2006		xx:xx : Taken-over by Jan Müller

*************************************************************************/

#include "StdAfx.h"
#include <CryLibrary.h>
#include <IGameStartup.h>
#include <IConsole.h>
#include <ICmdLine.h>
#include <platform_impl.h>
#include <StringUtils.h>
#include <algorithm>
#include <string>

#define SECUROM_INCLUDE_EXE_FUNCTIONS
#include <CopyProtection.h>

#define GAMEDLL_FILENAME "CryGame.dll"

int RunGame(const char *commandLine, bool allowMultipleClients)
{
	HANDLE mutex = CreateMutex(NULL, TRUE, allowMultipleClients ? nullptr : "CrytekApplication");
	if (!allowMultipleClients && GetLastError() == ERROR_ALREADY_EXISTS)
	{
		if (CryStringUtils::stristr(commandLine, "-devmode") == 0 && CryStringUtils::stristr(commandLine, "-modrestart") == 0)
		{
			MessageBox(0, "There is already a Crytek application running. Cannot start another one!", "Error", MB_OK | MB_DEFAULT_DESKTOP_ONLY);
			return 1;
		}

		if(MessageBox(GetDesktopWindow(), "There is already a Crytek application running\nDo you want to start another one?", "Too many apps", MB_YESNO)!=IDYES)
			return 1;
	}

	static const char logFileName[] = "Game.log";

	// load the game dll
	HMODULE gameDll = CryLoadLibrary(GAMEDLL_FILENAME);

	if (!gameDll)
	{
/*
		LPVOID lpMsgBuf;
		DWORD dw = GetLastError(); 

		FormatMessage(
			FORMAT_MESSAGE_ALLOCATE_BUFFER | 
			FORMAT_MESSAGE_FROM_SYSTEM |
			FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			dw,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			(LPTSTR) &lpMsgBuf,
			0, NULL );
*/

		MessageBox(0, "Failed to load the Game DLL!", "Error", MB_OK | MB_DEFAULT_DESKTOP_ONLY);
		// failed to load the dll

		return 0;
	}

	// get address of startup function
	IGameStartup::TEntryFunction CreateGameStartup = (IGameStartup::TEntryFunction)CryGetProcAddress(gameDll, "CreateGameStartup");

	if (!CreateGameStartup)
	{
		// dll is not a compatible game dll
		CryFreeLibrary(gameDll);

		MessageBox(0, "Specified Game DLL is not valid!", "Error", MB_OK | MB_DEFAULT_DESKTOP_ONLY);

		CloseHandle(mutex);
		return 0;
	}

	SSystemInitParams startupParams;

	memset(&startupParams, 0, sizeof(SSystemInitParams));

	startupParams.hInstance = GetModuleHandle(0);
	startupParams.sLogFileName = logFileName;
	if (strlen(commandLine) >= sizeof(startupParams.szSystemCmdLine))
	{
		CryFreeLibrary(gameDll);
		CloseHandle(mutex);
		return 1;
	}
	strcpy_s(startupParams.szSystemCmdLine, commandLine);
	//startupParams.pProtectedFunctions[0] = &TestProtectedFunction;

#ifdef SECUROM_32
	startupParams.pCheckFunc = &AuthCheckFunction;
	startupParams.pProtectedFunctions[eProtectedFunc_Load] = &ProtectedFunction_Load;
#endif

	// create the startup interface
	IGameStartup *pGameStartup = CreateGameStartup();

	if (!pGameStartup)
	{
		// failed to create the startup interface
		CryFreeLibrary(gameDll);

		MessageBox(0, "Failed to create the GameStartup Interface!", "Error", MB_OK | MB_DEFAULT_DESKTOP_ONLY);

		CloseHandle(mutex);
		return 0;
	}

	// run the game
	if (pGameStartup->Init(startupParams))
	{
		gEnv = startupParams.pSystem->GetGlobalEnvironment();
		//startupParams.pProtectedFunctions[0](0,0);

		// Check for signature file.
		if (!gEnv->pCryPak->IsFileExist("config/config.dat"))
			return -1;

		const ICmdLine* args = startupParams.pSystem->GetICmdLine();
		const ICmdLineArg* level = args->FindArg(eCLAT_Normal, "restart");
		if (!level)
			level = args->FindArg(eCLAT_Normal, "load");
		pGameStartup->Run(level && *level->GetValue() ? level->GetValue() : NULL);

		char* pRestartLevelName = NULL;
		bool isLevelRequested = pGameStartup->GetRestartLevel(&pRestartLevelName) && pRestartLevelName;
		char pRestartMod[255];
		bool isModRequested = pGameStartup->GetRestartMod(pRestartMod, 255);

		if (isLevelRequested || isModRequested)
		{
			std::string restartArguments;
			if (isLevelRequested)
				restartArguments += std::string(" restart \"") + pRestartLevelName + "\"";

			restartArguments += " -modrestart";
			if (isModRequested)
				restartArguments += std::string(" -mod \"") + pRestartMod + "\"";

			const char* options[] = { "mod", "root", "mwll_user_root", "dx9", "dx10", "devmode", "multiclient" };
			for (const char* option : options)
			{
				if (isModRequested && strcmp(option, "mod") == 0)
					continue;

				if (const ICmdLineArg* arg = args->FindArg(eCLAT_Pre, option))
				{
					restartArguments += std::string(" -") + option;
					if (*arg->GetValue())
						restartArguments += std::string(" \"") + arg->GetValue() + "\"";
				}
			}
			char executablePath[_MAX_PATH];
			const DWORD length = GetModuleFileNameA(NULL, executablePath, _countof(executablePath));
			if (length > 0 && length < _countof(executablePath))
			{
				std::string restartCommand = std::string("\"") + executablePath + "\"" + restartArguments;
				STARTUPINFOA startup = {};
				startup.cb = sizeof(startup);
				PROCESS_INFORMATION process = {};
				if (CreateProcessA(executablePath, &restartCommand[0], NULL, NULL, FALSE, 0, NULL, NULL, &startup, &process))
				{
					CloseHandle(process.hThread);
					CloseHandle(process.hProcess);
				}
				else
				{
					MessageBox(0, "Failed to restart Crysis.", "Error", MB_OK | MB_DEFAULT_DESKTOP_ONLY);
				}
			}
			else
			{
				MessageBox(0, "Could not resolve the Crysis executable for restart.", "Error", MB_OK | MB_DEFAULT_DESKTOP_ONLY);
			}
		}
		else
		{
			// check if there is a patch to install. If there is, do it now.
			const char* pfilename = pGameStartup->GetPatch();
			if(pfilename)
			{
				STARTUPINFO si;
				ZeroMemory( &si, sizeof(si) );
				si.cb = sizeof(si);
				PROCESS_INFORMATION pi;
				CreateProcess(pfilename, NULL, NULL, NULL, FALSE, /*flags*/ 0, NULL, NULL, &si, &pi);
			}
		}

		pGameStartup->Shutdown();
		pGameStartup = 0;

		CryFreeLibrary(gameDll);
	}
	else
	{
		MessageBox(0, "Failed to initialize the GameStartup Interface!", "Error", MB_OK | MB_DEFAULT_DESKTOP_ONLY);

		// if initialization failed, we still need to call shutdown
		pGameStartup->Shutdown();
		pGameStartup = 0;

		CryFreeLibrary(gameDll);

		CloseHandle(mutex);
		return 0;
	}

	CloseHandle(mutex);

	return 0;
}

//////////////////////////////////////////////////////////////////////////
// Support relaunching for windows media center edition.
//////////////////////////////////////////////////////////////////////////
#if defined(WIN32) && !defined(XENON)
#if(_WIN32_WINNT < 0x0501)
#define SM_MEDIACENTER          87
#endif
bool ReLaunchMediaCenter()
{
	// Skip if not running on a Media Center
	if( GetSystemMetrics( SM_MEDIACENTER ) == 0 ) 
		return false;

	// Get the path to Media Center
	char szExpandedPath[MAX_PATH];
	if( !ExpandEnvironmentStrings( "%SystemRoot%\\ehome\\ehshell.exe", szExpandedPath, MAX_PATH) )
		return false;

	// Skip if ehshell.exe doesn't exist
	if( GetFileAttributes( szExpandedPath ) == 0xFFFFFFFF )
		return false;

	// Launch ehshell.exe 
	INT_PTR result = (INT_PTR)ShellExecute( NULL, TEXT("open"), szExpandedPath, NULL, NULL, SW_SHOWNORMAL);
	return (result > 32);
}
#endif //defined(WIN32) && !defined(XENON)


///////////////////////////////////////////////
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
	{
	const bool allowMultipleClients = std::any_of(__argv + 1, __argv + __argc, [](const char* arg)
	{
		return _stricmp(arg, "-multiclient") == 0;
	});

	// we need pass the full command line, including the filename
	// lpCmdLine does not contain the filename.

	const bool restarting = (__argc > 1 && _stricmp(__argv[1], "restart") == 0) ||
		std::any_of(__argv + 1, __argv + __argc, [](const char* arg)
		{
			return _stricmp(arg, "-modrestart") == 0;
		});
	if (restarting)
		Sleep(5000); //wait for old instance to be deleted

	int nRes = RunGame(GetCommandLineA(), allowMultipleClients);

	//////////////////////////////////////////////////////////////////////////
	// Support relaunching for windows media center edition.
	//////////////////////////////////////////////////////////////////////////
#if defined(WIN32) && !defined(XENON)
	if (strstr(lpCmdLine,"ReLaunchMediaCenter") != 0)
	{
		ReLaunchMediaCenter();
	}
#endif
	//////////////////////////////////////////////////////////////////////////

	return nRes;
}
