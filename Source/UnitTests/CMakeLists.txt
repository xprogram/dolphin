enable_testing()
add_custom_target(unittests)
add_custom_command(TARGET unittests POST_BUILD COMMAND ${CMAKE_CTEST_COMMAND})

string(APPEND CMAKE_RUNTIME_OUTPUT_DIRECTORY "/Tests")

# Use the node.js executable to run tests because the Emscripten browser test runner (at least on
# Windows) doesn't seem to work properly since https://github.com/emscripten-core/emscripten/pull/10046
# Besides, even if it did work, this is cleaner since we don't randomly create browser windows during testing...
if(EMSCRIPTEN)
  find_program(CROSSCOMPILING_TEST_RUNNER NAMES node nodejs
    HINTS
    $ENV{NODE_DIR}
    PATH_SUFFIXES bin
    DOC "Node.js binary for running tests built with Emscripten")

  if(NOT EXISTS ${CROSSCOMPILING_TEST_RUNNER})
    message(FATAL_ERROR "Could not find the Node.js executable, which is required to run tests when using Emscripten!")
  endif()

  # Make sure it's a version that supports wasm
  # TODO: also accept executables that are known to support required wasm extensions without these flags
  set(TEST_RUNNER_EXECUTION_FLAGS "--experimental-wasm-threads;--experimental-wasm-bulk-memory")
  execute_process(COMMAND ${CROSSCOMPILING_TEST_RUNNER} ${TEST_RUNNER_EXECUTION_FLAGS} -e "console.log(process.versions.node)"
    RESULT_VARIABLE TEST_RUNNER_VALIDATION_RUN_RESULT OUTPUT_VARIABLE CROSSCOMPILING_TEST_RUNNER_VER)
  
  string(STRIP "${CROSSCOMPILING_TEST_RUNNER_VER}" CROSSCOMPILING_TEST_RUNNER_VER)

  if(TEST_RUNNER_VALIDATION_RUN_RESULT)
    message(FATAL_ERROR "Could not find a Node.js interpreter that can be used to run the tests."
        " Please ensure you are using the one that comes with Emscripten.")
  endif()

  message(STATUS "Found Node.js version ${CROSSCOMPILING_TEST_RUNNER_VER} for running the test suite")
endif()

# Since this is a Core dependency, it can't be linked as a normal library.
# Otherwise CMake inserts the library after core, but before other core
# dependencies like videocommon which also use Host_ functions, which makes the
# GNU linker complain.
add_library(unittests_stubhost OBJECT StubHost.cpp)

if(EMSCRIPTEN)
  file(WRITE "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTests.boot.js" "if(require('worker_threads').isMainThread)UnitTest()")
  add_library(testboot INTERFACE)
  em_link_extern_post_js_ex(testboot "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/UnitTests.boot.js")
endif()

macro(add_dolphin_test target)
  add_executable(${target} EXCLUDE_FROM_ALL
    ${ARGN}
    $<TARGET_OBJECTS:unittests_stubhost>
  )
  set_target_properties(${target} PROPERTIES FOLDER Tests)
  target_link_libraries(${target} PRIVATE core uicommon gtest_main)
  add_dependencies(unittests ${target})

  if(EMSCRIPTEN)
    target_link_libraries(${target} PRIVATE
      testboot
    
      # Some application main() functions will block Node's
      # event loop instead of yielding to allow external processing
      # to occur, so it must be run inside a worker thread to avoid
      # locking up the entire process
      -sPROXY_TO_PTHREAD
      
      # Allow the application to exit instead of leaving the
      # runtime active after the program completes execution
      -sEXIT_RUNTIME
      
      -sEXPORT_NAME=UnitTest)

    add_test(NAME ${target}
      COMMAND ${CROSSCOMPILING_TEST_RUNNER} ${TEST_RUNNER_EXECUTION_FLAGS} $<TARGET_FILE:${target}>
      WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
  else()
    add_test(NAME ${target} COMMAND ${target})
  endif()
endmacro()

add_subdirectory(Common)
add_subdirectory(Core)
add_subdirectory(VideoCommon)
