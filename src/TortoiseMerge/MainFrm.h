// TortoiseMerge - a Diff/Patch program

// Copyright (C) 2004 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
#pragma once

#include "BaseView.h"
#include "LeftView.h"
#include "RightView.h"
#include "BottomView.h"
#include "DiffData.h"
#include "LocatorBar.h"
#include "FilePatchesDlg.h"
#include "Utils.h"
#include "TempFiles.h"
#include "XSplitter.h"
#include "NewMenu.h"
#include "NewToolBar.h"

class CMainFrame : public CNewFrameWnd, public CPatchFilesDlgCallBack
{
	
public:
	CMainFrame();
	virtual ~CMainFrame();

#ifdef _DEBUG
	virtual void	AssertValid() const;
	virtual void	Dump(CDumpContext& dc) const;
#endif
protected: 
	DECLARE_DYNAMIC(CMainFrame)

	virtual BOOL	PreCreateWindow(CREATESTRUCT& cs);
	virtual BOOL	OnCreateClient(LPCREATESTRUCT lpcs, CCreateContext* pContext);

	afx_msg void	OnFileSave();
	afx_msg void	OnFileSaveAs();
	afx_msg void	OnFileOpen();
	afx_msg void	OnViewWhitespaces();
	afx_msg int		OnCreate(LPCREATESTRUCT lpCreateStruct);
	afx_msg void	OnSize(UINT nType, int cx, int cy);
	afx_msg void	OnUpdateFileSave(CCmdUI *pCmdUI);
	afx_msg void	OnUpdateFileSaveAs(CCmdUI *pCmdUI);
	afx_msg void	OnViewOnewaydiff();
	afx_msg void	OnUpdateViewOnewaydiff(CCmdUI *pCmdUI);
	afx_msg void	OnUpdateViewWhitespaces(CCmdUI *pCmdUI);
	afx_msg void	OnViewOptions();

	DECLARE_MESSAGE_MAP()
protected:
	void			UpdateLayout();
	virtual BOOL	PatchFile(CString sFilePath, CString sVersion);
	BOOL			CheckResolved();
	void			SaveFile(CString sFilePath);
protected: 
	CStatusBar		m_wndStatusBar;
	CNewToolBar		m_wndToolBar;
	CReBar			m_wndReBar;
	CLocatorBar		m_wndLocatorBar;
	CXSplitter		m_wndSplitter;
	CXSplitter		m_wndSplitter2;
	CFilePatchesDlg m_dlgFilePatches;

	CPatch			m_Patch;
	BOOL			m_bInitSplitter;
	CTempFiles		m_TempFiles;

public:
	CLeftView *		m_pwndLeftView;
	CRightView *	m_pwndRightView;
	CBottomView *	m_pwndBottomView;
	BOOL			m_bOneWay;
	CDiffData		m_Data;
	BOOL			LoadViews();
	afx_msg void OnClose();
};


