cmake_minimum_required(VERSION 3.12)

project(vulkan-prebuilt VERSION 1.0.0)

# Add interface library
add_library(vulkan-prebuilt
    INTERFACE
)

if (CMAKE_CROSSCOMPILING)
    if (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Darwin")
        set(VULKAN_PREBUILT_BIN_GLSLC sdk/ios_tvos-only/bin/glslc)
        set(VULKAN_PREBUILT_BIN_SPIRV_CROSS sdk/ios_tvos-only/bin/spirv-cross)
    elseif (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Windows")
        set(VULKAN_PREBUILT_BIN_GLSLC sdk/windows/Bin/glslc.exe)
        set(VULKAN_PREBUILT_BIN_SPIRV_CROSS sdk/windows/Bin/spirv-cross.exe)
    elseif (${CMAKE_HOST_SYSTEM_NAME} MATCHES "Linux")
        set(VULKAN_PREBUILT_BIN_GLSLC sdk/linux/bin/glslc)
        set(VULKAN_PREBUILT_BIN_SPIRV_CROSS sdk/linux/bin/spirv-cross)
    else()
        message(FATAL_ERROR "Usage of this prebuilt library is not supported on this host platform.")
    endif()

    # Per non-development platform configuration
    if (IOS OR TVOS)
        # The iOS/tvOS libraries must be lipo'ed together when using legacy build systems.
        if (IOS)
            set(_LIBS ios-arm64 ios-x86_64-simulator)
        else()
            set(_LIBS tvos-arm64 tvos-x86_64-simulator)
        endif()
        list(TRANSFORM _LIBS PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/sdk/ios_tvos-only/MoltenVK/MoltenVK.xcframework/)
        list(TRANSFORM _LIBS APPEND /libMoltenVK.a)

        # On iOS, a 'dummy' symbol for armv7 must also be included. Vulkan is not available on armv7 -- this symbol is there only to prevent linker warnings.
        if (IOS)
            list(APPEND _LIBS ${CMAKE_CURRENT_SOURCE_DIR}/other/dummy_ios_armv7_symbol/lib/dummy.a)
        endif()

        add_custom_target(vulkan-prebuilt-lipo ALL
            COMMAND lipo -create ${_LIBS} -output ${CMAKE_CURRENT_BINARY_DIR}/libMoltenVK.a
            DEPENDS ${_LIBS}
            BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/libMoltenVK.a
            COMMENT "Combining arch libraries (${_LIBS}) into a single 'fat' library (${CMAKE_CURRENT_BINARY_DIR}/libModelVK.a)"
        )

        add_dependencies(vulkan-prebuilt
            vulkan-prebuilt-lipo
        )

        set(VULKAN_PREBUILT_HEADERS sdk/ios_tvos-only/MoltenVK/include/)
        set(VULKAN_PREBUILT_LIBRARY_PATH ${CMAKE_CURRENT_BINARY_DIR})
        set(VULKAN_PREBUILT_LIBRARY_NAME libMoltenVK.a)
        set(VULKAN_PREBUILT_DEPENDENT_LIBRARIES
            "-framework Metal"
        )
    endif()
else()
    # Per development platform configuration
    if (APPLE)
        # Fallback to building for MacOS
        set(VULKAN_PREBUILT_HEADERS sdk/macos-only/MoltenVK/include/)
        set(VULKAN_PREBUILT_LIBRARY_PATH sdk/macos-only/MoltenVK/MoltenVK.xcframework/macos-arm64_x86_64)
        set(VULKAN_PREBUILT_LIBRARY_NAME libMoltenVK.a)
        set(VULKAN_PREBUILT_DEPENDENT_LIBRARIES
            "-framework IOSurface"
            "-framework Metal"
            "-framework Cocoa"
            "-framework IOKit"
            "-framework QuartzCore"
        )
        set(VULKAN_PREBUILT_BIN_GLSLC sdk/macos-only/bin/glslc)
        set(VULKAN_PREBUILT_BIN_SPIRV_CROSS sdk/macos-only/bin/spirv-cross)
    elseif (WIN32 AND NOT WINDOWS_STORE)
        set(VULKAN_PREBUILT_HEADERS sdk/windows/Include/)
        set(VULKAN_PREBUILT_LIBRARY_PATH sdk/windows/Lib)
        set(VULKAN_PREBUILT_LIBRARY_NAME vulkan-1.lib)
        set(VULKAN_PREBUILT_DEPENDENT_LIBRARIES)
        set(VULKAN_PREBUILT_BIN_GLSLC sdk/windows/Bin/glslc.exe)
        set(VULKAN_PREBUILT_BIN_SPIRV_CROSS sdk/windows/Bin/spirv-cross.exe)
    elseif (UNIX)
        set(VULKAN_PREBUILT_HEADERS sdk/linux/include/)
        set(VULKAN_PREBUILT_LIBRARY_PATH sdk/linux/lib)
        set(VULKAN_PREBUILT_LIBRARY_NAME libvulkan.a)
        set(VULKAN_PREBUILT_DEPENDENT_LIBRARIES)
        set(VULKAN_PREBUILT_BIN_GLSLC sdk/linux/bin/glslc)
        set(VULKAN_PREBUILT_BIN_SPIRV_CROSS sdk/linux/bin/spirv-cross)
    endif()
endif()

# Configure installation variables
# Introduce variables:
# * CMAKE_INSTALL_LIBDIR
# * CMAKE_INSTALL_BINDIR
# * CMAKE_INSTALL_INCLUDEDIR
include(GNUInstallDirs)

set(GENERATED_DIR "${CMAKE_CURRENT_BINARY_DIR}/generated")
set(INSTALL_CONFIGDIR "${CMAKE_INSTALL_LIBDIR}/cmake/${PROJECT_NAME}")
set(VERSION_CONFIG "${GENERATED_DIR}/${PROJECT_NAME}ConfigVersion.cmake")
set(PROJECT_CONFIG "${GENERATED_DIR}/${PROJECT_NAME}Config.cmake")
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")
set(NAMESPACE "${PROJECT_NAME}::")

if (DEFINED VULKAN_PREBUILT_HEADERS)
    # Add header search paths
    target_include_directories(vulkan-prebuilt
        INTERFACE
            $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${VULKAN_PREBUILT_HEADERS}>
    )
endif()

if (DEFINED VULKAN_PREBUILT_LIBRARY_NAME)
    # Link required frameworks/libraries to the interface library
    target_link_libraries(vulkan-prebuilt
        INTERFACE
            # Linking for the install interface is done in Config.cmake.in
            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/${VULKAN_PREBUILT_LIBRARY_PATH}/${VULKAN_PREBUILT_LIBRARY_NAME}>
            ${VULKAN_PREBUILT_DEPENDENT_LIBRARIES}
    )
endif()

# build and install project config
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "cmake/Config.cmake.in"
    "${PROJECT_CONFIG}"
    INSTALL_DESTINATION "${INSTALL_CONFIGDIR}"
)

# build config version into bin directory
write_basic_package_version_file(
    "${VERSION_CONFIG}"
    COMPATIBILITY AnyNewerVersion
)

# install target
install(
    TARGETS vulkan-prebuilt
    EXPORT "${TARGETS_EXPORT_NAME}"
    LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    INCLUDES DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)

#install files
install(
    FILES
        "${VERSION_CONFIG}"
        "${PROJECT_CONFIG}"
    DESTINATION ${INSTALL_CONFIGDIR}
)

if (DEFINED VULKAN_PREBUILT_HEADERS)
    # install headers
    install(
        DIRECTORY
            ${VULKAN_PREBUILT_HEADERS}
        DESTINATION
            ${CMAKE_INSTALL_INCLUDEDIR}
    )
endif()

if (DEFINED VULKAN_PREBUILT_LIBRARY_NAME)
    # install prebuilt libraries
    install(
        FILES
            ${VULKAN_PREBUILT_LIBRARY_PATH}/${VULKAN_PREBUILT_LIBRARY_NAME}
        DESTINATION
            ${CMAKE_INSTALL_LIBDIR}
    )
endif()

# install vulkan bins
install(
    PROGRAMS
        ${VULKAN_PREBUILT_BIN_GLSLC}
        ${VULKAN_PREBUILT_BIN_SPIRV_CROSS}
    DESTINATION
        ${CMAKE_INSTALL_BINDIR}
)

install(
    EXPORT "${TARGETS_EXPORT_NAME}"
    NAMESPACE "${NAMESPACE}"
    DESTINATION "${INSTALL_CONFIGDIR}"
)
