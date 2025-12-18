#pragma once
#include "global.h"

HRESULT STDMETHODCALLTYPE hkPresent(IDXGISwapChain3* pSwapChain, UINT SyncInterval, UINT Flags);
void STDMETHODCALLTYPE hkExecuteCommandLists(ID3D12CommandQueue* queue, UINT NumCommandLists, ID3D12CommandList* const* ppCommandLists);
HRESULT STDMETHODCALLTYPE hkResizeBuffers(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);

DWORD WINAPI MainThread(LPVOID);