#include "SevenZipHandler.h"

#include <Shlwapi.h>

// Handler GUIDs

// {23170F69-40C1-278A-1000-000110010000}
DEFINE_GUID(CLSID_CFormatZip,
	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x01, 0x00, 0x00);

// {23170F69-40C1-278A-1000-000110020000}
DEFINE_GUID(CLSID_CFormatBZip2,
	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x02, 0x00, 0x00);

// {23170F69-40C1-278A-1000-000110030000}
DEFINE_GUID(CLSID_CFormatRar,
	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x03, 0x00, 0x00);

// {23170F69-40C1-278A-1000-000110070000}
DEFINE_GUID(CLSID_CFormat7z,
	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x07, 0x00, 0x00);

// {23170F69-40C1-278A-1000-000110080000}
DEFINE_GUID(CLSID_CFormatCab,
	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x08, 0x00, 0x00);

// {23170F69-40C1-278A-1000-0001100A0000}
DEFINE_GUID(CLSID_CFormatLzma,
	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x0A, 0x00, 0x00);

// {23170F69-40C1-278A-1000-0001100B0000}
DEFINE_GUID(CLSID_CFormatLzma86,
	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0x0B, 0x00, 0x00);

// {23170F69-40C1-278A-1000-000110E70000}
DEFINE_GUID(CLSID_CFormatIso,
	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xE7, 0x00, 0x00);

// {23170F69-40C1-278A-1000-000110EE0000}
DEFINE_GUID(CLSID_CFormatTar,
	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xEE, 0x00, 0x00);

// {23170F69-40C1-278A-1000-000110EF0000}
DEFINE_GUID(CLSID_CFormatGZip,
	0x23170F69, 0x40C1, 0x278A, 0x10, 0x00, 0x00, 0x01, 0x10, 0xEF, 0x00, 0x00);

void SevenZipHandler::Open()
{
	const TCHAR* filePath = _T("abc.zip");
	IStream* fileStream;

#ifdef _UNICODE
	const WCHAR* filePathStr = filePath.c_str();
#else
	WCHAR filePathStr[MAX_PATH];
	MultiByteToWideChar(CP_UTF8, 0, filePath, strlen(filePath) + 1, filePathStr, MAX_PATH);
#endif
	if (FAILED(SHCreateStreamOnFileEx(filePathStr, STGM_READ, FILE_ATTRIBUTE_NORMAL, FALSE, NULL, &fileStream)))
	{
		return;
	}

	auto inStream = new InStreamWrapper(fileStream);

	IInArchive* archive = (IInArchive*)CreateObject(CLSID_CFormatZip, IID_IInArchive);

	if (!archive)
	{
		//PrintError("Can not get class object");
		return;
	}

	const UInt64 scanSize = 1 << 23;
	if (archive->Open(inStream, &scanSize, nullptr) != S_OK)
	{
		//PrintError("Can not open file as archive", archiveName);
		return;
	}
	// List command
	UInt32 numItems = 0;
	archive->GetNumberOfItems(&numItems);

}
