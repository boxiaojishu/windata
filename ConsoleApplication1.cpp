/*
 * Copyright (c) 2021-2031, 深圳市柏晓技术有限公司
 * All rights reserved.
 *
 */

#include <winsock2.h>  
#include <WS2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <iostream>
#include "Windows.h"
#include <atlstr.h>
#include <stdlib.h>
#include <tchar.h>
#pragma comment(lib, "user32.lib")
#include <Tlhelp32.h>
#pragma comment(lib, "Kernel32.lib")
#include <string>
#include <list>
#pragma comment(lib, "comsuppw.lib")
#include "psapi.h"
#include <comutil.h> 



using namespace std;

#define LOGFILE "C://desk.txt" //exe文件所在文件夹

SAFEARRAY* runtimeId = NULL;


int WriteToLog(WCHAR* str)
{
    FILE* log;
    fopen_s(&log, LOGFILE, "a+");
    if (log == NULL)
        return -1;
    fwprintf(log, L"%s \n", str);
    fclose(log);
    return 0;
}

list<string> applist;


#define BUFFER_SIZE 128

bool getDevcieInfo(char* cmd, list<string>& resultList) {
    char buffer[BUFFER_SIZE];
    bool ret = false;
    FILE* pipe = _popen(cmd, "r"); //打开管道，并执行命令 
    if (!pipe)
        return ret;

    const char* name[20] = { "UUID","ProcessorId","SerialNumber" };
    int len0 = strlen(name[0]), len1 = strlen(name[1]), len2 = strlen(name[2]);
    bool isOk = false;
    while (!feof(pipe))
    {
        if (fgets(buffer, BUFFER_SIZE, pipe))
        {
            if (strncmp(name[0], buffer, len0) == 0
                || strncmp(name[1], buffer, len1) == 0
                || strncmp(name[2], buffer, len2) == 0) // 能够正确获取信息
            {
                isOk = true;
                continue;
            }
            if (isOk == false
                || strcmp("\r\n", buffer) == 0) //去掉windows无用的空行
            {
                continue;
            }
            ret = true;
            resultList.push_back(string(buffer));
        }
    }
    _pclose(pipe); // 关闭管道 
    return ret;
}


string getDeviceFingerPrint() {
    list<string> strList;
    list<string>::iterator it;
    hash<string> str_hash;
    size_t num;
    char tmp[11] = { 0 };

    // 主板UUID存在，就使用主板UUID生成机器指纹
    if (getDevcieInfo((char*)"wmic csproduct get UUID", strList)
        && (*strList.begin()).compare("FFFFFFFF-FFFF-FFFF-FFFF-FFFFFFFFFFFF\r\n") != 0)
    {
        //cout << *strList.begin() << endl;
        cout << (*strList.begin()).substr(0, (*strList.begin()).length() - 4) << endl;

        num = str_hash(*strList.begin());
        sprintf_s(tmp, "%u", num);
        //cout << string(tmp) << endl;
        return string((*strList.begin()).substr(0, (*strList.begin()).length() - 4));
    }

    // 主板UUID不存在，使用CPUID、BIOS序列号、硬盘序列号生成机器指纹
    string otherStr("");
    strList.clear();
    if (getDevcieInfo((char*)"wmic cpu get processorid", strList)) {
        //otherStr.append((*strList.begin()).pop_back());
        otherStr.append((*strList.begin()).substr(0, (*strList.begin()).length() - 4));
        cout << *strList.begin() << endl;
    }
    cout << otherStr << endl;
    strList.clear();
    if (getDevcieInfo((char*)"wmic bios get serialnumber", strList)) {
        otherStr.append((*strList.begin()).substr(0, (*strList.begin()).length() - 9));
        cout << *strList.begin() << endl;
        cout << otherStr << endl;
    }
    strList.clear();
    if (getDevcieInfo((char*)"wmic diskdrive get serialnumber", strList)) {
        string allDiskNum("");
        // 硬盘可能有多块
        for (it = strList.begin(); it != strList.end(); it++)
        {
            allDiskNum.append(*it);
        }
        cout << *strList.begin() << endl;

        otherStr.append((*strList.begin()).substr(0, (*strList.begin()).length() - 4));
    }
    cout << otherStr << endl;

    num = str_hash(otherStr);
    sprintf_s(tmp, "%u", num);
    //cout << string(tmp) << endl;
    return string(otherStr);
}



//将宽字节wchar_t*转化为单字节char*  
inline char* UnicodeToAnsi(const wchar_t* szStr)
{
    int nLen = WideCharToMultiByte(CP_ACP, 0, szStr, -1, NULL, 0, NULL, NULL);
    if (nLen == 0)
    {
        return NULL;
    }
    char* pResult = new char[nLen];
    WideCharToMultiByte(CP_ACP, 0, szStr, -1, pResult, nLen, NULL, NULL);
    return pResult;
}

char* unicodetoutf8(const WCHAR* widestr)
{
    char* utf8str = NULL;
    int charlen = -1;
    charlen = WideCharToMultiByte(CP_UTF8, 0, widestr, -1, NULL, 0, NULL, NULL);
    utf8str = (char*)malloc(charlen);
    WideCharToMultiByte(CP_UTF8, 0, widestr, -1, utf8str, charlen, NULL, NULL);
    return utf8str;
}

int main()
{
    CoInitialize(NULL);
    std::cout << "Hello World!\n";
    WORD sockVersion = MAKEWORD(2, 2);
    WSADATA data;
    if (WSAStartup(sockVersion, &data) != 0)
    {
        return 0;
    }

    char filepath[128] = ".//base.ini";
    char buf[128] = "";
    char szTitle[] = "Base";
    GetPrivateProfileStringA(szTitle, "HostIP", "TestData", buf, sizeof(buf), filepath);
    printf("it is : %s\n",buf);
    WriteToLog((WCHAR*)buf);
    sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons(12345);
    inet_pton(AF_INET, buf, &serAddr.sin_addr);

    SOCKET sclient = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (sclient == INVALID_SOCKET)
    {
        WriteToLog((WCHAR*)L"invalid socket!");
        return 0;
    }
    if (connect(sclient, (sockaddr*)&serAddr, sizeof(serAddr)) == SOCKET_ERROR)
    {  //连接失败 
        WriteToLog((WCHAR*)L"connect error !");
        closesocket(sclient);
        //return 0;
    }
    DWORD dwPID;

    char sdata[128] = { 0 };
    sprintf_s(sdata, "deskname:%s", getDeviceFingerPrint().c_str());
    if (SOCKET_ERROR == send(sclient, sdata, strlen(sdata), 0))
    {
        WriteToLog((WCHAR*)L"send()函数deskname出错");
    }

    WriteToLog((WCHAR *)L"it is start:");
    setlocale(LC_ALL, "chinese-simplified");
    WCHAR* oldCaption = NULL;
    DWORD dwTime = 0;
    LASTINPUTINFO lpi;

    while (1)
    {
        lpi.cbSize = sizeof(lpi);
        GetLastInputInfo(&lpi);//关于此windows API接口的介绍，参见同文件夹下的文档
        dwTime = ::GetTickCount() - lpi.dwTime;
        if (dwTime >= 180000)
        {
            WriteToLog((WCHAR*)L"系统处于睡眠状态.");
            Sleep(5000);
            continue;
        }

        HWND hFocus = GetForegroundWindow();
        if (hFocus != NULL) // The Active Windows Has Change
        {
            

            WCHAR szClass[256];
            GetClassName(hFocus, szClass, sizeof(szClass) / sizeof(WCHAR));//函数调用
            
            int WinLeng = GetWindowTextLength(hFocus);
            WCHAR* WindowCaption = (WCHAR*)malloc(sizeof(WCHAR) * (WinLeng + 2));

            memset(WindowCaption, 0, sizeof(WCHAR) * (WinLeng + 2));
            GetWindowText(hFocus, (LPWSTR)WindowCaption, (WinLeng + 1));
            if (wcslen(WindowCaption) > 0)
            {
                WriteToLog(WindowCaption);
                WriteToLog(szClass);
                
                if (oldCaption != NULL)
                {
                    if (lstrcmp(WindowCaption, oldCaption) == 0) {
                        free(oldCaption);
                        oldCaption = WindowCaption;
                    }
                    else
                    {
                        free(oldCaption);

                    }
                }
                oldCaption = WindowCaption;

                CString strPath = WindowCaption;

            }
            else
            {
                Sleep(5000);
                continue;
            }

            GetWindowThreadProcessId(hFocus, &dwPID);

            HANDLE hProcess;
            HANDLE hBrowser = OpenProcess(PROCESS_ALL_ACCESS, FALSE, dwPID);  //打开一个已存在的进程对象,并返回进程的句柄，这就是我们要的进程句柄了
            char szModName[MAX_PATH];
            //int count = cbNeeded / sizeof(DWORD);
            WCHAR exePath[256];
            memset(exePath, 0, 256);
            GetModuleFileNameEx(hBrowser, 0, exePath, sizeof(szModName) * sizeof(WCHAR));
            DWORD cchSize;
            QueryFullProcessImageName(hBrowser, 0, exePath, &cchSize);

            CloseHandle(hBrowser);
            TCHAR szBuffer[1024] = { 0 };
            WCHAR exefile[256];
            _wsplitpath_s(exePath, NULL,0, NULL,0, exefile,256,NULL,0 );
			WriteToLog(exefile);
           
            wsprintf(szBuffer, _T("%s,%s"), WindowCaption, exePath);
			WriteToLog(szBuffer);
            //wprintf(L"it is 8 %s", loStrRet);
            char* temp = unicodetoutf8(szBuffer);
            //send(sclient, temp, strlen(temp), 0);
            if (SOCKET_ERROR == send(sclient, temp, strlen(temp), 0))
            {
                WriteToLog((WCHAR*)L"send()函数出错");
                free(temp);
                closesocket(sclient);
                WSACleanup();
                return 0;
            }
            free(temp);

            WriteToLog(szBuffer);

        }
        else  WriteToLog((WCHAR*)L"getforegroundwindows failed");//写入目标文件
        Sleep(5000);
    }
    closesocket(sclient);

    WSACleanup();
    CoUninitialize();
}


