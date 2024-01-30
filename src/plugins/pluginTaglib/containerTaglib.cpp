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

#include "containerTaglib.h"

#include "coverReaderTaglib.h"
#include "tagReaderTaglib.h"

NContainerTaglib::NContainerTaglib(QObject *parent) : QObject(parent)
{
    m_plugins << new NTagReaderTaglib() << new NCoverReaderTaglib();
}

NContainerTaglib::~NContainerTaglib()
{
    foreach (NPlugin *plugin, m_plugins)
        delete plugin;
}

QList<NPlugin *> NContainerTaglib::plugins() const
{
    return m_plugins;
}
