#pragma once

#include <Windows.h>
#include <7zip/IStream.h>
#include "stdio.h"


	class InStreamWrapper : public IInStream, public IStreamGetSize
	{
	private:

		long				m_refCount;
		IStream*	m_baseStream;

	public:

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
