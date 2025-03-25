file(GLOB SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/Source/Shared/*.c
    # ${CMAKE_CURRENT_SOURCE_DIR}/Source/Shared/libsamplerate/*.c
)

add_library(zelse_shared SHARED ${SOURCES})

set_target_properties(zelse_shared PROPERTIES LIBRARY_OUTPUT_DIRECTORY ${PD_OUTPUT_PATH})
set_target_properties(zelse_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${PD_OUTPUT_PATH})
set_target_properties(zelse_shared PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${PD_OUTPUT_PATH})
foreach(OUTPUTCONFIG ${CMAKE_CONFIGURATION_TYPES})
	    string(TOUPPER ${OUTPUTCONFIG} OUTPUTCONFIG)
		set_target_properties(zelse_shared PROPERTIES LIBRARY_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PD_OUTPUT_PATH})
		set_target_properties(zelse_shared PROPERTIES RUNTIME_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PD_OUTPUT_PATH})
		set_target_properties(zelse_shared PROPERTIES ARCHIVE_OUTPUT_DIRECTORY_${OUTPUTCONFIG} ${PD_OUTPUT_PATH})
endforeach(OUTPUTCONFIG CMAKE_CONFIGURATION_TYPES)

if(APPLE)
    target_link_options(zelse_shared PRIVATE -undefined dynamic_lookup)
elseif(WIN32)
    find_library(PD_LIBRARY NAMES pd HINTS ${PD_LIB_PATH})
	target_link_libraries(zelse_shared PRIVATE ws2_32 ${PD_LIBRARY})
endif()

if(PD_FLOATSIZE64)
    target_compile_definitions(zelse_shared PRIVATE PD_FLOATSIZE=64)
endif()

function(message)
    if (NOT MESSAGE_QUIET)
        _message(${ARGN})
    endif()
endfunction()

# message(STATUS "Configuring Opus")
# set(MESSAGE_QUIET ON)
# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/Source/Shared/opus)
# set(MESSAGE_QUIET OFF)
