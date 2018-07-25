// Copyright (C) 2011 Seth A. Robinson
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_PROTON_READER_H_INCLUDED__
#define __C_PROTON_READER_H_INCLUDED__

#include "IrrCompileConfig.h"

#ifdef __IRR_COMPILE_WITH_PROTON_ARCHIVE_LOADER_

#include "IReadFile.h"
#include "irrArray.h"
#include "irrString.h"
#include "IFileSystem.h"
#include "CFileList.h"

namespace irr
{
namespace io
{
	
	//! Archiveloader capable of loading Proton Archives
	class CArchiveLoaderProton : public IArchiveLoader
	{
	public:

		//! Constructor
		CArchiveLoaderProton(io::IFileSystem* fs);

		//! returns true if the file maybe is able to be loaded by this class
		//! based on the file extension (e.g. ".Proton")
		virtual bool isALoadableFileFormat(const io::path& filename) const;

		//! Check if the file might be loaded by this class
		/** Check might look into the file.
		\param file File handle to check.
		\return True if file seems to be loadable. */
		virtual bool isALoadableFileFormat(io::IReadFile* file) const;

		//! Check to see if the loader can create archives of this type.
		/** Check based on the archive type.
		\param fileType The archive type to check.
		\return True if the archile loader supports this type, false if not */
		virtual bool isALoadableFileFormat(E_FILE_ARCHIVE_TYPE fileType) const;

		//! Creates an archive from the filename
		/** \param file File handle to check.
		\return Pointer to newly created archive, or 0 upon error. */
		virtual IFileArchive* createArchive(const io::path& filename, bool ignoreCase, bool ignorePaths) const;

		//! creates/loads an archive from the file.
		//! \return Pointer to the created archive. Returns 0 if loading failed.
		virtual io::IFileArchive* createArchive(io::IReadFile* file, bool ignoreCase, bool ignorePaths) const;

	private:
		io::IFileSystem* FileSystem;
	};

/*!
	Proton file Reader written by Seth A. Robinson
*/
	class CProtonReader : public virtual IFileArchive, virtual CFileList
	{
	public:

		//! constructor
		CProtonReader(IReadFile* file, bool ignoreCase, bool ignorePaths);

		//! destructor
		virtual ~CProtonReader();

		//! opens a file by file name
		virtual IReadFile* createAndOpenFile(const io::path& filename);

		//! opens a file by index
		virtual IReadFile* createAndOpenFile(u32 index);

		//! returns the list of files
		virtual const IFileList* getFileList() const;

		//! get the archive type
		virtual E_FILE_ARCHIVE_TYPE getType() const;

		//! Searches for a file or folder within the list, returns the index
		virtual s32 findFile(const io::path& filename, bool isFolder) const;

	protected:

		IReadFile* File;

	};


} // end namespace io
} // end namespace irr

#endif // __IRR_COMPILE_WITH_PROTON_ARCHIVE_LOADER_
#endif // __C_PROTON_READER_H_INCLUDED__

