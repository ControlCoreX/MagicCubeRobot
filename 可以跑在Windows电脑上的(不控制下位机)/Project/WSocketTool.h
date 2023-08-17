#pragma once

#include <string>
// WinSocket
#include <winsock.h>

#pragma comment(lib,"WS2_32")
using namespace std;
class WSocketTool
{
public:
	WSocketTool();
	void sendGetData(string url);
	string receiptData();

private:
	// 套接字
	SOCKET sock;
	// 地址信息
	SOCKADDR_IN addr;
	string sd;
	char rd[1000];
	// 初始化
	void loadSocket();
	void createSocket();
	void createAddr();
	void socketConnectAddr();
	// 发送数据
	void sendData();
	// 查找字符串
	string find(string str, string s1, string s2);

};

