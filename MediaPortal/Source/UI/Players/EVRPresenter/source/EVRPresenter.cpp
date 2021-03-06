#include <windows.h>
#include <streams.h>
#include <stdio.h>
#include <atlbase.h>

#include <d3d9.h>
#include <dshow.h>
#include <list>
#include <vector>

using namespace std;

#include <comutil.h>
#include <evr.h>
#include <dxva2api.h>
#include "IEVRCallback.h"
#include "EVRCustomPresenter.h"

void Log(const char *fmt, ...) ;

static list<EVRCustomPresenter*> m_evrPresenters;
typedef list<EVRCustomPresenter*>::iterator itevrAllocator;


__declspec(dllexport) int EvrInit(IEVRCallback* callback, DWORD dwD3DDevice, IBaseFilter* evrFilter, DWORD monitor)
{
	HRESULT hr;

	CComQIPtr<IMFVideoRenderer> pRenderer = evrFilter;
	if (!pRenderer) 
   {
		Log("Could not get IMFVideoRenderer");
		return -1;
	}

  int id=0;
  while (true)
  {
    bool inUse=false;
    for (itevrAllocator it=m_evrPresenters.begin(); it !=m_evrPresenters.end();it++)
    {
      EVRCustomPresenter* allocator=*it;
      if (allocator->Id()==id)
      {
        inUse=true;
        break;
      }
    }
    if (!inUse) break;
    id++;
  }

	EVRCustomPresenter* presenter = new EVRCustomPresenter(id,callback, (LPDIRECT3DDEVICE9) dwD3DDevice, (HMONITOR) monitor);
  m_evrPresenters.push_back(presenter);
  hr = pRenderer->InitializeRenderer(NULL, presenter);
  if (FAILED(hr) ) 
  {
	  Log("InitializeRenderer failed: 0x%x", hr);
    pRenderer.Release();
	  return -1;
  }
  pRenderer.Release();
	return id;
}


__declspec(dllexport) void EvrDeinit(int id)
{
  try
  {
		Log("EvrDeinit: Release: %d", id);
    for (itevrAllocator it=m_evrPresenters.begin(); it != m_evrPresenters.end(); it++)
    {
      EVRCustomPresenter* allocator=*it;
      if (allocator->Id()==id)
      {
        int hr;
        do
		    {
			    hr=allocator->Release();
		    } while (hr>0);
        m_evrPresenters.erase(it);

   	    Log("EvrDeinit(): Release allocator");

	      allocator=NULL;
	      Log("EvrDeinit: m_evrPresenter released");
        break;
      }
    }
  }
  catch(...)
  {
    Log("EvrDeinit:exception");
  }
}


__declspec(dllexport) void EvrEnableFrameSkipping(int handle, bool onOff)
{
  for (itevrAllocator it=m_evrPresenters.begin(); it !=m_evrPresenters.end(); it++)
  {
    EVRCustomPresenter* allocator=*it;
    if (allocator->Id()==handle)
    {
      allocator->EnableFrameSkipping(onOff);
    }
  }
}


__declspec(dllexport) void EvrFreeResources(int id)
{
	Log("EvrFreeResources:%d", id);
  for (itevrAllocator it=m_evrPresenters.begin(); it != m_evrPresenters.end(); it++)
  {
    EVRCustomPresenter* allocator=*it;
    if (allocator->Id()==id)
    {
      allocator->FreeDirectxResources();
      return;
    }
  }
}


__declspec(dllexport) void EvrReAllocResources(int id)
{
	Log("EvrReAllocResources:%d", id);
  for (itevrAllocator it=m_evrPresenters.begin(); it != m_evrPresenters.end(); it++)
  {
    EVRCustomPresenter* allocator=*it;
    if (allocator->Id()==id)
    {
      allocator->ReAllocDirectxResources();
      return;
    }
  }
}


void Log(const char *fmt, ...) 
{
	va_list ap;
	va_start(ap,fmt);

	char buffer[1000]; 
	int tmp;
	va_start(ap,fmt);
	tmp=vsprintf(buffer, fmt, ap);
	va_end(ap); 

	FILE* fp = fopen("log/evr.log","a+");
	if (fp!=NULL)
	{
		SYSTEMTIME systemTime;
		GetLocalTime(&systemTime);
		fprintf(fp,"%02.2d-%02.2d-%04.4d %02.2d:%02.2d:%02.2d.%03.3d [%x]%s\n",
			systemTime.wDay, systemTime.wMonth, systemTime.wYear,
			systemTime.wHour,systemTime.wMinute,systemTime.wSecond,
			systemTime.wMilliseconds,
			GetCurrentThreadId(),
			buffer);
		fclose(fp);
	}
};


HRESULT __fastcall UnicodeToAnsi(LPCOLESTR pszW, LPSTR* ppszA)
{
  ULONG cbAnsi, cCharacters;
  DWORD dwError;

  // If input is null then just return the same.
  if (pszW == NULL)
  {
    *ppszA = NULL;
    return NOERROR;
  }

  cCharacters = wcslen(pszW)+1;
  // Determine number of bytes to be allocated for ANSI string. An
  // ANSI string can have at most 2 bytes per character (for Double
  // Byte Character Strings.)
  cbAnsi = cCharacters*2;

  // Use of the OLE allocator is not required because the resultant
  // ANSI  string will never be passed to another COM component. You
  // can use your own allocator.
  *ppszA = (LPSTR) CoTaskMemAlloc(cbAnsi);
  if (NULL == *ppszA)
      return E_OUTOFMEMORY;

  // Convert to ANSI.
  if (0 == WideCharToMultiByte(CP_ACP, 0, pszW, cCharacters, *ppszA, cbAnsi, NULL, NULL))
  {
    dwError = GetLastError();
    CoTaskMemFree(*ppszA);
    *ppszA = NULL;
    return HRESULT_FROM_WIN32(dwError);
  }
  return NOERROR;
}
