add_executable(dolphin-nogui
  Platform.cpp
  Platform.h
  MainNoGUI.cpp
)

if(ENABLE_X11 AND X11_FOUND)
  target_sources(dolphin-nogui PRIVATE PlatformX11.cpp)
endif()

if(WIN32)
  target_sources(dolphin-nogui PRIVATE PlatformWin32.cpp)
endif()

if(${CMAKE_SYSTEM_NAME} STREQUAL "Linux")
  target_sources(dolphin-nogui PRIVATE PlatformFBDev.cpp)
endif()

if(EMSCRIPTEN)
  target_sources(dolphin-nogui PRIVATE PlatformWeb.cpp)
  
  target_link_libraries(dolphin-nogui PRIVATE
    lz4sysdir

    # Target only the web environment ("worker" is for pthreads), to eliminate unnecessary support code
    -sENVIRONMENT=web,worker
    
    # Extra flags for proper interop with external wrapping code
    -sNO_INVOKE_RUN
    -sEXIT_RUNTIME
    -sEXPORTED_RUNTIME_METHODS=['FS','callMain']

    -sALLOW_MEMORY_GROWTH
    -sEXPORT_NAME=DolphinNoGUI)
else()
  target_sources(dolphin-nogui PRIVATE PlatformHeadless.cpp)
endif()

set_target_properties(dolphin-nogui PROPERTIES OUTPUT_NAME dolphin-emu-nogui)

target_link_libraries(dolphin-nogui
PRIVATE
  core
  uicommon
  cpp-optparse
)

if(USE_DISCORD_PRESENCE)
  target_compile_definitions(dolphin-nogui PRIVATE -DUSE_DISCORD_PRESENCE)
endif()

set(CPACK_PACKAGE_EXECUTABLES ${CPACK_PACKAGE_EXECUTABLES} dolphin-nogui)
install(TARGETS dolphin-nogui RUNTIME DESTINATION ${bindir})

