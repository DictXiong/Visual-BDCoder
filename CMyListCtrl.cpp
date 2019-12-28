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
ע: ����˺�׺��, ������Ϸ��򵯳�������ʾ.
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
            tmp.Format(L"��֧�ֵĺ�׺��, �����ļ�: %s", path);
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
