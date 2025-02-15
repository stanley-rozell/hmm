cmake_minimum_required(VERSION 3.16 FATAL_ERROR)

# set project name and version (MAJOR.minor.patch)
project(hmm VERSION 0.2.0 LANGUAGES CXX C)
# some name variants I will be using:
set(CMAKE_PROJECT_NAME_CAP HMM)#all caps

# TODO(mfbolus): Update
set(PROJECT_URL https://github.com/stanley-rozell/hmm)

# WINDOWS WARNING
if (WIN32)
message(WARNING "Build/Use of this library on Windows is untested and
  will likely not work properly. If this is the case,
  please file an issue and/or make a pull request @${PROJECT_URL} .")
endif()

# Specify C++ standard
# n.b. this has to be before adding targets
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
message(STATUS "CMAKE_CXX_STANDARD = ${CMAKE_CXX_STANDARD}")

# Add project cmake modules to module path
set(CMAKE_MODULE_PATH
  ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake/Modules/")
include(util)
# dump_cmake_variables()

# CMake-provided
# provides: CHECK_INCLUDE_FILE_CXX(<include> <variable> [<flags>])
include(CheckIncludeFileCXX)

# provides: CHECK_LIBRARY_EXISTS(LIBRARY FUNCTION LOCATION VARIABLE)
include(CheckLibraryExists)

# Let user know where things getting installed since they did not specify
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  message(STATUS "*** CMAKE_INSTALL_PREFIX was initalised by cmake
    to the default value of ${CMAKE_INSTALL_PREFIX} ...")
endif()

# There may be differences in how you handle this as a function of OS
if(APPLE)
  message(STATUS "Detected macOS")
  # set(CMAKE_MACOSX_RPATH 1)
  set(CMAKE_INSTALL_RPATH "@executable_path/../lib")
elseif(UNIX)
  message(STATUS "Detected Unix")
  set(CMAKE_INSTALL_RPATH "$ORIGIN/../lib")
  # # from armadillo re: linux (ommitting as not explicitly using mkl here):
  # # "necessary when linking with Intel MKL on Linux systems"
  # set(CMAKE_SHARED_LINKER_FLAGS
  # "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-as-needed")
endif()

# Adhere to GNU filesystem layout conventions
# (see install config below)
include(GNUInstallDirs)

################################################################################
############ OPTIONS
################################################################################
# Hard-coded for usage later with clangd:
set(CMAKE_EXPORT_COMPILE_COMMANDS true)

# user settings:
message(STATUS "CMAKE_SYSTEM_NAME          = ${CMAKE_SYSTEM_NAME}"         )
message(STATUS "CMAKE_CXX_COMPILER_ID      = ${CMAKE_CXX_COMPILER_ID}"     )
message(STATUS "CMAKE_CXX_COMPILER_VERSION = ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS "")

################################################################################
############ Find Libraries
################################################################################

# For each dependecy, check whether exists and get
# include directories (list PROJECT_REQUIRED_INCLUDE_DIRS),
# libraries (list PROJECT_REQUIRED_LIBRARIES), etc.
set(PROJECT_REQUIRED_CXX_FLAGS "${CMAKE_CXX_FLAGS}") #CXXFLAGS required by dependencies
set(PROJECT_REQUIRED_INCLUDE_DIRS ";") #include dirs ""
set(PROJECT_REQUIRED_LIBRARY_DIRS ";") #library dirs ""
set(PROJECT_REQUIRED_LIBRARIES ";") # libraries ""

# Only non-std dependency is optional:
# for building documentation`
find_package(Doxygen COMPONENTS dot
             OPTIONAL_COMPONENTS mscgen dia)

# save the CXX flags configured for later use by dependency.
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${PROJECT_REQUIRED_CXX_FLAGS}")

# Set optimization level
# (have to check if a compiler that supports)
if(CMAKE_COMPILER_IS_GNUCXX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
else()
  if(DEFINED CMAKE_CXX_COMPILER_ID)
    if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
    elseif(CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
      set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2")
    endif()
  endif()
endif()

# after getting these lists, take out duplicates
list(REMOVE_DUPLICATES CMAKE_CXX_FLAGS)
list(REMOVE_DUPLICATES PROJECT_REQUIRED_CXX_FLAGS)
list(REMOVE_DUPLICATES PROJECT_REQUIRED_INCLUDE_DIRS)
list(REMOVE_DUPLICATES PROJECT_REQUIRED_LIBRARY_DIRS)
list(REMOVE_DUPLICATES PROJECT_REQUIRED_LIBRARIES)

message(STATUS "")
message(STATUS "*** Project will use the following libraries:")
message(STATUS "*** PROJECT_REQUIRED_LIBRARIES = ${PROJECT_REQUIRED_LIBRARIES}")
message(STATUS "")

# With variables/includes set, configure the template header,
# copy the headers, and add include directories to entire project.
message(STATUS "Copying ${PROJECT_SOURCE_DIR}/include/ to
  ${PROJECT_BINARY_DIR}/include/")
file(
  COPY ${PROJECT_SOURCE_DIR}/include/hmm_h/
  DESTINATION ${PROJECT_BINARY_DIR}/include/hmm_h/
  PATTERN "*mex*" EXCLUDE)
message(STATUS "Generating ${PROJECT_BINARY_DIR}/include/hmm.h")
configure_file(${PROJECT_SOURCE_DIR}/include/hmm.h.in
  ${PROJECT_BINARY_DIR}/include/hmm.h)

# cf target_include_directories()
include_directories(${PROJECT_BINARY_DIR}/include/
  ${PROJECT_REQUIRED_INCLUDE_DIRS})

# Finished configuring: print status
message(STATUS "CMAKE_CXX_FLAGS               = ${CMAKE_CXX_FLAGS}")
message(STATUS "PROJECT_REQUIRED_INCLUDE_DIRS = \
  ${PROJECT_REQUIRED_INCLUDE_DIRS}")

################################################################################
############ Add Targets
################################################################################
# library itself
add_library(${CMAKE_PROJECT_NAME} SHARED)
target_link_libraries(${CMAKE_PROJECT_NAME}
  PUBLIC ${PROJECT_REQUIRED_LIBRARIES})
target_link_directories(${CMAKE_PROJECT_NAME}
  PUBLIC ${PROJECT_REQUIRED_LIBRARY_DIRS})
target_include_directories(${CMAKE_PROJECT_NAME} INTERFACE
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/include>
  $<INSTALL_INTERFACE:include>)
set_target_properties(${CMAKE_PROJECT_NAME} PROPERTIES VERSION
  ${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}
  SOVERSION ${CMAKE_PROJECT_VERSION_MAJOR})

add_subdirectory(src) #add source files...

# build examples
file(
    COPY ${PROJECT_SOURCE_DIR}/examples/
    DESTINATION ${PROJECT_BINARY_DIR}/examples/
    PATTERN "*.cpp" EXCLUDE
    PATTERN "*.txt" EXCLUDE
    )
add_subdirectory(examples)

# build doxygen documention
# build documentation
if(DOXYGEN_FOUND)
  set(DOXYGEN_FULL_PATH_NAMES "NO")
  set(DOXYGEN_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/doxygen")
  set(DOXYGEN_HTML_OUTPUT "${PROJECT_BINARY_DIR}/doxygen/html")
  set(DOXYGEN_GENERATE_XML "YES")
  set(DOXYGEN_XML_OUTPUT "${PROJECT_BINARY_DIR}/doxygen/xml")
  set(DOXYGEN_EXAMPLE_PATH "examples")
  set(DOXYGEN_EXAMPLE_PATTERNS "*.cpp")
  set(DOXYGEN_EXCLUDE_PATTERNS "*.m")

  doxygen_add_docs(doxygen
      ${PROJECT_BINARY_DIR}/include/ ${PROJECT_SOURCE_DIR}/src/ ${PROJECT_SOURCE_DIR}/examples/
      ALL)
elseif()
  message("Doxygen not found. Will not generate docs ... ")
endif()

################################################################################
############ Install Configuration
################################################################################
message(STATUS ""                                                      )
message(STATUS "CMAKE_INSTALL_PREFIX     = ${CMAKE_INSTALL_PREFIX}"    )
message(STATUS "CMAKE_INSTALL_LIBDIR     = ${CMAKE_INSTALL_LIBDIR}"    )
message(STATUS "CMAKE_INSTALL_INCLUDEDIR = ${CMAKE_INSTALL_INCLUDEDIR}")
message(STATUS "CMAKE_INSTALL_DATADIR    = ${CMAKE_INSTALL_DATADIR}"   )
message(STATUS "CMAKE_INSTALL_BINDIR     = ${CMAKE_INSTALL_BINDIR}"    )

# add installation includedir
list(APPEND PROJECT_REQUIRED_INCLUDE_DIRS
  ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_INCLUDEDIR})
list(REMOVE_DUPLICATES PROJECT_REQUIRED_INCLUDE_DIRS)

# Note that the trailing / character in "include/" is critical
install(DIRECTORY ${PROJECT_BINARY_DIR}/include/
  DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
  PATTERN ".svn" EXCLUDE
  PATTERN "*.cmake" EXCLUDE
  PATTERN "*~" EXCLUDE
  PATTERN "*orig" EXCLUDE
)

install(
  TARGETS ${CMAKE_PROJECT_NAME}
  EXPORT ${CMAKE_PROJECT_NAME}#-> file <export-name>.cmake by default
  ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

# Export the package for use from outside the build-tree
export(PACKAGE ${CMAKE_PROJECT_NAME})

# provides `configure_package_config_file`,
# `write_basic_package_version_file`
include(CMakePackageConfigHelpers)

# Create {Project}Config.cmake file
message(STATUS "Generating
  '${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake'")
configure_package_config_file(
  ${CMAKE_CURRENT_SOURCE_DIR}/cmake/InstallFiles/Config.cmake.in
  "${CMAKE_CURRENT_BINARY_DIR}/InstallFiles/${CMAKE_PROJECT_NAME}Config.cmake"
  INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/CMake"
)

# generate the version file for the config file
message(STATUS "Generating '${PROJECT_BINARY_DIR}/${CMAKE_PROJECT_NAME}ConfigVersion.cmake'")
write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/InstallFiles/${CMAKE_PROJECT_NAME}ConfigVersion.cmake"
  VERSION "${CMAKE_PROJECT_VERSION_MAJOR}.${CMAKE_PROJECT_VERSION_MINOR}.${CMAKE_PROJECT_VERSION_PATCH}"
  COMPATIBILITY AnyNewerVersion)

# Install the export set for use with the install-tree
install(EXPORT ${CMAKE_PROJECT_NAME}
  DESTINATION "${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/CMake")

# Install files to be found by cmake users with find_package()
install(FILES
  "${PROJECT_BINARY_DIR}/InstallFiles/${CMAKE_PROJECT_NAME}Config.cmake"
  "${PROJECT_BINARY_DIR}/InstallFiles/${CMAKE_PROJECT_NAME}ConfigVersion.cmake"
  DESTINATION "${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/CMake")

#Install examples
install(
      DIRECTORY ${PROJECT_BINARY_DIR}/examples/
      DESTINATION ${CMAKE_INSTALL_DATADIR}/${CMAKE_PROJECT_NAME}/examples
      USE_SOURCE_PERMISSIONS
      PATTERN "*.cmake" EXCLUDE
      PATTERN ".*" EXCLUDE
      PATTERN "Makefile" EXCLUDE
      PATTERN "CMakeFiles" EXCLUDE
      PATTERN "*.mat" EXCLUDE
      PATTERN "*.h5" EXCLUDE
)


enable_testing()

# does the application run
add_test(NAME RunTest COMMAND hmm)

# does the usage message work?
#add_test(NAME UsageTest COMMAND hmm)

# add_test(NAME UsageTest COMMAND hmm)
# set_tests_properties(UsageTest
#   PROPERTIES PASS_REGULAR_EXPRESSION "UsageTest:.*number"
#   )

function(do_test tname target arg result)
    add_test(NAME ${tname} COMMAND ${target} ${arg})
    set_tests_properties(${tname}
      PROPERTIES PASS_REGULAR_EXPRESSION ${result}
      )
endfunction(do_test)


#looks for this string in the answer!
set(TEST_ANS "guessed states")

do_test(2state hmm 2 ${TEST_ANS})
do_test(3state hmm 3 ${TEST_ANS})

add_test(NAME Stress_2_1k COMMAND hmm 2 1000 PROPERTIES PASS_REGULAR_EXPRESSION ${TEST_ANS})
add_test(NAME Stress_3_50k COMMAND hmm 3 50000 PROPERTIES PASS_REGULAR_EXPRESSION ${TEST_ANS})
