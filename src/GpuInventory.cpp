#include "GpuInventory.h"

#include <windows.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

namespace {

constexpr UINT kSoftwareAdapterVendorId = 0x1414; // Microsoft Basic Render Driver

} // namespace

GpuInventory::GpuInventory() {
    ComPtr<IDXGIFactory6> factory6;
    ComPtr<IDXGIFactory1> factory1;
    if (FAILED(CreateDXGIFactory1(IID_PPV_ARGS(&factory1)))) {
        return;
    }
    factory1.As(&factory6); // optional, used for high-perf ordering

    int idx = 0;
    for (UINT i = 0;; ++i) {
        ComPtr<IDXGIAdapter1> adapter;
        HRESULT hr = E_FAIL;
        if (factory6) {
            hr = factory6->EnumAdapterByGpuPreference(
                i, DXGI_GPU_PREFERENCE_UNSPECIFIED, IID_PPV_ARGS(&adapter));
        } else {
            hr = factory1->EnumAdapters1(i, &adapter);
        }
        if (hr == DXGI_ERROR_NOT_FOUND || FAILED(hr)) {
            break;
        }

        DXGI_ADAPTER_DESC1 desc{};
        if (FAILED(adapter->GetDesc1(&desc))) {
            continue;
        }
        if ((desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) != 0) {
            continue;
        }
        if (desc.VendorId == kSoftwareAdapterVendorId) {
            continue;
        }

        GpuAdapter a;
        a.index = idx++;
        a.luidHigh = static_cast<quint32>(desc.AdapterLuid.HighPart);
        a.luidLow  = desc.AdapterLuid.LowPart;
        a.name     = QString::fromWCharArray(desc.Description);
        a.dedicatedVideoMemory  = desc.DedicatedVideoMemory;
        a.dedicatedSystemMemory = desc.DedicatedSystemMemory;
        a.sharedSystemMemory    = desc.SharedSystemMemory;
        a.vendorId = desc.VendorId;
        a.deviceId = desc.DeviceId;
        adapters_.append(a);
    }
}

int GpuInventory::indexForLuid(quint32 high, quint32 low) const {
    for (const auto& a : adapters_) {
        if (a.luidHigh == high && a.luidLow == low) {
            return a.index;
        }
    }
    return -1;
}
