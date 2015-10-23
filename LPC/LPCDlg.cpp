
// LPCDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "LPC.h"
#include "LPCDlg.h"
#include "afxdialogex.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLPCDlg �Ի���



CLPCDlg::CLPCDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CLPCDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CLPCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CTRL, m_listctrl);
	DDX_Control(pDX, PIC_SRC, m_pic_src);
	DDX_Control(pDX, PIC_DST, m_pic_dst);
}

BEGIN_MESSAGE_MAP(CLPCDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OPEN, &CLPCDlg::OnBnClickedOpen)
	ON_BN_CLICKED(IDC_CORRECT, &CLPCDlg::OnBnClickedCorrect)
	ON_BN_CLICKED(IDC_SAVE, &CLPCDlg::OnBnClickedSave)
//	ON_NOTIFY(HDN_ITEMCLICK, 0, &CLPCDlg::OnHdnItemclickListCtrl)
ON_NOTIFY(NM_CLICK, IDC_LIST_CTRL, &CLPCDlg::OnNMClickListCtrl)
END_MESSAGE_MAP()


// CLPCDlg ��Ϣ�������

BOOL CLPCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������	
	m_listctrl.InsertColumn(0, "�ļ���", LVCFMT_LEFT, 90);//
	m_listctrl.InsertColumn(2, "λ��", LVCFMT_LEFT, 150);
	m_listctrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);



	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CLPCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CLPCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CLPCDlg::OnBnClickedOpen()
{	
	CString filePath;
	static TCHAR strDirName[MAX_PATH];
	BROWSEINFO bi;
	CString szString = TEXT("ѡ��һ���ļ���");
	bi.hwndOwner = ::GetFocus();
	bi.pidlRoot = NULL;
	bi.pszDisplayName = strDirName;
	bi.lpszTitle = szString;
	bi.ulFlags = BIF_BROWSEFORCOMPUTER | BIF_DONTGOBELOWDOMAIN | BIF_RETURNONLYFSDIRS;
	bi.lpfn = NULL;
	bi.lParam = 0;
	bi.iImage = 0;
	LPITEMIDLIST pItemIDList = ::SHBrowseForFolder(&bi);
	if (pItemIDList == NULL)
	{
		return;
	}
	::SHGetPathFromIDList(pItemIDList, strDirName);
	filePath = strDirName;
	if (!filePath.IsEmpty())//�Ƿ�򿪳ɹ�
	{
		m_listctrl.DeleteAllItems();
		WIN32_FIND_DATA findfiledata;
		HANDLE hFind;
		hFind = FindFirstFile(filePath+"\\*.jpg", &findfiledata);
		if (hFind == INVALID_HANDLE_VALUE)
		{
			MessageBox(_T("No files found."), NULL, MB_OK);
		}
		int index = 0;
		CString filepath;
		CString filefindname;
		do
		{
			filefindname = findfiledata.cFileName;
			int isFolder=filefindname.Find(".");
			if (isFolder<=1)
				continue;
			filepath = filePath + "\\" + filefindname;
			index = m_listctrl.InsertItem(0, findfiledata.cFileName);
			m_listctrl.SetItemText(index, 1, filepath);
		} while (FindNextFile(hFind, &findfiledata));
	}
	else//ȡ����
	{
		//ȡ����
		MessageBox(_T("Fail to open folder."),NULL,MB_OK);
	}	
}


void CLPCDlg::OnBnClickedCorrect()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	if (PIC_SOURCE.empty())
		return;
	Mat src,dst;
	src = PIC_SOURCE;
	BR br(Size(src.cols, src.rows));
	br.BR_Correction(PIC_SOURCE, dst,1);
	MFC_ShowImage(dst, PIC_DST);
}


void CLPCDlg::OnBnClickedSave()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
}


void CLPCDlg::OnNMClickListCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	POSITION pos = m_listctrl.GetFirstSelectedItemPosition();
	if (pos == NULL) return;
	int item = m_listctrl.GetNextSelectedItem(pos);
	//Bitmap bitmap = 
	//m_pic_src.SetBitmap()
	String filePath = m_listctrl.GetItemText(item, 1);
	Mat bitmap = imread(filePath);
	MFC_ShowImage(bitmap, PIC_SRC);
	PIC_SOURCE = bitmap.clone();
	*pResult = 0;

}
/*--------------------
���ܣ���ָ���ؼ���ʾͼƬ
������src des �ؼ�ID
���أ�void
--------------------*/
void CLPCDlg::MFC_ShowImage(Mat& src, UINT ID)
{
	if (src.empty())
	{
		return;
	}
	CDC* pDC = GetDlgItem(ID)->GetDC();		// �����ʾ�ؼ��� DC
	HDC hDC = pDC->GetSafeHdc();				// ��ȡ HDC(�豸���) �����л�ͼ����
	CRect rect;
	GetDlgItem(ID)->GetClientRect(&rect);	// ��ȡ�ؼ��ߴ�λ��
	CvvImage cimg;
	IplImage cpy = src;
	cimg.CopyOf(&cpy);						// ����ͼƬ
	cimg.DrawToHDC(hDC, &rect);				// ��ͼƬ���Ƶ���ʾ�ؼ���ָ��������
	ReleaseDC(pDC);
}