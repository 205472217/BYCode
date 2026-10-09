// HttpServer.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#pragma comment(lib,"ws2_32.lib")

#include "event.h"
#include "evhttp.h"
#include "event2/http.h"
#include "event2/event.h"
#include "event2/buffer.h"
#include "event2/bufferevent.h"
#include "event2/bufferevent_compat.h"
#include "event2/http_struct.h"
#include "event2/http_compat.h"
#include "event2/util.h"
#include "event2/listener.h"
#pragma comment(lib,"libevent.lib")
#pragma comment(lib,"libevent_core.lib")
#pragma comment(lib,"libevent_extras.lib")
#pragma comment(lib,"libssl.lib")
#pragma comment(lib,"pthreadVC2.lib")
#pragma comment(lib,"ssh.lib")
#pragma comment(lib,"ssleay32.lib")

#include "json.h"


#if  0	//原始libevent
//缓存buffer大小定义
#define BUF_MAX 1024*10

//解析post请求数据
void get_post_message(char* buf, struct evhttp_request* req)
{
	size_t post_size = 0;

	post_size = evbuffer_get_length(req->input_buffer);//获取数据长度
	printf("====line:%d,post len:%d\n", __LINE__, post_size);
	if (post_size <= 0)
	{
		printf("====line:%d,post msg is empty!\n", __LINE__);
		return;
	}
	else
	{
		size_t copy_len = post_size > BUF_MAX ? BUF_MAX : post_size;
		printf("====line:%d,post len:%d, copy_len:%d\n", __LINE__, post_size, copy_len);
		memcpy(buf, evbuffer_pullup(req->input_buffer, -1), copy_len);
		buf[post_size] = '\0';
		printf("====line:%d,post msg:%s\n", __LINE__, buf);
	}
}

//解析http头，主要用于get请求时解析uri和请求参数
char* find_http_header(struct evhttp_request* req, struct evkeyvalq* params, const char* query_char)
{
	if (req == NULL || params == NULL || query_char == NULL)
	{
		printf("====line:%d,%s\n", __LINE__, "input params is null.");
		return NULL;
	}

	struct evhttp_uri* decoded = NULL;
	char* query = NULL;
	char* query_result = NULL;
	const char* path;
	const char* uri = evhttp_request_get_uri(req);//获取请求uri

	if (uri == NULL)
	{
		printf("====line:%d,evhttp_request_get_uri return null\n", __LINE__);
		return NULL;
	}
	else
	{
		printf("====line:%d,Got a GET request for <%s>\n", __LINE__, uri);
	}

	//解码uri
	decoded = evhttp_uri_parse(uri);
	if (!decoded)
	{
		printf("====line:%d,It's not a good URI. Sending BADREQUEST\n", __LINE__);
		evhttp_send_error(req, HTTP_BADREQUEST, 0);
		return NULL;
	}

	//获取uri中的path部分
	path = evhttp_uri_get_path(decoded);
	if (path == NULL)
	{
		path = "/";
	}
	else
	{
		printf("====line:%d,path is:%s\n", __LINE__, path);
	}

	//获取uri中的参数部分
	query = (char*)evhttp_uri_get_query(decoded);
	if (query == NULL)
	{
		printf("====line:%d,evhttp_uri_get_query return null\n", __LINE__);
		return NULL;
	}

	//查询指定参数的值
	evhttp_parse_query_str(query, params);
	query_result = (char*)evhttp_find_header(params, query_char);

	return query_result;
}

//处理get请求
void http_handler_testget_msg(struct evhttp_request* req, void* arg)
{
	if (req == NULL)
	{
		printf("====line:%d,%s\n", __LINE__, "input param req is null.");
		return;
	}

	char* sign = NULL;
	char* data = NULL;
	struct evkeyvalq sign_params = { 0 };
	sign = find_http_header(req, &sign_params, "sign");//获取get请求uri中的sign参数
	if (sign == NULL)
	{
		printf("====line:%d,%s\n", __LINE__, "request uri no param sign.");
	}
	else
	{
		printf("====line:%d,get request param: sign=[%s]\n", __LINE__, sign);
	}

	data = find_http_header(req, &sign_params, "data");//获取get请求uri中的data参数
	if (data == NULL)
	{
		printf("====line:%d,%s\n", __LINE__, "request uri no param data.");
	}
	else
	{
		printf("====line:%d,get request param: data=[%s]\n", __LINE__, data);
	}
	printf("\n");

	//回响应
	struct evbuffer* retbuff = NULL;
	retbuff = evbuffer_new();
	if (retbuff == NULL)
	{
		printf("====line:%d,%s\n", __LINE__, "retbuff is null.");
		return;
	}
	evbuffer_add_printf(retbuff, "Receive get request,Thamks for the request!");
	evhttp_send_reply(req, HTTP_OK, "Client", retbuff);
	evbuffer_free(retbuff);
}

//处理post请求
void http_handler_testpost_msg(struct evhttp_request* req, void* arg)
{
	if (req == NULL)
	{
		printf("====line:%d,%s\n", __LINE__, "input param req is null.");
		return;
	}

	char buf[BUF_MAX] = { 0 };
	get_post_message(buf, req);//获取请求数据，一般是json格式的数据
	if (buf == NULL)
	{
		printf("====line:%d,%s\n", __LINE__, "get_post_message return null.");
		return;
	}
	else
	{
		//可以使用json库解析需要的数据
		printf("====line:%d,request data:%s", __LINE__, buf);
	}

	//回响应
	struct evbuffer* retbuff = NULL;
	retbuff = evbuffer_new();
	if (retbuff == NULL)
	{
		printf("====line:%d,%s\n", __LINE__, "retbuff is null.");
		return;
	}
	evbuffer_add_printf(retbuff, "Receive post request,Thamks for the request!");
	evhttp_send_reply(req, HTTP_OK, "Client", retbuff);
	evbuffer_free(retbuff);
}

int main()
{
	struct evhttp* http_server = NULL;
	short http_port = 8081;
	char http_addr[] = ("0.0.0.0");

	//WSAStartup
	WSADATA wsaData;
	WORD wVersionRequested = MAKEWORD(2, 2);
	int err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) 
	{
		/* Tell the user that we could not find a usable */
		/* Winsock DLL.                                  */
		printf("WSAStartup failed with error: %d\n", err);
		goto exe_exit;
	}

	//初始化
	event_init();
	//启动http服务端
	http_server = evhttp_start(http_addr, http_port);
	if (http_server == NULL)
	{
		printf("====line:%d,%s\n", __LINE__, "http server start failed.");
		goto exe_exit;
	}

	//设置请求超时时间(s)
	evhttp_set_timeout(http_server, 5);
	//设置事件处理函数，evhttp_set_cb针对每一个事件(请求)注册一个处理函数，
	//区别于evhttp_set_gencb函数，是对所有请求设置一个统一的处理函数
	evhttp_set_cb(http_server, "/me/testpost", http_handler_testpost_msg, NULL);
	evhttp_set_cb(http_server, "/me/testget", http_handler_testget_msg, NULL);

	//循环监听
	event_dispatch();

	//实际上不会释放，代码不会运行到这一步
	evhttp_free(http_server);

exe_exit:
	system("pause");

	return 0;
}

#else

#include "httpConcurrencyServer.h"

//回调函数 -- 注意：是在线程里回调的，注意线程安全
//HTTP命令支持跨域访问
bool checkOptionsRequest(struct evhttp_request* req)
{
	try
	{
		if (req->type == EVHTTP_REQ_OPTIONS) //为支持跨域访问，需要对OPTIONS进行快速处理
		{
			evkeyvalq* header = evhttp_request_get_output_headers(req);
			if (header)
			{
				evhttp_add_header(header, "Access-Control-Allow-Origin", "*");
				evhttp_add_header(header, "Access-Control-Allow-Methods", "GET,PUT,POST,HEAD,OPTIONS,DELETE,PATCH");
				evhttp_add_header(header, "Access-Control-Allow-Headers", "Content-Type,Content-Length,Authorization,Accept,X-Requested-With");
				evhttp_add_header(header, "Access-Control-Max-Age", "3600");
			}
			evhttp_send_reply(req, HTTP_OK, nullptr, nullptr);
			return true;
		}
	}
	catch (...)
	{
		;
	}
	return false;
}

//HTTP服务回调函数
void global_http_generic_handler(struct evhttp_request* req, void* arg)
{
	if (checkOptionsRequest(req)) return;
	httpServer::httpThread* pServer = reinterpret_cast<httpServer::httpThread*>(arg);
	if (pServer == nullptr) return;
	int http_code = HTTP_OK;
	std::tstring pathStr, bodyLogStr, resultLogStr;
	std::string resultStr, activeBodyStr, activeResultStr;
	try {
		char pathchar[MAX_PATH];
		char querychar[MAX_PATH];
		char hostchar[MAX_PATH];
		char urichar[MAX_PATH];
		memset(pathchar, 0, sizeof(char) * MAX_PATH);
		memset(querychar, 0, sizeof(char) * MAX_PATH);
		memset(hostchar, 0, sizeof(char) * MAX_PATH);
		memset(urichar, 0, sizeof(char) * MAX_PATH);

		const char* temppathchar = evhttp_uri_get_path(req->uri_elems);
		if (temppathchar) COMMON_STRCPY(pathchar, temppathchar, static_cast<ULONG>(MAX_PATH - 1));
		const char* tempquerychar = evhttp_uri_get_query(req->uri_elems);
		if (tempquerychar) COMMON_STRCPY(querychar, tempquerychar, static_cast<ULONG>(MAX_PATH - 1));
		if (req->remote_host) COMMON_STRCPY(hostchar, req->remote_host, static_cast<ULONG>(MAX_PATH - 1));

		if (req->input_headers) {
			const char* tempurichar = evhttp_find_header(req->input_headers, "Host");
			if (tempurichar) COMMON_STRCPY(urichar, tempurichar, static_cast<ULONG>(MAX_PATH - 1));
		}

		struct evkeyvalq params;
		size_t querylen = 0U;
		char* querydecodechar = evhttp_uridecode(querychar, 0, &querylen);
		if (querydecodechar) {
			memset(querychar, 0, sizeof(char) * MAX_PATH);
			COMMON_STRCPY(querychar, querydecodechar, static_cast<ULONG>(MAX_PATH - 1));
			evhttp_parse_query_str(querydecodechar, &params);
			delete[]querydecodechar;
		}
		std::tstring queryStr, hostStr, uriStr;
#if defined(UNICODE) || defined(_UNICODE)
		std::string tempPathStr = pathchar;
		std::string tempHostStr = hostchar;
		std::string tempQueryStr = querychar;
		std::string tempUriStr = urichar;
		ansi_to_unicode(tempPathStr.c_str(), tempPathStr.length(), pathStr);
		ansi_to_unicode(tempQueryStr.c_str(), tempQueryStr.length(), queryStr);
		ansi_to_unicode(tempHostStr.c_str(), tempHostStr.length(), hostStr);
		ansi_to_unicode(tempUriStr.c_str(), tempUriStr.length(), uriStr);
#else
		pathStr = pathchar;
		queryStr = querychar;
		hostStr = hostchar;
		uriStr = urichar;

		pathStr = UrlDecode(pathStr);//ghq add
#endif
		bodyLogStr = _T("");
		char* post_data = reinterpret_cast<char*>(EVBUFFER_DATA(req->input_buffer));//(char *)EVBUFFER_DATA(req->input_buffer);//注：我们要求的是UTF-8封装的json格式
		size_t post_len = EVBUFFER_LENGTH(req->input_buffer);
		if (post_len > 0U) {
			std::string bodyStr;// = post_data;
			if (req->body_size > 0U) {// && bodyStr.length() >= req->body_size)
				char* tempchar = new char[req->body_size + 1U];
				memset(tempchar, 0, sizeof(char) * (req->body_size + 1U));
				//::CopyMemory(tempchar, post_data, req->body_size);
				memcpy(tempchar, post_data, req->body_size);
				bodyStr = tempchar;
				delete[]tempchar;
			}
#if defined(UNICODE) || defined(_UNICODE)
			utf8_to_unicode(bodyStr.c_str(), bodyStr.length(), bodyLogStr);
			unicode_to_ansi(bodyLogStr.c_str(), bodyLogStr.length(), activeBodyStr);
			_debug_to(_T("http server receive message from %s, path is %s, query param is %s, body is %s"), hostStr.c_str(), pathStr.c_str(), queryStr.c_str(), bodyLogStr.c_str());
#else
#ifdef WIN32
			utf8_to_ansi(bodyStr.c_str(), bodyStr.length(), activeBodyStr);
#else
			activeBodyStr = bodyStr;
#endif
			_debug_to(_T("[%d]http server receive message from %s, path is %s, query param is %s, body is %s"), messageIndex, hostStr.c_str(), pathStr.c_str(), queryStr.c_str(), activeBodyStr.c_str());
#endif
		}

		json::Value json_root;
		size_t tempPos;
		int overtime;
		std::vector<std::tstring> queryVector;
		globalSpliteString(queryStr, queryVector, _T("&"));
		std::map<std::tstring, std::tstring> queryMap;
		std::tstring tempParamName, tempParamValue, messageId, paramIdStr;
		for (std::vector<std::tstring>::iterator query_it = queryVector.begin(); query_it != queryVector.end(); query_it++)
		{
			tempPos = query_it->find(_T("="));
			if (tempPos == std::tstring::npos || tempPos == 0U) continue;
			tempParamName = query_it->substr(0U, tempPos);
			tempParamValue = query_it->substr(tempPos + 1U, query_it->length() - tempPos - 1U);
			if (tempParamName == _T("message")) messageId = tempParamValue;
			else if (tempParamName == _T("overtime")) globalStrToIntDef(const_cast<LPTSTR>(tempParamValue.c_str()), overtime,3,10);
			queryMap[tempParamName] = tempParamValue;
		}

		std::vector<std::tstring> pathVector;
		globalSpliteString(pathStr, pathVector, _T("/"));
		std::vector<std::tstring>::iterator path_it = pathVector.begin();

		std::tstring deal_id_name;

		if (req->type == EVHTTP_REQ_GET) {
			//自己处理
			resultLogStr = _T("GET request...");
		}
		else if (req->type == EVHTTP_REQ_POST) {
			//自己处理 
			resultLogStr = _T("POST request...");
		}
		else {
			resultLogStr = _T("OTHER request...");


			//json_root = json::objectValue;
			//if (!messageId.empty()) json_root["message"] = messageId.c_str();
			//json_root["code"] = 10003;
			//json_root["error"] = "not allow method";
			////http_code = HTTP_BADREQUEST;
		}

		//这里一般我会阻塞等待外部处理

#ifdef WIN32//这里进行了修改，只有WINDOWS下需要转化，linux下直接认为是UTF8编码
		ansi_to_utf8(resultStr.c_str(), resultStr.length(), activeResultStr);
#else
		activeResultStr = resultStr;
#endif

		//回应
		try {
			struct evbuffer* buf = evbuffer_new();
			if (!buf) {
				_debug_to(_T("http server response message fail for malloc buffer fail"));
				return;
			}
			evbuffer_add_printf(buf, "%s\n", activeResultStr.c_str());

			evkeyvalq* header = evhttp_request_get_output_headers(req);
			if (header) {
				evhttp_add_header(header, "Access-Control-Allow-Origin", "*");
				evhttp_add_header(header, "Access-Control-Allow-Methods", "GET,PUT,POST,HEAD,OPTIONS,DELETE,PATCH");
				evhttp_add_header(header, "Access-Control-Allow-Headers", "Content-Type,Content-Length,Authorization,Accept,X-Requested-With");
				evhttp_add_header(header, "Access-Control-Max-Age", "3600");
				evhttp_add_header(header, "Content-Type", "application/json; charset=UTF-8");
			}
			evhttp_send_reply(req, http_code, "proxy", buf);
			evbuffer_free(buf);

#if defined(UNICODE) || defined(_UNICODE)
			ansi_to_unicode(resultStr.c_str(), resultStr.length(), resultLogStr);
			_debug_to(_T("http server response is %s"), resultLogStr.c_str());
#else
			_debug_to(_T("http server response is %s"), resultStr.c_str());
#endif
		}
		catch (...) {
			_debug_to(_T("http server deal throw exception"));
		}
	}
	catch (...) {

	}
}
int main()
{
	//初始化
	httpServer::complex_httpServer server;
	int httpPort = 8081;//监听端口
	int threadCount = 10;//开启线程池个数
	int ret = server.start_http_server(global_http_generic_handler, nullptr, httpPort, threadCount, 10240);//开启监听
	if (ret < 0) {
		_debug_to(_T("启动端口为%d的HTTP监听失败"), httpPort);
	}
	else {
		_debug_to(_T("启动端口为%d的HTTP监听成功"), httpPort);
	}

	//结束监听
	//if (ret >= 0) {
	//	server.stop_http_server();
	//}
	system("pause");
}


#endif
