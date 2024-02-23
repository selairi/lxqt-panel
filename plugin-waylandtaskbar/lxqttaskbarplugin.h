/* BEGIN_COMMON_COPYRIGHT_HEADER
 * (c)LGPL2+
 *
 * LXQt - a lightweight, Qt based, desktop toolset
 * https://lxqt.org
 *
 * Copyright: 2012 Razor team
 * Authors:
 *   Alexander Sokoloff <sokoloff.a@gmail.com>
 *
 * This program or library is free software; you can redistribute it
 * and/or modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.

 * You should have received a copy of the GNU Lesser General
 * Public License along with this library; if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA
 *
 * END_COMMON_COPYRIGHT_HEADER */


#ifndef LXQTTASKBARPLUGIN_H
#define LXQTTASKBARPLUGIN_H

#include "../panel/ilxqtpanel.h"
#include "../panel/ilxqtpanelplugin.h"
#include "lxqttaskbar.h"
#include <QDebug>
class LXQtTaskBar2;

class LXQtWaylandTaskBarPlugin : public QObject, public ILXQtPanelPlugin
{
    Q_OBJECT
public:
    LXQtWaylandTaskBarPlugin(const ILXQtPanelPluginStartupInfo &startupInfo);
    ~LXQtWaylandTaskBarPlugin();

    QString themeId() const { return QStringLiteral("WaylandTaskBar"); }
    virtual Flags flags() const { return HaveConfigDialog | NeedsHandle; }

    QWidget *widget() { return mTaskBar2; }
    QDialog *configureDialog();

    void settingsChanged() { mTaskBar2->settingsChanged(); }
    void realign();

    bool isSeparate() const { return true; }
    bool isExpandable() const { return true; }
private:
    LXQtTaskBar2 *mTaskBar2;
};

class LXQtWaylandTaskBarPluginLibrary: public QObject, public ILXQtPanelPluginLibrary
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "lxqt.org/Panel/PluginInterface/3.0")
    Q_INTERFACES(ILXQtPanelPluginLibrary)
public:
    ILXQtPanelPlugin *instance(const ILXQtPanelPluginStartupInfo &startupInfo) const { return new LXQtWaylandTaskBarPlugin(startupInfo);}
};

#endif // LXQTTASKBARPLUGIN_H
