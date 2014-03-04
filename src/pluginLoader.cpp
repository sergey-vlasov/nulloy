/********************************************************************
**  Nulloy Music Player, http://nulloy.com
**  Copyright (C) 2010-2013 Sergey Vlasov <sergey@vlasov.me>
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

#include "pluginLoader.h"

#include "global.h"
#include "common.h"
#include "settings.h"

#include "pluginContainer.h"
#include "plugin.h"

#include "waveformBuilderInterface.h"
#include "playbackEngineInterface.h"
#include "tagReaderInterface.h"
#include "coverReaderInterface.h"

#include <QMessageBox>
#include <QObject>
#include <QPluginLoader>
#include <QStringList>

#ifdef _N_GSTREAMER_PLUGINS_BUILTIN_
#include "playbackEngineGstreamer.h"
#include "waveformBuilderGstreamer.h"
#endif

Q_DECLARE_METATYPE(NPlugin *)
Q_DECLARE_METATYPE(QPluginLoader *)

namespace NPluginLoader
{
	static bool _init = FALSE;
	static QList<Descriptor> _descriptors;
	static QMap<N::PluginType, NPlugin *> _usedPlugins;
	static QMap<QPluginLoader *, bool> _usedLoaders;
	QString _containerPrefer = "GStreamer";

	void _loadPlugins();
	NPlugin* _findPlugin(N::PluginType type);
}

void NPluginLoader::deinit()
{
	foreach(QPluginLoader *loader, _usedLoaders.keys()) {
		if (loader->isLoaded())
			loader->unload();
	}
}

NPlugin* NPluginLoader::_findPlugin(N::PluginType type)
{
	QString typeString = ENUM_NAME(N, PluginType, type);
	QString settingsContainer = NSettings::instance()->value("Plugins/" + typeString).toString();

	QList<int> indexesFilteredByType;
	for (int i = 0; i < _descriptors.count(); ++i) {
		if (_descriptors.at(i)[TypeRole] == type)
			indexesFilteredByType << i;
	}

	if (indexesFilteredByType.isEmpty())
		return NULL;

	int index = -1;
	foreach (QString container, QStringList () << settingsContainer << _containerPrefer) {
		foreach (int i, indexesFilteredByType) {
			if (_descriptors.at(i)[ContainerNameRole] == container) {
				index = i;
				break;
			}
		}
		if (index != -1)
			break;
	}

	if (index == -1)
		index = 0;

	NPlugin *plugin = _descriptors.at(index)[PluginObjectRole].value<NPlugin *>();
	plugin->init();

	QPluginLoader *loader = _descriptors.at(index)[LoaderObjectRole].value<QPluginLoader *>();
	_usedLoaders[loader] = TRUE;

	QString containerName = _descriptors.at(index)[ContainerNameRole].toString();
	NSettings::instance()->setValue(QString() + "Plugins/" + typeString, containerName);

	return plugin;
}

void NPluginLoader::_loadPlugins()
{
	if (_init)
		return;
	_init = TRUE;

#if 0
	QMap<QString, bool> usedFlags;
	QList<NPlugin *> plugins;
	QList<NPlugin *> pluginsStatic;
#ifdef _N_GSTREAMER_PLUGINS_BUILTIN_
	pluginsStatic << new NPlaybackEngineGStreamer() << new NWaveformBuilderGstreamer();
#endif
	pluginsStatic << QPluginLoader::staticInstances();

	foreach (NPlugin *plugin, pluginsStatic) {
		if (plugin) {
			plugins << plugin;
			plugin->init();
			QString id = plugin->identifier();
			id.insert(id.lastIndexOf('/'), " (Built-in)");
			_identifiers << id;
			_loaders << NULL;
			usedFlags << TRUE;
		}
	}
#endif

	QStringList pluginsDirList;
	pluginsDirList << QCoreApplication::applicationDirPath() + "/plugins";
#ifndef Q_WS_WIN
	if (NCore::rcDir() != QCoreApplication::applicationDirPath())
		pluginsDirList << NCore::rcDir() + "/plugins";
	if (QDir(QCoreApplication::applicationDirPath()).dirName() == "bin") {
		QDir dir(QCoreApplication::applicationDirPath());
		dir.cd("../lib/nulloy/plugins");
		pluginsDirList << dir.absolutePath();
	}
#endif

#ifdef Q_WS_WIN
		QStringList subDirsList;
		foreach (QString dirStr, pluginsDirList) {
			QDir dir(dirStr);
			if (dir.exists()) {
				foreach (QString subDir, dir.entryList(QDir::Dirs))
					subDirsList << dirStr + "/" + subDir;
			}
		}
		_putenv(QString("PATH=" + pluginsDirList.join(";") + ";" +
			subDirsList.join(";") + ";" + getenv("PATH")).replace('/', '\\').toUtf8());
#endif
	foreach (QString dirStr, pluginsDirList) {
		QDir dir(dirStr);
		if (!dir.exists())
			continue;
		foreach (QString fileName, dir.entryList(QDir::Files)) {
			QString fileFullPath = dir.absoluteFilePath(fileName);
#ifdef Q_WS_WIN
			// skip non plugin files
			if (!fileName.startsWith("plugin", Qt::CaseInsensitive) || !fileName.endsWith("dll", Qt::CaseInsensitive))
				continue;
#endif
			if (!QLibrary::isLibrary(fileFullPath))
				continue;
			QPluginLoader *loader = new QPluginLoader(fileFullPath);
			_usedLoaders[loader] = FALSE;
			QObject *instance = loader->instance();
			NPluginContainer *container = qobject_cast<NPluginContainer *>(instance);
			if (container) {
				QList<NPlugin *> _plugins = container->plugins();
				foreach (NPlugin *plugin, _plugins) {
					Descriptor d;
					d[TypeRole] = plugin->type();
					d[ContainerNameRole] = container->name();
					d[PluginObjectRole] = QVariant::fromValue<NPlugin *>(plugin);
					d[LoaderObjectRole] = QVariant::fromValue<QPluginLoader *>(loader);
					_descriptors << d;
				}
			} else {
				QMessageBox::warning(NULL, QObject::tr("Plugin loading error"),
				                     QObject::tr("Failed to load plugin: ") +
				                     fileFullPath + "\n\n" + loader->errorString(), QMessageBox::Close);
				delete loader;
			}
		}
	}

	NFlagIterator<N::PluginType> iter(N::MaxPluginType);
	while (iter.hasNext()) {
		iter.next();
		N::PluginType type = iter.value();
		_usedPlugins[type] = _findPlugin(type);
	}

	// unload non-used
	foreach(QPluginLoader *loader, _usedLoaders.keys(FALSE))
		loader->unload();

	if (!_usedPlugins[N::WaveformBuilderType] ||
	    !_usedPlugins[N::PlaybackEngineType] ||
	    !_usedPlugins[N::TagReaderType])
	{
		QStringList message;
		if (!_usedPlugins[N::WaveformBuilderType])
			message << QObject::tr("No Waveform plugin found.");
		if (!_usedPlugins[N::PlaybackEngineType])
			message << QObject::tr("No Playback plugin found.");
		if (!_usedPlugins[N::TagReaderType])
			message << QObject::tr("No TagReader plugin found.");
		QMessageBox::critical(NULL, QObject::tr("Plugin loading error"), message.join("\n"), QMessageBox::Close);
		exit(1);
	}
}

NPlaybackEngineInterface* NPluginLoader::playbackPlugin()
{
	_loadPlugins();
	return dynamic_cast<NPlaybackEngineInterface *>(_usedPlugins[N::PlaybackEngineType]);
}

NWaveformBuilderInterface* NPluginLoader::waveformPlugin()
{
	_loadPlugins();
	return dynamic_cast<NWaveformBuilderInterface *>(_usedPlugins[N::WaveformBuilderType]);
}

NTagReaderInterface* NPluginLoader::tagReaderPlugin()
{
	_loadPlugins();
	return dynamic_cast<NTagReaderInterface *>(_usedPlugins[N::TagReaderType]);
}

NCoverReaderInterface* NPluginLoader::coverReaderPlugin()
{
	_loadPlugins();
	return dynamic_cast<NCoverReaderInterface *>(_usedPlugins[N::CoverReaderType]);
}

QList<NPluginLoader::Descriptor> NPluginLoader::descriptors()
{
	_loadPlugins();
	return _descriptors;
}

