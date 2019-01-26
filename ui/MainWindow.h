#pragma once

#include <QtCore>
#include <QMainWindow>
#include <QtGui/QDesktopServices>
#include "finder/finder.h"
#include "ui_MainWindow.h"
#include "indexer/indexer.h"


namespace Ui {
    class MainWindow;
}


class MainWindow : public QMainWindow {
Q_OBJECT

public:
    explicit MainWindow(QString rootPath, QWidget *parent = nullptr)
            : QMainWindow(parent),
              ui(new Ui::MainWindow),
              rootDir(rootPath) {
        ui->setupUi(this);

        setInfo("Loading index...");
        ui->dirLabel->setText(rootPath);
        ui->queryLineEdit->setEnabled(false);
        indexer = std::make_unique<Indexer>();
        indexer->moveToThread(&indexerThread);
        finder = std::make_unique<Finder>();
        finder->moveToThread(&finderThread);

        connect(ui->queryLineEdit, &QLineEdit::textChanged, this, &MainWindow::onQueryChanged);
        connect(ui->resultListWidget, &QListWidget::itemDoubleClicked, this, &MainWindow::onListItemDoubleClick);

        connect(this, &MainWindow::runIndexer, indexer.get(), &Indexer::findGoodFiles);
        connect(this, &MainWindow::cancelIndexer, indexer.get(), &Indexer::cancel);
        connect(this, &MainWindow::initIndexer, indexer.get(), &Indexer::init);
        connect(indexer.get(), &Indexer::goodFileFound, this, &MainWindow::onFoundGoodFile);
        connect(indexer.get(), &Indexer::done, this, &MainWindow::onIndexerDone);
        connect(indexer.get(), &Indexer::ready, this, &MainWindow::onIndexerReady);

        connect(finder.get(), &Finder::done, this, &MainWindow::onFinderDone);
        connect(this, &MainWindow::runFinder, finder.get(), &Finder::processFile);

        emit initIndexer(rootDir.absolutePath());

        indexerThread.start();
        finderThread.start();

        //emit runIndexer("mes");
    }


    ~MainWindow() {
        indexerThread.quit();
        indexerThread.wait();
        finderThread.quit();
        finderThread.wait();
    }

    void updateTotalCounter() {
        ui->tolalLabel->setText("Total: " + QString::number(ui->resultListWidget->count()));
    }

    void setInfo(QString info) {
        ui->infoLabel->setText(info);
    }

    void checkDone() {
        if (indexerDone && finderCounter == indexerCounter) {
            if (ui->resultListWidget->count() == 0) {
                setInfo("No results");
            } else {
                setInfo("Done.");
            }
        }
    }

public slots:

    void onIndexerReady() {
        setInfo("Ready");
        ui->queryLineEdit->setEnabled(true);
    };

    void onQueryChanged(QString const &text) {
        ui->resultListWidget->clear();
        currentQuery = text;
        updateTotalCounter();
        finderCounter = 0;
        indexerCounter = 0;
        indexerDone = false;
        if (!currentQuery.isEmpty()) {
            setInfo("Finding...");
            emit runIndexer(currentQuery);
        } else {
            setInfo("Empty query");
        }
    };

    void onFoundGoodFile(QString query, QFileInfo fileInfo) {
        QFileInfo fullFileInfo(rootDir, fileInfo.filePath());
        if (query == currentQuery) {
            indexerCounter++;
            emit runFinder(query, fullFileInfo);
        }
    }

    void onFinderDone(bool found, QString query, QFileInfo fileInfo) {
        if (query == currentQuery) {
            finderCounter++;
            if (found) {
                ui->resultListWidget->addItem(rootDir.relativeFilePath(fileInfo.filePath()));
                updateTotalCounter();
            }
            checkDone();
        }
    }

    void onIndexerDone(QString query) {
        if (query == currentQuery) {
            indexerDone = true;
            checkDone();
        }
    }

    void onListItemDoubleClick(QListWidgetItem *item) {
        QString filePath = rootDir.absoluteFilePath(item->text());
        QDesktopServices::openUrl(QUrl("file://" + filePath));
    }

signals:
    void initIndexer(QString rootPath);
    void runIndexer(QString query);
    void cancelIndexer();
    void runFinder(QString query, QFileInfo fileInfo);

private:
    std::unique_ptr<Indexer> indexer;
    std::unique_ptr<Finder> finder;
    QThread indexerThread;
    QThread finderThread;
    std::unique_ptr<Ui::MainWindow> ui;
    QDir rootDir;

    QString currentQuery;
    int finderCounter = 0;
    int indexerCounter = 0;
    bool indexerDone = true;
};