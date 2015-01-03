isEqual(QT_MAJOR_VERSION, 4) || isEqual(QT_MINOR_VERSION, 1) {
    message("Cannot build ShaderSandbox with Qt version $$QT_VERSION.")
    error("Use at least Qt 5.2.")
}

QT       += core gui multimedia widgets

TARGET = ShaderSandbox
TEMPLATE = app
win32:RC_ICONS += ../rc/Icon.ico
ICON = ../rc/Icon.icns

TRANSLATIONS = ../translations/codeeditor_de.ts \
               ../translations/codeeditor_en.ts \
               ../translations/codeeditor_fr.ts

CODECFORSRC     = UTF-8

QMAKE_CXXFLAGS += -Wall -DNDEBUG -O2 -pedantic
macx {
    QMAKE_CXXFLAGS += -stdlib=libc++ -mmacosx-version-min=10.7
}

QMAKE_EXTRA_TARGETS += valgrind-check

CONFIG+=c++14 no_keywords

valgrind-check.depends = check
valgrind-check.commands = valgrind ./$$TARGET

RESOURCES += \
    ../application.qrc

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

HEADERS += Instances/IInstance.hpp \
    Instances/WindowInstance.hpp \
    AudioInputProcessor.hpp \
    Backend.hpp \
    BootLoader.hpp \
    CodeEditor.hpp \
    CodeHighlighter.hpp \
    EditorWindow.hpp \
    LiveThread.hpp \
    Renderer.hpp \
    SettingsBackend.hpp \
    SettingsTab.hpp \
    SettingsWindow.hpp \ 
    ObjectLoaderDialog.hpp

SOURCES += Instances/WindowInstance.cpp \
    AudioInputProcessor.cpp \
    Backend.cpp \
    BootLoader.cpp \
    CodeEditor.cpp \
    CodeHighlighter.cpp \
    EditorWindow.cpp \
    main.cpp \
    Renderer.cpp \
    SettingsBackend.cpp \
    SettingsTab.cpp \
    SettingsWindow.cpp \
    ObjectLoaderDialog.cpp


