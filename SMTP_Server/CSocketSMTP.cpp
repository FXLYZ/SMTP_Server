#include "pch.h"
#include "CSocketSMTP.h"


CSocketSMTP::CSocketSMTP()
{
}


CSocketSMTP::~CSocketSMTP()
{
}


void CSocketSMTP::OnAccept(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类

	CAsyncSocket::OnAccept(nErrorCode);
}
