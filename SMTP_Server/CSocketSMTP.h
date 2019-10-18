#pragma once
#include <afxsock.h>
class CSocketSMTP :
	public CAsyncSocket
{
public:
	CSocketSMTP();
	~CSocketSMTP();
	virtual void OnAccept(int nErrorCode);
};

