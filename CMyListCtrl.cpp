#include "pch.h"
#include "CMyListCtrl.h"
#include "vbdcoderDlg.h"
BEGIN_MESSAGE_MAP(CMyListCtrl, CListCtrl)
    ON_WM_DROPFILES()
END_MESSAGE_MAP()


void CMyListCtrl::OnDropFiles(HDROP hDropInfo)
{
    // TODO: 在此添加消息处理程序代码和/或调用默认值
    
    auto dlg = (CvbdcoderDlg*)CvbdcoderDlg::FromHandle(AfxGetMainWnd()->GetSafeHwnd());
    wchar_t path[_MAX_PATH];
    auto size = GetItemCount();

    UINT nNumOfFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
    for (int i = 0; i != nNumOfFiles; i++)
    {
        DragQueryFile(hDropInfo, i, path, _MAX_PATH);
        dlg->addToListItems(path, -1);
    }
    dlg->updateList();
    for (int i = 0; i != nNumOfFiles; i++)
    {
        SetCheck(size + i, true);
    }
    CListCtrl::OnDropFiles(hDropInfo);
    DragFinish(hDropInfo);
}
