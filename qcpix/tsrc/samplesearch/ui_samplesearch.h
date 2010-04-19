/********************************************************************************
** Form generated from reading ui file 'SampleSearch.ui'
**
** Created: Tue Sep 8 16:44:38 2009
**      by: Qt User Interface Compiler version 4.5.2-tower
**
** WARNING! All changes made in this file will be lost when recompiling ui file!
********************************************************************************/

#ifndef UI_SAMPLESEARCH_H
#define UI_SAMPLESEARCH_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QHeaderView>
#include <QtGui/QMainWindow>
#include <QtGui/QMenuBar>
#include <QtGui/QStatusBar>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class Ui_SampleSearchClass
{
public:
    QWidget *centralwidget;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *SampleSearchClass)
    {
        if (SampleSearchClass->objectName().isEmpty())
            SampleSearchClass->setObjectName(QString::fromUtf8("SampleSearchClass"));
        SampleSearchClass->resize(800, 600);
        centralwidget = new QWidget(SampleSearchClass);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        SampleSearchClass->setCentralWidget(centralwidget);
        menubar = new QMenuBar(SampleSearchClass);
        menubar->setObjectName(QString::fromUtf8("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 21));
        SampleSearchClass->setMenuBar(menubar);
        statusbar = new QStatusBar(SampleSearchClass);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        SampleSearchClass->setStatusBar(statusbar);

        retranslateUi(SampleSearchClass);

        QMetaObject::connectSlotsByName(SampleSearchClass);
    } // setupUi

    void retranslateUi(QMainWindow *SampleSearchClass)
    {
        SampleSearchClass->setWindowTitle(QApplication::translate("SampleSearchClass", "MainWindow", 0, QApplication::UnicodeUTF8));
        Q_UNUSED(SampleSearchClass);
    } // retranslateUi

};

namespace Ui {
    class SampleSearchClass: public Ui_SampleSearchClass {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SAMPLESEARCH_H
