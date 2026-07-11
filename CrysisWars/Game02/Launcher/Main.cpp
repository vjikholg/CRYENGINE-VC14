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
#include <platform_impl.h>
#include <StringUtils.h>

#define SECUROM_INCLUDE_EXE_FUNCTIONS
#include <CopyProtection.h>

#define GAMEDLL_FILENAME "CryGame.dll"

int RunGame(const char *commandLine)
{
	HANDLE mutex = CreateMutex(NULL, TRUE, "CrytekApplication");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		if (CryStringUtils::stristr(commandLine, "-devmode") == 0 && CryStringUtils::stristr(commandLine, "-modrestart") == 0)
		{
			MessageBox(0, "There is already a Crytek application running. Cannot start another one!", "Error", MB_OK | MB_DEFAULT_DESKTOP_ONLY);
			return 1;
		}

		if(MessageBox(GetDesktopWindow(), "There is already a Crytek application running\nDo you want to start another one?", "Too many apps", MB_YESNO)!=IDYES)
			return 1;
	}

	//restart parameters
	static const size_t MAX_RESTART_LEVEL_NAME = 256;
	char fileName[MAX_RESTART_LEVEL_NAME];
	strcpy(fileName, "");
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
	strcpy(startupParams.szSystemCmdLine, commandLine);
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

		char * pRestartLevelName = NULL;
		if (fileName[0])
			pRestartLevelName = fileName;

 		char pBuffer[256];
 		const char* substr = strstr(commandLine, "restart");
 		if(substr != NULL)
 		{
 			int len = substr-commandLine;
 			strncpy(pBuffer, commandLine, len);
 			strcpy(pBuffer + len, commandLine + len + 7);
 			pGameStartup->Run(pBuffer);	//restartLevel to be loaded
 		}
 		else
 		{
 			const char* loadstr = strstr(commandLine, "load");
 			if(loadstr != NULL)
 			{
 				int len = loadstr - commandLine;
 				strncpy(pBuffer, commandLine, len);
 				strcpy(pBuffer + len, commandLine + len + 4);
 				pGameStartup->Run(pBuffer);	//restartLevel to be loaded
 			}
 			else
 				pGameStartup->Run(NULL);
		}

		bool isLevelRequested = pGameStartup->GetRestartLevel(&pRestartLevelName);
		if (pRestartLevelName)
		{
			if (strlen(pRestartLevelName) < MAX_RESTART_LEVEL_NAME)
				strcpy(fileName, pRestartLevelName);
		}

		char pRestartMod[255];
		bool isModRequested = pGameStartup->GetRestartMod(pRestartMod, 255);

		if (isLevelRequested || isModRequested)
		{
			STARTUPINFO si;
			ZeroMemory( &si, sizeof(si) );
			si.cb = sizeof(si);
			PROCESS_INFORMATION pi;

			if (isLevelRequested)
			{
				strcpy(pBuffer, "restarting: restart ");
				strcat(pBuffer, fileName);
			}

			if (isModRequested)
			{
				strcat(pBuffer, "restarting:  -mod ");
				strcat(pBuffer, pRestartMod);
			}
			
			CreateProcess("bin32/crysis.exe", pBuffer, NULL, NULL, FALSE, /*flags*/ 0, NULL, NULL, &si, &pi);
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
	// we need pass the full command line, including the filename
	// lpCmdLine does not contain the filename.

	//check for a restart
	const char* pos = strstr(lpCmdLine, "restart");
	if(pos != NULL)
	{
		Sleep(5000); //wait for old instance to be deleted
		return RunGame(lpCmdLine);	//pass the restart level if restarting
	}
	else
		pos = strstr(lpCmdLine, "load");// commandLine.find("load");

	if(pos != NULL)
		RunGame(lpCmdLine);

	int nRes = RunGame(GetCommandLineA());

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
