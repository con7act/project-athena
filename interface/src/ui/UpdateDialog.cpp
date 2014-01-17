//
//  UpdateDialog.cpp
//  interface
//
//  Created by Leonardo Murillo <leo@highfidelity.io> on 1/8/14.
//  Copyright (c) 2013, 2014 High Fidelity, Inc. All rights reserved.
//

#include <QApplication>
#include <QDesktopWidget>
#include <QTextBlock>
#include <QtGui>
#include <QtUiTools>

#include "Application.h"
#include "SharedUtil.h"
#include "UpdateDialog.h"

UpdateDialog::UpdateDialog(QWidget *parent, QString releaseNotes, QString latestVersion, QUrl downloadURL) : QDialog(parent, Qt::Dialog) {
    Application* application = Application::getInstance();
    
    QUiLoader updateDialogLoader;
    
    QFile updateDialogUi("resources/ui/updateDialog.ui");
    updateDialogUi.open(QFile::ReadOnly);
    dialogWidget = updateDialogLoader.load(&updateDialogUi, parent);
    updateDialogUi.close();
    
    const QString updateRequired = QString("You are currently running build %1, the latest build released is %2. \
                                           Please download and install the most recent release to access the latest features and bug fixes.")
                                           .arg(application->applicationVersion(), latestVersion);
    
    
    setAttribute(Qt::WA_DeleteOnClose);
    
    QPushButton *downloadButton = dialogWidget->findChild<QPushButton *>("downloadButton");
    QPushButton *skipButton = dialogWidget->findChild<QPushButton *>("skipButton");
    QPushButton *closeButton = dialogWidget->findChild<QPushButton *>("closeButton");
    QLabel *updateContent = dialogWidget->findChild<QLabel *>("updateContent");
    
    updateContent->setText(updateRequired);
    
    connect(downloadButton, SIGNAL(released()), this, SLOT(handleDownload(QUrl downloadURL)));
    connect(skipButton, SIGNAL(released()), this, SLOT(handleSkip()));
    connect(closeButton, SIGNAL(released()), this, SLOT(handleClose()));
    dialogWidget->show();
}

void UpdateDialog::handleDownload(QUrl downloadURL) {
    Application* application = Application::getInstance();
    QDesktopServices::openUrl(downloadURL);
    application->quit();
}

void UpdateDialog::handleSkip() {
    this->close();
}

void UpdateDialog::handleClose() {
    this->close();
}