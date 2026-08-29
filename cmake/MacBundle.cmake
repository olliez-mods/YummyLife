# The .app machinery every macOS target shares.
#
# Included from the APPLE branch of the top-level CMakeLists.txt, which is the
# only place any of this means anything - it calls sips, iconutil and codesign,
# none of which exist elsewhere.
#
# Defines the mac_icon target, and yummylife_mac_bundle() for turning a declared
# MACOSX_BUNDLE target into a finished, runnable .app.

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


# Finish a declared MACOSX_BUNDLE target into a runnable .app:
#
#     yummylife_mac_bundle(<target> <name>)
#
# <name> is what the bundle is called on disk and in Info.plist, so the target
# can keep its _mac suffix - matching the Linux and Windows target names -
# while what players see stays plain YummyLife.app.
#
# Call this after add_executable(), and before adding any other POST_BUILD step
# that writes into the bundle: the last thing this attaches is the signing, and
# anything that writes into a bundle after it has been signed invalidates it.
function(yummylife_mac_bundle target name)
    set_target_properties(${target} PROPERTIES
        OUTPUT_NAME "${name}"
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_SOURCE_DIR}/Info.plist.in"
        MACOSX_BUNDLE_BUNDLE_NAME "${name}"
        MACOSX_BUNDLE_EXECUTABLE_NAME "${name}"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.olliez-mods.${name}"
        MACOSX_BUNDLE_SHORT_VERSION_STRING "${YUMMYLIFE_VERSION}"
        MACOSX_BUNDLE_BUNDLE_VERSION "${YUMMYLIFE_VERSION}"
    )

    add_dependencies(${target} mac_icon)

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_BUNDLE_DIR:${target}>/Contents/Resources"
        COMMAND ${CMAKE_COMMAND} -E copy "${MAC_ICON_ICNS}" "$<TARGET_BUNDLE_DIR:${target}>/Contents/Resources/"
        COMMENT "Copying mac_icon.icns into the bundle"
        VERBATIM
    )

    # Copy the Homebrew libraries the app needs into the bundle and re-sign it,
    # so it runs on a machine with no Homebrew installed. Runs last, everything
    # else that writes into the bundle has to happen before the signing.
    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND "${CMAKE_SOURCE_DIR}/mac-bundle.sh" "$<TARGET_BUNDLE_DIR:${target}>" "${name}"
        COMMENT "Bundling libraries into ${name}.app"
        VERBATIM
    )
endfunction()
