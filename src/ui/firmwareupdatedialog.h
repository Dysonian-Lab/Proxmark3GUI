 #ifndef FIRMWAREUPDATEDIALOG_H
 #define FIRMWAREUPDATEDIALOG_H

 #include <QDialog>
 #include <QNetworkAccessManager>
 #include <QNetworkReply>
 #include <QFile>
 #include <QTimer>

 #include "common/util.h"

 namespace Ui
 {
 class FirmwareUpdateDialog;
 }

class FirmwareUpdateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FirmwareUpdateDialog(Util *util, QWidget *parent = nullptr);
    ~FirmwareUpdateDialog();

    struct FirmwareInfo
    {
        QString version;
        QString downloadUrl;
        QString fileName;
    };

private slots:
    void on_checkVersionButton_clicked();
    void on_downloadButton_clicked();
    void on_flashButton_clicked();
    void on_closeButton_clicked();

    void onNetworkReply(QNetworkReply *reply);
    void onDownloadProgress(qint64 received, qint64 total);
    void onFlashProgress();

private:
    Ui::FirmwareUpdateDialog *ui;
    QNetworkAccessManager *networkManager;
    QNetworkReply *currentReply;
    QFile *firmwareFile;
    QTimer *flashTimer;
    Util* util;

    FirmwareInfo latestFirmware;
    QString firmwareSavePath;

    void checkForLatestVersion();
    void downloadFirmware(const QString& url);
    void flashFirmware();
    void setUIState(bool checking, bool downloading, bool readyToFlash);
    bool extractVersionFromOutput(const QString& output, QString& version);
};

#endif // FIRMWAREUPDATEDIALOG_H