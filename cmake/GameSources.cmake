# The source lists for the three programs this tree builds. Each one mirrors an
# upstream recipe, named in the comment above it, so that these stay checkable
# against the build system they came from.
#
# Included from the top-level CMakeLists.txt. Sets, in the caller's scope:
#   CLIENT_SOURCE_FILES   the game
#   EDITOR_SOURCE_FILES   the object/sprite/animation editor
#   SERVER_SOURCE_FILES   the headless server

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
