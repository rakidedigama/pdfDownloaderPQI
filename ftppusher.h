#ifndef FTPPUSHER_H
#define FTPPUSHER_H

#include <QThread>
#include <QFtp>
#include <QMutex>
#include <QStringList>
#include <QFile>

class FTPPusher : public QThread
{
    Q_OBJECT

public:
    explicit FTPPusher(QObject *parent = 0);

protected:
    virtual void run();
    int m_iConnectionState;
    bool m_bStopRequested;




    QMutex* fileMutex;
    QStringList filesOnRemote;
    QStringList pendingDownloads;
    QString lastOkDownload;
    //QString currentDownload;


    QFile* file;
    QFtp *ftp;
    QString sHost;
    QString sUser;
    QString sPassWord;
    QString sFolder;
    QString sPJFolder;
    QString sTestPJ;
    unsigned uTimeout;
    int m_iLastGet;
    
signals:
    void reconnect();
    void cd();
    void pushPrintJob(QString);
    void requestFileList();
    void requestFileDownload(QString);

    void handleIncomingXML(QString);
    
private slots:
    void stateChanged(int);
    void connect();
    void cdToFolder();
    void pushPJ(QString);
    void transfer(qint64 done, qint64 total );
    void fileEntry(QUrlInfo info);
    void checkRemoteListing();
    void getRemoteFile(QString);
    void readyRead(int, bool);
    void initFTP();

    
};

#endif // FTPPUSHER_H
