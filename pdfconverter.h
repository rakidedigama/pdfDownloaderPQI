#ifndef PDFCONVERTER_H
#define PDFCONVERTER_H

#include <QThread>
#include <QFile>
#include <QDir>
#include <QProcess>
#include <QMap>
#include <QTextCodec>

#include <iostream>

#include "IniFile.h"
#include "Tiffconverter.h"

class PDFConverter : public QThread
{
    Q_OBJECT
protected:
//    bool    m_bStopRequested;
    QString hotFolder;
    virtual void run();
    bool createLvt(QString, unsigned dpi = 100);
public:
    bool stopRequested();
    bool m_bStopRequested;
    explicit PDFConverter(QString dirpath,QObject *parent = 0);
    ~PDFConverter();
    
signals:
    
public slots:
    
};

#endif // PDFCONVERTER_H
