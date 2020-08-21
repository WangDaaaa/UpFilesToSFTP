#include <iostream>
#include "FileUpLoad.h"
using namespace std;

int main(int argc , char ** agv)
{

	if (argc < 2)
	{
		cout << "参数没添加！" << endl;
		return 0;
	}
	cout << agv[1] << endl;
	
	FileUpLoad* pThis = new FileUpLoad;

	pThis->GetZipPath(agv[1]);

	if (true == pThis->UncompressZipFile(pThis->m_str_SourceAddress))
	{
		pThis->UpLoadFuntion();

		cout << "完成..." << endl;

	}

	pThis->DeleteFiles();

	if (nullptr != pThis)
	{
		delete pThis;
	}
	return 0;
}