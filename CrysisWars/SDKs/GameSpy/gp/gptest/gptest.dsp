# Microsoft Developer Studio Project File - Name="gptest" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=gptest - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "gptest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "gptest.mak" CFG="gptest - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "gptest - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "gptest - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""$/Gamespy/GOA/GP/gptest", WFHAAAAA"
# PROP Scc_LocalPath "."
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "gptest - Win32 Release"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MT /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386 /ignore:4089
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "gptest - Win32 Debug"

# PROP BASE Use_MFC 5
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 5
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "gptest - Win32 Release"
# Name "gptest - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\gptest.cpp

!IF  "$(CFG)" == "gptest - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "gptest - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\gptest.rc
# End Source File
# Begin Source File

SOURCE=.\gptestDlg.cpp

!IF  "$(CFG)" == "gptest - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "gptest - Win32 Debug"

!ENDIF 

# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp

!IF  "$(CFG)" == "gptest - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "gptest - Win32 Debug"

# ADD CPP /Yc"stdafx.h"

!ENDIF 

# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\gptest.h
# End Source File
# Begin Source File

SOURCE=.\gptestDlg.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\gptest.ico
# End Source File
# Begin Source File

SOURCE=.\res\gptest.rc2
# End Source File
# End Group
# Begin Group "Cached GP Info"

# PROP Default_Filter "info"
# Begin Source File

SOURCE=.\gp.info
# End Source File
# End Group
# Begin Group "PresenceSDK"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\gp.c

!IF  "$(CFG)" == "gptest - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "gptest - Win32 Debug"

# SUBTRACT CPP /X /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\gp.h
# End Source File
# Begin Source File

SOURCE=..\gpi.c

!IF  "$(CFG)" == "gptest - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "gptest - Win32 Debug"

# SUBTRACT CPP /X /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\gpi.h
# End Source File
# Begin Source File

SOURCE=..\gpiBuddy.c

!IF  "$(CFG)" == "gptest - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "gptest - Win32 Debug"

# SUBTRACT CPP /X /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\gpiBuddy.h
# End Source File
# Begin Source File

SOURCE=..\gpiBuffer.c

!IF  "$(CFG)" == "gptest - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "gptest - Win32 Debug"

# SUBTRACT CPP /X /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\gpiBuffer.h
# End Source File
# Begin Source File

SOURCE=..\gpiCallback.c

!IF  "$(CFG)" == "gptest - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "gptest - Win32 Debug"

# SUBTRACT CPP /X /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\gpiCallback.h
# End Source File
# Begin Source File

SOURCE=..\gpiConnect.c

!IF  "$(CFG)" == "gptest - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "gptest - Win32 Debug"

# SUBTRACT CPP /X /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\gpiConnect.h
# End Source File
# Begin Source File

SOURCE=..\gpiInfo.c

!IF  "$(CFG)" == "gptest - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "gptest - Win32 Debug"

# SUBTRACT CPP /X /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\gpiInfo.h
# End Source File
# Begin Source File

SOURCE=..\gpiOperation.c

!IF  "$(CFG)" == "gptest - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "gptest - Win32 Debug"

# SUBTRACT CPP /X /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\gpiOperation.h
# End Source File
# Begin Source File

SOURCE=..\gpiPeer.c

!IF  "$(CFG)" == "gptest - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "gptest - Win32 Debug"

# SUBTRACT CPP /X /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\gpiPeer.h
# End Source File
# Begin Source File

SOURCE=..\gpiProfile.c

!IF  "$(CFG)" == "gptest - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "gptest - Win32 Debug"

# SUBTRACT CPP /X /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\gpiProfile.h
# End Source File
# Begin Source File

SOURCE=..\gpiSearch.c

!IF  "$(CFG)" == "gptest - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "gptest - Win32 Debug"

# SUBTRACT CPP /X /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\gpiSearch.h
# End Source File
# Begin Source File

SOURCE=..\gpiTransfer.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\gpiTransfer.h
# End Source File
# Begin Source File

SOURCE=..\gpiUnique.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\gpiUnique.h
# End Source File
# Begin Source File

SOURCE=..\gpiUtility.c

!IF  "$(CFG)" == "gptest - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "gptest - Win32 Debug"

# SUBTRACT CPP /X /YX /Yc /Yu

!ENDIF 

# End Source File
# Begin Source File

SOURCE=..\gpiUtility.h
# End Source File
# End Group
# Begin Group "GsCommon"

# PROP Default_Filter ""
# Begin Source File

SOURCE=..\..\darray.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\darray.h
# End Source File
# Begin Source File

SOURCE=..\..\common\gsAssert.c
# End Source File
# Begin Source File

SOURCE=..\..\common\gsAssert.h
# End Source File
# Begin Source File

SOURCE=..\..\common\gsAvailable.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\common\gsAvailable.h
# End Source File
# Begin Source File

SOURCE=..\..\common\gsCommon.h
# End Source File
# Begin Source File

SOURCE=..\..\common\gsDebug.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\common\gsDebug.h
# End Source File
# Begin Source File

SOURCE=..\..\common\gsMemory.c
# End Source File
# Begin Source File

SOURCE=..\..\common\gsMemory.h
# End Source File
# Begin Source File

SOURCE=..\..\common\gsPlatform.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\common\gsPlatform.h
# End Source File
# Begin Source File

SOURCE=..\..\common\gsPlatformSocket.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\common\gsPlatformSocket.h
# End Source File
# Begin Source File

SOURCE=..\..\common\gsPlatformThread.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\common\gsPlatformThread.h
# End Source File
# Begin Source File

SOURCE=..\..\common\gsPlatformUtil.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\common\gsPlatformUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\common\gsStringUtil.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\common\gsStringUtil.h
# End Source File
# Begin Source File

SOURCE=..\..\hashtable.c
# SUBTRACT CPP /YX /Yc /Yu
# End Source File
# Begin Source File

SOURCE=..\..\hashtable.h
# End Source File
# Begin Source File

SOURCE=..\..\md5.h
# End Source File
# Begin Source File

SOURCE=..\..\md5c.c

!IF  "$(CFG)" == "gptest - Win32 Release"

# SUBTRACT CPP /YX /Yc /Yu

!ELSEIF  "$(CFG)" == "gptest - Win32 Debug"

# SUBTRACT CPP /X /YX /Yc /Yu

!ENDIF 

# End Source File
# End Group
# Begin Source File

SOURCE=.\GP.log
# End Source File
# End Target
# End Project
