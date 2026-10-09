#pragma once

#ifdef UNICODE
#define tstring wstring
#else
#define tstring string
#endif

#define HAVE_STRUCT_TIMESPEC


#ifdef WIN32
#include "event2/event.h"
#include "event2/buffer.h"
#include "event2/http.h"
#include "event2/http_struct.h"
#include "event2/keyvalq_struct.h"
#include "event2/buffer_compat.h"
#include "event2/thread.h"
#include "pthread.h"
#include <WinSock2.h>

//custom
#include "tchar.h"
#include "public.h"

#define _debug_to wprintf

#ifdef WIN32
#define COMMON_STRCPY(x, y, z) strcpy_s(x, z, y)
#else
#define COMMON_STRCPY(x, y, z) strncpy(x, y, z);
#endif

namespace httpServer
{
	typedef void(*http_cb_Func)(struct evhttp_request *, void *);//定义函数指针

	struct httpThread
	{
		HANDLE  threadhandle;
		struct event_base* event_base;
		struct evhttp* http_server;
		void *pparam;
		int   mark;
		HWND  mainWindow;
		DWORD threadid;
		CRITICAL_SECTION get_CriticalSection;

		httpThread()
		{
            threadhandle = nullptr;
            event_base = nullptr;
            http_server = nullptr;
            pparam = nullptr;
            threadid = 0;
            mainWindow = nullptr;
		}
	};


	struct complex_httpServer
	{
		struct httpThread *pSeverThread;
		HANDLE http_checkEvent[2];

		CRITICAL_SECTION get_CriticalSection;

		std::tstring ipaddr;
		int port;

		http_cb_Func pFunc;//回调函数指针

		HWND mainWindow;

		int nthreads;
		int backlog;
		int nfd;
		complex_httpServer()
		{
			::InitializeCriticalSection(&get_CriticalSection);
            mainWindow = nullptr;

            pSeverThread = nullptr;
            http_checkEvent[0] = nullptr;
            http_checkEvent[1] = nullptr;

			port = 8130;
			nthreads = 10;
			backlog = 10240;
			nfd = -1;
		}
		complex_httpServer(http_cb_Func _pFunc, HWND _mainWindow, int _port, int httpthreads, int nbacklog, std::tstring _ipaddr = _T(""))
		{
			::InitializeCriticalSection(&get_CriticalSection);
			if (_port < 1 || _port > 65535) _port = 8123;
			else port = _port;
            if (!globalIsIPStringValid(_ipaddr))
			{
				ipaddr = _T("0.0.0.0");
			}
			else
			{
				ipaddr = _ipaddr;
			}
			pFunc = _pFunc;

			mainWindow = _mainWindow;
            pSeverThread = nullptr;

            http_checkEvent[0] = nullptr;
            http_checkEvent[1] = nullptr;

			nthreads = httpthreads;
			backlog = nbacklog;
			nfd = -1;
		}
		~complex_httpServer()
		{
			stop_http_server();
			::DeleteCriticalSection(&get_CriticalSection);
		}

		int start_http_server();
		int start_http_server(http_cb_Func _pFunc, HWND _mainWindow, int _port, int httpthreads, int nbacklog, std::tstring _ipaddr = _T(""));
		void stop_failed_server(int threadcount);
		void stop_http_server();
	};
};
#else
#include <pthread.h>
#include "event2/thread.h"
#include "event2/buffer.h"
#include "evhttp.h"
#include "event.h"
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <unistd.h>
#include "public.h"
#include "logutilPublic.h"
namespace httpServer
{
	typedef void(*http_cb_Func)(struct evhttp_request *, void *);//定义函数指针

    typedef struct httpThread
	{
		pthread_t ths;
		struct event_base * eventbase;
		struct evhttp* httpserver;
		void *pparam;
		int   mark;

		char pathchar[MAX_PATH];
		char querychar[MAX_PATH];
		char hostchar[MAX_PATH];
		char urichar[MAX_PATH];
    }httpThread;

	typedef class complex_httpServer
	{
	public:
        httpThread* pSeverThread;
		http_cb_Func pFunc;//回调函数指针
		void* mainWindow;
		int nthreads;
		int backlog;
		int nfd;
		int port;

        std::tstring ipaddr;

		int starthttpserver();
        int start_http_server(http_cb_Func _pFunc, void* _mainWindow, int _port, int httpthreads, int nbacklog, std::tstring _ipaddr = _T(""));
		void stop_http_server();
	}complex_httpServer;
}
#endif
