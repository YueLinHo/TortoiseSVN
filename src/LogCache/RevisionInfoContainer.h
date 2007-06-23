// TortoiseSVN - a Windows shell extension for easy version control

// Copyright (C) 2007-2007 - TortoiseSVN

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
#pragma once

///////////////////////////////////////////////////////////////
// necessary includes
///////////////////////////////////////////////////////////////

#include "StringDictonary.h"
#include "PathDictionary.h"
#include "TokenizedStringContainer.h"

///////////////////////////////////////////////////////////////
// begin namespace LogCache
///////////////////////////////////////////////////////////////

namespace LogCache
{

///////////////////////////////////////////////////////////////
//
// CRevisionInfoContainer
//
//		stores all log information except the actual version
//		number. So, please note, that the indices used here
//		are not the revision number (revisons may come in
//		in arbitrary order while new information will only
//		be appended here).
//
//		Every entry holds the data for exactly one revision.
//		You may add change paths and merged revision info 
//		to the last revision, only.
//
//		Internal storage for revision "index" is as follows:
//
//			* comments[index], authors[authors[index]] and
//			  timeStamps[index] contain the values passed to 
//			  Insert()
//
//			* CDictionaryBasedPath (&paths, rootPaths[index])
//			  is the common root path for all changed paths
//			  in that revision (returns true in IsInvalid(),
//			  if there are no changed paths for this revision).
//
//			* changesOffsets[index] .. changesOffsets[index+1]-1
//			  is the range within changes and changedPaths 
//			  that contains all changes to the revision
//
//			* copyFromOffsets[index] .. copyFromOffsets[index+1]-1
//			  is the corresponding range within copyFromPaths 
//			  and copyFromRevisions 
//
//			* mergedRevisionsOffsets[index] .. mergedRevisionsOffsets[index+1]-1
//			  is the range within mergedFromPaths, mergedToPaths,
//			  mergedRangeStarts and mergedRangeDeltas that 
//			  contains the info of all merges done in this
//			  revision.	Negative mergedRangeDeltas[] denote
//			  an "undone" merge.
//
//		changes contains the TChangeAction values. If a non-
//		empty fromPath has been passed to AddChange(), "1" is
//		added to the action value. Only in that case, there
//		will be entries in copyFromPaths and copyFromRevisions.
//		(so, iterators need two differen change indices to
//		represent their current position).
//
///////////////////////////////////////////////////////////////

class CRevisionInfoContainer
{
private:

	// string pool for author names

	CStringDictionary authorPool;

	// common directory for all paths

	CPathDictionary paths;

	// comment, author and timeStamp per revision index

	CTokenizedStringContainer comments;
	std::vector<index_t> authors;
	std::vector<__time64_t> timeStamps;

	// common root of all changed paths in this revision

	std::vector<index_t> rootPaths;

	// mark the ranges that contain the changed path info

	std::vector<index_t> changesOffsets;
	std::vector<index_t> copyFromOffsets;

	// mark the ranges that contain the merged revision info

	std::vector<index_t> mergedRevisionsOffsets;

	// changed path info
	// (note, that copyFrom info will have less entries)

	std::vector<unsigned char> changes;
	std::vector<index_t> changedPaths;
	std::vector<index_t> copyFromPaths;
	std::vector<revision_t> copyFromRevisions;

	// merged revisions info
	// (mergedRangeDeltas[] < 0 -> "undo merge")

	std::vector<index_t> mergedFromPaths;
	std::vector<index_t> mergedToPaths;
	std::vector<revision_t> mergedRangeStarts;
	std::vector<revision_t> mergedRangeDeltas;

	// sub-stream IDs

	enum
	{
		AUTHOR_POOL_STREAM_ID = 1,
		COMMENTS_STREAM_ID = 2,
		PATHS_STREAM_ID = 3,
		AUTHORS_STREAM_ID = 4,
		TIMESTAMPS_STREAM_ID = 5,
		ROOTPATHS_STREAM_ID = 6,
		CHANGES_OFFSETS_STREAM_ID = 7,
		COPYFROM_OFFSETS_STREAM_ID = 8,
		CHANGES_STREAM_ID = 9,
		CHANGED_PATHS_STREAM_ID = 10,
		COPYFROM_PATHS_STREAM_ID = 11,
		COPYFROM_REVISIONS_STREAM_ID = 12,
		MERGEDREVISION_OFFSETS_STREAM_ID = 13,
		MERGED_FROM_PATHS_STREAM_ID = 14,
		MERGED_TO_PATHS_STREAM_ID = 15,
		MERGED_RANGE_STARTS_STREAM_ID = 16,
		MERGED_RANGE_DELTAS_STREAM_ID = 17
	};

	// index checking utility

	void CheckIndex (index_t index) const;

	// update / modify utilities

	void UpdateAuthors ( const CRevisionInfoContainer& newData
					   , const std::vector<index_t>& indexes
					   , index_t count);
	void UpdateTimeStamps ( const CRevisionInfoContainer& newData
						  , const std::vector<index_t>& indexes
						  , index_t count);
	void UpdateComments ( const CRevisionInfoContainer& newData
						, const std::vector<index_t>& indexes
						, index_t count);

	void UpdateChanges ( const CRevisionInfoContainer& newData
					   , const std::vector<index_t>& indexes
					   , index_t count);
	void UpdateMergers ( const CRevisionInfoContainer& newData
					   , const std::vector<index_t>& indexes
					   , index_t count);

	void Append ( const CRevisionInfoContainer& newData
				, index_t startIndex);

public:

	///////////////////////////////////////////////////////////////
	//
	// TChangeAction
	//
	//		the action IDs to use with AddChange 
	//		(actually, a bit mask).
	//
	///////////////////////////////////////////////////////////////

	enum TChangeAction
	{
		HAS_COPY_FROM   = 0x01,

		ACTION_ADDED	= 0x04,
		ACTION_CHANGED	= 0x08,
		ACTION_REPLACED	= 0x10,
		ACTION_DELETED	= 0x20,

		ANY_ACTION		= ACTION_ADDED 
						| ACTION_CHANGED
						| ACTION_REPLACED
						| ACTION_DELETED
	};

	///////////////////////////////////////////////////////////////
	//
	// CChangesIterator
	//
	//		a very simplistic forward iterator class.
	//		It will be used to provide a convenient
	//		interface to a revision's changes list.
	//
	///////////////////////////////////////////////////////////////

	class CChangesIterator
	{
	private:

		// the container we operate on 

		const CRevisionInfoContainer* container;

		// the two different change info indices

		index_t changeOffset;
		index_t copyFromOffset;

	public:

		// construction

		CChangesIterator();
		CChangesIterator ( const CRevisionInfoContainer* aContainer
						 , index_t aChangeOffset
						 , index_t aCopyFromOffset);

		// data access

		CRevisionInfoContainer::TChangeAction GetAction() const;
		CDictionaryBasedPath GetPath() const;

		bool HasFromPath() const;
		CDictionaryBasedPath GetFromPath() const;
		revision_t GetFromRevision() const;

		// general status (points to an action)

		bool IsValid() const;

		// move pointer

		CChangesIterator& operator++();		// prefix
		CChangesIterator operator++(int);	// postfix

		// comparison

		bool operator== (const CChangesIterator& rhs);
		bool operator!= (const CChangesIterator& rhs);

		// pointer-like behavior

		const CChangesIterator* operator->() const;
	};

	friend class CChangesIterator;

	///////////////////////////////////////////////////////////////
	//
	// CMergedRevisionsIterator
	//
	//		a very simplistic forward iterator class.
	//		It will be used to provide a convenient
	//		interface to a revision's merged revision info.
	//
	///////////////////////////////////////////////////////////////

	class CMergedRevisionsIterator
	{
	private:

		// the container we operate on 

		const CRevisionInfoContainer* container;

		// the merged revision info index

		index_t offset;

	public:

		// construction

		CMergedRevisionsIterator();
		CMergedRevisionsIterator ( const CRevisionInfoContainer* container
								 , index_t offset);

		// data access

		CDictionaryBasedPath GetFromPath() const;
		CDictionaryBasedPath GetToPath() const;

		revision_t GetRangeStart() const;
		revision_t GetRangeDelta() const;

		// general status (points to an action)

		bool IsValid() const;

		// move pointer

		CMergedRevisionsIterator& operator++();		// prefix
		CMergedRevisionsIterator operator++(int);	// postfix

		// comparison

		bool operator== (const CMergedRevisionsIterator& rhs);
		bool operator!= (const CMergedRevisionsIterator& rhs);

		// pointer-like behavior

		const CMergedRevisionsIterator* operator->() const;
	};

	friend class CMergedRevisionsIterator;

	// construction / destruction

	CRevisionInfoContainer(void);
	~CRevisionInfoContainer(void);

	// add information
	// AddChange() and AddMergedRevision() always adds to the last revision

	index_t Insert ( const std::string& author
				   , const std::string& comment
				   , __time64_t timeStamp);

	void AddChange ( TChangeAction action
				   , const std::string& path
				   , const std::string& fromPath
				   , revision_t fromRevision);

	void AddMergedRevision ( const std::string& fromPath
				           , const std::string& toPath
				           , revision_t revisionStart
				           , revision_t revisionDelta);

	// reset content

	void Clear();

	// get information

	index_t size() const;

	const char* GetAuthor (index_t index) const;
	__time64_t GetTimeStamp (index_t index) const;
	std::string GetComment (index_t index) const;

	index_t GetAuthorID (index_t index) const;
	CDictionaryBasedPath GetRootPath (index_t index) const;

	// iterate over all changes

	CChangesIterator GetChangesBegin (index_t index) const;
	CChangesIterator GetChangesEnd (index_t index) const;

	// iterate over all merged revisions

	CMergedRevisionsIterator GetMergedRevisionsBegin (index_t index) const;
	CMergedRevisionsIterator GetMergedRevisionsEnd (index_t index) const;

	// r/o access to internal pools

	const CStringDictionary& GetAuthors() const;
	const CPathDictionary& GetPaths() const;
	const CTokenizedStringContainer& GetComments() const;

	// update / modify existing data
	// indexes must be in ascending order
	// indexes[] may be size() -> results in an Append()

	void Update ( const CRevisionInfoContainer& newData
				, const std::vector<index_t>& indexes
				, bool updateAuthors
				, bool updateTimeStamps
				, bool updateComments
				, bool updateChanges
				, bool updateMergers);

	// stream I/O

	friend IHierarchicalInStream& operator>> ( IHierarchicalInStream& stream
											 , CRevisionInfoContainer& container);
	friend IHierarchicalOutStream& operator<< ( IHierarchicalOutStream& stream
											  , const CRevisionInfoContainer& container);
};

///////////////////////////////////////////////////////////////
// inlines
///////////////////////////////////////////////////////////////
// CMergedRevisionsIterator
///////////////////////////////////////////////////////////////
// construction
///////////////////////////////////////////////////////////////

inline CRevisionInfoContainer::CChangesIterator::CChangesIterator()
	: container (NULL)
	, changeOffset(0)
	, copyFromOffset(0)
{
}

inline CRevisionInfoContainer::CChangesIterator::CChangesIterator 
	( const CRevisionInfoContainer* aContainer
	, index_t aChangeOffset
	, index_t aCopyFromOffset)
	: container (aContainer)
	, changeOffset (aChangeOffset)
	, copyFromOffset (aCopyFromOffset)
{
}

///////////////////////////////////////////////////////////////
// data access
///////////////////////////////////////////////////////////////

inline CRevisionInfoContainer::TChangeAction 
CRevisionInfoContainer::CChangesIterator::GetAction() const
{
	assert (IsValid());
	int action = container->changes[changeOffset] & ANY_ACTION;
	return (CRevisionInfoContainer::TChangeAction)(action);
}

inline bool 
CRevisionInfoContainer::CChangesIterator::HasFromPath() const
{
	assert (IsValid());
	return (container->changes[changeOffset] & HAS_COPY_FROM) != 0;
}

inline CDictionaryBasedPath 
CRevisionInfoContainer::CChangesIterator::GetPath() const
{
	assert (IsValid());
	index_t pathID = container->changedPaths[changeOffset];
	return CDictionaryBasedPath (&container->paths, pathID);
}

inline CDictionaryBasedPath 
CRevisionInfoContainer::CChangesIterator::GetFromPath() const
{
	assert (HasFromPath());
	index_t pathID = container->copyFromPaths [copyFromOffset];
	return CDictionaryBasedPath (&container->paths, pathID);
}

inline revision_t 
CRevisionInfoContainer::CChangesIterator::GetFromRevision() const
{
	assert (HasFromPath());
	return container->copyFromRevisions [copyFromOffset];
}

///////////////////////////////////////////////////////////////
// general status (points to an action)
///////////////////////////////////////////////////////////////

inline bool CRevisionInfoContainer::CChangesIterator::IsValid() const
{
	return (container != NULL)
		&& (changeOffset < (index_t)container->changes.size())
		&& (copyFromOffset <= (index_t)container->copyFromPaths.size());
}

///////////////////////////////////////////////////////////////
// move pointer
///////////////////////////////////////////////////////////////

inline CRevisionInfoContainer::CChangesIterator& 
CRevisionInfoContainer::CChangesIterator::operator++()		// prefix
{
	if (HasFromPath())
		++copyFromOffset;
	++changeOffset;

	return *this;
}

inline CRevisionInfoContainer::CChangesIterator 
CRevisionInfoContainer::CChangesIterator::operator++(int)	// postfix
{
	CChangesIterator result (*this);
	operator++();
	return result;
}

///////////////////////////////////////////////////////////////
// comparison
///////////////////////////////////////////////////////////////

inline bool CRevisionInfoContainer::CChangesIterator::operator== 
	(const CChangesIterator& rhs)
{
	return (container == rhs.container)
		&& (changeOffset == rhs.changeOffset);
}

inline bool CRevisionInfoContainer::CChangesIterator::operator!= 
	(const CChangesIterator& rhs)
{
	return !operator==(rhs);
}

///////////////////////////////////////////////////////////////
// pointer-like behavior
///////////////////////////////////////////////////////////////

inline const CRevisionInfoContainer::CChangesIterator* 
CRevisionInfoContainer::CChangesIterator::operator->() const
{
	return this;
}

///////////////////////////////////////////////////////////////
// CMergedRevisionsIterator
///////////////////////////////////////////////////////////////
// construction
///////////////////////////////////////////////////////////////

inline CRevisionInfoContainer::CMergedRevisionsIterator::CMergedRevisionsIterator()
	: container (NULL)
	, offset(0)
{
}

inline CRevisionInfoContainer::CMergedRevisionsIterator::CMergedRevisionsIterator 
	( const CRevisionInfoContainer* container
	, index_t offset)
	: container (container)
	, offset (offset)
{
}

///////////////////////////////////////////////////////////////
// data access
///////////////////////////////////////////////////////////////

inline CDictionaryBasedPath 
CRevisionInfoContainer::CMergedRevisionsIterator::GetFromPath() const
{
	index_t pathID = container->mergedFromPaths [offset];
	return CDictionaryBasedPath (&container->paths, pathID);
}

inline CDictionaryBasedPath 
CRevisionInfoContainer::CMergedRevisionsIterator::GetToPath() const
{
	index_t pathID = container->mergedToPaths [offset];
	return CDictionaryBasedPath (&container->paths, pathID);
}

inline revision_t 
CRevisionInfoContainer::CMergedRevisionsIterator::GetRangeStart() const
{
	return container->mergedRangeStarts [offset];
}

inline revision_t 
CRevisionInfoContainer::CMergedRevisionsIterator::GetRangeDelta() const
{
	return container->mergedRangeDeltas [offset];
}

///////////////////////////////////////////////////////////////
// general status (points to an action)
///////////////////////////////////////////////////////////////

inline bool 
CRevisionInfoContainer::CMergedRevisionsIterator::IsValid() const
{
	return (container != NULL)
		&& (offset < (index_t)container->mergedFromPaths.size());
}

///////////////////////////////////////////////////////////////
// move pointer
///////////////////////////////////////////////////////////////

inline CRevisionInfoContainer::CMergedRevisionsIterator& 
CRevisionInfoContainer::CMergedRevisionsIterator::operator++()		// prefix
{
	++offset;
	return *this;
}

inline CRevisionInfoContainer::CMergedRevisionsIterator 
CRevisionInfoContainer::CMergedRevisionsIterator::operator++(int)	// postfix
{
	CMergedRevisionsIterator result (*this);
	++offset;
	return result;
}

///////////////////////////////////////////////////////////////
// comparison
///////////////////////////////////////////////////////////////

inline bool CRevisionInfoContainer::CMergedRevisionsIterator::operator== 
	(const CMergedRevisionsIterator& rhs)
{
	return (container == rhs.container)
		&& (offset == rhs.offset);
}

inline bool CRevisionInfoContainer::CMergedRevisionsIterator::operator!= 
	(const CMergedRevisionsIterator& rhs)
{
	return !operator==(rhs);
}

///////////////////////////////////////////////////////////////
// pointer-like behavior
///////////////////////////////////////////////////////////////

inline const CRevisionInfoContainer::CMergedRevisionsIterator* 
CRevisionInfoContainer::CMergedRevisionsIterator::operator->() const
{
	return this;
}

///////////////////////////////////////////////////////////////
// CRevisionInfoContainer
///////////////////////////////////////////////////////////////
// index checking utility
///////////////////////////////////////////////////////////////

inline void CRevisionInfoContainer::CheckIndex (index_t index) const
{
	if (index >= (index_t)size())
		throw std::exception ("revision info index out of range");
}

///////////////////////////////////////////////////////////////
// get information
///////////////////////////////////////////////////////////////

inline index_t CRevisionInfoContainer::size() const
{
	return (index_t)authors.size();
}

inline index_t CRevisionInfoContainer::GetAuthorID (index_t index) const
{
	CheckIndex (index);
	return authors [index];
}

inline const char* CRevisionInfoContainer::GetAuthor (index_t index) const
{
	CheckIndex (index);
	return authorPool [authors [index]];
}

inline __time64_t CRevisionInfoContainer::GetTimeStamp (index_t index) const
{
	CheckIndex (index);
	return timeStamps [index];
}

inline std::string CRevisionInfoContainer::GetComment (index_t index) const
{
	CheckIndex (index);
	return comments [index];
}

inline CDictionaryBasedPath 
CRevisionInfoContainer::GetRootPath (index_t index) const
{
	CheckIndex (index);
	return CDictionaryBasedPath (&paths, rootPaths [index]);
}

// iterate over all changes

inline CRevisionInfoContainer::CChangesIterator 
CRevisionInfoContainer::GetChangesBegin (index_t index) const
{
	CheckIndex (index);
	return CChangesIterator ( this
							, changesOffsets[index]
							, copyFromOffsets[index]);
}

inline CRevisionInfoContainer::CChangesIterator 
CRevisionInfoContainer::GetChangesEnd (index_t index) const
{
	CheckIndex (index);
	return CChangesIterator ( this
							, changesOffsets[index+1]
							, copyFromOffsets[index+1]);
}

// iterate over all merged revisions

inline CRevisionInfoContainer::CMergedRevisionsIterator 
CRevisionInfoContainer::GetMergedRevisionsBegin (index_t index) const
{
	CheckIndex (index);
	return CMergedRevisionsIterator ( this
									, mergedRevisionsOffsets[index]);
}

inline CRevisionInfoContainer::CMergedRevisionsIterator 
CRevisionInfoContainer::GetMergedRevisionsEnd (index_t index) const
{
	CheckIndex (index);
	return CMergedRevisionsIterator ( this
									, mergedRevisionsOffsets[index+1]);
}

// r/o access to internal pools

inline const CStringDictionary& CRevisionInfoContainer::GetAuthors() const
{
	return authorPool;
}

inline const CPathDictionary& CRevisionInfoContainer::GetPaths() const
{
	return paths;
}

inline const CTokenizedStringContainer& CRevisionInfoContainer::GetComments() const
{
	return comments;
}

///////////////////////////////////////////////////////////////
// stream I/O
///////////////////////////////////////////////////////////////

IHierarchicalInStream& operator>> ( IHierarchicalInStream& stream
								  , CRevisionInfoContainer& container);
IHierarchicalOutStream& operator<< ( IHierarchicalOutStream& stream
								   , const CRevisionInfoContainer& container);

///////////////////////////////////////////////////////////////
// end namespace LogCache
///////////////////////////////////////////////////////////////

}


