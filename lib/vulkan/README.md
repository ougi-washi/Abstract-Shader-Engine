This repository contains a copy of prebuilt binaries and headers from
the Vulkan SDK. This is being done to allow prebuilt Vulkan libraries
to be pulled in by Hunter.

# Supported platforms

- macOS
- iOS
- tvOS
- Win64
- Linux

iOS and tvOS require an appropriate toolchain file to be used.

# Source Packages

The prebuilt binaries contained in the repository are copied directly
from official Vulkan SDK releases. The following lists the various
releases used, as well as their versions.

## MacOS

Filename: [vulkansdk-macos-1.2.162.1.dmg](https://vulkan.lunarg.com/sdk/home#sdk/downloadConfirm/1.2.162.1/mac/vulkansdk-macos-1.2.162.1.dmg)

MD5: `64088b71f9703ac032f427a17a7f7b41`

## iOS / tvOS

Filename: [vulkansdk-macos-1.2.154.0.dmg](https://vulkan.lunarg.com/sdk/home#sdk/downloadConfirm/1.2.154.0/mac/vulkansdk-macos-1.2.154.0.dmg)

MD5: `f0fea776e02ac92dbb47ba8bdad4130c`

## Win64

Filename: [VulkanSDK-1.2.154.1-Installer.exe](https://vulkan.lunarg.com/sdk/home#sdk/downloadConfirm/1.2.154.1/windows/VulkanSDK-1.2.154.1-Installer.exe)

MD5: `7609be3d8b7d11d3438064eabb4cd4b7`

## Linux

Filename: [vulkansdk-linux-x86_64-1.2.154.0.tar.gz](https://vulkan.lunarg.com/sdk/home#sdk/downloadConfirm/1.2.154.0/linux/vulkansdk-linux-x86_64-1.2.154.0.tar.gz)

MD5: `91c548e851d4c679c072c109aae8a775`

libvulkan.a:
On Linux, the vulkan sdk provides only a shared library, so we'll need to build the static library ourselves.
1. Extract the content of vulkansdk-linux-x86_64-1.2.154.0.tar.gz
2. Go to the extracted folder (`1.2.154.0`)
2. Modify source/Vulkan-Loader/loader/CMakeLists.txt by changing line 233 from `add_library(vulkan SHARED ${NORMAL_LOADER_SRCS} ${OPT_LOADER_SRCS})` to `add_library(vulkan STATIC ${NORMAL_LOADER_SRCS} ${OPT_LOADER_SRCS})`
3. Run `./vulkansdk vulkan-loader`
4. The static library will be created at `x86_64/lib/libvulkan.a`

Enabling the validation layers on Linux:
The validation layers require that the app to be dynamically linked against the vulkan library.
A dynamic version of this library can be found in the Vulkan SDK.
In order to use the Vulkan SDK, the environment must point to folder containing it.
Add in your `.bashrc` or `.zshrc` the following line:
`source ${path-to-vulkan-sdk}/1.2.154.0/setup-env.sh`, where `path-to-vulkan-sdk` points to the folder where the sdk has been extracted.

# Porting and Testing

The script `build.sh` can be used to build the library and 'install' it
to the `install/` folder for the current platform.

To test, the script `test/build.sh` can be used. That script builds a
simple test program that links to the 'installed' library, again for the
current platform. It is necessary for the library to have been built
before running the test build script.
