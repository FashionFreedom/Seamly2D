#if (_MSC_VER == 1200)
  // Visual Studio 6: forced WINNT version needed to use IsDebuggerPresent()
  #define _WIN32_WINNT 0x400
#endif

#include <rpc.h>
#include <rpcndr.h>
#include "Thinfinity.VirtualUI.h"
#include <windows.h>
#include <Shlwapi.h>

HINSTANCE VirtualUILibrary::LibHandle = NULL;


HINSTANCE VirtualUILibrary::GetHandle() {
    if (LibHandle == NULL) {
        std::wstring TargetDir = GetDLLPath();
        if (TargetDir != L"") {
            std::wstring LibFilename = TargetDir + std::wstring(L"\\Thinfinity.VirtualUI.dll");
            LibHandle = LoadLibraryW(LibFilename.c_str());
        }
    }
    return LibHandle;
}

std::wstring VirtualUILibrary::GetDLLPath() {
    HKEY hKey;
    std::wstring keybase1(L"SOFTWARE\\Wow6432Node\\Cybele Software\\Setups\\Thinfinity\\VirtualUI");
    std::wstring keybase2(L"SOFTWARE\\Cybele Software\\Setups\\Thinfinity\\VirtualUI");
    std::wstring aux;
    boolean keyFound = false;
    if (IsDebuggerPresent()) {
        aux = L"\\Dev";
        keybase1 += aux;
        keybase2 += aux;
    }
    else {
		WCHAR szFileName[MAX_PATH];
        GetModuleFileNameW(NULL, szFileName, MAX_PATH);
		PathRemoveFileSpecW(szFileName);
		StrCatW(szFileName, L"\\OEM.ini");
		if (PathFileExistsW(szFileName)) {
			WCHAR szOEMKey32[MAX_PATH];
			WCHAR szOEMKey64[MAX_PATH];
            GetPrivateProfileStringW (L"PATHS",
                                      L"Key32",
									  NULL,
                                      szOEMKey32,
                                      MAX_PATH,
									  szFileName);
            keybase1 = szOEMKey32;
			keybase1 = keybase1.substr(1);
            GetPrivateProfileStringW (L"PATHS",
                                      L"Key64",
									  NULL,
                                      szOEMKey64,
                                      MAX_PATH,
									  szFileName);
            keybase2 = szOEMKey64;
			keybase2 = keybase2.substr(1);
            aux = L"";
        }
    }

    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, (LPCWSTR)keybase1.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        aux = keybase1;
        keyFound = true;
    }
    else
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, (LPCWSTR)keybase2.c_str(), 0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        aux = keybase2;
        keyFound = true;
    }

    if (keyFound)
    {
        ULONG dataType = 0x00000002;  //RRF_RT_REG_SZ
        ULONG chars = 512;
        char dst[512];
#ifdef _WIN64
        if (RegQueryValueExW(hKey, L"TargetDir_x64", NULL, &dataType, (LPBYTE)dst, &chars) == ERROR_SUCCESS)
#else
        if (RegQueryValueExW(hKey, L"TargetDir_x86", NULL, &dataType, (LPBYTE)dst, &chars) == ERROR_SUCCESS)
#endif
        {
			RegCloseKey(hKey);
			return std::wstring((wchar_t *)&dst);
        }
        RegCloseKey(hKey);
    };
    return std::wstring(L"");
}




VirtualUI::VirtualUI() {
	m_refcount = 0;
	m_VirtualUI = NULL;
	m_ClientSettings = NULL;
	m_BrowserInfo = NULL;
	m_DevServer = NULL;
	m_Recorder = NULL;
	m_FileSystemFilter = NULL;
	m_RegistryFilter = NULL;
	m_HTMLDoc = NULL;

	OnBrowserResize = NULL;
	OnGetUploadDir = NULL;
	OnClose = NULL;
	OnReceiveMessage = NULL;
	OnDownloadEnd = NULL;
	OnUploadEnd = NULL;
	OnRecorderChanged = NULL;
	OnDragFile = NULL;
	OnSaveDialog = NULL;
	OnDragFile2 = NULL;
	OnDropFile = NULL;


	m_VirtualUILib = new VirtualUILibrary();
	HINSTANCE LibHandle = m_VirtualUILib->GetHandle();

	if (LibHandle != 0) {
        typedef HRESULT(__stdcall *DllGetInstance)(IVirtualUI**);
        DllGetInstance GetInstance = (DllGetInstance)GetProcAddress(LibHandle, "DllGetInstance");
        GetInstance(&m_VirtualUI);
    }

	m_EventSink = new VirtualUISink(this);
}

VirtualUI::~VirtualUI()
{
	if (m_VirtualUI != NULL) {
		m_VirtualUI->Release();
	}
	m_EventSink = NULL;
	m_ClientSettings = NULL;
	m_VirtualUI = NULL;
	m_BrowserInfo = NULL;
	m_DevServer = NULL;
	m_Recorder = NULL;
	m_FileSystemFilter = NULL;
	m_RegistryFilter = NULL;
	m_HTMLDoc = NULL;
	m_VirtualUILib = NULL;
}

HRESULT __stdcall VirtualUI::GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->GetTypeInfo(Index, LocaleID, TypeInfo);
    else {
        TypeInfo = NULL;
        return E_NOTIMPL;
    }
}

HRESULT __stdcall VirtualUI::GetTypeInfoCount(UINT *Count) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->GetTypeInfoCount(Count);
    else {
        Count = 0;
        return 0;
    }
}

HRESULT __stdcall VirtualUI::GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->GetIDsOfNames(IID, Names, NameCount, LocaleID, DispIDs);
    else {
        return E_NOTIMPL;
    }
}

HRESULT __stdcall VirtualUI::Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->Invoke(DispID, IID, LocaleID, Flags, Params, VarResult, ExcepInfo, ArgErr);
    else {
        return E_NOTIMPL;
    }
}

ULONG __stdcall VirtualUI::AddRef() {
    return (ULONG)InterlockedIncrement(&m_refcount);
}

ULONG __stdcall VirtualUI::Release() {
    ULONG count = InterlockedDecrement(&m_refcount);
    if (count == 0) {
        delete this;
    }
    return count;
}

HRESULT __stdcall VirtualUI::QueryInterface(REFIID riid, void **ppvObject) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->QueryInterface(riid, ppvObject);
    else {
        return E_NOINTERFACE;
    }
}

HRESULT __stdcall VirtualUI::Start(long Timeout, VARIANT_BOOL *OutRetVal) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->Start(Timeout, OutRetVal);
    return S_OK;
}

HRESULT __stdcall VirtualUI::Stop(void) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->Stop();
    return S_OK;
};

HRESULT __stdcall VirtualUI::get_Active(VARIANT_BOOL *Value) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->get_Active(Value);
    else {
        *Value = 0;
    }
    return S_OK;
}

HRESULT __stdcall VirtualUI::get_Enabled(VARIANT_BOOL *Value) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->get_Enabled(Value);
    else {
        *Value = 0;
    }
    return S_OK;
}

HRESULT __stdcall VirtualUI::put_Enabled(VARIANT_BOOL Value) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->put_Enabled(Value);
    return S_OK;
}

HRESULT __stdcall VirtualUI::get_DevMode(VARIANT_BOOL *Value) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->get_DevMode(Value);
    else {
        *Value = 0;
    }
    return S_OK;
}

HRESULT __stdcall VirtualUI::put_DevMode(VARIANT_BOOL Value) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->put_DevMode(Value);
    return S_OK;
}

HRESULT __stdcall VirtualUI::get_StdDialogs(VARIANT_BOOL *Value) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->get_StdDialogs(Value);
    else {
        *Value = 0;
    }
    return S_OK;
}

HRESULT __stdcall VirtualUI::put_StdDialogs(VARIANT_BOOL Value) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->put_StdDialogs(Value);
    return S_OK;
}

HRESULT __stdcall VirtualUI::get_BrowserInfo(IBrowserInfo **Value) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->get_BrowserInfo(Value);
    else {
        return S_OK;
    }
}

HRESULT __stdcall VirtualUI::get_DevServer(IDevServer **Value) {
    return S_OK;
}

HRESULT __stdcall VirtualUI::get_ClientSettings(IClientSettings **Value) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->get_ClientSettings(Value);
    else {
        return S_OK;
    }
}

HRESULT __stdcall VirtualUI::DownloadFile(BSTR LocalFilename, BSTR RemoteFilename, BSTR MimeType) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->DownloadFile(LocalFilename, RemoteFilename, MimeType);
    return S_OK;
}

HRESULT __stdcall VirtualUI::PrintPdf(BSTR AFileName) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->PrintPdf(AFileName);
    return S_OK;
}

HRESULT __stdcall VirtualUI::PreviewPdf(BSTR AFileName) {
	if (m_VirtualUI != NULL)
		return m_VirtualUI->PreviewPdf(AFileName);
	return S_OK;
}

HRESULT __stdcall VirtualUI::OpenLinkDlg(BSTR url, BSTR caption) {
    if (m_VirtualUI != NULL)
        m_VirtualUI->OpenLinkDlg(url, caption);
    return S_OK;
}

HRESULT __stdcall VirtualUI::SendMessage(BSTR Data) {
    if (m_VirtualUI != NULL)
        m_VirtualUI->SendMessage(Data);
    return S_OK;
}

HRESULT __stdcall VirtualUI::AllowExecute(BSTR Filename) {
    if (m_VirtualUI != NULL)
        m_VirtualUI->AllowExecute(Filename);
    return S_OK;
}

HRESULT __stdcall VirtualUI::SetImageQualityByWnd(long Wnd, BSTR Class, long Quality) {
    if (m_VirtualUI != NULL)
        m_VirtualUI->SetImageQualityByWnd(Wnd, Class, Quality);
    return S_OK;
}

HRESULT __stdcall VirtualUI::Uploadfile(BSTR ServerDirectory) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->Uploadfile(ServerDirectory);
    return S_OK;
}

HRESULT __stdcall VirtualUI::UploadFileEx(BSTR ServerDirectory, BSTR* FileName, VARIANT_BOOL *OutRetVal) {
	if (m_VirtualUI != NULL)
		return m_VirtualUI->UploadFileEx(ServerDirectory, FileName, OutRetVal);
	return S_OK;
}

HRESULT __stdcall VirtualUI::Suspend() {
	if (m_VirtualUI != NULL)
		return m_VirtualUI->Suspend();
    return S_OK;
}

HRESULT __stdcall VirtualUI::Resume() {
	if (m_VirtualUI != NULL)
		return m_VirtualUI->Resume();
	return S_OK;
}

HRESULT __stdcall VirtualUI::FlushWindow(long Wnd, VARIANT_BOOL *OutRetVal) {
    if (m_VirtualUI != NULL)
		return m_VirtualUI->FlushWindow(Wnd, OutRetVal);
	return S_OK;
};

HRESULT __stdcall VirtualUI::TakeScreenshot(long Wnd, BSTR FileName, VARIANT_BOOL *OutRetVal) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->TakeScreenshot(Wnd, FileName, OutRetVal);
    return S_OK;
}

HRESULT __stdcall VirtualUI::ShowVirtualKeyboard(void) {
	if (m_VirtualUI != NULL)
		return m_VirtualUI->ShowVirtualKeyboard();
	return S_OK;
}

HRESULT __stdcall VirtualUI::get_Recorder(IRecorder **Value) {
	if (m_VirtualUI != NULL)
		return m_VirtualUI->get_Recorder(Value);
	else {
		return S_OK;
	}
}

HRESULT __stdcall VirtualUI::get_FileSystemFilter(IFileSystemFilter **Value) {
	if (m_VirtualUI != NULL)
		return m_VirtualUI->get_FileSystemFilter(Value);
	else {
		return S_OK;
	}
}

HRESULT __stdcall VirtualUI::get_RegistryFilter(IRegistryFilter **Value) {
	if (m_VirtualUI != NULL)
		return m_VirtualUI->get_RegistryFilter(Value);
	else {
		return S_OK;
	}
}

HRESULT __stdcall VirtualUI::Get_Options(long *Value) {
	if (m_VirtualUI != NULL)
		return m_VirtualUI->Get_Options(Value);
	else {
		return S_OK;
	}
}

HRESULT __stdcall VirtualUI::Set_Options(long Value) {
	if (m_VirtualUI != NULL)
		return m_VirtualUI->Set_Options(Value);
	else {
		return S_OK;
	}
}

HRESULT __stdcall VirtualUI::Get_HTMLDoc(IHTMLDoc **Value) {
	if (m_VirtualUI != NULL)
		return m_VirtualUI->Get_HTMLDoc(Value);
	else {
		return S_OK;
	}
}

HRESULT __stdcall VirtualUI::Get_Performance(IPerformance** OutRetVal) {
    if (m_VirtualUI != NULL)
        return m_VirtualUI->Get_Performance(OutRetVal);
    else {
        return S_OK;
    }
}

bool VirtualUI::Start() {
    return Start(60000);
}

bool VirtualUI::Start(int Timeout) {
    VARIANT_BOOL ret = 0;
    this->Start(Timeout, &ret);
    return (ret != 0);
}

CClientSettings* VirtualUI::ClientSettings() {
	if (m_ClientSettings == NULL) {
		m_ClientSettings = new CClientSettings(m_VirtualUI);
	}
	return m_ClientSettings;
}

CBrowserInfo* VirtualUI::BrowserInfo() {
	if (m_BrowserInfo == NULL) {
		m_BrowserInfo = new CBrowserInfo(m_VirtualUI);
	}
	return m_BrowserInfo;
}

CDevServer* VirtualUI::DevServer() {
	if (m_DevServer == NULL) {
		m_DevServer = new CDevServer(m_VirtualUI);
	}
	return m_DevServer;
}

void VirtualUI::DownloadFile(std::wstring LocalFilename) {
    DownloadFile(LocalFilename, std::wstring(L""), std::wstring(L""));
}

void VirtualUI::DownloadFile(std::wstring LocalFilename, std::wstring RemoteFilename) {
    DownloadFile(LocalFilename, RemoteFilename, std::wstring(L""));
}

void VirtualUI::DownloadFile(std::wstring LocalFilename, std::wstring RemoteFilename, std::wstring MimeType) {
    BSTR localFileName =  SysAllocString(LocalFilename.c_str());
    BSTR remoteFileName = SysAllocString(RemoteFilename.c_str());
    BSTR mimeType = SysAllocString(MimeType.c_str());
    this->DownloadFile(localFileName, remoteFileName, mimeType);
    SysFreeString(localFileName);
    SysFreeString(remoteFileName);
    SysFreeString(mimeType);
}

void VirtualUI::Uploadfile(std::wstring ServerDirectory) {
    BSTR serverDir = SysAllocString(ServerDirectory.c_str());
    this->Uploadfile(serverDir);
    SysFreeString(serverDir);
}

void VirtualUI::Uploadfile() {
    Uploadfile(std::wstring(L""));
}

bool VirtualUI::UploadFileEx(std::wstring ServerDirectory, std::wstring &FileName) {
	BSTR serverDir = SysAllocString(ServerDirectory.c_str());
	BSTR fName;
	VARIANT_BOOL ret = 0;
	this->UploadFileEx(serverDir, &fName, &ret);
	FileName = std::wstring(fName);
	SysFreeString(fName);
	SysFreeString(serverDir);
	return (ret != 0);
}

bool VirtualUI::UploadFileEx(std::wstring &FileName) {
	return UploadFileEx(std::wstring(L""), FileName);
}

bool VirtualUI::TakeScreenshot(HWND Wnd, std::wstring FileName) {
    VARIANT_BOOL ret = 0;
    BSTR fileName = SysAllocString(FileName.c_str());
#if defined(__MINGW64__)
    long lWnd = (INT_PTR)Wnd;
#else
    long lWnd = (long)Wnd;
#endif
    this->TakeScreenshot(lWnd, fileName, &ret);
    SysFreeString(fileName);
    return (ret != 0);
}

void VirtualUI::PrintPdf(std::wstring FileName) {
    BSTR fileName = SysAllocString(FileName.c_str());
    this->PrintPdf(fileName);
    SysFreeString(fileName);
}

void VirtualUI::PreviewPdf(std::wstring FileName) {
	BSTR fileName = SysAllocString(FileName.c_str());
	this->PreviewPdf(fileName);
	SysFreeString(fileName);
}

void VirtualUI::OpenLinkDlg(std::wstring Url, std::wstring Caption) {
    BSTR url = SysAllocString(Url.c_str());
    BSTR caption = SysAllocString(Caption.c_str());
    this->OpenLinkDlg(url, caption);
    SysFreeString(url);
    SysFreeString(caption);
}

void VirtualUI::SendMessage(std::wstring Data) {
    BSTR data = SysAllocString(Data.c_str());
    this->SendMessage(data);
    SysFreeString(data);
}

void VirtualUI::AllowExecute(std::wstring Filename) {
    BSTR fileName = SysAllocString(Filename.c_str());
    this->AllowExecute(fileName);
    SysFreeString(fileName);
}

void VirtualUI::SetImageQualityByWnd(long Wnd, std::wstring Classname, long Quality) {
    BSTR classname = SysAllocString(Classname.c_str());
    this->SetImageQualityByWnd(Wnd, classname, Quality);
    SysFreeString(classname);
}

bool VirtualUI::Active() {
    VARIANT_BOOL ret = 0;
    this->get_Active(&ret);
    return (ret != 0);
}

bool VirtualUI::Enabled() {
    VARIANT_BOOL ret = 0;
    this->get_Enabled(&ret);
    return (ret != 0);
}

void VirtualUI::Enabled(bool value) {
    VARIANT_BOOL enabled = value ? -1 : 0;
    this->put_Enabled(enabled);
}

bool VirtualUI::DevMode() {
    VARIANT_BOOL ret = 0;
    this->get_DevMode(&ret);
    return (ret != 0);
}

void VirtualUI::DevMode(bool value) {
    VARIANT_BOOL enabled = value ? -1 : 0;
    this->put_DevMode(enabled);
}

bool VirtualUI::StdDialogs() {
    VARIANT_BOOL ret = 0;
    this->get_StdDialogs(&ret);
    return (ret != 0);
}

void VirtualUI::StdDialogs(bool value){
    VARIANT_BOOL enabled = value ? -1 : 0;
    this->put_StdDialogs(enabled);
}

CRecorder* VirtualUI::Recorder() {
	if (m_Recorder == NULL) {
		m_Recorder = new CRecorder(m_VirtualUI);
	}
	return m_Recorder;
}

CFileSystemFilter* VirtualUI::FileSystemFilter() {
	if (m_FileSystemFilter == NULL) {
		m_FileSystemFilter = new CFileSystemFilter(m_VirtualUI);
	}
	return m_FileSystemFilter;
}

CRegistryFilter* VirtualUI::RegistryFilter() {
	if (m_RegistryFilter == NULL) {
		m_RegistryFilter = new CRegistryFilter(m_VirtualUI);
	}
	return m_RegistryFilter;
}

long VirtualUI::Options() {
	long res;
	this->Get_Options(&res);
	return res;
}

void VirtualUI::Options(long Value) {
	this->Set_Options(Value);
}

CHTMLDoc* VirtualUI::HTMLDoc() {
	if (m_HTMLDoc == NULL) {
		m_HTMLDoc = new CHTMLDoc(m_VirtualUI);
	}
	return m_HTMLDoc;
}




VirtualUISink::VirtualUISink(VirtualUI* virtualUI) {
    m_VirtualUI = virtualUI;
    m_refcount = 0;
	m_pIConnectionPoint = NULL;
    IConnectionPointContainer* pIConnectionPointContainerTemp = NULL;
    IUnknown* pIUnknown = NULL;

    this->QueryInterface(IID_IUnknown, (void**)&pIUnknown);
    if (pIUnknown) {
        m_VirtualUI->QueryInterface(IID_IConnectionPointContainer, (void**)&pIConnectionPointContainerTemp);

        if (pIConnectionPointContainerTemp) {
#if defined(__MINGW32__) || defined(__MINGW64__)
            pIConnectionPointContainerTemp ->FindConnectionPoint(IID_IEvents, &m_pIConnectionPoint);
#else
            pIConnectionPointContainerTemp ->FindConnectionPoint(__uuidof(IEvents), &m_pIConnectionPoint);
#endif
            pIConnectionPointContainerTemp->Release();
            pIConnectionPointContainerTemp = NULL;
        }

        if (m_pIConnectionPoint) {
            m_pIConnectionPoint->Advise(pIUnknown, &m_dwEventCookie);
        }

        pIUnknown->Release();
        pIUnknown = NULL;
    }
}

VirtualUISink::~VirtualUISink() {
    if (m_pIConnectionPoint)
    {
        m_pIConnectionPoint->Unadvise(m_dwEventCookie);
        m_dwEventCookie = 0;
        m_pIConnectionPoint->Release();
        m_pIConnectionPoint = NULL;
    }
}

HRESULT __stdcall VirtualUISink::GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo) {
    *TypeInfo = NULL;
    return E_NOTIMPL;
}

HRESULT __stdcall VirtualUISink::GetTypeInfoCount(UINT *Count) {
    *Count = 0;
    return E_NOTIMPL;
}

HRESULT __stdcall VirtualUISink::GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs) {
    return E_NOTIMPL;
}

HRESULT __stdcall VirtualUISink::Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr) {
    if (DispID == 101) {
        BSTR *directory = GetAsBSTRRef(Params, 1);
        VARIANT_BOOL *handled = GetAsVariantBoolRef(Params, 0);
        OnGetUploadDir(directory, handled);
    }
    if (DispID == 102) {
        long *Width = GetAsIntegerRef(Params, 2);
        long *Height = GetAsIntegerRef(Params, 1);
        VARIANT_BOOL *ResizeMaximized = GetAsVariantBoolRef(Params, 0);
        OnBrowserResize(Width, Height, ResizeMaximized);
    }
    if (DispID == 103) {
        OnClose();
    }
    if (DispID == 104) {
        BSTR data = GetAsBSTR(Params, 0);
        OnReceiveMessage(&data);
    }
    if (DispID == 105) {
        BSTR data = GetAsBSTR(Params, 0);
        OnDownloadEnd(&data);
    }
	if (DispID == 106) {
		OnRecorderChanged();
	}
	if (DispID == 107) {
		BSTR data = GetAsBSTR(Params, 0);
		OnUploadEnd(&data);
	}
	if (DispID == 201) {
		long action = GetAsInteger(Params, 3);
		long x = GetAsInteger(Params, 2);
		long y = GetAsInteger(Params, 1);
		BSTR fileNames = GetAsBSTR(Params, 0);
		OnDragFile((DragAction)action, x, y, fileNames);
	}
	if (DispID == 202) {
		BSTR fileName = GetAsBSTR(Params, 0);
		OnSaveDialog(fileName);
	}
	if (DispID == 203) {
		long action = GetAsInteger(Params, 4);
		long screenX = GetAsInteger(Params, 3);
		long screenY = GetAsInteger(Params, 2);
		BSTR fileNames = GetAsBSTR(Params, 1);
		VARIANT_BOOL *Accept = GetAsVariantBoolRef(Params, 0);
		OnDragFile2((DragAction)action, screenX, screenY, fileNames, Accept);
	}
	if (DispID == 204) {
        long screenX = GetAsInteger(Params, 4);
        long screenY = GetAsInteger(Params, 3);
        BSTR fileNames = GetAsBSTR(Params, 2);
        BSTR fileSizes = GetAsBSTR(Params, 1);
        BSTR* ignoreFiles = GetAsBSTRRef(Params, 0);
        OnDropFile(screenX, screenY, fileNames, fileSizes, ignoreFiles);
	}
	return S_OK;
}

ULONG __stdcall VirtualUISink::AddRef() {
    return (ULONG)InterlockedIncrement(&m_refcount);
}

ULONG __stdcall VirtualUISink::Release() {
    ULONG count = InterlockedDecrement(&m_refcount);
    if (count == 0) {
        delete this;
    }
    return count;
}

HRESULT __stdcall VirtualUISink::QueryInterface(REFIID riid, void **ppvObject) {
    *ppvObject = NULL;
    if (riid == IID_IUnknown) {
        *ppvObject = this;
        this->AddRef();
    }
#if defined(__MINGW32__) || defined(__MINGW64__)
    if ((riid == IID_IDispatch) || (riid == IID_IEvents))
#else
    if ((riid == IID_IDispatch) || (riid == __uuidof(IEvents)))
#endif
    {
        *ppvObject = (IDispatch*)this;
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

BSTR VirtualUISink::GetAsBSTR(DISPPARAMS* args, int index) {
    if (V_VT(&(args->rgvarg)[index]) == VT_BSTR)
        return V_BSTR(&(args->rgvarg)[index]);
    return NULL;
}

BSTR* VirtualUISink::GetAsBSTRRef(DISPPARAMS* args, int index) {
    if (V_VT(&(args->rgvarg)[index]) == (VT_BYREF | VT_BSTR))
        return (V_BSTRREF(&(args->rgvarg)[index]));
    return NULL;
}

VARIANT_BOOL VirtualUISink::GetAsVariantBool(DISPPARAMS* args, int index) {
    if (V_VT(&(args->rgvarg)[index]) == VT_BOOL)
        return V_BOOL(&(args->rgvarg)[index]);
    return 0;
}

VARIANT_BOOL* VirtualUISink::GetAsVariantBoolRef(DISPPARAMS* args, int index) {
    if (V_VT(&(args->rgvarg)[index]) == (VT_BYREF | VT_BOOL))
        return V_BOOLREF(&(args->rgvarg)[index]);
    return 0;
}

long VirtualUISink::GetAsInteger(DISPPARAMS* args, int index) {
    if (V_VT(&(args->rgvarg)[index]) == VT_I4)
        return V_I4(&(args->rgvarg)[index]);
    return 0;
}

long* VirtualUISink::GetAsIntegerRef(DISPPARAMS* args, int index) {
    if (V_VT(&(args->rgvarg)[index]) == (VT_BYREF | VT_I4))
        return V_I4REF(&(args->rgvarg)[index]);
    return 0;
}

IDispatch* VirtualUISink::GetAsInterface(DISPPARAMS* args, int index) {
	if (V_VT(&(args->rgvarg)[index]) == VT_DISPATCH)
		return V_DISPATCH(&(args->rgvarg)[index]);
	if (V_VT(&(args->rgvarg)[index]) == (VT_BYREF | VT_DISPATCH))
		return (args->rgvarg)[index].pdispVal;
	return NULL;
}

HRESULT __stdcall VirtualUISink::OnGetUploadDir(BSTR *Directory, VARIANT_BOOL* Handled) {
    if (m_VirtualUI->OnGetUploadDir) {
        std::wstring dir = *Directory;
        bool h = (*Handled != 0);
        m_VirtualUI->OnGetUploadDir(dir, h);
        *Handled = (h ? -1 : 0);
        SysReAllocString(Directory, dir.c_str());
    }
    return S_OK;
}

HRESULT __stdcall VirtualUISink::OnBrowserResize(long* Width, long* Height, VARIANT_BOOL* ResizeMaximized) {
    if (m_VirtualUI->OnBrowserResize) {
        bool resMax = (*ResizeMaximized != 0);
        int w = *Width;
        int h = *Height;
        m_VirtualUI->OnBrowserResize(w, h, resMax);
        *Width = w;
        *Height = h;
        *ResizeMaximized = (resMax ? -1 : 0);
    }
    return S_OK;
}

HRESULT __stdcall VirtualUISink::OnClose(void) {
	if (m_VirtualUI->OnClose) {
		m_VirtualUI->OnClose();
	}
	return S_OK;
}

HRESULT __stdcall VirtualUISink::OnReceiveMessage(BSTR *Data) {
    if (m_VirtualUI->OnReceiveMessage) {
        std::wstring data = *Data;
        m_VirtualUI->OnReceiveMessage(data);
    }
    return S_OK;
}

HRESULT __stdcall VirtualUISink::OnDownloadEnd(BSTR *Filename) {
	if (m_VirtualUI->OnDownloadEnd) {
		std::wstring fName = *Filename;
		m_VirtualUI->OnDownloadEnd(fName);
	}
	return S_OK;
}

HRESULT __stdcall VirtualUISink::OnUploadEnd(BSTR *Filename) {
	if (m_VirtualUI->OnUploadEnd) {
		std::wstring fName = *Filename;
		m_VirtualUI->OnUploadEnd(fName);
	}
	return S_OK;
}

HRESULT __stdcall VirtualUISink::OnRecorderChanged(void) {
	if (m_VirtualUI->OnRecorderChanged) {
		m_VirtualUI->OnRecorderChanged();
	}
	return S_OK;
}

HRESULT __stdcall VirtualUISink::OnDragFile(DragAction Action, long X, long Y, BSTR Filenames) {
	if (m_VirtualUI->OnDragFile) {
		std::wstring fName;
		if (Filenames == NULL)
			fName = L"";
		else {
			fName = Filenames;
		}
		m_VirtualUI->OnDragFile(Action, X,  Y, fName);
	}
	return S_OK;
}

HRESULT __stdcall VirtualUISink::OnSaveDialog(BSTR Filename) {
	if (m_VirtualUI->OnSaveDialog) {
		std::wstring fName;
		if (Filename == NULL)
			fName = L"";
		else {
			fName = Filename;
		}
		m_VirtualUI->OnSaveDialog(fName);
	}
	return S_OK;
}

HRESULT __stdcall VirtualUISink::OnDragFile2(DragAction Action, long ScreenX, long ScreenY, BSTR Filenames, VARIANT_BOOL* Accept) {
	if (m_VirtualUI->OnDragFile2) {
		std::wstring fName;
		if (Filenames == NULL)
			fName = L"";
		else {
			fName = Filenames;
		}
		bool accept = (*Accept != 0);
		m_VirtualUI->OnDragFile2(Action, ScreenX, ScreenY, fName, accept);
		*Accept = (accept ? -1 : 0);
	}
	return S_OK;
}

HRESULT __stdcall VirtualUISink::OnDropFile(long ScreenX, long ScreenY, BSTR Filenames, BSTR FileSizes, BSTR* IgnoreFiles) {
	if (m_VirtualUI->OnDropFile) {
		std::wstring fNames;
		if (Filenames == NULL)
			fNames = L"";
		else {
			fNames = Filenames;
		}
		std::wstring fSizes;
		if (FileSizes == NULL)
			fSizes = L"";
		else {
			fSizes = FileSizes;
		}
        std::wstring fIgnoreFiles;
        if (SysStringLen(*IgnoreFiles) == 0)
            fIgnoreFiles = L"";
        else {
            fIgnoreFiles = *IgnoreFiles;
        }
        m_VirtualUI->OnDropFile(ScreenX, ScreenY, fNames, fSizes, fIgnoreFiles);
        SysReAllocString(IgnoreFiles, fIgnoreFiles.c_str());
	}
	return S_OK;
}





CClientSettings::CClientSettings(IVirtualUI *virtualUI) {
	m_VirtualUI = virtualUI;
	m_ClientSettings = NULL;
	m_refcount = 0;
	if (m_VirtualUI != NULL) {
		m_VirtualUI->get_ClientSettings(&m_ClientSettings);
	}
}

CClientSettings::~CClientSettings() {
    if (m_VirtualUI != NULL)
        m_VirtualUI->Release();
    m_ClientSettings = NULL;
    m_VirtualUI = NULL;
}

HRESULT __stdcall CClientSettings::GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo) {
    if (m_ClientSettings != NULL)
        return m_ClientSettings->GetTypeInfo(Index, LocaleID, TypeInfo);
    else {
        TypeInfo = NULL;
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CClientSettings::GetTypeInfoCount(UINT *Count) {
    if (m_ClientSettings != NULL)
        return m_ClientSettings->GetTypeInfoCount(Count);
    else {
        Count = 0;
        return 0;
    }
}

HRESULT __stdcall CClientSettings::GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs) {
    if (m_ClientSettings != NULL)
        return m_ClientSettings->GetIDsOfNames(IID, Names, NameCount, LocaleID, DispIDs);
    else {
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CClientSettings::Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr) {
    if (m_ClientSettings != NULL)
        return m_ClientSettings->Invoke(DispID, IID, LocaleID, Flags, Params, VarResult, ExcepInfo, ArgErr);
    else {
        return E_NOTIMPL;
    }
}

ULONG __stdcall CClientSettings::AddRef() {
    return (ULONG)InterlockedIncrement(&m_refcount);
}

ULONG __stdcall CClientSettings::Release() {
    ULONG count = InterlockedDecrement(&m_refcount);
    if (count == 0) {
        delete this;
    }
    return count;
}

HRESULT __stdcall CClientSettings::QueryInterface(REFIID riid, void **ppvObject) {
    *ppvObject = NULL;
    if (riid == IID_IUnknown) {
        *ppvObject = this;
        this->AddRef();
    }

    if (*ppvObject == NULL) {
        return E_NOINTERFACE;
    }
    return NO_ERROR;
}

HRESULT __stdcall CClientSettings::get_MouseMoveGestureStyle(enum MouseMoveGestureStyle *Value) {
    if (m_ClientSettings != NULL)
        m_ClientSettings->get_MouseMoveGestureStyle(Value);
    else {
        *Value = MM_STYLE_ABSOLUTE;
    }
    return S_OK;
}

HRESULT __stdcall CClientSettings::put_MouseMoveGestureStyle(enum MouseMoveGestureStyle Value) {
    if (m_ClientSettings != NULL)
        m_ClientSettings->put_MouseMoveGestureStyle(Value);
    return S_OK;
}

HRESULT __stdcall CClientSettings::get_MouseMoveGestureAction(enum MouseMoveGestureAction *Value) {
    if (m_ClientSettings != NULL)
        m_ClientSettings->get_MouseMoveGestureAction(Value);
    else {
        *Value = MM_ACTION_MOVE;
    }
    return S_OK;
}

HRESULT __stdcall CClientSettings::put_MouseMoveGestureAction(enum MouseMoveGestureAction Value) {
    if (m_ClientSettings != NULL)
        m_ClientSettings->put_MouseMoveGestureAction(Value);
    return S_OK;
}

HRESULT __stdcall CClientSettings::get_CursorVisible(VARIANT_BOOL *Value) {
    if (m_ClientSettings != NULL)
        m_ClientSettings->get_CursorVisible(Value);
    else {
        *Value = -1;
    }
    return S_OK;
}

HRESULT __stdcall CClientSettings::put_CursorVisible(VARIANT_BOOL Value) {
    if (m_ClientSettings != NULL)
        m_ClientSettings->put_CursorVisible(Value);
    return S_OK;
}

HRESULT __stdcall CClientSettings::get_MouseWheelStepValue(long *Value) {
    if (m_ClientSettings != NULL)
        m_ClientSettings->get_MouseWheelStepValue(Value);
    else {
        *Value = 120;
    }
    return S_OK;
}

HRESULT __stdcall CClientSettings::put_MouseWheelStepValue(long Value) {
    if (m_ClientSettings != NULL)
        m_ClientSettings->put_MouseWheelStepValue(Value);
    return S_OK;
}

HRESULT __stdcall CClientSettings::get_MouseWheelDirection(long *Value) {
	if (m_ClientSettings != NULL)
		m_ClientSettings->get_MouseWheelDirection(Value);
	else {
		*Value = 1;
	}
	return S_OK;
}

HRESULT __stdcall CClientSettings::put_MouseWheelDirection(long Value) {
	if (m_ClientSettings != NULL)
		m_ClientSettings->put_MouseWheelDirection(Value);
	return S_OK;
}

MouseMoveGestureStyle CClientSettings::MouseMoveGestStyle() {
    MouseMoveGestureStyle ret;
    this->get_MouseMoveGestureStyle(&ret);
    return ret;
}

void CClientSettings::MouseMoveGestStyle(MouseMoveGestureStyle value) {
    this->put_MouseMoveGestureStyle(value);
}

MouseMoveGestureAction CClientSettings::MouseMoveGestAction() {
    MouseMoveGestureAction ret;
    this->get_MouseMoveGestureAction(&ret);
    return ret;
}

void CClientSettings::MouseMoveGestAction(MouseMoveGestureAction value) {
    this->put_MouseMoveGestureAction(value);
}

HRESULT __stdcall CClientSettings::get_MousePressAsRightButton(VARIANT_BOOL* Value) {
	if (m_ClientSettings != NULL)
		m_ClientSettings->get_MousePressAsRightButton(Value);
	else {
		*Value = 0;
	}
	return S_OK;
}

HRESULT __stdcall CClientSettings::put_MousePressAsRightButton(VARIANT_BOOL Value) {
	if (m_ClientSettings != NULL)
		m_ClientSettings->put_MousePressAsRightButton(Value);
	return S_OK;
}

bool CClientSettings::CursorVisible() {
    VARIANT_BOOL ret = 0;
    this->get_CursorVisible(&ret);
    return (ret != 0);
}

void CClientSettings::CursorVisible(bool value) {
    VARIANT_BOOL visible = value ? -1 : 0;
    this->put_CursorVisible(visible);
}

long CClientSettings::MouseWheelStepValue() {
    long ret = 0;
    this->get_MouseWheelStepValue(&ret);
    return ret;
}

void CClientSettings::MouseWheelStepValue(long value) {
    this->put_MouseWheelStepValue(value);
}

long CClientSettings::MouseWheelDirection() {
	long ret = 0;
	this->get_MouseWheelDirection(&ret);
	return ret;
}

void CClientSettings::MouseWheelDirection(long value) {
	this->put_MouseWheelDirection(value);
}

bool CClientSettings::MousePressAsRightButton() {
	VARIANT_BOOL ret = 0;
	this->get_MousePressAsRightButton(&ret);
	return (ret != 0);
}

void CClientSettings::MousePressAsRightButton(bool value) {
	VARIANT_BOOL visible = value ? -1 : 0;
	this->put_MousePressAsRightButton(visible);
}



CRecorder::CRecorder(IVirtualUI *virtualUI) {
	m_VirtualUI = virtualUI;
    m_Recorder = NULL;
	m_refcount = 0;
	if (m_VirtualUI != NULL) {
		m_VirtualUI->get_Recorder(&m_Recorder);
	}
	m_RecTracks = new CRecTracks(m_Recorder);
}

CRecorder::~CRecorder() {
	if (m_VirtualUI != NULL)
		m_VirtualUI->Release();
	m_RecTracks = NULL;
	m_Recorder = NULL;
	m_VirtualUI = NULL;
}

HRESULT __stdcall CRecorder::GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo) {
	if (m_Recorder != NULL)
		return m_Recorder->GetTypeInfo(Index, LocaleID, TypeInfo);
	else {
		TypeInfo = NULL;
		return E_NOTIMPL;
	}
}

HRESULT __stdcall CRecorder::GetTypeInfoCount(UINT *Count) {
	if (m_Recorder != NULL)
		return m_Recorder->GetTypeInfoCount(Count);
	else {
		Count = 0;
		return 0;
	}
}

HRESULT __stdcall CRecorder::GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs) {
	if (m_Recorder != NULL)
		return m_Recorder->GetIDsOfNames(IID, Names, NameCount, LocaleID, DispIDs);
	else {
		return E_NOTIMPL;
	}
}

HRESULT __stdcall CRecorder::Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr) {
	if (m_Recorder != NULL)
		return m_Recorder->Invoke(DispID, IID, LocaleID, Flags, Params, VarResult, ExcepInfo, ArgErr);
	else {
		return E_NOTIMPL;
	}
}

ULONG __stdcall CRecorder::AddRef() {
	return (ULONG)InterlockedIncrement(&m_refcount);
}

ULONG __stdcall CRecorder::Release() {
	ULONG count = InterlockedDecrement(&m_refcount);
	if (count == 0) {
		delete this;
	}
	return count;
}

HRESULT __stdcall CRecorder::QueryInterface(REFIID riid, void **ppvObject) {
	*ppvObject = NULL;
	if (riid == IID_IUnknown) {
		*ppvObject = this;
		this->AddRef();
	}

	if (*ppvObject == NULL) {
		return E_NOINTERFACE;
	}
	return NO_ERROR;
}

HRESULT __stdcall CRecorder::get_Filename(BSTR *Value) {
	if (m_Recorder != NULL)
		return m_Recorder->get_Filename(Value);
	else {
		*Value = NULL;
		return S_OK;
	}
}

HRESULT __stdcall CRecorder::put_Filename(BSTR Value) {
	if (m_Recorder != NULL)
		m_Recorder->put_Filename(Value);
	return S_OK;
}

HRESULT __stdcall CRecorder::get_Position(long *Value) {
	if (m_Recorder != NULL)
		return m_Recorder->get_Position(Value);
	else {
		*Value = NULL;
		return S_OK;
	}
}

HRESULT __stdcall CRecorder::get_Count(long *Value) {
	if (m_Recorder != NULL)
		return m_Recorder->get_Count(Value);
	else {
		*Value = NULL;
		return S_OK;
	}
}

HRESULT __stdcall CRecorder::get_State(RecorderState *Value) {
	if (m_Recorder != NULL)
		return m_Recorder->get_State(Value);
	else {
		*Value = Inactive;
		return S_OK;
	}
}

HRESULT __stdcall CRecorder::get_Options(long *Value) {
	if (m_Recorder != NULL)
		return m_Recorder->get_Options(Value);
	else {
		*Value = NULL;
		return S_OK;
	}
}

HRESULT __stdcall CRecorder::put_Options(long Value) {
	if (m_Recorder != NULL)
		m_Recorder->put_Options(Value);
	return S_OK;
}

HRESULT __stdcall CRecorder::get_Tracks(IRecTracks **Value) {
	if (m_Recorder != NULL)
		return m_Recorder->get_Tracks(Value);
	else {
		return S_OK;
	}
}

HRESULT __stdcall CRecorder::Rec(BSTR Label) {
	if (m_Recorder != NULL)
		m_Recorder->Rec(Label);
	return S_OK;
}

HRESULT __stdcall CRecorder::Play(long From, long To) {
	if (m_Recorder != NULL)
		m_Recorder->Play(From, To);
	return S_OK;
}

HRESULT __stdcall CRecorder::Stop(void) {
	if (m_Recorder != NULL)
		m_Recorder->Stop();
	return S_OK;
}

std::wstring CRecorder::Filename() {
  std::wstring ret;
	BSTR value;
	this->get_Filename(&value);
  if (value != NULL)
  	ret = std::wstring(value);
	SysFreeString(value);
	return ret;
}

void CRecorder::Filename(std::wstring value) {
	BSTR fileName = SysAllocString(value.c_str());
	this->put_Filename(fileName);
	SysFreeString(fileName);
}

long CRecorder::Position() {
	long ret = 0;
	this->get_Position(&ret);
	return ret;
}

long CRecorder::Count() {
	long ret = 0;
	this->get_Count(&ret);
	return ret;
}

RecorderState CRecorder::State() {
	RecorderState ret = Inactive;
	this->get_State(&ret);
	return ret;
}

long CRecorder::Options() {
	long ret = 0;
	this->get_Options(&ret);
	return ret;
}

void CRecorder::Options(long value) {
	this->put_Options(value);
}

CRecTracks* CRecorder::Tracks() {
	return m_RecTracks;
}

void CRecorder::Rec(std::wstring Label) {
	BSTR lab = SysAllocString(Label.c_str());
	this->Rec(lab);
	SysFreeString(lab);
}





CRecTracks::CRecTracks(IRecorder* Recorder) {
	m_Recorder = Recorder;
	m_refcount = 0;
}

CRecTracks::~CRecTracks() {
	m_Recorder = NULL;
}

HRESULT __stdcall CRecTracks::GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo) {
	if (m_Recorder != NULL) {
    IRecTracks* recTracks;
		m_Recorder->get_Tracks(&recTracks);
    if (recTracks != NULL) {
      HRESULT res = recTracks->GetTypeInfo(Index, LocaleID, TypeInfo);
      recTracks = NULL;
      return res;
    }
  }
  TypeInfo = NULL;
	return E_NOTIMPL;
}

HRESULT __stdcall CRecTracks::GetTypeInfoCount(UINT *Count) {
	if (m_Recorder != NULL) {
    IRecTracks* recTracks;
		m_Recorder->get_Tracks(&recTracks);
    if (recTracks != NULL) {
      HRESULT res = recTracks->GetTypeInfoCount(Count);
      recTracks = NULL;
      return res;
    }
  }
  Count = 0;
  return 0;
}

HRESULT __stdcall CRecTracks::GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs) {
	if (m_Recorder != NULL) {
    IRecTracks* recTracks;
		m_Recorder->get_Tracks(&recTracks);
    if (recTracks != NULL) {
      HRESULT res = recTracks->GetIDsOfNames(IID, Names, NameCount, LocaleID, DispIDs);
      recTracks = NULL;
      return res;
    }
  }
  return E_NOTIMPL;
}

HRESULT __stdcall CRecTracks::Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr) {
	if (m_Recorder != NULL) {
    IRecTracks* recTracks;
		m_Recorder->get_Tracks(&recTracks);
    if (recTracks != NULL) {
      HRESULT res = recTracks->Invoke(DispID, IID, LocaleID, Flags, Params, VarResult, ExcepInfo, ArgErr);
      recTracks = NULL;
      return res;
    }
  }
  return E_NOTIMPL;
}

HRESULT __stdcall CRecTracks::QueryInterface(REFIID riid, void **ppvObject) {
	if (m_Recorder != NULL) {
    IRecTracks* recTracks;
		m_Recorder->get_Tracks(&recTracks);
    if (recTracks != NULL) {
      HRESULT res = recTracks->QueryInterface(riid, ppvObject);
      recTracks = NULL;
      return res;
    }
  }
  return E_NOINTERFACE;
}

ULONG __stdcall CRecTracks::AddRef() {
	return (ULONG)InterlockedIncrement(&m_refcount);
}

ULONG __stdcall CRecTracks::Release() {
	ULONG count = InterlockedDecrement(&m_refcount);
	if (count == 0) {
		delete this;
	}
	return count;
}

HRESULT __stdcall CRecTracks::get_Count(long *Value) {
	if (m_Recorder != NULL) {
    IRecTracks* recTracks;
		m_Recorder->get_Tracks(&recTracks);
    if (recTracks != NULL) {
  		HRESULT res = recTracks->get_Count(Value);
      recTracks = NULL;
      return res;
    }
  }
	return S_OK;
}

HRESULT __stdcall CRecTracks::get_Item(long Index, IRecTrack **Value) {
	if (m_Recorder != NULL) {
    IRecTracks* recTracks;
		m_Recorder->get_Tracks(&recTracks);
    if (recTracks != NULL) {
      HRESULT res = recTracks->get_Item(Index, Value);
      recTracks = NULL;
      return res;
    }
  }
	return S_OK;
}

long CRecTracks::Count() {
	long res = 0;
	this->get_Count(&res);
	return res;
}

IRecTrack* CRecTracks::Item(long index) {
	IRecTrack* res = NULL;
	this->get_Item(index, &res);
	return res;
}





CBrowserInfo::CBrowserInfo(IVirtualUI *virtualUI) {
    m_VirtualUI = virtualUI;
    m_BrowserInfo = NULL;
    m_refcount = 0;
    if (m_VirtualUI != NULL) {
        m_VirtualUI->get_BrowserInfo(&m_BrowserInfo);
    }
}

CBrowserInfo::~CBrowserInfo() {
    if (m_VirtualUI != NULL)
        m_VirtualUI->Release();
    m_BrowserInfo = NULL;
    m_VirtualUI = NULL;
}

HRESULT __stdcall CBrowserInfo::GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo) {
    if (m_BrowserInfo != NULL)
        return m_BrowserInfo->GetTypeInfo(Index, LocaleID, TypeInfo);
    else {
        TypeInfo = NULL;
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CBrowserInfo::GetTypeInfoCount(UINT *Count) {
    if (m_BrowserInfo != NULL)
        return m_BrowserInfo->GetTypeInfoCount(Count);
    else {
        Count = 0;
        return 0;
    }
}

HRESULT __stdcall CBrowserInfo::GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs) {
    if (m_BrowserInfo != NULL)
        return m_BrowserInfo->GetIDsOfNames(IID, Names, NameCount, LocaleID, DispIDs);
    else {
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CBrowserInfo::Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr) {
    if (m_BrowserInfo != NULL)
        return m_BrowserInfo->Invoke(DispID, IID, LocaleID, Flags, Params, VarResult, ExcepInfo, ArgErr);
    else {
        return E_NOTIMPL;
    }
}

ULONG __stdcall CBrowserInfo::AddRef() {
    return (ULONG)InterlockedIncrement(&m_refcount);
}

ULONG __stdcall CBrowserInfo::Release() {
    ULONG count = InterlockedDecrement(&m_refcount);
    if (count == 0) {
        delete this;
    }
    return count;
}

HRESULT __stdcall CBrowserInfo::QueryInterface(REFIID riid, void **ppvObject) {
    *ppvObject = NULL;
    if (riid == IID_IUnknown) {
        *ppvObject = this;
        this->AddRef();
    }

    if (*ppvObject == NULL) {
        return E_NOINTERFACE;
    }
    return NO_ERROR;
}

HRESULT __stdcall CBrowserInfo::get_ViewWidth(long *Value) {
    if (m_BrowserInfo != NULL)
        return m_BrowserInfo->get_ViewWidth(Value);
    else {
        *Value = 0;
        return S_OK;
    }
}

HRESULT __stdcall CBrowserInfo::put_ViewWidth(long Value) {
    if (m_BrowserInfo != NULL)
		return m_BrowserInfo->put_ViewWidth(Value);
    return S_OK;
}

HRESULT __stdcall CBrowserInfo::get_ViewHeight(long *Value) {
    if (m_BrowserInfo != NULL)
        return m_BrowserInfo->get_ViewHeight(Value);
    else {
        *Value = 0;
        return S_OK;
    }
}

HRESULT __stdcall CBrowserInfo::put_ViewHeight(long Value) {
    if (m_BrowserInfo != NULL)
        return m_BrowserInfo->put_ViewHeight(Value);
    return S_OK;
}

HRESULT __stdcall CBrowserInfo::get_BrowserWidth(long *Value) {
    if (m_BrowserInfo != NULL)
        return m_BrowserInfo->get_BrowserWidth(Value);
    else {
        *Value = 0;
        return S_OK;
    }
}

HRESULT __stdcall CBrowserInfo::get_BrowserHeight(long *Value) {
    if (m_BrowserInfo != NULL)
        return m_BrowserInfo->get_BrowserHeight(Value);
    else {
        *Value = 0;
        return S_OK;
    }
}

HRESULT __stdcall CBrowserInfo::get_ScreenWidth(long *Value) {
    if (m_BrowserInfo != NULL)
        return m_BrowserInfo->get_ScreenWidth(Value);
    else {
        *Value = 0;
        return S_OK;
    }
}

HRESULT __stdcall CBrowserInfo::get_ScreenHeight(long *Value) {
    if (m_BrowserInfo != NULL)
        return m_BrowserInfo->get_ScreenHeight(Value);
    else {
        *Value = 0;
        return S_OK;
    }
}

HRESULT __stdcall CBrowserInfo::get_Username(BSTR *Value) {
    if (m_BrowserInfo != NULL)
        return m_BrowserInfo->get_Username(Value);
    else {
        *Value = NULL;
        return S_OK;
    }
}

HRESULT __stdcall CBrowserInfo::get_IPAddress(BSTR *Value) {
    if (m_BrowserInfo != NULL)
        return m_BrowserInfo->get_IPAddress(Value);
    else {
        *Value = NULL;
        return S_OK;
    }
}

HRESULT __stdcall CBrowserInfo::get_UserAgent(BSTR *Value) {
    if (m_BrowserInfo != NULL)
        return m_BrowserInfo->get_UserAgent(Value);
    else {
        *Value = NULL;
        return S_OK;
    }
}

HRESULT __stdcall CBrowserInfo::get_ScreenResolution(long *Value) {
    if (m_BrowserInfo != NULL)
        return m_BrowserInfo->get_ScreenResolution(Value);
    else {
        *Value = 0;
        return S_OK;
    }
}

HRESULT __stdcall CBrowserInfo::get_Orientation(enum Orientation *Value) {
    if (m_BrowserInfo != NULL)
        return m_BrowserInfo->get_Orientation(Value);
    else {
        *Value = PORTRAIT;
        return S_OK;
    }
}

HRESULT __stdcall CBrowserInfo::get_UniqueBrowserId(BSTR *Value) {
    if (m_BrowserInfo != NULL)
        return m_BrowserInfo->get_UniqueBrowserId(Value);
    else {
        *Value = NULL;
        return S_OK;
    }
}

HRESULT __stdcall CBrowserInfo::get_CustomData(BSTR *Value) {
	if (m_BrowserInfo != NULL)
		return m_BrowserInfo->get_CustomData(Value);
	else {
		*Value = NULL;
		return S_OK;
	}
}

HRESULT __stdcall CBrowserInfo::put_CustomData(BSTR Value) {
	if (m_BrowserInfo != NULL)
		m_BrowserInfo->put_CustomData(Value);
	return S_OK;
}

HRESULT __stdcall CBrowserInfo::GetCookie(BSTR Name, BSTR *Value) {
    if (m_BrowserInfo != NULL)
        return m_BrowserInfo->GetCookie(Name, Value);
    else {
        *Value = NULL;
        return S_OK;
    }
}

HRESULT __stdcall CBrowserInfo::SetCookie(BSTR Name, BSTR Value, BSTR Expires) {
    if (m_BrowserInfo != NULL)
        return m_BrowserInfo->SetCookie(Name, Value, Expires);
    return S_OK;
}

HRESULT __stdcall CBrowserInfo::get_Location(BSTR *Value) {
    if (m_BrowserInfo != NULL)
        return m_BrowserInfo->get_Location(Value);
    else {
        *Value = NULL;
        return S_OK;
    }
}

HRESULT __stdcall CBrowserInfo::get_SelectedRule(BSTR* Value) {
	if (m_BrowserInfo != NULL)
		return m_BrowserInfo->get_SelectedRule(Value);
	else {
		*Value = NULL;
		return S_OK;
	}
}

HRESULT __stdcall CBrowserInfo::get_ExtraData(BSTR *Value) {
	if (m_BrowserInfo != NULL)
		return m_BrowserInfo->get_ExtraData(Value);
	else {
		*Value = NULL;
		return S_OK;
	}
}

HRESULT __stdcall CBrowserInfo::GetExtraDataValue(BSTR Name, BSTR *Value) {
	if (m_BrowserInfo != NULL)
		return m_BrowserInfo->GetExtraDataValue(Name, Value);
	else {
		*Value = NULL;
		return S_OK;
	}
}

int CBrowserInfo::ViewWidth() {
    long ret;
    this->get_ViewWidth(&ret);
    return ret;
}

void CBrowserInfo::ViewWidth(int value) {
    this->put_ViewWidth(value);
}

int CBrowserInfo::ViewHeight() {
    long ret;
    this->get_ViewHeight(&ret);
    return ret;
}

void CBrowserInfo::ViewHeight(int value) {
    this->put_ViewHeight(value);
}

int CBrowserInfo::BrowserWidth() {
    long ret;
    this->get_BrowserWidth(&ret);
    return ret;
}

int CBrowserInfo::BrowserHeight() {
    long ret;
    this->get_BrowserHeight(&ret);
    return ret;
}

int CBrowserInfo::ScreenWidth() {
    long ret;
    this->get_ScreenWidth(&ret);
    return ret;
}

int CBrowserInfo::ScreenHeight() {
    long ret;
    this->get_ScreenHeight(&ret);
    return ret;
}

std::wstring CBrowserInfo::Username() {
    BSTR value;
    this->get_Username(&value);
    std::wstring ret(value);
    SysFreeString(value);
    return ret;
}

std::wstring CBrowserInfo::IPAddress() {
    BSTR value;
    this->get_IPAddress(&value);
    std::wstring ret(value);
    SysFreeString(value);
    return ret;
}

std::wstring CBrowserInfo::UserAgent() {
    BSTR value;
    this->get_UserAgent(&value);
    std::wstring ret(value);
    SysFreeString(value);
    return ret;
}

std::wstring CBrowserInfo::UniqueBrowserId() {
    BSTR value;
    this->get_UniqueBrowserId(&value);
    std::wstring ret(value);
    SysFreeString(value);
    return ret;
}

std::wstring CBrowserInfo::CustomData() {
	BSTR value;
	this->get_CustomData(&value);
	std::wstring ret;
	if (value != NULL) {
		ret = std::wstring(value);
	}
	SysFreeString(value);
	return ret;
}

void CBrowserInfo::CustomData(std::wstring value) {
	BSTR bValue = SysAllocString(value.c_str());
	this->put_CustomData(bValue);
	SysFreeString(bValue);
}

std::wstring CBrowserInfo::Location() {
    BSTR value;
    this->get_Location(&value);
	std::wstring ret(value);
    SysFreeString(value);
    return ret;
}

int CBrowserInfo::ScreenResolution() {
    long ret;
    this->get_ScreenResolution(&ret);
    return ret;
}

Orientation CBrowserInfo::Orientation() {
    enum Orientation ret;
    this->get_Orientation(&ret);
    return ret;
}

std::wstring CBrowserInfo::SelectedRule() {
	BSTR value;
	this->get_SelectedRule(&value);
	std::wstring ret(value);
	SysFreeString(value);
	return ret;
}

std::wstring CBrowserInfo::ExtraData() {
	BSTR value;
	this->get_ExtraData(&value);
	std::wstring ret(value);
	SysFreeString(value);
	return ret;
}

std::wstring CBrowserInfo::GetExtraDataValue(std::wstring Name) {
	BSTR value;
	BSTR name = SysAllocString(Name.c_str());
	this->GetExtraDataValue(name, &value);
	std::wstring ret(value);
	SysFreeString(name);
	SysFreeString(value);
	return ret;
}





HRESULT __stdcall CFileSystemFilter::GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo) {
    if (m_Filter != NULL)
        return m_Filter->GetTypeInfo(Index, LocaleID, TypeInfo);
    else {
        TypeInfo = NULL;
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CFileSystemFilter::GetTypeInfoCount(UINT *Count) {
    if (m_Filter != NULL)
        return m_Filter->GetTypeInfoCount(Count);
    else {
        Count = 0;
        return 0;
    }
}

HRESULT __stdcall CFileSystemFilter::GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs) {
    if (m_Filter != NULL)
        return m_Filter->GetIDsOfNames(IID, Names, NameCount, LocaleID, DispIDs);
    else {
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CFileSystemFilter::Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr) {
    if (m_Filter != NULL)
        return m_Filter->Invoke(DispID, IID, LocaleID, Flags, Params, VarResult, ExcepInfo, ArgErr);
    else {
        return E_NOTIMPL;
    }
}

ULONG __stdcall CFileSystemFilter::AddRef() {
    return (ULONG)InterlockedIncrement(&m_refcount);
}

ULONG __stdcall CFileSystemFilter::Release() {
    ULONG count = InterlockedDecrement(&m_refcount);
    if (count == 0) {
        delete this;
    }
    return count;
}

HRESULT __stdcall CFileSystemFilter::QueryInterface(REFIID riid, void **ppvObject) {
    if (m_Filter != NULL)
        return m_Filter->QueryInterface(riid, ppvObject);
    else {
        return E_NOINTERFACE;
    }
}

CFileSystemFilter::CFileSystemFilter(IVirtualUI *virtualUI) {
    m_VirtualUI = virtualUI;
    m_Filter = NULL;
    m_refcount = 0;
    if (m_VirtualUI != NULL) {
        m_VirtualUI->get_FileSystemFilter(&m_Filter);
    }
}

CFileSystemFilter::~CFileSystemFilter() {
    if (m_VirtualUI != NULL)
        m_VirtualUI->Release();
    m_Filter = NULL;
    m_VirtualUI = NULL;
}

HRESULT __stdcall CFileSystemFilter::get_User(BSTR *Value) {
    if (m_Filter != NULL)
        return m_Filter->get_User(Value);
    else {
        *Value = NULL;
        return S_OK;
    }
}

HRESULT __stdcall CFileSystemFilter::put_User(BSTR Value) {
    if (m_Filter != NULL)
        return m_Filter->put_User(Value);
    return S_OK;
}

HRESULT __stdcall CFileSystemFilter::get_CfgFile(BSTR *Value) {
    if (m_Filter != NULL)
        return m_Filter->get_CfgFile(Value);
    else {
        *Value = NULL;
        return S_OK;
    }
}

HRESULT __stdcall CFileSystemFilter::put_CfgFile(BSTR Value) {
    if (m_Filter != NULL)
        return m_Filter->put_CfgFile(Value);
    return S_OK;
}

HRESULT __stdcall CFileSystemFilter::get_Active(VARIANT_BOOL *Value)
 {
    if (m_Filter != NULL)
        return m_Filter->get_Active(Value);
    else {
        *Value = NULL;
        return S_OK;
    }
}

HRESULT __stdcall CFileSystemFilter::put_Active(VARIANT_BOOL Value) {
    if (m_Filter != NULL)
        return m_Filter->put_Active(Value);
    return S_OK;
}

std::wstring CFileSystemFilter::User() {
    BSTR value;
    this->get_User(&value);
    std::wstring ret(value);
    SysFreeString(value);
    return ret;
}

void CFileSystemFilter::User(std::wstring value) {
    BSTR temp = SysAllocString(value.c_str());
    this->put_User(temp);
    SysFreeString(temp);
}

std::wstring CFileSystemFilter::CfgFile() {
    BSTR value;
    this->get_CfgFile(&value);
    std::wstring ret(value);
    SysFreeString(value);
    return ret;
}

void CFileSystemFilter::CfgFile(std::wstring value) {
    BSTR temp = SysAllocString(value.c_str());
    this->put_CfgFile(temp);
    SysFreeString(temp);
}

bool CFileSystemFilter::Active()
 {
    VARIANT_BOOL ret = 0;
    this->get_Active(&ret);
    return (ret != 0);
}

void CFileSystemFilter::Active(bool value) {
    VARIANT_BOOL temp = value ? -1 : 0;
    this->put_Active(temp);
}

HRESULT __stdcall CRegistryFilter::GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo) {
    if (m_Filter != NULL)
        return m_Filter->GetTypeInfo(Index, LocaleID, TypeInfo);
    else {
        TypeInfo = NULL;
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CRegistryFilter::GetTypeInfoCount(UINT *Count) {
    if (m_Filter != NULL)
        return m_Filter->GetTypeInfoCount(Count);
    else {
        Count = 0;
        return 0;
    }
}

HRESULT __stdcall CRegistryFilter::GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs) {
    if (m_Filter != NULL)
        return m_Filter->GetIDsOfNames(IID, Names, NameCount, LocaleID, DispIDs);
    else {
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CRegistryFilter::Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr) {
    if (m_Filter != NULL)
        return m_Filter->Invoke(DispID, IID, LocaleID, Flags, Params, VarResult, ExcepInfo, ArgErr);
    else {
        return E_NOTIMPL;
    }
}

ULONG __stdcall CRegistryFilter::AddRef() {
    return (ULONG)InterlockedIncrement(&m_refcount);
}

ULONG __stdcall CRegistryFilter::Release() {
    ULONG count = InterlockedDecrement(&m_refcount);
    if (count == 0) {
        delete this;
    }
    return count;
}

HRESULT __stdcall CRegistryFilter::QueryInterface(REFIID riid, void **ppvObject) {
    if (m_Filter != NULL)
        return m_Filter->QueryInterface(riid, ppvObject);
    else {
        return E_NOINTERFACE;
    }
}

CRegistryFilter::CRegistryFilter(IVirtualUI *virtualUI) {
    m_VirtualUI = virtualUI;
    m_Filter = NULL;
    m_refcount = 0;
    if (m_VirtualUI != NULL) {
        m_VirtualUI->get_RegistryFilter(&m_Filter);
    }
}

CRegistryFilter::~CRegistryFilter() {
    if (m_VirtualUI != NULL)
        m_VirtualUI->Release();
    m_Filter = NULL;
    m_VirtualUI = NULL;
}

HRESULT __stdcall CRegistryFilter::get_User(BSTR *Value) {
    if (m_Filter != NULL)
        return m_Filter->get_User(Value);
    else {
        *Value = NULL;
        return S_OK;
    }
}

HRESULT __stdcall CRegistryFilter::put_User(BSTR Value) {
    if (m_Filter != NULL)
        return m_Filter->put_User(Value);
    return S_OK;
}

HRESULT __stdcall CRegistryFilter::get_CfgFile(BSTR *Value) {
    if (m_Filter != NULL)
        return m_Filter->get_CfgFile(Value);
    else {
        *Value = NULL;
        return S_OK;
    }
}

HRESULT __stdcall CRegistryFilter::put_CfgFile(BSTR Value) {
    if (m_Filter != NULL)
        return m_Filter->put_CfgFile(Value);
    return S_OK;
}

HRESULT __stdcall CRegistryFilter::get_Active(VARIANT_BOOL *Value)
 {
    if (m_Filter != NULL)
        return m_Filter->get_Active(Value);
    else {
        *Value = NULL;
        return S_OK;
    }
}

HRESULT __stdcall CRegistryFilter::put_Active(VARIANT_BOOL Value) {
    if (m_Filter != NULL)
        return m_Filter->put_Active(Value);
    return S_OK;
}

std::wstring CRegistryFilter::User() {
    BSTR value;
    this->get_User(&value);
    std::wstring ret(value);
    SysFreeString(value);
    return ret;
}

void CRegistryFilter::User(std::wstring value) {
    BSTR temp = SysAllocString(value.c_str());
    this->put_User(temp);
    SysFreeString(temp);
}

std::wstring CRegistryFilter::CfgFile() {
    BSTR value;
    this->get_CfgFile(&value);
    std::wstring ret(value);
    SysFreeString(value);
    return ret;
}

void CRegistryFilter::CfgFile(std::wstring value) {
    BSTR temp = SysAllocString(value.c_str());
    this->put_CfgFile(temp);
    SysFreeString(temp);
}

bool CRegistryFilter::Active()
 {
    VARIANT_BOOL ret = 0;
    this->get_Active(&ret);
    return (ret != 0);
}

void CRegistryFilter::Active(bool value) {
    VARIANT_BOOL temp = value ? -1 : 0;
    this->put_Active(temp);
}


CDevServer::CDevServer(IVirtualUI *virtualUI) {
    m_VirtualUI = virtualUI;
    m_DevServer = NULL;
    m_refcount = 0;
    if (m_VirtualUI != NULL) {
        m_VirtualUI->get_DevServer(&m_DevServer);
    }
}

CDevServer::~CDevServer() {
    if (m_VirtualUI != NULL)
        m_VirtualUI->Release();
    m_DevServer = NULL;
    m_VirtualUI = NULL;
}

HRESULT __stdcall CDevServer::GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo) {
    if (m_DevServer != NULL)
        return m_DevServer->GetTypeInfo(Index, LocaleID, TypeInfo);
    else {
        TypeInfo = NULL;
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CDevServer::GetTypeInfoCount(UINT *Count) {
    if (m_DevServer != NULL)
        return m_DevServer->GetTypeInfoCount(Count);
    else {
        Count = 0;
        return 0;
    }
}

HRESULT __stdcall CDevServer::GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs) {
    if (m_DevServer != NULL)
        return m_DevServer->GetIDsOfNames(IID, Names, NameCount, LocaleID, DispIDs);
    else {
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CDevServer::Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr) {
    if (m_DevServer != NULL)
        return m_DevServer->Invoke(DispID, IID, LocaleID, Flags, Params, VarResult, ExcepInfo, ArgErr);
    else {
        return E_NOTIMPL;
    }
}

ULONG __stdcall CDevServer::AddRef() {
    return (ULONG)InterlockedIncrement(&m_refcount);
}

ULONG __stdcall CDevServer::Release() {
    ULONG count = InterlockedDecrement(&m_refcount);
    if (count == 0) {
        delete this;
    }
    return count;
}

HRESULT __stdcall CDevServer::QueryInterface(REFIID riid, void **ppvObject) {
    *ppvObject = NULL;
    if (riid == IID_IUnknown) {
        *ppvObject = this;
        this->AddRef();
    }

    if (*ppvObject == NULL) {
        return E_NOINTERFACE;
    }
    return NO_ERROR;
}

HRESULT __stdcall CDevServer::get_Enabled(VARIANT_BOOL *Value) {
    if (m_DevServer != NULL)
        return m_DevServer->get_Enabled(Value);
    else {
        *Value = 0;
        return S_OK;
    }
}

HRESULT __stdcall CDevServer::put_Enabled(VARIANT_BOOL Value) {
    if (m_DevServer != NULL)
        m_DevServer->put_Enabled(Value);
    return S_OK;
}

HRESULT __stdcall CDevServer::get_Port(long *Value) {
    if (m_DevServer != NULL)
        return m_DevServer->get_Port(Value);
    else {
        *Value = 0;
        return S_OK;
    }
}

HRESULT __stdcall CDevServer::put_Port(long Value) {
    if (m_DevServer != NULL)
        m_DevServer->put_Port(Value);
    return S_OK;
}

HRESULT __stdcall CDevServer::get_StartBrowser(VARIANT_BOOL *Value) {
    if (m_DevServer != NULL)
        return m_DevServer->get_StartBrowser(Value);
    else {
        *Value = 0;
        return S_OK;
    }
}

HRESULT __stdcall CDevServer::put_StartBrowser(VARIANT_BOOL Value) {
    if (m_DevServer != NULL)
        m_DevServer->put_StartBrowser(Value);
    return S_OK;
}

bool CDevServer::Enabled() {
    VARIANT_BOOL ret = 0;
    this->get_Enabled(&ret);
    return (ret != 0);
}

void CDevServer::Enabled(bool value) {
    VARIANT_BOOL enabled = value ? -1 : 0;
    this->put_Enabled(enabled);
}

int CDevServer::Port() {
    long ret;
    this->get_Port(&ret);
    return ret;
}

void CDevServer::Port(int value) {
    this->put_Port(value);
}

void CDevServer::StartBrowser(bool value) {
    VARIANT_BOOL start = value ? -1 : 0;
    this->put_StartBrowser(start);
}





JSObject::JSObject(std::wstring id) {
	m_refcount = 0;
	m_JSObject = NULL;
    m_EventSink = NULL;
	m_JSProperties = NULL;
	m_JSMethods = NULL;
	m_JSEvents = NULL;
	m_JSObjects = NULL;
	OnExecuteMethod = NULL;
	OnPropertyChange = NULL;

  m_VirtualUILib = new VirtualUILibrary();
  HINSTANCE LibHandle = m_VirtualUILib->GetHandle();

	if (LibHandle != 0) {
		typedef HRESULT(__stdcall *DllCreateJSObject)(IJSObject**);
		DllCreateJSObject CreateJSObject = (DllCreateJSObject)GetProcAddress(LibHandle, "DllCreateJSObject");
		CreateJSObject(&m_JSObject);

		m_EventSink = new JSObjectSink(this);
		m_JSProperties = new CJSProperties(this);
		m_JSMethods = new CJSMethods(this);
		m_JSEvents = new CJSEvents(this);
		m_JSObjects = new CJSObjects(this);
	}
	Id(id);
}

JSObject::~JSObject() {
	m_EventSink = NULL;
	if (m_JSObject != NULL) {
		m_JSObject->Release();
	}
    m_JSProperties = NULL;
    m_JSMethods = NULL;
    m_JSEvents = NULL;
    m_JSObjects = NULL;
    m_JSObject = NULL;
    m_VirtualUILib = NULL;
}

HRESULT __stdcall JSObject::GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo) {
    if (m_JSObject != NULL)
        return m_JSObject->GetTypeInfo(Index, LocaleID, TypeInfo);
    else {
        TypeInfo = NULL;
        return E_NOTIMPL;
    }
}

HRESULT __stdcall JSObject::GetTypeInfoCount(UINT *Count) {
    if (m_JSObject != NULL)
        return m_JSObject->GetTypeInfoCount(Count);
    else {
        Count = 0;
        return 0;
    }
}

HRESULT __stdcall JSObject::GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs) {
    if (m_JSObject != NULL)
        return m_JSObject->GetIDsOfNames(IID, Names, NameCount, LocaleID, DispIDs);
    else {
        return E_NOTIMPL;
    }
}

HRESULT __stdcall JSObject::Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr) {
    if (m_JSObject != NULL)
        return m_JSObject->Invoke(DispID, IID, LocaleID, Flags, Params, VarResult, ExcepInfo, ArgErr);
    else {
        return E_NOTIMPL;
    }
}

HRESULT __stdcall JSObject::QueryInterface(REFIID riid, void **ppvObject) {
    if (m_JSObject != NULL)
        return m_JSObject->QueryInterface(riid, ppvObject);
    else {
        return E_NOINTERFACE;
    }
}

ULONG __stdcall JSObject::AddRef() {
    return (ULONG)InterlockedIncrement(&m_refcount);
}

ULONG __stdcall JSObject::Release() {
    ULONG count = InterlockedDecrement(&m_refcount);
    if (count == 0) {
        delete this;
    }
    return count;
}

HRESULT __stdcall JSObject::get_Id(BSTR *Value) {
    if (m_JSObject != NULL)
        return m_JSObject->get_Id(Value);
    else {
        *Value = NULL;
        return S_OK;
    }
}

HRESULT __stdcall JSObject::put_Id(BSTR Value) {
    if (m_JSObject != NULL)
        m_JSObject->put_Id(Value);
    return S_OK;
}

HRESULT __stdcall JSObject::get_Properties(IJSProperties **Value) {
    if (m_JSObject != NULL)
        return m_JSObject->get_Properties(Value);
    else {
        *Value = NULL;
        return S_OK;
    }
}

HRESULT __stdcall JSObject::get_Methods(IJSMethods **Value) {
    if (m_JSObject != NULL)
        return m_JSObject->get_Methods(Value);
    else {
        *Value = NULL;
        return S_OK;
    }
}

HRESULT __stdcall JSObject::get_Events(IJSEvents **Value) {
    if (m_JSObject != NULL)
        return m_JSObject->get_Events(Value);
    else {
        *Value = NULL;
        return S_OK;
    }
}

HRESULT __stdcall JSObject::get_Objects(IJSObjects **Value) {
    if (m_JSObject != NULL)
        return m_JSObject->get_Objects(Value);
    else {
        *Value = NULL;
        return S_OK;
    }
}

HRESULT __stdcall JSObject::FireEvent(BSTR Name, IJSArguments *Arguments) {
    if (m_JSObject != NULL)
        return m_JSObject->FireEvent(Name, Arguments);
    return S_OK;
}

void JSObject::FireEvent(std::wstring Name, IJSArguments* Arguments) {
    if (m_JSObject != NULL) {
        BSTR bName = SysAllocString(Name.c_str());
        m_JSObject->FireEvent(bName, Arguments);
        SysFreeString(bName);
    }
}

HRESULT __stdcall JSObject::ApplyChanges(void) {
    if (m_JSObject != NULL)
        m_JSObject->ApplyChanges();
    return S_OK;
}

HRESULT __stdcall JSObject::ApplyModel(void) {
    if (m_JSObject != NULL)
        m_JSObject->ApplyModel();
    return S_OK;
}

std::wstring JSObject::Id() {
    BSTR value;
    this->get_Id(&value);
    std::wstring ret((wchar_t*)value);
    SysFreeString(value);
    return ret;
}

void JSObject::Id(std::wstring value) {
    BSTR bValue = SysAllocString(value.c_str());
    this->put_Id(bValue);
    SysFreeString(bValue);
}

CJSProperties* JSObject::Properties() {
    return m_JSProperties;
}

CJSMethods* JSObject::Methods() {
    return m_JSMethods;
}

CJSEvents* JSObject::Events() {
    return m_JSEvents;;
}

CJSObjects* JSObject::Objects() {
    return m_JSObjects;
}



JSObjectSink::JSObjectSink(JSObject* jsObject) {
	m_JSObject = jsObject;
    m_refcount = 0;
    IConnectionPointContainer* pIConnectionPointContainerTemp = NULL;
    IUnknown* pIUnknown = NULL;

    this->QueryInterface(IID_IUnknown, (void**)&pIUnknown);
    if (pIUnknown) {
        m_JSObject->QueryInterface(IID_IConnectionPointContainer, (void**)&pIConnectionPointContainerTemp);

        if (pIConnectionPointContainerTemp) {
#if defined(__MINGW32__) || defined(__MINGW64__)
            pIConnectionPointContainerTemp->FindConnectionPoint(IID_IJSObjectEvents, &m_pIConnectionPoint);
#else
            pIConnectionPointContainerTemp->FindConnectionPoint(__uuidof(IJSObjectEvents), &m_pIConnectionPoint);
#endif
            pIConnectionPointContainerTemp->Release();
            pIConnectionPointContainerTemp = NULL;
        }

        if (m_pIConnectionPoint) {
            m_pIConnectionPoint->Advise(pIUnknown, &m_dwEventCookie);
        }

        pIUnknown->Release();
        pIUnknown = NULL;
    }
}

JSObjectSink::~JSObjectSink() {
    if (m_pIConnectionPoint)
    {
        m_pIConnectionPoint->Unadvise(m_dwEventCookie);
        m_dwEventCookie = 0;
        m_pIConnectionPoint->Release();
        m_pIConnectionPoint = NULL;
    }
}

HRESULT __stdcall JSObjectSink::GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo) {
    *TypeInfo = NULL;
    return E_NOTIMPL;
}

HRESULT __stdcall JSObjectSink::GetTypeInfoCount(UINT *Count) {
    *Count = 0;
    return E_NOTIMPL;
}

HRESULT __stdcall JSObjectSink::GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs) {
    return E_NOTIMPL;
}

HRESULT __stdcall JSObjectSink::Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr) {
    if (DispID == 101) {
        IJSObject* Sender = (IJSObject*)GetAsInterface(Params, 1);
        IJSMethod* Method = (IJSMethod*)GetAsInterface(Params, 0);
        OnExecuteMethod(Sender, Method);
    }
    if (DispID == 102) {
        IJSObject* Sender = (IJSObject*)GetAsInterface(Params, 1);
        IJSProperty* Prop = (IJSProperty*)GetAsInterface(Params, 0);
        OnPropertyChange(Sender, Prop);
	}
    return S_OK;
}

ULONG __stdcall JSObjectSink::AddRef() {
    return (ULONG)InterlockedIncrement(&m_refcount);
}

ULONG __stdcall JSObjectSink::Release() {
    ULONG count = InterlockedDecrement(&m_refcount);
    if (count == 0) {
        delete this;
    }
    return count;
}

HRESULT __stdcall JSObjectSink::QueryInterface(REFIID riid, void **ppvObject) {
    *ppvObject = NULL;
    if (riid == IID_IUnknown) {
        *ppvObject = this;
        this->AddRef();
    }
#if defined(__MINGW32__) || defined(__MINGW64__)
    if ((riid == IID_IDispatch) || (riid == IID_IJSObjectEvents))
#else
    if ((riid == IID_IDispatch) || (riid == __uuidof(IJSObjectEvents)))
#endif
    {
        *ppvObject = (IDispatch*)this;
        AddRef();
        return S_OK;
    }
	return E_NOINTERFACE;
}

IDispatch* JSObjectSink::GetAsInterface(DISPPARAMS* args, int index) {
	if (V_VT(&(args->rgvarg)[index]) == VT_DISPATCH)
		return V_DISPATCH(&(args->rgvarg)[index]);
	if (V_VT(&(args->rgvarg)[index]) == (VT_BYREF | VT_DISPATCH))
		return (args->rgvarg)[index].pdispVal;
	return NULL;
}

HRESULT __stdcall JSObjectSink::OnExecuteMethod(IJSObject *Caller, IJSMethod *Method) {
    if (m_JSObject->OnExecuteMethod) {
        m_JSObject->OnExecuteMethod(Caller, Method);
    }
    return S_OK;
}

HRESULT __stdcall JSObjectSink::OnPropertyChange(IJSObject *Caller, IJSProperty *Prop) {
    if (m_JSObject->OnPropertyChange) {
        m_JSObject->OnPropertyChange(Caller, Prop);
    }
	return S_OK;
}



JSCallback::JSCallback(JSMethodCallback Proc) {
	m_refcount = 0;
	m_Proc = Proc;
}

HRESULT __stdcall JSCallback::Callback(IJSObject* Parent, IJSMethod* Method) {
	if (m_Proc != NULL)
		m_Proc(Parent, Method);
	return S_OK;
}

HRESULT __stdcall JSCallback::GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo) {
	TypeInfo = NULL;
	return E_NOTIMPL;
}

HRESULT __stdcall JSCallback::GetTypeInfoCount(UINT *Count) {
	Count = 0;
	return S_OK;
}

HRESULT __stdcall JSCallback::GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs) {
	return E_NOTIMPL;
}

HRESULT __stdcall JSCallback::Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr) {
	return E_NOTIMPL;
}

HRESULT __stdcall JSCallback::QueryInterface(REFIID riid, void **ppvObject) {
	*ppvObject = NULL;
	if (riid == IID_IUnknown) {
		*ppvObject = this;
		this->AddRef();
	}

	if (*ppvObject == NULL) {
		return E_NOINTERFACE;
	}
	return NO_ERROR;
}

ULONG __stdcall JSCallback::AddRef() {
	return (ULONG)InterlockedIncrement(&m_refcount);
}

ULONG __stdcall JSCallback::Release() {
	ULONG count = InterlockedDecrement(&m_refcount);
	if (count == 0) {
		delete this;
	}
	return count;
}




JSBinding::JSBinding(JSPropertySet Proc) {
	m_refcount = 0;
    m_Proc = Proc;
}

HRESULT __stdcall JSBinding::Set(IJSObject* Parent, IJSProperty* Prop) {
    if (m_Proc != NULL)
        m_Proc(Parent, Prop);
    return S_OK;
}

HRESULT __stdcall JSBinding::GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo) {
	TypeInfo = NULL;
	return E_NOTIMPL;
}

HRESULT __stdcall JSBinding::GetTypeInfoCount(UINT *Count) {
	Count = 0;
	return S_OK;
}

HRESULT __stdcall JSBinding::GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs) {
	return E_NOTIMPL;
}

HRESULT __stdcall JSBinding::Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr) {
	return E_NOTIMPL;
}

HRESULT __stdcall JSBinding::QueryInterface(REFIID riid, void **ppvObject) {
	*ppvObject = NULL;
	if (riid == IID_IUnknown) {
		*ppvObject = this;
		this->AddRef();
	}

	if (*ppvObject == NULL) {
		return E_NOINTERFACE;
	}
	return NO_ERROR;
}

ULONG __stdcall JSBinding::AddRef() {
	return (ULONG)InterlockedIncrement(&m_refcount);
}

ULONG __stdcall JSBinding::Release() {
	ULONG count = InterlockedDecrement(&m_refcount);
	if (count == 0) {
		delete this;
	}
	return count;
}



CJSProperties::CJSProperties(IJSObject* jsObject) {
    m_refcount = 0;
    m_JSObject = jsObject;
    if (m_JSObject != NULL)
        m_JSObject->get_Properties(&m_JSProperties);
}

CJSProperties::~CJSProperties() {
    m_JSObject = NULL;
    if (m_JSProperties != NULL) {
        m_JSProperties->Release();
    }
    m_JSProperties = NULL;
}

HRESULT __stdcall CJSProperties::GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo) {
    if (m_JSProperties != NULL)
        return m_JSProperties->GetTypeInfo(Index, LocaleID, TypeInfo);
    else {
        TypeInfo = NULL;
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CJSProperties::GetTypeInfoCount(UINT *Count) {
    if (m_JSProperties != NULL)
        return m_JSProperties->GetTypeInfoCount(Count);
    else {
        Count = 0;
        return 0;
    }
}

HRESULT __stdcall CJSProperties::GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs) {
    if (m_JSProperties != NULL)
        return m_JSProperties->GetIDsOfNames(IID, Names, NameCount, LocaleID, DispIDs);
    else {
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CJSProperties::Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr) {
    if (m_JSProperties != NULL)
        return m_JSProperties->Invoke(DispID, IID, LocaleID, Flags, Params, VarResult, ExcepInfo, ArgErr);
    else {
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CJSProperties::QueryInterface(REFIID riid, void **ppvObject) {
    if (m_JSProperties != NULL)
        return m_JSProperties->QueryInterface(riid, ppvObject);
    else {
        return E_NOINTERFACE;
    }
}

ULONG __stdcall CJSProperties::AddRef() {
    return (ULONG)InterlockedIncrement(&m_refcount);
}

ULONG __stdcall CJSProperties::Release() {
    ULONG count = InterlockedDecrement(&m_refcount);
    if (count == 0) {
        delete this;
    }
    return count;
}

HRESULT __stdcall CJSProperties::get_Count(long *Value) {
    if (m_JSProperties != NULL)
        return m_JSProperties->get_Count(Value);
    return S_OK;
}

HRESULT __stdcall CJSProperties::get_Item(VARIANT Index, IJSProperty **Value) {
    if (m_JSProperties != NULL)
        return m_JSProperties->get_Item(Index, Value);
    return S_OK;
}

HRESULT __stdcall CJSProperties::Add(BSTR Name, IJSProperty **OutRetVal) {
    if (m_JSProperties != NULL)
        return m_JSProperties->Add(Name, OutRetVal);
    return S_OK;
}

long CJSProperties::Count() {
    long res = 0;
    this->get_Count(&res);
    return res;
}

IJSProperty* CJSProperties::Item(long Index) {
    IJSProperty* res = NULL;
    VARIANT varValue;
    VariantInit(&varValue);
    V_VT(&varValue) = VT_I4;
    V_I4(&varValue) = Index;
    this->get_Item(varValue, &res);
    return res;
}

IJSProperty* CJSProperties::Item(std::wstring Name) {
    BSTR bName = SysAllocString(Name.c_str());
    IJSProperty* res = NULL;
    VARIANT varValue;
    VariantInit(&varValue);
    V_VT(&varValue) = VT_BSTR;
    V_BSTR(&varValue) = bName;
    this->get_Item(varValue, &res);
    SysFreeString(bName);
    return res;
}

IJSProperty* CJSProperties::Add(std::wstring Name) {
    IJSProperty* res = NULL;
    BSTR bName = SysAllocString(Name.c_str());
    this->Add(bName, &res);
    SysFreeString(bName);
    return res;
}

HRESULT __stdcall CJSProperties::Clear(void) {
    if (m_JSProperties != NULL)
        return m_JSProperties->Clear();
    return S_OK;
}

HRESULT __stdcall CJSProperties::Remove(IJSProperty *Item) {
    if (m_JSProperties != NULL)
        return m_JSProperties->Remove(Item);
    return S_OK;
}



CJSMethods::CJSMethods(IJSObject* jsObject) {
    m_refcount = 0;
    m_JSObject = jsObject;
    if (m_JSObject != NULL)
        m_JSObject->get_Methods(&m_JSMethods);
}

CJSMethods::~CJSMethods() {
    m_JSObject = NULL;
    if (m_JSMethods != NULL) {
        m_JSMethods->Release();
    }
    m_JSMethods = NULL;
}

HRESULT __stdcall CJSMethods::GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo) {
    if (m_JSMethods != NULL)
        return m_JSMethods->GetTypeInfo(Index, LocaleID, TypeInfo);
    else {
        TypeInfo = NULL;
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CJSMethods::GetTypeInfoCount(UINT *Count) {
    if (m_JSMethods != NULL)
        return m_JSMethods->GetTypeInfoCount(Count);
    else {
        Count = 0;
        return 0;
    }
}

HRESULT __stdcall CJSMethods::GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs) {
    if (m_JSMethods != NULL)
        return m_JSMethods->GetIDsOfNames(IID, Names, NameCount, LocaleID, DispIDs);
    else {
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CJSMethods::Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr) {
    if (m_JSMethods != NULL)
        return m_JSMethods->Invoke(DispID, IID, LocaleID, Flags, Params, VarResult, ExcepInfo, ArgErr);
    else {
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CJSMethods::QueryInterface(REFIID riid, void **ppvObject) {
    if (m_JSMethods != NULL)
        return m_JSMethods->QueryInterface(riid, ppvObject);
    else {
        return E_NOINTERFACE;
    }
}

ULONG __stdcall CJSMethods::AddRef() {
    return (ULONG)InterlockedIncrement(&m_refcount);
}

ULONG __stdcall CJSMethods::Release() {
    ULONG count = InterlockedDecrement(&m_refcount);
    if (count == 0) {
        delete this;
    }
    return count;
}

HRESULT __stdcall CJSMethods::get_Count(long *Value) {
    if (m_JSMethods != NULL)
        return m_JSMethods->get_Count(Value);
    return S_OK;
}

HRESULT __stdcall CJSMethods::get_Item(VARIANT Index, IJSMethod **Value) {
    if (m_JSMethods != NULL)
        return m_JSMethods->get_Item(Index, Value);
    return S_OK;
}

HRESULT __stdcall CJSMethods::Add(BSTR Name, IJSMethod **OutRetVal) {
    if (m_JSMethods != NULL)
        return m_JSMethods->Add(Name, OutRetVal);
    return S_OK;
}

long CJSMethods::Count() {
    long res = 0;
    this->get_Count(&res);
    return res;
}

IJSMethod* CJSMethods::Item(long Index) {
    IJSMethod* res = NULL;
    VARIANT varValue;
    VariantInit(&varValue);
    V_VT(&varValue) = VT_I4;
    V_I4(&varValue) = Index;
    this->get_Item(varValue, &res);
    return res;
}

IJSMethod* CJSMethods::Item(std::wstring Name) {
    BSTR bName = SysAllocString(Name.c_str());
    IJSMethod* res = NULL;
    VARIANT varValue;
    VariantInit(&varValue);
    V_VT(&varValue) = VT_BSTR;
    V_BSTR(&varValue) = bName;
    this->get_Item(varValue, &res);
    SysFreeString(bName);
    return res;
}

IJSMethod* CJSMethods::Add(std::wstring Name) {
    IJSMethod* res = NULL;
    BSTR bName = SysAllocString(Name.c_str());
    this->Add(bName, &res);
    SysFreeString(bName);
    return res;
}

HRESULT __stdcall CJSMethods::Clear(void) {
    if (m_JSMethods != NULL)
        return m_JSMethods->Clear();
    return S_OK;
}

HRESULT __stdcall CJSMethods::Remove(IJSMethod *Item) {
    if (m_JSMethods != NULL)
        return m_JSMethods->Remove(Item);
    return S_OK;
}




CJSEvents::CJSEvents(IJSObject* jsObject) {
    m_refcount = 0;
    m_JSObject = jsObject;
    if (m_JSObject != NULL)
        m_JSObject->get_Events(&m_JSEvents);
}

CJSEvents::~CJSEvents() {
    m_JSObject = NULL;
    if (m_JSEvents != NULL) {
        m_JSEvents->Release();
    }
    m_JSEvents = NULL;
}

HRESULT __stdcall CJSEvents::GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo) {
    if (m_JSEvents != NULL)
        return m_JSEvents->GetTypeInfo(Index, LocaleID, TypeInfo);
    else {
        TypeInfo = NULL;
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CJSEvents::GetTypeInfoCount(UINT *Count) {
    if (m_JSEvents != NULL)
        return m_JSEvents->GetTypeInfoCount(Count);
    else {
        Count = 0;
        return 0;
    }
}

HRESULT __stdcall CJSEvents::GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs) {
    if (m_JSEvents != NULL)
        return m_JSEvents->GetIDsOfNames(IID, Names, NameCount, LocaleID, DispIDs);
    else {
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CJSEvents::Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr) {
    if (m_JSEvents != NULL)
        return m_JSEvents->Invoke(DispID, IID, LocaleID, Flags, Params, VarResult, ExcepInfo, ArgErr);
    else {
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CJSEvents::QueryInterface(REFIID riid, void **ppvObject) {
    if (m_JSEvents != NULL)
        return m_JSEvents->QueryInterface(riid, ppvObject);
    else {
        return E_NOINTERFACE;
    }
}

ULONG __stdcall CJSEvents::AddRef() {
    return (ULONG)InterlockedIncrement(&m_refcount);
}

ULONG __stdcall CJSEvents::Release() {
    ULONG count = InterlockedDecrement(&m_refcount);
    if (count == 0) {
        delete this;
    }
    return count;
}

HRESULT __stdcall CJSEvents::get_Count(long *Value) {
    if (m_JSEvents != NULL)
        return m_JSEvents->get_Count(Value);
    return S_OK;
}

HRESULT __stdcall CJSEvents::get_Item(VARIANT Index, IJSEvent **Value) {
    if (m_JSEvents != NULL)
        return m_JSEvents->get_Item(Index, Value);
    return S_OK;
}

HRESULT __stdcall CJSEvents::Add(BSTR Name, IJSEvent **OutRetVal) {
    if (m_JSEvents != NULL)
        return m_JSEvents->Add(Name, OutRetVal);
    return S_OK;
}

long CJSEvents::Count() {
    long res = 0;
    this->get_Count(&res);
    return res;
}

IJSEvent* CJSEvents::Item(long Index) {
    IJSEvent* res = NULL;
    VARIANT varValue;
    VariantInit(&varValue);
    V_VT(&varValue) = VT_I4;
    V_I4(&varValue) = Index;
    this->get_Item(varValue, &res);
    return res;
}

IJSEvent* CJSEvents::Item(std::wstring Name) {
    BSTR bName = SysAllocString(Name.c_str());
    IJSEvent* res = NULL;
    VARIANT varValue;
    VariantInit(&varValue);
    V_VT(&varValue) = VT_BSTR;
    V_BSTR(&varValue) = bName;
    this->get_Item(varValue, &res);
    SysFreeString(bName);
    return res;
}

IJSEvent* CJSEvents::Add(std::wstring Name) {
    IJSEvent* res = NULL;
    BSTR bName = SysAllocString(Name.c_str());
    this->Add(bName, &res);
    SysFreeString(bName);
    return res;
}

HRESULT __stdcall CJSEvents::Clear(void) {
    if (m_JSEvents != NULL)
        return m_JSEvents->Clear();
    return S_OK;
}

HRESULT __stdcall CJSEvents::Remove(IJSEvent *Item) {
    if (m_JSEvents != NULL)
        return m_JSEvents->Remove(Item);
    return S_OK;
}



CJSObjects::CJSObjects(IJSObject* jsObject) {
    m_refcount = 0;
    m_JSObject = jsObject;
    if (m_JSObject != NULL)
        m_JSObject->get_Objects(&m_JSObjects);
}

CJSObjects::~CJSObjects() {
    m_JSObject = NULL;
    if (m_JSObjects != NULL) {
        m_JSObjects->Release();
    }
    m_JSObjects = NULL;
}

HRESULT __stdcall CJSObjects::GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo) {
    if (m_JSObjects != NULL)
        return m_JSObjects->GetTypeInfo(Index, LocaleID, TypeInfo);
    else {
        TypeInfo = NULL;
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CJSObjects::GetTypeInfoCount(UINT *Count) {
    if (m_JSObjects != NULL)
        return m_JSObjects->GetTypeInfoCount(Count);
    else {
        Count = 0;
        return 0;
    }
}

HRESULT __stdcall CJSObjects::GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs) {
    if (m_JSObjects != NULL)
        return m_JSObjects->GetIDsOfNames(IID, Names, NameCount, LocaleID, DispIDs);
    else {
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CJSObjects::Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr) {
    if (m_JSObjects != NULL)
        return m_JSObjects->Invoke(DispID, IID, LocaleID, Flags, Params, VarResult, ExcepInfo, ArgErr);
    else {
        return E_NOTIMPL;
    }
}

HRESULT __stdcall CJSObjects::QueryInterface(REFIID riid, void **ppvObject) {
    if (m_JSObjects != NULL)
        return m_JSObjects->QueryInterface(riid, ppvObject);
    else {
        return E_NOINTERFACE;
    }
}

ULONG __stdcall CJSObjects::AddRef() {
    return (ULONG)InterlockedIncrement(&m_refcount);
}

ULONG __stdcall CJSObjects::Release() {
    ULONG count = InterlockedDecrement(&m_refcount);
    if (count == 0) {
        delete this;
    }
    return count;
}

HRESULT __stdcall CJSObjects::get_Count(long *Value) {
    if (m_JSObjects != NULL)
        return m_JSObjects->get_Count(Value);
    return S_OK;
}

HRESULT __stdcall CJSObjects::get_Item(VARIANT Index, IJSObject **Value) {
    if (m_JSObjects != NULL)
        return m_JSObjects->get_Item(Index, Value);
    return S_OK;
}

HRESULT __stdcall CJSObjects::Add(BSTR Name, IJSObject **OutRetVal) {
    if (m_JSObjects != NULL)
        return m_JSObjects->Add(Name, OutRetVal);
    return S_OK;
}

long CJSObjects::Count() {
    long res = 0;
    this->get_Count(&res);
    return res;
}

IJSObject* CJSObjects::Item(long Index) {
    IJSObject* res = NULL;
    VARIANT varValue;
    VariantInit(&varValue);
    V_VT(&varValue) = VT_I4;
    V_I4(&varValue) = Index;
    this->get_Item(varValue, &res);
    return res;
}

IJSObject* CJSObjects::Item(std::wstring Name) {
    BSTR bName = SysAllocString(Name.c_str());
    IJSObject* res = NULL;
    VARIANT varValue;
    VariantInit(&varValue);
    V_VT(&varValue) = VT_BSTR;
    V_BSTR(&varValue) = bName;
    this->get_Item(varValue, &res);
    SysFreeString(bName);
    return res;
}

IJSObject* CJSObjects::Add(std::wstring Name) {
    IJSObject* res = NULL;
    BSTR bName = SysAllocString(Name.c_str());
    this->Add(bName, &res);
    SysFreeString(bName);
    return res;
}

HRESULT __stdcall CJSObjects::Clear(void) {
    if (m_JSObjects != NULL)
        return m_JSObjects->Clear();
    return S_OK;
}

HRESULT __stdcall CJSObjects::Remove(IJSObject *Item) {
    if (m_JSObjects != NULL)
        return m_JSObjects->Remove(Item);
    return S_OK;
}




CHTMLDoc::CHTMLDoc(IVirtualUI* virtualUI) {
	m_VirtualUI = virtualUI;
	m_HTMLDoc = NULL;
	m_refcount = 0;
	if (m_VirtualUI != NULL) {
		m_VirtualUI->Get_HTMLDoc(&m_HTMLDoc);
	}
}

CHTMLDoc::~CHTMLDoc() {
	if (m_VirtualUI != NULL)
		m_VirtualUI->Release();
	m_HTMLDoc = NULL;
	m_VirtualUI = NULL;
}

HRESULT __stdcall CHTMLDoc::GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo) {
	if (m_HTMLDoc != NULL)
		return m_HTMLDoc->GetTypeInfo(Index, LocaleID, TypeInfo);
	else {
		TypeInfo = NULL;
		return E_NOTIMPL;
	}
}

HRESULT __stdcall CHTMLDoc::GetTypeInfoCount(UINT *Count) {
	if (m_HTMLDoc != NULL)
		return m_HTMLDoc->GetTypeInfoCount(Count);
	else {
		Count = 0;
		return 0;
	}
}

HRESULT __stdcall CHTMLDoc::GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs) {
	if (m_HTMLDoc != NULL)
		return m_HTMLDoc->GetIDsOfNames(IID, Names, NameCount, LocaleID, DispIDs);
	else {
		return E_NOTIMPL;
	}
}

HRESULT __stdcall CHTMLDoc::Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr) {
	if (m_HTMLDoc != NULL)
		return m_HTMLDoc->Invoke(DispID, IID, LocaleID, Flags, Params, VarResult, ExcepInfo, ArgErr);
	else {
		return E_NOTIMPL;
	}
}

HRESULT __stdcall CHTMLDoc::QueryInterface(REFIID riid, void **ppvObject) {
	*ppvObject = NULL;
	if (riid == IID_IUnknown) {
		*ppvObject = this;
		this->AddRef();
	}

	if (*ppvObject == NULL) {
		return E_NOINTERFACE;
	}
	return NO_ERROR;
}

ULONG __stdcall CHTMLDoc::AddRef() {
	return (ULONG)InterlockedIncrement(&m_refcount);
}

ULONG __stdcall CHTMLDoc::Release() {
	ULONG count = InterlockedDecrement(&m_refcount);
	if (count == 0) {
		delete this;
	}
	return count;
}

HRESULT __stdcall CHTMLDoc::CreateSessionURL(BSTR Url, BSTR Filename) {
	if (m_HTMLDoc != NULL)
		m_HTMLDoc->CreateSessionURL(Url, Filename);
	return S_OK;
}

HRESULT __stdcall CHTMLDoc::CreateComponent(BSTR Id, BSTR Html, unsigned int ReplaceWnd) {
	if (m_HTMLDoc != NULL)
		m_HTMLDoc->CreateComponent(Id, Html, ReplaceWnd);
	return S_OK;
}

HRESULT __stdcall CHTMLDoc::GetSafeURL(BSTR Filename, int Minutes, BSTR* Value) {
	if (m_HTMLDoc != NULL)
		m_HTMLDoc->GetSafeURL(Filename, Minutes, Value);
	return S_OK;
}

HRESULT __stdcall CHTMLDoc::CreateForwarding(BSTR Url, BSTR* Value) {
	if (m_HTMLDoc != NULL)
		m_HTMLDoc->CreateForwarding(Url, Value);
	return S_OK;
}

HRESULT __stdcall CHTMLDoc::LoadScript(BSTR Url, BSTR Filename) {
	if (m_HTMLDoc != NULL)
		m_HTMLDoc->LoadScript(Url, Filename);
	return S_OK;
}

HRESULT __stdcall CHTMLDoc::ImportHTML(BSTR Url, BSTR Filename) {
	if (m_HTMLDoc != NULL)
		m_HTMLDoc->ImportHTML(Url, Filename);
	return S_OK;
}

void CHTMLDoc::CreateSessionURL(std::wstring Url, std::wstring Filename) {
	BSTR url = SysAllocString(Url.c_str());
	BSTR filename = SysAllocString(Filename.c_str());
	this->CreateSessionURL(url, filename);
	SysFreeString(filename);
	SysFreeString(url);
}

void CHTMLDoc::CreateComponent(std::wstring Id, std::wstring Html, unsigned int ReplaceWnd) {
	BSTR id = SysAllocString(Id.c_str());
	BSTR html = SysAllocString(Html.c_str());
	this->CreateComponent(id, html, ReplaceWnd);
	SysFreeString(id);
	SysFreeString(html);
}

std::wstring CHTMLDoc::GetSafeURL(std::wstring Filename, int Minutes) {
	BSTR value;
	BSTR filename = SysAllocString(Filename.c_str());
	this->GetSafeURL(filename, Minutes, &value);
	std::wstring ret(value);
	SysFreeString(filename);
	SysFreeString(value);
	return ret;
}

void CHTMLDoc::LoadScript(std::wstring Url, std::wstring Filename) {
	BSTR url = SysAllocString(Url.c_str());
	BSTR filename = SysAllocString(Filename.c_str());
	this->LoadScript(url, filename);
	SysFreeString(filename);
	SysFreeString(url);
}

void CHTMLDoc::ImportHTML(std::wstring Url, std::wstring Filename) {
	BSTR url = SysAllocString(Url.c_str());
	BSTR filename = SysAllocString(Filename.c_str());
	this->ImportHTML(url, filename);
	SysFreeString(filename);
	SysFreeString(url);
}

std::wstring CHTMLDoc::CreateForwarding(std::wstring Url) {
    BSTR value;
    BSTR url = SysAllocString(Url.c_str());
    this->CreateForwarding(url, &value);
    std::wstring ret(value);
    SysFreeString(url);
    SysFreeString(value);
    return ret;
}




std::wstring VUITrackName(IRecTrack* track) {
  std::wstring ret;
	BSTR res;
	track->get_Name(&res);
	if (res != NULL)
		ret = std::wstring(res);
	SysFreeString(res);
  return ret;
}

long VUITrackPosition(IRecTrack* track) {
  long res;
  track->get_Position(&res);
  return res;
}


std::wstring JSROGetArgumentAsString(IJSMethod* AMethod, std::wstring argName) {
	std::wstring ret;
	IJSArguments* args;
	AMethod->get_Arguments(&args);

	IJSArgument* arg;
	BSTR bValue = SysAllocString(argName.c_str());
	VARIANT varValue;
	VariantInit(&varValue);
	V_VT(&varValue) = VT_BSTR;
	V_BSTR(&varValue) = bValue;
	args->get_Item(varValue, &arg);

	BSTR argValue;
	arg->get_AsString(&argValue);
	if (argValue != NULL)
		ret = std::wstring(argValue);
	SysFreeString(argValue);

	return ret;
}

int JSROGetArgumentAsInt(IJSMethod* AMethod, std::wstring argName) {
	int ret;
	std::wstring strRes = JSROGetArgumentAsString(AMethod, argName);
	ret = _wtoi(strRes.c_str());
	return ret;
}

bool JSROGetArgumentAsBool(IJSMethod* AMethod, std::wstring argName) {
	std::wstring strRes = JSROGetArgumentAsString(AMethod, argName);
	return (strRes == L"1") || (strRes == L"True") || (strRes == L"true");
}

float JSROGetArgumentAsFloat(IJSMethod* AMethod, std::wstring argName) {
	float ret;
	std::wstring strRes = JSROGetArgumentAsString(AMethod, argName);
#if (_MSC_VER == 1200)
	//Visual Studio 6
	wchar_t* end;
	ret = wcstod(strRes.c_str(), &end);
#else
	ret = (float)_wtof(strRes.c_str());
#endif
	return ret;
}


IJSEvent* JSROGetEvent(JSObject* AObject, std::wstring eventName) {
	BSTR bValue = SysAllocString(eventName.c_str());
	VARIANT varValue;
	VariantInit(&varValue);
	V_VT(&varValue) = VT_BSTR;
	V_BSTR(&varValue) = bValue;

	IJSEvent* ret;
	AObject->Events()->get_Item(varValue, &ret);
	SysFreeString(bValue);
	return ret;
}


void JSROSetEventArgAsString(IJSEvent* AEvent, std::wstring argName, std::wstring argValue) {
	BSTR bValue = SysAllocString(argName.c_str());
	VARIANT varValue;
	VariantInit(&varValue);
	V_VT(&varValue) = VT_BSTR;
	V_BSTR(&varValue) = bValue;

	BSTR bArgValue = SysAllocString(argValue.c_str());
	AEvent->ArgumentAsString(varValue, bArgValue, &AEvent);
	SysFreeString(bArgValue);
}

void JSROSetEventArgAsInt(IJSEvent* AEvent, std::wstring argName, int argValue) {
	BSTR bValue = SysAllocString(argName.c_str());
	VARIANT varValue;
	VariantInit(&varValue);
	V_VT(&varValue) = VT_BSTR;
	V_BSTR(&varValue) = bValue;

	AEvent->ArgumentAsInt(varValue, argValue, &AEvent);
	SysFreeString(bValue);
}

void JSROSetEventArgAsBool(IJSEvent* AEvent, std::wstring argName, bool argValue) {
	BSTR bValue = SysAllocString(argName.c_str());
	VARIANT varValue;
	VariantInit(&varValue);
	V_VT(&varValue) = VT_BSTR;
	V_BSTR(&varValue) = bValue;

	VARIANT_BOOL argV = (argValue ? TRUE : FALSE);
	AEvent->ArgumentAsBool(varValue, argV, &AEvent);
	SysFreeString(bValue);
}

void JSROSetEventArgAsFloat(IJSEvent* AEvent, std::wstring argName, float argValue) {
	BSTR bValue = SysAllocString(argName.c_str());
	VARIANT varValue;
	VariantInit(&varValue);
	V_VT(&varValue) = VT_BSTR;
	V_BSTR(&varValue) = bValue;

	AEvent->ArgumentAsFloat(varValue, argValue, &AEvent);
	SysFreeString(bValue);
}


void JSROAddMethodParameter(IJSMethod* AMethod, std::wstring paramName, IJSDataType ADataType) {
	BSTR bName = SysAllocString(paramName.c_str());
	AMethod->AddArgument(bName, ADataType, &AMethod);
	SysFreeString(bName);
}

void JSROSetMethodReturnType(IJSMethod* AMethod, IJSDataType ADataType) {
	IJSValue* value;
	AMethod->get_ReturnValue(&value);
	value->put_DataType(ADataType);
}

void JSROAddEventArgument(IJSEvent* AEvent, std::wstring argName, IJSDataType ADataType) {
	BSTR bValue = SysAllocString(argName.c_str());
	AEvent->AddArgument(bValue, ADataType, &AEvent);
	SysFreeString(bValue);
}


void JSROSetPropAsString(IJSProperty* AProperty, std::wstring propValue) {
	BSTR bArgValue = SysAllocString(propValue.c_str());
	AProperty->put_AsString(bArgValue);
	SysFreeString(bArgValue);
}

void JSROSetPropAsInt(IJSProperty* AProperty, int propValue) {
	AProperty->put_AsInt(propValue);
}

void JSROSetPropAsBool(IJSProperty* AProperty, bool propValue) {
	VARIANT_BOOL argV = (propValue ? TRUE : FALSE);
	AProperty->put_AsBool(argV);
}

void JSROSetPropAsFloat(IJSProperty* AProperty, float propValue) {
	AProperty->put_AsFloat(propValue);
}


std::wstring JSROGetPropertyName(IJSProperty* AProperty) {
	BSTR bName;
	AProperty->get_Name(&bName);
	std::wstring name = std::wstring(bName);
	SysFreeString(bName);
	return name;
}


std::wstring JSROGetPropAsString(IJSProperty* AProperty) {
	std::wstring ret;
	BSTR propValue;
	AProperty->get_AsString(&propValue);
	if (propValue != NULL)
		ret = std::wstring(propValue);
	SysFreeString(propValue);
	return ret;
}

int JSROGetPropAsInt(IJSProperty* AProperty) {
	long ret = 0;
	AProperty->get_AsInt(&ret);
	return ret;
}

bool JSROGetPropAsBool(IJSProperty* AProperty) {
	VARIANT_BOOL ret;
	AProperty->get_AsBool(&ret);
	return (ret == TRUE);
}

float JSROGetPropAsFloat(IJSProperty* AProperty) {
	float ret = 0;
	AProperty->get_AsFloat(&ret);
	return ret;
}
