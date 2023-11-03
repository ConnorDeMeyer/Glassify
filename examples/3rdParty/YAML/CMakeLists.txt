set(PROJECT_NAME Yaml)

################################################################################
# Source groups
################################################################################
set(Source_Files
    "src/binary.cpp"
    "src/contrib/graphbuilder.cpp"
    "src/contrib/graphbuilderadapter.cpp"
    "src/convert.cpp"
    "src/depthguard.cpp"
    "src/directives.cpp"
    "src/emit.cpp"
    "src/emitfromevents.cpp"
    "src/emitter.cpp"
    "src/emitterstate.cpp"
    "src/emitterutils.cpp"
    "src/exceptions.cpp"
    "src/exp.cpp"
    "src/memory.cpp"
    "src/node.cpp"
    "src/node_data.cpp"
    "src/nodebuilder.cpp"
    "src/nodeevents.cpp"
    "src/null.cpp"
    "src/ostream_wrapper.cpp"
    "src/parse.cpp"
    "src/parser.cpp"
    "src/regex_yaml.cpp"
    "src/scanner.cpp"
    "src/scanscalar.cpp"
    "src/scantag.cpp"
    "src/scantoken.cpp"
    "src/simplekey.cpp"
    "src/singledocparser.cpp"
    "src/stream.cpp"
    "src/tag.cpp"
)
source_group("Source Files" FILES ${Source_Files})

set(ALL_FILES
    ${Source_Files}
)

################################################################################
# Target
################################################################################
add_library(${PROJECT_NAME} STATIC ${ALL_FILES})

use_props(${PROJECT_NAME} "${CMAKE_CONFIGURATION_TYPES}" "${DEFAULT_CXX_PROPS}")
set(ROOT_NAMESPACE Yaml)

set_target_properties(${PROJECT_NAME} PROPERTIES
    VS_GLOBAL_KEYWORD "Win32Proj"
)
################################################################################
# Output directory
################################################################################
set_target_properties(${PROJECT_NAME} PROPERTIES
    OUTPUT_DIRECTORY_DEBUG   "${CMAKE_BINARY_DIR}/lib/$<CONFIG>/$ENV{PlatformTarget}/"
    OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/lib/$<CONFIG>/$ENV{PlatformTarget}/"
	INTERPROCEDURAL_OPTIMIZATION_RELEASE "TRUE"
)

################################################################################
# Include directories
################################################################################
target_include_directories(${PROJECT_NAME} PUBLIC
    "${CMAKE_CURRENT_SOURCE_DIR}/include;"
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
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
        /W3;
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

