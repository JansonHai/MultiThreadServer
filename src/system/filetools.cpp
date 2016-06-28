#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include "filetools.h"

std::string fl_pwd()
{
	char path[4096];
	if (NULL != getcwd(path,sizeof(path)))
	{
		return std::string(path);
	}
	else
	{
		return std::string("");
	}
}

int fl_cd(const char * path)
{
	return chdir(path);
}

std::string fl_getext(const char * file)
{
	if (NULL == file)
	{
		return std::string("");
	}
	char ext[128];
	char * ch;
	int len = strlen(file);
	ch = &file[len - 1];
	while (len > 0 && *ch != '.')
	{
		--len;
		--ch;
	}
	if (0 == len)
	{
		return std::string("");
	}
	++ch;
	int i=0;
	while (*ch && i < 127)
	{
		ext[i++] = *ch++;
	}
	ext[i] = '\0';
	return std::string(ext);
}

bool fl_isFile(const char * file)
{
	struct stat dirstat;
	int ret = lstat(file, &dirstat);
	if (-1 == ret)
	{
		return false;
	}
	if (S_ISREG(dirstat.st_mode))
	{
		return true;
	}
	return false;
}

bool fl_isDir(const char * dir)
{
	struct stat dirstat;
	int ret = lstat(dir, &dirstat);
	if (-1 == ret)
	{
		return false;
	}
	if (S_ISDIR(dirstat.st_mode))
	{
		return true;
	}
	return false;
}

bool fl_isFileExist(const char * file)
{
	if (0 == access(file, F_OK))
	{
		return true;
	}
	return false;
}

std::vector<std::string> fl_getdirs(const char * dirpath)
{
	std::vector<std::string> list;
	list.clear();

	if (NULL == dirpath)
	{
		return list;
	}

	char path[4096];
	strcpy(path, dirpath);
	int len = strlen(path);
	if (len > 1 && '/' == path[len -1])
	{
		path[len - 1] = '\0';
	}

	DIR *pDir = opendir(path);
	if (NULL == pDir)
	{
		return list;
	}

	struct dirent * dirinfo;
	struct stat dirstat;
	char realPath[4096];
	int ret = 0;

	while (NULL != (dirinfo = readdir(pDir)))
	{
		snprintf(realPath, sizeof(realPath), "%s/%s", path, dirinfo->d_name);
		ret = lstat(realPath, &dirstat);
		if (0 == ret)
		{
			if (S_ISDIR(dirstat.st_mode))
			{
				list.push_back(std::string(realPath));
			}
		}
	}

	closedir(pDir);

	return list;
}

void fl_getdirs(const char * dirpath,std::vector<std::string> &list)
{
	list.clear();

	if (NULL == dirpath)
	{
		return;
	}

	char path[4096];
	strcpy(path, dirpath);
	int len = strlen(path);
	if (len > 1 && '/' == path[len -1])
	{
		path[len - 1] = '\0';
	}

	DIR *pDir = opendir(path);
	if (NULL == pDir)
	{
		return;
	}

	struct dirent * dirinfo;
	struct stat dirstat;
	char realPath[4096];
	int ret = 0;

	while (NULL != (dirinfo = readdir(pDir)))
	{
		snprintf(realPath, sizeof(realPath), "%s/%s", path, dirinfo->d_name);
		ret = lstat(realPath, &dirstat);
		if (0 == ret)
		{
			if (S_ISDIR(dirstat.st_mode))
			{
				list.push_back(std::string(realPath));
			}
		}
	}

	closedir(pDir);
}


std::vector<std::string> fl_getfiles(const char * dirpath)
{
	std::vector<std::string> list;
	list.clear();

	if (NULL == dirpath)
	{
		return list;
	}

	char path[4096];
	strcpy(path, dirpath);
	int len = strlen(path);
	if (len > 1 && '/' == path[len -1])
	{
		path[len - 1] = '\0';
	}

	DIR *pDir = opendir(path);
	if (NULL == pDir)
	{
		return list;
	}

	struct dirent * dirinfo;
	struct stat dirstat;
	char realPath[4096];
	int ret = 0;

	while (NULL != (dirinfo = readdir(pDir)))
	{
		snprintf(realPath, sizeof(realPath), "%s/%s", path, dirinfo->d_name);
		ret = lstat(realPath, &dirstat);
		if (0 == ret)
		{
			if (S_ISREG(dirstat.st_mode))
			{
				list.push_back(std::string(realPath));
			}
		}
	}

	closedir(pDir);

	return list;
}

void fl_getfiles(const char * dirpath, std::vector<std::string> &list)
{
	list.clear();

	if (NULL == dirpath)
	{
		return;
	}

	char path[4096];
	strcpy(path, dirpath);
	int len = strlen(path);
	if (len > 1 && '/' == path[len -1])
	{
		path[len - 1] = '\0';
	}

	DIR *pDir = opendir(path);
	if (NULL == pDir)
	{
		return;
	}

	struct dirent * dirinfo;
	struct stat dirstat;
	char realPath[4096];
	int ret = 0;

	while (NULL != (dirinfo = readdir(pDir)))
	{
		snprintf(realPath, sizeof(realPath), "%s/%s", path, dirinfo->d_name);
		ret = lstat(realPath, &dirstat);
		if (0 == ret)
		{
			if (S_ISREG(dirstat.st_mode))
			{
				list.push_back(std::string(realPath));
			}
		}
	}

	closedir(pDir);
}

std::vector<std::string> fl_getdirall(const char * dirpath)
{
	std::vector<std::string> list;
	list.clear();

	if (NULL == dirpath)
	{
		return list;
	}

	char path[4096];
	strcpy(path, dirpath);
	int len = strlen(path);
	if (len > 1 && '/' == path[len -1])
	{
		path[len - 1] = '\0';
	}

	DIR *pDir = opendir(path);
	if (NULL == pDir)
	{
		return list;
	}

	struct dirent * dirinfo;
	struct stat dirstat;
	char realPath[4096];

	while (NULL != (dirinfo = readdir(pDir)))
	{
		snprintf(realPath, sizeof(realPath), "%s/%s", path, dirinfo->d_name);
		list.push_back(std::string(realPath));
	}

	closedir(pDir);

	return list;
}

void fl_getdirall(const char * dirpath, std::vector<std::string> &list)
{
	list.clear();

	if (NULL == dirpath)
	{
		return;
	}

	char path[4096];
	strcpy(path, dirpath);
	int len = strlen(path);
	if (len > 1 && '/' == path[len -1])
	{
		path[len - 1] = '\0';
	}

	DIR *pDir = opendir(path);
	if (NULL == pDir)
	{
		return;
	}

	struct dirent * dirinfo;
	struct stat dirstat;
	char realPath[4096];

	while (NULL != (dirinfo = readdir(pDir)))
	{
		snprintf(realPath, sizeof(realPath), "%s/%s", path, dirinfo->d_name);
		list.push_back(std::string(realPath));
	}

	closedir(pDir);
}
