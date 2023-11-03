set(PROJECT_NAME Inheritance)

################################################################################
# Source groups
################################################################################
set(Source_Files
    "main.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_executable(${PROJECT_NAME} ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE Inheritance)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/exe/$<CONFIG>/$ENV{PlatformTarget}/"
    OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/exe/$<CONFIG>/$ENV{PlatformTarget}/"
	INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)
################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/../../include"
)

################################################################################
# Compile definitions
################################################################################
if("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x64")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG;"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
		"YAML_CPP_STATIC_DEFINE"
		"GLAS_STORAGE"
		"GLAS_SERIALIZATION_BINARY"
		"GLAS_SERIALIZATION_JSON"
		"GLAS_SERIALIZATION_YAML"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
elseif("${CMAKE_VS_PLATFORM_NAME}" STREQUAL "x86")
    target_compile_definitions(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:"
            "_DEBUG;"
        ">"
        "$<$<CONFIG:Release>:"
            "NDEBUG"
        ">"
		"YAML_CPP_STATIC_DEFINE"
		"GLAS_STORAGE"
		"GLAS_SERIALIZATION_BINARY"
		"GLAS_SERIALIZATION_JSON"
		"GLAS_SERIALIZATION_YAML"
        "WIN32;"
        "_CONSOLE;"
        "UNICODE;"
        "_UNICODE"
    )
endif()

################################################################################
# Compile and link options
################################################################################
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /Oi;
            /Gy
        >
        $<$<CONFIG:Debug>:
            /Od
        >
        /permissive-;
        /sdl;
        /W4;
        /WX;
        ${DEFAULT_CXX_DEBUG_INFORMATION_FORMAT};
        ${DEFAULT_CXX_EXCEPTION_HANDLING}
    )
    target_link_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:
            /OPT:REF;
            /OPT:ICF
        >
        /DEBUG;
        /SUBSYSTEM:CONSOLE
    )
endif()

################################################################################
# Dependencies
################################################################################
add_dependencies(${PROJECT_NAME}
    Yaml
)

set(ADDITIONAL_LIBRARY_DEPENDENCIES
    #"$(CoreLibraryDependencies)"
    "Yaml"
)
target_link_libraries(${PROJECT_NAME} PRIVATE "${ADDITIONAL_LIBRARY_DEPENDENCIES}")

target_link_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_BINARY_DIR}/lib/$<CONFIG>/${CMAKE_VS_PLATFORM_NAME}"
)


