#include "helloworldwidget.h"

#include <QVBoxLayout>
#include <QLabel>

HelloWorldWidget::HelloWorldWidget(const NetworkManager::VpnSetting::Ptr &setting, QWidget *parent)
    : SettingWidget(setting, parent)
    , m_setting(setting)
{

    lineEditPath = new QLineEdit(this);
    lineEditPath->setReadOnly(true);
    lineEditPath->setPlaceholderText("Путь к файлу не выбран");
    lineEditPath->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    buttonBrowse = new QPushButton("Выбрать файл", this);
    buttonShowName = new QPushButton("Показать имя файла", this);

    auto* pathLayout = new QHBoxLayout;
    pathLayout->addWidget(lineEditPath);
    pathLayout->addWidget(buttonBrowse);

    auto* mainLayout = new QVBoxLayout(this);
    mainLayout->addLayout(pathLayout);
    mainLayout->addWidget(buttonShowName);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(10);

    connect(buttonBrowse, &QPushButton::clicked, this, &HelloWorldWidget::onBrowseClicked);
    connect(buttonShowName, &QPushButton::clicked, this, &HelloWorldWidget::onShowNameClicked);

    setLayout(mainLayout);
    setWindowTitle("Выбор файла");
    loadConfig(setting);
}

void HelloWorldWidget::loadConfig(const NetworkManager::Setting::Ptr &setting)
{
    m_setting = setting.staticCast<NetworkManager::VpnSetting>();

    const QVariantMap data = m_setting->data();
    QString path = data.value(QStringLiteral("config")).toString();
    if (!path.isEmpty()) {
        m_pathKey = QStringLiteral("config");
    } else {
        path = data.value(QStringLiteral("service")).toString();
        if (!path.isEmpty())
            m_pathKey = QStringLiteral("service");
    }

    lineEditPath->setText(path);
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

    const QString path = lineEditPath->text();
    if (!path.isEmpty())
        data.insert(m_pathKey, path);

    result.insert(QStringLiteral("service-type"), QStringLiteral("org.freedesktop.NetworkManager.helloworld"));
    result.insert(QStringLiteral("data"), data);

    return result;
}

void HelloWorldWidget::onBrowseClicked() {
	QString filePath = QFileDialog::getOpenFileName(this, "Выберите файл");
	if (!filePath.isEmpty()) {
	    lineEditPath->setText(filePath);
	    Q_EMIT changed();
	}
}

void HelloWorldWidget::onShowNameClicked() {
	QString filePath = lineEditPath->text();
	if (filePath.isEmpty()) {
	    QMessageBox::information(this, "Информация", "Сначала выберите файл.");
	    return;
	}
	QString fileName = QFileInfo(filePath).fileName();
	QMessageBox::information(this, "Имя файла", QString("Выбран файл:\n<b>%1</b>").arg(fileName));
}
