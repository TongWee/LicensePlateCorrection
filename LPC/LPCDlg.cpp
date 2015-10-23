
// LPCDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "LPC.h"
#include "LPCDlg.h"
#include "afxdialogex.h"
#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CLPCDlg 对话框



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


// CLPCDlg 消息处理程序

BOOL CLPCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码	
	m_listctrl.InsertColumn(0, "文件名", LVCFMT_LEFT, 90);//
	m_listctrl.InsertColumn(2, "位置", LVCFMT_LEFT, 150);
	m_listctrl.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);



	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CLPCDlg::OnPaint()
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
HCURSOR CLPCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CLPCDlg::OnBnClickedOpen()
{	
	CString filePath;
	static TCHAR strDirName[MAX_PATH];
	BROWSEINFO bi;
	CString szString = TEXT("选择一个文件夹");
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
	if (!filePath.IsEmpty())//是否打开成功
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
	else//取消打开
	{
		//取消打开
		MessageBox(_T("Fail to open folder."),NULL,MB_OK);
	}	
}


void CLPCDlg::OnBnClickedCorrect()
{
	// TODO:  在此添加控件通知处理程序代码
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
	// TODO:  在此添加控件通知处理程序代码
}


void CLPCDlg::OnNMClickListCtrl(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO:  在此添加控件通知处理程序代码
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
功能：在指定控件显示图片
参数：src des 控件ID
返回：void
--------------------*/
void CLPCDlg::MFC_ShowImage(Mat& src, UINT ID)
{
	if (src.empty())
	{
		return;
	}
	CDC* pDC = GetDlgItem(ID)->GetDC();		// 获得显示控件的 DC
	HDC hDC = pDC->GetSafeHdc();				// 获取 HDC(设备句柄) 来进行绘图操作
	CRect rect;
	GetDlgItem(ID)->GetClientRect(&rect);	// 获取控件尺寸位置
	CvvImage cimg;
	IplImage cpy = src;
	cimg.CopyOf(&cpy);						// 复制图片
	cimg.DrawToHDC(hDC, &rect);				// 将图片绘制到显示控件的指定区域内
	ReleaseDC(pDC);
}