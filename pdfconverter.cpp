#include "pdfconverter.h"
#include "QDebug"

PDFConverter::PDFConverter(QString dirPath,QObject *parent) :
    QThread(parent)
{
    hotFolder = dirPath;
    m_bStopRequested = false;
}

PDFConverter::~PDFConverter()
{
    m_bStopRequested = true;
    wait();
}


void PDFConverter::run()
{
    using namespace std;
    IniFile ini("PDFConverter.ini");

    //QString hotFolder = QString::fromStdString(ini.GetSetValue("Folders","HotFolder","C:/temp", "Forward slashes please..."));

    unsigned dpi = ini.GetSetValue("Resolution","dpi",100, "");

    if (ini.IsDirty())
        ini.Save();

    QDir dir(hotFolder);
    if (!dir.exists())
    {
        if (!dir.mkdir(hotFolder))
        {
            cout << hotFolder.toStdString() << " did not exist and could not be created. Exiting" << endl;
        }
        else
            cout << "Created " << hotFolder.toStdString() << endl;
    }

    QStringList types;
    types += "*.pdf";
    //types += "*.lvt"; // fiksaa


    QTextCodec *codec = QTextCodec::codecForName("UTF-8");
    QTextCodec::setCodecForLocale(codec);



    QFileInfoList list;// = dir.entryInfoList(types,QDir::NoDotAndDotDot | QDir::Files);
    //cout << " Waiting for new PDFs "  << endl;

    while (!m_bStopRequested)
    {

        list.clear();
        list = dir.entryInfoList(types,QDir::NoDotAndDotDot | QDir::Files);
        //cout << "Number of pdf files: " << list.size()<< endl;
        qDebug()<< "PDF directory is :" << dir.path();
        if(list.size()<1){
            qDebug()<<"No pdf files. Exiting conversion";
            m_bStopRequested = true;

        }
        QMap<QString, unsigned> stuff;
        foreach(QFileInfo item,list)
        {
            QString fileName = item.fileName();
            cout << "fileName :" << fileName.toStdString()<< endl;

            fileName = fileName.left(fileName.toStdString().find_last_of('.'));
            //cout << "New fileName: " << fileName.toStdString()<<endl;
            if (stuff.contains(fileName)){
                stuff[fileName] = 2;
            cout << "stuff contains" << fileName.toStdString()<<endl;
            }

            else
            {
                stuff[fileName] = 1;                
            }
        }

        for (QMap<QString, unsigned>::iterator it = stuff.begin(); it != stuff.end(); it++)
        {
            if (it.value() == 1)                 
            {
                cout << "Create LVT: "<< "Creating LVT " << endl;
                QString path(hotFolder + it.key() + ".pdf") ;
                cout << "path:" << path.toStdString()<<endl;
                if (hotFolder.endsWith('/')){
                   // cout << hotFolder << it.key() << ".pdf" << endl;
                    createLvt(hotFolder + it.key() + ".pdf",dpi);
                }
                else{
                    //cout << "Create LVT: "<< hotFolder<< "/"<< it.key() << ".pdf" << endl;
                    createLvt(hotFolder + "/" + it.key() + ".pdf",dpi);
                }
            }
            cout<<"value not 1"<<endl;
        }

        // delete converted pdf file
        msleep(1000);
    }


}
bool PDFConverter::stopRequested(){
    return m_bStopRequested;
}

bool PDFConverter::createLvt(QString fileName, unsigned dpi)
{
    using namespace std;
    QFile file("gswin64c.exe");
    if(!file.exists())
    {
        cout << "Error: missing gswin64c executable" << endl;
        return false;
    }


    QString tiff = fileName.left(fileName.toStdString().find_last_of('.')) + ".tiff";
    cout << "tiff filename: " << tiff.toStdString()<<endl;
    QString pdf = fileName.left(fileName.toStdString().find_last_of('.')) + ".pdf";
    QString pwImg = fileName.left(fileName.toStdString().find_last_of('/'));// + ".lvt"; // pelkkä kansio
    QString list;
    list += "-dBATCH -dNOPAUSE ";
    list += "-r" + QString::number(dpi);
    list += " -sDEVICE=tiff32nc ";
    list +="-o \"";
    list += tiff;
    list +="\"";

    list += " ";
    list +="\"";
    list += fileName.toUtf8();
    list +="\"";

    QString qsCommand = "gswin64c.exe "  + list;
    cout << "Running " << qsCommand.toStdString() << endl;
    if (QProcess::execute(qsCommand) < 0)
    {
        cout << "Error: Failed to execute "  << qsCommand.toStdString() << endl;
        return false;
    }    

    // Get pdf page count
    // -q -c "(in.pdf) (r) file runpdfbegin pdfpagecount = quit"
    /*QString countCMD = "gswin64c -q -c \"(" + fileName + ") (r) file runpdfbegin pdfpagecount = quit\"";
    QProcess process;
    process.start(countCMD);
    process.waitForFinished(-1); // will wait forever until finished
    QByteArray out = process.readAllStandardOutput();
    bool bOk = true;
    unsigned pageCount = QString(out).toInt(&bOk);
    if (!bOk)
    {
        cout << "Could not read how many pages in PDF, returning" << endl;
        return false;
    }

    cout << "PDF Contains " << pageCount << " pages" << endl;
    */


    TiffConverter::ConvertMultipageCMYKTiffToLVTImageCMYK(tiff.toStdString(),pwImg.toStdString(),dpi);
    QFile(tiff).remove();
    cout << "LVT Created. Deleting pdf: "<< pdf.toStdString();

    QFile f(fileName);
    QFileInfo fileInfo(f.fileName());
    QString name(fileInfo.fileName());

    QString pdfSaveFolder = hotFolder;
    pdfSaveFolder += "/pdfarchive";
    QDir dir(pdfSaveFolder);
    if (!dir.exists())
    {
        if (!dir.mkdir(pdfSaveFolder))
        {
            cout << pdfSaveFolder.toStdString() << " did not exist and could not be created. Exiting" << endl;
        }
        else
            cout << "Created " << pdfSaveFolder.toStdString() << endl;
    }
    QString pdfSrc = pdf;
    qDebug()<< "pdfSource name" << pdfSrc;
    QString pdfDst = pdfSaveFolder + '/' + name;
    qDebug()<<" pdfDest name"<< pdfDst;
    QFile::copy(pdfSrc, pdfDst);
    QFile(pdf).remove();
    return true;
}
