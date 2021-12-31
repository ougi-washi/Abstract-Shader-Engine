cmake_minimum_required(VERSION 3.18)

set(_BUILD_FROM_INSTALLED_PACKAGE 1)


project(VulkanPrebuiltTest LANGUAGES C CXX)

# Find library
if (_BUILD_FROM_INSTALLED_PACKAGE)

    add_subdirectory(.. vulkanprebuild)
    set(_LIBRARY_NAME vulkan-prebuilt)

else()

    set(vulkan-prebuilt_DIR "../install/lib/cmake/vulkan-prebuilt/")
    find_package(vulkan-prebuilt)
    if (NOT _FOUND AND NOT VULKAN-PREBUILT_FOUND AND NOT vulkan-prebuilt_FOUND)
        message(FATAL_ERROR "could not find lib.")
    endif()
    set(_LIBRARY_NAME vulkan-prebuilt::vulkan-prebuilt)

endif()

add_executable(VulkanPrebuiltTest
    TestMain.cpp
)

target_link_libraries(VulkanPrebuiltTest
    PUBLIC
        ${_LIBRARY_NAME}
)
