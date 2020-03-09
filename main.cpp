#include <iostream>
#include "Dronegine.h"
#include <string>
#include <tchar.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <pdh.h>
#include <vector>

#define KEY_UP 0x26
#define KEY_DOWN 0x28
#define KEY_LEFT 0x25
#define KEY_RIGHT 0x27

int numDigits(int number)
{
    int digits = 0;
    if (number < 0)
        digits = 1; // remove this line if '-' counts as a digit
    while (number)
    {
        number /= 10;
        digits++;
    }
    return digits;
}

class process
{
public:
    process(std::string str, int p, int cThr, int pPrcId)
    {
        name = str;
        pid = p;
        cntThreads = cThr;
        parentProcID = pPrcId;
        ZeroMemory(&prevPROCuser, sizeof(FILETIME));
        ZeroMemory(&prevPROCkernel, sizeof(FILETIME));
        cpuUsage = 0;
        ramUsage = 0;
    }

    std::string GetProcessInfo()
    {
        std::string buf = name + " | " + std::to_string(pid) + " | " + std::to_string(cntThreads);
        return buf;
    }

    ~process()
    {
    }

    std::string name;
    int pid;
    int cntThreads;
    int parentProcID;
    FILETIME prevPROCuser, prevPROCkernel;
    short cpuUsage;
    size_t ramUsage;
};

class Application : public Dronegine
{
public:
    Application()
    {
        showProcessInformation();
        ZeroMemory(&prevCPUuser, sizeof(FILETIME));
        ZeroMemory(&prevCPUkernel, sizeof(FILETIME));
    }

    ~Application()
    {
    }
    virtual bool Update(float fElapsedtime)
    {
        if (m_keys[KEY_UP].bHeld == true)
        {
            if (this->arrayShift > 0)
            {
                arrayShift--;
            }
        }

        if (m_keys[KEY_DOWN].bHeld == true)
        {
            if (this->arrayShift / 2 < this->prcs.size())
            {
                arrayShift++;
            }
        }

        if (counter > 0.5f)
        {
            updateUsageTime();
            updateRAMUsage();
            counter = 0.0f;
        }
        else
        {
            counter += fElapsedtime;
        }

        int x = 2, y = 3;
        WriteString(10, 1, "PROCESS NAME");
        WriteString(maxNameSize + x, 1, "|");
        WriteString(maxNameSize + x + 2, 1, "PID");
        WriteString(maxNameSize + x + 10, 1, "|");
        WriteString(maxNameSize + x + 40, 1, std::to_string(counter));
        FillXByText(0, 2, '-', GetWidth());
        for (int i = 0 + arrayShift; i < this->prcs.size(); i++)
        {
            if (y <= GetHeight())
            {
                int columnWidth = maxNameSize;
                int xShift = 0;
                WriteString(x, y, this->prcs[i].name);
                WriteString(columnWidth + x, y, "|");
                xShift = maxNameSize + x;

                xShift += 2;
                WriteString(x + xShift, y, std::to_string(this->prcs[i].pid));
                xShift += 6; // 4 max dig number + 2 char of spaces
                WriteString(x + xShift, y, "|");
                xShift += 5;
                WriteString(x + xShift, y, std::to_string(this->prcs[i].parentProcID));
                xShift += 5;
                WriteString(x + xShift, y, "|");
                xShift += 5;
                WriteString(x + xShift, y, std::to_string(this->prcs[i].cpuUsage));
                xShift += 5;
                WriteString(x + xShift, y, "|");
                xShift += 5;
                WriteString(x + xShift, y, std::to_string(this->prcs[i].ramUsage));

                y++;
            }
        }
        return true;
    }

    void showProcessInformation()
    {
        HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
        if (hSnapshot)
        {
            this->maxNameSize = 0;
            PROCESSENTRY32 pe32;
            pe32.dwSize = sizeof(PROCESSENTRY32);
            if (Process32First(hSnapshot, &pe32))
            {
                do
                {
                    char name[MAX_PATH];
                    sprintf(name, "%s", pe32.szExeFile);
                    std::string strname(name);
                    process p(strname, pe32.th32ProcessID, pe32.cntThreads, pe32.th32ParentProcessID);
                    this->prcs.push_back(p);

                    if (strname.size() > this->maxNameSize)
                    {
                        this->maxNameSize = strname.size();
                    }
                } while (Process32Next(hSnapshot, &pe32));
            }
            CloseHandle(hSnapshot);
        }
    }

    ULONGLONG SubtractTimes(const FILETIME &ftA, const FILETIME &ftB)
    {
        LARGE_INTEGER a, b;
        a.LowPart = ftA.dwLowDateTime;
        a.HighPart = ftA.dwHighDateTime;

        b.LowPart = ftB.dwLowDateTime;
        b.HighPart = ftB.dwHighDateTime;

        return a.QuadPart - b.QuadPart;
    }

    void updateUsageTime()
    {
        FILETIME CPUidle, CPUkernel, CPUuser;
        FILETIME PROCcreation, PROCexit, PROCkernel, PROCuser;

        GetSystemTimes(&CPUidle, &CPUkernel, &CPUuser);
        for (int i = 0; i < prcs.size(); i++)
        {
            HANDLE h = OpenProcess(PROCESS_ALL_ACCESS, true, prcs[i].pid);
            GetProcessTimes(h, &PROCcreation, &PROCexit, &PROCkernel, &PROCuser);

            ULONGLONG ftSysKernelDiff =
                SubtractTimes(CPUkernel, this->prevCPUkernel);
            ULONGLONG ftSysUserDiff =
                SubtractTimes(CPUuser, this->prevCPUuser);

            ULONGLONG ftProcKernelDiff =
                SubtractTimes(PROCkernel, prcs[i].prevPROCkernel);
            ULONGLONG ftProcUserDiff =
                SubtractTimes(PROCuser, prcs[i].prevPROCuser);

            ULONGLONG nTotalSys = ftSysKernelDiff + ftSysUserDiff;
            ULONGLONG nTotalProc = ftProcKernelDiff + ftProcUserDiff;

            short CpuUsage = (short)((100.0 * nTotalProc) / nTotalSys);
            prcs[i].cpuUsage = ((CpuUsage > 100) || (CpuUsage < 0)) ? 0 : CpuUsage;

            prcs[i].prevPROCuser = PROCuser;
            prcs[i].prevPROCkernel = PROCkernel;
            CloseHandle(h);
        }
        this->prevCPUkernel = CPUkernel;
        this->prevCPUuser = CPUuser;
    }

    void updateRAMUsage()
    {
        for (int i = 0; i < prcs.size(); i++)
        {
            PROCESS_MEMORY_COUNTERS pmc;
            HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, true, prcs[i].pid);

            if (GetProcessMemoryInfo(hProcess, &pmc, sizeof(pmc)))
            {
                prcs[i].ramUsage = (pmc.WorkingSetSize);
            }
            CloseHandle(hProcess);
        }
    }

private:
    bool needTopdateProc = true;
    int arrayShift = 0;
    int maxNameSize = 0;
    std::vector<process> prcs;
    FILETIME prevCPUuser, prevCPUkernel;
    float counter = 0;
};

int main()
{
    Application *app = new Application();
    app->ConstructConsole(120, 50);
    app->Start();

    return 1;
}