#pragma once

#include <iostream>
#include "Dronegine.h"
#include <string>
#include <tchar.h>
#include <tlhelp32.h>
#include <Psapi.h>
#include <pdh.h>
#include <vector>
#include <algorithm>
#include <map>

#define KEY_UP 0x26
#define KEY_DOWN 0x28
#define KEY_LEFT 0x25
#define KEY_RIGHT 0x27
#define KEY_C 0x43
#define KEY_I 0x49
#define KEY_N 0x4E
#define KEY_R 0x52
#define KEY_M 0x4D
#define KEY_T 0x54

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
    int shift = 0;
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