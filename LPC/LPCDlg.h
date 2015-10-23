
// LPCDlg.h : 头文件
//

#pragma once
#include "afxwin.h"
#include "afxcmn.h"
#include "CvvImage.h"
#include "BR.h"
#include "c:\opencv\build\include\opencv2\imgcodecs\imgcodecs_c.h" 
#include "c:\opencv\build\include\opencv2\imgproc\types_c.h"
using namespace cv;
using namespace br;
using namespace std;
// CLPCDlg 对话框
class CLPCDlg : public CDialogEx
{
// 构造
public:
	CLPCDlg(CWnd* pParent = NULL);	// 标准构造函数
	Mat PIC_SOURCE;


// 对话框数据
	enum { IDD = IDD_LPC_DIALOG };

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
	afx_msg void OnBnClickedOpen();
	afx_msg void OnBnClickedCorrect();
	afx_msg void OnBnClickedSave();
	CListBox m_listbox;
	CListCtrl m_listctrl;
//	afx_msg void OnHdnItemclickListCtrl(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnNMClickListCtrl(NMHDR *pNMHDR, LRESULT *pResult);
	CStatic m_pic_src;
	CStatic m_pic_dst;

	void MFC_ShowImage(Mat& src, UINT ID);


};
