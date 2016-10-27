#include "SevenZipHandler.h"
#include "UTF8.h"
#include <Shlwapi.h>
#include "GUIDs.h"
#include <vector>
#include <algorithm>
#include <map>
using namespace NWindows;
std::map<std::pair<uint32_t, uint64_t>, std::string> DumpToolLibInfos{
	{ { 0x246ECE3C, 87866 }, "MaiDumpTool v233.2zEX v233.2z10" },
	{ { 0xBA7B8611, 87866 }, "MaiDumpTool v233.2z7 v233.2z8 v233.2z9" },
	{ { 0x67B037E4, 87954 }, "MaiDumpTool v233.2z2" },
	{ { 0xB392D8BE, 87056 }, "MaiDumpTool v233.1" },
	{ { 0x8F554660, 86442 }, "MaiDumpTool v233" },

	{ { 0x1DF8B729, 78682 }, "Vitamin V2.0" },
	{ { 0x6F6F71BB, 107851 }, "Vitamin V1.1" },
	{ { 0x4FD25E74, 110535 }, "Vitamin Leaked Version" },
};

void SevenZipHandler::Open()
{
	const char* filePath = "PCSE00575_FULLGAME_01.00.VPK";
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
	std::vector<ArchiveItemInfo> infoList;

	for (auto i = 0u; i < numItems; ++i) {
		ArchiveItemInfo info;

		PROPVARIANT prop;

		if (GetProperty(prop, archive, i, kpidIsDir, VT_BOOL))
			info.isDir = prop.boolVal != 0;

		if (GetProperty(prop, archive, i, kpidPath, VT_BSTR))
			info.path = narrow(prop.bstrVal);

		if (GetProperty(prop, archive, i, kpidSize, VT_UI8))
			info.size = prop.uhVal.QuadPart;

		if (GetProperty(prop, archive, i, kpidEncrypted, VT_BOOL))
			info.encrypted = prop.boolVal != 0;

		if (GetProperty(prop, archive, i, kpidCRC, VT_UI4))
			info.CRC = prop.uintVal;
		else
			info.CRC = 0;

		if (GetProperty(prop, archive, i, kpidMethod, VT_BSTR))
			info.method = narrow(prop.bstrVal);

		infoList.push_back(info);
	}
	archive->Close();
	archive->Release();

	std::sort(infoList.begin(), infoList.end(), [](const ArchiveItemInfo& a, const ArchiveItemInfo& b) {return a.path < b.path;});

	for (int i = 0; i < infoList.size(); ++i) {
		auto& info = infoList[i];
		info.index = i;
		auto lastSlash = info.path.find_last_of('\\');
		if (lastSlash == std::string::npos) {
			info.parent = -1;
			info.name = info.path;
		}
		else {
			info.parentPath = info.path.substr(0, lastSlash);
			info.name = info.path.substr(lastSlash + 1);
			info.parent = -1;
			for (int j = 0; j < infoList.size(); ++j) {
				if (infoList[j].path == info.parentPath) {
					info.parent = j;
					break;
				}
			}
			if (info.parent < 0) {
				//throw std::runtime_error("Not found parent.");
			}
		}
	}
	for (auto&& info : infoList) {
		if (info.name == "eboot.bin") {
			std::string dumpTool = "Unknown";
			auto parentPath = info.parentPath;
			if (!parentPath.empty()) {
				parentPath += "\\";
			}

			auto maiLibPath = parentPath + "mai_moe\\mai.suprx";
			FindDumpToolLib(infoList, maiLibPath, dumpTool);

			auto vitaminLibPath = parentPath + "sce_module\\steroid.suprx";
			FindDumpToolLib(infoList, vitaminLibPath, dumpTool);
		}
	}
}

void SevenZipHandler::FindDumpToolLib(std::vector<ArchiveItemInfo> &infoList, std::string libPath, std::string& dumpTool)
{
	auto libIt = std::find_if(infoList.begin(), infoList.end(), [&libPath](const ArchiveItemInfo&a) {return a.path == libPath;});
	if (libIt != infoList.end()) {
		auto& maiLibInfo = *libIt;
		auto crcIt = DumpToolLibInfos.find(std::make_pair(maiLibInfo.CRC, maiLibInfo.size));
		if (crcIt != DumpToolLibInfos.end()) {
			dumpTool = crcIt->second;
		}
	}
}
