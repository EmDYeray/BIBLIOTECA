cmake_minimum_required(VERSION 3.24)
if(NOT CMAKE_SYSTEM_NAME STREQUAL "iOS")
    message(FATAL_ERROR "Para Linux usa Makefile. Este target requiere el SDK iphoneos.")
endif()
project(Biblioteca VERSION 1.0.0 LANGUAGES C CXX OBJC)

include(FetchContent)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)
set(SDL_TEST OFF CACHE BOOL "" FORCE)
FetchContent_Declare(SDL2
    GIT_REPOSITORY https://github.com/libsdl-org/SDL.git
    GIT_TAG 5d249570393f7a37e037abf22cd6012a4cc56a71)
FetchContent_MakeAvailable(SDL2)

# Use only the iOS libraries from vcpkg and include their static dependencies.
set(PKG_CONFIG_USE_CMAKE_PREFIX_PATH FALSE)
set(PKG_CONFIG_ARGN --static)
find_package(PkgConfig REQUIRED)
pkg_check_modules(CAIRO REQUIRED IMPORTED_TARGET cairo freetype2)
find_library(OPENGLES_FRAMEWORK OpenGLES REQUIRED)
find_library(COREGRAPHICS_FRAMEWORK CoreGraphics REQUIRED)
find_library(CORETEXT_FRAMEWORK CoreText REQUIRED)

file(GLOB BOOK_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp")
list(FILTER BOOK_SOURCES EXCLUDE REGEX "/(main|desktop|checks)\\.cpp$")
add_executable(Biblioteca MACOSX_BUNDLE ios/main.cpp ${BOOK_SOURCES})
target_include_directories(Biblioteca PRIVATE src)
target_compile_features(Biblioteca PRIVATE cxx_std_20)
target_compile_definitions(Biblioteca PRIVATE BIBLIOTECA_IPAD=1)
target_link_directories(Biblioteca PRIVATE ${CAIRO_LIBRARY_DIRS})
target_link_libraries(Biblioteca PRIVATE SDL2::SDL2main SDL2::SDL2-static
    PkgConfig::CAIRO ${OPENGLES_FRAMEWORK} ${COREGRAPHICS_FRAMEWORK} ${CORETEXT_FRAMEWORK})

set(IPAD_BUNDLE_ID "org.cuentosentreestrellas.biblioteca" CACHE STRING "Bundle identifier")
set_target_properties(Biblioteca PROPERTIES
    MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/ios/Info.plist.in"
    XCODE_ATTRIBUTE_PRODUCT_BUNDLE_IDENTIFIER "${IPAD_BUNDLE_ID}"
    XCODE_ATTRIBUTE_TARGETED_DEVICE_FAMILY "2"
    XCODE_ATTRIBUTE_CODE_SIGNING_ALLOWED "NO")

foreach(folder IN ITEMS covers fonts materials popups shaders)
    add_custom_command(TARGET Biblioteca POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets/${folder}"
        "$<TARGET_BUNDLE_DIR:Biblioteca>/assets/${folder}"
        VERBATIM)
endforeach()
