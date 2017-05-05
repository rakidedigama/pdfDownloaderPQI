#include "xmlmsghandler.h"

#include <iostream>
#include <QFile>
#include <QXmlStreamReader>
#include <QtXmlPatterns/QXmlSchema>
#include <QtXmlPatterns/QXmlSchemaValidator>
#include <QMap>
#include <QDateTime>
#include <QDir>

#include "json/json.h"
#include "IniFile.h"



XMLMsgHandler::XMLMsgHandler(QString ini, QObject* p)
    : QThread(p),m_mq("XMLMsgHandler","PW","SeittiXMLService.ini")

{
    m_qsIni = ini;
}

void XMLMsgHandler::findAndRunLatestQuality()
{

}

void XMLMsgHandler::handleFile(QString file)
{
    using namespace std;
    QFile fileXML(file);
    bool open = fileXML.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!open)
    {
       cout << "Couldn't open : " << file.toStdString() << endl;
       return;
    }



    IniFile ini(m_qsIni.toStdString().c_str());
    QString qsXSDPath = QString::fromStdString(ini.GetSetValue("Validation", "IncomingXSDSchema", "C:/Users/lvt/Documents/SE_XML/SE_incoming_Schema_20161125.xsd"));
    string sMongoIP = ini.GetSetValue("Mongo","IP","127.0.0.1");
    string sMongoCollection = ini.GetSetValue("Mongo","ReelDataCollection","SE.Reeldata");
    if (ini.IsDirty())
        ini.Save();


    QFile fileXSD(qsXSDPath);
    open = fileXSD.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!open)
    {
       cout << "Couldn't open xsd file: " << qsXSDPath.toStdString() << endl;
       return;
    }


    QXmlSchema schema;
    //schema.setMessageHandler(&messageHandler);

    schema.load(&fileXSD);

    bool errorOccurred = false;
    if (!schema.isValid())
    {
        errorOccurred = true;
    }
    else
    {
       QXmlSchemaValidator validator(schema);
        if (!validator.validate(&fileXML))
            errorOccurred = true;
    }

    if (errorOccurred)
    {
        cout << "Validation failed " << endl;
        return;
    }



    if (fileXML.atEnd())
        fileXML.seek(0);
    QXmlStreamReader xml(&fileXML);



    QMap<QString,QString> values;
    if (xml.readNextStartElement())
    {
            if (xml.name() == "root")
            {
                while(xml.readNextStartElement())
                {
                    cout << "Element: " << xml.name().toString().toStdString() << endl;

                    while(xml.readNextStartElement())
                    {
                        //cout << xml.name().toString().toStdString() << " : " << xml.readElementText().toStdString() <<endl;
                        //if(xml.name() == "REELNO")
                        values[xml.name().toString()] = xml.readElementText();
                    }
                }
            }
    }

    cout << values.size() << endl;
    cout << "Reel #: " << values["REELNO"].toStdString() << endl;
    cout << "Grade #: " << values["GRADEID"].toStdString() << endl;
    cout << "PM #: " << values["PMNO"].toStdString() << endl;
    cout << "INFO #: " << values["INFO"].toStdString() << endl;
    cout << "Date #: " << values["SDATETIME"].toStdString() << endl;
    cout << "OPTIONS2 #: " << values["OPTIONS2"].toStdString() << endl;
    cout << "OPTIONS3 #: " << values["OPTIONS3"].toStdString() << endl;
    cout << "OPTIONS4 #: " << values["OPTIONS4"].toStdString() << endl;
    QString dateStr = values["SDATETIME"];
    QDateTime t;
    t.setDate(QDate(
                  dateStr.mid(0,4).toInt(),
                  dateStr.mid(4,2).toInt(),
                  dateStr.mid(6,2).toInt()
                  ));
    t.setTime(QTime(
                  dateStr.mid(8,2).toInt(),
                  dateStr.mid(10,2).toInt(),
                  dateStr.mid(12,2).toInt()
                  ));
    cout << "QDateTIme: " <<  t.toString("yyyy-MM-dd hh:mm:ss").toStdString() << endl;
    QString timeString = dateStr.mid(0,4) + "-" + dateStr.mid(4,2) + "-" + dateStr.mid(6,2) + " "
            +  dateStr.mid(8,2) + ":" + dateStr.mid(10,2) +  ":" + dateStr.mid(12,2);

    cout << "Self parsed " << timeString.toStdString() << endl;

    Json::Value msg;
    //Json::Value array;
    msg["name"] = "PW_REELDATA";
    msg["dTime"] = (double)t.toMSecsSinceEpoch()/1000;
    msg["REELNO"] = values["REELNO"].toStdString();
    msg["GRADEID"] = values["GRADEID"].toStdString();
    msg["PMNO"] = values["PMNO"].toStdString();
    msg["INFO"] = values["INFO"].toStdString();

    m_mq.writeMessage("REELDATA","",false,"JSON",msg.toStyledString().c_str(),msg.toStyledString().length(),100);

    mongo::DBClientConnection c;
    try
    {
        c.connect(sMongoIP);
        std::cout << "Connected to MongoDB at " << sMongoIP << std::endl;
        mongo::BSONObjBuilder b;
        b.append("_id", (double)t.toMSecsSinceEpoch()/1000);
        b.append("REELNO",values["REELNO"].toStdString());
        b.append("GRADEID",values["GRADEID"].toStdString());
        b.append("PMNO",values["PMNO"].toStdString());
        b.append("SDATETIME",values["SDATETIME"].toStdString());
        b.append("OPTIONS2",values["OPTIONS2"].toStdString());
        b.append("OPTIONS3",values["OPTIONS3"].toStdString());
        b.append("OPTIONS4",values["OPTIONS4"].toStdString());
        b.append("INFO",values["INFO"].toStdString());
        c.insert(sMongoCollection,b.obj());
        c.ensureIndex(sMongoCollection, mongo::fromjson("{_id:1}"));
    }
    catch( const mongo::DBException &e )
    {
        cout << "Could not connect to MongoDB: " << e.what() << endl;
    }

    fileXML.close();
    if(!QDir("Done").exists())
        QDir().mkdir("Done");
    QString newName = "Done/" + file;
    QDir().rename(file,newName);
    QDir().remove(file);
}















void XMLMsgHandler::readTest()
{
   using namespace std;

    cout << "Starting... filename: C:/Users/lvt/Documents/SE_XML/S_PE5_35502839.xml"   << endl;
    QFile fileXML("C:/Users/lvt/Documents/SE_XML/S_PE5_35502839.xml");
    bool open = fileXML.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!open)
    {
       cout << "Couldn't open  xml file" << endl;
       return;
    }
    else
    {
       cout << "XML File opened OK" << endl;
    }




    QFile fileXSD("C:/Users/lvt/Documents/SE_XML/SE_incoming_Schema_20161125.xsd");
    open = fileXSD.open(QIODevice::ReadOnly | QIODevice::Text);
    if (!open)
    {
       cout << "Couldn't open xsd file" << endl;
       return;
    }
    else
    {
       cout << "XSD File opened OK" << endl;
    }


    QXmlSchema schema;
    //schema.setMessageHandler(&messageHandler);

    schema.load(&fileXSD);

    bool errorOccurred = false;
    if (!schema.isValid())
    {
        errorOccurred = true;
    }
    else
    {
       QXmlSchemaValidator validator(schema);
        if (!validator.validate(&fileXML))
            errorOccurred = true;
    }

    if (errorOccurred)
    {
        cout << "Validation failed " << endl;
    }
    else
        cout << "Validation OK!"  << endl;


    if (fileXML.atEnd())
        fileXML.seek(0);
    QXmlStreamReader xml(&fileXML);
    cout << "Encoding: " << xml.documentEncoding().toString().toStdString() << endl;


    QMap<QString,QString> values;
    if (xml.readNextStartElement())
    {
            if (xml.name() == "root")
            {
                while(xml.readNextStartElement())
                {
                    cout << "Element: " << xml.name().toString().toStdString() << endl;

                    while(xml.readNextStartElement())
                    {
                        //cout << xml.name().toString().toStdString() << " : " << xml.readElementText().toStdString() <<endl;
                        //if(xml.name() == "REELNO")
                        values[xml.name().toString()] = xml.readElementText();
                    }
                }
            }
    }

    cout << values.size() << endl;
    cout << "Reel #: " << values["REELNO"].toStdString() << endl;
    cout << "Grade #: " << values["GRADEID"].toStdString() << endl;
    cout << "PM #: " << values["PMNO"].toStdString() << endl;




}
