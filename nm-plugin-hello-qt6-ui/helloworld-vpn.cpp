/*
    SPDX-FileCopyrightText: 2016 Jan Grulich <jgrulich@redhat.com>

    SPDX-License-Identifier: LGPL-2.1-only OR LGPL-3.0-only OR LicenseRef-KDE-Accepted-LGPL
*/

#include "helloworld-vpn.h"
#include "helloworldwidget.h"

#include <KPluginFactory>

K_PLUGIN_CLASS_WITH_JSON(HelloWorldUiPlugin, "plasmanetworkmanagement_helloworldui.json")
 

HelloWorldUiPlugin::HelloWorldUiPlugin(QObject *parent, const QVariantList &)
    : VpnUiPlugin(parent)
{
}

HelloWorldUiPlugin::~HelloWorldUiPlugin() = default;

SettingWidget *HelloWorldUiPlugin::widget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
{
    return new HelloWorldWidget(setting, parent);
}

SettingWidget *HelloWorldUiPlugin::askUser(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent)
{
	Q_UNUSED(setting);
	Q_UNUSED(hints);
	Q_UNUSED(parent);
    return NULL;
}

QString HelloWorldUiPlugin::suggestedFileName(const NetworkManager::ConnectionSettings::Ptr &connection) const
{
    Q_UNUSED(connection);
    return {};
}

#include "helloworld-vpn.moc"

