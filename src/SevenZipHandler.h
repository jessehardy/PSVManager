#ifndef SevenZipHandler_h__
#define SevenZipHandler_h__

#include <Windows.h>
#include <tchar.h>
#include <stdexcept>
#include <assert.h>
#include "InStreamWrapper.h"
#include <7zip/Archive/IArchive.h>
#include <vector>

#define kDllName _T("7z.dll")

struct ArchiveItemInfo {
	int index;
	int parent;
	bool isDir;
	std::string path;
	std::string parentPath;
	uint64_t size;
	uint64_t packSize;
	bool encrypted;
	uint32_t CRC;
	std::string method;
	std::string name;
};


class SevenZipHandler
{
private:

	typedef UINT32(WINAPI * CreateObjectFunc)(const GUID* clsID, const GUID* interfaceID, void** outObject);

	HMODULE				m_dll = nullptr;
	CreateObjectFunc	m_func = nullptr;

public:
	SevenZipHandler() {
		Load();
	}
	~SevenZipHandler() {
		Free();
	}

	void Load() {
		m_dll = LoadLibrary(kDllName);
		if (m_dll == NULL)
		{
			throw std::runtime_error("LoadLibrary");
		}

		m_func = reinterpret_cast<CreateObjectFunc>(GetProcAddress(m_dll, "CreateObject"));
		if (m_func == NULL)
		{
			Free();
			throw std::runtime_error("Loaded library is missing required CreateObject function");
		}
	}
	void Free() {
		if (m_dll != NULL)
		{
			FreeLibrary(m_dll);
			m_dll = NULL;
			m_func = NULL;
		}
	}

	void* CreateObject(const GUID& clsID, const GUID& interfaceID) const {
		assert(m_func);

		void* r;
		HRESULT hr = m_func(&clsID, &interfaceID, &r);
		if (FAILED(hr))
		{
			r = nullptr;
		}
		return r;
	}
	bool GetProperty(PROPVARIANT &prop, IInArchive* archive, unsigned int i, VARTYPE propId, VARENUM vt)
	{
		memset(&prop, 0, sizeof(prop));
		archive->GetProperty(i, propId, &prop);
		if (prop.vt != vt && prop.vt != VT_EMPTY)
			throw std::runtime_error("Type Error!");
		return prop.vt == vt;
	}
	bool GetArchiveProperty(PROPVARIANT &prop, IInArchive* archive, VARTYPE propId, VARENUM vt)
	{
		memset(&prop, 0, sizeof(prop));
		archive->GetArchiveProperty(propId, &prop);
		if (prop.vt != vt && prop.vt != VT_EMPTY)
			throw std::runtime_error("Type Error!");
		return prop.vt == vt;
	}

	void Open();

	void FindDumpToolLib(std::vector<ArchiveItemInfo> &infoList, std::string libPath, std::string& dumpTool);

};

#endif // SevenZipHandler_h__