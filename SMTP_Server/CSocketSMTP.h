#pragma once
#include <afxsock.h>
class CSocketSMTP :
	public CAsyncSocket
{
public:
	CSocketSMTP();
	~CSocketSMTP();
	CAsyncSocket* targetsocket;
	CString status;
	bool IsLog;
	bool DataFinish;
	bool Quit;
	bool IsBmp;
	CString pic;
	char data[4096];
	virtual void OnAccept(int nErrorCode);
	virtual void OnReceive(int nErrorCode);
	virtual void OnClose(int nErrorCode);
};

