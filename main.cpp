#include <iostream>
#include "Dronegine.h"
#include <tchar.h>
#include <Psapi.h>
#include <pdh.h>

#define KEY_UP 0x26
#define KEY_DOWN 0x28
#define KEY_LEFT 0x25
#define KEY_RIGHT 0x27

class Application : public Dronegine
{
public:
    Application()
    {
        EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded);
        cProcesses = cbNeeded / sizeof(DWORD);
    }

    ~Application()
    {
    }

    bool GetProccessId()
    {
        return (EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded));
    }

    virtual bool Update(float fElapsedtime)
    {
        if(m_keys[KEY_UP].bHeld == true)
        {
            if(this->arrayShift > 0)
            {
                arrayShift--;
            }
        }

        if(m_keys[KEY_DOWN].bHeld == true)
        {
            if(this->arrayShift / 2 < this->cProcesses)
            {
                arrayShift++;
            }
        }

        int x = 2, y = 0;
        for (int i = 0 + arrayShift; i < cProcesses; i++)
        {
            if (cProcesses > 0)
            {
                if (y <= GetHeight())
                {
                    WriteString(x, y, std::to_string(aProcesses[i]));
                    y++;
                }
            }
        }
        return true;
    }

    void PrintProcessNameAndID(DWORD processID)
    {
        TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");

        // Get a handle to the process.

        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION |
                                          PROCESS_VM_READ,
                                      FALSE, processID);

        // Get the process name.

        if (NULL != hProcess)
        {
            HMODULE hMod;
            DWORD cbNeeded;

            if (EnumProcessModules(hProcess, &hMod, sizeof(hMod),
                                   &cbNeeded))
            {
                GetModuleBaseName(hProcess, hMod, szProcessName,
                                  sizeof(szProcessName) / sizeof(TCHAR));
            }
        }

        // Print the process name and identifier.
        // Release the handle to the process.

        CloseHandle(hProcess);
    }

private:
    DWORD aProcesses[1024], cbNeeded = -1, cProcesses = -1;
    bool needTopdateProc = true;
    int arrayShift = 0;
};

int main()
{
    Application *app = new Application();
    app->ConstructConsole(80, 50);
    app->Start();

    return 1;
}