include_guard(GLOBAL)

# The following is forked from the Emscripten toolchain file.
# These functions needed support for the keyword signature
#############################################################

# Internal function: Do not call from user CMakeLists.txt files. Use one of
# em_link_js_library_ex()/em_link_pre_js_ex()/em_link_post_js_ex() instead.
function(em_add_tracked_link_flag_ex target flagname)

  # A counter to guarantee unique names for js library files.
  set(dolphin_link_js_counter 1)

  # User can input list of JS files either as a single list, or as variable
  # arguments to this function, so iterate over varargs, and treat each item in
  # varargs as a list itself, to support both syntax forms.
  foreach(jsFileList ${ARGN})
    foreach(jsfile ${jsFileList})
      # If the user edits the JS file, we want to relink the emscripten
      # application, but unfortunately it is not possible to make a link step
      # depend directly on a source file. Instead, we must make a dummy no-op
      # build target on that source file, and make the project depend on
      # that target.

      # Sanitate the source .js filename to a good symbol name to use as a dummy
      # filename.
      get_filename_component(jsname "${jsfile}" NAME)
      string(REGEX REPLACE "[/:\\\\.\ ]" "_" dummy_js_target ${jsname})
      set(dummy_lib_name ${target}_${dolphin_link_js_counter}_${dummy_js_target})
      set(dummy_c_name "${CMAKE_BINARY_DIR}/${dummy_js_target}_tracker.c")
      
      get_target_property(target_type ${target} TYPE)
      if(NOT (target_type STREQUAL "INTERFACE_LIBRARY"))
        get_target_property(target_all_exclusion ${target} EXCLUDE_FROM_ALL)
      endif()

      # Create a new static library target that with a single dummy .c file.
      if(target_all_exclusion)
        add_library(${dummy_lib_name} STATIC EXCLUDE_FROM_ALL ${dummy_c_name})
      else()
        add_library(${dummy_lib_name} STATIC ${dummy_c_name})
      endif()
      
      # Link the js-library to the target
      get_filename_component(js_file_absolute_path "${jsfile}" ABSOLUTE )
      target_link_libraries(${dummy_lib_name} PUBLIC "${flagname} \"${js_file_absolute_path}\"")
      
      # Make the dummy .c file depend on the .js file we are linking, so that if
      # the .js file is edited, the dummy .c file, and hence the static library
      # will be rebuild (no-op). This causes the main application to be
      # relinked, which is what we want.  This approach was recommended by
      # http://www.cmake.org/pipermail/cmake/2010-May/037206.html
      add_custom_command(OUTPUT ${dummy_c_name} COMMAND ${CMAKE_COMMAND} -E touch ${dummy_c_name} DEPENDS ${jsfile})

      if(target_type STREQUAL "EXECUTABLE")
        target_link_libraries(${target} PRIVATE ${dummy_lib_name})
      elseif(target_type STREQUAL "INTERFACE_LIBRARY")
        target_link_libraries(${target} INTERFACE ${dummy_lib_name})
      else()
        target_link_libraries(${target} PUBLIC ${dummy_lib_name})
      endif()

      math(EXPR dolphin_link_js_counter "${dolphin_link_js_counter} + 1")
    endforeach()
  endforeach()
endfunction()

# This function links a (list of ) .js library file(s) to the given CMake project.
# Example: em_link_js_library_ex(my_executable "lib1.js" "lib2.js")
#    will result in emcc passing --js-library lib1.js --js-library lib2.js to
#    the emscripten linker, as well as tracking the modification timestamp
#    between the linked .js files and the main project, so that editing the .js
#    file will cause the target project to be relinked.
function(em_link_js_library_ex target)
  em_add_tracked_link_flag_ex(${target} "--js-library" ${ARGN})
endfunction()

# This function is identical to em_link_js_library_ex(), except the .js files will
# be added with '--pre-js file.js' command line flag, which is generally used to
# add some preamble .js code to a generated output file.
function(em_link_pre_js_ex target)
  em_add_tracked_link_flag_ex(${target} "--pre-js" ${ARGN})
endfunction()

# This function is identical to em_link_js_library_ex(), except the .js files will
# be added with '--post-js file.js' command line flag, which is generally used
# to add some postamble .js code to a generated output file.
function(em_link_post_js_ex target)
  em_add_tracked_link_flag_ex(${target} "--post-js" ${ARGN})
endfunction()

function(em_link_extern_post_js_ex target)
  em_add_tracked_link_flag_ex(${target} "--extern-post-js" ${ARGN})
endfunction()
