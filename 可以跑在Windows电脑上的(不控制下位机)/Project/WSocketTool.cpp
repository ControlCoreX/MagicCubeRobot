#include "WSocketTool.h"

// 构造函数
WSocketTool::WSocketTool() {
	loadSocket();
	createSocket();
	createAddr();
	socketConnectAddr();
}

// 1.加载Socket库
void WSocketTool::loadSocket() {
	WSADATA ver;
	WSAStartup(MAKEWORD(1, 1), &ver);
}

// 2.建立流式套接字
void WSocketTool::createSocket() {
	//套接字
	sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (sock <= 0)
		exit(-1);
}

// 3.设置
void WSocketTool::createAddr() {
	//初始化addr
	memset(&addr, 0, sizeof(SOCKADDR_IN));
	//设置属性
	addr.sin_family = AF_INET;
	//端口
	addr.sin_port = htons(8088);
	//IP
	addr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");
}

// 4.连接主机地址
void WSocketTool::socketConnectAddr() {
	int con = connect(sock, (SOCKADDR*)&addr, sizeof(SOCKADDR_IN));
}

// 发送数据
void WSocketTool::sendData() {
	// TCP发送数据
	int count = 0;
	// 类型转换
	char* h = const_cast<char*>(sd.c_str());
	int len = sd.size();
	while (count < len) {
		count = send(sock, h, len, 0);
		h += count;
		len -= count;
	}
}

// GET请求
void WSocketTool::sendGetData(string url) {
	sd = "GET " + url + " HTTP/1.1\r\nHost: 127.0.0.1:8088\r\nConnection: keep-alive\r\n\r\n";
	sendData();
}

// 接收数据
string WSocketTool::receiptData() {
	memset(rd, 0, 1000);
	recv(sock, rd, 1000, 0);
	// 解析数据
	return find(rd, "<BODY>\r\n", "\r\n</BODY>");
}

// 查找str字符串以s1为开头s2为结尾的内容
string WSocketTool::find(string str, string s1, string s2) {
	int pos = 0;
	int start = str.find(s1, pos);
	if (start == -1)
	{
		return "";
	}
	start += s1.size();
	int end = str.find(s2, start);
	if (end == -1)
	{
		return "";
	}
	pos = end + s2.size();
	return str.substr(start, end - start);
}
