// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

namespace D3D9on12
{
	template<class DeviceType>
	DeviceType* DeviceFactory<DeviceType>::CreateDevice(Adapter& adapter, _Inout_ D3DDDIARG_CREATEDEVICE& CreateDeviceArgs) {
		DeviceType* device = new DeviceType(adapter, CreateDeviceArgs);

		memcpy((void*)&device->m_Callbacks, CreateDeviceArgs.pCallbacks, sizeof(device->m_Callbacks));
		memset(device->m_streamFrequency, 0, sizeof(device->m_streamFrequency));
		ThrowFailure(device->Init(adapter.GetDevice(), adapter.GetCommandQueue()));
		device->UpdateCreateArgsForRuntime(/*inout*/ CreateDeviceArgs);

		return device;
	}
}