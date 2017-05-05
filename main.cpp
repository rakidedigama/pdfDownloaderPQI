#include <QCoreApplication>
#include "ftppusher.h"
#include "xmlmsghandler.h"
#include "QConsoleDebugStream.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    Q_ConsoleDebugStream console(std::cout, "log_SeittiXMLService");

    XMLMsgHandler m("SeittiXMLService.ini");
    FTPPusher p;

    QObject::connect(&p,SIGNAL(handleIncomingXML(QString)),&m,SLOT(handleFile(QString)));
    p.start();

    return a.exec();
}
