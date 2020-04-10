#pragma once
#include<mysql/mysql.h>
#include<iostream>
using namespace std;
#include<string>
#include<exception>

class Mysql
{
public:
	Mysql();
	~Mysql();

	MYSQL* mpcon;
	MYSQL_RES* mp_res;
	MYSQL_ROW mp_row;

private:

};
