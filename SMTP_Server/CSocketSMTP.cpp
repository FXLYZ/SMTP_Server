#include "pch.h"
#include "CSocketSMTP.h"
#include "SMTP_ServerDlg.h"
#include "base.h"


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
	pDlg->ServerLog.SetWindowTextW(temp + L"监听到连接请求\n");
	targetsocket = new CSocketSMTP();
	if (Accept(*targetsocket))
	{
		char* buf = "220 SMTP is ready\n";
		targetsocket->Send(buf, strlen(buf), 0);
		targetsocket->AsyncSelect(FD_READ);
		pDlg->ServerLog.GetWindowTextW(temp);
		pDlg->ServerLog.SetWindowTextW(temp + L"S: 回复连接请求成功\n");
		pDlg->ServerLog.GetWindowTextW(temp);
		pDlg->ServerLog.SetWindowTextW(temp + L"S:"+buf);
	}
	else
	{
		pDlg->ServerLog.GetWindowTextW(temp);
		pDlg->ServerLog.SetWindowTextW(temp + L"S: 回复连接请求失败\n");
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
		pDlg->ServerLog.SetWindowTextW(temp + L"C:"+receive.Left(length)+L"\n");
	}
	else
	{
		if (DataFinish)
		{
			pDlg->ServerLog.GetWindowTextW(temp);
			pDlg->ServerLog.SetWindowTextW(temp + L"C:" + receive.Left(4)+L"\n");
		}
	}
	if (length!=0)
	{
		if (IsLog)
		{
			char* buf;
			if (receive.Left(4)=="EHLO")
			{
				buf = "250 OK 127.0.0.1\n";
				Send(buf, strlen(buf), 0);
				AsyncSelect(FD_READ);
				pDlg->ServerLog.GetWindowTextW(temp);
				pDlg->ServerLog.SetWindowTextW(temp + L"C:"+buf);
				return;
			}
			else if (receive.Left(10) == "MAIL FROM:")
			{
				buf = "250 Sender OK\n";

				Send(buf, strlen(buf), 0);
				AsyncSelect(FD_READ);
				pDlg->ServerLog.GetWindowTextW(temp);
				pDlg->ServerLog.SetWindowTextW(temp + L"C:" + buf);
				return;
			}
			else if (receive.Left(8) == "RCPT TO:")
			{
				buf = "250 Receiver OK\n";
				Send(buf, strlen(buf), 0);
				AsyncSelect(FD_READ);
				pDlg->ServerLog.GetWindowTextW(temp);
				pDlg->ServerLog.SetWindowTextW(temp + L"C:" + buf);
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
				pDlg->ServerLog.SetWindowTextW(temp + L"C:" + buf);
				return;
			}
			else if (receive.Left(4) == "QUIT" || receive.Left(4) == "RSET")
			{
				buf = "221 Quit, Goodbye !\n";
				Send(send, strlen(buf), 0);
				pDlg->ServerLog.GetWindowTextW(temp);
				pDlg->ServerLog.SetWindowTextW(temp + L"C:" + buf);
				Close();
				return;
			}
			else
			{
				buf = "500 order is wrong\n";
				Send(send, strlen(buf), 0);
				pDlg->ServerLog.GetWindowTextW(temp);
				pDlg->ServerLog.SetWindowTextW(temp + L"C:" + buf);
				Close();
				return;
			}
		}
		else
		{
			CSMTPServerDlg* pDlg = (CSMTPServerDlg*)AfxGetMainWnd();
			CString temp;
			pDlg->GetWindowTextW(temp);
			pDlg->SetWindowTextW(temp + receive.Left(length));
			if (receive.Find(_T("\r\n.\r\n")) != -1)
			{
				DataFinish = true;
				IsLog =true;
				char* buf= "250 Message accepted for delivery\r\n";
				Send(buf, strlen(buf), 0);
				pDlg->GetWindowTextW(temp);
				pDlg->SetWindowTextW(temp + buf);
				pDlg->GetWindowTextW(pic);
				if (pic.Find(_T("Content-Type: image/bmp")) != -1)
				{
					//截取bmp图片的base64编码
					int Attachment_Start = pic.Find(_T("Content-Disposition: attachment"), 0);
					int Bmp_Start = pic.Find(_T("\r\n\r\n"), Attachment_Start);
					CString Start = pic.Mid(Bmp_Start + 4, pic.GetLength() - Bmp_Start - 4);
					int length = Start.Find(_T("\r\n\r\n"), 0);
					pic = Start.Left(length);
					HBITMAP picture;
					//解码
					DeCode(pic, picture);
					//输入到对话框
					//pDlg->m_bmp.SetBitmap(picture);
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
