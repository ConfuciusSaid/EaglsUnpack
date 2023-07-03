
// EaglsUnpackDlg.h: 头文件
//

#pragma once


// CEaglsUnpackDlg 对话框
class CEaglsUnpackDlg : public CDialogEx
{
// 构造
public:
	CEaglsUnpackDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_EAGLSUNPACK_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	CEdit m_editIdxPath;
	CEdit m_editPakPath;
	CEdit m_editSavePath;
	CEdit m_editSaveName;
	afx_msg void OnDropFiles(HDROP hDropInfo);
	afx_msg void OnBnClickedBtnSavepath();
	CEdit m_editUnpackPath;
	afx_msg void OnBnClickedBtnUnpack();
	afx_msg void OnBnClickedBtnPack();
};
