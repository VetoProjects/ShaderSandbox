#include <QTranslator>

#include "EditorWindow.hpp"
#include "Renderer.hpp"
#include "BootLoader.hpp"
#include "Instances/WindowInstance.hpp"
#include "ObjectLoaderDialog.hpp"

// no constexpr, because QString is not a literal type
static const QString socketName = "VeTo";

/**
 * @brief main
 * @param argc
 * @param argv
 * @return return code of the QApplication
 * @author Veit Heller(s0539501) & Tobias Brosge(s0539713)
 * @version 0.2.4
 *
 * The main class. Sets up the application and returns its' return code.
 */
int main(int argc, char *argv[]){
    QLocalSocket socket;
    socket.connectToServer(socketName);
    if(socket.waitForConnected(600)){
        socket.write("");
        return 0;
    }

    QApplication a(argc, argv);

    if(QFontDatabase::addApplicationFont(":/fonts/Inconsolata.otf") == -1)
        qWarning() << a.tr("Failed to load font Inconsolata.");

    auto language = QLocale::system().name();
    language.chop(3);
    auto dir = Backend::directoryOf(QStringLiteral(""));

    QTranslator translator;
    translator.load(dir.absoluteFilePath("translations/codeeditor_" + language));
    a.installTranslator(&translator);

    Backend server(&a);

    auto ids = server.loadIds();

    if(ids.length() == 0)
        ids.append(server.nextID());

    for(auto id : ids){
        QHash<QString,QVariant> settings = server.getSettings(id);
        Instances::WindowInstance *instance = new Instances::WindowInstance(id, settings, &server);
        server.addInstance(instance, false);
    }

    auto *bootLoader = new BootLoader(socketName, &server);
    bootLoader->start();

    QFile style(dir.absoluteFilePath("style/default.css"));
    style.open(QFile::ReadOnly | QFile::Text);

    a.setStyleSheet(QTextStream(&style).readAll());

    return a.exec();
}
