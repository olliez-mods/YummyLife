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

# The part of minorGems that pulls in no SDL, OpenGL or sound. The server links
# only these; the client and editor add MINORGEMS_GRAPHICS_SOURCE_FILES on top.
set(MINORGEMS_HEADLESS_SOURCE_FILES
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
    minorGems/game/doublePair.cpp
    minorGems/util/StringTree.cpp
    minorGems/util/crc32.cpp
)

# Everything the headless build leaves out: the SDL/GL screen and sprite layer,
# sound, and the client-only crypto and auto-update code.
set(MINORGEMS_GRAPHICS_SOURCE_FILES
    minorGems/graphics/openGL/ScreenGL_SDL.cpp
    minorGems/graphics/openGL/SingleTextureGL.cpp
    minorGems/game/platforms/SDL/gameSDL.cpp
    minorGems/game/platforms/openGL/gameGraphicsGL.cpp
    minorGems/game/platforms/openGL/SpriteGL.cpp
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
)

set(MINORGEMS_COMMON_SOURCE_FILES
    ${MINORGEMS_HEADLESS_SOURCE_FILES}
    ${MINORGEMS_GRAPHICS_SOURCE_FILES}
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

# SocketPoll is linked by the server alone, which is why it shows up in neither
# list above. The Linux implementation is built on epoll, so everything else
# takes the portable one - the same choice minorGems' own Makefile.MacOSX makes
# (POLL_PLATFORM = Unix).
#
# Despite living in network/unix, that file is select()-based rather than
# poll()-based and carries its own WIN32 branch, so it serves the Windows
# server too. Winsock's FD_SETSIZE of 64 is smaller than the Unix 1024, which
# costs the Windows build more select calls per wait, but the file already
# batches by FD_SETSIZE for exactly that reason.
if(NOT WIN32 AND NOT APPLE)
    set(MINORGEMS_SOCKET_POLL_SOURCE_FILES minorGems/network/linux/SocketPollLinux.cpp)
else()
    set(MINORGEMS_SOCKET_POLL_SOURCE_FILES minorGems/network/unix/SocketPollUnix.cpp)
endif()

set(MINORGEMS_SERVER_SOURCE_FILES
    ${MINORGEMS_HEADLESS_SOURCE_FILES}
    ${MINORGEMS_PLATFORM_SOURCE_FILES}
    ${MINORGEMS_SOCKET_POLL_SOURCE_FILES}
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

# The editor (EditOneLife) is a second program built from the same tree. This
# list is the LAYER_SOURCE block of gameSource/makeFileListEditor, the upstream
# recipe, so keep the two in step if that file changes.
#
# hetuwStubs.cpp is ours: the shared banks reach into a few HetuwMod statics and
# the editor has no HetuwMod. The server links it for the same reason.
# PNGImageConverter is editor-only though, the client never reads a PNG
# (TGAImageConverter.h is header only, which is why no converter shows up in the
# client build at all).
set(EDITOR_SOURCE_FILES
    gameSource/editor.cpp
    gameSource/hetuwStubs.cpp
    gameSource/PageComponent.cpp
    gameSource/GamePage.cpp
    gameSource/TextField.cpp
    gameSource/Button.cpp
    gameSource/TextButton.cpp
    gameSource/whiteSprites.cpp
    gameSource/serialWebRequests.cpp
    gameSource/message.cpp
    gameSource/accountHmac.cpp
    gameSource/EditorImportPage.cpp
    gameSource/spriteBank.cpp
    gameSource/Picker.cpp
    gameSource/objectBank.cpp
    gameSource/EditorObjectPage.cpp
    gameSource/transitionBank.cpp
    gameSource/EditorTransitionPage.cpp
    gameSource/SpriteButton.cpp
    gameSource/SpriteToggleButton.cpp
    gameSource/CheckboxButton.cpp
    gameSource/animationBank.cpp
    gameSource/EditorAnimationPage.cpp
    gameSource/EditorSpriteTrimPage.cpp
    gameSource/ValueSlider.cpp
    gameSource/ageControl.cpp
    gameSource/overlayBank.cpp
    gameSource/keyLegend.cpp
    gameSource/LoadingPage.cpp
    gameSource/folderCache.cpp
    gameSource/binFolderCache.cpp
    gameSource/PickableStatics.cpp
    gameSource/soundBank.cpp
    gameSource/SoundWidget.cpp
    gameSource/convolution.cpp
    gameSource/fft.cpp
    gameSource/ogg.cpp
    gameSource/zoomView.cpp
    gameSource/categoryBank.cpp
    gameSource/EditorCategoryPage.cpp
    gameSource/RadioButtonSet.cpp
    gameSource/EditorScenePage.cpp
    gameSource/groundSprites.cpp
    commonSource/fractalNoise.cpp
    gameSource/spellCheck.cpp
    gameSource/SoundUsage.cpp
    gameSource/objectMetadata.cpp
    gameSource/emotion.cpp
    gameSource/exporter.cpp
    gameSource/EditorExportPage.cpp
    gameSource/importer.cpp
    gameSource/settingsToggle.cpp
    minorGems/graphics/converters/PNGImageConverter.cpp
)

# The headless game server (OneLifeServer).
# hetuwStubs.cpp is ours, and is here for the same reason the editor has it.
set(SERVER_SOURCE_FILES
    server/server.cpp
    server/map.cpp
    gameSource/transitionBank.cpp
    gameSource/categoryBank.cpp
    gameSource/objectBank.cpp
    gameSource/animationBank.cpp
    gameSource/ageControl.cpp
    gameSource/folderCache.cpp
    gameSource/SoundUsage.cpp
    gameSource/objectMetadata.cpp
    gameSource/GridPos.cpp
    commonSource/fractalNoise.cpp
    commonSource/sayLimit.cpp
    gameSource/settingsToggle.cpp
    gameSource/hetuwStubs.cpp
    server/kissdb.cpp
    server/lineardb3.cpp
    server/lifeLog.cpp
    server/foodLog.cpp
    server/backup.cpp
    server/triggers.cpp
    server/dbCommon.cpp
    server/playerStats.cpp
    server/lineageLog.cpp
    server/failureLog.cpp
    server/names.cpp
    server/monument.cpp
    server/lineageLimit.cpp
    server/curses.cpp
    server/curseLog.cpp
    server/spiral.cpp
    server/objectSurvey.cpp
    server/language.cpp
    server/familySkipList.cpp
    server/lifeTokens.cpp
    server/fitnessScore.cpp
    server/CoordinateTimeTracking.cpp
    server/arcReport.cpp
    server/curseDB.cpp
    server/trustDB.cpp
    server/eveMovingGrid.cpp
    server/specialBiomes.cpp
    server/cravings.cpp
    server/offspringTracker.cpp
    server/ipBanList.cpp
    server/ahapGate.cpp
    server/periodicPlacements.cpp
    server/timeLogger.cpp
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
        MACOSX_BUNDLE_BUNDLE_NAME "YummyLife"
        MACOSX_BUNDLE_EXECUTABLE_NAME "YummyLife"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.olliez-mods.YummyLife"
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

    set_target_properties(EditOneLife_mac PROPERTIES
        OUTPUT_NAME "EditOneLife"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME "EditOneLife"
        MACOSX_BUNDLE_EXECUTABLE_NAME "EditOneLife"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.olliez-mods.EditOneLife"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${YUMMYLIFE_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${YUMMYLIFE_VERSION}"
    )

    target_include_directories(EditOneLife_mac PRIVATE ${SDL_INCLUDE_DIR})
    target_link_libraries(EditOneLife_mac ${SDL_LIBRARY})
    target_link_libraries(EditOneLife_mac "-framework OpenGL")

    # The editor imports sprites from PNG files, so unlike the client it needs a
    # PNG decoder. PNG::PNG pulls zlib in with it. No OpenSSL here: nothing the
    # editor links reaches CPP-HTTPLib.
    find_package(PNG REQUIRED)
    target_link_libraries(EditOneLife_mac PNG::PNG)

    add_dependencies(EditOneLife_mac mac_icon)

    add_custom_command(TARGET EditOneLife_mac POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:EditOneLife_mac>/Contents/Resources"
        COMMAND ${CMAKE_COMMAND} -E copy "${MAC_ICON_ICNS}" "$<TARGET_BUNDLE_DIR:EditOneLife_mac>/Contents/Resources/"
        COMMENT "Copying mac_icon.icns into the bundle"
        VERBATIM
    )

    add_custom_command(TARGET EditOneLife_mac POST_BUILD
        COMMAND "${CMAKE_SOURCE_DIR}/mac-bundle.sh" "$<TARGET_BUNDLE_DIR:EditOneLife_mac>" "EditOneLife"
        COMMENT "Bundling libraries into EditOneLife.app"
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
