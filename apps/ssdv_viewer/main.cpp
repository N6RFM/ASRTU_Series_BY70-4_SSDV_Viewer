#include "ssdv_viewer_window.h"

#include <QApplication>
#include <QTranslator>

#include "translation.h"

int main(int argc, char* argv[])
{
    QApplication application(argc, argv);
    QCoreApplication::setOrganizationName(QStringLiteral("ASRTU"));
    QCoreApplication::setApplicationName(QStringLiteral("ASRTU_SSDV_Viewer"));

    QTranslator translator;
    installSystemTranslation(application, translator);

    SsdvViewerWindow window;
    window.show();
    return application.exec();
}