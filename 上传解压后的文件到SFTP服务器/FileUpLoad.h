#pragma once

#include <curl/curl.h>
#include <iostream>
#include <string>
#include <Windows.h>
#include "unzip.h"
#include <shlwapi.h>	
#include <atlstr.h>	  
#include <vector>
#include <io.h>		 
#include <direct.h>   
#include "rapidxml.hpp"
#include "rapidxml_utils.hpp"
#include "rapidxml_print.hpp"

using namespace std;
using namespace rapidxml;

class FileUpLoad
{
public:
	FileUpLoad();
	~FileUpLoad();
public:
	string m_str_url;						// sftp 地址
	string m_str_user;						// sftp 用户名
	string m_str_password;					// sftp 密码
	string m_str_info;						// 拼接user和password

	wchar_t* m_str_SourceAddress;			// 存放输入的zip文件的路径
	char   * m_buffer_zip;					// 存储当前路径信息 用于查找解压后的所有文件
	char   * m_buffer_delete;				// 存储当前路径信息 用于删除临时文件
	char   * m_buffer_xml;					// 存储当前路径信息 用于查找xml配置文件
	char   * m_buffer_in;					// 临时存放输入的路径
	char   * m_DeleteBuffer;				// 存放删除命令

	vector<string> m_files;					// 存储文件路径容器  files存储服务器路径 
	vector<string> m_temp;					// 存储文件路径容器  temp存储本地文件路径

public:

	bool GetCurrentPath();
	void GetZipPath(char* strtmp);
	void GetXMLInfo();
	bool UncompressZipFile(const wchar_t* str_SourceAddress);
	void GetFiles(string path, vector<string>& files);
	void UpLoadFuntion();
	void DeleteFiles();
};

