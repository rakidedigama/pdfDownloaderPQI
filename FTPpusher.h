#ifndef FTPPUSHER_H
#define FTPPUSHER_H

#include <QThread>
#include <QFtp>
#include <QMutex>
#include <QStringList>
#include <QFile>
#include <QDir>

class FTPPusher : public QThread
{
    Q_OBJECT

public:
    explicit FTPPusher(QObject *parent = 0);

protected:
    virtual void run();
    int m_iConnectionState;
    bool m_bStopRequested;
    bool is_transferred;
    bool is_pdf_transferred;
    bool pdflistupdated;
    bool firstxml;
    bool pdfFound;
    bool newxml;
    bool xmlFound;



    //QMutex* fileMutex;
    QStringList filesOnRemote;
    QStringList pendingDownloads;
    QStringList pdffilesList;
    QString lastOkDownload;
    QString temp_path;
    QString download_path;
    QString cPath;
    //QString currentDownload;

    QDir oDirectory;
    QFile* file;
    QFile* pdfFile;
    QFtp *ftp;
    QString sHost;
    QString sUser;
    QString sPassWord;
    QString sFolder;
    QString pdfFolder;
    QString ftpCD;
    QString sPJFolder;
    QString sTestPJ;
    QString jobID;
    unsigned uTimeout;
    int m_iLastGet;
    
signals:
    void reconnect();
    void cd(QString);
    void pushPrintJob(QString);
    void requestFileList();
    void requestFileDownload(QString);
    void PDFconverter();
   // void requestPDFDownload(QString);
    //void requestPDFlist(QString); // Find pdfs for xml file
    void handleIncomingXML(QString);

public slots:
    void checkPDFList();

private slots:
    //void downloadPDFs(QString);
    void convertPDFs();
    void stateChanged(int);
    void connect();
    void cdToFolder(QString);
    void pushPJ(QString);
    //void transfer(int,bool );
   // void transferPDF(qint64 done,qint64 total);

    void transfer(qint64 done, qint64 total );
    void fileEntry(QUrlInfo info);
    void checkRemoteListing();
    void getRemoteFile(QString);
    void readyRead(int, bool);
    void initFTP();

    
};

#endif // FTPPUSHER_H
