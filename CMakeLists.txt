# v3.2 is the largest required cmake minimum version for my dependencies
# if the user is using a lower-version cmake, then they will receive a more instructive message (fail-early rather than continuing processing/generation and getting to an unsupported feature somewhere down the list)
# https://stackoverflow.com/questions/35550116/which-cmake-version-as-the-minimum
# v3.10 required for C++17 MSVC fix
# v3.11 required for CMP0072 policy
cmake_minimum_required(VERSION 3.11 FATAL_ERROR)

# reference: https://cmake.org/cmake/help/v3.0/policy/CMP0025.html#policy:CMP0025
if(POLICY CMP0025)
    # compiler id for AppleClang is now 'AppleClang' rather than OLD (default) policy of 'Clang'
    cmake_policy(SET CMP0025 NEW)
endif()

# reference: https://cmake.org/cmake/help/v3.11/policy/CMP0072.html
if(POLICY CMP0072)
    # force FindOpenGL to use GLVND by default (if multiple OpenGL libs are installed, it prefers to use the new one)
    cmake_policy(SET CMP0072 NEW)
endif()

# notably sets the PROJECT_NAME variable, C/CXX as default build-languages, but i'll overwrite it to just target CXX
# https://cmake.org/cmake/help/latest/command/project.html
# note: including 'C' as a language is necessary to prevent some "CMAKE MISSING INTERNAL VARIABLES ERRORS" that I was getting on Linux
# reference: https://github.com/google/iree/issues/229
project(wave-tool LANGUAGES CXX C)

# use C++17 features (globally, for all subsequent targets)
# https://crascit.com/2015/03/28/enabling-cxx11-in-cmake/
# https://stackoverflow.com/questions/44960715/how-to-enable-stdc17-in-vs2017-with-cmake
set(CMAKE_CXX_STANDARD 17) # C++17 MSVC fix in CMake 3.10
set(CMAKE_CXX_STANDARD_REQUIRED ON) # prevent default fallback to an older standard if compiler doesn't support C++17
set(CMAKE_CXX_EXTENSIONS OFF) # only use the official standards (e.g. for g++, -std=c++17 and not -std=gnu++17)

# https://docs.microsoft.com/en-us/cpp/build/reference/md-mt-ld-use-run-time-library?view=vs-2019
# https://stackoverflow.com/questions/14172856/compile-with-mt-instead-of-md-using-cmake
# https://stackoverflow.com/questions/20800166/cmake-compile-with-mt-instead-of-md
# https://stackoverflow.com/questions/31037882/whats-the-cmake-syntax-to-set-and-use-variables
# https://cmake.org/cmake/help/v3.0/command/set.html
# for MSVC, switch to multi-threaded static linking of MSVCRT runtime library
# DEPRECATED due to GLFW being compiled with /MD no matter what (future solution would be to fork GLFW and fix this, since they don't seem to want to allow us to use /MT)
# reference: https://github.com/glfw/glfw/issues/528
#if(MSVC)
#    # init CompilerFlags variable as a list of these terms found in the cmake cache file
#    set(CompilerFlags
#        CMAKE_CXX_FLAGS
#        CMAKE_CXX_FLAGS_DEBUG
#        CMAKE_CXX_FLAGS_MINSIZEREL
#        CMAKE_CXX_FLAGS_RELEASE
#        CMAKE_CXX_FLAGS_RELWITHDEBINFO
#    )
#    # check each of these cache entries and replace any /MD with /MT and /MDd with /MTd
#    # note: since these variables are already in the cache, the pattern set(... CACHE ... FORCE) must be used to overwrite it
#    foreach(CompilerFlag ${CompilerFlags})
#        if(${CompilerFlag} MATCHES "/MD")
#            string(REPLACE "/MD" "/MT" ${CompilerFlag} "${${CompilerFlag}}")
#            set(${CompilerFlag} "${${CompilerFlag}}" CACHE STRING "msvc compiler flags" FORCE)
#        endif()
#        if(${CompilerFlag} MATCHES "/MDd")
#            string(REPLACE "/MDd" "/MTd" ${CompilerFlag} "${${CompilerFlag}}")
#            set(${CompilerFlag} "${${CompilerFlag}}" CACHE STRING "msvc compiler flags" FORCE)
#        endif()
#    endforeach()
#endif()

# OpenGL...
# reference: https://shot511.github.io/2018-05-29-how-to-setup-opengl-project-with-cmake/
find_package(OpenGL REQUIRED)

# GLFW...
# reference: https://www.glfw.org/docs/latest/build_guide.html#build_link_cmake_source
# reference: https://github.com/glfw/glfw/blob/master/CMakeLists.txt
# reference: https://github.com/wjakob/nanogui/blob/master/CMakeLists.txt
# reference: https://github.com/aaronmjacobs/InitGL/blob/master/CMakeLists.txt
# be explicit with options...
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLFW_VULKAN_STATIC OFF CACHE BOOL "" FORCE)
# fork off and build GLFW...
add_subdirectory("${CMAKE_CURRENT_SOURCE_DIR}/deps/glfw")

# glad...
# reference: https://github.com/Dav1dde/glad/issues/186
# note: I used the web loader/generator at...
# reference: https://glad.dav1d.de/
# note: you can see what build settings I used at the top of glad.c/h
# note: I don't generate a loader, since I use glfw instead
# note: it seems like you get syntax errors in glad.c now if you omit khr (missing typedefs), so leave it be (possibly similar to below link?)
# reference: https://github.com/KhronosGroup/OpenGL-Registry/issues/162
# note: OpenGL profile 3.3 is good for most systems, 4.1 is max supported by macOS (Apple has deprecated OpenGL and will remove in future), otherwise use a higher profile for more features
# reference: https://support.apple.com/en-ca/HT202823
add_library(glad STATIC "${CMAKE_CURRENT_SOURCE_DIR}/deps/glad/src/glad.c" "${CMAKE_CURRENT_SOURCE_DIR}/deps/glad/include/glad/glad.h" "${CMAKE_CURRENT_SOURCE_DIR}/deps/glad/include/KHR/khrplatform.h")
target_include_directories(glad PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/deps/glad/include")

# GLM...
# header-only so nothing to do here

# stb...
# header-only so nothing to do here

# Dear ImGui...
# reference: https://retifrav.github.io/blog/2019/08/04/glfw-dear-imgui/
file(GLOB WAVE_TOOL_DEAR_IMGUI_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/*.h")
list(APPEND WAVE_TOOL_DEAR_IMGUI_SOURCE_FILES "${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/examples/imgui_impl_glfw.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/examples/imgui_impl_glfw.h" "${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/examples/imgui_impl_opengl3.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui/examples/imgui_impl_opengl3.h")
add_library(dear-imgui STATIC ${WAVE_TOOL_DEAR_IMGUI_SOURCE_FILES})
# note: must include GLFW and glad, since that is the implementation we are using
target_include_directories(dear-imgui PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/deps/imgui" "${CMAKE_CURRENT_SOURCE_DIR}/deps/glfw/include" "${CMAKE_CURRENT_SOURCE_DIR}/deps/glad/include")
# note: we don't have to link GLFW/glad here with dear-imgui, since everything will be linked later to make our executable
# this definition is required to fix compile errors where it tries to find GLEW by default on certain platforms (e.g. Linux)
# reference: https://stackoverflow.com/questions/57535712/how-to-structure-a-cmake-project-with-different-3rd-party-libraries
target_compile_definitions(dear-imgui PRIVATE IMGUI_IMPL_OPENGL_LOADER_GLAD)

# Boost...
# using only the header-only parts so nothing to do here

# reference: https://stackoverflow.com/questions/35411489/add-all-files-under-a-folder-to-a-cmake-glob
# reference: https://stackoverflow.com/questions/7533502/how-can-i-merge-multiple-lists-of-files-together-with-cmake
# reference: https://stackoverflow.com/questions/15550777/how-do-i-exclude-a-single-file-from-a-cmake-fileglob-pattern
# note: headers aren't explicitly needed to be specified here to the target since they will be found anyway, but it's convenient to include them since it allows IDEs to see them
# reference: https://cmake.org/cmake/help/v3.0/command/file.html
# reference: https://stackoverflow.com/questions/1027247/is-it-better-to-specify-source-files-with-glob-or-each-file-individually-in-cmak/18538444
# VERY IMPORTANT note: globbing source files is not recommended by cmake since your build system won't be able to detect a change to this file's timestamp when a new source file is added
# thus, your build system won't automatically regenerate to recognize the new file
# this is especially problematic if you are working with other devs and pull down their changes (where they added new files), resulting in unintuitive build errors
# the fix for this is to manually run "touch CMakeLists.txt" to update the timestamp yourself after you create new source files or after you pull down changes from other devs (if they added new files)
# MY RATIONALE FOR DOING IT THIS WAY RATHER THAN MANUALLY LISTING ALL SOURCE FILES...
# 1. it seems like the most "automatic" way of working, albeit not 100% automatic (only problem is to remember to run touch utility)
# 2. the file name exists only in 1 place (on disk), so renaming/deleting a file won't have to also be done in this script
# 3. way shorter cmake script, especially for many source file projects
# 4. in both ways, you have to do something manually, and I feel a "touch" is easier than writing a bunch of filenames here in this script
# 5. globbing could result in false positives, but these could be rare and can be fixed with REMOVE_ITEM or partial manual listing
# note: apparently cmake 3.12 added a CONFIGURE_DEPENDS option that reruns cmake when glob value changes, but this seems like it could be problematic with different generators and could slow down the build pipeline a bit
file(GLOB_RECURSE WAVE_TOOL_SOURCE_FILES_IN_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cc" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cxx" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.c++" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hh" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.hpp" "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h++")
# note: you will also have to add paths to any dependency sources only when they are required to be built directly with your files
list(APPEND WAVE_TOOL_ALL_SOURCE_FILES ${WAVE_TOOL_SOURCE_FILES_IN_SRC_DIR})
message(STATUS "main target source files = ${WAVE_TOOL_ALL_SOURCE_FILES}")
# adds an executable target called <wave-tool> to be built from the source files listed
# the source files can be removed from here and specified later using target_sources()
# https://cmake.org/cmake/help/latest/command/add_executable.html
# warning: use CMAKE_CURRENT_SOURCE_DIR rather than CMAKE_SOURCE_DIR since the the latter points to the src directory of the parent project. So, if my project is a sub-project of another then mine won't build
# https://github.com/KhronosGroup/Vulkan-LoaderAndValidationLayers/issues/1428
add_executable(wave-tool ${WAVE_TOOL_ALL_SOURCE_FILES})

# reference: https://pabloariasal.github.io/2018/02/19/its-time-to-do-cmake-right/
# SCOPE SPECIFIERS...
# PRVATE - for internal building of target
# INTERFACE - for external users (usage requirements) of target
# PUBLIC = both PRIVATE and INTERFACE
# thus the include/ directory is PUBLIC since it is needed for both internally building target and external users to possess
# and src/ is PRIVATE since only private headers reside here and are only needed for internal building
target_include_directories(wave-tool PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/boost/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/glad/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/glfw/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps/glm"
    "${CMAKE_CURRENT_SOURCE_DIR}/deps"
)

# https://foonathan.net/blog/2018/10/17/cmake-warnings.html
# set the warning flags for the respective compilers using cmake generator expressions
# (removed, since this causes build nightmares when using warning-heavy libs) using -Werror and /WX to treat warnings as compile errors (ensures warnings get either fixed or suppressed right away rather than getting hidden in files that never get compiled again)
# using -Wall + -Wextra and /W4 to provide a sufficient amount of general warnings
# note: /Wall on MSVC is practically unusable due to the enormous amount of warnings generated by the standard library
# note: if additional warning flags are to be added, I believe the project would have to be clean-rebuilt to apply the new warning detection to previously compiled files
# note: for some reason, the cmakecache file doesn't reflect these changes (e.g. still says default of /W3), but they seem to still apply correctly
# DEPRECATED (cotire doesn't work well will some generator expressions, like the ones below...)
#target_compile_options(wave-tool PRIVATE
#    $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>,$<CXX_COMPILER_ID:GNU>>:
#        -Wall -Wextra>
#        # opt. could add -Werror (if your libs don't throw so many warnings or you have good warning policy handling)
#    $<$<CXX_COMPILER_ID:MSVC>:
#        /W4>
#        # opt. could add /WX (if your libs don't throw so many warnings or you have good warning policy handling)
#)

# reference: https://stackoverflow.com/questions/10046114/in-cmake-how-can-i-test-if-the-compiler-is-clang
if(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang" OR CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    target_compile_options(wave-tool PRIVATE -Wall -Wextra)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    target_compile_options(wave-tool PRIVATE /W4)
endif()

# reference: https://cmake.org/cmake/help/v3.10/module/FindOpenGL.html
# reference: https://www.glfw.org/docs/latest/build_guide.html#build_link_cmake_source
# I think that the order matters in some cases (i've seen that a lib on the left depends on a lib to the right of it)
target_link_libraries(wave-tool PRIVATE dear-imgui glad glfw OpenGL::GL)

if(MSVC)
    # reference: https://stackoverflow.com/questions/7304625/how-do-i-change-the-startup-project-of-a-visual-studio-solution-via-cmake
    # sets the startup project in the Visual Studio solution (so that user doesn't have to explicitly right click target and set option)
    set_property(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}" PROPERTY VS_STARTUP_PROJECT wave-tool)

    # reference: https://stackoverflow.com/questions/23950887/does-cmake-offer-a-method-to-set-the-working-directory-for-a-given-build-system
    # reference: https://stackoverflow.com/questions/55713475/correct-use-of-vs-debugger-working-directory-etc
    # sets the working directory root for all relative paths (e.g. for asset referencing) when running in Visual Studio
    # by setting this to the same directory as where the executable is built, the program can be run either directly with the .exe or in VS
    set_property(TARGET wave-tool PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:wave-tool>")
endif()
