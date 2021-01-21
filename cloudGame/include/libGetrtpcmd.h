#pragma once
#include <string>  
#include <iostream> 
 class libGetrtpcmd
{
	public:
		static BYTE KeyCodeGet(int jskeycode);
		static std::string HttpGet(std::string url);
		static std::string HttpPostRaw(std::string url, std::string data);
		static void KeyDown(HANDLE handle, int key);
		static void KeyUp(HANDLE handle, int key);
		static void KeyPing(HANDLE handle);
		static void MouseMove(HANDLE handle, int x,int y,int code);
		static void MouseUp(HANDLE handle, int x, int y, int code);
		static void MouseDown(HANDLE handle, int x, int y, int code);
		static void JoystickCtrl(HANDLE handle, int X, int Y, int Z, int rX, int rY, int rZ, int slider, int dial, int wheel, BYTE hat, BYTE buttons[16]);
		static void KeyCodeInit();
		static BOOL DeviceOpen(HANDLE &handle, WORD wVID, WORD wPID);
		static void DeviceClose(HANDLE & handle);
		static bool wfile(std::string url, std::string filepath);
};

