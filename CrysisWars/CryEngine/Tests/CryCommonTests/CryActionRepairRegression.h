#pragma once
#include <ILog.h>
#include <IPhysics.h>
#include <string>
#include <vector>

struct IEntity;
struct IStatObj;

class CryActionRegressionDiagnostics : public ILogCallback
{
public:
    CryActionRegressionDiagnostics();
    ~CryActionRegressionDiagnostics();
    void OnWriteToConsole(const char*, bool) override;
    void OnWriteToFile(const char*, bool) override;
    bool Contains(const char*) const;
    std::vector<std::string> lines;
private:
    int m_verbosity, m_fileVerbosity;
};

class CryActionRegressionWindowsFixture
{
public:
    CryActionRegressionWindowsFixture();
    ~CryActionRegressionWindowsFixture();
    LONG Store(DWORD, const char*, DWORD);
    static void CaptureKey(const char*);
    static DWORD WINAPI TempPath(DWORD, LPSTR);
    static CryActionRegressionWindowsFixture* active;
    std::string path, observed, temporary;
    DWORD temporaryResult = 0;
    unsigned observations = 0, tempCalls = 0;
};

class CryActionRegressionBreakFixture
{
public:
    explicit CryActionRegressionBreakFixture(IPhysicalWorld*);
    ~CryActionRegressionBreakFixture();
    IPhysicalEntity* CreateBody(IEntity*, int);
    IStatObj* CreatePlane();
    static void ClearPlaneGrid(IStatObj*);
    void Destroy(IPhysicalEntity*);
private:
    IPhysicalWorld* m_world;
    std::vector<IPhysicalEntity*> m_bodies;
};
