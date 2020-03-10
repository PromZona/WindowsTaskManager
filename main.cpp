#include "main.h"

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
            treeProcessListShow();
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

            if (m_keys[KEY_T].bHeld == true)
            {
                if(showState == FLAT) showState = TREE;
                else if(showState == TREE) showState = FLAT;
                procSort();
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

        //WriteString(maxNameSize + x + 42, 1, std::to_string(fathers.size()));
        FillXByText(0, 2, '-', GetWidth());
    }

    void treeProcessListShow()
    {
        int x = 2, y = 3;
        flatUIshow(x);
        for (int i = 0 + arrayShift; i < prcs.size(); i++)
        {
            if (y < GetHeight())
            {
                int columnWidth = maxNameSize;
                int xShift = 0;
                if (prcs[i].shift > 0)
                {
                    xShift += prcs[i].shift;
                    WriteString(x, y, std::string(" -", prcs[i].shift) + this->prcs[i].name);
                }
                else
                {
                    WriteString(x, y, this->prcs[i].name);
                }
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

        if (showState == TREE)
        {
            std::vector<process> buf;
            for (int i = 0; i < fathers.size(); i++)
            {
                for (int j = 0; j < prcs.size(); j++) // find father
                {
                    if (prcs[j].pid == fathers[i])
                    {
                        buf.push_back(prcs[j]);
                        break;
                    }
                }

                for (int j = 0; j < prcs.size(); j++) // find all childs
                {
                    if (prcs[j].parentProcID == fathers[i])
                    {
                        prcs[j].shift = 2;
                        buf.push_back(prcs[j]);
                    }
                }
            }
            prcs = buf;
        }
    }

    void showProcessInformation()
    {
        prcs.clear();
        fathers.clear();
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
        for (int i = 0; i < prcs.size(); i++)
        {
            for(int j = 0; j < prcs.size(); j++)
            {
                if(prcs[j].pid == prcs[i].parentProcID)
                {
                    break;
                }

                if (j == prcs.size()-1)
                {
                    fathers.push_back(prcs[i].pid);
                }
            }
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
    std::vector<int> fathers;
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
    delete app;
    return 1;
}