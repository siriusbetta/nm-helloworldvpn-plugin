#include "helloworldwidget.h"

#include <QVBoxLayout>
#include <QLabel>

HelloWorldWidget::HelloWorldWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
    : SettingWidget(setting, parent)
    , m_setting(setting)
{
    auto *layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel(QStringLiteral("HelloWorld VPN settings"), this));

    setLayout(layout);

    Q_EMIT validChanged(true);
}

void HelloWorldWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    m_setting = setting.staticCast<NetworkManager::VpnSetting>();
}

void HelloWorldWidget::loadSecrets(const NetworkManager::Setting::Ptr &setting)
{
    Q_UNUSED(setting);
}

QVariantMap HelloWorldWidget::setting() const
{
    QVariantMap result;

    QVariantMap data;
    data.insert(QStringLiteral("gateway"), QStringLiteral("127.0.0.1"));

    result.insert(QStringLiteral("service-type"), QStringLiteral("org.freedesktop.NetworkManager.helloworld"));
    result.insert(QStringLiteral("data"), data);

    return result;
}
