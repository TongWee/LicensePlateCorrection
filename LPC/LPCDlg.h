
// LPCDlg.h : ͷ�ļ�
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
// CLPCDlg �Ի���
class CLPCDlg : public CDialogEx
{
// ����
public:
	CLPCDlg(CWnd* pParent = NULL);	// ��׼���캯��
	Mat PIC_SOURCE;


// �Ի�������
	enum { IDD = IDD_LPC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��


// ʵ��
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
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
