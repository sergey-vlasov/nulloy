/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2024 Sergey Vlasov <sergey@vlasov.me>
**
**  This program can be distributed under the terms of the GNU
**  General Public License version 3.0 as published by the Free
**  Software Foundation and appearing in the file LICENSE.GPL3
**  included in the packaging of this file.  Please review the
**  following information to ensure the GNU General Public License
**  version 3.0 requirements will be met:
**
**  http://www.gnu.org/licenses/gpl-3.0.html
**
*********************************************************************/

#include "preferencesDialogHandler.h"

#include "global.h"
#include "i18nLoader.h"
#include "pluginLoader.h"
#include "settings.h"
#include "shortcutEditorModel.h"

#ifndef _N_NO_SKINS_
#include "skinLoader.h"
#endif

#include <QStyleFactory>
#include <QTextCodec>

using namespace NPluginLoader;

static const char *LANGUAGE = QT_TRANSLATE_NOOP("PreferencesDialog", "English");

NPreferencesDialogHandler::~NPreferencesDialogHandler() {}

NPreferencesDialogHandler::NPreferencesDialogHandler(QWidget *parent)
    : NDialogHandler(QUrl::fromLocalFile(":src/preferencesDialog.qml"), parent)
{
    setBeforeShowCallback([this]() {
        QQmlContext *context = rootContext();

        // translations >>
        {
            QList<QVariantMap> data;
            foreach (QLocale::Language language, NI18NLoader::translations()) {
                QString languageString = QLocale::languageToString(language);
                QString localizedString = NI18NLoader::translate(language, "preferencesDialog",
                                                                 LANGUAGE);
                QVariantMap item;
                item["text"] = QString("%1 (%2)").arg(localizedString).arg(languageString);
                item["value"] = QLocale(language).bcp47Name().split('-').first();
                data << item;
            }
            context->setContextProperty("languagesModel", QVariant::fromValue(data));
        }
        // << translations

        // skins >>
#ifndef _N_NO_SKINS_
        {
            QList<QVariantMap> data;
            foreach (QString str, NSkinLoader::skinIdentifiers()) {
                QString id = str.section('/', 2);
                QVariantMap item;
                item["text"] = QString(id).replace('/', ' ').replace(" (Built-in)",
                                                                     tr(" (Built-in)"));
                item["value"] = id;
                data << item;
            }
            context->setContextProperty("skinsModel", QVariant::fromValue(data));
        }
#endif
        // << skins

        // styles >>
        {
            QStringList data;
            foreach (QString str, QStyleFactory::keys()) {
                data << str;
            }
            context->setContextProperty("stylesModel", QVariant::fromValue(data));
        }
        // << styles

        // encodings >>
        {
            QStringList data;
            foreach (int mib, QTextCodec::availableMibs()) {
                data << QTextCodec::codecForMib(mib)->name();
            }
            context->setContextProperty("encodingsModel", QVariant::fromValue(data));
        }
        // << encodings

        // plugins >>
        {
            QList<QVariantMap> data;
            QList<Descriptor> descriptors = NPluginLoader::descriptors();
            NFlagIterator<N::PluginType> iter(N::MaxPlugin);
            while (iter.hasNext()) {
                iter.next();
                N::PluginType type = iter.value();
                QString typeString = ENUM_TO_STR(N, PluginType, type);

                QList<int> indexesFilteredByType;
                for (int i = 0; i < descriptors.count(); ++i) {
                    if (descriptors.at(i)[TypeRole] == type) {
                        indexesFilteredByType << i;
                    }
                }

                QStringList pluginsList;
                foreach (int i, indexesFilteredByType) {
                    QString containerName = descriptors.at(i)[ContainerNameRole].toString();
                    pluginsList << containerName;
                }
                QVariantMap item;
                item["pluginType"] = typeString;
                item["plugins"] = pluginsList;
                data << item;
            }
            context->setContextProperty("pluginsModel", QVariant::fromValue(data));
        }
        // << plugins

        // >> shortcuts
        NShortcutEditorModel *shortcutEditorModel =
            new NShortcutEditorModel(NSettings::instance()->shortcuts(), this);
        context->setContextProperty("shortcutEditorModel", shortcutEditorModel);
        // << shortcuts

        context->setContextProperty("preferencesDialogHandler", this);

        NSettingsOverlay *settingsOveraly = new NSettingsOverlay(this);
        context->setContextProperty("settings", settingsOveraly);
        connect(settingsOveraly, &NSettingsOverlay::committed, [this]() { emit settingsApplied(); });
    });

#ifndef _N_NO_UPDATE_CHECK_
    setAfterShowCallback([this]() {
        QObject *root = rootObject();
        root->setProperty("versionString", m_versionCached);
        root->setProperty("noUpdateCheck", false);
    });
#endif
}

#ifndef _N_NO_UPDATE_CHECK_
void NPreferencesDialogHandler::setVersionLabel(const QString &version)
{
    m_versionCached = version;

    QObject *root = rootObject();
    if (!root) {
        return;
    }
    root->setProperty("versionString", m_versionCached);
}
#endif
