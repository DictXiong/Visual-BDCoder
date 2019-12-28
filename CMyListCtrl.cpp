 #include "pch.h"
#include "CMyListCtrl.h"
#include "vbdcoderDlg.h"
BEGIN_MESSAGE_MAP(CMyListCtrl, CListCtrl)
    ON_WM_DROPFILES()
END_MESSAGE_MAP()

/*
OnDropFiles -> void
消息处理函数
重写了列表的拖放操作, 支持多文件同时拖放. 如果拖入的是文件, 则加入列表. 
注: 检测了后缀名, 如果不合法则弹出窗口提示.
*/
void CMyListCtrl::OnDropFiles(HDROP hDropInfo)
{
    auto dlg = (CvbdcoderDlg*)CvbdcoderDlg::FromHandle(AfxGetMainWnd()->GetSafeHwnd());
    wchar_t path[_MAX_PATH];
    auto size = GetItemCount();

    UINT nNumOfFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
    int addedCount = 0;
    for (int i = 0; i != nNumOfFiles; i++)
    {
        DragQueryFile(hDropInfo, i, path, _MAX_PATH);
        DWORD attrib = GetFileAttributes(path);
        if (!CvbdcoderDlg::checkPath(path))
        {
            CString tmp;
            tmp.Format(L"不支持的后缀名, 跳过文件: %s", path);
            AfxMessageBox(tmp);
            continue;
        }
        if ( attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY) == 0)
        {
            dlg->addToListItems(path, -1);
            addedCount++;
        }
    }
    dlg->updateList();
    for (int i = 0; i != addedCount; i++)
    {
        SetCheck(size + i, true);
    }
    CListCtrl::OnDropFiles(hDropInfo);
    DragFinish(hDropInfo);
}
