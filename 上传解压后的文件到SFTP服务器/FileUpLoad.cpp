#include "FileUpLoad.h"

FileUpLoad::FileUpLoad()
{
	m_buffer_zip	= new  char[MAX_PATH];
	m_buffer_delete = new  char[MAX_PATH];
	m_buffer_xml	= new  char[MAX_PATH];
	m_buffer_in		= new  char[MAX_PATH];
/*-------------------------------------------获取当前路径 存储到buffer中-------------------------------------------------*/
	GetCurrentPath();
/*-------------------------------------------获取当前路径 存储到buffer中-------------------------------------------------*/

/*-------------------------------------------从XML文件中获取服务器信息----------------------------------------------------*/
	GetXMLInfo();
/*-------------------------------------------从XML文件中获取服务器信息----------------------------------------------------*/
	
}
FileUpLoad::~FileUpLoad()
{
	delete m_str_SourceAddress;
	delete m_buffer_zip;
	delete m_buffer_delete;
	delete m_buffer_xml;
	delete m_buffer_in;
	delete m_DeleteBuffer;
}

/*-------------------------------------------获取当前路径---------------------------------------------------------------*/
bool FileUpLoad::GetCurrentPath()
{
	if (nullptr == _getcwd(m_buffer_zip, MAX_PATH))
	{
		cout << "获取当前路径失败..." << endl;
		return false;
	}

	if (nullptr == _getcwd(m_buffer_delete, MAX_PATH))
	{
		cout << "获取当前路径失败..." << endl;
		return false;
	}

	if (nullptr == _getcwd(m_buffer_xml, MAX_PATH))
	{
		cout << "获取当前路径失败..." << endl;
		return false;
	}
	return true;

}
/*-------------------------------------------获取当前路径---------------------------------------------------------------*/

/*-------------------------------------------获取命令行输入的zip文件的路径------------------------------------------------*/
void FileUpLoad::GetZipPath(char* strtmp)
{
	
	m_buffer_in = strtmp;
	int len = MultiByteToWideChar(CP_ACP, 0, m_buffer_in, strlen(m_buffer_in), NULL, 0);
	m_str_SourceAddress = new wchar_t[len + 2];
	MultiByteToWideChar(CP_ACP, 0, m_buffer_in, strlen(m_buffer_in), m_str_SourceAddress, len);
	m_str_SourceAddress[len] = '\0';

	// 命令行参数
	m_DeleteBuffer = new char[MAX_PATH];
	strcpy_s(m_DeleteBuffer, MAX_PATH, "rd /s/q ");
}
/*-------------------------------------------获取命令行输入的zip文件的路径------------------------------------------------*/

/*-------------------------------------------读取配置文件XML中的服务器信息------------------------------------------------*/
void FileUpLoad::GetXMLInfo()
{
	//解析XML文件
	strcat_s(m_buffer_xml, MAX_PATH, "\\info.xml");

	rapidxml::file<> ffile(m_buffer_xml);

	xml_document<> doc;
	doc.parse<0>(ffile.data());
	// 获取 one  节点
	xml_node<>* one = doc.first_node();
	//获取 two  节点
	xml_node<>* two = one->first_node();
	// 获取三级节点
	xml_node<>* three = two->first_node();
	m_str_url = three->first_attribute()->value();
	m_str_url = m_str_url + "/";
	three = three->next_sibling();
	m_str_user = three->first_attribute()->value();
	three = three->next_sibling();
	m_str_password = three->first_attribute()->value();

	// 拼接账号和密码
	m_str_info = m_str_user + ":" + m_str_password;
}
/*-------------------------------------------读取配置文件XML中的服务器信息------------------------------------------------*/

/*-------------------------------------------解压str_SourceAddress中的zip文件-------------------------------------------*/
bool FileUpLoad::UncompressZipFile(const wchar_t* str_SourceAddress)
{
	// 当前路径下新建立个用于存储解压文件的文件夹
	strcat_s(m_buffer_zip, MAX_PATH,"\\Temp\\");
	strcat_s(m_buffer_delete, MAX_PATH,"\\Temp");
	
	strcat_s(m_DeleteBuffer, MAX_PATH, m_buffer_delete);
	
	CString str_TargetAddress = m_buffer_zip;
	
	// 判断地址是否存在
	if (0 == PathFileExistsW(str_SourceAddress))
	{
		// 不存在
		cout <<"路径不存在或者存在错误 请检查!"<< endl;
		return false;
	}

	// 打开zip文件 
	HZIP hzip = OpenZip(str_SourceAddress, NULL);
	if (nullptr == hzip)
	{
		cout << "open zip file failed " << str_SourceAddress << endl;
		return false;
	}

	// To zip如果有中文文件夹则会出现中文乱码

	// 含有zip文件信息的struct
	ZIPENTRY zipinfo;
	GetZipItem(hzip, -1, &zipinfo);
	int numitems = zipinfo.index;
	for (int i = 0; i < numitems; i++)
	{
		GetZipItem(hzip, i, &zipinfo);
		CString  strFile = str_TargetAddress + zipinfo.name;
		DWORD retCode = UnzipItem(hzip, i, strFile);
		if (ZR_OK != retCode)
		{
			cout << "unzip file:" << strFile.GetBuffer() << "failed, error code:" << retCode << endl;
			return 0;
		}
	}
	CloseZip(hzip);

	return true;
}
/*-------------------------------------------解压str_SourceAddress中的zip文件-------------------------------------------*/

/*-------------------------------------------获取解压出来所有文件的路径----------------------------------------------------*/
void FileUpLoad::GetFiles(string path, vector<string>& filepaths)
{
	intptr_t   hFile = 0;//文件句柄，过会儿用来查找
	struct _finddata_t fileinfo;//文件信息
	string p;
	if ((hFile = _findfirst(p.assign(path).append("/*").c_str(), &fileinfo)) != -1)
		//如果查找到第一个文件
	{
		do
		{
			if ((fileinfo.attrib & _A_SUBDIR))//如果是文件夹
			{
				if (strcmp(fileinfo.name, ".") != 0 && strcmp(fileinfo.name, "..") != 0)
					GetFiles(p.assign(path).append("/").append(fileinfo.name), filepaths);
			}
			else//如果是文件
			{
				filepaths.push_back(p.assign(path).append("/").append(fileinfo.name));
			}
		} while (_findnext(hFile, &fileinfo) == 0);	//能寻找到其他文件

		_findclose(hFile);	//结束查找，关闭句柄
	}
}
/*-------------------------------------------获取解压出来所有文件的路径----------------------------------------------------*/

/*-------------------------------------------回调函数------------------------------------------------------------------*/
size_t read_callback(void* ptr, size_t size, size_t nmemb, void* stream)
{
	return fread(ptr, size, nmemb, (FILE*)stream);
}
/*-------------------------------------------回调函数------------------------------------------------------------------*/

/*-------------------------------------------上传本地文件到sftp服务器----------------------------------------------------*/
void FileUpLoad::UpLoadFuntion()
{
	

	// 得到需要上传的文件路径
	GetFiles(m_buffer_delete, m_files);
	GetFiles(m_buffer_delete, m_temp);
	size_t nSize = strlen(m_buffer_delete) ;

	// 截断字符串
	for (auto ite = m_files.begin(); ite != m_files.end(); ite++)
	{
		(*ite).erase(0, nSize);
	}
	
	// 加载Libcur(curl_global_init)
	if (0 != curl_global_init(CURL_GLOBAL_ALL))
	{
		cout << "curl_global_init failed..." << endl;
	}

	// 实操句柄(curl_easy_init) CURL = void 
	CURL* curlHandle = curl_easy_init();
	if (nullptr == curlHandle)
	{
		cout << "curl_easy_init failed..." << endl;
	}

	// 打开本地文件
	cout << "please waiting... it is uploading..." << endl;

	// 上传本地文件
	FILE* SendFile = nullptr;
	
	for (size_t i = 0; i < m_temp.size(); i++)
	{
		if (0 != fopen_s(&SendFile, m_temp[i].c_str(), "rb"))
		{
			cout << "open file failed..." << endl;
			return;
		}
		string str_temp = m_str_url + m_files[i];
		if (nullptr == SendFile)
		{
			return;
		}
		curl_easy_setopt(curlHandle, CURLOPT_READFUNCTION, read_callback);
		curl_easy_setopt(curlHandle, CURLOPT_UPLOAD, 1L);
		curl_easy_setopt(curlHandle, CURLOPT_URL, str_temp.c_str());
		curl_easy_setopt(curlHandle, CURLOPT_READDATA, SendFile);
		curl_easy_setopt(curlHandle, CURLOPT_USERPWD, m_str_info.c_str());
		curl_easy_setopt(curlHandle, CURLOPT_FTP_CREATE_MISSING_DIRS, 1); 

		if (CURLE_OK != curl_easy_perform(curlHandle))
		{
			// 打印错误信息
			fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(curl_easy_perform(curlHandle)));
			// 卸载
			curl_easy_cleanup(curlHandle);
			curl_global_cleanup();
			fprintf(stderr, "finished update.");
			fclose(SendFile);
			return;
		}
		str_temp.clear();
		size_t pos = m_temp[i].rfind("/");
		if (pos == string::npos)
		{
			cout << "寻找/失败..." << endl;
		}
		str_temp = m_temp[i].substr(pos + 1);
		cout << to_string(i + 1) + "/" + to_string(m_temp.size()) <<"文件: "<<str_temp << "  已完成..." << endl;

		if (nullptr != SendFile)
		{
			fclose(SendFile);
		}
		str_temp.clear();
	}
	// 卸载
	curl_easy_cleanup(curlHandle);
	curl_global_cleanup();
	
}
/*-------------------------------------------上传本地文件到sftp服务器----------------------------------------------------*/

/*-------------------------------------------删除生成的临时文件-----------------------------------------------------------*/
void FileUpLoad::DeleteFiles()
{
	system(m_DeleteBuffer);
}
/*-------------------------------------------删除生成的临时文件-----------------------------------------------------------*/
