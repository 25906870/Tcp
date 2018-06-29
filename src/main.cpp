#include"Tcp.h"
#include "Dispose.h"

/*
* 根据帧头桢尾来读取的服务示例
*/
BOOL CALLBACK cosonleHandler(unsigned long ev)
{
	BOOL bRet = FALSE;
	switch (ev)
	{
	case CTRL_CLOSE_EVENT:
		bRet = TRUE;
		std::cout << "clear call back" << endl;
		//MessageBox(NULL, _T("QUIT?"), _T("Console"), MB_YESNO);
		/*g_sys_para.mt.lock();
		g_sys_para.quit = true;
		g_sys_para.mt.unlock();*/
		/*while (gf_isAllThreadStoped() == false)
		{
			Sleep(500);
			continue;
		}*/
		//MessageBox(NULL, _T("QUIT?"), _T("Console"), MB_YESNO);
		/*g_sys_para.mt.lock();
		gf_clearGlobalData();
		g_sys_para.mt.unlock();
		system("pause");
		break;*/
	default:
		break;
	}
	return bRet;
}



class read_HeadEnd :public TcpBaseHandler
{
public:
	std::map<sock_ptr, DisposeRecivedMsg> recvframe;
	vector<std::vector<uint8_t>> re_vec;
public:
	virtual int onWriteDone(const uint8_t *frame, int framelength, sock_ptr ptr, void *ser)
	{
		string temp = cast::array_to_hex_string<uint8_t>(frame, framelength);
		cout << temp << endl;
		return 0;
	}
	int onWriteDone(writebuf_ptr frame, int framelength, sock_ptr ptr, void *ser) 
	{
		return 0;
	}
	//sock 用来和对应客户端进行通信
	virtual int onMsgArrived(buf_ptr buf, int readlength, sock_ptr ptr, void *ser)
	{
		//std::istream istr(buf.get());

		int length = readlength;
		vector<uint8_t> total_rev;
		for (size_t i = 0; i < length; i++)
		{
			total_rev.push_back((*buf)[i]);
		}
		if (recvframe[ptr].ReadFrame(total_rev.data(), length, re_vec))
		{
			for (int i = 0; i < re_vec.size(); i++)
			{
				cout << cast::array_to_hex_string(re_vec[i].data(), re_vec[i].size()) << endl;
			}
			//存 map mac-socket
			//解析
			//ser->write();
		}
		else
		{
			cout << "wait data" << endl;
		}
		((Tcp*)ser)->Write(ptr, total_rev.data(),length);
		return 0;
	}
	virtual int onAccept(sock_ptr ptr, void *ser)
	{
		unsign_str head, end;
		head.push_back(0xaa);
		end.push_back(0x55);
		recvframe[ptr] = DisposeRecivedMsg(head, end, READ_ACCORDINGLENGTH);
		return 0;
	}
	virtual int onError(const boost::system::error_code& error, sock_ptr ptr, void *ser)
	{
		ptr->close();
		return 0;
	}
	virtual int onConnect(sock_ptr ptr, void *ser)
	{
		return 0;
	}
};
// tcp_hpp server test
int main() //服务端
{
	//以下函数为win下触发了关闭按钮调用的清理回掉函数
#ifdef _MSC_VER
	SetConsoleCtrlHandler(cosonleHandler, TRUE);
#else
#endif

	
	try
	{
		io_service io_service_;
		read_HeadEnd server;
		//Tcp clt("127.0.0.1", 8080, io_service_);
		Tcp ser(8080, io_service_);
		ser.SetHandler(&server);
		//clt.startConnect();
		ser.startServer();
		io_service_.run();
	}
	catch (const std::exception&e)
	{
		cout << "liscence port is busy " << e.what() << endl;
	}
	//std::cin.g;
	return 1;
}

int main1() //客户端
{
	io_service io_service_;
	read_HeadEnd clt;
	//Tcp clt("127.0.0.1", 8080, io_service_);
	Tcp tcpclt("127.0.0.1",1235, io_service_);
	tcpclt.SetHandler(&clt);
	tcpclt.Connect("0011223344556677","127.0.0.1",8080);
	tcpclt.Connect("0011223344556677", "127.0.0.1", 8081);
	tcpclt.GetSock("0011223344556677");
	io_service_.run();
	return 0;
}
