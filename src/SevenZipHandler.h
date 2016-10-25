#ifndef SevenZipHandler_h__
#define SevenZipHandler_h__

#include <Windows.h>
#include <tchar.h>
#include <stdexcept>
#include <assert.h>
#include "InStreamWrapper.h"
#include <7zip/Archive/IArchive.h>

#define kDllName _T("7z.dll")

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

	bool Load() {
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
	void Open();
};

#endif // SevenZipHandler_h__