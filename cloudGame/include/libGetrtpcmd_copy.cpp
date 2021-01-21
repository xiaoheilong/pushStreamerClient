//#include "pch.h"
#define WIN32_LEAN_AND_MEAN             // 从 Windows 头文件中排除极少使用的内容
// Windows 头文件
#include <windows.h>
#include "libGetrtpcmd.h"
#include <string>  
#include <iostream> 
#include "curl/curl.h"
#include <assert.h> 
#include <stdio.h>
#include <stdlib.h>

#include <io.h>

#include <list>
#include <map>

#include <setupapi.h>
#include <hidsdi.h>
using namespace std;

#pragma comment(lib, "hid.lib")
#pragma comment(lib,"setupapi") 

#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "wldap32.lib")
#pragma comment(lib, "crypt32.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "libcurl.lib")
#pragma comment(lib, "libssl.lib")
#pragma comment(lib, "libcrypto.lib")

list<std::string>  intlist;
int* keycodelist;
int Modkeycodelist;
map<int, BYTE> KeyCode;
map<int, BYTE> ModifiersKeyCodes;
#define  SKIP_PEER_VERIFICATION 1  
//#define  SKIP_HOSTNAME_VERFICATION 1  
#pragma pack(push)
#pragma pack(1)
typedef struct _HIDMINI_CONTROL_INFO {
	UCHAR ReportId;
	UCHAR CommandCode;
	UINT32 timeout; //you must ping the driver every (timeout / 5) seconds or the driver will reset itself and release all keypresses
	BYTE modifier = 0;
	BYTE padding = 0;
	BYTE key0 = 0;
	BYTE key1 = 0;
	BYTE key2 = 0;
	BYTE key3 = 0;
	BYTE key4 = 0;
	BYTE key5 = 0;
} HIDMINI_CONTROL_INFO, *PHIDMINI_CONTROL_INFO;

#pragma pack(1)
typedef struct _HIDMINI_CONTROL_INFO_Mouse {
	BYTE ReportId;
	BYTE CommandCode;
	BYTE buttons;
	UINT16 X;
	UINT16 Y;
} HIDMINI_CONTROL_INFO_Mouse, *PHIDMINI_CONTROL_INFO_Mouse;
#pragma pack(pop)

struct FtpFile {
	const char *filename;
	FILE *stream;
};

static size_t my_fwrite(void *buffer, size_t size, size_t nmemb, void *stream)
{
	struct FtpFile *out = (struct FtpFile *)stream;
	if (out && !out->stream) {
		out->stream = fopen(out->filename, "wb");//打开文件进行写入
		if (!out->stream)
			return -1;
	}
	return fwrite(buffer, size, nmemb, out->stream);
}

/*
ptr是指向存储数据的指针，
size是每个块的大小，
nmemb是指块的数目，
stream是用户参数。
所以根据以上这些参数的信息可以知道，ptr中的数据的总长度是size*nmemb
*/
size_t call_wirte_func(const char *ptr, size_t size, size_t nmemb, std::string *stream)
{
	assert(stream != NULL);
	size_t len = size * nmemb;
	stream->append(ptr, len);
	return len;
}
int write_func(char *ptr, size_t size, size_t nmemb, void *userdata)
{
	string* buffer = (string*)userdata;
	size_t len = size*nmemb;
	// 	int offset = strlen(buffer);
	// 	memcpy(buffer + offset, ptr, len);
	// 	buffer[len + offset] = 0;
	buffer->append(ptr, len);
	return len;
}
// 返回http header回调函数    
size_t header_callback(const char  *ptr, size_t size, size_t nmemb, std::string *stream)
{
	assert(stream != NULL);
	size_t len = size * nmemb;
	stream->append(ptr, len);
	return len;
}

BYTE libGetrtpcmd::KeyCodeGet(int jskeycode)
{
	auto it = KeyCode.find(jskeycode);
	if (it != KeyCode.end()) {
		return KeyCode[jskeycode];
	}
	
	auto itModifiers = ModifiersKeyCodes.find(jskeycode);
	if (itModifiers != ModifiersKeyCodes.end()) {
		return ModifiersKeyCodes[jskeycode];
	}
	return 0;
}

std::string libGetrtpcmd::HttpGet(std::string url)
{
	CURL *curl;
	CURLcode code;
	std::string szbuffer;
	std::string szheader_buffer;
	char errorBuffer[CURL_ERROR_SIZE];
	//std::string url = "http://www.douban.com";
	//std::string url = "https://vip.icbc.com.cn/icbc/perbank/index.jsp";  
	std::string useragent = "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:13.0) Gecko/20100101 Firefox/13.0.1";
	/*
	CURL_GLOBAL_ALL                //初始化所有的可能的调用。
	CURL_GLOBAL_SSL               //初始化支持 安全套接字层。
	CURL_GLOBAL_WIN32            //初始化win32套接字库。
	CURL_GLOBAL_NOTHING         //没有额外的初始化。
	*/
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if (curl) {
		// 远程URL，支持 http, https, ftp  
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent.c_str());
		// 官方下载的DLL并不支持GZIP，Accept-Encoding:deflate, gzip  
		//curl_easy_setopt(curl, CURLOPT_ENCODING, "gzip, deflate");
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);//调试信息打开  
		//https 访问专用：start  
#ifdef SKIP_PEER_VERIFICATION  
		//跳过服务器SSL验证，不使用CA证书  
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, FALSE);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		//如果不跳过SSL验证，则可指定一个CA证书目录  
		//curl_easy_setopt(curl, CURLOPT_CAPATH, "this is ca ceat");  
		//curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT);
		//curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
#endif  

#ifdef SKIP_HOSTNAME_VERFICATION  
		//验证服务器端发送的证书，默认是 2(高)，1（中），0（禁用）  
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif  
		//https 访问专用：end  

		//发送cookie值给服务器  
		//curl_easy_setopt(curl, CURLOPT_COOKIE, "name1=var1; name2=var2;");   
		/* 与服务器通信交互cookie，默认在内存中，可以是不存在磁盘中的文件或留空 */
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "./cookie.txt");
		/* 与多个CURL或浏览器交互cookie，会在释放内存后写入磁盘文件 */
		curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "./cookie.txt");

		/* POST 数据 */
		// curl_easy_setopt(curl, CURLOPT_POSTFIELDS, "name=daniel&project=curl");  
		//设置重定向的最大次数  
		curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
		//设置301、302跳转跟随location  
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		//抓取内容后，回调函数  
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &szbuffer);
		//抓取头信息，回调函数  
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void *)&szheader_buffer);

		/*curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);*/
		/*
		CURLE_OK    任务完成一切都好
		CURLE_UNSUPPORTED_PROTOCOL  不支持的协议，由URL的头部指定
		CURLE_COULDNT_CONNECT   不能连接到remote 主机或者代理
		CURLE_REMOTE_ACCESS_DENIED  访问被拒绝
		CURLE_HTTP_RETURNED_ERROR   Http返回错误
		CURLE_READ_ERROR    读本地文件错误
		CURLE_SSL_CACERT    访问HTTPS时需要CA证书路径
		*/
		code = curl_easy_perform(curl);
		if (CURLE_OK == code) {
			double val;

			/* check for bytes downloaded */
			code = curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &val);
			if ((CURLE_OK == code) && (val > 0))
				printf("Data downloaded: %0.0f bytes.\n", val);

			/* check for total download time */
			code = curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &val);
			if ((CURLE_OK == code) && (val > 0))
				printf("Total download time: %0.3f sec.\n", val);

			/* check for average download speed */
			code = curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &val);
			if ((CURLE_OK == code) && (val > 0))
				printf("Average download speed: %0.3f kbyte/sec.\n", val / 1024);
			
			printf("%s\n", szbuffer.c_str());
		}
		else {
			//fprintf(stderr, "Failed to get '%s' [%s]\n", url, errorBuffer);
			// exit(EXIT_FAILURE);  
		}

		/* 释放内存 */
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();

	//getchar();
	return szbuffer;
}

std::string libGetrtpcmd::HttpPostRaw(std::string url, std::string data)
{
	CURL *curl;
	CURLcode code;
	std::string szbuffer;
	std::string szheader_buffer;
	char errorBuffer[CURL_ERROR_SIZE];
	//std::string url = "http://www.douban.com";
	//std::string url = "https://vip.icbc.com.cn/icbc/perbank/index.jsp";  
	std::string useragent = "Mozilla/5.0 (Windows NT 6.1; WOW64; rv:13.0) Gecko/20100101 Firefox/13.0.1";
	/*
	CURL_GLOBAL_ALL                //初始化所有的可能的调用。
	CURL_GLOBAL_SSL               //初始化支持 安全套接字层。
	CURL_GLOBAL_WIN32            //初始化win32套接字库。
	CURL_GLOBAL_NOTHING         //没有额外的初始化。
	*/
	curl_global_init(CURL_GLOBAL_ALL);
	curl = curl_easy_init();
	if (curl) {
		// 远程URL，支持 http, https, ftp  
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
		curl_easy_setopt(curl, CURLOPT_USERAGENT, useragent.c_str());
		// 官方下载的DLL并不支持GZIP，Accept-Encoding:deflate, gzip  
		
		//curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);//调试信息打开  
		//https 访问专用：start  
#ifdef SKIP_PEER_VERIFICATION  
		//跳过服务器SSL验证，不使用CA证书  
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, FALSE);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		//如果不跳过SSL验证，则可指定一个CA证书目录  
		//curl_easy_setopt(curl, CURLOPT_CAPATH, "this is ca ceat");  
		//curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT);
		//curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
#endif  

#ifdef SKIP_HOSTNAME_VERFICATION  
		//验证服务器端发送的证书，默认是 2(高)，1（中），0（禁用）  
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
#endif  
		//https 访问专用：end  

		//发送cookie值给服务器  
		//curl_easy_setopt(curl, CURLOPT_COOKIE, "name1=var1; name2=var2;");   
		/* 与服务器通信交互cookie，默认在内存中，可以是不存在磁盘中的文件或留空 */
		curl_easy_setopt(curl, CURLOPT_COOKIEFILE, "./cookie.txt");
		/* 与多个CURL或浏览器交互cookie，会在释放内存后写入磁盘文件 */
		curl_easy_setopt(curl, CURLOPT_COOKIEJAR, "./cookie.txt");

		/* POST 数据 */
		struct curl_slist* headers = NULL;
		headers = curl_slist_append(headers, "Content-Type:application/json;charset=UTF-8");
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);

		curl_easy_setopt(curl, CURLOPT_POSTFIELDS,data.c_str());  
		//设置重定向的最大次数  
		curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 5);
		//设置301、302跳转跟随location  
		curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
		//抓取内容后，回调函数  
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_func);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &szbuffer);
		//抓取头信息，回调函数  
		curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, header_callback);
		curl_easy_setopt(curl, CURLOPT_HEADERDATA, (void *)&szheader_buffer);

		/*curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L);*/
		/*
		CURLE_OK    任务完成一切都好
		CURLE_UNSUPPORTED_PROTOCOL  不支持的协议，由URL的头部指定
		CURLE_COULDNT_CONNECT   不能连接到remote 主机或者代理
		CURLE_REMOTE_ACCESS_DENIED  访问被拒绝
		CURLE_HTTP_RETURNED_ERROR   Http返回错误
		CURLE_READ_ERROR    读本地文件错误
		CURLE_SSL_CACERT    访问HTTPS时需要CA证书路径
		*/
		code = curl_easy_perform(curl);
		if (CURLE_OK == code) {
			double val;

			/* check for bytes downloaded */
			code = curl_easy_getinfo(curl, CURLINFO_SIZE_DOWNLOAD, &val);
			if ((CURLE_OK == code) && (val > 0))
				printf("Data downloaded: %0.0f bytes.\n", val);

			/* check for total download time */
			code = curl_easy_getinfo(curl, CURLINFO_TOTAL_TIME, &val);
			if ((CURLE_OK == code) && (val > 0))
				printf("Total download time: %0.3f sec.\n", val);

			/* check for average download speed */
			code = curl_easy_getinfo(curl, CURLINFO_SPEED_DOWNLOAD, &val);
			if ((CURLE_OK == code) && (val > 0))
				printf("Average download speed: %0.3f kbyte/sec.\n", val / 1024);

			printf("%s\n", szbuffer.c_str());
		}
		else {
			//fprintf(stderr, "Failed to get '%s' [%s]\n", url, errorBuffer);
			// exit(EXIT_FAILURE);  
		}

		/* 释放内存 */
		curl_easy_cleanup(curl);
	}
	curl_global_cleanup();
	return szbuffer;
}

void libGetrtpcmd::KeyDown(HANDLE handle,int jskeycode)
{
	HIDMINI_CONTROL_INFO keyinfo;
	keyinfo.CommandCode = 2;
	keyinfo.ReportId = 1;
	keyinfo.timeout = 1000;
	keyinfo.padding = 0;
	
	auto it = KeyCode.find(jskeycode);
	if (it != KeyCode.end()) {
		bool blnhs = false;
		for (int i = 0; i < 5; i++)
		{
			if (keycodelist[i] == jskeycode)
			{
				blnhs = true;
			}
		}
		if (!blnhs) 
		{
			for (int i = 0; i < 5; i++)
			{
				if (keycodelist[i] == 0)
				{
					keycodelist[i] = jskeycode;
					break;
				}
			}
		}
		//keyinfo.key0 = KeyCode[jskeycode];
		keyinfo.key0 = KeyCode[keycodelist[0]];
		keyinfo.key1 = KeyCode[keycodelist[1]];
		keyinfo.key2 = KeyCode[keycodelist[2]];
		keyinfo.key3 = KeyCode[keycodelist[3]];
		keyinfo.key4 = KeyCode[keycodelist[4]];
	}
	auto itModifiers = ModifiersKeyCodes.find(jskeycode);
	if (itModifiers != ModifiersKeyCodes.end()) {
		Modkeycodelist = jskeycode;
	}
	keyinfo.modifier = ModifiersKeyCodes[Modkeycodelist];
	HidD_SetFeature(handle, &keyinfo, sizeof(keyinfo)+1);
}
void libGetrtpcmd::KeyUp(HANDLE handle, int jskeycode)
{
	HIDMINI_CONTROL_INFO keyinfo;
	keyinfo.CommandCode = 2;
	keyinfo.ReportId = 1;
	keyinfo.timeout = 1000;
	keyinfo.padding = 0;
	for (int i = 0; i < 5; i++) 
	{
		if (keycodelist[i] == jskeycode) 
		{
			keycodelist[i] = 0;
		}
	}
	if (Modkeycodelist == jskeycode) 
	{
		Modkeycodelist = 0;
	}
	keyinfo.modifier = ModifiersKeyCodes[Modkeycodelist];
	keyinfo.key0 = KeyCode[keycodelist[0]];
	keyinfo.key1 = KeyCode[keycodelist[1]];
	keyinfo.key2 = KeyCode[keycodelist[2]];
	keyinfo.key3 = KeyCode[keycodelist[3]];
	keyinfo.key4 = KeyCode[keycodelist[4]];
	HidD_SetFeature(handle, &keyinfo, sizeof(keyinfo) + 1);
}
void libGetrtpcmd::KeyPing(HANDLE handle)
{
	HIDMINI_CONTROL_INFO keyinfo;
	keyinfo.CommandCode = 3;
	keyinfo.ReportId = 1;
	keyinfo.timeout = 1000;
	keyinfo.padding = 0;
	HidD_SetFeature(handle, &keyinfo, sizeof(keyinfo) + 1);
}
void libGetrtpcmd::MouseMove(HANDLE handle, int x, int y, int code)
{
	HIDMINI_CONTROL_INFO_Mouse mouseinfo;
	mouseinfo.buttons = 0x00000000;
	if (code == 1)
	{
		mouseinfo.buttons = 0x00000001;
	}
	if (code == 2)
	{
		mouseinfo.buttons = 4;
	}
	if (code == 3)
	{
		mouseinfo.buttons = 2;
	}
	mouseinfo.X = x;
	mouseinfo.Y = y;
	mouseinfo.ReportId = 1;
	mouseinfo.CommandCode = 2;
	HidD_SetFeature(handle, &mouseinfo, sizeof(mouseinfo) + 1);
}
void libGetrtpcmd::MouseUp(HANDLE handle, int x, int y, int code)
{
	HIDMINI_CONTROL_INFO_Mouse mouseinfo;
	mouseinfo.buttons = 0x00000000;
	if (code == 1)
	{
		mouseinfo.buttons = 0x00000001;
	}
	if (code == 2)
	{
		mouseinfo.buttons = 4;
	}
	if (code == 3)
	{
		mouseinfo.buttons = 2;
	}
	mouseinfo.X = x;
	mouseinfo.Y = y;
	mouseinfo.ReportId = 1;
	mouseinfo.CommandCode = 2;
	HidD_SetFeature(handle, &mouseinfo, sizeof(mouseinfo) + 1);
}
void libGetrtpcmd::MouseDown(HANDLE handle, int x, int y, int code)
{
	HIDMINI_CONTROL_INFO_Mouse mouseinfo;
	mouseinfo.buttons = 0x00000000;
	if (code == 1)
	{
		mouseinfo.buttons = 0x00000001;
	}
	if (code == 2)
	{
		mouseinfo.buttons = 4;
	}
	if (code == 3)
	{
		mouseinfo.buttons = 2;
	}
	mouseinfo.X = x;
	mouseinfo.Y = y;
	mouseinfo.ReportId = 1;
	mouseinfo.CommandCode = 2;
	HidD_SetFeature(handle, &mouseinfo, sizeof(mouseinfo)+1);
}
void libGetrtpcmd::KeyCodeInit()
{
	keycodelist = new int[5];
	for (int i = 0; i < 5; i++)
	{
		keycodelist[i] = 0;
	}
	KeyCode.insert(pair<int, BYTE>(0, 0));
	KeyCode.insert(pair<int, BYTE>(48, 39));
	KeyCode.insert(pair<int, BYTE>(49, 30));
	KeyCode.insert(pair<int, BYTE>(50, 31));
	KeyCode.insert(pair<int, BYTE>(51, 32));
	KeyCode.insert(pair<int, BYTE>(52, 33));
	KeyCode.insert(pair<int, BYTE>(53, 34));
	KeyCode.insert(pair<int, BYTE>(54, 35));
	KeyCode.insert(pair<int, BYTE>(55, 36));
	KeyCode.insert(pair<int, BYTE>(56, 37));
	KeyCode.insert(pair<int, BYTE>(57, 38));
	KeyCode.insert(pair<int, BYTE>(65, 4));
	KeyCode.insert(pair<int, BYTE>(66, 5));
	KeyCode.insert(pair<int, BYTE>(67, 6));
	KeyCode.insert(pair<int, BYTE>(68, 7));
	KeyCode.insert(pair<int, BYTE>(69, 8));
	KeyCode.insert(pair<int, BYTE>(70, 9));
	KeyCode.insert(pair<int, BYTE>(71, 10));
	KeyCode.insert(pair<int, BYTE>(72, 11));
	KeyCode.insert(pair<int, BYTE>(73, 12));
	KeyCode.insert(pair<int, BYTE>(74, 13));
	KeyCode.insert(pair<int, BYTE>(75, 14));
	KeyCode.insert(pair<int, BYTE>(76, 15));
	KeyCode.insert(pair<int, BYTE>(77, 16));
	KeyCode.insert(pair<int, BYTE>(78, 17));
	KeyCode.insert(pair<int, BYTE>(79, 18));
	KeyCode.insert(pair<int, BYTE>(80, 19));
	KeyCode.insert(pair<int, BYTE>(81, 20));
	KeyCode.insert(pair<int, BYTE>(82, 21));
	KeyCode.insert(pair<int, BYTE>(83, 22));
	KeyCode.insert(pair<int, BYTE>(84, 23));
	KeyCode.insert(pair<int, BYTE>(85, 24));
	KeyCode.insert(pair<int, BYTE>(86, 25));
	KeyCode.insert(pair<int, BYTE>(87, 26));
	KeyCode.insert(pair<int, BYTE>(88, 27));
	KeyCode.insert(pair<int, BYTE>(89, 28));
	KeyCode.insert(pair<int, BYTE>(90, 29));
	KeyCode.insert(pair<int, BYTE>(96, 98));
	KeyCode.insert(pair<int, BYTE>(97, 89));
	KeyCode.insert(pair<int, BYTE>(98, 90));
	KeyCode.insert(pair<int, BYTE>(99, 91));
	KeyCode.insert(pair<int, BYTE>(100, 92));
	KeyCode.insert(pair<int, BYTE>(101, 93));
	KeyCode.insert(pair<int, BYTE>(102, 94));
	KeyCode.insert(pair<int, BYTE>(103, 95));
	KeyCode.insert(pair<int, BYTE>(104, 96));
	KeyCode.insert(pair<int, BYTE>(105, 97));
	KeyCode.insert(pair<int, BYTE>(106, 85));
	KeyCode.insert(pair<int, BYTE>(107, 87));
	KeyCode.insert(pair<int, BYTE>(108, 88));
	KeyCode.insert(pair<int, BYTE>(109, 86));
	KeyCode.insert(pair<int, BYTE>(110, 99));
	KeyCode.insert(pair<int, BYTE>(111, 84));
	KeyCode.insert(pair<int, BYTE>(112, 58));
	KeyCode.insert(pair<int, BYTE>(113, 59));
	KeyCode.insert(pair<int, BYTE>(114, 60));
	KeyCode.insert(pair<int, BYTE>(115, 61));
	KeyCode.insert(pair<int, BYTE>(116, 62));
	KeyCode.insert(pair<int, BYTE>(117, 63));
	KeyCode.insert(pair<int, BYTE>(118, 64));
	KeyCode.insert(pair<int, BYTE>(119, 65));
	KeyCode.insert(pair<int, BYTE>(120, 66));
	KeyCode.insert(pair<int, BYTE>(121, 67));
	KeyCode.insert(pair<int, BYTE>(122, 68));
	KeyCode.insert(pair<int, BYTE>(123, 69));
	KeyCode.insert(pair<int, BYTE>(8, 42));
	KeyCode.insert(pair<int, BYTE>(9, 43));
	KeyCode.insert(pair<int, BYTE>(13, 40));
	KeyCode.insert(pair<int, BYTE>(20, 57));
	KeyCode.insert(pair<int, BYTE>(27, 41));
	KeyCode.insert(pair<int, BYTE>(32, 44));
	KeyCode.insert(pair<int, BYTE>(33, 75));
	KeyCode.insert(pair<int, BYTE>(34, 78));
	KeyCode.insert(pair<int, BYTE>(35, 77));
	KeyCode.insert(pair<int, BYTE>(36, 74));
	KeyCode.insert(pair<int, BYTE>(37, 80));
	KeyCode.insert(pair<int, BYTE>(38, 82));
	KeyCode.insert(pair<int, BYTE>(39, 79));
	KeyCode.insert(pair<int, BYTE>(40, 81));
	KeyCode.insert(pair<int, BYTE>(45, 73));
	KeyCode.insert(pair<int, BYTE>(46, 76));
	KeyCode.insert(pair<int, BYTE>(144, 83));
	KeyCode.insert(pair<int, BYTE>(186, 51));
	KeyCode.insert(pair<int, BYTE>(187, 46));
	KeyCode.insert(pair<int, BYTE>(188, 54));
	KeyCode.insert(pair<int, BYTE>(189, 45));
	KeyCode.insert(pair<int, BYTE>(190, 55));
	KeyCode.insert(pair<int, BYTE>(191, 56));
	KeyCode.insert(pair<int, BYTE>(192, 53));
	KeyCode.insert(pair<int, BYTE>(219, 47));
	KeyCode.insert(pair<int, BYTE>(220, 49));
	KeyCode.insert(pair<int, BYTE>(221, 48));
	KeyCode.insert(pair<int, BYTE>(222, 50));
	ModifiersKeyCodes.insert(pair<int, BYTE>(0, 0));
	ModifiersKeyCodes.insert(pair<int, BYTE>(17, 1));
	ModifiersKeyCodes.insert(pair<int, BYTE>(16, 2));
	ModifiersKeyCodes.insert(pair<int, BYTE>(18, 4));
	ModifiersKeyCodes.insert(pair<int, BYTE>(91, 8));
	ModifiersKeyCodes.insert(pair<int, BYTE>(317, 16));
	ModifiersKeyCodes.insert(pair<int, BYTE>(316, 32));
	ModifiersKeyCodes.insert(pair<int, BYTE>(318, 64));
	ModifiersKeyCodes.insert(pair<int, BYTE>(391, 128));
	
}
BOOL libGetrtpcmd::DeviceOpen(HANDLE &handle, WORD wVID, WORD wPID)
{
	BOOL bRet = FALSE;
	GUID hidGuid;
	HDEVINFO hardwareDeviceInfo;
	SP_INTERFACE_DEVICE_DATA deviceInfoData;
	PSP_INTERFACE_DEVICE_DETAIL_DATA functionClassDeviceData = NULL;
	ULONG predictedLength = 0;
	ULONG requiredLength = 0;
	CloseHandle(handle);
	handle = INVALID_HANDLE_VALUE;
	deviceInfoData.cbSize = sizeof(SP_INTERFACE_DEVICE_DATA);
	HidD_GetHidGuid(&hidGuid);
	hardwareDeviceInfo = SetupDiGetClassDevs(&hidGuid, NULL, NULL, (DIGCF_PRESENT | DIGCF_DEVICEINTERFACE));
	for (int i = 0; i < 1024; i++)
	{
		if (!SetupDiEnumDeviceInterfaces(hardwareDeviceInfo, 0, &hidGuid, i, &deviceInfoData)) continue;
		SetupDiGetDeviceInterfaceDetail(hardwareDeviceInfo, &deviceInfoData, NULL, 0, &requiredLength, NULL);
		predictedLength = requiredLength;
		functionClassDeviceData = (PSP_INTERFACE_DEVICE_DETAIL_DATA)malloc(predictedLength);
		if (!functionClassDeviceData) continue;
		functionClassDeviceData->cbSize = sizeof(SP_INTERFACE_DEVICE_DETAIL_DATA);
		if (!SetupDiGetDeviceInterfaceDetail(hardwareDeviceInfo, &deviceInfoData, functionClassDeviceData, predictedLength, &requiredLength, NULL))break;
		handle = CreateFile(functionClassDeviceData->DevicePath, 0, FILE_SHARE_READ | FILE_SHARE_WRITE, (LPSECURITY_ATTRIBUTES)NULL, OPEN_EXISTING, 0, NULL);
		if (handle != INVALID_HANDLE_VALUE)
		{
			HIDD_ATTRIBUTES attri;
			HidD_GetAttributes(handle, &attri);
			if ((attri.VendorID == wVID) &&
				(attri.ProductID == wPID))
			{
				bRet = TRUE;
				break;
			}
			CloseHandle(handle);
			handle = INVALID_HANDLE_VALUE;
		}
	}
	SetupDiDestroyDeviceInfoList(hardwareDeviceInfo);
	return bRet;
}
void  libGetrtpcmd::DeviceClose(HANDLE &handle)

{
	CloseHandle(handle);
	handle = INVALID_HANDLE_VALUE;
}
bool libGetrtpcmd::wfile(std::string url, std::string filepath)
{
	CURL *curl;
	CURLcode res;
	struct FtpFile ftpfile = {
		filepath.c_str(), //定义下载到本地的文件位置和路径
		NULL
	};
	curl_global_init(CURL_GLOBAL_DEFAULT);
	curl = curl_easy_init();  //初始化一个curl指针
	if (curl) { //curl对象存在的情况下执行的操作
		//设置远端地址
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
#ifdef SKIP_PEER_VERIFICATION  
		//跳过服务器SSL验证，不使用CA证书  
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, FALSE);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, FALSE);
		curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
		//如果不跳过SSL验证，则可指定一个CA证书目录  
		//curl_easy_setopt(curl, CURLOPT_CAPATH, "this is ca ceat");  
		//curl_easy_setopt(curl, CURLOPT_SSLENGINE_DEFAULT);
		//curl_easy_setopt(curl, CURLOPT_CAINFO, "cacert.pem");
#endif  
		//执行写入文件流操作
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, my_fwrite);//当有数据被写入，回调函数被调用，
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, &ftpfile); //设置结构体的指针传递给回调函数
		//启用时会汇报所有的信息，存放在STDERR或指定的CURLOPT_STDERR中
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
		curl_easy_setopt(curl, CURLOPT_USERPWD, "SUREN:SUREN");
		//写入文件
		res = curl_easy_perform(curl);
		//释放curl对象
		curl_easy_cleanup(curl);
		if (res != CURLE_OK)
		{
			return false;
		}
	}
	if (ftpfile.stream)
	{
		//关闭文件流
		fclose(ftpfile.stream);
	}
	//释放全局curl对象
	curl_global_cleanup();
	return true;
}
