#include <QCoreApplication>
#include "ftppusher.h"
#include "pdfconverter.h"
#include "QConsoleDebugStream.h"
#include "QDebug"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Q_ConsoleDebugStream console(std::cout, "log_SeittiXMLService");
    qDebug() <<"Looking for ini file"<<endl;
    FTPPusher p;

    qDebug()<<"Pusher starting";
    p.start();

    return a.exec();

}
