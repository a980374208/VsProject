#include <QGuiApplication>
#include <QQmlApplicationEngine>
#include "DirectXRendererItem.h"

int main(int argc, char* argv[])
{
    QGuiApplication app(argc, argv);
#if defined(DEBUG) | defined(_DEBUG)
    _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);
#endif
    qmlRegisterType<DirectXRendererItem>("CustomComponents", 1, 0, "DirectXRendererItem");

    QQmlApplicationEngine engine;
    engine.load(QUrl(QStringLiteral("qrc:/qt/qml/qml_dx11_test/main.qml")));

    if (engine.rootObjects().isEmpty())
        return -1;

    return app.exec();
}
