#include "MyProxy.h"
#include <tchar.h>

int _tmain(int argc, _TCHAR* argv[])
{
	setlocale(LC_ALL, "");
	vector<string> blackList;
	blackList.push_back("bbs.3dmgame.com");
	unordered_map<string, string> redirect;
	redirect.insert(unordered_map<string, string>::value_type("jwc.hit.edu.cn", "today.hit.edu.cn"));
	cout << "�X�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�[" << endl;
	cout << "�U                               HTTP����������ļ�ʵ��                                 �U" << endl;
	cout << "�U                                                                                        �U" << endl;
	cout << "�U                                                                           ���ߣ��ʻ�һ �U" << endl;
	cout << "�U                                                                           2017��5��6�� �U" << endl;
	cout << "�^�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�T�a" << endl;
	MyProxy my_proxy(10240, blackList, redirect);
	return 0;
}
