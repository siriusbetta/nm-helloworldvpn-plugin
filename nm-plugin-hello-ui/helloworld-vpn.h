
#ifndef PLASMA_NM_HELLOWORLD_VPN_H 
#define PLASMA_NM_HELLOWORLD_VPN_H 

#include "vpnuiplugin.h"

#include <QVariant>

class Q_DECL_EXPORT HelloWorldUiPlugin : public VpnUiPlugin
{
    Q_OBJECT
public:
    explicit HelloWorldUiPlugin(QObject *parent = nullptr, const QVariantList & = QVariantList());
    ~HelloWorldUiPlugin() override;
    SettingWidget *widget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent) override;
    SettingWidget *askUser(const NetworkManager::VpnSetting::Ptr &setting, const QStringList &hints, QWidget *parent) override;

    QString suggestedFileName(const NetworkManager::ConnectionSettings::Ptr &connection) const override;
};

#endif //  PLASMA_NM_IODINE_H
