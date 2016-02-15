/****************************************************************************
**
** Copyright (C) 2009 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain
** additional rights. These rights are described in the Nokia Qt LGPL
** Exception version 1.0, included in the file LGPL_EXCEPTION.txt in this
** package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://www.qtsoftware.com/contact.
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef DATABASE_H
#define DATABASE_H

#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>

static bool createConnection()
{

QSqlDatabase dbx = QSqlDatabase::addDatabase("QSQLITE");

    dbx.setDatabaseName("./database/reg_databasevxa.sqlite");
    if (!dbx.open()) {
        QMessageBox::critical(0, qApp->tr("Cannot open database"),
            qApp->tr("Unable to establish a database connection.\n"
                     "This example needs SQLite support. Please read "
                     "the Qt SQL driver documentation for information how "
                     "to build it.\n\n"
                     "Click Cancel to exit."), QMessageBox::Cancel);
        return false;
    }

    QSqlQuery queryx;






QString createQuery;

createQuery = "CREATE TABLE IF NOT EXISTS registrations ("
"  `id` int primary key,"
"  last_name varchar(100) ,"
"  first_name  varchar(100) ,"
"  other_names varchar(100) ,"
"  dob_day varchar(2) ,"
"  dob_month varchar(4) ,"
"  dob_year varchar(4) ,"
"  mobile_number varchar(100) ,"
"  home_address varchar(255) ,"
"  state varchar(2) ,"
"  lga varchar(100) ,"
"  ward varchar(100) ,"
"  left_finger_index, "
"  left_finger_hash1 varchar(40) ,"
"  left_finger_blob1 ,"
"  left_finger_hash2 varchar(40) ,"
"  left_finger_blob2 ,"
"  left_finger_hash3 varchar(40) ,"
"  left_finger_blob3 ,"
"  right_finger_index  ,"
"  right_finger_hash1 varchar(40) ,"
"  right_finger_blob1 ,"
"  right_finger_hash2 varchar(40) ,"
"  right_finger_blob2 ,"
"  right_finger_hash3 varchar(40) ,"
"  right_finger_blob3 ,"
"  int_created datetime ,"
"  modified)";

return queryx.exec(createQuery);


 if(queryx.exec(createQuery)){
qDebug("Error Runing Table Query");
return false;
}else{
    qDebug("Table Created");
}


    return true;
/*
QSqlDatabase db = QSqlDatabase::addDatabase("QMYSQL");
db.setHostName("localhost");
db.setDatabaseName("inits_openvr");
db.setUserName("root");
db.setPassword("12345678");

bool ok = db.open();
 if (!ok) {
        QMessageBox::critical(0, void DashBoard::on_btnReports_clicked()
{
    if(controlLogin()){
        ReportsDialog *reportsDialog = new ReportsDialog(this);
        reportsDialog->exec();
    }
}qApp->tr("Cannot open database"),
            qApp->tr("Unable to establish a database connection.\n"
                     "This application needs MySQL support."
                     ".\n\n"
                     "Click Cancel to exit."), QMessageBox::Cancel);
        return false;
    }



QSqlQuery query;
query.exec("CREATE TABLE IF NOT EXISTS `registrations` ("
"  `id` int(10) unsigned NOT NULL AUTO_INCREMENT,"
"  `last_name` varchar(100) DEFAULT NULL,"
"  `first_name` varchar(100) DEFAULT NULL,"
"  `other_names` varchar(100) DEFAULT NULL,"
"  `dob_day` varchar(2) DEFAULT NULL,"
"  `dob_month` varchar(4) DEFAULT NULL,"
"  `dob_year` varchar(4) DEFAULT NULL,"
"  `mobile_number` varchar(100) DEFAULT NULL,"
"  `home_address` varchar(255) DEFAULT NULL,"
"  `state` varchar(2) DEFAULT NULL,"
"  `lga` varchar(100) DEFAULT NULL,"
"  `ward` varchar(100) DEFAULT NULL,"
"  `left_finger_index` tinyint(3) unsigned DEFAULT NULL,"
"  `left_finger_hash1` varchar(40) DEFAULT NULL,"
"  `left_finger_blob1` blob,"
"  `left_finger_hash2` varchar(40) DEFAULT NULL,"
"  `left_finger_blob2` blob,"
"  `left_finger_hash3` varchar(40) DEFAULT NULL,"
"  `left_finger_blob3` blob,"
"  `right_finger_index` tinyint(3) unsigned DEFAULT NULL,"
"  `right_finger_hash1` varchar(40) DEFAULT NULL,"
"  `right_finger_blob1` blob,"
"  `right_finger_hash2` varchar(40) DEFAULT NULL,"
"  `right_finger_blob2` blob,"
"  `right_finger_hash3` varchar(40) DEFAULT NULL,"
"  `right_finger_blob3` blob,"
"  `int_created` datetime DEFAULT NULL,"
"  `modified` timestamp NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,"
"  PRIMARY KEY (`id`)"
") ENGINE=MyISAM  DEFAULT CHARSET=latin1 AUTO_INCREMENT=1");

//QMessageBox::critical(0, "asa","ffff", QMessageBox::Cancel);
//QMessageBox::critical(0, "xxasa","ccffff", QMessageBox::Cancel);
*/

/*
    return;

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(":memory:");
    if (!db.open()) {
        QMessageBox::critical(0, qApp->tr("Cannot open database"),
            qApp->tr("Unable to establish a database connection.\n"
                     "This example needs SQLite support. Please read "
                     "the Qt SQL driver documentation for information how "
                     "to build it.\n\n"
                     "Click Cancel to exit."), QMessageBox::Cancel);
        return false;
    }
void DashBoard::on_btnReports_clicked()
{
    if(controlLogin()){
        ReportsDialog *reportsDialog = new ReportsDialog(this);
        reportsDialog->exec();
    }
}
    QSqlQuery query;

    query.exec("create table registrations (id int primary key, "
                                     "first_name varchar(100), "
                                     "last_name varchar(100), "
                                     "date_of_birth varchar(10) "
                                     );



*/
    return true;
}

#endif


