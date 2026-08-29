# Which parts of the vendored minorGems tree each program compiles.
#
# Split three ways because the server links no graphics: the headless list is
# what it takes, the graphics list is what the client and editor add on top,
# and the platform list is picked by host. Nothing here is conditional on the
# target being built, only on the platform being built for.
#
# Included from the top-level CMakeLists.txt. Sets, in the caller's scope:
#   MINORGEMS_HEADLESS_SOURCE_FILES   no SDL, OpenGL or sound
#   MINORGEMS_GRAPHICS_SOURCE_FILES   the screen, sprite, sound and crypto layer
#   MINORGEMS_PLATFORM_SOURCE_FILES   win32 or linux/unix implementations
#   MINORGEMS_SOURCE_FILES            what the client and editor link
#   MINORGEMS_SERVER_SOURCE_FILES     what the server links

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
