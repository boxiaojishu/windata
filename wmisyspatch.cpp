/*
 * Copyright (c) 2021-2031, 深圳市柏晓技术有限公司
 * All rights reserved.
 *
 */

#include <iostream>
#include <comdef.h>
#include <Wbemidl.h>
#include <wuapi.h>
#include <ATLComTime.h>
#include <wuerror.h>
#include <atlbase.h>
#include <atlconv.h>

using namespace std;


#pragma comment(lib, "wbemuuid.lib")

int getupdatepatchlist(TCHAR* ppatchlist)
{
    HRESULT hres;

    // 第一步：初始化COM
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres))
    {
        cout << "Failed to initialize COM library. Error code = 0x"
            << hex << hres << endl;
        return 1;                  // Program has failed.
    }

    // 第二步：设置COM安全级别
    hres = CoInitializeSecurity(
        NULL,
        -1,                          // COM 认证
        NULL,                        // 服务认证
        NULL,                        // 保留NULL
        RPC_C_AUTHN_LEVEL_DEFAULT,   // 默认权限
        RPC_C_IMP_LEVEL_IMPERSONATE, // 默认模拟
        NULL,                        // 认证信息
        EOAC_NONE,                   // Additional capabilities
        NULL                         // Reserved
    );


    if (FAILED(hres))
    {
        cout << "安全级别初始化失败，错误代码 = 0x" << hex << hres << endl;
        CoUninitialize();
        return 1;                    // Program has failed.
    }

    // 第三步：获取初始化本地WMI
    IWbemLocator* pLoc = NULL;

    hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);

    if (FAILED(hres))
    {
        cout << "创建IWbemLocator对象失败，错误代码 = 0x" << hex << hres << endl;
        CoUninitialize();
        return 1;                 // Program has failed.
    }

    // 第四步：通过 IWbemLocator::ConnectServer 方法连接WMI
    IWbemServices* pSvc = NULL;

    // 使用IWbemServices 连接 root\cimv2 命名空间
    hres = pLoc->ConnectServer(
        _bstr_t(L"ROOT\\CIMV2"), // 对象路径
        NULL,                    // 用户名为空默认当前用户
        NULL,                    // 用户密码为空默认当前密码
        0,                       // 本地，NULL表示当前
        NULL,                    // 安全标志
        0,                       // 授权人
        0,                       // 上下文对象
        &pSvc                    // IWbemServices代理指针
    );

    if (FAILED(hres))
    {
        cout << "无法连接，错误代码 = 0x" << hex << hres << endl;
        pLoc->Release();
        CoUninitialize();
        return 1;                // Program has failed.
    }

    cout << "已连接到 ROOT\\CIMV2 WMI 命名空间" << endl;


    // 第五步：设置代理安全级别

    hres = CoSetProxyBlanket(
        pSvc,                        // 要设置的代理指针
        RPC_C_AUTHN_WINNT,           // RPC_C_AUTHN_xxx
        RPC_C_AUTHZ_NONE,            // RPC_C_AUTHZ_xxx
        NULL,                        // 委托服务名
        RPC_C_AUTHN_LEVEL_CALL,      // RPC_C_AUTHN_LEVEL_xxx
        RPC_C_IMP_LEVEL_IMPERSONATE, // RPC_C_IMP_LEVEL_xxx
        NULL,                        // 客户端身份
        EOAC_NONE                    // 代理能力
    );

    if (FAILED(hres))
    {
        cout << "代理设置失败，错误代码 = 0x" << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 1;               // Program has failed.
    }

    // 第六步：使用 IWbemServices 指针获取系统名
    IEnumWbemClassObject* pEnumerator = NULL;
    hres = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_QuickFixEngineering"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

    if (FAILED(hres))
    {
        cout << "查询系统名失败，错误代码 = 0x" << hex << hres << endl;
        pSvc->Release();
        pLoc->Release();
        CoUninitialize();
        return 1;               // Program has failed.
    }

    // 第七步：获取查询数据
    IWbemClassObject* pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator)
    {
        HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

        if (0 == uReturn)
        {
            break;
        }

        VARIANT vtProp;

        // 获取Name属性值
        hr = pclsObj->Get(L"Caption", 0, &vtProp, 0, 0);

        _snwprintf_s(ppatchlist, 102400,102400, L"%s,%s", ppatchlist, vtProp.bstrVal);
        VariantClear(&vtProp);

       // VARIANT vtProp;

        // 获取Name属性值
        hr = pclsObj->Get(L"CSName", 0, &vtProp, 0, 0);

        _snwprintf_s(ppatchlist, 102400, 102400,L"%s,%s", ppatchlist, vtProp.bstrVal);
       // printf("string type  %d\n", vtProp.vt);
        VariantClear(&vtProp);

        // 获取Name属性值
        hr = pclsObj->Get(L"Description", 0, &vtProp, 0, 0);

        _snwprintf_s(ppatchlist, 102400, 102400,L"%s,%s", ppatchlist, vtProp.bstrVal);
        VariantClear(&vtProp);

        // 获取Name属性值
        hr = pclsObj->Get(L"FixComments", 0, &vtProp, 0, 0);

        _snwprintf_s(ppatchlist, 102400, 102400,L"%s,%s", ppatchlist, vtProp.bstrVal);
        VariantClear(&vtProp);

        // 获取Name属性值
        hr = pclsObj->Get(L"HotFixID", 0, &vtProp, 0, 0);
        _snwprintf_s(ppatchlist, 102400, 102400,L"%s,%s", ppatchlist, vtProp.bstrVal);

        VariantClear(&vtProp);



        // 获取Name属性值
        hr = pclsObj->Get(L"InstalledBy", 0, &vtProp, 0, 0);

        _snwprintf_s(ppatchlist, 102400, 102400,L"%s,%s", ppatchlist, vtProp.bstrVal);
        VariantClear(&vtProp);

        // 获取Name属性值
        hr = pclsObj->Get(L"InstalledOn", 0, &vtProp, 0, 0);

        _snwprintf_s(ppatchlist, 102400, 102400,L"%s,%s", ppatchlist, vtProp.bstrVal);
        VariantClear(&vtProp);

        // 获取Name属性值
        hr = pclsObj->Get(L"ServicePackInEffect", 0, &vtProp, 0, 0);
       // wcout << " ServicePackInEffect : " << vtProp.bstrVal << endl;
        _snwprintf_s(ppatchlist, 102400, 102400,L"%s,%s;", ppatchlist, vtProp.bstrVal);
        VariantClear(&vtProp);

        pclsObj->Release();
    }

    // 清理工作
    pSvc->Release();
    pLoc->Release();
    pEnumerator->Release();
    CoUninitialize();

    return 0;   // Program successfully completed.

}
