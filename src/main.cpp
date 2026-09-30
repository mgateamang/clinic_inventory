#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include <QQmlContext>
#include "inventory_controller.hpp"

int main(int argc, char* argv[]) {
    QGuiApplication app(argc, argv);
    app.setApplicationName("AIDANS-Inventory");
    app.setOrganizationName("ElizabethClinic");

    QString configPath = "config.ini";
    for (int i = 1; i < argc; ++i) {
        QString arg = argv[i];
        if (arg == "--config" && i + 1 < argc) {
            configPath = argv[++i];
        }
    }

    aidans::InventoryController controller(configPath);

    QQmlApplicationEngine engine;
    engine.rootContext()->setContextProperty("controller", &controller);

    const QUrl url(QStringLiteral("qrc:/Main.qml"));
    QObject::connect(
        &engine,
        &QQmlApplicationEngine::objectCreated,
        &app,
        [url](QObject* obj, const QUrl& objUrl) {
            if (!obj && url == objUrl)
                QCoreApplication::exit(-1);
        },
        Qt::QueuedConnection
    );

    engine.load(url);

    return app.exec();
}
