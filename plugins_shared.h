#pragma	once

#ifdef PLAYCLAW_EXPORTS
#define PLAYCLAW_API __declspec(dllexport)
#else
#define PLAYCLAW_API __declspec(dllimport)
#endif

#define PLUGIN_EXPORT extern "C" __declspec(dllexport)


// plugin flags
#define PC_PLUGIN_FLAG_IS_ACTIVE				0x00000001
#define PC_PLUGIN_FLAG_IS_OVERLAY				0x00000002
#define PC_PLUGIN_FLAG_OVERLAY_HAS_ALIGN		0x00010000



// locking struct
struct OverlayLockStruct
{
	BYTE*		pBuffer;
	DWORD		dwWidth;
	DWORD		dwHeight;
	DWORD		dwPitch;
};

PLAYCLAW_API BOOL				PC_LockOverlay(DWORD dwPluginID, OverlayLockStruct *map_desc);
PLAYCLAW_API void				PC_UnlockOverlay(DWORD dwPluginID, BOOL bMarkUpdated);



// common plugin variables
#define PLUGIN_VAR_ENABLED		"enabled"

// common overlay variables
#define OVR_VAR_POSITION_X		"pos_x"
#define OVR_VAR_POSITION_Y		"pos_y"
#define OVR_VAR_LINK_LEFT		"link_left"
#define OVR_VAR_LINK_TOP		"link_top"
#define OVR_VAR_SIZE_X			"size_x"
#define OVR_VAR_SIZE_Y			"size_y"
#define OVR_VAR_RENDER_LAYER	"render_layer"
#define OVR_VAR_CAPTURE_FLAGS	"capture_flags"
#define OVR_VAR_ALIGN_LEFT		"align_left"


PLAYCLAW_API void				PC_SetPluginVar(DWORD dwPluginID, const char *name, int value);
PLAYCLAW_API void				PC_SetPluginVar(DWORD dwPluginID, const char *name, const WCHAR *value);
PLAYCLAW_API int				PC_GetPluginVarInt(DWORD dwPluginID, const char *name);
PLAYCLAW_API const WCHAR*		PC_GetPluginVarStr(DWORD dwPluginID, const char *name);
PLAYCLAW_API const char*		PC_GetVarPrefix(DWORD dwPluginID);


//---------------- hotkeys -----------------------

typedef void(__stdcall* PC_HOTKEY_CB)(DWORD, UINT, BOOL);

PLAYCLAW_API int				PC_ShowBindDialog();
PLAYCLAW_API void				PC_GetHotkeyTitle(int hotkey, WCHAR *szTitle, DWORD dwTitleSize);
PLAYCLAW_API DWORD				PC_CreateHotkey(DWORD hotkey, PC_HOTKEY_CB proc, UINT param);
PLAYCLAW_API void				PC_DeleteHotkey(DWORD hotkeyIndex);


//--------------- frame capture -------------------

PLAYCLAW_API void				PC_StartPureCapture();
PLAYCLAW_API void				PC_StopPureCapture();
// capture frame in RGB24 array
// buffers are allocated by HeapAlloc/HeapFree
PLAYCLAW_API BYTE*				PC_CaptureFrame(BYTE *pBufferRGB24, DWORD *pdwWidth, DWORD *pdwHeight);



//--------------- misc functions -------------------

// realtime info from game
PLAYCLAW_API DWORD				PC_GetFPSCounter();

// capture/recording/streaming 
PLAYCLAW_API BOOL				PC_IsCapturing();
PLAYCLAW_API BOOL				PC_IsRecording();
PLAYCLAW_API BOOL				PC_IsPausedRecording();
PLAYCLAW_API BOOL				PC_IsPreRecording();
PLAYCLAW_API BOOL				PC_IsStreaming();
PLAYCLAW_API void				PC_StartRecording();
PLAYCLAW_API void				PC_StopRecording();
PLAYCLAW_API DWORD				PC_GetRecordedFileSize();	// in kbytes
PLAYCLAW_API DWORD				PC_GetRecordedFileTime();	// in seconds
PLAYCLAW_API DWORD				PC_GetPreRecordedDataSize();	// in msec
PLAYCLAW_API DWORD				PC_GetStreamedSize();	// in kbytes
PLAYCLAW_API DWORD				PC_GetStreamState();
PLAYCLAW_API DWORD				PC_GetDeltaScreenshotTime();

// common system functions
PLAYCLAW_API const WCHAR*		PC_GetAppPath();
PLAYCLAW_API void				PC_DebugPrint(const WCHAR *format, ...);
PLAYCLAW_API DWORD				PC_GetConfirmedProcessID();
PLAYCLAW_API const WCHAR*		PC_GetCurrentProfileName();
PLAYCLAW_API void				PC_GetAppDataPath(WCHAR *pszPath, DWORD dwMaxChars);

// languages
PLAYCLAW_API void				PC_LocalizeDialog(DWORD dwPluginID, HWND wnd);
PLAYCLAW_API const WCHAR*		PC_GetLocalizedString(DWORD dwPluginID, const char *str_id);





#define SAFE_DELETE(x) {if(x){delete (x);(x)=0;}}
#define SAFE_GDIPLUS_DELETE(p) { if(p) {::delete (p); (p)=0;}}
#define SAFE_CLOSE_HANDLE(h)		{ if (h) { CloseHandle(h); (h)=NULL; } }