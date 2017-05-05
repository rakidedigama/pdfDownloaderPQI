#include "ftppusher.h"


#include <QDir>
#include <QTimer>

#include <string>
#include "IniFile.h"
#include <iostream>
//#include "zipper.h"

FTPPusher::FTPPusher(QObject *parent) :
    QThread(parent)
{
    m_iConnectionState = QFtp::Unconnected;
    m_bStopRequested = false;

    fileMutex = new QMutex();

    using namespace std;


    IniFile ini("SeittiXMLService.ini");
    sHost = QString::fromStdString(ini.GetSetValue("FTP", "Host","127.0.0.1", "Host ip or hostname"));
    sUser = QString::fromStdString(ini.GetSetValue("FTP", "User","user", "FTP username"));
    sPassWord = QString::fromStdString(ini.GetSetValue("FTP", "Password","password", "FTP password"));
    sFolder = QString::fromStdString(ini.GetSetValue("FTP", "Folder","/mnt/share09", "FTP folder"));
    uTimeout = ini.GetSetValue("FTP", "ConnectionTimeout",5000, "Connection timeout");

    ftp = 0;
    //initFTP();

    /*QDir downloadDir("Done");
    if (!downloadDir.exists())
        downloadDir.mkdir("Downloads");*/

    m_iLastGet = -1;

    if (ini.IsDirty())
        ini.Save();

    QObject::connect(this,SIGNAL(reconnect()),SLOT(connect()));
    QObject::connect(this,SIGNAL(cd()),SLOT(cdToFolder()));
   // QObject::connect(this,SIGNAL(pushPrintJob(QString)),this,SLOT(pushPJ(QString)));

    QObject::connect(this, SIGNAL(requestFileList()),this,SLOT(checkRemoteListing()));

    QObject::connect(this,SIGNAL(requestFileDownload(QString)),this,SLOT(getRemoteFile(QString)));



/*
    ftp = new QFtp(this);
    QObject::connect(ftp,SIGNAL(stateChanged(int)),this,SLOT(stateChanged(int)));
    QObject::connect(ftp,SIGNAL(commandFinished(int,bool)),this,SLOT(readyRead(int,bool)));
    QObject::connect(ftp,SIGNAL(dataTransferProgress(qint64,qint64)),this,SLOT(transfer(qint64,qint64)));
    QObject::connect(ftp,SIGNAL(listInfo(QUrlInfo)),this,SLOT(fileEntry(QUrlInfo)));*/

    //connect();
    //initFTP();
}

void FTPPusher::run()
{
    using namespace std;

    while(!m_bStopRequested)
    {

        if (QFtp::LoggedIn != m_iConnectionState && QFtp::Connecting != m_iConnectionState)
        {
            emit reconnect();
            msleep(uTimeout + 1000);

        }


        if (pendingDownloads.size()> 0)
        {
            using namespace  std;

            if (lastOkDownload == pendingDownloads.at(0) )
            {
                pendingDownloads.removeDuplicates();
                pendingDownloads.removeAt(0);
            }
            else
            {
                if (fileMutex->tryLock(1000) && QFtp::Get != ftp->currentCommand())
                {                    
                    file = new QFile(pendingDownloads.at(0));
                    if(file->open(QIODevice::WriteOnly))
                    {                        
                        m_iLastGet = ftp->get(pendingDownloads.at(0),file);
                        cout << "Getting " << pendingDownloads.at(0).toStdString() << "("<< m_iLastGet << ")" << endl;
                        msleep(1000);
                        // tassa jatetaan auki...
                    }
                }             
            }
        }
        else
        {

            msleep(30*1000);
            if (QFtp::LoggedIn == m_iConnectionState)
                emit requestFileList();
        }
    }

    cout << "thread out" << endl;
}


void FTPPusher::stateChanged(int iState)
{
    using namespace std;
    this->m_iConnectionState = iState;
    switch (m_iConnectionState)
    {
        case 3: cout << "FTP Connection state changed to connectED" << endl; break;
        case 1: cout << "FTP Connection state changed to host lookup" << endl; break;
        case 2: cout << "FTP Connection state changed to connectING" << endl; break;
        case 4: cout << "FTP Connection state changed to logged in" << endl; emit cd();break;
        case 5: cout << "FTP Connection state changed to closing" << endl; break;
        case 0: cout << "FTP Connection state changed to unconnected" << endl; break;
        default: cout << "unknown state " << iState << endl; break;
    }


}

void FTPPusher::connect()
{
    using namespace std;

    initFTP();
    cout << "About to connect to " << sHost.toStdString() << endl;
    ftp->connectToHost(sHost);
    int iRet = ftp->login(sUser,sPassWord);
    cout << "Return signal "  << iRet << endl;

}

void FTPPusher::cdToFolder()
{
    using namespace std;
    if (QFtp::LoggedIn == m_iConnectionState)// && !bCeedeed)
    {
        cout << "cd " << sFolder.toStdString() << endl;
        ftp->cd(sFolder);
    }
}


void FTPPusher::transfer(qint64 done, qint64 total)
{
    using namespace std;
    cout << "Transfer " << done << "/" << total <<endl;
    if (done/total == 1)
    {
       // ei saa olla if (fileMutex->tryLock())
        {            
            QString name = file->fileName();
            cout << "Close file " << name.toStdString() << endl;
            file->close();
            delete file;
            lastOkDownload = name;
            fileMutex->unlock();
            pendingDownloads.removeAll(name);

            ftp->remove(name);

            filesOnRemote.removeAll(name);

            emit handleIncomingXML(name);
        }
        /*if (pendingDownloads.size() > 1)
        {
            emit requestFileDownload(pendingDownloads.at(0));
        }*/
    }

}

void FTPPusher::checkRemoteListing()
{
    using namespace std;

    //if (fileMutex->tryLock())
  //  if (currentDownload.length() == 0)
    {

        cout << QDateTime::currentDateTime().toString("hh:mm:ss").toStdString() << ": " << "list files" << endl;
        //ftp->clearPendingCommands();
        ftp->list();
      //  fileMutex->unlock();
    }

}

void FTPPusher::getRemoteFile(QString q)
{
    using namespace std;
    if (!QFileInfo("Done/" + q).exists())
    {
        if (!pendingDownloads.contains(q))
            pendingDownloads.push_back(q);
    }
    else
    {
        cout << "Remove file (B) " << q.toStdString() << endl;
        ftp->remove(q);
    }
}

void FTPPusher::readyRead(int i, bool b)
{
    using namespace std;
    //cout << "Command finished " << i << endl;


    if (b)
    {
        cout << "Last get succesful ("<<  m_iLastGet<<")" << endl;
        /*if (i == m_iLastGet)
        {
            QString name = file->fileName();
            cout << "Close file " << name.toStdString() << endl;
            file->close();
            delete file;
            lastOkDownload = name;
            fileMutex->unlock();
            cout << "Remove file (A) " << name.toStdString() << endl;
            //ftp->remove(name);
            currentDownload = "";
        }*/
    }
    else
    {

     //   cout << "Last read failed ("<< i<< ") " << ftp->errorString().toStdString()<< endl;
        if (i == m_iLastGet)
        {
            /*cout << "MUTEX CLOSE " << currentDownload.toStdString() << endl;
            currentDownload = "";
            file->close();
            delete file;*/
            fileMutex->unlock(); // riski
        }
        fileMutex->unlock(); // riski
    }


}

void FTPPusher::fileEntry(QUrlInfo info)
{
    using namespace  std;
    if (info.isFile())
    {
        if (info.name().endsWith(".xml",Qt::CaseInsensitive))
        {
            QString remoteFile = info.name();
            {
                if (!filesOnRemote.contains(remoteFile))
                {
                    filesOnRemote.push_back(remoteFile);
                    std::cout << "Found file " << remoteFile.toStdString() << std::endl;
                    emit requestFileDownload(remoteFile);
                }

            }
        }

    }

}

void FTPPusher::initFTP()
{
  //  if (fileMutex->tryLock(10000))
    {
        if (0 != ftp)
        {
            ftp->close();
            delete ftp;
        }
        ftp = new QFtp(this);
        QObject::connect(ftp,SIGNAL(stateChanged(int)),this,SLOT(stateChanged(int)));
        QObject::connect(ftp,SIGNAL(commandFinished(int,bool)),this,SLOT(readyRead(int,bool)));
        QObject::connect(ftp,SIGNAL(dataTransferProgress(qint64,qint64)),this,SLOT(transfer(qint64,qint64)));
        QObject::connect(ftp,SIGNAL(listInfo(QUrlInfo)),this,SLOT(fileEntry(QUrlInfo)));
    //    fileMutex->unlock();
    }

   /* ftp = new QFtp(this);
    QObject::connect(ftp,SIGNAL(stateChanged(int)),this,SLOT(stateChanged(int)));
    QObject::connect(ftp,SIGNAL(dataTransferProgress(qint64,qint64)),this,SLOT(transfer(qint64,qint64)));
    QObject::connect(ftp,SIGNAL(listInfo(QUrlInfo)),this,SLOT(remoteListInfo(QUrlInfo)));
    QObject::connect(ftp,SIGNAL(commandFinished(int,bool)),this,SLOT(commandFinished(int,bool)));
    QObject::connect(ftp,SIGNAL(listInfo(QUrlInfo)),this,SLOT(listInfo(QUrlInfo)));

    QObject::connect(this,SIGNAL(sendPDF(QString,QString)),this,SLOT(pushPDF(QString,QString)));
    QObject::connect(this,SIGNAL(checkRemote()),this,SLOT(checkRemoteListing()));*/
}

void FTPPusher::pushPJ(QString qsPj)
{
    /*using namespace std;
    QString qsFolder = QString::fromStdString(this->sPJFolder);
    if (!qsFolder.endsWith("\\"))
        qsFolder = qsFolder + "\\";
    qsFolder = qsFolder + qsPj;
    cout << "About to push " << qsFolder.toStdString()<<endl;

    QString qsZipFile = qsFolder + ".zip";

    cout << "Beginning to zip " << qsFolder.toStdString() << " TO " << qsZipFile.toStdString() << endl;
    DWORD dStart = ::GetTickCount();

    bool bZipOK = Zipper::ZipDirRecursive(qsZipFile,QDir(qsFolder));
    if (bZipOK)
    {
        cout << "Zipped files in " << (::GetTickCount() - dStart)/100 << " seconds" << endl;

        cout << "Finished zipping, now push to " << QString(qsPj + ".zip").toStdString() << endl;

        QFile file(qsZipFile);
        if (file.open(QIODevice::ReadOnly))
        {
            if (m_iConnectionState != QFtp::LoggedIn)
            {
                emit reconnect();
                msleep(uTimeout + 1000);
            }
            ftp->put(file.readAll(), qsPj + ".zip");
        }
        else
            cout << "Could not open " << qsZipFile.toStdString() << endl;
    }*/
}

