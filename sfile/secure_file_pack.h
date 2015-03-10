#ifndef CHECKHEADER_SLIB_SFILE_SECUREFILEPACK
#define CHECKHEADER_SLIB_SFILE_SECUREFILEPACK

#include "definition.h"

#include "../../slib/core/string.h"
#include "../../slib/core/file.h"
#include "../../slib/core/time.h"

SLIB_SFILE_NAMESPACE_START

class SecureFilePackage
{
protected:
	String m_filePath;
	sl_uint8 m_password[32]; // SHA-256
	sl_bool m_flagPassword;

public:
	SecureFilePackage()
	{
	}

public:

	struct CreateParam {
		sl_uint16 type; // reserved
		sl_uint16 version; // reserved
		String password;
	};

	struct OpenParam {
		String password;
	};

	enum IndexType
	{
		indexTypeEmpty = 0
		, indexTypeDirectory = 1
		, indexTypeFile = 2
	};

	struct FileDesc
	{
		String filePath;
		IndexType type;
		sl_int64 position;
		sl_int64 size;
		Time timeModified;
	};

	struct Progress
	{
		sl_int64 nFiles;
		sl_int64 nSize;
		String lastFilePath;

		sl_bool flagRequestStop;

		Progress()
		{
			nFiles = 0;
			nSize = 0;
			flagRequestStop = sl_false;
		}
	};

	enum ErrorCode
	{
		errorOK = 0
		, errorNotOpened = 1
		, errorInvalidHeader = 2
		, errorPasswordMismatch = 3
	};

	static sl_bool create(String filePath, const CreateParam& param, String pathSourceDirectory, Progress* progress = sl_null);

	// pathBaseDirectory should be empty for fullpaths, otherwise must contain / as suffix for base directory
	static sl_bool createFromFiles(String filePath, const CreateParam& param, String pathSourceBase, List<String> listSourceFilePaths, Progress* progress = sl_null)
	{
		return createFromFiles(filePath, param, pathSourceBase, listSourceFilePaths, sl_false, progress);
	}

protected:
	static sl_bool createFromFiles(String filePath, const CreateParam& param, String pathSourceBase, List<String> listSourceFilePaths, sl_bool flagSorted, Progress* progress = sl_null);


public:
	ErrorCode open(String filePath, const OpenParam& param);

	sl_bool extract(String pathTargetDirectory, Progress* progress = sl_null);

	List<FileDesc> getFiles();

	// find file and returns the information
	sl_bool findFile(String fileName, FileDesc* output = sl_null);

	// read file at position
	Memory readFile(sl_int64 position, String* pFileName = sl_null);

};

SLIB_SFILE_NAMESPACE_END

#endif

