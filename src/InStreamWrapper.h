#pragma once

#include <Windows.h>
#include <7zip/IStream.h>
#include "stdio.h"
#include <Common/MyCom.h>
#include <7zip/IPassword.h>
#include <7zip/Archive/IArchive.h>
#include <string>


class CArchiveExtractCallback :
	public IArchiveExtractCallback,
	public ICryptoGetTextPassword,
	public CMyUnknownImp
{
public:
	MY_UNKNOWN_IMP1(ICryptoGetTextPassword)

		// IProgress
		STDMETHOD(SetTotal)(UInt64 size) {
		return S_OK;
	}
	STDMETHOD(SetCompleted)(const UInt64 *completeValue) {
		return S_OK;
	}

	// IArchiveExtractCallback
	STDMETHOD(GetStream)(UInt32 index, ISequentialOutStream **outStream, Int32 askExtractMode);
	STDMETHOD(PrepareOperation)(Int32 askExtractMode);
	STDMETHOD(SetOperationResult)(Int32 resultEOperationResult);

	// ICryptoGetTextPassword
	STDMETHOD(CryptoGetTextPassword)(BSTR *aPassword);

private:
	//CMyComPtr<IInArchive> _archiveHandler;
	//FString _directoryPath;  // Output directory
	//UString _filePath;       // name inside arcvhive
	//FString _diskFilePath;   // full path to file on disk
	bool _extractMode;
	//struct CProcessedFileInfo
	//{
	//	FILETIME MTime;
	//	UInt32 Attrib;
	//	bool isDir;
	//	bool AttribDefined;
	//	bool MTimeDefined;
	//} _processedFileInfo;

	//COutFileStream *_outFileStreamSpec;
	//CMyComPtr<ISequentialOutStream> _outFileStream;

public:
	void Init(IInArchive *archiveHandler);

	UInt64 NumErrors;
	bool PasswordIsDefined;
	//UString Password;

	CArchiveExtractCallback() : PasswordIsDefined(false) {}
};

class ArchiveOpenCallback :
	public IArchiveOpenCallback,
	public ICryptoGetTextPassword,
	public CMyUnknownImp
{
public:
	MY_UNKNOWN_IMP1(ICryptoGetTextPassword)


	STDMETHOD(SetTotal)(const UInt64 *files, const UInt64 *bytes) {
		return S_OK;
	}
	STDMETHOD(SetCompleted)(const UInt64 *files, const UInt64 *bytes) {
		return S_OK;
	}

	STDMETHOD(CryptoGetTextPassword)(BSTR *password) {
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


	bool PasswordIsDefined = false;
	std::string Password;
};


class InStreamWrapper : public IInStream, public IStreamGetSize, public CMyUnknownImp
{
private:

	long				m_refCount;
	IStream*	m_baseStream;

public:
	MY_UNKNOWN_IMP1(IStreamGetSize)
		/*
				HRESULT STDMETHODCALLTYPE InStreamWrapper::QueryInterface(REFIID iid, void** ppvObject)
				{
					return E_NOINTERFACE;
				}

				ULONG STDMETHODCALLTYPE InStreamWrapper::AddRef()
				{
					return static_cast<ULONG>(InterlockedIncrement(&m_refCount));
				}

				ULONG STDMETHODCALLTYPE InStreamWrapper::Release()
				{
					ULONG res = static_cast<ULONG>(InterlockedDecrement(&m_refCount));
					if (res == 0)
					{
						delete this;
					}
					return res;
				}
		*/
		InStreamWrapper(IStream*	baseStream) {
		m_baseStream = baseStream;
	}

	// ISequentialInStream
	STDMETHOD(Read)(void* data, UInt32 size, UInt32* processedSize) {
		ULONG read = 0;
		HRESULT hr = m_baseStream->Read(data, size, &read);
		if (processedSize != NULL)
		{
			*processedSize = read;
		}
		// Transform S_FALSE to S_OK
		return SUCCEEDED(hr) ? S_OK : hr;
	}

	// IInStream
	STDMETHOD(Seek)(Int64 offset, UInt32 seekOrigin, UInt64* newPosition) {
		LARGE_INTEGER move;
		ULARGE_INTEGER newPos;

		move.QuadPart = offset;
		HRESULT hr = m_baseStream->Seek(move, seekOrigin, &newPos);
		if (newPosition != NULL)
		{
			*newPosition = newPos.QuadPart;
		}
		return hr;
	}


	// IStreamGetSize
	STDMETHOD(GetSize)(UInt64* size) {
		STATSTG statInfo;
		HRESULT hr = m_baseStream->Stat(&statInfo, STATFLAG_NONAME);
		if (SUCCEEDED(hr))
		{
			*size = statInfo.cbSize.QuadPart;
		}
		return hr;
	}

};
