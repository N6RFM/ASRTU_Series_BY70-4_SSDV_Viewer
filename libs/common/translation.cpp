#include "translation.h"

#include <QApplication>
#include <QCoreApplication>
#include <QDir>
#include <QTranslator>

namespace {

// Accepts "--language=xx", "--language xx" and "-l xx", case-insensitively.
QString requestedLanguage()
{
    const QStringList arguments = QCoreApplication::arguments();
    for (int i = 1; i < arguments.size(); ++i) {
        const QString arg = arguments.at(i);
        if (arg.startsWith(QLatin1String("--language="), Qt::CaseInsensitive))
            return arg.mid(11).trimmed().toLower();
        if ((arg.compare(QLatin1String("--language"), Qt::CaseInsensitive) == 0 ||
             arg == QLatin1String("-l")) && i + 1 < arguments.size())
            return arguments.at(i + 1).trimmed().toLower();
    }
    return QString();
}

} // namespace

bool installSystemTranslation(QApplication& application,
                              QTranslator& translator)
{
    const QString language = requestedLanguage();
    const bool forceChinese = language == QLatin1String("zh");
    const bool forceJapanese = language == QLatin1String("ja");

    // English is the default regardless of system locale. --language=zh
    // shows the UI's original, untranslated Chinese source strings (no
    // translator needed for that); --language=ja loads the Japanese
    // translation; anything else, including no flag at all, loads English.
    // The catalogs use viewer-specific file names so they cannot be
    // overwritten by the ASRTU decoder suite, which installs its own
    // asrtu_*.qm files into the same translations directory.
    if (forceChinese)
        return false;

    const QString translation = QDir(QCoreApplication::applicationDirPath())
                                    .filePath(forceJapanese
                                        ? QStringLiteral("translations/ssdv_viewer_ja.qm")
                                        : QStringLiteral("translations/ssdv_viewer_en.qm"));
    if (!translator.load(translation))
        return false;
    return application.installTranslator(&translator);
}
