#include "ftppusher.h"


#include <QDir>
#include <QTimer>
#include "QDebug"

#include <string>
#include "IniFile.h"
#include <iostream>
//#include "zipper.h"

FTPPusher::FTPPusher(QObject *parent) :
    QThread(parent)
{
    m_iConnectionState = QFtp::Unconnected;
    m_bStopRequested = false;
    is_transferred = false;

    //fileMutex = new QMutex();

    using namespace std;


    IniFile ini("SeittiXMLService.ini");
    sHost = QString::fromStdString(ini.GetSetValue("FTP", "Host","//ftp.punamusta.com", "Host ip or hostname"));
    sUser = QString::fromStdString(ini.GetSetValue("FTP", "User","procemex", "FTP username"));
    sPassWord = QString::fromStdString(ini.GetSetValue("FTP", "Password","je0393", "FTP password"));
    sFolder = QString::fromStdString(ini.GetSetValue("FTP", "Folder","pdf", "FTP folder"));
    uTimeout = ini.GetSetValue("FTP", "ConnectionTimeout",5000, "Connection timeout");

    ftp = 0;
    cPath = oDirectory.currentPath();
    qDebug() << "host is: " << sHost;
    qDebug() << "User is: " << sUser;
    qDebug() << "password is:" << sPassWord;
    qDebug() << "Pending Downloads = " << pendingDownloads.size();

    //initFTP();

    /*QDir downloadDir("Done");
    if (!downloadDir.exists())
        downloadDir.mkdir("Downloads");*/

    m_iLastGet = -1;

    if (ini.IsDirty())
        ini.Save();
    else
        qDebug()<<"Ini is clean";



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
    //QDir oDirectory;

    download_path = "";
    qDebug()<<"run()";
    int fileCount = 0;
    while(!m_bStopRequested)
    {
        //Validating Connection
        if (QFtp::LoggedIn != m_iConnectionState && QFtp::Connecting != m_iConnectionState)
        {
            emit reconnect();
            qDebug()<<"Emit reconnect()";
            msleep(uTimeout + 1000);

        }

        qDebug()<<"Connection okay. Look for downloads";
        qDebug()<<"size of pending Downloads: "<<pendingDownloads.size();
        if (pendingDownloads.size()> 0)
        {
            qDebug()<<"pendingDownloads size greater than 0";
            using namespace  std;

            if (lastOkDownload == pendingDownloads.at(0) )
            {
                qDebug()<<"lastOkdownload = pendingDownloads at 0";
                pendingDownloads.removeDuplicates();
                pendingDownloads.removeAt(0);
                msleep(2000);
            }
            else
            {
                //if (fileMutex->tryLock(1000) && QFtp::Get != ftp->currentCommand())
                if ( QFtp::Get != ftp->currentCommand())
                {
                   // qDebug()<<"New QFile";
                    //Path for subdirectory
                    temp_path += "/";
                    temp_path += pendingDownloads.at(0);
                    temp_path.remove(QChar('.'), Qt::CaseInsensitive);
                    temp_path.remove(QChar('x'), Qt::CaseInsensitive);
                    temp_path.remove(QChar('m'), Qt::CaseInsensitive);
                    temp_path.remove(QChar('l'), Qt::CaseInsensitive);
                    download_path = cPath;
                    download_path += temp_path;

                    if (oDirectory.exists(download_path) )
                     {
                        qDebug()<< "Directory already exists " << download_path << endl;
                        msleep(1000);
                        //Loop between files.
                        pendingDownloads.removeAll(pendingDownloads.at(0));


                     }
                    else{// If directory/file does not exist, create sub directory. Open file & write to it.
                        oDirectory.mkdir(download_path);
                        qDebug()<< "subdirectory created: " << download_path;
                        download_path +="/";
                        //file = new QFile(pendingDownloads.at(0));
                        //file = new QFile(QString("/path/to/download/%1").arg(fileName));
                        QString fname = download_path;
                        fname.append(pendingDownloads.at(0));
                        qDebug()<<"Opening new file: " <<fname;
                        file = new QFile(fname);

                        //file = new QFile(QString(download_path).append(pendingDownloads.at(0))); // Initializing file
                        if(file->open(QIODevice::WriteOnly))
                        {
                            qDebug()<<"Download directory is "<< download_path;

                            //oDirectory.setCurrent(download_path);
                           // ftp->cd(download_path);
                            m_iLastGet = ftp->get(pendingDownloads.at(0),file);// signals to transfer file
                            cout << "Getting " << pendingDownloads.at(0).toStdString() << "("<< m_iLastGet << ")" << endl;
                            qDebug()<<"Transfering file:"<<pendingDownloads.at(0);
                            while(!is_transferred){
                               //qDebug()<<".";// wait for transfer to be finished
                             }
                        is_transferred = false;
                        //oDirectory.setCurrent(cPath);

                        // This is open for you ...
                        }
                    }
                    temp_path.clear();
                    download_path.clear();
                    msleep(2000);
                }             
            }
        }
        else
        {
            qDebug()<<"Pending downloads empty";
            msleep(3*1000);
            if (QFtp::LoggedIn == m_iConnectionState)
                qDebug()<<"emit requestFileList";
                emit requestFileList();  // Goes to checkRemoteListing > File Entry
        }
    }

    cout << "thread out" << endl;
}


void FTPPusher::stateChanged(int iState)
{

    using namespace std;
    this->m_iConnectionState = iState;
    qDebug()<<"stateChanged() "<<m_iConnectionState;
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
    qDebug()<< "Connection state: " << m_iConnectionState;
    qDebug()<< "connecting() " ;

    ftp->connectToHost(sHost);
    int iRet = ftp->login(sUser,sPassWord);
    cout << "Return signal "  << iRet << endl;
    qDebug()<< "Return signal is: "  << iRet;
    qDebug()<< "Connection state: " << m_iConnectionState;

}

void FTPPusher::cdToFolder()
{
    using namespace std;
    if (QFtp::LoggedIn == m_iConnectionState)// && !bCeedeed)
    {
       // << sFolder.toStdString()<<endl;
        cout << "cd " << sFolder.toStdString() << endl;
        ftp->cd(sFolder);
    }
    else{
        cout<< "Not logged in"<<endl;
        qDebug()<<"Not logged in"<<endl;
    }
     qDebug()<<"changed folder to: " << sFolder<<endl;
}


void FTPPusher::transfer(qint64 done, qint64 total)
{
    using namespace std;
    qDebug()<<"Transfer"<< done <<"/"<<total;
    cout << "Transfer " << done << "/" << total <<endl;
    if (done/total == 1)
    {
       // ei saa olla if (fileMutex->tryLock())
        {
            QFileInfo fileInfo(file->fileName());
            QString name(fileInfo.fileName());//file name without path
            QString fullname = file->fileName();//with path

            cout << "Close file " << name.toStdString() << endl;
            qDebug()<<"Close file" << name;
            file->close();
            delete file;
            lastOkDownload = name;

            //fileMutex->unlock();
            pendingDownloads.removeAll(name);//Removing file from list

            //ftp->remove(name);

            filesOnRemote.removeAll(name);
            is_transferred = true;
            qDebug()<<"emit HandleXML";
            emit handleIncomingXML(fullname);
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
    if (pendingDownloads.length() == 0)
    {
        qDebug()<<"check remote listing";
        cout << QDateTime::currentDateTime().toString("hh:mm:ss").toStdString() << ": " << "list files" << endl;
        //ftp->clearPendingCommands();
        qDebug()<<"listing files..."<<endl;
       ftp->list(sFolder); // Goes to fileEntry()
        // ftp->list(sFolder);
      //  fileMutex->unlock();
    }

}

void FTPPusher::getRemoteFile(QString q)
{
    //qDebug()<<"get RemoteFile: "<< q;
    using namespace std;
    //if (!QFileInfo(q).exists())
    if (!QFileInfo(q).exists())//looks for file in download directory
    {
        //qDebug()<<"file ";
        if (!pendingDownloads.contains(q))//appending to pendingdownloads
            qDebug()<<"getRemotefile()" << q <<" adding to pending downloads list"<<endl;
            pendingDownloads.push_back(q);
    }
    else
    {
        qDebug()<< "getRemotefile() : File already downloaded"<< q <<endl;
        //qDebug()<< " Remove file: "<<q ;
        //qDebug()<<"Remove file";
        //cout << "Remove file (B) " << q.toStdString() << endl;
        //ftp->remove(q);
    }
}

void FTPPusher::readyRead(int i, bool b)
{
    using namespace std;
    //cout << "Command finished " << i << endl;
    qDebug()<<"Ready Read()";

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
            //fileMutex->unlock(); // riski
        }
        //fileMutex->unlock(); // riski
    }


}

void FTPPusher::fileEntry(QUrlInfo info)
{
    qDebug()<<"FileEntry(): "<<info.name()<<"Last modified:" << info.lastModified().toString();
    using namespace  std;

    temp_path += "/";
    temp_path += info.name();
    temp_path.remove(QChar('.'), Qt::CaseInsensitive);
    temp_path.remove(QChar('x'), Qt::CaseInsensitive);
    temp_path.remove(QChar('m'), Qt::CaseInsensitive);
    temp_path.remove(QChar('l'), Qt::CaseInsensitive);
    download_path += cPath;
    download_path += temp_path;
    qDebug()<<"current path is : " << cPath;
    if(oDirectory.exists(download_path)){
        qDebug()<<"Directory" << download_path<< "already exists!!!!";
    }
    else{
        if (info.isFile())
        {
            if (info.name().endsWith(".xml",Qt::CaseInsensitive))
            {
                QString remoteFile = info.name();
                {
                    if (!filesOnRemote.contains(remoteFile))//if file not there in filesonremote list
                    {
                        filesOnRemote.push_back(remoteFile);//equivalent to appending list
                        std::cout << "Found file " << remoteFile.toStdString() << std::endl;

                        qDebug()<<"Found file" << remoteFile << "on Remote; Added to remoteList;  emit RequestFileDownload";
                        emit requestFileDownload(remoteFile);

                    }
                    else{
                        qDebug()<<"File already on RemoteList";
                    }


                }
            }

        }
    }
    temp_path.clear();
    download_path.clear();
    qDebug()<<"Path cleared";



}

void FTPPusher::initFTP()
{
       qDebug()<<"initFTP()";
  //  if (fileMutex->tryLock(10000))
    {
        if (0 != ftp)
        {
            qDebug()<<"ftp>close";
            ftp->close();
            delete ftp;
        }
        ftp = new QFtp(this);
        QObject::connect(ftp,SIGNAL(stateChanged(int)),this,SLOT(stateChanged(int)));
        QObject::connect(ftp,SIGNAL(commandFinished(int,bool)),this,SLOT(readyRead(int,bool)));
        QObject::connect(ftp,SIGNAL(dataTransferProgress(qint64,qint64)),this,SLOT(transfer(qint64,qint64)));
        QObject::connect(ftp,SIGNAL(listInfo(QUrlInfo)),this,SLOT(fileEntry(QUrlInfo)));
        qDebug()<<"Finish initFTP()";
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

