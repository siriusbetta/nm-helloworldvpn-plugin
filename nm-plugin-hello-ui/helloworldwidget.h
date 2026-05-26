#pragma once

#include <NetworkManagerQt/VpnSetting>
#include <NetworkManagerQt/ConnectionSettings>

#include "vpnuiplugin.h"

class HelloWorldWidget : public SettingWidget
{
    Q_OBJECT

public:
    explicit HelloWorldWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent = nullptr);

    void loadConfig(const NetworkManager::Setting::Ptr &setting) override;
    void loadSecrets(const NetworkManager::Setting::Ptr &setting) override;
    QVariantMap setting() const override;

private:
    NetworkManager::VpnSetting::Ptr m_setting;
};
