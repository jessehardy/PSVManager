#include "SevenZipHandler.h"
#include "UTF8.h"
#include <Shlwapi.h>
#include "GUIDs.h"
#include <vector>

using namespace NWindows;

void SevenZipHandler::Open()
{
	const char* filePath = "abc.zip";
	IStream* fileStream;

	if (FAILED(SHCreateStreamOnFileEx(widen(filePath).c_str(), STGM_READ, FILE_ATTRIBUTE_NORMAL, FALSE, NULL, &fileStream)))
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

	struct ArchiveItemInfo {
		std::string path;
		uint64_t size;
		bool encrypted;
		uint32_t CRC;
		std::string method;
	};

	std::vector<ArchiveItemInfo> infoList;

	for (auto i = 0u; i < numItems; ++i) {
		ArchiveItemInfo info;
			// Get uncompressed size of file
			PROPVARIANT prop;

			// Get name of file
			memset(&prop, 0, sizeof(prop));
			archive->GetProperty(i, kpidPath, &prop);
			if (prop.vt == VT_BSTR)
				info.path = narrow(prop.bstrVal);
			else if (prop.vt != VT_EMPTY)
				throw std::runtime_error("Error");

			memset(&prop, 0, sizeof(prop));
			archive->GetProperty(i, kpidSize, &prop);
			if (prop.vt == VT_UI8)
				info.size = prop.uhVal.QuadPart;
			else if (prop.vt != VT_EMPTY)
				throw std::runtime_error("Error");


			memset(&prop, 0, sizeof(prop));
			archive->GetProperty(i, kpidEncrypted, &prop);
			if (prop.vt == VT_BOOL)
				info.encrypted = prop.boolVal != 0;
			else if (prop.vt != VT_EMPTY)
				throw std::runtime_error("Error");

			memset(&prop, 0, sizeof(prop));
			archive->GetProperty(i, kpidCRC, &prop);
			if (prop.vt == VT_UI4)
				info.CRC = prop.uintVal;
			else if (prop.vt == VT_EMPTY)
				info.CRC = 0;
			else
				throw std::runtime_error("Error");

			memset(&prop, 0, sizeof(prop));
			archive->GetProperty(i, kpidMethod, &prop);
			if (prop.vt == VT_BSTR)
				info.method = narrow(prop.bstrVal);
			else if (prop.vt != VT_EMPTY)
				throw std::runtime_error("Error");

			infoList.push_back(info);
	}
	archive->Close();
	archive->Release();
}
