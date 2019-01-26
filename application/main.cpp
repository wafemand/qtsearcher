#include <iostream>
#include <QApplication>
#include <QtCore/QFileInfo>
#include <QtWidgets/QFileDialog>
#include "ui/MainWindow.h"

int main(int argc, char **argv) {
    qRegisterMetaType<QFileInfo>("QFileInfo");

    QApplication application(argc, argv);
    QString rootPath;
    if (application.arguments().size() > 1){
        rootPath = application.arguments()[1];
    } else {
        rootPath = QFileDialog::getExistingDirectory(
                nullptr,
                "Select directory for search",
                QString(),
                QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);
    }

    MainWindow mainWindow(rootPath);
    mainWindow.show();

    application.exec();

    return 0;
}