// Copyright (c) Microsoft Corporation.
// Licensed under the MIT license.

namespace D3D9on12
{
	template<class DeviceType>
	DeviceType* DeviceFactory<DeviceType>::CreateDevice(Adapter& adapter, _Inout_ D3DDDIARG_CREATEDEVICE& CreateDeviceArgs) {
		ConstantsManagerFactoryImpl constantsManagerFactory;
		FastUploadAllocatorFactoryImpl fastUploadAllocatorFactory;
		PipelineStateFactoryImpl pipelineStateFactory;
		PipelineStateCacheFactoryImpl pipelineStateCacheFactory;
		DeviceType* device = new DeviceType(adapter, CreateDeviceArgs, 
			constantsManagerFactory,fastUploadAllocatorFactory,
			pipelineStateFactory,
			pipelineStateCacheFactory);

		
		ThrowFailure(device->Init(adapter.GetDevice(), adapter.GetCommandQueue()));
		device->UpdateCreateArgsForRuntime(/*inout*/ CreateDeviceArgs);

		return device;
	}
}