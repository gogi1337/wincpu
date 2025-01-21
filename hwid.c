#include <stdio.h>
#include <windows.h>
#include <wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

void get_hwid() {
    HRESULT hres;
    
    // Initialize COM
    hres = CoInitializeEx(0, COINIT_MULTITHREADED);
    if (FAILED(hres)) {
        printf("Failed to initialize COM library\n");
        return;
    }

    // Initialize security
    hres = CoInitializeSecurity(
        NULL,
        -1,
        NULL,
        NULL,
        RPC_C_AUTHN_LEVEL_DEFAULT,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE,
        NULL
    );

    if (FAILED(hres)) {
        printf("Failed to initialize security\n");
        CoUninitialize();
        return;
    }

    // Create WMI locator
    IWbemLocator *pLoc = NULL;
    hres = CoCreateInstance(
        &CLSID_WbemLocator,
        0,
        CLSCTX_INPROC_SERVER,
        &IID_IWbemLocator,
        (LPVOID *)&pLoc
    );

    if (FAILED(hres)) {
        printf("Failed to create IWbemLocator object\n");
        CoUninitialize();
        return;
    }

    // Connect to WMI
    IWbemServices *pSvc = NULL;
    hres = pLoc->lpVtbl->ConnectServer(
        pLoc,
        L"ROOT\\CIMV2",
        NULL,
        NULL,
        0,
        0,
        0,
        0,
        &pSvc
    );

    if (FAILED(hres)) {
        printf("Could not connect to WMI\n");
        pLoc->lpVtbl->Release(pLoc);
        CoUninitialize();
        return;
    }

    // Set security levels
    hres = CoSetProxyBlanket(
        (IUnknown *)pSvc,
        RPC_C_AUTHN_WINNT,
        RPC_C_AUTHZ_NONE,
        NULL,
        RPC_C_AUTHN_LEVEL_CALL,
        RPC_C_IMP_LEVEL_IMPERSONATE,
        NULL,
        EOAC_NONE
    );

    if (FAILED(hres)) {
        printf("Could not set proxy blanket\n");
        pSvc->lpVtbl->Release(pSvc);
        pLoc->lpVtbl->Release(pLoc);
        CoUninitialize();
        return;
    }

    // Query WMI for motherboard serial number
    IEnumWbemClassObject *pEnumerator = NULL;
    hres = pSvc->lpVtbl->ExecQuery(
        pSvc,
        L"WQL",
        L"SELECT * FROM Win32_ComputerSystemProduct",
        WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
        NULL,
        &pEnumerator
    );

    if (FAILED(hres)) {
        printf("Query for motherboard info failed\n");
        pSvc->lpVtbl->Release(pSvc);
        pLoc->lpVtbl->Release(pLoc);
        CoUninitialize();
        return;
    }

    // Get the data from the query
    IWbemClassObject *pclsObj = NULL;
    ULONG uReturn = 0;

    while (pEnumerator) {
        hres = pEnumerator->lpVtbl->Next(pEnumerator, WBEM_INFINITE, 1, &pclsObj, &uReturn);

        if (uReturn == 0) break;

        VARIANT vtProp;
        hres = pclsObj->lpVtbl->Get(pclsObj, L"UUID", 0, &vtProp, 0, 0);
        
        if (SUCCEEDED(hres)) {
            printf("Hardware ID (UUID): %ls\n", vtProp.bstrVal);
            VariantClear(&vtProp);
        }

        pclsObj->lpVtbl->Release(pclsObj);
    }

    // Cleanup
    pEnumerator->lpVtbl->Release(pEnumerator);
    pSvc->lpVtbl->Release(pSvc);
    pLoc->lpVtbl->Release(pLoc);
    CoUninitialize();
}