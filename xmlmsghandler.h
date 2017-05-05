#ifndef XMLMSGHANDLER_H
#define XMLMSGHANDLER_H

#include "Windows.h"
#include "WinSock2.h"

#ifndef STATIC_LIBMONGOCLIENT
#define STATIC_LIBMONGOCLIENT
#endif

#include "WinSock2.h"
#include "Windows.h"
#include "mongo/client/dbclient.h"

#include "MQTopicInterface.h"

#include <QObject>
#include <QThread>

class XMLMsgHandler : public QThread
{
    Q_OBJECT
private:
    QString m_qsIni;
    MQTopicInterface m_mq;
public:
    XMLMsgHandler(QString ini, QObject* p = NULL);


    void findAndRunLatestQuality();
    void readTest();
public slots:
    void handleFile(QString);
};

#endif // XMLMSGHANDLER_H
