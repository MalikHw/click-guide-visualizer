set(CMAKE_SYSTEM_NAME Windows)
set(CMAKE_SYSTEM_PROCESSOR AMD64)

set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

if (NOT DEFINED XWIN_DIR)
    if (DEFINED ENV{XWIN_DIR})
        set(XWIN_DIR $ENV{XWIN_DIR})
    else()
        message(FATAL_ERROR "XWIN_DIR is not set. Point it at an xwin splat output directory.")
    endif()
endif()

set(CMAKE_C_COMPILER clang-cl)
set(CMAKE_CXX_COMPILER clang-cl)
set(CMAKE_LINKER lld-link)
set(CMAKE_RC_COMPILER llvm-rc)
set(CMAKE_AR llvm-lib)
set(CMAKE_MT llvm-mt)

set(CMAKE_C_COMPILER_TARGET x86_64-pc-windows-msvc)
set(CMAKE_CXX_COMPILER_TARGET x86_64-pc-windows-msvc)

set(_CGV_MSVC_FLAGS "/imsvc${XWIN_DIR}/crt/include /imsvc${XWIN_DIR}/sdk/include/ucrt /imsvc${XWIN_DIR}/sdk/include/um /imsvc${XWIN_DIR}/sdk/include/shared /imsvc${XWIN_DIR}/sdk/include/winrt")
set(_CGV_MSVC_FLAGS "${_CGV_MSVC_FLAGS} /vctoolsdir ${XWIN_DIR}/crt /winsdkdir ${XWIN_DIR}/sdk")
set(_CGV_MSVC_FLAGS "${_CGV_MSVC_FLAGS} -Wno-unused-command-line-argument -Wno-msvc-not-found -D_CRT_SECURE_NO_WARNINGS")

set(CMAKE_C_FLAGS_INIT "${_CGV_MSVC_FLAGS}")
set(CMAKE_CXX_FLAGS_INIT "${_CGV_MSVC_FLAGS} /EHsc")

set(_CGV_LINK_FLAGS "/libpath:${XWIN_DIR}/crt/lib/x86_64 /libpath:${XWIN_DIR}/sdk/lib/um/x86_64 /libpath:${XWIN_DIR}/sdk/lib/ucrt/x86_64")

set(CMAKE_EXE_LINKER_FLAGS_INIT "${_CGV_LINK_FLAGS}")
set(CMAKE_SHARED_LINKER_FLAGS_INIT "${_CGV_LINK_FLAGS}")
set(CMAKE_MODULE_LINKER_FLAGS_INIT "${_CGV_LINK_FLAGS}")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)
