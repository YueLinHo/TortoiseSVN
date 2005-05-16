#include "StdAfx.h"
#include ".\cacheddirectory.h"
#include "SVNHelpers.h"
#include "SVNStatusCache.h"
#include "SVNStatus.h"

CCachedDirectory::CCachedDirectory(void)
{
	m_entriesFileTime = 0;
	m_propsDirTime = 0;
	m_currentFullStatus = m_mostImportantFileStatus = svn_wc_status_none;
	m_bCurrentFullStatusValid = false;
}

CCachedDirectory::~CCachedDirectory(void)
{
	AutoLocker lock(m_critSec);
	
}

CCachedDirectory::CCachedDirectory(const CTSVNPath& directoryPath)
{
	ATLASSERT(directoryPath.IsDirectory() || !PathFileExists(directoryPath.GetWinPath()));

	m_directoryPath = directoryPath;
	m_entriesFileTime = 0;
	m_propsDirTime = 0;
	m_currentFullStatus = m_mostImportantFileStatus = svn_wc_status_none;
	m_bCurrentFullStatusValid = false;
}

BOOL CCachedDirectory::SaveToDisk(HANDLE hFile)
{
	AutoLocker lock(m_critSec);
#define WRITEVALUETOFILE(x) if (!WriteFile(hFile, &x, sizeof(x), &written, NULL)) return false;
	DWORD written = 0;
	int value = 0;
	WRITEVALUETOFILE(value);	// 'version' of this save-format
	value = (int)m_entryCache.size();
	WRITEVALUETOFILE(value);	// size of the cache map
	// now iterate through the maps and save every entry.
	for (CacheEntryMap::iterator I = m_entryCache.begin(); I != m_entryCache.end(); ++I)
	{
		CString key = I->first;
		value = key.GetLength();
		WRITEVALUETOFILE(value);
		if (value)
		{
			if (!WriteFile(hFile, key, value*sizeof(TCHAR), &written, NULL))
				return false;
			if (!I->second.SaveToDisk(hFile))
				return false;
		}
	}
	value = (int)m_childDirectories.size();
	WRITEVALUETOFILE(value);
	for (ChildDirStatus::iterator I = m_childDirectories.begin(); I != m_childDirectories.end(); ++I)
	{
		CString path = I->first.GetWinPathString();
		value = path.GetLength();
		WRITEVALUETOFILE(value);
		if (value)
		{
			if (!WriteFile(hFile, path, value*sizeof(TCHAR), &written, NULL))
				return false;
			svn_wc_status_kind status = I->second;
			WRITEVALUETOFILE(status);
		}
	}
	WRITEVALUETOFILE(m_entriesFileTime);
	WRITEVALUETOFILE(m_propsDirTime);
	value = m_directoryPath.GetWinPathString().GetLength();
	WRITEVALUETOFILE(value);
	if (value)
	{
		if (!WriteFile(hFile, m_directoryPath.GetWinPathString(), value*sizeof(TCHAR), &written, NULL))
			return false;
	}
	if (!m_ownStatus.SaveToDisk(hFile))
		return false;
	WRITEVALUETOFILE(m_currentFullStatus);
	WRITEVALUETOFILE(m_bCurrentFullStatusValid);
	WRITEVALUETOFILE(m_mostImportantFileStatus);
	return true;
}

BOOL CCachedDirectory::LoadFromDisk(HANDLE hFile)
{
	AutoLocker lock(m_critSec);
#define LOADVALUEFROMFILE(x) if (!ReadFile(hFile, &x, sizeof(x), &read, NULL)) return false;
	DWORD read = 0;
	int value = 0;
	LOADVALUEFROMFILE(value);
	if (value != 0)
		return false;		// not the correct version
	int mapsize = 0;
	LOADVALUEFROMFILE(mapsize);
	for (int i=0; i<mapsize; ++i)
	{
		LOADVALUEFROMFILE(value);
		if (value)
		{
			CString sKey;
			if (!ReadFile(hFile, sKey.GetBuffer(value), value*sizeof(TCHAR), &read, NULL))
			{
				sKey.ReleaseBuffer(0);
				return false;
			}
			sKey.ReleaseBuffer(value);
			CStatusCacheEntry entry;
			if (!entry.LoadFromDisk(hFile))
				return false;
			m_entryCache[sKey] = entry;
		}
	}
	LOADVALUEFROMFILE(mapsize);
	for (int i=0; i<mapsize; ++i)
	{
		LOADVALUEFROMFILE(value);
		if (value)
		{
			CString sPath;
			if (!ReadFile(hFile, sPath.GetBuffer(value), value*sizeof(TCHAR), &read, NULL))
			{
				sPath.ReleaseBuffer(0);
				return false;
			}
			sPath.ReleaseBuffer(value);
			svn_wc_status_kind status;
			LOADVALUEFROMFILE(status);
			m_childDirectories[CTSVNPath(sPath)] = status;
		}
	}
	LOADVALUEFROMFILE(m_entriesFileTime);
	LOADVALUEFROMFILE(m_propsDirTime);
	LOADVALUEFROMFILE(value);
	if (value)
	{
		CString sPath;
		if (!ReadFile(hFile, sPath.GetBuffer(value), value*sizeof(TCHAR), &read, NULL))
		{
			sPath.ReleaseBuffer(0);
			return false;
		}
		sPath.ReleaseBuffer(value);
		m_directoryPath.SetFromWin(sPath);
	}
	if (!m_ownStatus.LoadFromDisk(hFile))
		return false;

	LOADVALUEFROMFILE(m_currentFullStatus);
	LOADVALUEFROMFILE(m_bCurrentFullStatusValid);
	LOADVALUEFROMFILE(m_mostImportantFileStatus);
	return true;

}

CStatusCacheEntry CCachedDirectory::GetStatusForMember(const CTSVNPath& path, bool bRecursive)
{
	CString strCacheKey;
	bool bThisDirectoryIsUnversioned = false;
	bool bRequestForSelf = false;
	if(path.IsEquivalentTo(m_directoryPath))
	{
		bRequestForSelf = true;
	}

	// In all most circumstances, we ask for the status of a member of this directory.
	ATLASSERT(m_directoryPath.IsEquivalentTo(path.GetContainingDirectory()) || bRequestForSelf);

	// Check if the entries file has been changed
	CTSVNPath entriesFilePath(m_directoryPath);
	entriesFilePath.AppendPathString(_T(SVN_WC_ADM_DIR_NAME) _T("\\entries"));
	CTSVNPath propsDirPath(m_directoryPath);
	propsDirPath.AppendPathString(_T(SVN_WC_ADM_DIR_NAME) _T("\\props"));
	if(m_entriesFileTime == entriesFilePath.GetLastWriteTime() && m_propsDirTime == propsDirPath.GetLastWriteTime())
	{
		if(m_entriesFileTime == 0)
		{
			// We are a folder which is not in a working copy
			bThisDirectoryIsUnversioned = true;
			m_ownStatus.SetStatus(NULL);

			// We should never have anything in our file item cache if we're unversioned
			ATLASSERT(m_entryCache.empty());
			
			// However, a member *DIRECTORY* might be the top of WC
			// so we need to ask them to get their own status
			if(!path.IsDirectory())
			{
				if ((PathFileExists(path.GetWinPath()))||(bRequestForSelf))
					return CStatusCacheEntry();
				// the entry doesn't exist anymore! 
				// Let the crawler remove it from the cache.
				CSVNStatusCache::Instance().AddFolderForCrawling(path);
				return CStatusCacheEntry();
			}
			else
			{
				// If we're in the special case of a directory being asked for its own status
				// and this directory is unversioned, then we should just return that here
				if(bRequestForSelf)
				{
					return CStatusCacheEntry();
				}
			}
		}

		if(path.IsDirectory())
		{
			// We don't have directory status in our cache
			// Ask the directory if it knows its own status
			CCachedDirectory * dirEntry = CSVNStatusCache::Instance().GetDirectoryCacheEntry(path);
			if(dirEntry->IsOwnStatusValid())
			{
				// This directory knows its own status
				// but is it still versioned?
				
				CTSVNPath svnFilePath(dirEntry->m_directoryPath);
				if (!svnFilePath.HasAdminDir())
				{
					CSVNStatusCache::Instance().AddFolderForCrawling(svnFilePath);
					return CStatusCacheEntry();
				}

				// To keep recursive status up to date, we'll request that children are all crawled again
				// This will be very quick if nothing's changed, because it will all be cache hits
				AutoLocker lock(m_critSec);
				ChildDirStatus::const_iterator it;
				for(it = dirEntry->m_childDirectories.begin(); it != dirEntry->m_childDirectories.end(); ++it)
				{
					CTSVNPath childPath = it->first;
					CSVNStatusCache::Instance().AddFolderForCrawling(it->first);
				}

				return dirEntry->GetOwnStatus(bRecursive);
			}
		}
		else
		{
			// Look up a file in our own cache
			AutoLocker lock(m_critSec);
			strCacheKey = GetCacheKey(path);
			CacheEntryMap::iterator itMap = m_entryCache.find(strCacheKey);
			if(itMap != m_entryCache.end())
			{
				// We've hit the cache - check for timeout
				if(!itMap->second.HasExpired((long)GetTickCount()))
				{
					if(itMap->second.DoesFileTimeMatch(path.GetLastWriteTime()))
					{
						// Note: the filetime matches after a modified has been committed too.
						// So in that case, we would return a wrong status (e.g. 'modified' instead
						// of 'normal') here.
						return itMap->second;
					}
				}
			}
		}
	}
	else
	{
		AutoLocker lock(m_critSec);
		m_entriesFileTime = entriesFilePath.GetLastWriteTime();
		m_propsDirTime = propsDirPath.GetLastWriteTime();
		m_entryCache.clear();
		strCacheKey = GetCacheKey(path);
	}

	svn_opt_revision_t revision;
	revision.kind = svn_opt_revision_unspecified;

	// We've not got this item in the cache - let's add it
	// We never bother asking SVN for the status of just one file, always for its containing directory

	if(path.IsDirectory() && path.GetFileOrDirectoryName().CompareNoCase(_T(SVN_WC_ADM_DIR_NAME)) == 0)
	{
		// We're being asked for the status of an .SVN directory
		// It's not worth asking for this
		AddEntry(path, NULL);
		return CStatusCacheEntry();
	}

	{
		AutoLocker lock(m_critSec);
		CSVNStatusCache& mainCache = CSVNStatusCache::Instance();
		SVNPool subPool(mainCache.m_svnHelp.Pool());
		m_mostImportantFileStatus = svn_wc_status_none;
		m_childDirectories.clear();
		m_entryCache.clear();
		m_bCurrentFullStatusValid = false;
		m_ownStatus.SetStatus(NULL);

		if(!bThisDirectoryIsUnversioned)
		{
			ATLTRACE("svn_cli_stat for '%ws' (req %ws)\n", m_directoryPath.GetWinPath(), path.GetWinPath());
			svn_error_t* pErr = svn_client_status2 (
				NULL,
				m_directoryPath.GetSVNApiPath(),
				&revision,
				GetStatusCallback,
				this,
				FALSE,
				TRUE,									//getall
				FALSE,
				TRUE,									//noignore
				FALSE,									//ignore externals
				mainCache.m_svnHelp.ClientContext(),
				subPool
				);

			if(pErr)
			{
				// Handle an error
				// The most likely error on a folder is that it's not part of a WC
				// In most circumstances, this will have been caught earlier,
				// but in some situations, we'll get this error.
				// If we allow ourselves to fall on through, then folders will be asked
				// for their own status, and will set themselves as unversioned, for the 
				// benefit of future requests
				ATLTRACE("svn_cli_stat err: '%s'\n", pErr->message);
				svn_error_clear(pErr);
				// No assert here! Since we _can_ get here, an assertion is not an option!
				// Reasons to get here: 
				// - renaming a folder with many subfolders --> results in "not a working copy" if the revert
				//   happens between our checks and the svn_client_status() call.
				// - reverting a move/copy --> results in "not a working copy" (as above)
				m_currentFullStatus = m_mostImportantFileStatus = svn_wc_status_none;
				return CStatusCacheEntry();
			}
		}
		else
		{
			ATLTRACE("Skipped SVN status for unversioned folder\n");
		}
	}
	// Now that we've refreshed our SVN status, we can see if it's 
	// changed the 'most important' status value for this directory.
	// If it has, then we should tell our parent
	UpdateCurrentStatus();

	if (path.IsDirectory())
	{
		CCachedDirectory * dirEntry = CSVNStatusCache::Instance().GetDirectoryCacheEntry(path);
		if(dirEntry->IsOwnStatusValid())
		{
			CSVNStatusCache::Instance().AddFolderForCrawling(path);
			return dirEntry->GetOwnStatus(bRecursive);
		}

		// If the status *still* isn't valid here, it means that 
		// the current directory is unversioned, and we shall need to ask its children for info about themselves
		return dirEntry->GetStatusForMember(path,bRecursive);
	}
	else
	{
		CacheEntryMap::iterator itMap = m_entryCache.find(strCacheKey);
		if(itMap != m_entryCache.end())
		{
			return itMap->second;
		}
	}

	AddEntry(path, NULL);
	return CStatusCacheEntry();
}

void 
CCachedDirectory::AddEntry(const CTSVNPath& path, const svn_wc_status2_t* pSVNStatus)
{
	AutoLocker lock(m_critSec);
	if(path.IsDirectory())
	{
		CCachedDirectory * childDir = CSVNStatusCache::Instance().GetDirectoryCacheEntry(path);
		childDir->m_ownStatus.SetStatus(pSVNStatus);
		m_bCurrentFullStatusValid = false;
	}
	else
	{
		m_entryCache[GetCacheKey(path)] = CStatusCacheEntry(pSVNStatus, path.GetLastWriteTime(), path.IsReadOnly());
	}
}


CString 
CCachedDirectory::GetCacheKey(const CTSVNPath& path)
{
	// All we put into the cache as a key is just the end portion of the pathname
	// There's no point storing the path of the containing directory for every item
	return path.GetWinPathString().Mid(m_directoryPath.GetWinPathString().GetLength()).MakeLower();
}

CString 
CCachedDirectory::GetFullPathString(const CString& cacheKey)
{
	return m_directoryPath.GetWinPathString() + _T("\\") + cacheKey;
}

void CCachedDirectory::GetStatusCallback(void *baton, const char *path, svn_wc_status2_t *status)
{
	CCachedDirectory* pThis = (CCachedDirectory*)baton;

	if (path == NULL)
		return;
		
	CTSVNPath svnPath;

	if(status->entry)
	{
		svnPath.SetFromSVN(path, (status->entry->kind == svn_node_dir));

		if(svnPath.IsDirectory())
		{
			if(!svnPath.IsEquivalentTo(pThis->m_directoryPath))
			{
				// Add any versioned directory, which is not our 'self' entry, to the list for having its status updated
				CSVNStatusCache::Instance().AddFolderForCrawling(svnPath);

				// Make sure we know about this child directory
				// This initial status value is likely to be overwritten from below at some point
				pThis->m_childDirectories[svnPath] = SVNStatus::GetMoreImportant(status->text_status, status->prop_status);
			}
		}
		else
		{
			// Keep track of the most important status of all the files in this directory
			// Don't include subdirectories in this figure, because they need to provide their 
			// own 'most important' value
			pThis->m_mostImportantFileStatus = SVNStatus::GetMoreImportant(pThis->m_mostImportantFileStatus, status->text_status);
			pThis->m_mostImportantFileStatus = SVNStatus::GetMoreImportant(pThis->m_mostImportantFileStatus, status->prop_status);
		}
	}
	else
	{
		svnPath.SetFromSVN(path);
		// Subversion returns no 'entry' field for versioned folders if they're
		// part of another working copy (nested layouts).
		// So we have to make sure that such an 'unversioned' folder really
		// is unversioned.
		if ((status->text_status == svn_wc_status_unversioned)&&(!svnPath.IsEquivalentTo(pThis->m_directoryPath))&&(svnPath.IsDirectory()))
		{
			if (svnPath.HasAdminDir())
			{
				CSVNStatusCache::Instance().AddFolderForCrawling(svnPath);
				// Mark the directory as 'versioned' (status 'normal' for now).
				// This initial value will be overwritten from below some time later
				pThis->m_childDirectories[svnPath] = svn_wc_status_normal;
				// also mark the status in the status object as normal
				status->text_status = svn_wc_status_normal;
			}
		}
		if (status->text_status == svn_wc_status_external)
		{
			CSVNStatusCache::Instance().AddFolderForCrawling(svnPath);
			// Mark the directory as 'versioned' (status 'normal' for now).
			// This initial value will be overwritten from below some time later
			pThis->m_childDirectories[svnPath] = svn_wc_status_normal;
			// also mark the status in the status object as normal
			status->text_status = svn_wc_status_normal;
		}
	}

	pThis->AddEntry(svnPath, status);
}

bool 
CCachedDirectory::IsOwnStatusValid() const
{
	return m_ownStatus.HasBeenSet() && 
		   !m_ownStatus.HasExpired(GetTickCount()) &&
		   // 'external' isn't a valid status. That just
		   // means the folder is not part of the current working
		   // copy but it still has its own 'real' status
		   m_ownStatus.GetEffectiveStatus()!=svn_wc_status_external;
}

svn_wc_status_kind CCachedDirectory::CalculateRecursiveStatus() const
{
	// Combine our OWN folder status with the most important of our *FILES'* status.
	svn_wc_status_kind retVal = SVNStatus::GetMoreImportant(m_mostImportantFileStatus, m_ownStatus.GetEffectiveStatus());

	if ((retVal != svn_wc_status_modified)&&(retVal != m_ownStatus.GetEffectiveStatus()))
	{
		if ((retVal == svn_wc_status_added)||(retVal == svn_wc_status_deleted)||(retVal == svn_wc_status_missing))
			retVal = svn_wc_status_modified;
	}

	// Now combine all our child-directorie's status
	
	ChildDirStatus::const_iterator it;
	for(it = m_childDirectories.begin(); it != m_childDirectories.end(); ++it)
	{
		retVal = SVNStatus::GetMoreImportant(retVal, it->second);
		if ((retVal != svn_wc_status_modified)&&(retVal != m_ownStatus.GetEffectiveStatus()))
		{
			if ((retVal == svn_wc_status_added)||(retVal == svn_wc_status_deleted)||(retVal == svn_wc_status_missing))
				retVal = svn_wc_status_modified;
		}
	}
	
	return retVal;
}

// Update our composite status and deal with things if it's changed
void CCachedDirectory::UpdateCurrentStatus()
{
	svn_wc_status_kind newStatus = CalculateRecursiveStatus();


	// Our status has changed - tell the shell
	if ((newStatus != m_currentFullStatus)&&(m_currentFullStatus != svn_wc_status_none))
	{
		ATLTRACE("Dir %ws, status change to %d\n", m_directoryPath.GetWinPath(), newStatus);		
		CSVNStatusCache::Instance().UpdateShell(m_directoryPath);
	}
	m_currentFullStatus = newStatus;

	// And tell our parent, if we've got one...
	CTSVNPath parentPath = m_directoryPath.GetContainingDirectory();
	if(!parentPath.IsEmpty())
	{
		// We have a parent
		CSVNStatusCache::Instance().GetDirectoryCacheEntry(parentPath)->UpdateChildDirectoryStatus(m_directoryPath, m_currentFullStatus);
	}
	m_bCurrentFullStatusValid = true;
}


// Receive a notification from a child that its status has changed
void CCachedDirectory::UpdateChildDirectoryStatus(const CTSVNPath& childDir, svn_wc_status_kind childStatus)
{
	svn_wc_status_kind currentStatus = m_childDirectories[childDir];
	if ((currentStatus != childStatus)||(!IsOwnStatusValid()))
	{
		{
			AutoLocker lock(m_critSec);
			m_childDirectories[childDir] = childStatus;
		}
		UpdateCurrentStatus();
	}
}

CStatusCacheEntry CCachedDirectory::GetOwnStatus(bool bRecursive)
{
	// Don't return recursive status if we're unversioned ourselves.
	if(bRecursive && m_ownStatus.GetEffectiveStatus() > svn_wc_status_unversioned)
	{
		CStatusCacheEntry recursiveStatus(m_ownStatus);
		if(!m_bCurrentFullStatusValid)
		{
			UpdateCurrentStatus();
		}
		recursiveStatus.ForceStatus(m_currentFullStatus);
		return recursiveStatus;				
	}
	else
	{
		return m_ownStatus;
	}
}

void CCachedDirectory::RefreshStatus()
{
	// Make sure that our own status is up-to-date
	GetStatusForMember(m_directoryPath,true);

	// We also need to check if all our file members have the right date on them
	CacheEntryMap::iterator itMembers;
	DWORD now = GetTickCount();
	
	AutoLocker lock(m_critSec);
	for(itMembers = m_entryCache.begin(); itMembers != m_entryCache.end(); ++itMembers)
	{
		if (itMembers->first)
		{
			CTSVNPath filePath(m_directoryPath);
			filePath.AppendPathString(itMembers->first);
			if (!filePath.IsEquivalentTo(m_directoryPath))
			{
				if ((itMembers->second.HasExpired(now))||(!itMembers->second.DoesFileTimeMatch(filePath.GetLastWriteTime())))
				{
					// We need to request this item as well
					GetStatusForMember(filePath,true);
					// GetStatusForMember now has recreated the m_entryCache map.
					// So we have to get out of this for-loop since the iterator now
					// is invalid!
					break;
				}
				else if (itMembers->second.IsDirectory())
				{
					// crawl all subfolders too! Otherwise a change deep inside the
					// tree which has changed won't get propagated up the tree.
					CSVNStatusCache::Instance().AddFolderForCrawling(filePath);
				}
			}
		}
	}
}
