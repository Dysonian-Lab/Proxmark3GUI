#include "firmwareupdatedialog.h"
#include "ui_firmwareupdatedialog.h"
#include "common/util.h"

#include <QNetworkRequest>
#include <QFileDialog>
#include <QStandardPaths>
#include <QApplication>
#include <QDesktopServices>
#include <QUrl>

FirmwareUpdateDialog::FirmwareUpdateDialog(Util *util, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::FirmwareUpdateDialog),
    networkManager(new QNetworkAccessManager(this)),
    currentReply(nullptr),
    firmwareFile(nullptr),
    flashTimer(new QTimer(this)),
    util(util)
{
    ui->setupUi(this);

    latestFirmware.version = "v4.21611";
    latestFirmware.downloadUrl = "https://github.com/RfidResearchGroup/proxmark3/releases/download/v4.21611/proxmark3_pm3_iceman_v4.21611.zip";
    latestFirmware.fileName = "proxmark3_pm3_iceman_v4.21611.zip";

    QString downloadDir = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation);
    firmwareSavePath = downloadDir + "/proxmark3_pm3_iceman_v4.21611.zip";
    ui->savePathEdit->setText(firmwareSavePath);

    ui->latestVersionEdit->setText(latestFirmware.version);
    ui->downloadUrlEdit->setText(latestFirmware.downloadUrl);

    ui->currentVersionEdit->setText("Unknown (connect to device)");

    setUIState(false, false, false);

    connect(networkManager, &QNetworkAccessManager::finished,
            this, &FirmwareUpdateDialog::onNetworkReply);

    connect(ui->refreshCurrentButton, &QPushButton::clicked, [this]() {
        QString output = this->util->execCMDWithOutput("hw ver", Util::ReturnTrigger(10000));
        QString version;
        if (extractVersionFromOutput(output, version)) {
            ui->currentVersionEdit->setText(version);
        } else {
            ui->currentVersionEdit->setText("Unable to get version");
        }
    });

    connect(ui->browseButton, &QPushButton::clicked, [this]() {
        QString fileName = QFileDialog::getSaveFileName(this, tr("Save Firmware"),
                                                        firmwareSavePath,
                                                        tr("ZIP Files (*.zip)"));
        if (!fileName.isEmpty()) {
            firmwareSavePath = fileName;
            ui->savePathEdit->setText(firmwareSavePath);
        }
    });

    connect(flashTimer, &QTimer::timeout, this, &FirmwareUpdateDialog::onFlashProgress);
}

FirmwareUpdateDialog::~FirmwareUpdateDialog()
{
    if (currentReply) {
        currentReply->abort();
        currentReply->deleteLater();
    }
    if (firmwareFile) {
        firmwareFile->close();
        delete firmwareFile;
    }
    delete ui;
}

void FirmwareUpdateDialog::on_checkVersionButton_clicked()
{
    setUIState(true, false, false);
    ui->statusLabel->setText("Checking for latest version...");

    QNetworkRequest request;
    request.setUrl(QUrl("https://api.github.com/repos/RfidResearchGroup/proxmark3/releases/latest"));
    request.setRawHeader("User-Agent", "Proxmark3GUI");

    if (currentReply) {
        currentReply->abort();
    }
    currentReply = networkManager->get(request);
}

void FirmwareUpdateDialog::on_downloadButton_clicked()
{
    setUIState(false, true, false);
    ui->statusLabel->setText("Downloading firmware...");
    downloadFirmware(latestFirmware.downloadUrl);
}

void FirmwareUpdateDialog::on_flashButton_clicked()
{
    setUIState(false, false, false);
    ui->statusLabel->setText("Flashing firmware...");
    flashFirmware();
}

void FirmwareUpdateDialog::on_closeButton_clicked()
{
    if (flashTimer->isActive()) {
        flashTimer->stop();
    }
    if (currentReply && currentReply->isRunning()) {
        currentReply->abort();
    }
    if (firmwareFile && firmwareFile->isOpen()) {
        firmwareFile->close();
    }
    accept();
}

void FirmwareUpdateDialog::onNetworkReply(QNetworkReply *reply)
{
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray data = reply->readAll();
        QString response = QString::fromUtf8(data);

        if (reply->url().toString().contains("releases/latest")) {
            QRegularExpression versionRegex("\"tag_name\"\\s*:\\s*\"([^\"]+)\"");
            QRegularExpressionMatch match = versionRegex.match(response);
            if (match.hasMatch()) {
                QString version = match.captured(1);
                ui->latestVersionEdit->setText(version);
                latestFirmware.version = version;

                QRegularExpression urlRegex("\"browser_download_url\"\\s*:\\s*\"([^\"]+iceman[^\"]+\\.zip)\"");
                QRegularExpressionMatch urlMatch = urlRegex.match(response);
                if (urlMatch.hasMatch()) {
                    latestFirmware.downloadUrl = urlMatch.captured(1);
                    ui->downloadUrlEdit->setText(latestFirmware.downloadUrl);

                    latestFirmware.fileName = latestFirmware.downloadUrl.split("/").last();
                    firmwareSavePath = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/" + latestFirmware.fileName;
                    ui->savePathEdit->setText(firmwareSavePath);
                }
            }
            ui->statusLabel->setText("Latest version: " + latestFirmware.version);
        } else if (reply->url().toString().contains("github.com") && reply->url().toString().endsWith(".zip")) {
            if (firmwareFile && firmwareFile->isOpen()) {
                firmwareFile->write(reply->readAll());
                firmwareFile->close();
                ui->statusLabel->setText("Download complete. Ready to flash.");
                setUIState(false, false, true);
                ui->downloadProgress->setValue(100);
            }
        }
    } else {
        ui->statusLabel->setText("Error: " + reply->errorString());
        setUIState(false, false, false);
    }

    reply->deleteLater();
    currentReply = nullptr;
}

void FirmwareUpdateDialog::onDownloadProgress(qint64 received, qint64 total)
{
    if (total > 0) {
        int percentage = static_cast<int>((received * 100) / total);
        ui->downloadProgress->setValue(percentage);
        ui->statusLabel->setText(QString("Downloading... %1%").arg(percentage));
    }
}

void FirmwareUpdateDialog::onFlashProgress()
{
    ui->statusLabel->setText("Flashing in progress...");
}

void FirmwareUpdateDialog::checkForLatestVersion()
{
    on_checkVersionButton_clicked();
}

void FirmwareUpdateDialog::downloadFirmware(const QString& url)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url));
    request.setRawHeader("User-Agent", "Proxmark3GUI");

    if (currentReply) {
        currentReply->abort();
    }

    firmwareFile = new QFile(firmwareSavePath);
    if (!firmwareFile->open(QIODevice::WriteOnly)) {
        ui->statusLabel->setText("Error: Cannot create file for download");
        setUIState(false, false, false);
        return;
    }

    currentReply = networkManager->get(request);
    connect(currentReply, &QNetworkReply::downloadProgress,
            this, &FirmwareUpdateDialog::onDownloadProgress);
}

void FirmwareUpdateDialog::flashFirmware()
{
    ui->downloadProgress->setValue(0);

    QStringList cmdParts = {"flash", firmwareSavePath};
    QString flashCmd;
    for (const QString& part : cmdParts) {
        if (!flashCmd.isEmpty()) flashCmd += " ";
        flashCmd += part;
    }
    util->execCMD(flashCmd);

    flashTimer->start(500);

    QTimer::singleShot(30000, [this]() {
        flashTimer->stop();
        ui->statusLabel->setText("Flashing complete. Please disconnect and reconnect your Proxmark3.");
        ui->downloadProgress->setValue(100);
    });
}

void FirmwareUpdateDialog::setUIState(bool checking, bool downloading, bool readyToFlash)
{
    ui->checkVersionButton->setEnabled(!checking);
    ui->downloadButton->setEnabled(readyToFlash || !downloading);
    ui->flashButton->setEnabled(readyToFlash);
    ui->browseButton->setEnabled(!downloading);

    if (downloading) {
        ui->downloadProgress->setValue(0);
    }
}

bool FirmwareUpdateDialog::extractVersionFromOutput(const QString& output, QString& version)
{
    QRegularExpression versionRegex("(\\$?\\s*)?(v?\\d+\\.\\d+\\.\\d+[\\w\\-]*)",
                                    QRegularExpression::CaseInsensitiveOption);
    QRegularExpressionMatchIterator it = versionRegex.globalMatch(output);

    if (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        version = match.captured(2);
        if (!version.startsWith("v", Qt::CaseInsensitive)) {
            version = "v" + version;
        }
        return true;
    }
    return false;
}