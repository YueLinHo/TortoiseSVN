#pragma once

///////////////////////////////////////////////////////////////
// necessary includes
///////////////////////////////////////////////////////////////

#include "ILogQuery.h"
#include "ILogReceiver.h"

#include "RevisionInfoContainer.h"

///////////////////////////////////////////////////////////////
// forward declarations
///////////////////////////////////////////////////////////////

class SVNInfoData;
class CTSVNPath;

namespace LogCache
{
	class CLogCachePool;
	class CCachedLogInfo;
}

using namespace LogCache;

///////////////////////////////////////////////////////////////
//
// CCacheLogQuery
//
//		Implements ILogQuery on the log cache. It requires
//		another query to fill the gaps in the cache it may
//		encounter. Those will be filled on-the-fly.
//
///////////////////////////////////////////////////////////////

class CCacheLogQuery : public ILogQuery
{
private:

	class CLogFiller : private ILogReceiver
	{
	private:

		// cache to use & update

		CCachedLogInfo* cache;
		CStringA URL;

		// connection to the SVN repository

		ILogQuery* svnQuery;

		// path to log for and the begin of the current gap 
		// in the log for that path

		std::auto_ptr<CDictionaryBasedPath> currentPath;
		revision_t firstNARevision;
		bool followRenames;

		// the original log receiver (may be NULL)

		ILogReceiver* receiver;

		// implement ILogReceiver

		virtual void ReceiveLog ( const LogChangedPathArray& changes
								, svn_revnum_t rev
								, const CString& author
								, const apr_time_t& timeStamp
								, const CString& message);

	public:

		// actually call SVN
		// return the last revision sent to the receiver

		revision_t FillLog ( CCachedLogInfo* cache
						   , const CStringA& URL
						   , ILogQuery* svnQuery
						   , revision_t startRevision
						   , revision_t endRevision
						   , const CDictionaryBasedPath& startPath
						   , int limit
						   , bool strictNodeHistory
						   , ILogReceiver* receiver);
	};

	// we get our cache from here

	CLogCachePool* caches;

	// cache to use & update

	CCachedLogInfo* cache;
	CStringA URL;

	// connection to the SVN repository (may be NULL)

	ILogQuery* svnQuery;

	// Determine the revision range to pass to SVN.

	revision_t NextAvailableRevision ( const CDictionaryBasedPath& path
									 , revision_t firstMissingRevision
								     , revision_t endRevision) const;

	// ask SVN to fill the log -- at least a bit
	// Possibly, it will stop long before endRevision and limit!

	revision_t FillLog ( revision_t startRevision
					   , revision_t endRevision
					   , const CDictionaryBasedPath& startPath
					   , int limit
					   , bool strictNodeHistory
					   , ILogReceiver* receiver);

	// fill the receiver's change list buffer 

	static std::auto_ptr<LogChangedPathArray> GetChanges 
		( CRevisionInfoContainer::CChangesIterator& first
		, CRevisionInfoContainer::CChangesIterator& last);

	// crawl the history and forward it to the receiver

	void InternalLog ( revision_t startRevision
					 , revision_t endRevision
					 , const CDictionaryBasedPath& startPath
					 , int limit
					 , bool strictNodeHistory
					 , ILogReceiver* receiver);

	// follow copy history until the startRevision is reached

	CDictionaryBasedPath TranslatePegRevisionPath 
		( revision_t pegRevision
		, revision_t startRevision
		, const CDictionaryBasedPath& startPath);

	// extract the repository-relative path of the URL / file name
	// and open the cache

	CDictionaryBasedPath GetRelativeRepositoryPath (SVNInfoData& info);

	// get UUID & repository-relative path

	SVNInfoData& GetRepositoryInfo ( const CTSVNPath& path
 								   , SVNInfoData& baseInfo
								   , SVNInfoData& headInfo) const;

	// decode special revisions:
	// base / head must be initialized with NO_REVISION
	// and will be used to cache these values.

	revision_t DecodeRevision ( const CTSVNPath& path
				  			  , const SVNRev& revision
							  , SVNInfoData& baseInfo
							  , SVNInfoData& headInfo) const;

	// get the (exactly) one path from targets
	// throw an exception, if there are none or more than one

	CTSVNPath GetPath (const CTSVNPathList& targets) const;

public:

	// construction / destruction

	CCacheLogQuery (CLogCachePool* caches, ILogQuery* svnQuery);
	virtual ~CCacheLogQuery(void);

	// query a section from log for multiple paths
	// (special revisions, like "HEAD", supported)

	virtual void Log ( const CTSVNPathList& targets
					 , const SVNRev& peg_revision
					 , const SVNRev& start
					 , const SVNRev& end
					 , int limit
					 , bool strictNodeHistory
					 , ILogReceiver* receiver);
};
