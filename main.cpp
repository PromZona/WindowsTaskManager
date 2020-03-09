#include <iostream>
#include "Dronegine.h"
#include <string>
#include <tchar.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <pdh.h>
#include <vector>
#include <algorithm>

#define KEY_UP 0x26
#define KEY_DOWN 0x28
#define KEY_LEFT 0x25
#define KEY_RIGHT 0x27
#define KEY_C 0x43
#define KEY_I 0x49
#define KEY_N 0x4E
#define KEY_R 0x52
#define KEY_M 0x4D

#define FLAT 0
#define TREE 1

#define ID_SORT 0
#define CPU_SORT 1
#define RAM_SORT 2
#define NAME_SORT 3

#define RAM_SHOW_BYTES 0
#define RAM_SHOW_KB 1
#define RAM_SHOW_MB 2

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

bool cmpPrcName(process pr1, process pr2)
{
    return (pr1.name.compare(pr2.name) > 0);
}

bool cmpPrcID(process pr1, process pr2)
{
    return (pr1.pid > pr2.pid);
}

bool cmpPrcCPU(process pr1, process pr2)
{
    return (pr1.cpuUsage > pr2.cpuUsage);
}

bool cmpPrcRAM(process pr1, process pr2)
{
    return (pr1.ramUsage > pr2.ramUsage);
}

///
bool rcmpPrcName(process pr1, process pr2)
{
    return (pr1.name.compare(pr2.name) < 0);
}

bool rcmpPrcID(process pr1, process pr2)
{
    return (pr1.pid < pr2.pid);
}

bool rcmpPrcCPU(process pr1, process pr2)
{
    return (pr1.cpuUsage < pr2.cpuUsage);
}

bool rcmpPrcRAM(process pr1, process pr2)
{
    return (pr1.ramUsage < pr2.ramUsage);
}

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
        this->sortingUpdateTimer += fElapsedtime;
        appInput();

        if (updateProcessesTimer > 10.0f)
        {
            showProcessInformation();
            updateUsageTime();
            updateRAMUsage();
            procSort();
            updateUsageTimer = 0.0f;
            updateProcessesTimer = 0.0f;
        }
        else
        {
            updateProcessesTimer += fElapsedtime;
        }

        if (updateUsageTimer > 1.0f)
        {
            updateUsageTime();
            updateRAMUsage();
            procSort();
            updateUsageTimer = 0.0f;
        }
        else
        {
            updateUsageTimer += fElapsedtime;
        }

        if (showState == FLAT)
        {
            flatProcessListShow();
        }
        else if (showState == TREE)
        {
        }

        return true;
    }

    void appInput()
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

        if (this->sortingUpdateTimer >= 0.5f)
        {
            if (m_keys[KEY_R].bHeld == true)
            {
                if (sortState == RAM_SORT)
                {
                    bigToLittle = !bigToLittle;
                }
                else
                {
                    sortState = RAM_SORT;
                }
                procSort();
                this->sortingUpdateTimer = 0;
            }

            if (m_keys[KEY_C].bHeld == true)
            {
                if (sortState == CPU_SORT)
                {
                    bigToLittle = !bigToLittle;
                }
                else
                {
                    sortState = CPU_SORT;
                }
                procSort();
                this->sortingUpdateTimer = 0;
            }

            if (m_keys[KEY_N].bHeld == true)
            {
                if (sortState == NAME_SORT)
                {
                    bigToLittle = !bigToLittle;
                }
                else
                {
                    sortState = NAME_SORT;
                }
                procSort();
                this->sortingUpdateTimer = 0;
            }

            if (m_keys[KEY_I].bHeld == true)
            {
                if (sortState == ID_SORT)
                {
                    bigToLittle = !bigToLittle;
                }
                else
                {
                    sortState = ID_SORT;
                }
                procSort();
                this->sortingUpdateTimer = 0;
            }

            if (m_keys[KEY_M].bHeld == true)
            {
                if (ramShowType == RAM_SHOW_BYTES)
                    ramShowType = RAM_SHOW_KB;
                else if (ramShowType == RAM_SHOW_KB)
                    ramShowType = RAM_SHOW_MB;
                else if (ramShowType == RAM_SHOW_MB)
                    ramShowType = RAM_SHOW_BYTES;
                this->sortingUpdateTimer = 0;
            }
        }
    }

    void flatProcessListShow()
    {
        int x = 2, y = 3;
        flatUIshow(x);
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
                COLOUR cl = FG_GREEN;
                if (this->prcs[i].cpuUsage > 25)
                {
                    cl = FG_YELLOW;
                }
                if (this->prcs[i].cpuUsage > 50)
                {
                    cl = FG_MAGENTA;
                }
                if (this->prcs[i].cpuUsage > 75)
                {
                    cl = FG_RED;
                }
                WriteString(x + xShift, y, std::to_string(this->prcs[i].cpuUsage), cl);
                xShift += 5;
                WriteString(x + xShift, y, "|");

                xShift += 5;
                cl = FG_GREEN;
                if (this->prcs[i].ramUsage / (1024 * 1024) > 100)
                {
                    cl = FG_YELLOW;
                }
                if (this->prcs[i].ramUsage / (1024 * 1024) > 500)
                {
                    cl = FG_MAGENTA;
                }
                if (this->prcs[i].ramUsage / (1024 * 1024) > 1000)
                {
                    cl = FG_RED;
                }
                size_t ram = this->prcs[i].ramUsage;
                if (ramShowType == RAM_SHOW_KB)
                    ram = ram / 1024;
                else if (ramShowType == RAM_SHOW_MB)
                    ram = ram / (1024 * 1024);
                WriteString(x + xShift, y, std::to_string(ram), cl);

                y++;
            }
        }
    }

    void flatUIshow(int x)
    {
        COLOUR cl = FG_WHITE;
        if (bigToLittle)
        {
            cl = FG_GREEN;
        }
        else
        {
            cl = FG_RED;
        }

        if (this->sortState == NAME_SORT)
            WriteString(10, 1, "PROCESS NAME", cl);
        else
            WriteString(10, 1, "PROCESS NAME");
        WriteString(maxNameSize + x, 1, "|");

        if (this->sortState == ID_SORT)
            WriteString(maxNameSize + x + 4, 1, "PID", cl);
        else
            WriteString(maxNameSize + x + 4, 1, "PID");
        WriteString(maxNameSize + x + 10, 1, "|");

        WriteString(maxNameSize + x + 12, 1, "PAR PID");
        WriteString(maxNameSize + x + 20, 1, "|");

        if (this->sortState == CPU_SORT)
            WriteString(maxNameSize + x + 24, 1, "CPU", cl);
        else
            WriteString(maxNameSize + x + 24, 1, "CPU");
        WriteString(maxNameSize + x + 30, 1, "|");

        if (this->sortState == RAM_SORT)
            WriteString(maxNameSize + x + 36, 1, "RAM", cl);
        else
            WriteString(maxNameSize + x + 36, 1, "RAM");

        // WriteString(maxNameSize + x + 40, 1, std::to_string(sortingUpdateTimer));
        FillXByText(0, 2, '-', GetWidth());
    }

    void procSort()
    {
        if (bigToLittle)
        {
            switch (sortState)
            {
            case NAME_SORT:
                std::sort(prcs.begin(), prcs.end(), cmpPrcName);
                break;
            case ID_SORT:
                std::sort(prcs.begin(), prcs.end(), cmpPrcID);
                break;
            case RAM_SORT:
                std::sort(prcs.begin(), prcs.end(), cmpPrcRAM);
                break;
            case CPU_SORT:
                std::sort(prcs.begin(), prcs.end(), cmpPrcCPU);
                break;
            }
        }
        else
        {
            switch (sortState)
            {
            case NAME_SORT:
                std::sort(prcs.begin(), prcs.end(), rcmpPrcName);
                break;
            case ID_SORT:
                std::sort(prcs.begin(), prcs.end(), rcmpPrcID);
                break;
            case RAM_SORT:
                std::sort(prcs.begin(), prcs.end(), rcmpPrcRAM);
                break;
            case CPU_SORT:
                std::sort(prcs.begin(), prcs.end(), rcmpPrcCPU);
                break;
            }
        }
    }

    void showProcessInformation()
    {
        prcs.clear();
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
        procSort();
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
    bool bigToLittle = true;
    int arrayShift = 0;
    int maxNameSize = 0;
    std::vector<process> prcs;
    FILETIME prevCPUuser, prevCPUkernel;
    float updateUsageTimer = 0;
    float updateProcessesTimer = 0;
    float sortingUpdateTimer = 0;
    int showState = FLAT;
    int sortState = RAM_SORT;
    int ramShowType = RAM_SHOW_KB;
};

int main()
{
    Application *app = new Application();
    app->ConstructConsole(120, 50);
    app->Start();

    return 1;
}