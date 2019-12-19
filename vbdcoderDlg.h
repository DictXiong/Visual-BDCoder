
// vbdcoderDlg.h: 头文件
//

#pragma once
#include"CMyListCtrl.h"
#include"InputFile.h"

// CvbdcoderDlg 对话框
class CvbdcoderDlg : public CDialogEx
{
// 构造
public:
	CvbdcoderDlg(CWnd* pParent = nullptr);	// 标准构造函数
    ~CvbdcoderDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_VBDCODER_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
    CWinThread* cThread;
    unsigned int ThreadID;
    HACCEL m_hAccel;
public:
    CMyListCtrl m_list;
    CComboBox m_preset;
    CString v_outputDir;
    BOOL v_format;
    BOOL v_code;
    CString v_videoBitrate;
    CString v_audioBitrate;
    BOOL v_copyAudioStream;
    COleDateTime v_start;
    COleDateTime v_end;
    BOOL v_cut;
    CString v_command;
    CString v_output;
    BOOL v_copyVideoStream;
    bool updateList();
    afx_msg void OnBnClickedCheckCut();
    afx_msg void OnBnClickedCheckCopyVideoStream();
    afx_msg void OnBnClickedCheckCopyAudioStream();
    bool addToListItems(CString path, int pos);
    afx_msg void OnBnClickedRadioFormat(UINT nCmdID);
    afx_msg void OnBnClickedRadioCode(UINT nCmdID);
    afx_msg void OnEnChangeEditVideoBitrate();
    afx_msg void OnEnChangeEditAudioBitrate();
    template<typename Lambda> bool setListItem(Lambda foo);
    bool setListItem();
    afx_msg void OnLvnItemchangedList(NMHDR* pNMHDR, LRESULT* pResult);
    bool syncToSettings();
    bool applySettings();
    bool updateSettings(bool flag);
    bool RefreshSettingBox();
    CStatic m_static;
    afx_msg void OnCbnSelchangeComboPreset();
    afx_msg void OnDtnDatetimechangeTimepicker(UINT nCmdID, NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnAbout();
    afx_msg void OnBnClickedButtonStart();
    CEdit m_output;
    static UINT runCoding(LPVOID pvParam);
    CEdit m_command;
    afx_msg void OnResetStatus();
    afx_msg void OnBnClickedButtonStopafter();
    afx_msg void OnBnClickedButtonStopnow();
    afx_msg void OnBnClickedButtonPause();
    CButton m_pause;
    void resetControlArea();
    afx_msg void OnAcceleratorDelete();
    virtual BOOL PreTranslateMessage(MSG* pMsg);
    CString para_outputDir;
    afx_msg void OnBnClickedButton1();
    afx_msg void OnNMDblclkList(NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnBnClickedButtonSavePreset();
    afx_msg void OnBnClickedButtonManagePresets();
};
