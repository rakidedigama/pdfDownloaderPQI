#include <QCoreApplication>
#include "ftppusher.h"
#include "xmlmsghandler.h"
#include "QConsoleDebugStream.h"
#include "QDebug"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Q_ConsoleDebugStream console(std::cout, "log_SeittiXMLService");
 //   qDebug() <<"Looking for ini file"<<endl;
    XMLMsgHandler m("SeittiXMLService.ini");
    FTPPusher p;

    QObject::connect(&p,SIGNAL(handleIncomingXML(QString)),&m,SLOT(handleFile(QString)));

    qDebug()<<"Pusher starting";
    p.start();



    //p.connect();
    //p.cdToFolder();



    return a.exec();

}
