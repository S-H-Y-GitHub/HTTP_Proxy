#pragma once
#include <stdio.h>
#include <Windows.h>
#include <process.h>
#include <string.h>
#include <tchar.h>
#include <iostream>

#pragma comment(lib,"Ws2_32.lib")

#define MAXSIZE 65507 //�������ݱ��ĵ���󳤶�
#define HTTP_PORT 80 //http �������˿�

class MyProxy
{
public:
	MyProxy();
	~MyProxy();
private:
	//Http ��Ҫͷ������
	struct HttpHeader {
		char method[4]; // POST ���� GET��ע����ЩΪ CONNECT����ʵ���ݲ�����
		char url[1024];       // ����� url
		char host[1024]; // Ŀ������ 
		char cookie[1024 * 10]; //cookie
		HttpHeader() {
			ZeroMemory(this, sizeof(HttpHeader));
		}
	};

	//������ز���
	SOCKET ProxyServer;
	sockaddr_in ProxyServerAddr;
	const int ProxyPort = 10240;

	//�����µ����Ӷ�ʹ�����߳̽��д������̵߳�Ƶ���Ĵ����������ر��˷���Դ
	//����ʹ���̳߳ؼ�����߷�����Ч��
	//const int ProxyThreadMaxNum = 20;
	//HANDLE ProxyThreadHandle[ProxyThreadMaxNum] = {0};
	//DWORD ProxyThreadDW[ProxyThreadMaxNum] = {0};

	struct ProxyParam {
		SOCKET clientSocket;
		SOCKET serverSocket;
	};

	BOOL InitSocket();
	static unsigned __stdcall ProxyThread(LPVOID lpParameter);
	static void ParseHttpHead(char* buffer, HttpHeader* httpHeader);
	static BOOL ConnectToServer(SOCKET* serverSocket, char* host);
};

inline MyProxy::MyProxy()
{
	printf("�����������������\n");
	printf("��ʼ��...\n");
	if (!InitSocket()) {
		printf("socket ��ʼ��ʧ��\n");
		return;
	}
	printf("����������������У������˿� %d\n", ProxyPort);
	SOCKET acceptSocket = INVALID_SOCKET;
	//������������ϼ���
	while (true) {
		acceptSocket = accept(ProxyServer, nullptr, nullptr);
		ProxyParam *lpProxyParam = new ProxyParam;
		if (lpProxyParam == nullptr) {
			continue;
		}
		lpProxyParam->clientSocket = acceptSocket;
		HANDLE hThread = reinterpret_cast<HANDLE>(_beginthreadex(nullptr, 0, &ProxyThread, static_cast<LPVOID>(lpProxyParam), 0, nullptr));
		CloseHandle(hThread);
		Sleep(200);
	}
}

inline MyProxy::~MyProxy()
{
	closesocket(ProxyServer);
	WSACleanup();
}

//************************************
// Method:	InitSocket
// FullName:	InitSocket
// Access:	public
// Returns:	BOOL
// Qualifier: ��ʼ���׽���
//************************************ 
inline BOOL MyProxy::InitSocket() {
	WSADATA wsaData;
	//�汾 2.2
	WORD wVersionRequested = MAKEWORD(2, 2);
	//���� dll �ļ� Scoket ��
	int err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		//�Ҳ��� winsock.dll
		printf("���� winsock ʧ�ܣ��������Ϊ: %d\n", WSAGetLastError());
		return FALSE;
	}
	if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2)
	{
		printf("�����ҵ���ȷ�� winsock �汾\n");
		WSACleanup();
		return FALSE;
	}
	ProxyServer = socket(AF_INET, SOCK_STREAM, 0);
	if (INVALID_SOCKET == ProxyServer) {
		printf("�����׽���ʧ�ܣ��������Ϊ��%d\n", WSAGetLastError());
		return FALSE;
	}
	ProxyServerAddr.sin_family = AF_INET;
	ProxyServerAddr.sin_port = htons(ProxyPort);
	ProxyServerAddr.sin_addr.S_un.S_addr = INADDR_ANY;
	if (bind(ProxyServer, reinterpret_cast<SOCKADDR*>(&ProxyServerAddr), sizeof(SOCKADDR)) == SOCKET_ERROR) {
		printf("���׽���ʧ��\n");
		return FALSE;
	}
	if (listen(ProxyServer, SOMAXCONN) == SOCKET_ERROR) {
		printf("�����˿�%d ʧ��", ProxyPort);
		return FALSE;
	}
	return TRUE;
}

//************************************
// Method:	ProxyThread
// FullName:	ProxyThread
// Access:	public
// Returns:	unsigned int __stdcall
// Qualifier: �߳�ִ�к���
// Parameter: LPVOID lpParameter
//************************************
inline unsigned int __stdcall MyProxy::ProxyThread(LPVOID lpParameter)
{
	char Buffer[MAXSIZE];
	char *CacheBuffer;
	ZeroMemory(Buffer, MAXSIZE);
	int recvSize;
	recvSize = recv(static_cast<ProxyParam*>(lpParameter)->clientSocket, Buffer, MAXSIZE, 0);
	if (recvSize <= 0)
	{
		std::cout << "û���յ�����" << std::endl;
		goto error;
	}
	HttpHeader* httpHeader = new HttpHeader();
	CacheBuffer = new char[recvSize + 1];
	ZeroMemory(CacheBuffer, recvSize + 1);
	memcpy(CacheBuffer, Buffer, recvSize);
	ParseHttpHead(CacheBuffer, httpHeader);
	delete[] CacheBuffer;
	if (!ConnectToServer(&static_cast<ProxyParam*>(lpParameter)->serverSocket, httpHeader->host))
	{
		std::cout << "���ӷ�����ʧ�ܣ�" << std::endl;
		goto error;
	}
	printf("������������ %s �ɹ�\n", httpHeader->host);
	//���ͻ��˷��͵� HTTP ���ݱ���ֱ��ת����Ŀ�������
	int ret = send(static_cast<ProxyParam *>(lpParameter)->serverSocket, Buffer, recvSize, 0);
	//�ȴ�Ŀ���������������
	recvSize = recv(static_cast<ProxyParam*>(lpParameter)->serverSocket, Buffer, MAXSIZE, 0);
	if (recvSize <= 0)
	{
		std::cout << "������û�з������ݣ�" << std::endl;
		goto error;
	}
	//��Ŀ����������ص�����ֱ��ת�����ͻ��� 
	ret = send(static_cast<ProxyParam*>(lpParameter)->clientSocket, Buffer, sizeof(Buffer), 0);
	//������
error:
	Sleep(200);
	closesocket(static_cast<ProxyParam*>(lpParameter)->clientSocket);
	closesocket(static_cast<ProxyParam*>(lpParameter)->serverSocket);
	delete lpParameter;
	_endthreadex(0);
	return 0;
}

//************************************
// Method:	ParseHttpHead
// FullName:	ParseHttpHead
// Access:	public
// Returns:	void
// Qualifier: ���� TCP �����е� HTTP ͷ��
// Parameter: char * buffer
// Parameter: HttpHeader * httpHeader
//************************************
inline void MyProxy::ParseHttpHead(char *buffer, HttpHeader * httpHeader)
{
	char *ptr;
	const char * delim = "\r\n";
	char *p = strtok_s(buffer, delim, &ptr);//��ȡ��һ��
	printf("%s\n", p);
	if (p[0] == 'G')
	{//GET ��ʽ
		memcpy(httpHeader->method, "GET", 3);
		memcpy(httpHeader->url, &p[4], strlen(p) - 13);
	}
	else if (p[0] == 'P')
	{//POST ��ʽ
		memcpy(httpHeader->method, "POST", 4);
		memcpy(httpHeader->url, &p[5], strlen(p) - 14);
	}
	printf("%s\n", httpHeader->url);
	p = strtok_s(nullptr, delim, &ptr);
	while (p) {
		switch (p[0]) {
		case 'H'://Host
			memcpy(httpHeader->host, &p[6], strlen(p) - 6);
			break;
		case 'C'://Cookie
			if (strlen(p) > 8)
			{
				char header[8];
				ZeroMemory(header, sizeof(header));
				memcpy(header, p, 6);
				if (!strcmp(header, "Cookie"))
				{
					memcpy(httpHeader->cookie, &p[8], strlen(p) - 8);
				}
			}
			break;
		default:
			break;
		}
		p = strtok_s(nullptr, delim, &ptr);
	}
}

//************************************
// Method:	ConnectToServer
// FullName:	ConnectToServer
// Access:	public
// Returns:	BOOL
// Qualifier: ������������Ŀ��������׽��֣�������
// Parameter: SOCKET * serverSocket
// Parameter: char * host
//************************************
inline BOOL MyProxy::ConnectToServer(SOCKET *serverSocket, char *host) {
	sockaddr_in serverAddr;
	serverAddr.sin_family = PF_INET;
	serverAddr.sin_port = htons(HTTP_PORT);
	HOSTENT *hostent = gethostbyname(host);
	if (!hostent) {
		return FALSE;
	}
	in_addr Inaddr = *reinterpret_cast<in_addr*>(*hostent->h_addr_list);
	serverAddr.sin_addr.s_addr = inet_addr(inet_ntoa(Inaddr));
	*serverSocket = socket(PF_INET, SOCK_STREAM, 0);
	if (*serverSocket == INVALID_SOCKET)
	{
		return FALSE;
	}
	if (connect(*serverSocket, reinterpret_cast<SOCKADDR *>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR)
	{
		closesocket(*serverSocket);
		return FALSE;
	}
	return TRUE;
}
