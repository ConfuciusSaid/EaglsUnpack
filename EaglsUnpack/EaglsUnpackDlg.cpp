
// EaglsUnpackDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "EaglsUnpack.h"
#include "EaglsUnpackDlg.h"
#include "afxdialogex.h"
#include "EAGLS_ANALYSE.h"
#include <time.h>
#include <string>

using std::string;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CEaglsUnpackDlg 对话框



CEaglsUnpackDlg::CEaglsUnpackDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_EAGLSUNPACK_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CEaglsUnpackDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_IDX, m_editIdxPath);
	DDX_Control(pDX, IDC_EDIT_PAK, m_editPakPath);
	DDX_Control(pDX, IDC_EDIT_SAVEPATH, m_editSavePath);
	DDX_Control(pDX, IDC_EDIT_SAVENAME, m_editSaveName);
	DDX_Control(pDX, IDC_EDIT_UNPACKPATH, m_editUnpackPath);
}

BEGIN_MESSAGE_MAP(CEaglsUnpackDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_DROPFILES()
	ON_BN_CLICKED(IDC_BTN_UNPACK, &CEaglsUnpackDlg::OnBnClickedBtnUnpack)
	ON_BN_CLICKED(IDC_BTN_PACK, &CEaglsUnpackDlg::OnBnClickedBtnPack)
END_MESSAGE_MAP()


// CEaglsUnpackDlg 消息处理程序

BOOL CEaglsUnpackDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	ChangeWindowMessageFilter(WM_DROPFILES, MSGFLT_ADD);
	ChangeWindowMessageFilter(0x0049, MSGFLT_ADD);       // 0x0049 == WM_COPYGLOBALDATA

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CEaglsUnpackDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CEaglsUnpackDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CEaglsUnpackDlg::OnDropFiles(HDROP hDropInfo)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	LPSTR pBuf = new char[MAX_PATH];
	string str;
	BOOL bRight = FALSE;

	DragQueryFile(hDropInfo, 0, pBuf, sizeof(char) * MAX_PATH);
	str = pBuf;
	if (str.substr(str.size() - 4, 4) == ".pak") {
		m_editPakPath.SetWindowTextA(str.c_str());
		str = str.substr(0, str.size() - 4);
		str += ".idx";
		m_editIdxPath.SetWindowTextA(str.c_str());
		bRight = TRUE;
	}else if (str.substr(str.size() - 4, 4) == ".idx") {
		m_editIdxPath.SetWindowTextA(str.c_str());
		str = str.substr(0, str.size() - 4);
		str += ".pak";
		m_editPakPath.SetWindowTextA(str.c_str());
		bRight = TRUE;
	}

	if (bRight) {
		for (int i = str.size(); i >= 0; i--) {
			if (str[i] == '\\') {
				m_editSaveName.SetWindowTextA((((string)&str.c_str()[i + 1]).substr(0, str.size() - i - 5) + "_new").c_str());
				m_editSavePath.SetWindowTextA(str.substr(0, i).c_str());
				break;
			}
		}
		m_editUnpackPath.SetWindowTextA(str.substr(0, str.size() - 4).c_str());
		
	}

	delete[]pBuf;
	CDialogEx::OnDropFiles(hDropInfo);
}

void CEaglsUnpackDlg::OnBnClickedBtnUnpack()
{
	// TODO: 在此添加控件通知处理程序代码
	CEaglsAnalyse ea;
	char IdxFilePath[MAX_PATH];
	char PakFilePath[MAX_PATH];

	m_editIdxPath.GetWindowTextA(IdxFilePath, MAX_PATH);
	m_editPakPath.GetWindowTextA(PakFilePath, MAX_PATH);

	if (ea.OpenScpack(IdxFilePath, PakFilePath)) {
		ea.Encrypt();
	}
}


void CEaglsUnpackDlg::OnBnClickedBtnPack()
{
	// TODO: 在此添加控件通知处理程序代码
	CEaglsAnalyse ea;
	char UnpackDirectory[MAX_PATH];
	char PackFilePath[MAX_PATH];
	char PackFileName[MAX_PATH];
	char IdxFilePath[MAX_PATH];
	DWORD dwSize;
	HANDLE hFile;

	m_editUnpackPath.GetWindowTextA(UnpackDirectory, MAX_PATH);
	m_editSavePath.GetWindowTextA(PackFilePath, MAX_PATH);
	m_editSaveName.GetWindowTextA(PackFileName, MAX_PATH);
	m_editIdxPath.GetWindowTextA(IdxFilePath, MAX_PATH);

	hFile = CreateFileA(IdxFilePath, GENERIC_ALL, 0, 0, OPEN_EXISTING, 0, 0);
	if (hFile == INVALID_HANDLE_VALUE) {
		MessageBox("无法打开.idx文件");
		return;
	}
	dwSize = GetFileSize(hFile, &dwSize);
	dwSize -= sizeof(DWORD);
	CloseHandle(hFile);

	if (ea.OpenUnpack(UnpackDirectory)) {
		ea.Pack(PackFilePath, PackFileName, dwSize, time(0));
	}
}
