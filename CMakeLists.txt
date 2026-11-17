cmake_minimum_required(VERSION 3.22)

set(CMAKE_OSX_DEPLOYMENT_TARGET "10.13" CACHE STRING "Minimum macOS version")
set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "macOS universal binary")

project(KOSMOS66 VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
if (MSVC)
    add_compile_options(/utf-8)   # panel labels are UTF-8 Cyrillic
endif()

# ---------------------------------------------------------------------------
# JUCE: use a local checkout if JUCE_DIR is given, otherwise download it.
#   cmake -B build -DJUCE_DIR=/path/to/JUCE
# ---------------------------------------------------------------------------
set(JUCE_DIR "" CACHE PATH "Path to a local JUCE checkout (optional)")
if (JUCE_DIR)
    add_subdirectory(${JUCE_DIR} ${CMAKE_BINARY_DIR}/JUCE)
else()
    include(FetchContent)
    FetchContent_Declare(JUCE
        GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
        GIT_TAG        9.0.5
        GIT_SHALLOW    ON)
    FetchContent_MakeAvailable(JUCE)
endif()

option(KOSMOS_BUILD_TESTS "Build the offline render / snapshot test tool" OFF)
option(KOSMOS_MINIMAL_X11 "Linux only: build without Xrandr/Xinerama/Xcursor/XInput headers" OFF)

set(KOSMOS_FORMATS VST3 Standalone)
if (APPLE)
    list(APPEND KOSMOS_FORMATS AU)
endif()

juce_add_plugin(Kosmos66
    COMPANY_NAME            "Zavod Elektropribor"
    COMPANY_WEBSITE         "https://example.com"
    BUNDLE_ID               com.elektropribor.kosmos66
    PLUGIN_MANUFACTURER_CODE Elpr
    PLUGIN_CODE             Ks66
    PRODUCT_NAME            "KOSMOS-66"
    IS_SYNTH                TRUE
    NEEDS_MIDI_INPUT        TRUE
    NEEDS_MIDI_OUTPUT       FALSE
    IS_MIDI_EFFECT          FALSE
    EDITOR_WANTS_KEYBOARD_FOCUS TRUE
    COPY_PLUGIN_AFTER_BUILD FALSE
    VST3_CATEGORIES         Instrument Synth
    AU_MAIN_TYPE            kAudioUnitType_MusicDevice
    FORMATS                 ${KOSMOS_FORMATS})

juce_add_binary_data(Kosmos66Data SOURCES
    Resources/Fonts/DejaVuSansCondensed-Bold.ttf
    Resources/Fonts/DejaVuSansCondensed.ttf)
set_target_properties(Kosmos66Data PROPERTIES POSITION_INDEPENDENT_CODE ON)

set(KOSMOS_SOURCES
    Source/PluginProcessor.cpp
    Source/PluginEditor.cpp
    Source/Params.cpp
    Source/Presets.cpp
    Source/DSP/Voice.cpp
    Source/DSP/SynthEngine.cpp
    Source/DSP/Effects.cpp
    Source/UI/SovietLookAndFeel.cpp
    Source/UI/Controls.cpp)

target_sources(Kosmos66 PRIVATE ${KOSMOS_SOURCES})

set(KOSMOS_DEFS
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_DISPLAY_SPLASH_SCREEN=0)
if (KOSMOS_MINIMAL_X11)
    list(APPEND KOSMOS_DEFS JUCE_USE_XRANDR=0 JUCE_USE_XINERAMA=0 JUCE_USE_XCURSOR=0
                            JUCE_USE_XINPUT=0 JUCE_USE_XRENDER=0 JUCE_ALSA=0 JUCE_JACK=0)
endif()

target_compile_definitions(Kosmos66 PUBLIC ${KOSMOS_DEFS})

target_link_libraries(Kosmos66
    PRIVATE
        Kosmos66Data
        juce::juce_audio_utils
        juce::juce_dsp
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags)

if (KOSMOS_BUILD_TESTS)
    juce_add_console_app(KosmosTest PRODUCT_NAME "KosmosTest")
    target_sources(KosmosTest PRIVATE Tests/RenderTest.cpp ${KOSMOS_SOURCES})
    target_compile_definitions(KosmosTest PRIVATE ${KOSMOS_DEFS} KOSMOS_TEST_BUILD=1 JUCE_MODAL_LOOPS_PERMITTED=1
        JucePlugin_Name="KOSMOS-66" JucePlugin_IsSynth=1 JucePlugin_WantsMidiInput=1
        JucePlugin_ProducesMidiOutput=0 JucePlugin_IsMidiEffect=0)
    target_link_libraries(KosmosTest PRIVATE Kosmos66Data juce::juce_audio_utils juce::juce_dsp
        juce::juce_recommended_config_flags)
endif()
