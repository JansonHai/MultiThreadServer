#include <map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>
#include <unistd.h>
#include <sys/stat.h>
#include "logger.h"
#include "envirment.h"

using std::string;
using std::map;
using std::pair;

static map<string, string> envirment_map;

void fl_setenv(const char * key, const char * value)
{
	if (envirment_map.find(key) != envirment_map.end())
	{
		envirment_map[key] = value;
	}
	else
	{
		envirment_map.insert(pair<string,string>(key,value));
	}
}

const char * fl_getenv(const char * key)
{
	if (envirment_map.find(key) != envirment_map.end())
	{
		return envirment_map[key].c_str();
	}
	else
	{
		return NULL;
	}
}

const char * fl_getenv(const char * key, const char * def)
{
	if (envirment_map.find(key) != envirment_map.end())
	{
		return envirment_map[key].c_str();
	}
	else
	{
		return def;
	}
}

int fl_getenv(const char * key, const int def)
{
	if (envirment_map.find(key) != envirment_map.end())
	{
		return atoi(envirment_map[key].c_str());
	}
	else
	{
		return def;
	}
}

double fl_getenv(const char * key, const double def)
{
	if (envirment_map.find(key) != envirment_map.end())
	{
		return atof(envirment_map[key].c_str());
	}
	else
	{
		return def;
	}
}

bool fl_reload_config(const char * configName)
{
	envirment_map.clear();
	bool isReloadOk = fl_load_config(configName);
	if (false == isReloadOk)
	{
		fprintf(stderr,"reload config file %s failed\n",configName);
		fl_log(2,"reload config file %s failed\n",configName);
	}
	return isReloadOk;
}

static void s_ParseValue(char * value)
{
	char tmp[1024];
	char val[1024];
	char key[1024];
	strcpy(tmp,value);
	const char * str;
	char * ch = value;
	int i,j;
	i = 0;
	while (*ch != '\0')
	{
		if ('$' == *ch && '(' == *(ch+1))
		{
			ch += 2;
			j = 0;
			while (*ch != '\0' && *ch != ')')
			{
				key[j++] = *ch;
				++ch;
			}
			key[j++] = '\0';
			if (')' == *ch) ++ch;
			str = fl_getenv(key);
			while (NULL != str && *str != '\0')
			{
				val[i++] = *str;
				++str;
			}
		}
		else
		{
			val[i++] = *ch;
			++ch;
		}
	}
	val[i++] = '\0';
	strcpy(value,val);
}

bool fl_load_config(const char * configName)
{
	//check is the file exist;
	if (0 != access(configName,F_OK))
	{
		fprintf(stderr,"config file %s is not found\n",configName);
		return false;
	}

	//check is the file a text file
	struct stat s;
	stat(configName,&s);
	if (false == S_ISREG(s.st_mode))
	{
		fprintf(stderr,"%s is not a config file\n",configName);
		return false;
	}

	//open config file,then handle
	FILE * cf = fopen(configName,"r");
	if (NULL == cf)
	{
		fprintf(stderr,"open config file %s failed\n",configName);
		return false;
	}
	char str[2048] , key[1024] , value[1024];
	int i = 0 , maxlen = 0;
	bool isKeyOk;
	char * ch;
	while (fgets(str,sizeof(str),cf) != NULL)
	{

		//this is a comment
		if ('#' == str[0] || '\n' == str[0])
		{
			continue;
		}

		//get key
		ch = str , i=0 , isKeyOk = true;
		maxlen = sizeof(key) - 1;
		while (*ch && i < maxlen)
		{
			if (' ' == *ch)
			{
				++ch;
				continue;
			}
			if ('=' == *ch)
			{
				++ch;
				break;
			}
			if ('\n' == *ch)
			{
				isKeyOk = false;
				++ch;
				break;
			}
			key[i++] = *ch++;
		}
		key[i++] = '\0';

		//check the config pair is or not only have the key,but no value
		if (false == isKeyOk)
		{
			fprintf(stderr,"error config pair found, key=[%s] found, but has no value\n",key);
			fclose(cf);
			return false;
		}

		//get value
		i=0;
		maxlen = sizeof(value) - 1;
		while ('\0' != *ch  && i < maxlen)
		{
			if (' ' == *ch|| '\n' == *ch || '\r' == *ch)
			{
				++ch;
				continue;
			}
			if ('#' == *ch) break;
			value[i++] = *ch++;
		}
		value[i++] = '\0';

		//check the value is or not empty
		if (0 == strlen(value))
		{
			fprintf(stderr,"error config pair found, key=[%s] found, but has no value\n",key);
			fclose(cf);
			return false;
		}

		s_ParseValue(value);

		if ('@' == key[0])
		{
			char configPath[2048];
			realpath(configName,configPath);
			int len = strlen(configPath);
			char * ch = &configPath[len-1];
			while (*ch != '/')
			{
				*ch = '\0';
				--ch;
			}
			char newConfigName[2048];
			sprintf(newConfigName,"%s%s", configPath, value);
			//this line means a config file, load it
			if (false == fl_load_config(newConfigName))
			{
				fclose(cf);
				return false;
			}
		}
		else
		{
			//key value pairs, set it
			fl_setenv(key, value);
		}

	}
	fclose(cf);
	return true;
}
