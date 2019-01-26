#pragma once


#include <QtCore>
#include <fstream>
#include <iostream>
#include "finder/prefix_function.h"


class Finder : public QObject {
Q_OBJECT

public:
    Finder(){

    }

    ~Finder() {
    }

public slots:
    void processFile(QString query, QFileInfo fileInfo) {
        prefix_function<std::string> pf(query.toStdString());
        QFile file(fileInfo.absoluteFilePath());
        file.open(QIODevice::ReadOnly);
        char c;
        while (file.getChar(&c)) {
            pf.update(c);
            if (pf.matched()) {
                emit done(true, query, fileInfo);
                return;
            }
        }
        emit done(false, query, fileInfo);
    }

signals:
    void done(bool found, QString query, QFileInfo fileInfo);


private:

};