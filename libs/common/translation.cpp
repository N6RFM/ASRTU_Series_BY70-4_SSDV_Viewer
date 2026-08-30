#include "translation.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QTranslator>

bool installSystemTranslation(QApplication& application,
                              QTranslator& translator)
{
    const QStringList arguments = QCoreApplication::arguments();
    const bool forceChinese = arguments.contains(QStringLiteral("--language=zh"));
    const bool forceJapanese = arguments.contains(QStringLiteral("--language=ja"));

    // English is the default regardless of system locale. --language=zh
    // shows the UI's original, untranslated Chinese source strings (no
    // translator needed for that); --language=ja loads the Japanese
    // translation; anything else, including no flag at all, loads English.
    if (forceChinese)
        return false;

    const QString translation = QDir(QCoreApplication::applicationDirPath())
                                    .filePath(forceJapanese
                                        ? QStringLiteral("translations/asrtu_ja.qm")
                                        : QStringLiteral("translations/asrtu_en.qm"));
    if (!translator.load(translation))
        return false;
    return application.installTranslator(&translator);
}
