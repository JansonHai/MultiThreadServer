#ifndef SRC_HEADER_FILETOOLS_H_
#define SRC_HEADER_FILETOOLS_H_

#include <stdio.h>
#include <string>
#include <vector>

//class FileTools
//{
//public:
//	static std::string pwd();
//	static int cd(const char * path);
//	static std::string getext(const char * file);
//	static bool isFile(const char * file);
//	static bool isFileExist(const char * file);
//	static bool isDir(const char * dir);
//
//	static std::vector<std::string> getdirs(const char * path);
//	static void getdirs(const char * path,std::vector<std::string> &list);
//
//	static std::vector<std::string> getfiles(const char * path);
//	static void getfiles(const char * path, std::vector<std::string> &list);
//
//	static std::vector<std::string> getdirall(const char * path);
//	static void getdirall(const char * path, std::vector<std::string> &list);
//};

std::string fl_pwd();
int fl_cd(const char * path);
std::string fl_getext(const char * file);
bool fl_isFile(const char * file);
bool fl_isDir(const char * dir);
bool fl_isFileExist(const char * file);


std::vector<std::string> fl_getdirs(const char * path);
void fl_getdirs(const char * path,std::vector<std::string> &list);

std::vector<std::string> fl_getfiles(const char * path);
void fl_getfiles(const char * path, std::vector<std::string> &list);

std::vector<std::string> fl_getdirall(const char * path);
void fl_getdirall(const char * path, std::vector<std::string> &list);


#endif /* SRC_HEADER_FILETOOLS_H_ */
