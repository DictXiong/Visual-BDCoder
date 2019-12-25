 #include "pch.h"
#include "CMyListCtrl.h"
#include "vbdcoderDlg.h"
BEGIN_MESSAGE_MAP(CMyListCtrl, CListCtrl)
    ON_WM_DROPFILES()
END_MESSAGE_MAP()

/*
OnDropFiles -> void
��Ϣ������
��д���б���ϷŲ���, ֧�ֶ��ļ�ͬʱ�Ϸ�. �����������ļ�, ������б�. 
*/
void CMyListCtrl::OnDropFiles(HDROP hDropInfo)
{
    auto dlg = (CvbdcoderDlg*)CvbdcoderDlg::FromHandle(AfxGetMainWnd()->GetSafeHwnd());
    wchar_t path[_MAX_PATH];
    auto size = GetItemCount();

    UINT nNumOfFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
    for (int i = 0; i != nNumOfFiles; i++)
    {
        DragQueryFile(hDropInfo, i, path, _MAX_PATH);
        DWORD attrib = GetFileAttributes(path);
        if (attrib != INVALID_FILE_ATTRIBUTES && (attrib & FILE_ATTRIBUTE_DIRECTORY) == 0)
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
