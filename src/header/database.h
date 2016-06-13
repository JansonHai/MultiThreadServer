#ifndef SRC_DATABASE_H_
#define SRC_DATABASE_H_

#include <stdio.h>
#include <mysql/mysql.h>

struct fl_mysql_conn
{
	int id;
	MYSQL * mysql;
	MYSQL_RES * result;
	bool isFreeResult;
	MYSQL_ROW row;
	int rowCount, maxRowCount;
	MYSQL_FIELD * field;

public:
	fl_mysql_conn()
	{
		id = -1;
		mysql = NULL;
		result = NULL;
		row = NULL;
		rowCount = 0;
		maxRowCount = 0;
		field = NULL;
		isFreeResult = true;
	}

};

void fl_database_init();
fl_mysql_conn * fl_database_get_conn();
void fl_database_reuse_conn(fl_mysql_conn * conn);
bool fl_database_connect(fl_mysql_conn * conn,const char * host,const char * user,const char * passwd,const char * db,unsigned int port);
bool fl_database_query(fl_mysql_conn * conn, const char * sql);
bool fl_database_real_query(fl_mysql_conn * conn, const char * sql, unsigned long length);
bool fl_database_fetch_row(fl_mysql_conn * conn);
void fl_database_free_result(fl_mysql_conn * conn);


#endif /* SRC_DATABASE_H_ */
