# D3D9On12

D3D9On12 is a mapping layer, which maps graphics commands from D3D9 to D3D12. D3D9On12 is **not** an implementation of the D3D9 **API**, but is instead an implementation of the D3D9 usermode **DDI** (device driver interface). That means it is not a binary named d3d9.dll, but is named d3d9on12.dll.

When an application creates a D3D9 device, they may choose for it to be a D3D9On12 device, rather than a native D3D9 device (see [Direct3DCreate9On12](https://github.com/microsoft/DirectX-Specs/blob/master/d3d/TranslationLayerResourceInterop.md#function-direct3dcreate9on12) and [Direct3DCreate9On12Ex](https://github.com/microsoft/DirectX-Specs/blob/master/d3d/TranslationLayerResourceInterop.md#function-direct3dcreate9on12ex)). When this happens, d3d9on12.dll is loaded by the D3D9 runtime and initialized. When the application calls rendering commands, D3D9 will validate those commands, and then will convert those commands to the D3D9 DDI and send it to D3D9On12, just like any D3D9 driver. D3D9On12 will take these commands and convert them into D3D12 API calls, which are further validated by the D3D12 runtime, optionally including the D3D12 debug layer, which are then converted to the D3D12 DDI and sent to the D3D12 driver.

Note that D3D9On12 is an *enlightened* D3D9 driver, and there are several places where it receives additional information compared to a traditional D3D9 driver, either to enable it to provide API-level information to D3D12 rather than driver-level information (as is the case for shaders), or to enable interop scenarios. When a D3D9 device is created with D3D9On12, the device will expose an `IDirect3DDevice9On12` interface which enables applications to submit work to both the D3D9 API and the D3D12 API with lightweight sharing and synchronization.

For more details about D3D9On12, see:
* [D3D Translation Layer spec](https://github.com/microsoft/DirectX-Specs/blob/master/d3d/TranslationLayerResourceInterop.md)
* [D3D9On12 blog posts](https://devblogs.microsoft.com/directx/coming-to-directx-12-d3d9on12-and-d3d11on12-resource-interop-apis/)

Make sure that you visit the [DirectX Landing Page](https://devblogs.microsoft.com/directx/landing-page/) for more resources for DirectX developers.

## How does it work?

The primary entrypoint to D3D9On12 is a custom version of the normal D3D9 driver [OpenAdapter](https://docs.microsoft.com/en-us/windows-hardware/drivers/ddi/d3dumddi/nc-d3dumddi-pfnd3dddi_openadapter) entrypoint, named `OpenAdapter_Private`, where D3D9 provides additional information to the mapping layer. In response to this, like a normal driver, D3D9On12 returns an adapter object, which exposes DDIs to create a device. The device is created like normal, but in addition to the normal DDI tables, D3D9on12 also exposes a set of DDIs defined by the `D3D9ON12_PRIVATE_DDI_TABLE` which can be retrieved by calling `GetPrivateDDITable`.

The device object internally uses an instance of the D3D12TranslationLayer immediate context to record commands. Similarly, most D3D9On12 objects are backed by an implementation from the D3D12TranslationLayer library. The code in this repository is largely an adaptor from the D3D9 DDI to the D3D12TranslationLayer library, where the real heavy lifting of converting to the D3D12 domain is done.

## Building

In order to build D3D9On12, the [WDK](https://docs.microsoft.com/en-us/windows-hardware/drivers/download-the-wdk) (Windows Driver Kit) must be installed, in order to provide `d3d10umddi.h` to D3D9On12, and in order to generate the D3D12TranslationLayer_WDK project, which hosts some code required to parse DXBC shaders and containers. The D3D12TranslationLayer and its subprojects, D3D12TranslationLayer_WDK and DXBCParser, will be fetched from GitHub when building with CMake if D3D12TranslationLayer_WDK isn't already included, such as by a parent CMakeLists.txt that has already entered that project. The `DxbcSigner.dll` will be pulled in automatically from NuGet.

If you had a local copy of the D3D12TranslationLayer project, your top level project would look something like this:

```CMake
cmake_minimum_required(VERSION 3.14)
include(FetchContent)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(parent)

FetchContent_Declare(
    d3d12translationlayer
    SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/D3D12TranslationLayer
)
FetchContent_MakeAvailable(d3d12translationlayer)

add_subdirectory(D3D9On12)
```

At the time of publishing, the D3D9On12 and D3D12TranslationLayer require **insider** versions of the SDK and WDK. Those can be found [here](https://www.microsoft.com/en-us/software-download/windowsinsiderpreviewWDK).

D3D9On12 requires C++17, and only supports building with MSVC at the moment.

### Usage
Note that in order to use your custom version of D3D9on12 you will also need to copy `WinPixEventRuntime.dll` (will automatically be placed in D3D9on12's build output) and `dxbcsigner.dll` along with it.

## Why open source?

The D3D9On12 mapping layer is included as an operating system component of Windows 10. Over the years and Windows 10 releases, it has grown in functionality, to the point where it is a complete and relatively performant implementation of a D3D9 driver. We are choosing to release the source to this component for two primary reasons:
1. To enable the community to contribute bugfixes and further performance improvements, which will improve the stability and performance of Windows 10. See [CONTRIBUTING](CONTRIBUTING.md).
2. To serve as an example of how to use the D3D12TranslationLayer library.

### What can you do with this?

There are minor differences between binaries built out of this repository and the versions that are included in the OS. To that end, shipping applications should not attempt to override the OS version of D3D9On12 with versions that they have built. We will not guarantee that newer versions of Windows will continue to support older versions of D3D9On12, since it is an OS component which may be revised together with D3D9. However, developers are welcome to override the Windows version of D3D9On12 for *local testing and experimentation*.

### Compatibility

When possible, we will attempt to maintain compatibility between D3D9 and D3D9On12. One should expect that the tip of D3D9On12's `master` branch should work nicely with the latest release of Windows 10. Support for configurations other than that are not guaranteed to work.

## Data Collection

The software may collect information about you and your use of the software and send it to Microsoft. Microsoft may use this information to provide services and improve our products and services. You may turn off the telemetry as described in the repository. There are also some features in the software that may enable you and Microsoft to collect data from users of your applications. If you use these features, you must comply with applicable law, including providing appropriate notices to users of your applications together with a copy of Microsoft's privacy statement. Our privacy statement is located at https://go.microsoft.com/fwlink/?LinkID=824704. You can learn more about data collection and use in the help documentation and our privacy statement. Your use of the software operates as your consent to these practices.

Note however that no data collection is performed when using your private builds.
