#include <afxinet.h>
#include <WinInet.h>
#include <string>
using namespace std;

class _autoRelease
{
public:
	_autoRelease(char* pdel) :pbuffer(pdel)
	{
		pwbuffer = NULL;
	}
	_autoRelease(WCHAR* pdel) :pwbuffer(pdel)
	{
		pbuffer = NULL;
	}
	~_autoRelease()
	{
		if (pbuffer)
		{
			delete[]pbuffer;
			pbuffer = NULL;
		}
		if (pwbuffer)
		{
			delete[]pwbuffer;
			pwbuffer = NULL;
		}
	}
protected:
	char* pbuffer;
	WCHAR* pwbuffer;
};
#define  AUTODEL(pbuff,name)  _autoRelease (name)(pbuff)

// unicode编码的CString->UTF8的char*,可选URLEncode
// 发起请求时url带汉字可使用
#include <string>

#define CSTRING_TO_UTF8STRING(unicodeStr, encode) \
	([&]() -> std::string { \
	char* pUTF8Buff = nullptr; \
	MFCHttp::UnicodeToUTF8((LPCWSTR)(unicodeStr), &pUTF8Buff); \
	AUTODEL(pUTF8Buff, uni1); \
	if (encode) { \
	char* pEncodedUTF8Buff = nullptr; \
	MFCHttp::URLEncode(pUTF8Buff, &pEncodedUTF8Buff); \
	AUTODEL(pEncodedUTF8Buff, uni2); \
	return pEncodedUTF8Buff ? std::string(pEncodedUTF8Buff) : std::string(); \
	} \
	return pUTF8Buff ? std::string(pUTF8Buff) : std::string(); \
}())

// UTF8的char*-->unicode编码的CString
// 请求返回中有汉字可使用
#define UTF8STRING_TO_CSTRING(utf8_string) \
	([&]() -> CString { \
	WCHAR* pUniBuff = NULL; \
	MFCHttp::UTF8ToUnicode((char*)utf8_string.c_str(), &pUniBuff); \
	AUTODEL(pUniBuff, unitmp); \
	return pUniBuff ? CString(pUniBuff) : CString(); \
}())

static CString toCString(std::string s_str)
{
	CString str; 
	str = s_str.c_str();//先创建对象，再使用=赋值

	return str;
}
static CString toCString(char* pBuff)
{
	CString str; 
	str = pBuff;//先创建对象，再使用=赋值

	return str;
}
static char* toCharBuff(CString str)
{
	USES_CONVERSION;
	char* pTemp = T2A(str);
	int nLen = strlen(pTemp);
	char* pBuff = new char[nLen+1];
	memset(pBuff,0,nLen+1);
	memcpy(pBuff,pTemp,nLen);

	return pBuff;
}
static std::string toStdString(CString str)
{
	std::string s_str = (CT2A)str;
	return s_str;
}



namespace MFCHttp
{
	//
	void UTF8ToUnicode(char* pUTF8Src, WCHAR** ppUnicodeDst);
	void UnicodeToUTF8(const WCHAR* pUnicodeSrc, char** ppUTF8Dst);
	void URLEncode(char* szIn, char** pOut);

	//
	BOOL Http_POST(CString strFullURLPath, CString strHeaders, CString strBody, std::string& sReceive);
	BOOL Http_GET(CString strFullURLPath, CString strFullFilePath);
	BOOL Http_PUT(LPCTSTR pHomeUrl, LPCTSTR pPageUrl, LONG nPort, LPCTSTR pFile, CString* psRes, PBOOL pbExit);
};


