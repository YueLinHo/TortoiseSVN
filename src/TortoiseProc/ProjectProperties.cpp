// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2003-2004 - Stefan Kueng

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

#include "StdAfx.h"
#include "shlwapi.h"
#include ".\ProjectProperties.h"

ProjectProperties::ProjectProperties(void)
{
	bNumber = TRUE;
	bWarnIfNoIssue = FALSE;
	nLogWidthMarker = 80;
}

ProjectProperties::~ProjectProperties(void)
{
}

BOOL ProjectProperties::ReadPropsTempfile(CString path)
{
	CString strLine;
	try
	{
		CStdioFile file(path, CFile::typeBinary | CFile::modeRead);
		// for every selected file/folder
		while (file.ReadString(strLine))
		{
			if (ReadProps(strLine))
			{
				file.Close();
				return TRUE;
			}
		}
		file.Close();
	}
	catch (CFileException* pE)
	{
		TRACE("CFileException in ReadPropsTempfile!\n");
		pE->Delete();
	}
	return FALSE;
}

BOOL ProjectProperties::ReadProps(CString path)
{
	BOOL bFoundBugtraqLabel = FALSE;
	BOOL bFoundBugtraqMessage = FALSE;
	BOOL bFoundBugtraqNumber = FALSE;
	BOOL bFoundBugtraqURL = FALSE;
	BOOL bFoundBugtraqWarnIssue = FALSE;
	BOOL bFoundLogWidth = FALSE;
	BOOL bFoundLogTemplate = FALSE;
	path.Replace('/', '\\');
	if (!PathIsDirectory(path))
	{
		path = path.Left(path.ReverseFind('\\'));
	}
	for (;;)
	{
		SVNProperties props(path);
		for (int i=0; i<props.GetCount(); ++i)
		{
			CString sPropName = props.GetItemName(i).c_str();
			stdstring sPropVal = props.GetItemValue(i);
			if ((!bFoundBugtraqLabel)&&(sPropName.Compare(BUGTRAQPROPNAME_LABEL)==0))
			{
#ifdef UNICODE
				sLabel = MultibyteToWide((char *)sPropVal.c_str()).c_str();
#else
				sLabel = sPropVal.c_str();
#endif
				bFoundBugtraqLabel = TRUE;
			}
			if ((!bFoundBugtraqMessage)&&(sPropName.Compare(BUGTRAQPROPNAME_MESSAGE)==0))
			{
#ifdef UNICODE
				sMessage = MultibyteToWide((char *)sPropVal.c_str()).c_str();
#else
				sMessage = sPropVal.c_str();
#endif
				bFoundBugtraqMessage = TRUE;
			}
			if ((!bFoundBugtraqNumber)&&(sPropName.Compare(BUGTRAQPROPNAME_NUMBER)==0))
			{
				CString val;
#ifdef UNICODE
				val = MultibyteToWide((char *)sPropVal.c_str()).c_str();
#else
				val = sPropVal.c_str();
#endif
				if (val.CompareNoCase(_T("false"))==0)
					bNumber = FALSE;
				else
					bNumber = TRUE;
				bFoundBugtraqNumber = TRUE;
			}
			if ((!bFoundBugtraqURL)&&(sPropName.Compare(BUGTRAQPROPNAME_URL)==0))
			{
#ifdef UNICODE
				sUrl = MultibyteToWide((char *)sPropVal.c_str()).c_str();
#else
				sUrl = sPropVal.c_str();
#endif
				bFoundBugtraqURL = TRUE;
			}
			if ((!bFoundBugtraqWarnIssue)&&(sPropName.Compare(BUGTRAQPROPNAME_WARNIFNOISSUE)==0))
			{
				CString val;
#ifdef UNICODE
				val = MultibyteToWide((char *)sPropVal.c_str()).c_str();
#else
				val = sPropVal.c_str();
#endif
				if (val.CompareNoCase(_T("true"))==0)
					bWarnIfNoIssue = TRUE;
				else
					bWarnIfNoIssue = FALSE;
				bFoundBugtraqWarnIssue = TRUE;
			}
			if ((!bFoundLogWidth)&&(sPropName.Compare(PROJECTPROPNAME_LOGWIDTHLINE)==0))
			{
				CString val;
#ifdef UNICODE
				val = MultibyteToWide((char *)sPropVal.c_str()).c_str();
#else
				val = sPropVal.c_str();
#endif
				if (!val.IsEmpty())
				{
					nLogWidthMarker = _ttoi(val);
				}
				bFoundLogWidth = TRUE;
			}
			if ((!bFoundLogTemplate)&&(sPropName.Compare(PROJECTPROPNAME_LOGTEMPLATE)==0))
			{
#ifdef UNICODE
				sLogTemplate = MultibyteToWide((char *)sPropVal.c_str()).c_str();
#else
				sLogTemplate = sPropVal.c_str();
#endif
				sLogTemplate.Replace(_T("\r"), _T(""));
				sLogTemplate.Replace(_T("\n"), _T("\r\n"));
				bFoundLogTemplate = TRUE;
			}
		}
		if (PathIsRoot(path))
			return FALSE;
		path = path.Left(path.ReverseFind('\\'));
		if (!PathFileExists(path + _T("\\") + _T(SVN_WC_ADM_DIR_NAME)))
		{
			if (bFoundBugtraqLabel | bFoundBugtraqMessage | bFoundBugtraqNumber
				| bFoundBugtraqURL | bFoundBugtraqWarnIssue | bFoundLogWidth
				| bFoundLogTemplate)
				return TRUE;
			return FALSE;
		}
	}
	return FALSE;		//never reached
}

BOOL ProjectProperties::FindBugID(const CString& msg, CWnd * pWnd)
{
	int offset1, offset2;
	if (sUrl.IsEmpty())
		return FALSE;
	//if we have a message format, we look for that in the last line of the log
	//message only
	if (!sMessage.IsEmpty())
	{
		CString sLastLine;
		if (msg.ReverseFind('\n')>=0)
			sLastLine = msg.Mid(msg.ReverseFind('\n')+1);
		if (sMessage.Find(_T("%BUGID%"))<0)
			return FALSE;
		CString sFirstPart = sMessage.Left(sMessage.Find(_T("%BUGID%")));
		CString sLastPart = sMessage.Mid(sMessage.Find(_T("%BUGID%"))+7);
		if (sLastLine.Left(sFirstPart.GetLength()).Compare(sFirstPart)!=0)
			return FALSE;
		if (sLastLine.Right(sLastPart.GetLength()).Compare(sLastPart)!=0)
			return FALSE;
		CString sBugIDPart = sLastLine.Mid(sFirstPart.GetLength(), sLastLine.GetLength() - sFirstPart.GetLength() - sLastPart.GetLength());
		if (sBugIDPart.IsEmpty())
			return FALSE;
		//the bug id part can contain several bug id's, separated by commas
        offset1 = msg.GetLength() - sLastLine.GetLength() + sFirstPart.GetLength();
		sBugIDPart.Trim(_T(","));
		while (sBugIDPart.Find(',')>=0)
		{
			offset2 = offset1 + sBugIDPart.Find(',');
			CHARRANGE range = {offset1, offset2};
			pWnd->SendMessage(EM_EXSETSEL, NULL, (LPARAM)&range);
			CHARFORMAT2 format;
			ZeroMemory(&format, sizeof(CHARFORMAT2));
			format.cbSize = sizeof(CHARFORMAT2);
			format.dwMask = CFM_LINK;
			format.dwEffects = CFE_LINK;
			pWnd->SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&format);
			sBugIDPart = sBugIDPart.Mid(sBugIDPart.Find(',')+1);
			offset1 = offset2 + 1;
		}
		offset2 = offset1 + sBugIDPart.GetLength();
		CHARRANGE range = {offset1, offset2};
		pWnd->SendMessage(EM_EXSETSEL, NULL, (LPARAM)&range);
		CHARFORMAT2 format;
		ZeroMemory(&format, sizeof(CHARFORMAT2));
		format.cbSize = sizeof(CHARFORMAT2);
		format.dwMask = CFM_LINK;
		format.dwEffects = CFE_LINK;
		pWnd->SendMessage(EM_SETCHARFORMAT, SCF_SELECTION, (LPARAM)&format);
		return TRUE;
	}
	return FALSE;
}

CString ProjectProperties::GetBugIDUrl(const CString& msg)
{
	CString ret;
	if (sUrl.IsEmpty())
		return ret;
	if (!sMessage.IsEmpty())
	{
		ret = sUrl;
		ret.Replace(_T("%BUGID%"), msg);
	}
	return ret;
}




