cmake_minimum_required(VERSION 3.13)

project(YummyLife)

set(CMAKE_CXX_STANDARD 14)

option(TEST_BUILD "Enable test build features" OFF)
option(PREVIEW_BUILD "Enable preview build features" OFF)

if (TEST_BUILD)
    add_compile_definitions(TEST_BUILD=1)
endif()

if (PREVIEW_BUILD)
    add_compile_definitions(PREVIEW_BUILD=1)
endif()

# Read the version out of game.cpp so the mac bundle reports the same version
# the window title does, rather than keeping a second copy of it in here.
file(READ "${CMAKE_SOURCE_DIR}/gameSource/game.cpp" GAME_CPP_HEAD LIMIT 512)
string(REGEX MATCH "versionNumber *= *([0-9]+)" _ "${GAME_CPP_HEAD}")
set(YUMMYLIFE_VERSION "${CMAKE_MATCH_1}")
string(REGEX MATCH "yumSubVersion *= *\"([^\"]*)\"" _ "${GAME_CPP_HEAD}")
set(YUMMYLIFE_VERSION "${YUMMYLIFE_VERSION}${CMAKE_MATCH_1}")

# The source lists live next door, in cmake/.
include(cmake/MinorGemsSources.cmake)
include(cmake/GameSources.cmake)

# These are the compile options from the original build system. Note that -O0
# in particular is sadly necessary to prevent some OHOL crashes.
set(CLIENT_COMPILE_OPTIONS -Wall -Wwrite-strings -Wchar-subscripts -Wparentheses -g -O0)

include_directories(.)

if(WIN32)
    add_definitions(-DWIN32 -DWIN_32 -DSTATICLIB -DCPPHTTPLIB_OPENSSL_SUPPORT)

    set(SDL_DIR "SDL-1.2.15")
    include_directories("${SDL_DIR}/include")
    link_directories("${SDL_DIR}/lib")

    get_filename_component(SDL_ABS_DIR "${CMAKE_SOURCE_DIR}/SDL-1.2.15" ABSOLUTE)
    if (NOT EXISTS ${SDL_ABS_DIR})
        message(FATAL_ERROR "SDL directory not found: ${SDL_DIR}")
    endif()

    # For 32-bit builds, use the 32-bit OpenSSL libraries.
    set(OPENSSL_ROOT_DIR "${CMAKE_SOURCE_DIR}/openssl-3.0.15")
    include_directories("${OPENSSL_ROOT_DIR}/include")
    link_directories("${OPENSSL_ROOT_DIR}/lib")

    add_executable(YummyLife_windows ${CLIENT_SOURCE_FILES} ${MINORGEMS_SOURCE_FILES} icon.rc)
    target_compile_options(YummyLife_windows PRIVATE ${CLIENT_COMPILE_OPTIONS})
    target_link_libraries(YummyLife_windows opengl32 glu32 mingw32 SDLmain SDL wsock32 winmm ssl crypto ws2_32 crypt32 shell32)
    target_link_options(YummyLife_windows PRIVATE -mwindows -static-libstdc++ -static-libgcc)

    # The editor, cross-compiled. Unlike the native editors there is no system
    # libpng to find here - Debian packages none for mingw - so zlib and libpng
    # are compiled straight into the target from the sources setup-libs.sh
    # fetches. Two small C libraries with no configure step needed: zlib ships a
    # ready zconf.h, and setup-libs.sh puts libpng's stock pnglibconf.h in place.
    set(ZLIB_DIR "zlib-1.3.1")
    set(LIBPNG_DIR "libpng-1.6.44")

    get_filename_component(ZLIB_ABS_DIR "${CMAKE_SOURCE_DIR}/${ZLIB_DIR}" ABSOLUTE)
    get_filename_component(LIBPNG_ABS_DIR "${CMAKE_SOURCE_DIR}/${LIBPNG_DIR}" ABSOLUTE)
    if (NOT EXISTS ${ZLIB_ABS_DIR} OR NOT EXISTS ${LIBPNG_ABS_DIR})
        message(FATAL_ERROR
            "zlib/libpng sources not found - run yummyLifeBuildScripts/setup-libs.sh")
    endif()

    # Every .c except the ones carrying a main(): example.c and pngtest.c
    set(ZLIB_SOURCE_FILES
        ${ZLIB_DIR}/adler32.c ${ZLIB_DIR}/compress.c ${ZLIB_DIR}/crc32.c
        ${ZLIB_DIR}/deflate.c ${ZLIB_DIR}/gzclose.c ${ZLIB_DIR}/gzlib.c
        ${ZLIB_DIR}/gzread.c ${ZLIB_DIR}/gzwrite.c ${ZLIB_DIR}/infback.c
        ${ZLIB_DIR}/inffast.c ${ZLIB_DIR}/inflate.c ${ZLIB_DIR}/inftrees.c
        ${ZLIB_DIR}/trees.c ${ZLIB_DIR}/uncompr.c ${ZLIB_DIR}/zutil.c
    )
    set(LIBPNG_SOURCE_FILES
        ${LIBPNG_DIR}/png.c ${LIBPNG_DIR}/pngerror.c ${LIBPNG_DIR}/pngget.c
        ${LIBPNG_DIR}/pngmem.c ${LIBPNG_DIR}/pngpread.c ${LIBPNG_DIR}/pngread.c
        ${LIBPNG_DIR}/pngrio.c ${LIBPNG_DIR}/pngrtran.c ${LIBPNG_DIR}/pngrutil.c
        ${LIBPNG_DIR}/pngset.c ${LIBPNG_DIR}/pngtrans.c ${LIBPNG_DIR}/pngwio.c
        ${LIBPNG_DIR}/pngwrite.c ${LIBPNG_DIR}/pngwtran.c ${LIBPNG_DIR}/pngwutil.c
    )

    # EXCLUDE_FROM_ALL like the other editors, so ask for it by name:
    #     cmake --build <dir> --target EditOneLife_windows
    add_executable(EditOneLife_windows EXCLUDE_FROM_ALL
        ${EDITOR_SOURCE_FILES} ${MINORGEMS_SOURCE_FILES}
        ${ZLIB_SOURCE_FILES} ${LIBPNG_SOURCE_FILES} icon.rc)
    target_compile_options(EditOneLife_windows PRIVATE ${CLIENT_COMPILE_OPTIONS})
    target_include_directories(EditOneLife_windows PRIVATE ${ZLIB_DIR} ${LIBPNG_DIR})

    # No ssl/crypto, like the other editors: nothing the editor links reaches
    # CPP-HTTPLib
    target_link_libraries(EditOneLife_windows opengl32 glu32 mingw32 SDLmain SDL wsock32 winmm ws2_32 shell32)
    target_link_options(EditOneLife_windows PRIVATE -mwindows -static-libstdc++ -static-libgcc)
elseif(APPLE)
    # enable_language(OBJC) needs CMake 3.16; only the mac build requires it
    if(CMAKE_VERSION VERSION_LESS 3.16)
        message(FATAL_ERROR "The macOS build requires CMake >= 3.16 (for Objective-C support)")
    endif()

    # __mac__ activates the existing Mac code paths in minorGems (working
    # directory handling, <OpenGL/gl.h> headers, <SDL.h> includes).
    # LINUX must stay undefined here or #ifdef LINUX / #elif __mac__
    # chains take the wrong branch.
    add_definitions(-D__mac__ -DBSD -DGL_SILENCE_DEPRECATION -DCPPHTTPLIB_OPENSSL_SUPPORT)

    enable_language(OBJC)

    # The .app machinery both mac targets share: the icon, the bundle
    # properties, and copying the Homebrew libraries in before signing.
    # Defines mac_icon and yummylife_mac_bundle().
    include(cmake/MacBundle.cmake)

    # MACOSX_BUNDLE makes this YummyLife.app rather than a bare binary, so it
    # can carry its own copy of SDL and an icon. The bundle is dropped into the
    # OHOL/AHAP folder next to the vanilla OneLife_<version>.app and chdirs to
    # the folder it sits in on startup (see the __mac__ block in gameSDL.cpp).
    add_executable(YummyLife_mac MACOSX_BUNDLE
        ${CLIENT_SOURCE_FILES} ${MINORGEMS_SOURCE_FILES}
        minorGems/game/platforms/SDL/mac/MenuBarHelper.m)
    target_compile_options(YummyLife_mac PRIVATE ${CLIENT_COMPILE_OPTIONS})

    yummylife_mac_bundle(YummyLife_mac "YummyLife")

    # SDL 1.2 API provided by sdl12-compat (brew install sdl12-compat).
    # FindSDL resolves it from the Homebrew prefix and adds SDLmain plus
    # -framework Cocoa itself. Under __mac__ the code includes <SDL.h>
    # directly, so SDL_INCLUDE_DIR (the .../include/SDL dir) is what the
    # compile line needs.
    find_package(SDL REQUIRED)
    target_include_directories(YummyLife_mac PRIVATE ${SDL_INCLUDE_DIR})
    target_link_libraries(YummyLife_mac ${SDL_LIBRARY})

    # openssl@3 is keg-only in Homebrew, so it is not in the default
    # search prefixes; ask brew where it lives (works for both the
    # /opt/homebrew and /usr/local prefixes). An explicit
    # -DOPENSSL_ROOT_DIR on the command line still wins.
    if(NOT DEFINED OPENSSL_ROOT_DIR)
        execute_process(COMMAND brew --prefix openssl@3
            OUTPUT_VARIABLE BREW_OPENSSL_PREFIX
            OUTPUT_STRIP_TRAILING_WHITESPACE
            RESULT_VARIABLE BREW_OPENSSL_RC
            ERROR_QUIET)
        if(BREW_OPENSSL_RC EQUAL 0)
            set(OPENSSL_ROOT_DIR "${BREW_OPENSSL_PREFIX}")
        endif()
    endif()
    find_package(OpenSSL REQUIRED)
    target_link_libraries(YummyLife_mac OpenSSL::SSL OpenSSL::Crypto)

    target_link_libraries(YummyLife_mac "-framework OpenGL")

    # The OHOL editor. EXCLUDE_FROM_ALL keeps it out of the normal build; ask
    # for it by name:
    #     cmake --build <dir> --target EditOneLife_mac
    #
    # A bundle rather than a bare binary because the editor looks for its data
    # folders (sprites, objects, animations) in the working directory, and the
    # __mac__ block in gameSDL.cpp only chdirs next to the program when it is
    # running out of a .app. Drop EditOneLife.app in the game folder like
    # YummyLife.app and it edits that folder's data.
    add_executable(EditOneLife_mac MACOSX_BUNDLE EXCLUDE_FROM_ALL
        ${EDITOR_SOURCE_FILES} ${MINORGEMS_SOURCE_FILES}
        minorGems/game/platforms/SDL/mac/MenuBarHelper.m)
    target_compile_options(EditOneLife_mac PRIVATE ${CLIENT_COMPILE_OPTIONS})

    yummylife_mac_bundle(EditOneLife_mac "EditOneLife")

    target_include_directories(EditOneLife_mac PRIVATE ${SDL_INCLUDE_DIR})
    target_link_libraries(EditOneLife_mac ${SDL_LIBRARY})
    target_link_libraries(EditOneLife_mac "-framework OpenGL")

    # The editor imports sprites from PNG files, so unlike the client it needs a
    # PNG decoder. PNG::PNG pulls zlib in with it. No OpenSSL here: nothing the
    # editor links reaches CPP-HTTPLib.
    find_package(PNG REQUIRED)
    target_link_libraries(EditOneLife_mac PNG::PNG)

else()
    add_definitions(-DLINUX -DCPPHTTPLIB_OPENSSL_SUPPORT)

    add_executable(YummyLife_linux ${CLIENT_SOURCE_FILES} ${MINORGEMS_SOURCE_FILES})
    target_compile_options(YummyLife_linux PRIVATE ${CLIENT_COMPILE_OPTIONS})

    find_package(SDL REQUIRED)
    include_directories(${SDL_INCLUDE_DIR})
    target_link_libraries(YummyLife_linux ${SDL_LIBRARY})

    find_package(OpenSSL REQUIRED)
    target_include_directories(YummyLife_linux PUBLIC ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(YummyLife_linux ${OPENSSL_LIBRARIES})

    find_package(OpenGL REQUIRED)
    include_directories(${OPENGL_INCLUDE_DIR})

    target_link_libraries(YummyLife_linux ${OPENGL_LIBRARIES})

    # The editor, built from the same sources as the mac one above. A bare
    # binary rather than a bundle: there is nothing to bundle, and the editor
    # reads its data folders straight out of the working directory, so run it
    # from the game folder.
    #
    # EXCLUDE_FROM_ALL like the mac editor, so ask for it by name:
    #     cmake --build <dir> --target EditOneLife_linux
    add_executable(EditOneLife_linux EXCLUDE_FROM_ALL
        ${EDITOR_SOURCE_FILES} ${MINORGEMS_SOURCE_FILES})
    target_compile_options(EditOneLife_linux PRIVATE ${CLIENT_COMPILE_OPTIONS})

    target_include_directories(EditOneLife_linux PRIVATE ${SDL_INCLUDE_DIR})
    target_link_libraries(EditOneLife_linux ${SDL_LIBRARY} ${OPENGL_LIBRARIES})

    # The editor imports sprites from PNG files, so unlike the client it needs a
    # PNG decoder. PNG::PNG pulls zlib in with it. No OpenSSL here: nothing the
    # editor links reaches CPP-HTTPLib.
    find_package(PNG REQUIRED)
    target_link_libraries(EditOneLife_linux PNG::PNG)
endif()

# The headless game server. Built for all three platforms from the same tree.
add_executable(OneLifeServer EXCLUDE_FROM_ALL
    ${SERVER_SOURCE_FILES} ${MINORGEMS_SERVER_SOURCE_FILES})
target_compile_options(OneLifeServer PRIVATE ${CLIENT_COMPILE_OPTIONS})

if(WIN32)
    # Sockets and timeGetTime. No -mwindows, unlike the client: the server is a
    # console program and needs its stdout. Threads come from the win32 files in
    # MINORGEMS_PLATFORM_SOURCE_FILES, so there is no Threads::Threads here.
    target_link_libraries(OneLifeServer wsock32 ws2_32 winmm)
    target_link_options(OneLifeServer PRIVATE -static-libstdc++ -static-libgcc)
else()
    find_package(Threads REQUIRED)
    target_link_libraries(OneLifeServer Threads::Threads)
endif()
