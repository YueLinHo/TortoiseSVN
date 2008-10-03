// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2003-2008 - TortoiseSVN

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#include "stdafx.h"
#include "TortoiseProc.h"
#include "MemDC.h"
#include <gdiplus.h>
#include "Revisiongraphdlg.h"
#include "MessageBox.h"
#include "SVN.h"
#include "TempFile.h"
#include "UnicodeUtils.h"
#include "TSVNPath.h"
#include "SVNInfo.h"
#include ".\revisiongraphwnd.h"
#include "CachedLogInfo.h"
#include "RevisionGraph/IRevisionGraphLayout.h"
#include "RevisionGraph/FullGraphBuilder.h"
#include "RevisionGraph/FullGraphFinalizer.h"
#include "RevisionGraph/VisibleGraphBuilder.h"
#include "RevisionGraph/StandardLayout.h"
#include "RevisionGraph/ShowWC.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

using namespace Gdiplus;

void CRevisionGraphWnd::InitView()
{
	m_bIsRubberBand = false;

    CRect viewRect = GetViewRect();
	SetScrollbars (0,0,viewRect.Width(),viewRect.Height());
}

void CRevisionGraphWnd::BuildPreview()
{
	m_Preview.DeleteObject();
	if (!m_bShowOverview)
		return;

	// is there a point in drawing this at all?

	if (GetNodeCount() > REVGRAPH_PREVIEW_MAX_NODES)
		return;

	float origZoom = m_fZoomFactor;
	// zoom the graph so that it is completely visible in the window
	DoZoom(1.0);

    CRect graphRect = GetGraphRect();
	float horzfact = float(graphRect.Width())/float(REVGRAPH_PREVIEW_WIDTH);
	float vertfact = float(graphRect.Height())/float(REVGRAPH_PREVIEW_HEIGHT);
	float fZoom = 1.0f/(max(horzfact, vertfact));
	if (fZoom > 1.0f)
		fZoom = 1.0f;
	int trycounter = 0;
	m_fZoomFactor = fZoom;

	while ((trycounter < 5)&&((graphRect.Width()>REVGRAPH_PREVIEW_WIDTH)||(graphRect.Height()>REVGRAPH_PREVIEW_HEIGHT)))
	{
		m_fZoomFactor = fZoom;
		DoZoom(m_fZoomFactor);
		fZoom *= 0.95f;
		trycounter++;
	}
	// make sure the preview window has a minimal size
	if ((graphRect.Width()>10) && (graphRect.Height()>10))
	{
		m_previewWidth = max(graphRect.Width(), 30);
		m_previewHeight = max(graphRect.Height(), 30);
	}
	else
	{
		// if the preview window is too small (at least one side is zero)
		// then don't show the preview at all.
		m_previewHeight = 0;
		m_previewWidth = 0;
	}

	CClientDC ddc(this);
	CDC dc;
	if (!dc.CreateCompatibleDC(&ddc))
		return;
	m_Preview.CreateCompatibleBitmap(&ddc, REVGRAPH_PREVIEW_WIDTH, REVGRAPH_PREVIEW_HEIGHT);
	HBITMAP oldbm = (HBITMAP)dc.SelectObject(m_Preview);
	// paint the whole graph
	DrawGraph(&dc, GetViewRect(), 0, 0, true);
	// now we have a bitmap the size of the preview window
	dc.SelectObject(oldbm);
	dc.DeleteDC();

	DoZoom(origZoom);
}

void CRevisionGraphWnd::SetScrollbars(int nVert, int nHorz, int oldwidth, int oldheight)
{
	CRect clientrect;
	GetClientRect(&clientrect);
	const CRect& pRect = GetGraphRect();

    SCROLLINFO ScrollInfo = {sizeof(SCROLLINFO), SIF_ALL};
	GetScrollInfo(SB_VERT, &ScrollInfo);

	if ((nVert)||(oldheight==0))
		ScrollInfo.nPos = nVert;
	else
		ScrollInfo.nPos = ScrollInfo.nPos * pRect.Height() / oldheight;

    ScrollInfo.nMin = 0;
	ScrollInfo.nMax = static_cast<int>(pRect.bottom * m_fZoomFactor);
	ScrollInfo.nPage = clientrect.Height();
	ScrollInfo.nTrackPos = 0;
	SetScrollInfo(SB_VERT, &ScrollInfo);

	GetScrollInfo(SB_HORZ, &ScrollInfo);
	if ((nHorz)||(oldwidth==0))
		ScrollInfo.nPos = nHorz;
	else
		ScrollInfo.nPos = ScrollInfo.nPos * pRect.Width() / oldwidth;

	ScrollInfo.nMax = static_cast<int>(pRect.right * m_fZoomFactor);
	ScrollInfo.nPage = clientrect.Width();
	SetScrollInfo(SB_HORZ, &ScrollInfo);
}

CRect CRevisionGraphWnd::GetGraphRect()
{
    return m_layout.get() != NULL
        ? m_layout->GetRect()
        : CRect (0,0,0,0);
}

CRect CRevisionGraphWnd::GetViewRect()
{
	CRect clientRect;
	GetClientRect (&clientRect);

    CRect result;
    result.UnionRect (clientRect, GetGraphRect()); 
    return result;
}

int CRevisionGraphWnd::GetNodeCount()
{
    return static_cast<int>(m_visibleGraph.GetNodeCount());
}

svn_revnum_t CRevisionGraphWnd::GetHeadRevision() const
{
    return m_fullHistory.get() != NULL
        ? m_fullHistory->GetHeadRevision()
        : 0;
}

int CRevisionGraphWnd::GetEncoderClsid(const WCHAR* format, CLSID* pClsid)
{
	UINT  num = 0;          // number of image encoders
	UINT  size = 0;         // size of the image encoder array in bytes

	ImageCodecInfo* pImageCodecInfo = NULL;

	if (GetImageEncodersSize(&num, &size)!=Ok)
		return -1;
	if(size == 0)
		return -1;  // Failure

	pImageCodecInfo = (ImageCodecInfo*)(malloc(size));
	if(pImageCodecInfo == NULL)
		return -1;  // Failure

	if (GetImageEncoders(num, size, pImageCodecInfo)==Ok)
	{
		for(UINT j = 0; j < num; ++j)
		{
			if( wcscmp(pImageCodecInfo[j].MimeType, format) == 0 )
			{
				*pClsid = pImageCodecInfo[j].Clsid;
				free(pImageCodecInfo);
				return j;  // Success
			}
		}

	}
	free(pImageCodecInfo);
	return -1;  // Failure
}

void CRevisionGraphWnd::Compare (TDiffFunc func, bool bHead)
{
	ASSERT(m_SelectedEntry1 != NULL);
	ASSERT(m_SelectedEntry2 != NULL);

	CString sRepoRoot = m_fullHistory.get() != NULL
                      ? m_fullHistory->GetRepositoryRoot()
                      : CString();

	CTSVNPath url1;
	CTSVNPath url2;
	url1.SetFromSVN (sRepoRoot + CUnicodeUtils::GetUnicode (m_SelectedEntry1->GetPath().GetPath().c_str()));
	url2.SetFromSVN (sRepoRoot + CUnicodeUtils::GetUnicode (m_SelectedEntry2->GetPath().GetPath().c_str()));

	SVNRev peg = (SVNRev)(bHead ? m_SelectedEntry1->GetRevision() : SVNRev());

    SVN svn;
	SVNDiff diff (&svn, this->m_hWnd);
	diff.SetAlternativeTool(!!(GetAsyncKeyState(VK_SHIFT) & 0x8000));
	(diff.*func)(url1, (bHead ? SVNRev::REV_HEAD : m_SelectedEntry1->GetRevision()),
		         url2, (bHead ? SVNRev::REV_HEAD : m_SelectedEntry2->GetRevision()),
		         peg, false, false);
}

bool CRevisionGraphWnd::FetchRevisionData 
    ( const CString& path
    , SVNRev pegRevision
    , const CAllRevisionGraphOptions& options)
{
    // (re-)fetch the data

    m_fullHistory.reset (new CFullHistory());

    bool showWCRev = options.GetOption<CShowWC>()->IsActive();
	bool result = m_fullHistory->FetchRevisionData (path, pegRevision, showWCRev, m_pProgress);
    if (result)
    {
        CFullGraphBuilder builder (*m_fullHistory, m_fullGraph);
        builder.Run();

        CFullGraphFinalizer finalizer (*m_fullHistory, m_fullGraph);
        finalizer.Run();
    }

    return result;
}

bool CRevisionGraphWnd::AnalyzeRevisionData 
    (const CAllRevisionGraphOptions& options)
{
    m_layout.reset();
    if (m_fullGraph.GetNodeCount() > 0)
    {
        // filter graph

        CVisibleGraphBuilder builder ( m_fullGraph
                                     , m_visibleGraph
                                     , options.GetCopyFilterOptions());
        builder.Run();
        options.GetModificationOptions().Apply (&m_visibleGraph);

        index_t index = 0;
        for (size_t i = 0, count = m_visibleGraph.GetRootCount(); i < count; ++i)
            index = m_visibleGraph.GetRoot (i)->InitIndex (index);

        // layout nodes

        std::auto_ptr<CStandardLayout> newLayout 
            ( new CStandardLayout ( m_fullHistory->GetCache()
                                  , &m_visibleGraph));
        options.GetLayoutOptions().Apply (newLayout.get());
        newLayout->Finalize();

        m_layout = newLayout;
    }

    return m_layout.get() != NULL;
}

CString CRevisionGraphWnd::GetLastErrorMessage() const
{
    return m_fullHistory->GetLastErrorMessage();
}

bool CRevisionGraphWnd::GetShowOverview() const
{
    return m_bShowOverview != FALSE;
}

void CRevisionGraphWnd::SetShowOverview (bool value)
{
    m_bShowOverview = value;
	if (m_bShowOverview)
		BuildPreview();
}

void CRevisionGraphWnd::CompareRevs(bool bHead)
{
    Compare (&SVNDiff::ShowCompare, bHead);
}

void CRevisionGraphWnd::UnifiedDiffRevs(bool bHead)
{
    Compare (&SVNDiff::ShowUnifiedDiff, bHead);
}

void CRevisionGraphWnd::DoZoom(float fZoomFactor)
{
	float oldzoom = m_fZoomFactor;
	m_fZoomFactor = fZoomFactor;

    m_nFontSize = max(1, int(12.0f * fZoomFactor));
    if (m_nFontSize < 7)
        m_nFontSize = min (7, int(15.0f * fZoomFactor));

	m_RoundRectPt.x = int(ROUND_RECT * fZoomFactor);
	m_RoundRectPt.y = int(ROUND_RECT * fZoomFactor);
	m_nIconSize = int(32 * fZoomFactor);
	for (int i=0; i<MAXFONTS; i++)
	{
		if (m_apFonts[i] != NULL)
		{
			m_apFonts[i]->DeleteObject();
			delete m_apFonts[i];
		}
		m_apFonts[i] = NULL;
	}

	SCROLLINFO si1 = {sizeof(SCROLLINFO), SIF_ALL};
	GetScrollInfo(SB_VERT, &si1);
	SCROLLINFO si2 = {sizeof(SCROLLINFO), SIF_ALL};
	GetScrollInfo(SB_HORZ, &si2);

	InitView();

	si1.nPos = int(float(si1.nPos)*m_fZoomFactor/oldzoom);
	si2.nPos = int(float(si2.nPos)*m_fZoomFactor/oldzoom);
	SetScrollPos (SB_VERT, si1.nPos);
	SetScrollPos (SB_HORZ, si2.nPos);

	Invalidate();
}

