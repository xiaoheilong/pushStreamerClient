#include "globaltools.h"
#include <WinSock2.h>
#include <assert.h>
#include <stdio.h>
#include <string>
#include <memory>
#include <iostream>
#include <windows.h>
int NSSleep(int intel)  // ms
{
    HANDLE hTimer = NULL;
    LARGE_INTEGER liDueTime;

    liDueTime.QuadPart = -1 * intel * 10000;

    // Create a waitable timer.
    hTimer = CreateWaitableTimer(NULL, TRUE, L"WaitableTimer");
    if (!hTimer)
    {
        printf("CreateWaitableTimer failed (%d)\n", GetLastError());
        return 1;
    }

    if (!SetWaitableTimer(
                hTimer, &liDueTime, 0, NULL, NULL, 0))
    {
        printf("SetWaitableTimer failed (%d)\n", GetLastError());
        return 2;
    }

    // Wait for the timer.
    if (WaitForSingleObject(hTimer, INFINITE) != WAIT_OBJECT_0)
        printf("WaitForSingleObject failed (%d)\n", GetLastError());

    return 0;
}
