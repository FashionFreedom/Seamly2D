#ifndef __THINFINITY_VIRTUALUI_H
#define __THINFINITY_VIRTUALUI_H

#if defined(__MINGW32__) || defined(__MINGW64__)
#include <initguid.h>
#endif

#include "rpc.h"
#include "rpcndr.h"
#include <string>

#ifndef COM_NO_WINDOWS_H
#include "windows.h"
#include "ole2.h"
#endif /*COM_NO_WINDOWS_H*/

#if defined(__MINGW32__) || defined(__MINGW64__)
// No pragma for autolink in MinGW. Needed libraries: oleaut32, shlwapi, uuid
#else
#ifdef __BORLANDC__
  #pragma comment(lib,"shlwapi.lib")
#else
  #if (_MSC_VER == 1200)
    //Visual Studio 6
    #pragma comment(lib,"shlwapi.lib")
  #endif
  #pragma comment(lib,"Advapi32")
  #pragma comment(lib,"OleAut32")
#endif
#endif

/*
* *********************************************************************************************
*  Thinfinity_TLB translation
* *********************************************************************************************
*/

/* Forward Declarations */

typedef interface IVirtualUI IVirtualUI;
typedef interface IBrowserInfo IBrowserInfo;
typedef interface IDevServer IDevServer;
typedef interface IClientSettings IClientSettings;
typedef interface IJSValue IJSValue;
typedef interface IJSNamedValue IJSNamedValue;
typedef interface IJSObject IJSObject;
typedef interface IJSObjects IJSObjects;
typedef interface IJSProperty IJSProperty;
typedef interface IJSProperties IJSProperties;
typedef interface IJSArgument IJSArgument;
typedef interface IJSArguments IJSArguments;
typedef interface IJSMethod IJSMethod;
typedef interface IJSMethods IJSMethods;
typedef interface IJSEvent IJSEvent;
typedef interface IJSEvents IJSEvents;
typedef interface IJSBinding IJSBinding;
typedef interface IJSCallback IJSCallback;
typedef interface IEvents IEvents;
typedef interface IJSObjectEvents IJSObjectEvents;
typedef interface IRecTrack IRecTrack;
typedef interface IRecTracks IRecTracks;
typedef interface IRecorder IRecorder;
typedef interface IFileSystemFilter IFileSystemFilter;
typedef interface IRegistryFilter IRegistryFilter;
typedef interface IHTMLDoc IHTMLDoc;
typedef interface IImageDelivery IImageDelivery;
typedef interface IImageEncoding IImageEncoding;
typedef interface IPerformance IPerformance;

typedef class VirtualUI VirtualUI;
typedef class JSObject JSObject;
typedef class CJSProperties CJSProperties;
typedef class CJSMethods CJSMethods;
typedef class CJSEvents CJSEvents;
typedef class CJSObjects CJSObjects;
typedef class CRecTracks CRecTracks;
typedef class CRecorder CRecorder;
typedef class CFileSystemFilter CFileSystemFilter;
typedef class CRegistryFilter CRegistryFilter;
typedef class CHTMLDoc CHTMLDoc;

/* header files for imported files */
#include "ocidl.h"

#ifdef __cplusplus
extern "C" {
#endif
	enum IJSDataType
	{
		JSDT_NULL = 0,
		JSDT_STRING = 1,
		JSDT_INT = 2,
		JSDT_BOOL = 3,
		JSDT_FLOAT = 4,
		JSDT_JSON = 5
	};

	enum Orientation
	{
		PORTRAIT = 0,
		LANDSCAPE = 1
	};

	enum MouseMoveGestureStyle
	{
		MM_STYLE_RELATIVE = 0,
		MM_STYLE_ABSOLUTE = 1
	};

	enum MouseMoveGestureAction
	{
		MM_ACTION_MOVE = 0,
		MM_ACTION_WHEEL = 1
	};

	enum DragAction
	{
		DRAG_START = 0,
		DRAG_OVER = 1,
		DRAG_DROP = 2,
		DRAG_CANCEL = 3,
		DRAG_ERROR = 99
	};

	enum ImageEncodingType
	{
		ET_JPEG = 0,
		ET_PNG = 1,
		ET_MIXED = 2
	};

	extern RPC_IF_HANDLE __MIDL_itf_Thinfinity2EVirtualUI_0000_0000_v0_0_c_ifspec;
	extern RPC_IF_HANDLE __MIDL_itf_Thinfinity2EVirtualUI_0000_0000_v0_0_s_ifspec;

	/* interface IVirtualUI */
	/* [object][oleautomation][dual][helpstring][uuid] */

	EXTERN_C const IID IID_IVirtualUI;

	MIDL_INTERFACE("4B85F81B-72A2-4FCD-9A6B-9CAC24B7A511")
	IVirtualUI : public IDispatch
	{
	public:
		virtual /* [id] */ HRESULT __stdcall Start(
			/* [in] */ long Timeout,
			/* [retval][out] */ VARIANT_BOOL *OutRetVal) = 0;

		virtual /* [id] */ HRESULT __stdcall Stop(void) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Active(
			/* [retval][out] */ VARIANT_BOOL *Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Enabled(
			/* [retval][out] */ VARIANT_BOOL *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_Enabled(
			/* [in] */ VARIANT_BOOL Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_DevMode(
			/* [retval][out] */ VARIANT_BOOL *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_DevMode(
			/* [in] */ VARIANT_BOOL Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_StdDialogs(
			/* [retval][out] */ VARIANT_BOOL *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_StdDialogs(
			/* [in] */ VARIANT_BOOL Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_BrowserInfo(
			/* [retval][out] */ IBrowserInfo **Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_DevServer(
			/* [retval][out] */ IDevServer **Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_ClientSettings(
			/* [retval][out] */ IClientSettings **Value) = 0;

		virtual /* [id] */ HRESULT __stdcall DownloadFile(
			/* [in] */ BSTR LocalFilename,
			/* [in] */ BSTR RemoteFilename,
			/* [in] */ BSTR MimeType) = 0;

		virtual /* [id] */ HRESULT __stdcall PrintPdf(
			/* [in] */ BSTR AFileName) = 0;

		virtual /* [id] */ HRESULT __stdcall OpenLinkDlg(
			/* [in] */ BSTR url,
			/* [in] */ BSTR caption) = 0;

		virtual /* [id] */ HRESULT __stdcall SendMessage(
			/* [in] */ BSTR Data) = 0;

		virtual /* [id] */ HRESULT __stdcall AllowExecute(
			/* [in] */ BSTR Filename) = 0;

		virtual /* [id] */ HRESULT __stdcall SetImageQualityByWnd(
			/* [in] */ long Wnd,
			/* [in] */ BSTR Class,
			/* [in] */ long Quality) = 0;

		virtual /* [id] */ HRESULT __stdcall Uploadfile(
			/* [in] */ BSTR ServerDirectory) = 0;

		virtual /* [id] */ HRESULT __stdcall TakeScreenshot(
			/* [in] */ long Wnd,
			/* [in] */ BSTR FileName,
			/* [retval][out] */ VARIANT_BOOL *OutRetVal) = 0;

		virtual /* [id] */ HRESULT __stdcall ShowVirtualKeyboard(void) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Recorder(
			/* [retval][out] */ IRecorder **Value) = 0;

		virtual /* [id] */ HRESULT __stdcall PreviewPdf(
			/* [in] */ BSTR AFileName) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_FileSystemFilter(
			/* [retval][out] */ IFileSystemFilter **Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_RegistryFilter(
			/* [retval][out] */ IRegistryFilter **Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall Get_Options(
			/* [retval][out] */ long *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall Set_Options(
			/* [in] */ long Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall Get_HTMLDoc(
			/* [retval][out] */ IHTMLDoc **Value) = 0;

		virtual /* [id] */ HRESULT __stdcall UploadFileEx(
			/* [in] */ BSTR ServerDirectory,
			/* [out] */ BSTR* FileName,
			/* [retval][out] */ VARIANT_BOOL *OutRetVal) = 0;
		virtual /* [id] */ HRESULT __stdcall Suspend() = 0;
		virtual /* [id] */ HRESULT __stdcall Resume() = 0;
		virtual /* [id] */ HRESULT __stdcall FlushWindow(
			/* [in] */ long Wnd,
			/* [retval][out */ VARIANT_BOOL *OutRetVal) = 0;

		virtual /* [id] */ HRESULT __stdcall Get_Performance(
			/* [retval][out */ IPerformance** OutRetVal) = 0;
	};

	/* interface IBrowserInfo */
	/* [object][oleautomation][dual][uuid] */

	EXTERN_C const IID IID_IBrowserInfo;

	MIDL_INTERFACE("4D9F5347-460B-4275-BDF2-F2738E7F6757")
	IBrowserInfo : public IDispatch
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_ViewWidth(
			/* [retval][out] */ long *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_ViewWidth(
			/* [in] */ long Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_ViewHeight(
			/* [retval][out] */ long *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_ViewHeight(
			/* [in] */ long Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_BrowserWidth(
			/* [retval][out] */ long *Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_BrowserHeight(
			/* [retval][out] */ long *Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_ScreenWidth(
			/* [retval][out] */ long *Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_ScreenHeight(
			/* [retval][out] */ long *Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Username(
			/* [retval][out] */ BSTR *Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_IPAddress(
			/* [retval][out] */ BSTR *Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_UserAgent(
			/* [retval][out] */ BSTR *Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_ScreenResolution(
			/* [retval][out] */ long *Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Orientation(
		/* [retval][out] */ enum Orientation *Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_UniqueBrowserId(
			/* [retval][out] */ BSTR *Value) = 0;

		virtual /* [id] */ HRESULT __stdcall GetCookie(
			/* [in] */ BSTR Name,
			/* [retval][out] */ BSTR *Value) = 0;

		virtual /* [id] */ HRESULT __stdcall SetCookie(
			/* [in] */ BSTR Name,
			/* [in] */ BSTR Value,
			/* [in] */ BSTR Expires) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Location(
			/* [retval][out] */ BSTR *Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_CustomData(
			/* [retval][out] */ BSTR *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_CustomData(
			/* [in] */ BSTR Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_SelectedRule(
			/* [retval][out] */ BSTR* Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_ExtraData(
			/* [retval][out] */ BSTR *Value) = 0;

		virtual /* [id] */ HRESULT __stdcall GetExtraDataValue(
			/* [in] */ BSTR Name,
			/* [retval][out] */ BSTR *Value) = 0;
	};

	/* interface IDevServer */
	/* [object][oleautomation][dual][uuid] */

	EXTERN_C const IID IID_IDevServer;

	MIDL_INTERFACE("B3EAC0CA-D7AB-4AB1-9E24-84A63C8C3F80")
	IDevServer : public IDispatch
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_Enabled(
			/* [retval][out] */ VARIANT_BOOL *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_Enabled(
			/* [in] */ VARIANT_BOOL Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Port(
			/* [retval][out] */ long *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_Port(
			/* [in] */ long Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_StartBrowser(
			/* [retval][out] */ VARIANT_BOOL *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_StartBrowser(
			/* [in] */ VARIANT_BOOL Value) = 0;
	};

	/* interface IClientSettings */
	/* [object][oleautomation][dual][uuid] */

	EXTERN_C const IID IID_IClientSettings;

	MIDL_INTERFACE("439624CA-ED33-47BE-9211-91290F29584A")
	IClientSettings : public IDispatch
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_MouseMoveGestureStyle(
		/* [retval][out] */ enum MouseMoveGestureStyle *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_MouseMoveGestureStyle(
		/* [in] */ enum MouseMoveGestureStyle Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_MouseMoveGestureAction(
		/* [retval][out] */ enum MouseMoveGestureAction *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_MouseMoveGestureAction(
		/* [in] */ enum MouseMoveGestureAction Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_CursorVisible(
			/* [retval][out] */ VARIANT_BOOL *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_CursorVisible(
			/* [in] */ VARIANT_BOOL Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_MouseWheelStepValue(
			/* [retval][out] */ long *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_MouseWheelStepValue(
			/* [in] */ long Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_MouseWheelDirection(
			/* [retval][out] */ long *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_MouseWheelDirection(
			/* [in] */ long Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_MousePressAsRightButton(
			/* [retval][out] */ VARIANT_BOOL* Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_MousePressAsRightButton(
			/* [in] */ VARIANT_BOOL Value) = 0;
	};

	/* interface IJSValue */
	/* [object][oleautomation][dual][uuid] */

	EXTERN_C const IID IID_IJSValue;

	MIDL_INTERFACE("6DE2E6A0-3C3A-47DC-9A93-928135EDAC90")
	IJSValue : public IDispatch
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_DataType(
		/* [retval][out] */ enum IJSDataType *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_DataType(
		/* [in] */ enum IJSDataType Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_RawValue(
			/* [retval][out] */ VARIANT *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_RawValue(
			/* [in] */ VARIANT Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_AsString(
			/* [retval][out] */ BSTR *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_AsString(
			/* [in] */ BSTR Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_AsInt(
			/* [retval][out] */ long *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_AsInt(
			/* [in] */ long Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_AsBool(
			/* [retval][out] */ VARIANT_BOOL *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_AsBool(
			/* [in] */ VARIANT_BOOL Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_AsFloat(
			/* [retval][out] */ float *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_AsFloat(
			/* [in] */ float Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_AsJSON(
			/* [retval][out] */ BSTR *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_AsJSON(
			/* [in] */ BSTR Value) = 0;
	};

	/* interface IJSNamedValue */
	/* [object][oleautomation][dual][uuid] */

	EXTERN_C const IID IID_IJSNamedValue;

	MIDL_INTERFACE("E492419B-00AC-4A91-9AE9-9A82B07E89AE")
	IJSNamedValue : public IJSValue
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_Name(
			/* [retval][out] */ BSTR *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_Name(
			/* [in] */ BSTR Value) = 0;
	};

	/* interface IJSObject */
	/* [object][oleautomation][dual][uuid] */

	EXTERN_C const IID IID_IJSObject;

	MIDL_INTERFACE("59342310-79A7-4B14-8B63-6DF05609AE30")
	IJSObject : public IDispatch
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_Id(
			/* [retval][out] */ BSTR *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_Id(
			/* [in] */ BSTR Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Properties(
			/* [retval][out] */ IJSProperties **Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Methods(
			/* [retval][out] */ IJSMethods **Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Events(
			/* [retval][out] */ IJSEvents **Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Objects(
			/* [retval][out] */ IJSObjects **Value) = 0;

		virtual /* [id] */ HRESULT __stdcall FireEvent(
			/* [in] */ BSTR Name,
			/* [in] */ IJSArguments *Arguments) = 0;

		virtual /* [id] */ HRESULT __stdcall ApplyChanges(void) = 0;

		virtual /* [id] */ HRESULT __stdcall ApplyModel(void) = 0;
	};

	/* interface IJSObjects */
	/* [object][oleautomation][dual][uuid] */

	EXTERN_C const IID IID_IJSObjects;

	MIDL_INTERFACE("C2406011-568E-4EAC-B95C-EF29E4806B86")
	IJSObjects : public IDispatch
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_Count(
			/* [retval][out] */ long *Value) = 0;

		virtual /* [defaultcollelem][id][propget] */ HRESULT __stdcall get_Item(
			/* [in] */ VARIANT Index,
			/* [retval][out] */ IJSObject **Value) = 0;

		virtual /* [id] */ HRESULT __stdcall Clear(void) = 0;

		virtual /* [id] */ HRESULT __stdcall Add(
			/* [in] */ BSTR Id,
			/* [retval][out] */ IJSObject **Value) = 0;

		virtual /* [id] */ HRESULT __stdcall Remove(
			/* [in] */ IJSObject *Item) = 0;
	};

	/* interface IJSProperty */
	/* [object][oleautomation][dual][uuid] */

	EXTERN_C const IID IID_IJSProperty;

	MIDL_INTERFACE("1F95C0E9-E7BF-48C9-AA35-88AD0149109B")
	IJSProperty : public IJSNamedValue
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_ReadOnly(
			/* [retval][out] */ VARIANT_BOOL *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_ReadOnly(
			/* [in] */ VARIANT_BOOL Value) = 0;

		virtual /* [id] */ HRESULT __stdcall OnGet(
			/* [in] */ IJSBinding *Binding,
			/* [retval][out] */ IJSProperty **OutRetVal) = 0;

		virtual /* [id] */ HRESULT __stdcall OnSet(
			/* [in] */ IJSBinding *Binding,
			/* [retval][out] */ IJSProperty **OutRetVal) = 0;
	};

	/* interface IJSProperties */
	/* [object][oleautomation][dual][uuid] */

	EXTERN_C const IID IID_IJSProperties;

	MIDL_INTERFACE("FCBB688F-8FB2-42C1-86FC-0AAF3B2A500C")
	IJSProperties : public IDispatch
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_Count(
			/* [retval][out] */ long *Value) = 0;

		virtual /* [defaultcollelem][id][propget] */ HRESULT __stdcall get_Item(
			/* [in] */ VARIANT Index,
			/* [retval][out] */ IJSProperty **Value) = 0;

		virtual /* [id] */ HRESULT __stdcall Clear(void) = 0;

		virtual /* [id] */ HRESULT __stdcall Add(
			/* [in] */ BSTR Name,
			/* [retval][out] */ IJSProperty **OutRetVal) = 0;

		virtual /* [id] */ HRESULT __stdcall Remove(
			/* [in] */ IJSProperty *Item) = 0;
	};

	/* interface IJSArgument */
	/* [object][oleautomation][dual][uuid] */

	EXTERN_C const IID IID_IJSArgument;

	MIDL_INTERFACE("8F8C4462-D7B5-4696-BAD5-16DFAA6E2601")
	IJSArgument : public IJSNamedValue
	{
	public:
	};

	/* interface IJSArguments */
	/* [object][oleautomation][dual][uuid] */

	EXTERN_C const IID IID_IJSArguments;

	MIDL_INTERFACE("FC097EF5-6D8A-4C80-A2AD-382FDC75E901")
	IJSArguments : public IDispatch
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_Count(
			/* [retval][out] */ long *Value) = 0;

		virtual /* [defaultcollelem][id][propget] */ HRESULT __stdcall get_Item(
			/* [in] */ VARIANT Index,
			/* [retval][out] */ IJSArgument **Value) = 0;

		virtual /* [id] */ HRESULT __stdcall Clear(void) = 0;

		virtual /* [id] */ HRESULT __stdcall Add(
			/* [in] */ BSTR Name,
			/* [retval][out] */ IJSArgument **OutRetVal) = 0;

		virtual /* [id] */ HRESULT __stdcall Remove(
			/* [in] */ IJSArgument *Item) = 0;
	};

	/* interface IJSMethod */
	/* [object][oleautomation][dual][uuid] */

	EXTERN_C const IID IID_IJSMethod;

	MIDL_INTERFACE("C45D6A8F-AD4A-47BB-AC3A-C125D6D5D27E")
	IJSMethod : public IDispatch
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_Name(
			/* [retval][out] */ BSTR *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_Name(
			/* [in] */ BSTR Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Arguments(
			/* [retval][out] */ IJSArguments **Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_ReturnValue(
			/* [retval][out] */ IJSValue **Value) = 0;

		virtual /* [id] */ HRESULT __stdcall AddArgument(
			/* [in] */ BSTR Name,
		/* [in] */ enum IJSDataType DataType,
			/* [retval][out] */ IJSMethod **OutRetVal) = 0;

		virtual /* [id] */ HRESULT __stdcall OnCall(
			/* [in] */ IJSCallback *Callback,
			/* [retval][out] */ IJSMethod **OutRetVal) = 0;
	};

	/* interface IJSMethods */
	/* [object][oleautomation][dual][uuid] */

	EXTERN_C const IID IID_IJSMethods;

	MIDL_INTERFACE("E4CB461F-586E-4121-ABD7-345B87BC423A")
	IJSMethods : public IDispatch
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_Count(
			/* [retval][out] */ long *Value) = 0;

		virtual /* [defaultcollelem][id][propget] */ HRESULT __stdcall get_Item(
			/* [in] */ VARIANT Index,
			/* [retval][out] */ IJSMethod **Value) = 0;

		virtual /* [id] */ HRESULT __stdcall Clear(void) = 0;

		virtual /* [id] */ HRESULT __stdcall Add(
			/* [in] */ BSTR Name,
			/* [retval][out] */ IJSMethod **OutRetVal) = 0;

		virtual /* [id] */ HRESULT __stdcall Remove(
			/* [in] */ IJSMethod *Item) = 0;
	};

	/* interface IJSEvent */
	/* [object][oleautomation][dual][uuid] */

	EXTERN_C const IID IID_IJSEvent;

	MIDL_INTERFACE("8B66EACD-9619-43CF-9196-DCDA17F5500E")
	IJSEvent : public IDispatch
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_Name(
			/* [retval][out] */ BSTR *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_Name(
			/* [in] */ BSTR Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Arguments(
			/* [retval][out] */ IJSArguments **Value) = 0;

		virtual /* [id] */ HRESULT __stdcall AddArgument(
			/* [in] */ BSTR Name,
		/* [in] */ enum IJSDataType DataType,
			/* [retval][out] */ IJSEvent **OutRetVal) = 0;

		virtual /* [id] */ HRESULT __stdcall ArgumentAsNull(
			/* [in] */ VARIANT Index,
			/* [retval][out] */ IJSEvent **OutRetVal) = 0;

		virtual /* [id] */ HRESULT __stdcall ArgumentAsString(
			/* [in] */ VARIANT Index,
			/* [in] */ BSTR Value,
			/* [retval][out] */ IJSEvent **OutRetVal) = 0;

		virtual /* [id] */ HRESULT __stdcall ArgumentAsInt(
			/* [in] */ VARIANT Index,
			/* [in] */ long Value,
			/* [retval][out] */ IJSEvent **OutRetVal) = 0;

		virtual /* [id] */ HRESULT __stdcall ArgumentAsBool(
			/* [in] */ VARIANT Index,
			/* [in] */ VARIANT_BOOL Value,
			/* [retval][out] */ IJSEvent **OutRetVal) = 0;

		virtual /* [id] */ HRESULT __stdcall ArgumentAsFloat(
			/* [in] */ VARIANT Index,
			/* [in] */ float Value,
			/* [retval][out] */ IJSEvent **OutRetVal) = 0;

		virtual /* [id] */ HRESULT __stdcall ArgumentAsJSON(
			/* [in] */ VARIANT Index,
			/* [in] */ BSTR Value,
			/* [retval][out] */ IJSEvent **OutRetVal) = 0;

		virtual /* [id] */ HRESULT __stdcall Fire(void) = 0;
	};

	/* interface IJSEvents */
	/* [object][oleautomation][dual][uuid] */

	EXTERN_C const IID IID_IJSEvents;

	MIDL_INTERFACE("6AE952B3-B6DA-4C81-80FF-D0A162E11D02")
	IJSEvents : public IDispatch
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_Count(
			/* [retval][out] */ long *Value) = 0;

		virtual /* [defaultcollelem][id][propget] */ HRESULT __stdcall get_Item(
			/* [in] */ VARIANT Index,
			/* [retval][out] */ IJSEvent **Value) = 0;

		virtual /* [id] */ HRESULT __stdcall Clear(void) = 0;

		virtual /* [id] */ HRESULT __stdcall Add(
			/* [in] */ BSTR Name,
			/* [retval][out] */ IJSEvent **OutRetVal) = 0;

		virtual /* [id] */ HRESULT __stdcall Remove(
			/* [in] */ IJSEvent *Item) = 0;
	};

	/* interface IJSBinding */
	/* [object][oleautomation][dual][uuid] */

	EXTERN_C const IID IID_IJSBinding;

	MIDL_INTERFACE("ACFC2953-37F1-479E-B405-D0BB75E156E6")
	IJSBinding : public IDispatch
	{
	public:
		virtual /* [id] */ HRESULT __stdcall Set(
			/* [in] */ IJSObject *Parent,
			/* [in] */ IJSProperty *Prop) = 0;
	};

	/* interface IJSCallback */
	/* [object][oleautomation][dual][uuid] */

	EXTERN_C const IID IID_IJSCallback;

	MIDL_INTERFACE("ADD570A0-491A-4E40-8120-57B4D1245FD3")
	IJSCallback : public IDispatch
	{
	public:
		virtual /* [id] */ HRESULT __stdcall Callback(
			/* [in] */ IJSObject *Parent,
			/* [in] */ IJSMethod *Method) = 0;
	};

	EXTERN_C const IID IID_IEvents;
#if defined(__MINGW32__) || defined(__MINGW64__)
    DEFINE_GUID(IID_IEvents, 0x1C5700BC, 0x2317, 0x4062, 0xB6, 0x14, 0x0A, 0x4E, 0x28, 0x6C, 0xFE, 0x68);
    __CRT_UUID_DECL(IEvents, 0x1C5700BC, 0x2317, 0x4062, 0xB6, 0x14, 0x0A, 0x4E, 0x28, 0x6C, 0xFE, 0x68);
#endif
	MIDL_INTERFACE("1C5700BC-2317-4062-B614-0A4E286CFE68")
	IEvents : public IDispatch
	{
	public:
		virtual /* [id] */ HRESULT __stdcall OnGetUploadDir(
			/* [in, out] */ BSTR *Directory,
			/* [in, out] */ VARIANT_BOOL* Handled) = 0;

		virtual /* [id] */ HRESULT __stdcall OnBrowserResize(
			/* [in, out] */ long* Width,
			/* [in, out] */ long* Height,
			/* [in, out] */ VARIANT_BOOL* ResizeMaximized) = 0;

		virtual /* [id] */ HRESULT __stdcall OnClose(void) = 0;

		virtual /* [id] */ HRESULT __stdcall OnReceiveMessage(
			/* [in] */ BSTR *Data) = 0;

		virtual /* [id] */ HRESULT __stdcall OnDownloadEnd(
			/* [in] */ BSTR *Filename) = 0;

		virtual /* [id] */ HRESULT __stdcall OnRecorderChanged(void) = 0;

		virtual /* [id] */ HRESULT __stdcall OnUploadEnd(
			/* [in] */ BSTR *Filename) = 0;

		virtual /* [id] */ HRESULT __stdcall OnDragFile(
			/* [in] */ DragAction Action,
			/* [in] */ long X,
			/* [in] */ long Y,
			/* [in] */ BSTR Filenames) = 0;

		virtual /* [id] */ HRESULT __stdcall OnSaveDialog(
			/* [in] */ BSTR Filename) = 0;

		virtual /* [id] */ HRESULT __stdcall OnDragFile2(
			/* [in] */ DragAction Action,
			/* [in] */ long ScreenX,
			/* [in] */ long ScreenY,
			/* [in] */ BSTR Filenames,
			/* [in, out] */ VARIANT_BOOL* Accept) = 0;

		virtual /* [id] */ HRESULT __stdcall OnDropFile(
			/* [in] */ long ScreenX,
			/* [in] */ long ScreenY,
			/* [in] */ BSTR Filenames,
			/* [in] */ BSTR FileSizes,
			/* [in, out] */ BSTR* IgnoreFiles) = 0;
    };

	EXTERN_C const IID IID_IJSObjectEvents;
#if defined(__MINGW32__) || defined(__MINGW64__)
    DEFINE_GUID(IID_IJSObjectEvents, 0xA3D640E8, 0xCD18, 0x4196, 0xA1, 0xA2, 0xC8, 0x7C, 0x82, 0xB0, 0xF8, 0x8B);
    __CRT_UUID_DECL(IJSObjectEvents, 0xA3D640E8, 0xCD18, 0x4196, 0xA1, 0xA2, 0xC8, 0x7C, 0x82, 0xB0, 0x8F, 0x8B);
#endif
	MIDL_INTERFACE("A3D640E8-CD18-4196-A1A2-C87C82B0F88B")
	IJSObjectEvents : public IDispatch
	{
	public:
		virtual /* [id] */ HRESULT __stdcall OnExecuteMethod(
			/* [in] */ IJSObject *Caller,
			/* [in] */ IJSMethod *Method) = 0;

		virtual /* [id] */ HRESULT __stdcall OnPropertyChange(
			/* [in] */ IJSObject *Caller,
			/* [in] */ IJSProperty *Prop) = 0;
	};

	enum RecorderState
	{
		Inactive = 0,
		Recording = 1,
		Playing = 2
	};

	EXTERN_C const IID IID_IRecTrack;

	MIDL_INTERFACE("D4744AE1-70CB-43DD-BEA5-A5310B2E24C6")
	IRecTrack : public IDispatch
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_Name(
			/* [retval][out] */ BSTR *Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Position(
			/* [retval][out] */ long *Value) = 0;
	};

	EXTERN_C const IID IID_IRecTracks;

	MIDL_INTERFACE("AB45B615-9309-471E-A455-3FE93F88E674")
	IRecTracks : public IDispatch
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_Item(
			/* [in] */ long Index,
			/* [retval][out] */ IRecTrack **Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Count(
			/* [retval][out] */ long *Value) = 0;
	};

	EXTERN_C const IID IID_IRecorder;

	MIDL_INTERFACE("D89DA2B6-B7BF-4065-80F5-6D78B331C7DD")
	IRecorder : public IDispatch
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_Filename(
			/* [retval][out] */ BSTR *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_Filename(
			/* [in] */ BSTR Value) = 0;

		virtual /* [id] */ HRESULT __stdcall Rec(
			/* [in] */ BSTR Label) = 0;

		virtual /* [id] */ HRESULT __stdcall Play(
			/* [in] */ long From,
			/* [in] */ long To) = 0;

		virtual /* [id] */ HRESULT __stdcall Stop(void) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Position(
			/* [retval][out] */ long *Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Count(
			/* [retval][out] */ long *Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_State(
			/* [retval][out] */ RecorderState *Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Options(
			/* [retval][out] */ long *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_Options(
			/* [in] */ long Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Tracks(
			/* [retval][out] */ IRecTracks **Value) = 0;

	};

	EXTERN_C const IID IID_IFileSystemFilter;

	MIDL_INTERFACE("3FE99D2F-0CFC-43D1-B762-0C7C15EB872E")
	IFileSystemFilter : public IDispatch
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_User(
			/* [retval][out] */ BSTR *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_User(
			/* [in] */ BSTR Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_CfgFile(
			/* [retval][out] */ BSTR *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_CfgFile(
			/* [in] */ BSTR Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Active(
			/* [retval][out] */ VARIANT_BOOL *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_Active(
			/* [in] */ VARIANT_BOOL Value) = 0;

	};

	EXTERN_C const IID IID_RegistryFilter;

	MIDL_INTERFACE("4834F840-915B-488B-ADEA-98890A04CEE6")
	IRegistryFilter : public IDispatch
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_User(
			/* [retval][out] */ BSTR *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_User(
			/* [in] */ BSTR Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_CfgFile(
			/* [retval][out] */ BSTR *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_CfgFile(
			/* [in] */ BSTR Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Active(
			/* [retval][out] */ VARIANT_BOOL *Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_Active(
			/* [in] */ VARIANT_BOOL Value) = 0;

	};

	EXTERN_C const IID IID_IHTMLDoc;

	MIDL_INTERFACE("A5A7F58C-D83C-4C89-872E-0C51A9B5D3B0")
	IHTMLDoc : public IDispatch
	{
	public:
		virtual /* [id] */ HRESULT __stdcall CreateSessionURL(
			/* [in] */ BSTR Url,
			/* [in] */ BSTR Filename) = 0;

		virtual /* [id] */ HRESULT __stdcall CreateComponent(
			/* [in] */ BSTR Id,
			/* [in] */ BSTR Html,
			/* [in] */ unsigned int ReplaceWnd) = 0;

		virtual /* [id] */ HRESULT __stdcall GetSafeURL(
			/* [in] */ BSTR Filename,
			/* [in] */ int Minutes,
			/* [retval][out] */ BSTR* Value) = 0;

		virtual /* [id] */ HRESULT __stdcall LoadScript(
			/* [in] */ BSTR Url,
			/* [in] */ BSTR Filename) = 0;

		virtual /* [id] */ HRESULT __stdcall ImportHTML(
			/* [in] */ BSTR Url,
			/* [in] */ BSTR Filename) = 0;

		virtual /* [id] */ HRESULT __stdcall CreateForwarding(
			/* [in] */ BSTR Url,
			/* [retval][out] */ BSTR* Value) = 0;

	};

	MIDL_INTERFACE("627D6FDB-04B7-42E8-945A-2D91ECE891E9")
	IImageDelivery : public IDispatch
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_ContentionTime(
			/* [retval][out] */ int* Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_ContentionTime(
			/* [in] */ int Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_NewWindowContentionTime(
			/* [retval][out] */ int* Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_NewWindowContentionTime(
			/* [in] */ int Value) = 0;
	};

	MIDL_INTERFACE("FEE77B14-8072-4700-AE67-11EA0ED7DA9A")
	IImageEncoding : public IDispatch
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_Format(
			/* [retval][out] */ ImageEncodingType* Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_Format(
			/* [in] */ ImageEncodingType Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_JPEGQuality(
			/* [retval][out] */ int* Value) = 0;

		virtual /* [id][propput] */ HRESULT __stdcall put_JPEGQuality(
			/* [in] */ int Value) = 0;
	};

	MIDL_INTERFACE("0D05F614-FDE6-457E-82BD-8D8692D6BB1B")
	IPerformance : public IDispatch
	{
	public:
		virtual /* [id][propget] */ HRESULT __stdcall get_Delivery(
			/* [retval][out] */ IImageDelivery* Value) = 0;

		virtual /* [id][propget] */ HRESULT __stdcall get_Encoding(
			/* [retval][out] */ IImageEncoding* Value) = 0;
	};

	/* Additional Prototypes for ALL interfaces */

	unsigned long             __RPC_USER  BSTR_UserSize(unsigned long *, unsigned long, BSTR *);
	unsigned char * __RPC_USER  BSTR_UserMarshal(unsigned long *, unsigned char *, BSTR *);
	unsigned char * __RPC_USER  BSTR_UserUnmarshal(unsigned long *, unsigned char *, BSTR *);
	void                      __RPC_USER  BSTR_UserFree(unsigned long *, BSTR *);

	unsigned long             __RPC_USER  VARIANT_UserSize(unsigned long *, unsigned long, VARIANT *);
	unsigned char * __RPC_USER  VARIANT_UserMarshal(unsigned long *, unsigned char *, VARIANT *);
	unsigned char * __RPC_USER  VARIANT_UserUnmarshal(unsigned long *, unsigned char *, VARIANT *);
	void                      __RPC_USER  VARIANT_UserFree(unsigned long *, VARIANT *);

#ifdef __cplusplus
}
#endif




/// <summary>
/// Allows to set some client settings.
/// </summary>
class CClientSettings : public IClientSettings {
private:
	IVirtualUI *m_VirtualUI;
	LONG m_refcount;
	IClientSettings *m_ClientSettings;

	HRESULT __stdcall GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo);
	HRESULT __stdcall GetTypeInfoCount(UINT *Count);
	HRESULT __stdcall GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs);
	HRESULT __stdcall Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject);

public:
	CClientSettings(IVirtualUI *virtualUI);
	~CClientSettings();

	HRESULT __stdcall get_MouseMoveGestureStyle(enum MouseMoveGestureStyle *Value);
	HRESULT __stdcall put_MouseMoveGestureStyle(enum MouseMoveGestureStyle Value);
	HRESULT __stdcall get_MouseMoveGestureAction(enum MouseMoveGestureAction *Value);
	HRESULT __stdcall put_MouseMoveGestureAction(enum MouseMoveGestureAction Value);
	HRESULT __stdcall get_CursorVisible(VARIANT_BOOL *Value);
	HRESULT __stdcall put_CursorVisible(VARIANT_BOOL Value);
	HRESULT __stdcall get_MouseWheelStepValue(long *Value);
	HRESULT __stdcall put_MouseWheelStepValue(long Value);
	HRESULT __stdcall get_MouseWheelDirection(long *Value);
	HRESULT __stdcall put_MouseWheelDirection(long Value);
	HRESULT __stdcall get_MousePressAsRightButton(VARIANT_BOOL* Value);
	HRESULT __stdcall put_MousePressAsRightButton(VARIANT_BOOL Value);

	/// <summary>
	/// Gets the MouseMoveGestureStyle value
	MouseMoveGestureStyle MouseMoveGestStyle();

	/// <summary>
	/// Valid for touch devices. Specifies whether the mouse pointer
	/// is shown and acts on the exact spot of the finger touch
	/// (absolute) or its position is managed relatively to the
	/// movement of the finger touch (relative).
	/// </summary>
	void MouseMoveGestStyle(MouseMoveGestureStyle value);

	/// <summary>
	/// Gets the MouseMoveGestureAction value
	/// </summary>
	MouseMoveGestureAction MouseMoveGestAction();

	/// <summary>
	/// Specifies whether the &quot;mouse move&quot; simulation on a
	/// touch device is interpreted as a mouse move or as a mouse
	/// wheel.
	/// </summary>
	void MouseMoveGestAction(MouseMoveGestureAction value);

	/// <summary>
	/// Gets the the mouse pointer state.
	/// </summary>
	bool CursorVisible();

	/// <summary>
	/// Hides/shows the mouse pointer.
	/// </summary>
	void CursorVisible(bool value);

	/// <summary>
	/// Gets the the scroll speed used when the "mouse move" simulation
	/// on a touch device is interpreted as a mouse wheel.
	/// </summary>
	long MouseWheelStepValue();

	/// <summary>
 	/// Specifies the scroll speed when the "mouse move" simulation on a
	/// touch device is interpreted as a mouse wheel. Default value is
	/// 120. Lower values results in a smooth scrolling.
	/// </summary>
	void MouseWheelStepValue(long value);

	/// <summary>
	/// Gets the scroll direction when the "mouse move" simulation on a
	/// touch device is interpreted as a mouse wheel.
	/// </summary>
	long MouseWheelDirection();

	/// <summary>
	/// Specifies the scroll direction when the "mouse move" simulation on a
	/// touch device is interpreted as a mouse wheel. Set this to 1 (default)
	/// to normal direction, or -1 to invert.
	/// </summary>
	void MouseWheelDirection(long value);

	/// <summary>
	/// Gets the the mouse press behaviour.
	/// </summary>
	bool MousePressAsRightButton();

	/// <summary>
	/// Secifies the mouse press behaviour
	/// </summary>
	void MousePressAsRightButton(bool value);

};


/// <summary>
/// Contains information regarding the end-user's screen, web
/// browser, the window containing VirtualUI Viewer and VirtualUI
/// Viewer itself. The VirtualUI Viewer tuns inside an HTML DIV
/// element contained in a frame o browser window on the
/// end-user's application page.
/// </summary>
class CBrowserInfo : public IBrowserInfo {
private:
	IVirtualUI *m_VirtualUI;
	LONG m_refcount;
	IBrowserInfo *m_BrowserInfo;

	HRESULT __stdcall GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo);
	HRESULT __stdcall GetTypeInfoCount(UINT *Count);
	HRESULT __stdcall GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs);
	HRESULT __stdcall Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject);

public:
	CBrowserInfo(IVirtualUI *virtualUI);
	~CBrowserInfo();

	HRESULT __stdcall get_ViewWidth(long *Value);
	HRESULT __stdcall put_ViewWidth(long Value);
	HRESULT __stdcall get_ViewHeight(long *Value);
	HRESULT __stdcall put_ViewHeight(long Value);
	HRESULT __stdcall get_BrowserWidth(long *Value);
	HRESULT __stdcall get_BrowserHeight(long *Value);
	HRESULT __stdcall get_ScreenWidth(long *Value);
	HRESULT __stdcall get_ScreenHeight(long *Value);
	HRESULT __stdcall get_Username(BSTR *Value);
	HRESULT __stdcall get_IPAddress(BSTR *Value);
	HRESULT __stdcall get_UserAgent(BSTR *Value);
	HRESULT __stdcall get_ScreenResolution(long *Value);
	HRESULT __stdcall get_Orientation(enum Orientation *Value);
	HRESULT __stdcall get_UniqueBrowserId(BSTR *Value);
	HRESULT __stdcall get_CustomData(BSTR *Value);
	HRESULT __stdcall put_CustomData(BSTR Value);
	HRESULT __stdcall GetCookie(BSTR Name, BSTR *Value);
	HRESULT __stdcall SetCookie(BSTR Name, BSTR Value, BSTR Expires);
	HRESULT __stdcall get_Location(BSTR *Value);
	HRESULT __stdcall get_SelectedRule(BSTR* Value);
	HRESULT __stdcall get_ExtraData(BSTR *Value);
	HRESULT __stdcall GetExtraDataValue(BSTR Name, BSTR *Value);

	/// <summary>
	/// Returns the width of the VirtualUI Viewer.
	/// </summary>
	int ViewWidth();

	/// <summary>
	/// Sets the width of the VirtualUI Viewer.
	/// </summary>
	void ViewWidth(int value);

	/// <summary>
	/// Returns the height of the VirtualUI Viewer.
	/// </summary>
	int ViewHeight();

	/// <summary>
	/// Sets the height of the VirtualUI Viewer.
	/// </summary>
	void ViewHeight(int value);

	/// <value>
	/// Returns the width of the HTML element containing the
	/// VirtualUI Viewer.
	/// </value>
	int BrowserWidth();

	/// <summary>
	/// \Returns the height of the HTML element containing the
	/// VirtualUI Viewer.
	/// </summary>
	int BrowserHeight();

	/// <summary>
	/// Returns the width of the end-user's monitor screen.
	/// </summary>
	int ScreenWidth();

	/// <summary>
	/// \Returns the height of the end-user's monitor screen.
	/// </summary>
	int ScreenHeight();

	/// <summary>
	/// Returns the logged-on Username.
	/// </summary>
    std::wstring Username();

	/// <summary>
	/// Returns the client's IP address.
	/// </summary>
    std::wstring IPAddress();

	/// <summary>
	/// Returns the browser's User Agent string.
	/// </summary>
    std::wstring UserAgent();

	/// <summary>
	/// UniqueBrowserId identifies an instance of a Web Browser. Each time
	/// an end-user opens the application from a different browser window,
	/// this ID will have a different value.
	/// </summary>
    std::wstring UniqueBrowserId();

	/// <summary>
	/// Returns custom application data
	/// </summary>
	std::wstring CustomData();

	/// <summary>
	/// Sets custom application data
	/// </summary>
	void CustomData(std::wstring value);

	/// <summary>
	/// Returns the URL of the current application.
	/// </summary>
    std::wstring Location();

	/// <summary>
	/// \Returns the application screen resolution defined in the
	/// application profile.
	/// </summary>
	int ScreenResolution();

	/// <summary>
	/// \Returns the browser's orientation.
	/// </summary>
	Orientation Orientation();

	/// <summary>
	/// Returns the selected Browser Rule.
	/// </summary>
	std::wstring SelectedRule();

	/// <summary>
	///  Returns aditional data from Browser (JSON format).
	/// </summary>
	std::wstring ExtraData();

	/// <summary>
	///  Returns a specific value from ExtraData by it's name.
	/// </summary>
	/// <param name="Name">Name of value to return.</param>
	std::wstring GetExtraDataValue(std::wstring Name);
};


/// <summary>
/// Session recording and playback.
/// </summary>
class CRecorder : public IRecorder {
private:
	IVirtualUI *m_VirtualUI;
	LONG m_refcount;
	IRecorder *m_Recorder;
	CRecTracks *m_RecTracks;

	HRESULT __stdcall GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo);
	HRESULT __stdcall GetTypeInfoCount(UINT *Count);
	HRESULT __stdcall GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs);
	HRESULT __stdcall Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject);

public:
	CRecorder(IVirtualUI *virtualUI);
	~CRecorder();

	HRESULT __stdcall get_Filename(BSTR *Value);
	HRESULT __stdcall put_Filename(BSTR Value);
	HRESULT __stdcall get_Position(long *Value);
	HRESULT __stdcall get_Count(long *Value);
	HRESULT __stdcall get_State(RecorderState *Value);
	HRESULT __stdcall get_Options(long *Value);
	HRESULT __stdcall put_Options(long Value);
	HRESULT __stdcall get_Tracks(IRecTracks **Value);
	HRESULT __stdcall Rec(BSTR Label);

	/// <summary>
	/// Returns the current filename.
	/// </summary>
	std::wstring Filename();

	/// <summary>
	/// Sets the filename for session recording. Is the path and name
	/// of the session file to be used. No extension is needed; the
	/// recorder will generate two files, with extensions idx and
	/// dat.
	/// </summary>
	void Filename(std::wstring value);

	/// <summary>
	/// \Returns the current entry of the session currently playing.
	/// </summary>
	long Position();

	/// <summary>
	/// \Returns the number of entries in the loaded session file for
	/// playback.
	/// </summary>
	long Count();

	/// <summary>
	/// \Reports the recorder state:
	///
	/// \- Inactive: Recorder is idle.
	///
	/// \- Recording: Record in progress.
	///
	/// \- Playing: Playing a session.
	/// </summary>
	RecorderState State();

	long Options();

	void Options(long value);

	/// <summary>
	/// \Returns the list of tracks in a session file (for playback
	/// purposes only).
	/// </summary>
	CRecTracks* Tracks();

	/// <summary>
	/// Begins session recording.
	/// </summary>
	/// Label: track name. This name will allow to play different
	/// recordings in the same session.
	void Rec(std::wstring Label);

	/// <summary>
	/// Begins session playback.
	/// </summary>
	/// <param name="From">Entry of playback start.</param>
	/// <param name="To">Last entry to play.</param>
	///
	/// <remarks>
	/// To play a recorded session:
	///
	/// \- Set the Filename property with the path and name of the
	/// session file to be played (the file with idx extension).
	///
	/// \- Call Play with the range of entries to play.
	///
	/// To play an entire session, pass 0 and the Count property.
	///
	/// To play only a specific track, pass the Position of track to
	/// reproduce as From, and the Position of next track as To. For
	/// the last track, the To parameter must be the Count property
	/// of recorder.
	/// </remarks>
	HRESULT __stdcall Play(long From, long To);

	/// <summary>
	/// Stops the current recording.
	/// </summary>
	HRESULT __stdcall Stop(void);
};


/// <summary>
/// Collection of recorded session tracks.
/// </summary>
class CRecTracks : public IRecTracks
{
private:
	LONG m_refcount;
	IRecorder* m_Recorder;

	HRESULT __stdcall GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo);
	HRESULT __stdcall GetTypeInfoCount(UINT *Count);
	HRESULT __stdcall GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs);
	HRESULT __stdcall Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr);
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

public:
	CRecTracks(IRecorder* Recorder);
	~CRecTracks();

	HRESULT __stdcall get_Item(long Index, IRecTrack **Value);
	HRESULT __stdcall get_Count(long *Value);

	/// <summary>
	/// \Returns a session track.
	/// </summary>
	IRecTrack* Item(long index);

	/// <summary>
	/// Number of tracks
	/// </summary>
	long Count();
};

/// <summary>
/// Application FileSystem Virtualization.
/// </summary>
class CFileSystemFilter : IFileSystemFilter {
private:
	IVirtualUI *m_VirtualUI;
	LONG m_refcount;
	IFileSystemFilter *m_Filter;

	HRESULT __stdcall GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo);
	HRESULT __stdcall GetTypeInfoCount(UINT *Count);
	HRESULT __stdcall GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs);
	HRESULT __stdcall Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject);

public:
	CFileSystemFilter(IVirtualUI *virtualUI);
	~CFileSystemFilter();

	HRESULT __stdcall get_User(BSTR *Value);
	HRESULT __stdcall put_User(BSTR Value);
	HRESULT __stdcall get_CfgFile(BSTR *Value);
	HRESULT __stdcall put_CfgFile(BSTR Value);
	HRESULT __stdcall get_Active(VARIANT_BOOL *Value);
	HRESULT __stdcall put_Active(VARIANT_BOOL Value);

	/// <summary>
	/// Returns the current user.
	/// </summary>
	std::wstring User();

	/// <summary>
	/// Sets the user for the filesystem virtualization.
	/// </summary>
	void User(std::wstring value);

	/// <summary>
	/// Returns the configuration filename.
	/// </summary>
	std::wstring CfgFile();

	/// <summary>
	/// Sets the configuration filename for the filesystem virtualization.
	/// </summary>
	void CfgFile(std::wstring value);

	/// <summary>
	/// Returns the filesystem virtualization's state.
	/// </summary>
	bool Active();

	/// <summary>
	/// Sets the filesystem virtualization state for the container application.
	/// </summary>
	void Active(bool value);
};

/// <summary>
/// Application FileSystem Virtualization.
/// </summary>
class CRegistryFilter : IRegistryFilter {
private:
	IVirtualUI *m_VirtualUI;
	LONG m_refcount;
	IRegistryFilter *m_Filter;

	HRESULT __stdcall GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo);
	HRESULT __stdcall GetTypeInfoCount(UINT *Count);
	HRESULT __stdcall GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs);
	HRESULT __stdcall Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject);

public:
	CRegistryFilter(IVirtualUI *virtualUI);
	~CRegistryFilter();

	HRESULT __stdcall get_User(BSTR *Value);
	HRESULT __stdcall put_User(BSTR Value);
	HRESULT __stdcall get_CfgFile(BSTR *Value);
	HRESULT __stdcall put_CfgFile(BSTR Value);
	HRESULT __stdcall get_Active(VARIANT_BOOL *Value);
	HRESULT __stdcall put_Active(VARIANT_BOOL Value);

	/// <summary>
	/// Returns the current user.
	/// </summary>
	std::wstring User();

	/// <summary>
	/// Sets the user for the registry virtualization.
	/// </summary>
	void User(std::wstring value);

	/// <summary>
	/// Returns the configuration filename.
	/// </summary>
	std::wstring CfgFile();

	/// <summary>
	/// Sets the configuration filename for the registry virtualization.
	/// </summary>
	void CfgFile(std::wstring value);

	/// <summary>
	/// Returns the registry virtualization's state.
	/// </summary>
	bool Active();

	/// <summary>
	/// Sets the registry virtualization state for the container application.
	/// </summary>
	void Active(bool value);
};

/// <summary>
/// Contains properties to manage the VirtualUI Development
/// Server as well as the access from the developer's web
/// browser.
/// </summary>
class CDevServer : public IDevServer
{
private:
	IVirtualUI *m_VirtualUI;
	LONG m_refcount;
	IDevServer *m_DevServer;

	HRESULT __stdcall GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo);
	HRESULT __stdcall GetTypeInfoCount(UINT *Count);
	HRESULT __stdcall GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs);
	HRESULT __stdcall Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject);

public:
	CDevServer(IVirtualUI *virtualUI);
	~CDevServer();

	HRESULT __stdcall get_Enabled(VARIANT_BOOL *Value);
	HRESULT __stdcall put_Enabled(VARIANT_BOOL Value);
	HRESULT __stdcall get_Port(long *Value);
	HRESULT __stdcall put_Port(long Value);
	HRESULT __stdcall get_StartBrowser(VARIANT_BOOL *Value);
	HRESULT __stdcall put_StartBrowser(VARIANT_BOOL Value);

	/// <summary>
	///   Gets the Development Server state.
	/// </summary>
	bool Enabled();

	/// <summary>
	///   Enables/disables the Development Server.
	/// </summary>
	void Enabled(bool value);

	/// <summary>
	///   Gets the Development Server's TCP/IP listening port.
	/// </summary>
	int Port();

	/// <summary>
	///   Sets the Development Server's TCP/IP listening port.
	/// </summary>
	void Port(int value);

	/// <summary>
	/// Instructs VirtualUI whether start or not the local web
	/// browser upon VirtualUI activation.
	/// </summary>
	void StartBrowser(bool value);
};


class VirtualUILibrary {
protected:
  static HINSTANCE LibHandle;
  std::wstring GetDLLPath();
public:
	HINSTANCE GetHandle();
};


class VirtualUISink : public IEvents
{
private:
	VirtualUI* m_VirtualUI;
	LONG m_refcount;
	IConnectionPoint* m_pIConnectionPoint;
	DWORD m_dwEventCookie;

	HRESULT __stdcall GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo);
	HRESULT __stdcall GetTypeInfoCount(UINT *Count);
	HRESULT __stdcall GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs);
	HRESULT __stdcall Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject);

	long GetAsInteger(DISPPARAMS* args, int index);
	long* GetAsIntegerRef(DISPPARAMS* args, int index);
	VARIANT_BOOL GetAsVariantBool(DISPPARAMS* args, int index);
	VARIANT_BOOL* GetAsVariantBoolRef(DISPPARAMS* args, int index);
	BSTR GetAsBSTR(DISPPARAMS* args, int index);
    BSTR* GetAsBSTRRef(DISPPARAMS* args, int index);
	IDispatch* GetAsInterface(DISPPARAMS* args, int index);

	HRESULT __stdcall OnGetUploadDir(BSTR *Directory, VARIANT_BOOL* Handled);
	HRESULT __stdcall OnBrowserResize(long* Width, long* Height, VARIANT_BOOL* ResizeMaximized);
	HRESULT __stdcall OnClose(void);
	HRESULT __stdcall OnReceiveMessage(BSTR *Data);
	HRESULT __stdcall OnDownloadEnd(BSTR *Filename);
	HRESULT __stdcall OnUploadEnd(BSTR *Filename);
	HRESULT __stdcall OnRecorderChanged(void);
	HRESULT __stdcall OnDragFile(DragAction Action, long X, long Y, BSTR Filenames);
	HRESULT __stdcall OnSaveDialog(BSTR Filename);
	HRESULT __stdcall OnDragFile2(DragAction Action, long ScreenX, long ScreenY, BSTR Filenames, VARIANT_BOOL* Accept);
	HRESULT __stdcall OnDropFile(long ScreenX, long ScreenY, BSTR Filenames, BSTR FileSizes, BSTR* IgnoreFiles);
public:
	VirtualUISink(VirtualUI* virtualUI);
	~VirtualUISink();
};

class JSObjectSink : public IJSObjectEvents
{
private:
	JSObject* m_JSObject;
	LONG m_refcount;
	IConnectionPoint* m_pIConnectionPoint;
	DWORD m_dwEventCookie;

	HRESULT __stdcall GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo);
	HRESULT __stdcall GetTypeInfoCount(UINT *Count);
	HRESULT __stdcall GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs);
	HRESULT __stdcall Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject);

	IDispatch* GetAsInterface(DISPPARAMS* args, int index);

	HRESULT __stdcall OnExecuteMethod(IJSObject *Caller, IJSMethod *Method);
	HRESULT __stdcall OnPropertyChange(IJSObject *Caller, IJSProperty *Prop);
public:
	JSObjectSink(JSObject* jsObject);
	~JSObjectSink();
};


/// <summary>
/// Main class. Has methods, properties and events to allow the
/// activation and control the behavior of VirtualUI.
/// </summary>
class VirtualUI : public IVirtualUI
{
private:
	VirtualUILibrary *m_VirtualUILib;
	LONG m_refcount;

	IVirtualUI *m_VirtualUI;
	CClientSettings *m_ClientSettings;
	CBrowserInfo *m_BrowserInfo;
	CDevServer *m_DevServer;
	VirtualUISink *m_EventSink;
	CRecorder *m_Recorder;
	CFileSystemFilter *m_FileSystemFilter;
	CRegistryFilter *m_RegistryFilter;
	CHTMLDoc *m_HTMLDoc;

	HRESULT __stdcall GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo);
	HRESULT __stdcall GetTypeInfoCount(UINT *Count);
	HRESULT __stdcall GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs);
	HRESULT __stdcall Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

public:
    VirtualUI();
    ~VirtualUI();

	HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject);

	HRESULT __stdcall Start(long Timeout, VARIANT_BOOL *OutRetVal);
	HRESULT __stdcall get_Active(VARIANT_BOOL *Value);
	HRESULT __stdcall get_Enabled(VARIANT_BOOL *Value);
	HRESULT __stdcall put_Enabled(VARIANT_BOOL Value);
	HRESULT __stdcall get_DevMode(VARIANT_BOOL *Value);
	HRESULT __stdcall put_DevMode(VARIANT_BOOL Value);
	HRESULT __stdcall get_StdDialogs(VARIANT_BOOL *Value);
	HRESULT __stdcall put_StdDialogs(VARIANT_BOOL Value);
	HRESULT __stdcall get_BrowserInfo(IBrowserInfo **Value);
	HRESULT __stdcall get_DevServer(IDevServer **Value);
	HRESULT __stdcall get_ClientSettings(IClientSettings **Value);
	HRESULT __stdcall DownloadFile(BSTR LocalFilename, BSTR RemoteFilename, BSTR MimeType);
	HRESULT __stdcall PrintPdf(BSTR AFileName);
	HRESULT __stdcall OpenLinkDlg(BSTR url, BSTR caption);
	HRESULT __stdcall SendMessage(BSTR Data);
	HRESULT __stdcall AllowExecute(BSTR Filename);
	HRESULT __stdcall SetImageQualityByWnd(long Wnd, BSTR Class, long Quality);
	HRESULT __stdcall Uploadfile(BSTR ServerDirectory);
	HRESULT __stdcall UploadFileEx(BSTR ServerDirectory, BSTR* FileName, VARIANT_BOOL *OutRetVal);
	HRESULT __stdcall TakeScreenshot(long Wnd, BSTR FileName, VARIANT_BOOL *OutRetVal);
	HRESULT __stdcall ShowVirtualKeyboard(void);
	HRESULT __stdcall get_Recorder(IRecorder **Value);
	HRESULT __stdcall get_FileSystemFilter(IFileSystemFilter **Value);
	HRESULT __stdcall get_RegistryFilter(IRegistryFilter **Value);
	HRESULT __stdcall Get_Options(long *Value);
	HRESULT __stdcall Set_Options(long Value);
	HRESULT __stdcall Get_HTMLDoc(IHTMLDoc **Value);
	HRESULT __stdcall Get_Performance(IPerformance** OutRetVal);
	HRESULT __stdcall PreviewPdf(BSTR AFileName);

	/// <summary>
	/// Starts the VirtualUI's activation process. Returns true if
	/// VirtualUI was fully activated or false if the timeout
	/// expired. The timeout is 60 seconds.
	/// </summary>
    bool Start();

	/// <summary>
	/// Starts the VirtualUI's activation process. Returns true if
	/// VirtualUI was fully activated or false if the passed timeout
	/// expired.
	/// </summary>
	/// <param name="Timeout">Maximum time, in seconds, until the
	///                       activation process is canceled.
	///                       Defaults to 60 seconds. </param>
	/// <remarks>
	/// To fully activate VirtualUI, the connection with the
	/// end-user's web browser must established within the time
	/// specified by Timeout parameter.
	/// </remarks>
	bool Start(int Timeout);

	/// <summary>
	/// Deactivates VirtualUI, closing the connection with the
	/// end-user's web browser.
	/// </summary>
	HRESULT __stdcall Stop(void);

	/// <summary>
	/// Sends the specified file to the end-user's web browser for
	/// saving it in the remote machine.
	/// </summary>
	/// <param name="LocalFilename">Name of both the local and
	///                             remote file . </param>
    void DownloadFile(std::wstring LocalFilename);

	/// <summary>
	/// Sends the specified file to the end-user's web browser for
	/// saving it in the remote machine.
	/// </summary>
	/// <param name="LocalFilename">Name of the local file to be
	///                             sent. </param>
	/// <param name="RemoteFilename">Name of the file in the remote
	///                              machine. </param>
    void DownloadFile(std::wstring LocalFilename, std::wstring RemoteFilename);

	/// <summary>
	/// Sends the specified file to the end-user's web browser for
	/// saving it in the remote machine.
	/// </summary>
	/// <param name="LocalFilename">Name of the local file to be
	///                             sent. </param>
	/// <param name="RemoteFilename">Name of the file in the remote
	///                              machine. </param>
	/// <param name="MimeType">content-type of the file. If specified,
	///                        the content will be handled by browser.
	///                        Leave blank to force download.</param>
    void DownloadFile(std::wstring LocalFilename, std::wstring RemoteFilename, std::wstring MimeType);

	/// <summary>
	/// Sends the specified PDF file to be shown on the end-user's
	/// web browser.
	/// </summary>
	/// <param name="AFileName">Name of the PDF file. </param>
	/// <remarks>
	/// PrintPDF is similar to DownloadFile, except that it downloads
	/// the file with a content-type: application/pdf.
	/// </remarks>
    void PrintPdf(std::wstring FileName);

	/// <summary>
	/// Sends the specified PDF file to be shown on the end-user's
	/// web browser. It's similar to PrintPdf, except that disables
	/// the printing options in the browser. Built-in browser printing
	/// commands will be available.
	/// </summary>
	/// <param name="AFileName">Name of the PDF file. </param>
    void PreviewPdf(std::wstring FileName);

	/// <summary>
	///   Displays a popup with a button to open a web link.
	/// </summary>
	/// <param name="url">Link to open.</param>
	/// <param name="caption">Text to display in popup.</param>
    void OpenLinkDlg(std::wstring Url, std::wstring Caption);

	/// <summary>
	/// Sends a data string to the web browser.
	/// </summary>
	/// <remarks>
	/// This method is used to send custom data to the browser for
	/// custom purposes.
	/// </remarks>
    void SendMessage(std::wstring Data);

	/// <summary>
	/// Allows the execution of the passed application.
	/// </summary>
	/// <param name="Filename">regular expression specifying the
	///                        filename(s) of the applications
	///                        allowed to run. </param>
	/// <remarks>
	/// Under VirtualUI environment, only applications precompiled
	/// with VirtualUI SDK should be allowed to run. Applications not
	/// under VirtualUI control, cannot be controlled.
	/// </remarks>
    void AllowExecute(std::wstring Filename);

	/// <summary>
	/// Allows to the the image quality for the specified window.
	/// </summary>
	/// <param name="Wnd">Window handle. </param>
	/// <param name="Class">Window classname. </param>
	/// <param name="Quality">Quality from 0 to 100. </param>
    void SetImageQualityByWnd(long Wnd, std::wstring Classname, long Quality);

	/// <summary>
	///   Selects a file from client machine, and it's uploaded to
	///   ServerDirectory
	/// </summary>
	/// <param name="ServerDirectory">Destination directory in Server.</param>
    void Uploadfile(std::wstring ServerDirectory);

	/// <summary>
	///   Selects a file from client machine, and it's uploaded to VirtualUI
	///   public path.
	/// </summary>
	void Uploadfile();

	/// <summary>
	///   Selects a file from client machine, and it's uploaded to
	///   ServerDirectory
	/// </summary>
	/// <param name="ServerDirectory">Destination directory in Server.</param>
	/// <param name="FileName">Returns the full path of uploaded file.</param>
	bool UploadFileEx(std::wstring ServerDirectory, std::wstring &FileName);

	/// <summary>
	///   Selects a file from client machine, and it's uploaded to VirtualUI
	///   public path.
	/// </summary>
	/// <param name="FileName">Returns the full path of uploaded file.</param>
	bool UploadFileEx(std::wstring &FileName);

	/// <summary>
	///   Suspends the UI streaming to the web browser
	/// </summary>
	HRESULT __stdcall Suspend();

	/// <summary>
	///   Resumes the UI streaming to the web browser
	/// </summary>
	HRESULT __stdcall Resume();

	/// <summary>
	///   Flushes any pending window's drawing.
	/// </summary>
	/// <param name="Wnd">The Window</param>
	HRESULT __stdcall FlushWindow(long Wnd,VARIANT_BOOL* OutRetVal);

	/// <summary>
	///   Takes a screenshot of a Window.
	/// </summary>
	/// <param name="Wnd">The Window to capture.</param>
	/// <param name="FileName">Full path of file to save screenshot.
	///                        Extensions allowed: jpg, bmp, png.</param>
	bool TakeScreenshot(HWND Wnd, std::wstring FileName);

	/// <summary>
	/// Returns the VirtualUI's state.
	/// </summary>
	bool Active();

	/// <summary>
	/// Gets the VirtualUI state for the container application.
	/// </summary>
	bool Enabled();

	/// <summary>
	/// Enables/disables VirtualUI for the container application.
	/// </summary>
	void Enabled(bool value);

	/// <summary>
	/// Gets the development mode.
	/// </summary>
	bool DevMode();

	/// <summary>
	/// Sets the development mode.
	/// </summary>
	/// <remarks>
	/// When in development mode, applications executed under the
	/// IDE, connect to the Development Server, allowing the access
	/// to the application from the browser while in debugging.
	/// </remarks>
	void DevMode(bool value);

	/// <summary>
	/// Gets the use of standard dialogs state.
	/// </summary>
	bool StdDialogs();

	/// <summary>
	/// Enables/disables the use of standard dialogs.
	/// </summary>
	/// <remarks>
	/// When set to false, the standard save, open and print dialogs
	/// are replaced by native browser ones, enabling you to extend
	/// the operations to the remote computer.
	/// </remarks>
	void StdDialogs(bool value);

	/// <summary>
	/// Controls some working parameters on the client side.
	/// </summary>
	CClientSettings* ClientSettings();

	/// <summary>
	/// Contains information regarding the end-user's environment.
	/// </summary>
	CBrowserInfo* BrowserInfo();

	/// <summary>
	/// Allows for managing the Development Server.
	/// </summary>
	CDevServer* DevServer();

	/// <summary>
	/// Controls session recording.
	/// </summary>
	CRecorder* Recorder();

	/// <summary>
	/// Controls the filesystem virtualization.
	/// </summary>
	CFileSystemFilter* FileSystemFilter();

	/// <summary>
	/// Controls the registry virtualization.
	/// </summary>
	CRegistryFilter* RegistryFilter();

	/// <summary>
	///  Get Option flags.
	/// </summary>
	long Options();

	/// <summary>
	///  Set Option flags.
	/// </summary>
	void Options(long Value);

	/// <summary>
	///   Contains methods to modify the behavior on the HTML page.
	/// </summary>
	CHTMLDoc* HTMLDoc();

	void(*OnBrowserResize)(int &Width, int &Height, bool &ResizeMaximized);
    void(*OnGetUploadDir)(std::wstring &Directory, bool &Handled);
	void(*OnClose)();
    void(*OnReceiveMessage)(std::wstring &Data);
    void(*OnDownloadEnd)(std::wstring &Filename);
	void(*OnUploadEnd)(std::wstring &Filename);
	void(*OnRecorderChanged)();
	void(*OnDragFile)(DragAction Action, int X, int Y, std::wstring Filenames);
	void(*OnSaveDialog)(std::wstring Filename);
	void(*OnDragFile2)(DragAction Action, long ScreenX, long ScreenY, std::wstring Filenames, bool &Accept);
	void(*OnDropFile)(long ScreenX, long ScreenY, std::wstring Filenames, std::wstring FileSizes, std::wstring &IgnoreFiles);
};


/// <summary>
/// Represents a custom remotable object.
/// </summary>
/// <remarks>
/// JSObject allows you to define an object model that is
/// mirrored on the client side, and allows for an easy, powerful
/// and straight-forward way to connect the web browser client
/// application and the remoted Windows application.
///
/// JSObject can contain properties (IJSProperties), methods
/// (IJSMethods), events (IJSEvents) and children objects.
/// Changes to properties values are propagated in from server to
/// client and viceversa, keeping the data synchronized.
///
/// JSObject is defined as a model seen from the client
/// perspective. A method (IJSMethod) is called on the client
/// side and executed on the server side. An event (IJSEvent) is
/// called on the server side and raised on the client side.
/// </remarks>
class JSObject : public IJSObject
{
private:
	VirtualUILibrary *m_VirtualUILib;
	LONG m_refcount;
	IJSObject* m_JSObject;
	JSObjectSink* m_EventSink;
	CJSProperties* m_JSProperties;
	CJSMethods* m_JSMethods;
	CJSEvents* m_JSEvents;
	CJSObjects* m_JSObjects;

	HRESULT __stdcall GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo);
	HRESULT __stdcall GetTypeInfoCount(UINT *Count);
	HRESULT __stdcall GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs);
	HRESULT __stdcall Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

public:
    JSObject(std::wstring id);
	~JSObject();

	HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject);

	HRESULT __stdcall get_Id(BSTR *Value);
	HRESULT __stdcall put_Id(BSTR Value);
	HRESULT __stdcall get_Properties(IJSProperties **Value);
	HRESULT __stdcall get_Methods(IJSMethods **Value);
	HRESULT __stdcall get_Events(IJSEvents **Value);
	HRESULT __stdcall get_Objects(IJSObjects **Value);
	HRESULT __stdcall FireEvent(BSTR Name, IJSArguments *Arguments);

    void FireEvent(std::wstring Name, IJSArguments* Arguments);

	/// <summary>
	/// When this method called, all properties getters are
	/// internally called looking for changes. Any change to the
	/// property value is sent to the client.
	/// </summary>
	HRESULT __stdcall ApplyChanges(void);

	/// <summary>
	/// Propagates the whole JSObject definition to the javascript
	/// client.
	/// </summary>
	HRESULT __stdcall ApplyModel(void);

	/// <summary>
	/// Identifier of the object. It must be unique among siblings
	/// objects.
	/// </summary>
    std::wstring Id();

    void Id(std::wstring value);

	/// <summary>
	/// List containing all properties of this object.
	/// </summary>
	CJSProperties* Properties();

	/// <summary>
	/// List containing all methods of this object.
	/// </summary>
	CJSMethods* Methods();

	/// <summary>
	/// List containing all events of this object.
	/// </summary>
	CJSEvents* Events();

	/// <summary>
	/// List containing all events of this object.
	/// </summary>
	CJSObjects* Objects();

	/// <summary>
	/// Fired when a method is executed on the remote object.
	/// </summary>
	void(*OnExecuteMethod)(IJSObject *Caller, IJSMethod *Method);

	/// <summary>
	/// Fired when a property value has been changed on the remote object.
	/// </summary>
	void(*OnPropertyChange)(IJSObject *Caller, IJSProperty *Prop);
};


typedef void(*JSMethodCallback)(IJSObject* Parent, IJSMethod* Method);

class JSCallback : public IJSCallback
{
private:
	LONG m_refcount;
	JSMethodCallback m_Proc;

	HRESULT __stdcall GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo);
	HRESULT __stdcall GetTypeInfoCount(UINT *Count);
	HRESULT __stdcall GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs);
	HRESULT __stdcall Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr);
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
public:
	JSCallback(JSMethodCallback Proc);
	HRESULT __stdcall Callback(IJSObject* Parent, IJSMethod* Method);
};


typedef void (*JSPropertySet)(IJSObject* Parent, IJSProperty* Prop);

class JSBinding : public IJSBinding
{
private:
	LONG m_refcount;
	JSPropertySet m_Proc;

	HRESULT __stdcall GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo);
	HRESULT __stdcall GetTypeInfoCount(UINT *Count);
	HRESULT __stdcall GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs);
	HRESULT __stdcall Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr);
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();
public:
	JSBinding(JSPropertySet Proc);
	HRESULT __stdcall Set(IJSObject* Parent, IJSProperty* Prop);
};


class CJSProperties : public IJSProperties
{
private:
	LONG m_refcount;
	IJSObject* m_JSObject;
	IJSProperties* m_JSProperties;

	HRESULT __stdcall GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo);
	HRESULT __stdcall GetTypeInfoCount(UINT *Count);
	HRESULT __stdcall GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs);
	HRESULT __stdcall Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr);
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

public:
	CJSProperties(IJSObject* jsObject);
	~CJSProperties();

	HRESULT __stdcall get_Count(long *Value);
	HRESULT __stdcall get_Item(VARIANT Index, IJSProperty **Value);
	HRESULT __stdcall Add(BSTR Name, IJSProperty **OutRetVal);
	HRESULT __stdcall Clear(void);
	HRESULT __stdcall Remove(IJSProperty *Item);

	long Count();
	IJSProperty* Item(long Index);
    IJSProperty* Item(std::wstring Name);
    IJSProperty* Add(std::wstring Name);
};


class CJSMethods : public IJSMethods
{
private:
	LONG m_refcount;
	IJSObject* m_JSObject;
	IJSMethods* m_JSMethods;

	HRESULT __stdcall GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo);
	HRESULT __stdcall GetTypeInfoCount(UINT *Count);
	HRESULT __stdcall GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs);
	HRESULT __stdcall Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr);
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

public:
	CJSMethods(IJSObject* jsObject);
	~CJSMethods();

	HRESULT __stdcall get_Count(long *Value);
	HRESULT __stdcall get_Item(VARIANT Index, IJSMethod **Value);
	HRESULT __stdcall Clear(void);
	HRESULT __stdcall Add(BSTR Name, IJSMethod **OutRetVal);
	HRESULT __stdcall Remove(IJSMethod *Item);

	long Count();
	IJSMethod* Item(long Index);
    IJSMethod* Item(std::wstring Name);
    IJSMethod* Add(std::wstring Name);
};


class CJSEvents : public IJSEvents
{
private:
	LONG m_refcount;
	IJSObject* m_JSObject;
	IJSEvents* m_JSEvents;

	HRESULT __stdcall GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo);
	HRESULT __stdcall GetTypeInfoCount(UINT *Count);
	HRESULT __stdcall GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs);
	HRESULT __stdcall Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr);
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

public:
	CJSEvents(IJSObject* jsObject);
	~CJSEvents();

	HRESULT __stdcall get_Count(long *Value);
	HRESULT __stdcall get_Item(VARIANT Index, IJSEvent **Value);
	HRESULT __stdcall Clear(void);
	HRESULT __stdcall Add(BSTR Name, IJSEvent **OutRetVal);
	HRESULT __stdcall Remove(IJSEvent *Item);

	long Count();
	IJSEvent* Item(long Index);
    IJSEvent* Item(std::wstring Name);
    IJSEvent* Add(std::wstring Name);
};


class CJSObjects : public IJSObjects
{
private:
	LONG m_refcount;
	IJSObject* m_JSObject;
	IJSObjects* m_JSObjects;

	HRESULT __stdcall GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo);
	HRESULT __stdcall GetTypeInfoCount(UINT *Count);
	HRESULT __stdcall GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs);
	HRESULT __stdcall Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr);
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

public:
	CJSObjects(IJSObject* jsObject);
	~CJSObjects();

	HRESULT __stdcall get_Count(long *Value);
	HRESULT __stdcall get_Item(VARIANT Index, IJSObject **Value);
	HRESULT __stdcall Clear(void);
	HRESULT __stdcall Add(BSTR Id, IJSObject **Value);
	HRESULT __stdcall Remove(IJSObject *Item);

	long Count();
	IJSObject* Item(long Index);
    IJSObject* Item(std::wstring Name);
    IJSObject* Add(std::wstring Name);
};


/// <summary>
/// Main class. Has methods, properties and events to allow to
/// manage some web behavior.
/// </summary>
class CHTMLDoc : public IHTMLDoc
{
private:
	LONG m_refcount;
	IVirtualUI* m_VirtualUI;
	IHTMLDoc* m_HTMLDoc;

	HRESULT __stdcall GetTypeInfo(UINT Index, LCID LocaleID, ITypeInfo FAR* FAR* TypeInfo);
	HRESULT __stdcall GetTypeInfoCount(UINT *Count);
	HRESULT __stdcall GetIDsOfNames(REFIID IID, LPOLESTR *Names, UINT NameCount, LCID LocaleID, DISPID *DispIDs);
	HRESULT __stdcall Invoke(DISPID DispID, REFIID IID, LCID LocaleID, WORD Flags, DISPPARAMS *Params, VARIANT *VarResult, EXCEPINFO *ExcepInfo, UINT *ArgErr);
	HRESULT __stdcall QueryInterface(REFIID riid, void **ppvObject);
	ULONG __stdcall AddRef();
	ULONG __stdcall Release();

	HRESULT __stdcall CreateSessionURL(BSTR Url, BSTR Filename);
	HRESULT __stdcall CreateComponent(BSTR Id, BSTR Html, unsigned int ReplaceWnd);
	HRESULT __stdcall GetSafeURL(BSTR Filename, int Minutes, BSTR* Value);
	HRESULT __stdcall LoadScript(BSTR Url, BSTR Filename);
	HRESULT __stdcall ImportHTML(BSTR Url, BSTR Filename);
	HRESULT __stdcall CreateForwarding(BSTR Url, BSTR* Value);

public:
	CHTMLDoc(IVirtualUI* virtualUI);
	~CHTMLDoc();

	/// <summary>
	/// Creates an url pointing to a local filename. This url is valid
	/// during the session lifetime and its private to this session.
	/// </summary>
	/// <param name="url">Arbritary relative url.</param>
	/// <param name="filename">Local filename</param>
	void CreateSessionURL(std::wstring Url, std::wstring Filename);

	/// <summary>
	/// Inserts an HTML. Used to insert regular HTML elements or
	/// WebComponents with custom elements.
	/// </summary>
	/// <param name="Id">ID to assign to the main element of the HTML to be inserted</param>
	/// <param name="Html">HTML snippet</param>
	/// <param name="ReplaceWnd">Wnd to be replaced and tied to</param>
	/// <remarks>
	/// When ReplaceWnd is <> 0 and points to a valid window handle, the positioning of the main element
	/// will follow the Wnd positioning, simulating an embedding.
	/// </remarks>
	void CreateComponent(std::wstring Id, std::wstring Html, unsigned int ReplaceWnd);

	/// <summary>
	/// Returns a safe, temporary and unique URL to access any local file.
	/// </summary>
	/// <param name="Filename">Local filename</param>
	/// <param name="Minutes">Expiration in minutes</param>
	std::wstring GetSafeURL(std::wstring Filename, int Minutes);

	/// <summary>
	///  Loads a script from URL. If Filename is specified, creates a session
	///  URL first and then load the script from that Filename.
	/// </summary>
	/// <param name="Url">elative URL</param>
	/// <param name="Filename">Local filename (optional)</param>
	void LoadScript(std::wstring Url, std::wstring Filename);

	/// <summary>
	///  Imports an HTML from URL. If Filename is specified, creates a session
	///  URL first and then imports the html file from that Filename.
	/// </summary>
	/// <param name="Url">elative URL</param>
	/// <param name="Filename">Local filename (optional)</param>
	void ImportHTML(std::wstring Url, std::wstring Filename);

	/// <summary>
	///  Creates reverse forwarding to the specified URL.
	///  Returns the virtualpath to be used
	/// </summary>
	/// <param name="Url">Relative URL</param>
	std::wstring CreateForwarding(std::wstring Url);
};


// Helper functions
std::wstring VUITrackName(IRecTrack* track);
long VUITrackPosition(IRecTrack* track);

std::wstring JSROGetArgumentAsString(IJSMethod* AMethod, std::wstring argName);
int JSROGetArgumentAsInt(IJSMethod* AMethod, std::wstring argName);
bool JSROGetArgumentAsBool(IJSMethod* AMethod, std::wstring argName);
float JSROGetArgumentAsFloat(IJSMethod* AMethod, std::wstring argName);

IJSEvent* JSROGetEvent(JSObject* AObject, std::wstring eventName);

void JSROSetEventArgAsString(IJSEvent* AEvent, std::wstring argName, std::wstring argValue);
void JSROSetEventArgAsInt(IJSEvent* AEvent, std::wstring argName, int argValue);
void JSROSetEventArgAsBool(IJSEvent* AEvent, std::wstring argName, bool argValue);
void JSROSetEventArgAsFloat(IJSEvent* AEvent, std::wstring argName, float argValue);

void JSROAddMethodParameter(IJSMethod* AMethod, std::wstring paramName, IJSDataType ADataType);
void JSROSetMethodReturnType(IJSMethod* AMethod, IJSDataType ADataType);
void JSROAddEventArgument(IJSEvent* AEvent, std::wstring argName, IJSDataType ADataType);

std::wstring JSROGetPropertyName(IJSProperty* AProperty);

void JSROSetPropAsString(IJSProperty* AProperty, std::wstring propValue);
void JSROSetPropAsInt(IJSProperty* AProperty, int propValue);
void JSROSetPropAsBool(IJSProperty* AProperty, bool propValue);
void JSROSetPropAsFloat(IJSProperty* AProperty, float propValue);

std::wstring JSROGetPropAsString(IJSProperty* AProperty);
int JSROGetPropAsInt(IJSProperty* AProperty);
bool JSROGetPropAsBool(IJSProperty* AProperty);
float JSROGetPropAsFloat(IJSProperty* AProperty);

#endif /*__THINFINITY_VIRTUALUI_H*/
