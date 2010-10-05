#include "t_contentinfodb.h"
#include "cpixcontentinfodbdef.h"
#include "cpixcontentinfodbread.h"
#include <qsignalspy.h>
#include <QSqlDatabase>
#include <qsqlquery.h>
#include <qdebug.h>
ContentinfodbTest::ContentinfodbTest()
    {
    }
ContentinfodbTest::~ContentinfodbTest()
    {
    }
void ContentinfodbTest::init()
    {

    }
void ContentinfodbTest::initTestCase()
    {
    }
void ContentinfodbTest::cleanupTestCase()
    {
    }
void ContentinfodbTest::Testupdate()
    {
    /*ContentInfoDbUpdate *ptr = new ContentInfoDbUpdate();
     ptr->UpdateDb("C:\\data\\contact.xml");
     ptr->UpdateDb("C:\\data\\application.xml");
     ptr->UpdateDb("C:\\data\\audio.xml");
     ptr->UpdateDb("C:\\data\\video.xml");
     ptr->UpdateDb("C:\\data\\photos.xml");
     ptr->UpdateDb("C:\\data\\notes.xml");
     ptr->UpdateDb("C:\\data\\calendar.xml");
     ptr->UpdateDb("C:\\data\\smsmms.xml");
     ptr->UpdateDb("C:\\data\\files.xml");
     ptr->UpdateDb("C:\\data\\email.xml");    */
    }
void ContentinfodbTest::TestRead()
    {
    /* ContentInfoDbReadPrivate *ptr = new ContentInfoDbReadPrivate(NULL);
     QStringList list = ptr->getPrimaryKeys();
     qDebug() << "primarykeys"<<list;
     for(int i =0;i<list.count();i++)
     {
     qDebug() << ptr->getValues(list.at(i),INDEXINGSTATUS);
     qDebug() << ptr->getValues(list.at(i),BLACKLISTSTATUS);
     qDebug() << ptr->getValues(list.at(i),DISPLAYICON);
     qDebug() << ptr->getValues(list.at(i),DISPLAYNAME);
     qDebug() << ptr->getValues(list.at(i),EXPECTIONID);
     qDebug() << ptr->getValues(list.at(i),DISPLAYORDER);
     qDebug() << ptr->getValues(list.at(i),TRANSLATION);        
     qDebug() << ptr->getShortTaburi(list.at(i));
     if (list.at(i) == "Video" || list.at(i) == "Audio" || list.at(i)
     == "Photo")
     {
     qDebug() << ptr->getLongTaburi(list.at(i), "Name");
     qDebug() << ptr->getLongTaburi(list.at(i), "Title");
     }
     else if (list.at(i) == "Contact")
     {
     qDebug() << ptr->getLongTaburi(list.at(i), "FAMILYNAME");
     qDebug() << ptr->getLongTaburi(list.at(i), "COMPANY");
     }
     else if (list.at(i) == "Application")
     {
     qDebug() << ptr->getLongTaburi(list.at(i), "Name");
     }
     else if (list.at(i) == "Notes")
     {
     qDebug() << ptr->getLongTaburi(list.at(i), "Memo");
     qDebug() << ptr->getLongTaburi(list.at(i), "Date");
     }
     else if (list.at(i) == "Calendar")
     {
     qDebug() << ptr->getLongTaburi(list.at(i), "Summary");
     qDebug() << ptr->getLongTaburi(list.at(i), "Description");
     }
     else if (list.at(i) == "smsmms")
     {
     qDebug() << ptr->getLongTaburi(list.at(i), "To");
     qDebug() << ptr->getLongTaburi(list.at(i), "Body");
     }
     else if (list.at(i) == "Files")
     {
     qDebug() << ptr->getLongTaburi(list.at(i), "Name");
     qDebug() << ptr->getLongTaburi(list.at(i), "Extension");
     }
     else if (list.at(i) == "Email")
     {
     qDebug() << ptr->getLongTaburi(list.at(i), "Sender");
     qDebug() << ptr->getLongTaburi(list.at(i), "Subject");
     }
     QStringList listuri = ptr->getActionuri(list.at(i));
     qDebug() << listuri;
     QString str1,str2;        
     for(int j=0;j<listuri.count();j++)
     {
     ptr->geturiDetails(list.at(i),listuri.at(j),str1,str2);
     qDebug() << str1<<str2;
     }
     }*/
    }
void ContentinfodbTest::TestDelete()
    {
    ContentInfoDbUpdate *ptr = new ContentInfoDbUpdate();
    ContentInfoDbReadPrivate *ptrread = new ContentInfoDbReadPrivate(NULL);
    QStringList list = ptrread->getPrimaryKeys();
    for (int i = 0; i < list.count(); i++)
        {
        if (i = 1)
            ptr->deleteCategory(list.at(i));
        }
    }
void ContentinfodbTest::TestCount()
    {
    ContentInfoDbReadPrivate *ptrread = new ContentInfoDbReadPrivate(NULL);
    int ret;
    ret = ptrread->categoryCount();
    ret = 0;
    }
QTEST_MAIN( ContentinfodbTest)

