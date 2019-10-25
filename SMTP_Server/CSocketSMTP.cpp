#include "pch.h"
#include "CSocketSMTP.h"
#include "SMTP_ServerDlg.h"
#include "base.h"
#include<iostream>

CSocketSMTP::CSocketSMTP()
{
	status = L"等待连接";
	targetsocket = NULL;
	IsLog = true;
	DataFinish = true;
	IsBmp = false;
	pic = L"";
	Quit=false;
	
}


CSocketSMTP::~CSocketSMTP()
{
}


void CSocketSMTP::OnAccept(int nErrorCode)
{
	CSMTPServerDlg* pDlg=(CSMTPServerDlg*)AfxGetMainWnd();
	CString temp;
	pDlg->ServerLog.GetWindowTextW(temp);
	pDlg->ServerLog.SetWindowTextW(temp + L"监听到连接请求\r\n");
	targetsocket = new CSocketSMTP();
	if (Accept(*targetsocket))
	{
		char* buf = "220 SMTP is ready\r\n";
		targetsocket->Send(buf, strlen(buf), 0);
		targetsocket->AsyncSelect(FD_READ);
		pDlg->ServerLog.GetWindowTextW(temp);
		pDlg->ServerLog.SetWindowTextW(temp + L"S: 回复连接请求成功\r\n");
		pDlg->ServerLog.GetWindowTextW(temp);
		pDlg->ServerLog.SetWindowTextW(temp + L"S:"+buf);
	}
	else
	{
		pDlg->ServerLog.GetWindowTextW(temp);
		pDlg->ServerLog.SetWindowTextW(temp + L"S: 回复连接请求失败\r\n");
	}
    CAsyncSocket::OnAccept(nErrorCode);
}


void CSocketSMTP::OnReceive(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类
	int length = Receive(data, sizeof(data), 0);
	CSMTPServerDlg* pDlg = (CSMTPServerDlg*)AfxGetMainWnd();
	CString temp;
	CString receive(data);
	if (IsLog)
	{
		pDlg->ServerLog.GetWindowTextW(temp);
		pDlg->ServerLog.SetWindowTextW(temp + L"C: "+receive.Left(length)+L"\r\n");
	}
	if (length!=0)
	{
		if (IsLog)
		{
			char* buf;
			if (receive.Left(4)=="EHLO")
			{
				buf = "250 OK 127.0.0.1\r\n";
				Send(buf, strlen(buf), 0);
				AsyncSelect(FD_READ);
				pDlg->ServerLog.GetWindowTextW(temp);
				pDlg->ServerLog.SetWindowTextW(temp + L"S: "+buf);
				return;
			}
			else if (receive.Left(10) == "MAIL FROM:")
			{
				buf = "250 Sender OK\r\n";

				Send(buf, strlen(buf), 0);
				AsyncSelect(FD_READ);
				pDlg->ServerLog.GetWindowTextW(temp);
				pDlg->ServerLog.SetWindowTextW(temp + L"S: " + buf);
				return;
			}
			else if (receive.Left(8) == "RCPT TO:")
			{
				buf = "250 Receiver OK\r\n";
				Send(buf, strlen(buf), 0);
				AsyncSelect(FD_READ);
				pDlg->ServerLog.GetWindowTextW(temp);
				pDlg->ServerLog.SetWindowTextW(temp + L"S: " + buf);
				return;
			}
			else if (receive.Left(4) == "DATA")
			{
				DataFinish = false;
				IsLog = false;
				buf = "354 Go ahead. End with <CRLF>.<CRLF>\r\n";
				Send(buf, strlen(buf), 0);
				AsyncSelect(FD_READ);
				pDlg->ServerLog.GetWindowTextW(temp);
				pDlg->ServerLog.SetWindowTextW(temp + L"S: " + buf);
				return;
			}
			else if (receive.Left(4) == "QUIT" || receive.Left(4) == "RSET")
			{
				buf = "221 Quit, Goodbye !\r\n";
				Send(send, strlen(buf), 0);
				pDlg->ServerLog.GetWindowTextW(temp);
				pDlg->ServerLog.SetWindowTextW(temp + L"S: " + buf);
				Close();
				return;
			}
			else
			{
				buf = "500 order is wrong\r\n";
				Send(send, strlen(buf), 0);
				pDlg->ServerLog.GetWindowTextW(temp);
				pDlg->ServerLog.SetWindowTextW(temp + L"S:" + buf);
				Close();
				return;
			}
		}
		else
		{
			CSMTPServerDlg* pDlg = (CSMTPServerDlg*)AfxGetMainWnd();
			CString temp;
			pDlg->MailContent.GetWindowTextW(temp);
			pDlg->MailContent.SetWindowTextW(temp + receive.Left(length));
			if (receive.Find(_T("\r\n.\r\n")) != -1)
			{
				DataFinish = true;
				IsLog =true;
				char* buf= "250 Message accepted for delivery\r\n";
				Send(buf, strlen(buf), 0);
				pDlg->ServerLog.GetWindowTextW(temp);
				pDlg->ServerLog.SetWindowTextW(temp + L"S: "+buf);
				pDlg->MailContent.GetWindowTextW(pic);
				int hasBmp = pic.Find(_T("Content-Type: image/bmp"));
				int hasPng = pic.Find(_T("Content-Type: image/png"));
				int begin=-1;
				CString type;
				if (hasBmp!=-1)
				{
					begin = hasBmp;
					type = L"bmp";
				}
				else if (hasPng!=-1)
				{
					begin = hasPng;
					type = L"png";
				}
				if (begin!=-1)
				{
					int Bmp_Start = pic.Find(_T("\r\n\r\n"), begin);
					int Bmp_End= pic.Find(_T("\r\n\r\n"), Bmp_Start +4);
					pic= pic.Mid(Bmp_Start + 4, Bmp_End - Bmp_Start - 4);
					HBITMAP picture;
					DeCode(pic, picture,type);
					pDlg->picture.SetBitmap(picture);
				}
			}
			AsyncSelect(FD_READ);
			return;
		}
	}
	CAsyncSocket::OnReceive(nErrorCode);
}


void CSocketSMTP::OnClose(int nErrorCode)
{
	// TODO: 在此添加专用代码和/或调用基类

	CAsyncSocket::OnClose(nErrorCode);
}
