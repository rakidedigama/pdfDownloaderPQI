#include "ftppusher.h"
#include "pdfconverter.h"


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
    pdflistupdated = false;
    xmlFound = false;
    //fileMutex = new QMutex();

    using namespace std;


    IniFile ini("pdfDownloader.ini");
    sHost = QString::fromStdString(ini.GetSetValue("FTP", "Host","ftp.punamusta.com", "Host ip or hostname"));
    sUser = QString::fromStdString(ini.GetSetValue("FTP", "User","procemex", "FTP username"));
    sPassWord = QString::fromStdString(ini.GetSetValue("FTP", "Password","je0393", "FTP password"));
    sFolder = QString::fromStdString(ini.GetSetValue("FTP", "Folder","/work", "FTP folder"));
    pdfFolder = QString("/PDF");
    uTimeout = ini.GetSetValue("FTP", "ConnectionTimeout",5000, "Connection timeout");
    cPath = QString::fromStdString(ini.GetSetValue("FTP", "cPath","C:/PINTAWORKS/PQI/Work", "Work Folder"));

    ftp = 0;

    //oDirectory = QDir(cPath);
    //QDir oDirectory(cPath);
    qDebug() << "host is: " << sHost;
    qDebug() << "User is: " << sUser;
    qDebug() << "password is:" << sPassWord;
    qDebug() << "Pending Downloads = " << pendingDownloads.size();

    std::cout<< "host is: " << sHost.toStdString() << std::endl;
    cout<< "User is: " << sUser.toStdString();
    cout << "password is:" << sPassWord.toStdString();
    cout << "Pending Downloads = " << pendingDownloads.size();

     m_iLastGet = -1;

    if (ini.IsDirty())
        ini.Save();
    else
        qDebug()<<"Ini is clean";

    QObject::connect(this,SIGNAL(reconnect()),SLOT(connect()));
    QObject::connect(this,SIGNAL(cd(QString)),SLOT(cdToFolder(QString)));
   // QObject::connect(this,SIGNAL(pushPrintJob(QString)),this,SLOT(pushPJ(QString)));

    QObject::connect(this, SIGNAL(requestFileList()),this,SLOT(checkRemoteListing()));
    QObject::connect(this,SIGNAL(requestFileDownload(QString)),this,SLOT(getRemoteFile(QString)));
    QObject::connect(this,SIGNAL(PDFconverter()),this,SLOT(convertPDFs()));
    //QObject::connect(this,SIGNAL(requestPDFDownload(QString)),SLOT(downloadPDFs(QString)));
}


void FTPPusher::run()
{
    using namespace std;
    //QDir oDirectory;

    download_path = "";
    qDebug()<<"run()";
    //int fileCount = 0;
    while(!m_bStopRequested)
    {

        //Validating Connection
        if (QFtp::LoggedIn != m_iConnectionState && QFtp::Connecting != m_iConnectionState)
        {
            emit reconnect();
            qDebug()<<"Emit reconnect()";
            msleep(uTimeout + 1000);

        }

        //qDebug()<<"Connection okay.";
        qDebug()<<"size of pending pdf Downloads: "<<pendingDownloads.size();
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

                     download_path = cPath;
                    //download_path += temp_path;

                    if (oDirectory.exists(download_path) )
                     {
                        qDebug()<< "Directory exists " << download_path << endl;
//                        msleep(1000);
//                        pendingDownloads.removeAll(pendingDownloads.at(0));


                     }
                    else{// If directory/file does not exist, create sub directory. Open file & write to it.
                       // oDirectory.mkdir(download_path);
                       // qDebug()<< "subdirectory created: " << download_path;
                        qDebug()<<"Cannot find path";
                    }
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
                            is_transferred = false;

                            ftp->get(pendingDownloads.at(0),file);// signals to transfer file
                            cout << "Getting " << pendingDownloads.at(0).toStdString() << "("<< m_iLastGet << ")" << endl;
                            qDebug()<<"Transfering file:"<<pendingDownloads.at(0);
//                            while(!is_transferred){
//                               //qDebug()<<".";// wait for transfer to be finished
//                            }
                            qDebug()<<"Transfer finished. Clear path";
                            if(pendingDownloads.size()==0){// PDF Downloads finished.change directory back to work

                                    qDebug()<<"Start pdf conversion";

                            }
                            temp_path.clear();
                            download_path.clear();
                        }
                    }

                    msleep(2000);
                }             
            }

        else
        {

                qDebug()<<"Pending downloads empty";
                msleep(3*1000);
                //if(ftpCD.contains("work")){// rest status
                    qDebug()<<"In Work Folder";
                    emit requestFileList();  // adds job to joblist from xmls
                    emit checkPDFList(); // check pdfs for current jobid
                    //emit pdfchecks



        }
    }

        temp_path.clear();
        download_path.clear();
}



void FTPPusher::stateChanged(int iState)
{
    using namespace std;
    this->m_iConnectionState = iState;
    qDebug()<<"stateChanged() "<<m_iConnectionState;
    cout<< "stateChanged() "<<m_iConnectionState;
    switch (m_iConnectionState)
    {
        case 3: cout << "FTP Connection state changed to connectED" << endl; break;
        case 1: cout << "FTP Connection state changed to host lookup" << endl; break;
        case 2: cout << "FTP Connection state changed to connectING" << endl; break;
        case 4: cout << "FTP Connection state changed to logged in" << endl; emit cd(sFolder);break;
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

void FTPPusher::cdToFolder(QString folder)
{
    using namespace std;
    if (QFtp::LoggedIn == m_iConnectionState)// && !bCeedeed)
    {
       // << sFolder.toStdString()<<endl;
        //cout << "cd " << sFolder.toStdString() << endl;
        cout << "cd " << folder.toStdString() << endl;
        ftp->cd(folder);
    }
    else{
        cout<< "Not logged in"<<endl;
        qDebug()<<"Not logged in"<<endl;
    }
    //ftpCD = folder;
    qDebug()<<"changed folder to: " << folder;
}



void FTPPusher::transfer(qint64 done, qint64 total)
{

    using namespace std;

    //cout << "Transfer " << done << "/" << total <<endl;

    if(file->exists()){
         qDebug()<<"Transfer pdf"<< done <<"/"<<total;
         cout << "Transfer " << done << "/" << total <<endl;
        if (done/total == 1)
        {
       // ei saa olla if (fileMutex->tryLock())

            QFileInfo fileInfo(file->fileName());
            QString name(fileInfo.fileName());//file name without path
            QString fullname = file->fileName();//with path

            cout << "Close file " << name.toStdString() << endl;
            qDebug()<<"Close file" << name;
            file->close();
            qDebug()<<"Closed file"<< name;

            delete file;
            lastOkDownload = name;
            pendingDownloads.removeAll(name);//Removing file from list
            pdffilesList.removeAll(name);
            ftp->remove(name);
            is_transferred = true;

           // filesOnRemote.removeAll(name);

               qDebug()<<"Removed from pending list";
               if(pendingDownloads.size()==0){// PDF Downloads finished.change directory back to work
//                       firstxml = true;
                       //newxml = false;
//                                    qDebug()<<"Emit cd back to work";
//                                    QString tPath = "..";
//                                    tPath.append(sFolder);
                       //emit cd(tPath);
                       qDebug()<<"Start pdf conversion in transfer";
                       cout << "Start pdf conversion in transfer";
                       emit PDFconverter();
                       // Emit path to initialize pdf converter

               }
        }
    }

}

void FTPPusher::convertPDFs()
{

    qDebug()<<"Starting PDF converter";
    PDFConverter pdf(cPath);
    pdf.start();
    while(pdf.stopRequested()==false){
        //do nothing while pdf converts
    }
    pdf.exit(0);
    // check pdfs AGAIN
    //emit checkPDFList(jobID);


   /* qDebug()<<"Emit cd back to work";
    QString tPath = "..";
    tPath.append(sFolder);
   // this->cdToFolder(tPath);
   emit cd(tPath);*/ // change folder back to xml

}

void FTPPusher::checkRemoteListing()
{
    // Lits XMLs
    using namespace std;


        qDebug()<<"check remote listing";
        cout << QDateTime::currentDateTime().toString("hh:mm:ss").toStdString() << ": " << "Looking for XMLs" << endl;

           ftp->list(sFolder);


}

void FTPPusher::getRemoteFile(QString q)
{

    using namespace std;

    if (!QFileInfo(q).exists())//looks for file in download directory
    {
        //qDebug()<<"file ";
        if (!pendingDownloads.contains(q))//appending to pendingdownloads
            qDebug()<<"getRemotefile()" << q <<" added to pending downloads list"<<endl;
            cout << "getRemotefile()" << q.toStdString() <<" added to pending downloads list"<<endl;
            pendingDownloads.push_back(q);
    }
    else
    {
        qDebug()<< "getRemotefile() : File already downloaded"<< q <<endl;
        cout << "getRemotefile() : File already downloaded"<< q.toStdString() <<endl;

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
   using namespace  std;
    if(info.name().endsWith(".xml",Qt::CaseInsensitive)){ // xml folder

//        temp_path += "/";
        QString xmlname = info.name();
        xmlFound = true;
        ftp->remove(info.name());
        xmlname.remove(QChar('.'), Qt::CaseInsensitive);
        xmlname.remove(QChar('x'), Qt::CaseInsensitive);
        xmlname.remove(QChar('m'), Qt::CaseInsensitive);
        xmlname.remove(QChar('l'), Qt::CaseInsensitive);

        // Add to list and delete file
        jobID = xmlname;

    }


    if(info.name().endsWith(".pdf",Qt::CaseInsensitive)){ // pdf folder

        pdflistupdated = false;
        QString remoteFile = info.name();

        if (info.name().contains(jobID) && !pdffilesList.contains(remoteFile))//if file not there in filesonremote list
        {
           // pdffilesList.push_back(remoteFile);//equivalent to appending list
            cout << "Found PDF file " << remoteFile.toStdString() << endl;
            qDebug()<<"Found PDF file" << remoteFile<< "on Remote; Added to pdfFilesList" << "SIZE: " << info.size();
            //emit requestPDFDownload();
            pdfFound = true;
            // check pdf list, and
            emit requestFileDownload(remoteFile);

        }
        else{
            if(pdffilesList.contains(remoteFile))
                cout << "PDF File " << remoteFile.toStdString() << "already on list" ;
                qDebug()<<"PDF File " << remoteFile << "already on list" ;
        }

        pdflistupdated = true;
        //is_pdf_transferred = false;
    }
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
       // QObject::connect(ftp,SIGNAL(dataTransferProgress(qint64,qint64)),this,SLOT(transferPDF(qint64,qint64)));
        QObject::connect(ftp,SIGNAL(listInfo(QUrlInfo)),this,SLOT(fileEntry(QUrlInfo)));
        qDebug()<<"Finish initFTP()";
    //    fileMutex->unlock();
    }

  }

void FTPPusher::checkPDFList(){
    if(xmlFound==true){

        qDebug()<<"PDF Listing. Job Id : " << jobID;
        cout << "PDF Listing. Job Id : " << jobID.toStdString()<<endl;
        //if(ftpCD.contains("work")){
             QString rp_pdfFolder = "..";
            rp_pdfFolder.append(pdfFolder);
            emit cd(rp_pdfFolder);
            qDebug()<<"list in" << rp_pdfFolder;
            cout << "list in" << rp_pdfFolder.toStdString()<<endl;
            ftp->list(rp_pdfFolder);
         //}
    }


//    if(ftp->list()==0){
//        qDebug()<<"Folder empty!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
//    }   
         //adds pdf files to pdffilelist


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

