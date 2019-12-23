﻿
// vbdcoderDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "vbdcoder.h"
#include "vbdcoderDlg.h"
#include "afxdialogex.h"

#include "InputFile.h"
#include<vector>
#include "CMyListCtrl.h"
#include<io.h>
#include<direct.h>
#include<windows.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 自定义常数
const int COLUMN = 9;
const int MAX_ITEM = 1 << 10;

// 自定义全局变量
std::vector<InputFile> listItems;
std::vector<InputFile> presets;
InputFile settingsNow(L"",0,0,0,0,0,0);

// 多线程操作
volatile BOOL threadRunning=false, toBeTerminated=false, terminateNow=false, suspendNow=false;
PROCESS_INFORMATION ffmpegProcess;

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
protected:
	DECLARE_MESSAGE_MAP()
public:
    virtual BOOL OnInitDialog();
    CString v_CR;
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
, v_CR(_T(""))
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Text(pDX, IDC_EDIT_CR, v_CR);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CvbdcoderDlg 对话框



CvbdcoderDlg::CvbdcoderDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_VBDCODER_DIALOG, pParent)
    , v_outputDir(_T(""))
    , v_format(FALSE)
    , v_code(FALSE)
    , v_videoBitrate(_T(""))
    , v_audioBitrate(_T(""))
    , v_copyAudioStream(FALSE)
    , v_start(COleDateTime::GetCurrentTime())
    , v_end(COleDateTime::GetCurrentTime())
    , v_cut(FALSE)
    , v_command(_T(""))
    , v_output(_T(""))
    , v_copyVideoStream(FALSE)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CvbdcoderDlg::~CvbdcoderDlg()
{
    if (threadRunning)
    {
        TerminateProcess(ffmpegProcess.hProcess, 127);
        TerminateProcess(cThread->m_hThread, 127);
    }
}

void CvbdcoderDlg::DoDataExchange(CDataExchange* pDX)
{
    CDialogEx::DoDataExchange(pDX);
    DDX_Control(pDX, IDC_LIST, m_list);
    DDX_Control(pDX, IDC_COMBO_PRESET, m_preset);
    DDX_Text(pDX, IDC_EDIT_OUTPUT_DIR, v_outputDir);
    DDX_Radio(pDX, IDC_RADIO_FORMAT, v_format);
    DDX_Radio(pDX, IDC_RADIO_CODE, v_code);
    DDX_Text(pDX, IDC_EDIT_VIDEO_BITRATE, v_videoBitrate);
    DDX_Text(pDX, IDC_EDIT_AUDIO_BITRATE, v_audioBitrate);
    DDX_Check(pDX, IDC_CHECK_COPY_AUDIO_STREAM, v_copyAudioStream);
    DDX_DateTimeCtrl(pDX, IDC_TIMEPICKER_START, v_start);
    DDX_DateTimeCtrl(pDX, IDC_TIMEPICKER_END, v_end);
    DDX_Check(pDX, IDC_CHECK_CUT, v_cut);
    DDX_Text(pDX, IDC_EDIT_COMMAND, v_command);
    DDX_Text(pDX, IDC_EDIT_OUTPUT, v_output);
    DDX_Check(pDX, IDC_CHECK_COPY_VIDEO_STREAM, v_copyVideoStream);
    DDX_Control(pDX, IDC_STATIC_CODE, m_static);
    DDX_Control(pDX, IDC_EDIT_OUTPUT, m_output);
    DDX_Control(pDX, IDC_EDIT_COMMAND, m_command);
    DDX_Control(pDX, IDC_BUTTON_PAUSE, m_pause);
}

BEGIN_MESSAGE_MAP(CvbdcoderDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
    ON_BN_CLICKED(IDC_CHECK_CUT, &CvbdcoderDlg::OnBnClickedCheckCut)
    ON_BN_CLICKED(IDC_CHECK_COPY_VIDEO_STREAM, &CvbdcoderDlg::OnBnClickedCheckCopyVideoStream)
    ON_BN_CLICKED(IDC_CHECK_COPY_AUDIO_STREAM, &CvbdcoderDlg::OnBnClickedCheckCopyAudioStream)
    ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_FORMAT, IDC_RADIO5, &CvbdcoderDlg::OnBnClickedRadioFormat)
    ON_CONTROL_RANGE(BN_CLICKED, IDC_RADIO_CODE, IDC_RADIO3, &CvbdcoderDlg::OnBnClickedRadioCode)
    ON_EN_CHANGE(IDC_EDIT_VIDEO_BITRATE, &CvbdcoderDlg::OnEnChangeEditVideoBitrate)
    ON_EN_CHANGE(IDC_EDIT_AUDIO_BITRATE, &CvbdcoderDlg::OnEnChangeEditAudioBitrate)
    ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST, &CvbdcoderDlg::OnLvnItemchangedList)
    ON_CBN_SELCHANGE(IDC_COMBO_PRESET, &CvbdcoderDlg::OnCbnSelchangeComboPreset)
    ON_NOTIFY_RANGE(DTN_DATETIMECHANGE, IDC_TIMEPICKER_START, IDC_TIMEPICKER_END, &CvbdcoderDlg::OnDtnDatetimechangeTimepicker)
    
    ON_COMMAND(ID_ABOUT, &CvbdcoderDlg::OnAbout)
    ON_BN_CLICKED(IDC_BUTTON_START, &CvbdcoderDlg::OnBnClickedButtonStart)
    ON_COMMAND(ID_RESET_STATUS, &CvbdcoderDlg::OnResetStatus)
    ON_BN_CLICKED(IDC_BUTTON_STOPAFTER, &CvbdcoderDlg::OnBnClickedButtonStopafter)
    ON_BN_CLICKED(IDC_BUTTON_STOPNOW, &CvbdcoderDlg::OnBnClickedButtonStopnow)
    ON_BN_CLICKED(IDC_BUTTON_PAUSE, &CvbdcoderDlg::OnBnClickedButtonPause)
    ON_COMMAND(ID_ACCELERATOR_DELETE, &CvbdcoderDlg::OnAcceleratorDelete)
    ON_BN_CLICKED(IDC_BUTTON1, &CvbdcoderDlg::OnBnClickedButton1)
    ON_NOTIFY(NM_DBLCLK, IDC_LIST, &CvbdcoderDlg::OnNMDblclkList)
    ON_BN_CLICKED(IDC_BUTTON_SAVE_PRESET, &CvbdcoderDlg::OnBnClickedButtonSavePreset)
    ON_BN_CLICKED(IDC_BUTTON_MANAGE_PRESETS, &CvbdcoderDlg::OnBnClickedButtonManagePresets)
END_MESSAGE_MAP()


// CvbdcoderDlg 消息处理程序

BOOL CvbdcoderDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	ShowWindow(SW_MINIMIZE);

	// TODO: 在此添加额外的初始化代码

    // 快捷键
    m_hAccel = ::LoadAccelerators(AfxGetInstanceHandle(), MAKEINTRESOURCE(IDR_ACCELERATOR1));


    // ListCtrl 风格
    DWORD dwStyle = ::GetWindowLong(m_list.m_hWnd, GWL_STYLE);
    dwStyle &= ~(LVS_TYPEMASK);
    dwStyle &= ~(LVS_EDITLABELS);
    SetWindowLong(m_list.m_hWnd, GWL_STYLE, dwStyle | LVS_REPORT | LVS_NOLABELWRAP | LVS_SHOWSELALWAYS);
    DWORD styles = LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES | LVS_EX_CHECKBOXES;
    ListView_SetExtendedListViewStyleEx(m_list.m_hWnd, styles, styles);
    // End

    // ListCtrl 表头
    TCHAR listHearders[COLUMN][10] = { L"文件", L"状态", L"格式", L"编码", L"视频码率", L"音频码率", L"剪切起点", L"剪切终点", L"路径"};
    int listColumnWidth[COLUMN] = {150,80,80,80,110,110,120,120,200};
    LV_COLUMN lvColumn;
    for (int i = 0; i < COLUMN; i++)
    {
        lvColumn.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH | LVCF_ORDER;
        lvColumn.fmt = LVCFMT_LEFT;
        lvColumn.pszText = listHearders[i];
        lvColumn.iSubItem = i;
        lvColumn.iOrder = i;
        lvColumn.cx = listColumnWidth[i];
        m_list.InsertColumn(i, &lvColumn);
    }
    // End

    // SPIN buddy
    CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN1);
    pSpin->SetBuddy((CEdit*)GetDlgItem(IDC_EDIT_VIDEO_BITRATE));
    pSpin->SetRange(100, 32767);
    UDACCEL Accel;
    Accel.nInc = 100;
    Accel.nSec = 100;
    pSpin->SetAccel(1, &Accel);

    pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN2);
    pSpin->SetBuddy((CEdit*)GetDlgItem(IDC_EDIT_AUDIO_BITRATE));
    pSpin->SetRange(0, 320);
    Accel.nInc = 16;
    Accel.nSec = 16;
    pSpin->SetAccel(1, &Accel);
    // End

    // Init presets
    settingsNow = InputFile(L"Default", H265, VBR, 3600, 128, false, false);
    updateSettings(false);
    presets.push_back(settingsNow);
    presets.push_back(InputFile(L"Default (old)", H264, PASS2, 5000, 128, false, false));
    presets.push_back(InputFile(L"Long", H264, PASS2, 3600, 128, false, false));
    presets.push_back(InputFile(L"CRF", H264, CRF, 26, 128, false, false));
    syncToSettings();
    m_preset.SelectString(-1, L"Default");

    // end

    // Time picker
    v_start.SetDateTime(0, 0, 0, 0, 0, 0);
    v_end.SetDateTime(0, 0, 0, 0, 0, 0);
    updateSettings(true);
    //end

    // edit hint
    ((CEdit*)GetDlgItem(IDC_EDIT_OUTPUT_DIR))->SetCueBanner(L"原始文件夹");
    // end

    // 测试

    //InputFile test(L"D:\\1.mp4",H264,VBR, 1000,1000);
    //listItems.push_back(test);
    addToListItems(L"D:\\tmp\\1.mp4",-1);
    updateList();
    m_list.SetCheck(0);
    m_list.SetItemState(0, LVIS_SELECTED | LVIS_FOCUSED, LVIS_SELECTED | LVIS_FOCUSED);
    // End

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CvbdcoderDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CvbdcoderDlg::OnPaint()
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
HCURSOR CvbdcoderDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



bool CvbdcoderDlg::updateList()
{
    int itemCountNow = m_list.GetItemCount();
    int listSize = listItems.size();
    LV_ITEM tmp;
    tmp.mask = LVIF_TEXT;

    for (int i=0;i!=listSize;i++)
    {
        auto itemInfo = listItems[i].getItemInfo();
        tmp.pszText = (LPWSTR)(LPCWSTR)itemInfo[0];
        tmp.iItem = i;
        tmp.iSubItem = 0;
        if (i < itemCountNow) m_list.SetItem(&tmp);
        else m_list.InsertItem(&tmp);
        for (int j = 1; j != COLUMN; j++)
        {
            tmp.iSubItem = j;
            tmp.pszText = (LPWSTR)(LPCWSTR)itemInfo[j];
            m_list.SetItem(&tmp);
        }
    }
    for (int i = listSize; i < itemCountNow; i++)
    {
        m_list.DeleteItem(i);
    }
    return 0;
}


void CvbdcoderDlg::OnBnClickedCheckCut()
{
    UpdateData(true);
    settingsNow.setCut(v_cut);

    setListItem();
    updateList();
    updateSettings(false);
}


void CvbdcoderDlg::OnBnClickedCheckCopyVideoStream()
{
    UpdateData(true);
    settingsNow.setCopyVideoStream(v_copyVideoStream);

    setListItem();
    updateList();
    updateSettings(false);
}


void CvbdcoderDlg::OnBnClickedCheckCopyAudioStream()
{
    UpdateData(true);
    settingsNow.setCopyAudioStream(v_copyAudioStream);

    setListItem();
    updateList();
    updateSettings(false);
}


bool CvbdcoderDlg::addToListItems(CString path, int pos)
{
    UpdateData(true);
    InputFile tmp(path, settingsNow);
    tmp.setCut(false);
    tmp.setCutRange(std::make_pair(0, 0));
    if (pos == -1) listItems.push_back(tmp);
    else listItems.insert(listItems.begin() + pos, tmp);
    return false;
}


void CvbdcoderDlg::OnBnClickedRadioFormat(UINT nCmdID)
{
    UpdateData(true);
    settingsNow.setFormat(v_format);

    setListItem();
    updateList();
    updateSettings(false);
}


void CvbdcoderDlg::OnBnClickedRadioCode(UINT nCmdID)
{
    UpdateData(true);
    settingsNow.setCode(v_code);

    setListItem();
    updateList();
    updateSettings(false);
}


void CvbdcoderDlg::OnEnChangeEditVideoBitrate()
{
    UpdateData(true);
    auto str = v_videoBitrate;
    str.Replace(L",", L"");
    int tmp = _wtoi(str);
    if (tmp > 0)
    {
        UpdateData(true);
        settingsNow.setVideoBitrate(tmp);

        setListItem();
        updateList();
        updateSettings(false);
    }
}


void CvbdcoderDlg::OnEnChangeEditAudioBitrate()
{
    UpdateData(true);
    auto str = v_audioBitrate;
    str.Replace(L",", L"");
    int tmp = _wtoi(str);
    if (tmp >= 0)
    {
        UpdateData(true);
        settingsNow.setAudioBitrate(tmp);

        setListItem();
        updateList();
        updateSettings(false);
    }
}

template<typename Lambda> bool CvbdcoderDlg::setListItem(Lambda foo)
{
    POSITION pos = m_list.GetFirstSelectedItemPosition();
    while (pos)
    {
        int iItem = m_list.GetNextSelectedItem(pos);
        foo(listItems[iItem]);
    }
    return false;
}


bool CvbdcoderDlg::setListItem()
{
    auto tmp = settingsNow.getData();
    setListItem([&](InputFile &i) {
        i.setData(tmp,false);
        });
    return false;
}

void CvbdcoderDlg::OnLvnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
    POSITION pos = m_list.GetFirstSelectedItemPosition();
    int iItem = m_list.GetNextSelectedItem(pos);
    if (iItem != -1)
    {
        InputFile tmp = listItems[iItem];
        auto a = tmp.getData();
        setListItem([&](InputFile i) {
            auto b = i.getData();
            for (auto j = 0; j != b.size(); j++)
            {
                if (a[j] != b[j]) a[j] = -1;
            }
            });
        //a[7] = a[8] = 0;
        tmp.setData(a);
        settingsNow.setData(tmp,true,true);
        updateSettings(false);
    }
}

bool CvbdcoderDlg::syncToSettings()
{
    m_preset.ResetContent();
    for (auto i : presets)
    {
        m_preset.AddString(i.getPath());

    }
    return false;
}


bool CvbdcoderDlg::applySettings()
{
    auto settings = settingsNow.getData();
    setListItem([=](InputFile i) {
        i.setData(settings,false);
        });
    updateList();
    return false;
}


bool CvbdcoderDlg::updateSettings(bool flag)
{
    UpdateData(true);
    if (flag)
    {
        auto str = v_videoBitrate;
        str.Replace(L",", L"");
        int vb = _wtoi(str);
        str = v_audioBitrate;
        str.Replace(L",", L"");
        int ab = _wtoi(str);
        InputFile settings(L"", v_format, v_code, vb, ab, v_copyVideoStream, v_copyAudioStream);
        settings.setCut(v_cut);
        settings.setCutRange(std::make_pair(v_start.GetHour() * 3600 + v_start.GetMinute() * 60 + v_start.GetSecond(), v_end.GetHour() * 3600 + v_end.GetMinute() * 60 + v_end.GetSecond()));
        settingsNow.setData(settings.getData(),false);
    }
    else
    {
        auto tmp = settingsNow.getData();
        v_format = tmp[0];
        v_code = tmp[1];
        CString str;
        str.Format(L"%d", tmp[2]);
        v_videoBitrate = tmp[2] == -1 ? L"(多个值)" : str;
        str.Format(L"%d", tmp[3]);
        v_audioBitrate = tmp[3] == -1 ? L"(多个值)" : str;
        v_copyVideoStream = tmp[4];
        v_copyAudioStream = tmp[5];
        v_cut = tmp[6];
        v_start.SetTime(tmp[7] / 3600, tmp[7] % 3600 / 60, tmp[7] % 60);
        v_end.SetTime(tmp[8] / 3600, tmp[8] % 3600 / 60, tmp[8] % 60);
        UpdateData(false);
        RefreshSettingBox();
    }
    return false;
}


bool CvbdcoderDlg::RefreshSettingBox()
{
    UpdateData(true);
    GetDlgItem(IDC_EDIT_VIDEO_BITRATE)->EnableWindow(!v_copyVideoStream);
    GetDlgItem(IDC_SPIN1)->EnableWindow(!v_copyVideoStream);
    GetDlgItem(IDC_EDIT_AUDIO_BITRATE)->EnableWindow(!v_copyAudioStream);
    GetDlgItem(IDC_SPIN2)->EnableWindow(!v_copyAudioStream);
    GetDlgItem(IDC_TIMEPICKER_START)->EnableWindow(v_cut);
    GetDlgItem(IDC_TIMEPICKER_END)->EnableWindow(v_cut);

    CSpinButtonCtrl* pSpin = (CSpinButtonCtrl*)GetDlgItem(IDC_SPIN1);
    UDACCEL Accel;
    if (v_code == CRF)
    {
        m_static.SetWindowTextW(L"CRF");
        pSpin->SetRange(1, 40);
        //pSpin->SetPos(settingsNow.getVideoBitrate());
        Accel.nInc = 1;
        Accel.nSec = 1;
        pSpin->SetAccel(1, &Accel);

    }
    else
    {
        m_static.SetWindowTextW(L"视频码率");
        pSpin->SetRange(100, 32767);
        //pSpin->SetPos(settingsNow.getVideoBitrate());
        Accel.nInc = 100;
        Accel.nSec = 100;
        pSpin->SetAccel(1, &Accel);
    }
    return false;
}


void CvbdcoderDlg::OnCbnSelchangeComboPreset()
{
    // TODO: 在此添加控件通知处理程序代码
    settingsNow = presets[m_preset.GetCurSel()];
    updateSettings(false);
}


void CvbdcoderDlg::OnDtnDatetimechangeTimepicker(UINT nCmdID, NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMDATETIMECHANGE pDTChange = reinterpret_cast<LPNMDATETIMECHANGE>(pNMHDR);
    UpdateData(true);
    int start = v_start.GetHour() * 3600 + v_start.GetMinute() * 60 + v_start.GetSecond();
    int end= v_end.GetHour() * 3600 + v_end.GetMinute() * 60 + v_end.GetSecond();
    if (start > end) end = start;
    settingsNow.setCutRange(std::make_pair(start, end));

    setListItem();
    updateList();
    updateSettings(false);
    *pResult = 0;
}

void CvbdcoderDlg::OnAbout()
{
    //aboutDlg.ShowWindow(SW_SHOW);
    //aboutDlg.UpdateWindow();
    auto aboutDlg = new CAboutDlg();
    aboutDlg->Create(IDD_ABOUTBOX, this);
    aboutDlg->ShowWindow(SW_SHOW);
}


BOOL FolderExist(CString strPath)
{
    WIN32_FIND_DATA wfd;
    BOOL bValue = FALSE;
    HANDLE hFind = ::FindFirstFile(strPath, &wfd);
    if ((hFind != INVALID_HANDLE_VALUE) && (wfd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))//文件找到
    {
        bValue = TRUE;
    }
    ::FindClose(hFind);
    return bValue;
}

BOOL CreateFolder(CString strPath)
{
    SECURITY_ATTRIBUTES attrib;
    attrib.bInheritHandle = FALSE;
    attrib.lpSecurityDescriptor = NULL;
    attrib.nLength = sizeof(SECURITY_ATTRIBUTES);
    return ::CreateDirectory(strPath, &attrib);
    //	return ::CreateDirectory(strPath,NULL);
}

void CvbdcoderDlg::OnBnClickedButtonStart()
{
    auto CStringToChar = [](CString& str)
    {
        int len = str.GetLength();
        TCHAR* tr = str.GetBuffer(len);
        str.ReleaseBuffer();
        return tr;
    };
    if (!threadRunning)
    {
        UpdateData(true);
        CString destDir = v_outputDir;
        destDir.Trim();
        if (destDir!=L"" && !FolderExist(destDir))
        {
            CString cmd = L"mkdir \"" + destDir + "\"";
            
            ShellExecute(NULL, L"open", L"cmd", cmd, NULL, SW_HIDE);

            if (!FolderExist(destDir))
            {
                AfxMessageBox(L"目标文件夹无效!");
                return;
            }
        }
        para_outputDir = destDir;
        threadRunning = true;
        cThread = AfxBeginThread(runCoding, this, THREAD_PRIORITY_NORMAL, 0, 0, NULL);
        
    }
}

UINT CvbdcoderDlg::runCoding(LPVOID pvParam)
{
    // Init
    auto these = (CvbdcoderDlg*)pvParam;
    these->m_output.SetWindowTextW(L"");
    these->m_command.SetWindowTextW(L"");
    auto CStringToChar = [](CString& str)
    {
        int len = str.GetLength();
        TCHAR* tr = str.GetBuffer(len);
        str.ReleaseBuffer();
        return tr;
    };

    // 找到第一个就绪的项目
    bool isAbleToWork = true;
    int listSize;
    InputFile* itemToBeCoded=NULL;
    while (true)
    {
        if (toBeTerminated)
        {
            toBeTerminated = false;
        }
        listSize = these->m_list.GetItemCount();
        for (int i = 0; i != listSize; i++)
        {
            if (these->m_list.GetCheck(i) && listItems[i].getStatus()==READY)
            {
                itemToBeCoded = &(listItems[i]);
                break;
            }
            else if (i == listSize - 1) isAbleToWork = false;
        }

        if (!isAbleToWork || listSize==0) break;
        // get to work

        auto commands = itemToBeCoded->translate(these->para_outputDir);

        itemToBeCoded->setStatus(CODING);
        these->updateList();

        SECURITY_ATTRIBUTES sa;
        HANDLE hRead, hWrite;
        sa.nLength = sizeof(SECURITY_ATTRIBUTES);
        sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;
        STARTUPINFO si = { sizeof(si) };
        si.wShowWindow = SW_HIDE;
        si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;

        CString output;
        
        for (auto i : commands)
        {
            these->m_command.SetWindowTextW(i);

            if (!CreatePipe(&hRead, &hWrite, &sa, 0))
            {
                AfxMessageBox(L"Error on CreatePipe()!");
                itemToBeCoded->setStatus(READY);
                these->updateList();
                return 0;
            }
            si.hStdError = hWrite;
            si.hStdOutput = hWrite;
            TCHAR* cmdline = CStringToChar(i);
            if (!CreateProcess(NULL, cmdline, NULL, NULL, TRUE, NULL, NULL, NULL, &si, &ffmpegProcess))
            {
                AfxMessageBox(L"Error on CreateProcess()!");
                itemToBeCoded->setStatus(READY);
                these->updateList();
                return 0;
            }
            CloseHandle(hWrite);

            char buffer[4096];
            memset(buffer, 0, 4096);
            DWORD byteRead;
            bool suspended = false;
            ULONGLONG timestamp = GetTickCount64();
            while (true)
            {
                if (terminateNow)
                {
                    TerminateProcess(ffmpegProcess.hProcess, 127);
                    itemToBeCoded->setStatus(READY);
                    these->updateList();
                    these->resetControlArea();
                    return 0;
                }
                if (ReadFile(hRead, buffer, 7, &byteRead, NULL) == NULL)
                {
                    these->m_output.SetWindowTextW(output);
                    these->m_output.LineScroll(these->m_output.GetLineCount(), 0);
                    break;
                }
                output += buffer;
                output.Replace(L"frame", L"\r\nFrm");
                if (GetTickCount64() - timestamp > 500)
                {
                    these->m_output.SetWindowTextW(output);
                    these->m_output.LineScroll(these->m_output.GetLineCount(), 0);
                    timestamp = GetTickCount64();
                }
            }
        }
        itemToBeCoded->setStatus(CODED);
        these->updateList();
    }
    these->resetControlArea();
    return 0;
}


void CvbdcoderDlg::OnResetStatus()
{
    for (auto& i : listItems)
    {
        i.setStatus(READY);
    }
    updateList();
}


void CvbdcoderDlg::OnBnClickedButtonStopafter()
{
    toBeTerminated = true;
}


void CvbdcoderDlg::OnBnClickedButtonStopnow()
{
    terminateNow = true;
    if (suspendNow) ResumeThread(ffmpegProcess.hThread);
}


void CvbdcoderDlg::OnBnClickedButtonPause()
{
    CString tmp;
    m_pause.GetWindowTextW(tmp);
    if (tmp== L"暂停")
    {
        suspendNow = true;
        m_pause.SetWindowTextW(L"继续");
        SuspendThread(ffmpegProcess.hThread);
    }
    else if (tmp== L"继续")
    {
        suspendNow = false;
        m_pause.SetWindowTextW(L"暂停");
        ResumeThread(ffmpegProcess.hThread);
    }
}


void CvbdcoderDlg::resetControlArea()
{
    threadRunning = false, toBeTerminated = false, terminateNow = false, suspendNow=false;
    m_pause.SetWindowTextW(L"暂停");
}


void CvbdcoderDlg::OnAcceleratorDelete()
{
    POSITION pos = m_list.GetFirstSelectedItemPosition();
    std::vector<int> toDelete;
    while (pos)
    {
        int iItem = m_list.GetNextSelectedItem(pos);
        if (listItems[iItem].getStatus() == CODING)
        {
            AfxMessageBox(L"不能删除正在执行中的任务!", MB_ICONWARNING);
            continue;
        }
        toDelete.push_back(iItem);
    }
    for (int i = toDelete.size() - 1; i >= 0; i--)
    {
        m_list.DeleteItem(toDelete[i]);
        listItems.erase(listItems.begin()+toDelete[i]);
    }
    //updateList();
}

BOOL CvbdcoderDlg::PreTranslateMessage(MSG* pMsg)
{
    if (GetFocus() == GetDlgItem(IDC_LIST) && ::TranslateAccelerator(m_hWnd, m_hAccel, pMsg))
    {
        return true;
    }
    return false;
}

void CvbdcoderDlg::OnBnClickedButton1()
{
    WCHAR szPath[MAX_PATH];     //存放选择的目录路径 
    CString str;

    ZeroMemory(szPath, sizeof(szPath));

    BROWSEINFO bi;
    bi.hwndOwner = m_hWnd;
    bi.pidlRoot = NULL;
    bi.pszDisplayName = szPath;
    bi.lpszTitle = L"请选择输出目录";
    bi.ulFlags = 0;
    bi.lpfn = NULL;
    bi.lParam = 0;
    bi.iImage = 0;
    //弹出选择目录对话框
    LPITEMIDLIST lp = SHBrowseForFolder(&bi);

    if (lp && SHGetPathFromIDList(lp, szPath))
    {
        v_outputDir = szPath;
        UpdateData(false);
    }

}

BOOL CAboutDlg::OnInitDialog()
{
    CDialogEx::OnInitDialog();

    // TODO:  在此添加额外的初始化
    v_CR = L"基于 BDcoder 开发 \r\n https://github.com/BearDic/bdcoder";
    UpdateData(false);

    return TRUE;  // return TRUE unless you set the focus to a control
                  // 异常: OCX 属性页应返回 FALSE
}


void CvbdcoderDlg::OnNMDblclkList(NMHDR* pNMHDR, LRESULT* pResult)
{
    LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
    // TODO: 在此添加控件通知处理程序代码.
    POSITION pos = m_list.GetFirstSelectedItemPosition();
    int iItem = m_list.GetNextSelectedItem(pos);
    if (iItem!=-1) ShellExecute(NULL, L"open", listItems[iItem].getPath(), NULL, NULL, SW_SHOWNORMAL);
    *pResult = 0;
}

//TODO!!!
void CvbdcoderDlg::OnBnClickedButtonSavePreset()
{
    // TODO: 在此添加控件通知处理程序代码
    AfxMessageBox(L"预设管理功能尚在开发中...");
}

//TODO!!!
void CvbdcoderDlg::OnBnClickedButtonManagePresets()
{
    // TODO: 在此添加控件通知处理程序代码
    AfxMessageBox(L"预设管理功能尚在开发中...");
}
