#pragma once
#ifndef  TCP_HPP_
#define  TCP_HPP_

#include"TcpBase.h"

enum TcpStartStatus
{
	StartServer = 1,
	StartClient,
	UnknowStartStatus
};
/*
*可以作为服务的或者客户启动
*根据不同的参数来区分
*/
class Tcp :public TcpBase
{
public:
	/*
	* 服务端的启动只需要port
	*/
	Tcp(int port, io_service &io):
		io_(io), acceptor(io_, tcp::endpoint(boost::asio::ip::tcp::v4(), port))
		, checkConnectTime(io)
		, flag_server(TcpStartStatus::UnknowStartStatus)
	{
		CheckSockIsopen();
	}


	/*
	*初始化client
	*/
	Tcp(string hostname,int port, io_service &io) :
		io_(io), acceptor(io_,tcp::endpoint(boost::asio::ip::tcp::v4(), port + 1))
		, checkConnectTime(io)
		, flag_server(TcpStartStatus::UnknowStartStatus)
	{
		tcp::resolver  resolver(io_);
		boost::system::error_code ec;
		tcp::resolver::query query(hostname, boost::lexical_cast<std::string, unsigned short>(port));
		tcp::resolver::iterator iter = resolver.resolve(query, ec);
		tcp::resolver::iterator end;

		if (iter != end && ec == 0)
		{
			clt_endpoint = *iter;
		}
		CheckSockIsopen();
	}

	/*
	*param @strs 需要删除的sock组合，为空全部删除
	*/

	~Tcp()
	{
		Close();
		clt_sock.clear();
		acceptor.close();

	}
	int Close(const vct_str *strs = NULL)
	{
		if (strs == NULL)
		{
			for (auto tp : clt_sock)
			{
				tp.second->close();
			}
			clt_sock.clear();
		}
		else
		{
			for (auto str:*strs)
			{
				clt_sock[str]->close();
				clt_sock.erase(str);
			}
		}
		return 0;
	};

	/*
	*param @sockptr
	*/
	int Close(sock_ptr sockptr)
	{
		sockptr->close();
		//sockptr->cancel();
	}
public:


	/*
	*作为服务启动 ，启动之后不可更改
	*作为服务器启动下端连上之后的sock 由使用者维护，建议采用std::map<string,sock_ptr> 
	*形式来存储key应为mac地址（或其他可以保证唯一性，不变的字符串）
	*/
	void startServer()
	{
		if (TcpStartStatus::StartClient == flag_server)
		{
			cout << "TcpStartStatus::StartClient is error" << endl;
			return;
		}
		else if (flag_server == TcpStartStatus::UnknowStartStatus)
			flag_server = TcpStartStatus::StartServer;
		sock_ptr sock(new tcp::socket(io_));
		 //智能指针
		//auto session_ptr = std::make_shared<session>(io_);
		//vec_ss.push_back(session_ptr);
		//session_ptr->SetHandler(handler);
		//auto sock = session_ptr->socket();
		acceptor.async_accept(*sock, [this, sock](const boost::system::error_code& ec) {
		
			if (ec)
			{
				if (m_handler != NULL)
					m_handler->onError(ec, sock, (void*)this);
				cout << "accept error." << boost::system::system_error(ec).what() << endl;
			}


			if (m_handler != NULL)
				m_handler->onAccept(sock,(void*)this);
			//clt_sock[sock->remote_endpoint().address().to_string()] = sock;
			auto read_buffer=std::make_shared<boost::array<uint8_t, MAXCACHESIZE>>();
			Read(sock,read_buffer);
			if (acceptor.is_open())
			{
				startServer();
			}
		});
	}
	
	/*
	*作为客户端启动，启动之后不可更改
	*如果多个clt启动该类内用std::map<string,sock_ptr> 使用时用mac地址取出对应的sock既可以进行通信
	*param : mac 控制器地址用来映射sock
	*		 host,port 对应的服务地址
	*/
	void Connect(const string &mac,const string &host,int port=3434)
	{
		if (TcpStartStatus::StartServer == flag_server)
		{
			cout << "TcpStartStatus::StartServer is error" << endl;
			return;
		}
		else if (flag_server == TcpStartStatus::UnknowStartStatus)
		{
			flag_server = TcpStartStatus::StartClient;
			acceptor.close();
		}
		auto sock = std::make_shared<tcp::socket>(io_);
		clt_sock[mac] = sock;
		//auto edpoint = clt_endpoint;
		if(port != 0)
		{
			tcp::resolver  resolver(io_);
			boost::system::error_code ec;
			tcp::resolver::query query(host, boost::lexical_cast<std::string, unsigned short>(port));
			tcp::resolver::iterator iter = resolver.resolve(query, ec);
			tcp::resolver::iterator end;

			if (iter != end && ec == 0)
			{
				clt_endpoint = *iter;
			}
		}
		sock->async_connect(clt_endpoint,
			[this, sock](const boost::system::error_code & error) {
			if (error)
			{
				if (m_handler != NULL)
				{
					m_handler->onError(error, sock, (void*)this);
				}
			}
			if (m_handler != NULL)
			{
				m_handler->onConnect(sock, (void*)this);
			}
			auto read_buffer = std::make_shared<boost::array<uint8_t, MAXCACHESIZE>>();
			Read(sock, read_buffer);
		});
	}

	/*
	* 作为客户端启动时的sock
	*/
	sock_ptr GetSock(const string &mac)
	{
		return clt_sock[mac];
	}

private:
	void CheckSockIsopen()
	{
		checkConnectTime.expires_from_now(boost::posix_time::seconds(3));
		checkConnectTime.async_wait([&](const boost::system::error_code &e) {
			for (auto tp = clt_sock.begin();
				tp != clt_sock.end();)
			{
				auto tp1 = tp;
				tp1++;
				if (!tp->second->is_open())
				{
					clt_sock.erase(tp);
				}
				tp = tp1;
			}
			CheckSockIsopen();
		});
	}
private:
	io_service  &io_;
	tcp::endpoint clt_endpoint;
	tcp::acceptor acceptor;
	boost::asio::deadline_timer checkConnectTime;
	/*
	* 服务的不用这个clt_sock 
	* 服务端 的sock由使用者保存，建议采用类似形式key应为唯一标志字符串
	*/
	std::map<string,sock_ptr> clt_sock;
	int flag_server;
};

#endif // ! TCP_HPP_

