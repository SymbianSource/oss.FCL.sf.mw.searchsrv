/*
 * Copyright (c) 2009 Nokia Corporation and/or its subsidiary(-ies).
 * All rights reserved.
 * This component and the accompanying materials are made available
 * under the terms of "Eclipse Public License v1.0"
 * which accompanies this distribution, and is available
 * at the URL "http://www.eclipse.org/legal/epl-v10.html".
 *
 * Initial Contributors:
 * Nokia Corporation - initial contribution.
 *
 * Contributors:
 *
 * Description:  SQL queries, tagname,macros.
 *
 */

#ifndef CONTENTINFODBCOMMON_H_
#define CONTENTINFODBCOMMON_H_

#ifndef CONTENTINFODB_UNITTESTING
#define CONTENTINFODB_CLASS(aClassName)
#else
#define CONTENTINFODB_CLASS(aClassName) class aClassName;
#endif 

#ifndef CONTENTINFODB_UNITTESTING
#define CONTENTINFODB_FRIEND_CLASS(aClassName)
#else
#define CONTENTINFODB_FRIEND_CLASS(aClassName) friend class aClassName;
#endif 

#ifdef BUILD_CONTENTINFODB
#define DECL_CONTENTINFODB Q_DECL_EXPORT
#else
#define DECL_CONTENTINFODB Q_DECL_IMPORT
#endif

//sql queries
// sql query to create a master table
#define QUERYCREATEMAINTABLE "create table maintable (name varchar(50) primary key,ins int default '0', bls int default '0',actionuri varchar(20),licon varchar(50),baseapp varchar(50),lname varchar(50),displayorder varchar(10),exceptionstring varchar(50),tsfile varchar(50),fieldmap varchar(20),indexcontentsize varchar(5),relevancy varchar(20),policy varchar(50),resultfirstline varchar(20))"

// sql query to insert row into the mentioned table
#define QUERYINSERTROW "INSERT INTO %1 (%2) values('%3')"

//sql query to update the value on mentioned table and field
#define QUERYUPDATEDATA "UPDATE %1 SET %2='%3' WHERE %4='%5'"

//sql query to create activity uri table
#define QUERYCREATEACTIONURITABLE "create table %1 (uristring varchar(300) primary key,licon varchar(50),lname varchar(50),uriref varchar(20),action varchar(20))"

//sql query to create field map table
#define QUERYCREATEFIELDMAPTABLE "create table %1 (field varchar(300) primary key,urilist varchar(50))"

//sql query to create a table for field map values
#define QUERYCREATEFIELDMAPSUBTABLE "create table %1 (auri varchar(20) primary key)"

//sql query to create a table for firstline information
#define QUERYCREATEFIRSTLINETABLE "create table %1 (field varchar(50) primary key)"

//sql query to create a table for relevancy information
#define QUERYCREATERELEVANCYTABLE "create table %1 (field varchar(50) primary key,boostvalue varchar(20))"

#define QUERYDATARETRIVAL "SELECT %1 FROM %2 WHERE %3 ='%4'"

#define QUERYPRIMARYKEYS "SELECT DISTINCT %1 FROM %2"

#define QUERYFILTERDATA "SELECT %1 FROM %2 WHERE %3 LIKE '%%4%'"

#define QUERYSORT "SELECT name FROM maintable ORDER BY displayorder"

#define QUERYDELETE "DELETE FROM maintable WHERE name ='%1'"

#define QUERYCOUNT "SELECT COUNT(*) FROM maintable"

// Database info
// master table name
#define MAINTABLENAME "maintable"

// connection name 
#define CONNECTIONNAME "contentinfodb"

//database type
#define DATABASETYPE "QSQLITE"

//path of contentinfo db
#define DBNAME "c:\\private\\2001f6fb\\contentinfo.sq"

//dynamic table creation
#define DYNAMICTABLE "table%1"

// xml tag name
#define TAGPRIMARYKEY "name"
#define TAGACTIONURI "actionuri"
#define TAGACTIONURISTRING "uristring"
#define TAGFIELDMAP "fieldmap"
#define TAGRELEVANCY "relevancy"
#define TAGFIELD "field"
#define TAGURILIST "urilist"
#define TAGAURI "auri" 
#define TAGRESULTFIRSTLINE "resultfirstline"
#define TAGBOOSTVALUE "boostvalue"
#define TAGACTION "action"
#define TAGLNAME "lname"
#define TAGLICON "licon"
#define TAGURIREF "uriref"

#define PRIMARYVALUE "primary"
#define SECONDARYVALUE "secondary"

#endif /* CONTENTINFODBCOMMON_H_ */
