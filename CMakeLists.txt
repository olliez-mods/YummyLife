cmake_minimum_required(VERSION 3.13)

project(YummyLife)

set(CMAKE_CXX_STANDARD 14)

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
    target_link_libraries(YummyLife_windows opengl32 glu32 mingw32 SDLmain SDL wsock32 winmm ssl crypto ws2_32 crypt32)
    target_link_options(YummyLife_windows PRIVATE -mwindows -static-libstdc++ -static-libgcc)
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
