#pragma once


#include <QtCore>
#include <fstream>
#include <iostream>
#include "index.h"


class Indexer : public QObject {
Q_OBJECT

    const QString indexPath = ".qtsearcher_index";
    const double SAVE_FACTOR = 1.1;

public:
    Indexer() = default;

    ~Indexer() {
        //saveIndex();
    }

public slots:

    void init(QString const &path) {
        rootDir = QDir(path);
        indexDir = rootDir;
        rootDir.setFilter(QDir::Files |
                          QDir::NoSymLinks |
                          QDir::NoDotAndDotDot);
        indexDir.setFilter(QDir::Files |
                           QDir::NoDotAndDotDot);

        if (!rootDir.exists(indexPath)) {
            rootDir.mkdir(indexPath);
        }
        indexDir.cd(indexPath);
        buildIndex();
        emit ready();
    }


    void findGoodFiles(QString const &query) {
        try {
            index.forEachGoodName(query.toStdString(), [this, &query](std::string name) {
                QCoreApplication::processEvents();
                QFileInfo fileInfo(QString::fromStdString(name));
                emit goodFileFound(query, fileInfo);
            });
        } catch (CancelException const &) {
            return;
        }
        emit done(query);
    };


    void cancel() {
        index.cancel();
    }


signals:
    void goodFileFound(QString query, QFileInfo const &fileInfo);
    void done(QString query);
    void ready();


private:
    void updateFileIndex(QString fileName) {
        QFileInfo indexFileInfo(indexDir, fileName);
        QFileInfo origFileInfo(rootDir, fileName);

        //std::cerr << fileName.toStdString() << std::endl;
        {
            std::ifstream stream(origFileInfo.absoluteFilePath().toStdString());
            index.remove(fileName.toStdString());
            index.add(fileName.toStdString(), stream);
        }

        indexDir.mkpath(indexFileInfo.path());
        if (SAVE_FACTOR * index.infoSize(fileName.toStdString()) < origFileInfo.size()) {
            std::ofstream stream(indexFileInfo.filePath().toStdString());
            index.save(fileName.toStdString(), stream);
        } else {
            QFile marker(indexFileInfo.absoluteFilePath());
            marker.open(QFile::WriteOnly);
            marker.close();
            //QFile::link(origFileInfo.absoluteFilePath(), indexFileInfo.absoluteFilePath());
        }
    }


    void loadFileIndex(QString fileName) {
        QFileInfo indexFileInfo(indexDir, fileName);
        QFileInfo origFileInfo(rootDir, fileName);

        if (indexFileInfo.size() == 0) {
            std::ifstream stream(origFileInfo.absoluteFilePath().toStdString());
            index.add(fileName.toStdString(), stream);
        } else {
            std::ifstream stream(indexFileInfo.absoluteFilePath().toStdString());
            index.load(fileName.toStdString(), stream);
        }
    }


    void buildIndex() {
        QDirIterator dirIterator(rootDir, QDirIterator::Subdirectories);

        while (dirIterator.hasNext()) {
            QString filePath = dirIterator.next();
            QString fileName = rootDir.relativeFilePath(filePath);

            QFileInfo indexFileInfo(indexDir, fileName);
            QFileInfo origFileInfo(rootDir, fileName);

            if (indexFileInfo.exists() && indexFileInfo.lastModified() > origFileInfo.lastModified()) {
                loadFileIndex(fileName);
            } else {
                updateFileIndex(fileName);
            }
        }
    }


    QDir rootDir;
    QDir indexDir;
    Index index;
};