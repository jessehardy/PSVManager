#include "SevenZipHandler.h"
#include "UTF8.h"
#include <Shlwapi.h>
#include "GUIDs.h"
#include <vector>
#include <algorithm>
#include <map>
#include <Common/MyCom.h>
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

	CMyComPtr<InStreamWrapper> inStream = new InStreamWrapper(fileStream);

	CMyComPtr<IInArchive> archive = (IInArchive*)CreateObject(CLSID_CFormatZip, IID_IInArchive);

	if (!archive)
	{
		//PrintError("Can not get class object");
		return;
	}

	CMyComPtr<ArchiveOpenCallback> openCallbackSpec = new ArchiveOpenCallback;


	const UInt64 scanSize = 1 << 23;
	if (archive->Open(inStream, &scanSize, openCallbackSpec) != S_OK)
	{
		//PrintError("Can not open file as archive", archiveName);
		return;
	}
	// List command
	UInt32 numItems = 0;
	archive->GetNumberOfItems(&numItems);
	std::vector<ArchiveItemInfo> infoList;

	PROPVARIANT prop;
	std::string method;
	bool isSolid = false;
	uint32_t warningFlags = 0;
	uint32_t errorFlags = 0;
	if (GetArchiveProperty(prop, archive, kpidMethod, VT_BSTR))
		method = narrow(prop.bstrVal);
	if (GetArchiveProperty(prop, archive, kpidSolid, VT_BOOL))
		isSolid = prop.boolVal != 0;
	if (GetArchiveProperty(prop, archive, kpidWarningFlags, VT_UI4))
		warningFlags = prop.uintVal;
	if (GetArchiveProperty(prop, archive, kpidErrorFlags, VT_UI4))
		errorFlags = prop.uintVal;

	for (auto i = 0u; i < numItems; ++i) {
		ArchiveItemInfo info;


		if (GetProperty(prop, archive, i, kpidIsDir, VT_BOOL))
			info.isDir = prop.boolVal != 0;

		if (GetProperty(prop, archive, i, kpidPath, VT_BSTR))
			info.path = narrow(prop.bstrVal);

		if (GetProperty(prop, archive, i, kpidSize, VT_UI8))
			info.size = prop.uhVal.QuadPart;

		if (GetProperty(prop, archive, i, kpidPackSize, VT_UI8))
			info.packSize = prop.uhVal.QuadPart;

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

	CArchiveExtractCallback *extractCallbackSpec = new CArchiveExtractCallback;
	CMyComPtr<IArchiveExtractCallback> extractCallback(extractCallbackSpec);
	extractCallbackSpec->Init(archive); // second parameter is output folder path
	extractCallbackSpec->PasswordIsDefined = false;
	// extractCallbackSpec->PasswordIsDefined = true;
	// extractCallbackSpec->Password = L"1";

	/*
	const wchar_t *names[] =
	{
	L"mt",
	L"mtf"
	};
	const unsigned kNumProps = sizeof(names) / sizeof(names[0]);
	NCOM::CPropVariant values[kNumProps] =
	{
	(UInt32)1,
	false
	};
	CMyComPtr<ISetProperties> setProperties;
	archive->QueryInterface(IID_ISetProperties, (void **)&setProperties);
	if (setProperties)
	setProperties->SetProperties(names, values, kNumProps);
	*/

	HRESULT result = archive->Extract(NULL, (UInt32)(Int32)(-1), false, extractCallback);

	if (result != S_OK)
	{
		//PrintError("Extract Error");
		return;
	}
	archive->Close();
	archive->Release();

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


void CArchiveExtractCallback::Init(IInArchive *archiveHandler)
{
	NumErrors = 0;
	_archiveHandler = archiveHandler;
	//_directoryPath = directoryPath;
	//NName::NormalizeDirPathPrefix(_directoryPath);
}


STDMETHODIMP CArchiveExtractCallback::GetStream(UInt32 index,
	ISequentialOutStream **outStream, Int32 askExtractMode)
{
	*outStream = 0;
	_outFileStream.Release();

	//{
	//	// Get Name
	//	NCOM::CPropVariant prop;
	//	RINOK(_archiveHandler->GetProperty(index, kpidPath, &prop));

	//	UString fullPath;
	//	if (prop.vt == VT_EMPTY)
	//		fullPath = kEmptyFileAlias;
	//	else
	//	{
	//		if (prop.vt != VT_BSTR)
	//			return E_FAIL;
	//		fullPath = prop.bstrVal;
	//	}
	//	_filePath = fullPath;
	//}

	//if (askExtractMode != NArchive::NExtract::NAskMode::kExtract)
	//	return S_OK;

	//{
	//	// Get Attrib
	//	NCOM::CPropVariant prop;
	//	RINOK(_archiveHandler->GetProperty(index, kpidAttrib, &prop));
	//	if (prop.vt == VT_EMPTY)
	//	{
	//		_processedFileInfo.Attrib = 0;
	//		_processedFileInfo.AttribDefined = false;
	//	}
	//	else
	//	{
	//		if (prop.vt != VT_UI4)
	//			return E_FAIL;
	//		_processedFileInfo.Attrib = prop.ulVal;
	//		_processedFileInfo.AttribDefined = true;
	//	}
	//}

	//RINOK(IsArchiveItemFolder(_archiveHandler, index, _processedFileInfo.isDir));

	//{
	//	// Get Modified Time
	//	NCOM::CPropVariant prop;
	//	RINOK(_archiveHandler->GetProperty(index, kpidMTime, &prop));
	//	_processedFileInfo.MTimeDefined = false;
	//	switch (prop.vt)
	//	{
	//	case VT_EMPTY:
	//		// _processedFileInfo.MTime = _utcMTimeDefault;
	//		break;
	//	case VT_FILETIME:
	//		_processedFileInfo.MTime = prop.filetime;
	//		_processedFileInfo.MTimeDefined = true;
	//		break;
	//	default:
	//		return E_FAIL;
	//	}

	//}
	//{
	//	// Get Size
	//	NCOM::CPropVariant prop;
	//	RINOK(_archiveHandler->GetProperty(index, kpidSize, &prop));
	//	UInt64 newFileSize;
	//	/* bool newFileSizeDefined = */ ConvertPropVariantToUInt64(prop, newFileSize);
	//}


	//{
	//	// Create folders for file
	//	int slashPos = _filePath.ReverseFind_PathSepar();
	//	if (slashPos >= 0)
	//		CreateComplexDir(_directoryPath + us2fs(_filePath.Left(slashPos)));
	//}

	//FString fullProcessedPath = _directoryPath + us2fs(_filePath);
	//_diskFilePath = fullProcessedPath;

	//if (_processedFileInfo.isDir)
	//{
	//	CreateComplexDir(fullProcessedPath);
	//}
	//else
	//{
	//	NFind::CFileInfo fi;
	//	if (fi.Find(fullProcessedPath))
	//	{
	//		if (!DeleteFileAlways(fullProcessedPath))
	//		{
	//			PrintError("Can not delete output file", fullProcessedPath);
	//			return E_ABORT;
	//		}
	//	}

	//	_outFileStreamSpec = new COutFileStream;
	//	CMyComPtr<ISequentialOutStream> outStreamLoc(_outFileStreamSpec);
	//	if (!_outFileStreamSpec->Open(fullProcessedPath, CREATE_ALWAYS))
	//	{
	//		PrintError("Can not open output file", fullProcessedPath);
	//		return E_ABORT;
	//	}
	//	_outFileStream = outStreamLoc;
	//	*outStream = outStreamLoc.Detach();
	//}
	return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::PrepareOperation(Int32 askExtractMode)
{
	_extractMode = false;
	switch (askExtractMode)
	{
	case NArchive::NExtract::NAskMode::kExtract:  _extractMode = true; break;
	};
	//switch (askExtractMode)
	//{
	//case NArchive::NExtract::NAskMode::kExtract:  PrintString(kExtractingString); break;
	//case NArchive::NExtract::NAskMode::kTest:  PrintString(kTestingString); break;
	//case NArchive::NExtract::NAskMode::kSkip:  PrintString(kSkippingString); break;
	//};
	//PrintString(_filePath);
	return S_OK;
}

STDMETHODIMP CArchiveExtractCallback::SetOperationResult(Int32 operationResult)
{
	switch (operationResult)
	{
	case NArchive::NExtract::NOperationResult::kOK:
		break;
	default:
	{
		NumErrors++;
		//PrintString("  :  ");
		//const char *s = NULL;
		//switch (operationResult)
		//{
		//case NArchive::NExtract::NOperationResult::kUnsupportedMethod:
		//	s = kUnsupportedMethod;
		//	break;
		//case NArchive::NExtract::NOperationResult::kCRCError:
		//	s = kCRCFailed;
		//	break;
		//case NArchive::NExtract::NOperationResult::kDataError:
		//	s = kDataError;
		//	break;
		//case NArchive::NExtract::NOperationResult::kUnavailable:
		//	s = kUnavailableData;
		//	break;
		//case NArchive::NExtract::NOperationResult::kUnexpectedEnd:
		//	s = kUnexpectedEnd;
		//	break;
		//case NArchive::NExtract::NOperationResult::kDataAfterEnd:
		//	s = kDataAfterEnd;
		//	break;
		//case NArchive::NExtract::NOperationResult::kIsNotArc:
		//	s = kIsNotArc;
		//	break;
		//case NArchive::NExtract::NOperationResult::kHeadersError:
		//	s = kHeadersError;
		//	break;
		//}
		//if (s)
		//{
		//	PrintString("Error : ");
		//	PrintString(s);
		//}
		//else
		//{
		//	char temp[16];
		//	ConvertUInt32ToString(operationResult, temp);
		//	PrintString("Error #");
		//	PrintString(temp);
		//}
	}
	}

	//if (_outFileStream)
	//{
	//	if (_processedFileInfo.MTimeDefined)
	//		_outFileStreamSpec->SetMTime(&_processedFileInfo.MTime);
	//	RINOK(_outFileStreamSpec->Close());
	//}
	//_outFileStream.Release();
	//if (_extractMode && _processedFileInfo.AttribDefined)
	//	SetFileAttrib(_diskFilePath, _processedFileInfo.Attrib);
	//PrintNewLine();
	return S_OK;
}


STDMETHODIMP CArchiveExtractCallback::CryptoGetTextPassword(BSTR *password)
{
	if (!PasswordIsDefined)
	{
		// You can ask real password here from user
		// Password = GetPassword(OutStream);
		// PasswordIsDefined = true;
		//PrintError("Password is not defined");
		return E_ABORT;
	}
	return E_ABORT;
	//return StringToBstr(Password, password);
}

