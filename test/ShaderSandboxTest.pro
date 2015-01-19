isEqual(QT_MAJOR_VERSION, 4) || isEqual(QT_MINOR_VERSION, 1) {
    message("Cannot build VetoLC with Qt version $$QT_VERSION.")
    error("Use at least Qt 5.2.")
}

QT       += core gui multimedia testlib widgets

TARGET = VetoLCTest
TEMPLATE = app

QMAKE_CXXFLAGS += -Wall -DNDEBUG -O1 -pedantic
macx {
    QMAKE_CXXFLAGS += -stdlib=libc++ -mmacosx-version-min=10.7
}

RESOURCES += \
    ../application.qrc

CONFIG +=c++11

translations.target = $$OUT_PWD/translations
src = $$PWD/../translations
win32 {
    translations.target ~= s,/,\\,g
    src ~= s,/,\\,g
}
translations.commands = $(COPY_DIR) \"$$src\" \"$$translations.target\"
first.depends = $(first) translations
export(first.depends)
export(translations.commands)

QMAKE_EXTRA_TARGETS += first translations

HEADERS += \
    EditorWindowTest.hpp \
    ../src/EditorWindow.hpp \
    CodeEditorTest.hpp \
    ../src/CodeEditor.hpp \
    BackendTest.hpp \
    ../src/Backend.hpp \
    SettingsBackendTest.hpp \
    ../src/SettingsBackend.hpp \
    ../src/AudioInputProcessor.hpp \
    RendererTest.hpp \
    ../src/Instances/WindowInstance.hpp \
    CodeHighlighterTest.hpp \
    ../src/SettingsWindow.hpp \
    ../src/SettingsTab.hpp \
    ../src/Renderer.hpp \
    ../src/CodeHighlighter.hpp \
    ../src/LiveThread.hpp \
    ../src/BootLoader.hpp \
    ../src/Instances/IInstance.hpp \
    ../src/Model3D.hpp \
    ../src/ObjectLoaderDialog.hpp

SOURCES += \
    main.cpp \
    ../src/CodeEditor.cpp \
    ../src/CodeHighlighter.cpp \
    ../src/EditorWindow.cpp \
    ../src/SettingsWindow.cpp \
    ../src/SettingsTab.cpp \
    ../src/Renderer.cpp \
    ../src/Backend.cpp \
    ../src/SettingsBackend.cpp \
    ../src/AudioInputProcessor.cpp \
    ../src/BootLoader.cpp \
    ../src/Instances/WindowInstance.cpp \
    ../src/Model3D.cpp \
    ../src/ObjectLoaderDialog.cpp
