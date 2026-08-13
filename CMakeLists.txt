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

set(MINORGEMS_COMMON_SOURCE_FILES
    minorGems/graphics/openGL/ScreenGL_SDL.cpp
    minorGems/graphics/openGL/SingleTextureGL.cpp
    minorGems/util/stringUtils.cpp
    minorGems/util/StringBufferOutputStream.cpp
    minorGems/util/ByteBufferInputStream.cpp
    minorGems/util/TranslationManager.cpp
    minorGems/network/NetworkFunctionLocks.cpp
    minorGems/network/LookupThread.cpp
    minorGems/network/web/WebRequest.cpp
    minorGems/network/web/WebRequestCompletionThread.cpp
    minorGems/network/web/URLUtils.cpp
    minorGems/util/SettingsManager.cpp
    minorGems/system/FinishedSignalThread.cpp
    minorGems/system/StopSignalThread.cpp
    minorGems/crypto/hashes/sha1.cpp
    minorGems/formats/encodingUtils.cpp
    minorGems/util/log/Log.cpp
    minorGems/util/log/AppLog.cpp
    minorGems/util/log/FileLog.cpp
    minorGems/util/log/PrintLog.cpp
    minorGems/util/printUtils.cpp
    minorGems/game/platforms/SDL/gameSDL.cpp
    minorGems/game/platforms/openGL/gameGraphicsGL.cpp
    minorGems/game/platforms/openGL/SpriteGL.cpp
    minorGems/game/doublePair.cpp
    minorGems/game/Font.cpp
    minorGems/game/drawUtils.cpp
    minorGems/game/platforms/SDL/DemoCodeChecker.cpp
    minorGems/sound/formats/aiff.cpp
    minorGems/sound/audioNoClip.cpp
    minorGems/sound/filters/SoundSamples.cpp
    minorGems/sound/filters/ReverbSoundFilter.cpp
    minorGems/sound/filters/coefficientFilters.cpp
    minorGems/crypto/keyExchange/curve25519.cpp
    minorGems/crypto/cryptoRandom.cpp
    minorGems/game/diffBundle/client/diffBundleClient.cpp
    minorGems/util/StringTree.cpp
    minorGems/util/crc32.cpp
)

if (WIN32)
    set(MINORGEMS_PLATFORM_SOURCE_FILES
        minorGems/io/win32/TypeIOWin32.cpp
        minorGems/io/file/win32/PathWin32.cpp
        minorGems/system/win32/TimeWin32.cpp
        minorGems/system/win32/ThreadWin32.cpp
        minorGems/system/win32/MutexLockWin32.cpp
        minorGems/system/win32/BinarySemaphoreWin32.cpp
        minorGems/network/win32/SocketWin32.cpp
        minorGems/network/win32/HostAddressWin32.cpp
        minorGems/network/win32/SocketClientWin32.cpp
        minorGems/network/win32/SocketServerWin32.cpp
        minorGems/io/file/win32/DirectoryWin32.cpp
    )
else()
    set(MINORGEMS_PLATFORM_SOURCE_FILES
        minorGems/io/linux/TypeIOLinux.cpp
        minorGems/io/file/linux/PathLinux.cpp
        minorGems/system/linux/ThreadLinux.cpp
        minorGems/system/linux/MutexLockLinux.cpp
        minorGems/system/linux/BinarySemaphoreLinux.cpp
        minorGems/network/linux/SocketLinux.cpp
        minorGems/network/linux/HostAddressLinux.cpp
        minorGems/network/linux/SocketClientLinux.cpp
        minorGems/network/linux/SocketServerLinux.cpp
        minorGems/system/unix/TimeUnix.cpp
        minorGems/io/file/unix/DirectoryUnix.cpp
    )
endif()

set(MINORGEMS_SOURCE_FILES
    ${MINORGEMS_COMMON_SOURCE_FILES}
    ${MINORGEMS_PLATFORM_SOURCE_FILES}
)

set(CLIENT_SOURCE_FILES
    gameSource/minitech.cpp
    gameSource/hetuwmod.cpp
    gameSource/hetuwFont.cpp
    gameSource/hetuwTCPConnection.cpp
    gameSource/phex.cpp
    gameSource/yummyLife.cpp
    gameSource/yummyGPS.cpp
    gameSource/yumBlob.cpp
    gameSource/yumConfig.cpp
    gameSource/yumRebirthComponent.cpp
    gameSource/game.cpp
    gameSource/spriteBank.cpp
    gameSource/objectBank.cpp
    gameSource/transitionBank.cpp
    gameSource/animationBank.cpp
    gameSource/whiteSprites.cpp
    gameSource/message.cpp
    gameSource/serialWebRequests.cpp
    gameSource/accountHmac.cpp
    gameSource/PageComponent.cpp
    gameSource/GamePage.cpp
    gameSource/LivingLifePage.cpp
    gameSource/pathFind.cpp
    gameSource/ageControl.cpp
    gameSource/ExtendedMessagePage.cpp
    gameSource/buttonStyle.cpp
    gameSource/Button.cpp
    gameSource/TextButton.cpp
    gameSource/RebirthChoicePage.cpp
    gameSource/TextField.cpp
    gameSource/LoadingPage.cpp
    gameSource/folderCache.cpp
    gameSource/binFolderCache.cpp
    gameSource/liveObjectSet.cpp
    commonSource/fractalNoise.cpp
    commonSource/sayLimit.cpp
    gameSource/ExistingAccountPage.cpp
    gameSource/KeyEquivalentTextButton.cpp
    gameSource/ServerActionPage.cpp
    gameSource/FinalMessagePage.cpp
    gameSource/AutoUpdatePage.cpp
    gameSource/soundBank.cpp
    gameSource/convolution.cpp
    gameSource/fft.cpp
    gameSource/ogg.cpp
    gameSource/musicPlayer2.cpp
    gameSource/groundSprites.cpp
    gameSource/SettingsPage.cpp
    gameSource/CheckboxButton.cpp
    gameSource/ValueSlider.cpp
    gameSource/SpriteButton.cpp
    gameSource/SpriteToggleButton.cpp
    gameSource/categoryBank.cpp
    gameSource/liveAnimationTriggers.cpp
    gameSource/ReviewPage.cpp
    gameSource/TextArea.cpp
    gameSource/RadioButtonSet.cpp
    gameSource/spellCheck.cpp
    gameSource/SoundUsage.cpp
    gameSource/TwinPage.cpp
    gameSource/objectMetadata.cpp
    gameSource/emotion.cpp
    gameSource/Picker.cpp
    gameSource/PickableStatics.cpp
    gameSource/photos.cpp
    gameSource/lifeTokens.cpp
    gameSource/PollPage.cpp
    gameSource/fitnessScore.cpp
    gameSource/GeneticHistoryPage.cpp
    gameSource/ServicesPage.cpp
    gameSource/photoCache.cpp
    gameSource/exporter.cpp
    gameSource/importer.cpp
    gameSource/settingsToggle.cpp
    gameSource/AHAPResultPage.cpp
    gameSource/AHAPSettingsPage.cpp
    gameSource/rocketAnimation.cpp
    gameSource/GridPos.cpp
)

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

    # MACOSX_BUNDLE makes this YummyLife.app rather than a bare binary, so it
    # can carry its own copy of SDL and an icon. The bundle is dropped into the
    # OHOL/AHAP folder next to the vanilla OneLife_<version>.app and chdirs to
    # the folder it sits in on startup (see the __mac__ block in gameSDL.cpp).
    add_executable(YummyLife_mac MACOSX_BUNDLE
        ${CLIENT_SOURCE_FILES} ${MINORGEMS_SOURCE_FILES}
        minorGems/game/platforms/SDL/mac/MenuBarHelper.m)
    target_compile_options(YummyLife_mac PRIVATE ${CLIENT_COMPILE_OPTIONS})

    # The target keeps the _mac suffix to stay consistent with the Linux and
    # Windows targets, but the bundle players see is plain YummyLife.app.
    set_target_properties(YummyLife_mac PROPERTIES
        OUTPUT_NAME "YummyLife"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/Info.plist.in"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${YUMMYLIFE_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${YUMMYLIFE_VERSION}"
    )

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

    # Build the .icns the bundle's CFBundleIconFile points at. macOS wants
    # every size in one file, sips resizes and iconutil packs them.
    set(MAC_ICON_PNG "${CMAKE_SOURCE_DIR}/mac_icon.png")
    set(MAC_ICON_ICNS "${CMAKE_BINARY_DIR}/mac_icon.icns")
    set(MAC_ICONSET_DIR "${CMAKE_BINARY_DIR}/mac_icon.iconset")
    add_custom_command(OUTPUT ${MAC_ICON_ICNS}
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${MAC_ICONSET_DIR}"
        COMMAND ${CMAKE_COMMAND} -E make_directory "${MAC_ICONSET_DIR}"
        COMMAND sips -z 16 16     "${MAC_ICON_PNG}" --out "${MAC_ICONSET_DIR}/icon_16x16.png"
        COMMAND sips -z 32 32     "${MAC_ICON_PNG}" --out "${MAC_ICONSET_DIR}/icon_16x16@2x.png"
        COMMAND sips -z 32 32     "${MAC_ICON_PNG}" --out "${MAC_ICONSET_DIR}/icon_32x32.png"
        COMMAND sips -z 64 64     "${MAC_ICON_PNG}" --out "${MAC_ICONSET_DIR}/icon_32x32@2x.png"
        COMMAND sips -z 128 128   "${MAC_ICON_PNG}" --out "${MAC_ICONSET_DIR}/icon_128x128.png"
        COMMAND sips -z 256 256   "${MAC_ICON_PNG}" --out "${MAC_ICONSET_DIR}/icon_128x128@2x.png"
        COMMAND sips -z 256 256   "${MAC_ICON_PNG}" --out "${MAC_ICONSET_DIR}/icon_256x256.png"
        COMMAND sips -z 512 512   "${MAC_ICON_PNG}" --out "${MAC_ICONSET_DIR}/icon_256x256@2x.png"
        COMMAND sips -z 512 512   "${MAC_ICON_PNG}" --out "${MAC_ICONSET_DIR}/icon_512x512.png"
        COMMAND sips -z 1024 1024 "${MAC_ICON_PNG}" --out "${MAC_ICONSET_DIR}/icon_512x512@2x.png"
        COMMAND iconutil -c icns "${MAC_ICONSET_DIR}" -o "${MAC_ICON_ICNS}"
        COMMAND ${CMAKE_COMMAND} -E rm -rf "${MAC_ICONSET_DIR}"
        DEPENDS "${MAC_ICON_PNG}"
        COMMENT "Generating mac_icon.icns"
        VERBATIM
    )
    add_custom_target(mac_icon DEPENDS ${MAC_ICON_ICNS})
    add_dependencies(YummyLife_mac mac_icon)

    add_custom_command(TARGET YummyLife_mac POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:YummyLife_mac>/Contents/Resources"
        COMMAND ${CMAKE_COMMAND} -E copy "${MAC_ICON_ICNS}" "$<TARGET_BUNDLE_DIR:YummyLife_mac>/Contents/Resources/"
        COMMENT "Copying mac_icon.icns into the bundle"
        VERBATIM
    )

    # Copy the Homebrew libraries the app needs into the bundle and re-sign it,
    # so it runs on a machine with no Homebrew installed. Runs last, everything
    # else that writes into the bundle has to happen before the signing.
    add_custom_command(TARGET YummyLife_mac POST_BUILD
        COMMAND "${CMAKE_SOURCE_DIR}/mac-bundle.sh" "$<TARGET_BUNDLE_DIR:YummyLife_mac>" "YummyLife"
        COMMENT "Bundling libraries into YummyLife.app"
        VERBATIM
    )
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
endif()
