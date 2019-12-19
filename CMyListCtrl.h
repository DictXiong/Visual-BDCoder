#pragma once
#include <afxcmn.h>
class CMyListCtrl :
    public CListCtrl
{
public:
    DECLARE_MESSAGE_MAP()
    afx_msg void OnDropFiles(HDROP hDropInfo);
};

