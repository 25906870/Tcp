#include "stdafx.h"
#include "Tcp.h"
namespace sansi{
	namespace protocol {

		sansi::protocol::TcpClient::TcpClient(const std::string & hostname,
			unsigned short port,
			boost::asio::io_service & io) :
			io_s(io),
			deadline(io_s),
			connecttime(io_s),
			hostname_(hostname),
			Csocket(io_s)
		{
			determine_connect = 1;
			flag_deadline = 0;
			tcp::resolver  resolver(io_s);
			boost::system::error_code ec;
			port_ = port;
			tcp::resolver::query query(hostname, boost::lexical_cast<std::string, unsigned short>(port));
			tcp::resolver::iterator iter = resolver.resolve(query, ec);
			tcp::resolver::iterator end;
			// pick the first endpoint
			if (iter != end && ec == 0)
			{
				endpoint = *iter;         //此定义一个类内成员endpoint 用来存储在使用者定义的list中 用来判断是否给谁发送帧
				
				memset(m_data, 0, sizeof(m_data));
				connecttime.expires_from_now(boost::posix_time::seconds(CONNECTTIMEOUT));
				connecttime.async_wait(boost::bind(&TcpClient::CheckConnectTime, this));
				Connect();
				/* Csocket->async_connect(endpoint, boost::bind(&TcpClient::handle_connect,
					this, boost::asio::placeholders::error));
				m_socket->connect(endpoint,ec);*/

			}
		}

		int sansi::protocol::TcpClient::write(uint8_t * tm, int length)
		{
			/*boost::asio::async_write(*Csocket,boost::asio::buffer(tm, length),
				[this, tm, length](boost::system::error_code ec, std::size_t wlength)*/
			Csocket.async_write_some(boost::asio::buffer(tm, length),
				[this, tm, length](boost::system::error_code ec, std::size_t wlength) 
			{
				std::cout << " ec " << ec << std::endl;
				if (ec == boost::asio::error::eof
					|| ec == boost::asio::error::bad_descriptor
					|| ec == boost::asio::error::connection_aborted
					|| ec == boost::asio::error::connection_reset)
				{
					cout << " disconnected: " << ec.message();

					if (!handlerlist.empty())
						handlerlist.front()->onError(ec, this);
				}
				else
				{
					if (!handlerlist.empty())
					{
						if (!handlerlist.empty())
							handlerlist.front()->onWriteDone(tm, length, this);
						//CheckTime();
					}

					//deadline_.expires_from_now(boost::posix_time::seconds(DealineTime));

				}
				SetDealineTime(DealineTime);
			});
			return 0;
		}

		int sansi::protocol::TcpClient::Read(size_t bytes_transferred)
		{
			//deadline.expires_from_now(boost::posix_time::seconds(DealineTime));
			//boost::asio::async_read(*Csocket, boost::asio::buffer(m_data),
			//	[this, bytes_transferred](boost::system::error_code ec, std::size_t length)
			Csocket.async_receive(boost::asio::buffer(m_data),
				[this, bytes_transferred](boost::system::error_code ec, std::size_t length) 
			{
				std::cout << "async receive length " << length << " error " << ec <<std::endl;
				if (ec == boost::asio::error::eof
					|| ec == boost::asio::error::bad_descriptor
					|| ec == boost::asio::error::connection_aborted
					|| ec == boost::asio::error::connection_reset)
				{
					std::cout << "Mix Read disconnected: " << ec.message();
					if (!handlerlist.empty())
						handlerlist.front()->onError(ec, this);
				}
				else
				{
					//deadline.expires_from_now(boost::posix_time::seconds(DealineTime));
					//AlreadyRead(m_data,length)  这里是将缓冲区的内容传给handler 由handler 进行处理 数据内容
					if (handlerlist.empty())
						cout << "please add handler" << endl;
					else
						handlerlist.front()->onMsgArrived(m_data, length, this);

				}
				SetDealineTime(DealineTime);

			});
			return 0;
		}

		int TcpClient::sync_write(uint8_t * frame, int length)
		{
			Csocket.write_some(boost::asio::buffer(frame,length));
			return 0;
		}

		int TcpClient::sync_read(vector<uint8_t> &frame)
		{
			int bytes_transferred = Csocket.read_some(boost::asio::buffer(m_data));
			for (size_t i = 0; i < bytes_transferred; i++)
			{
				frame.push_back(m_data[i]);
			}

			return 0;
		}

		int sansi::protocol::TcpClient::RemoveHander(DisposeHandler * p)
		{
			if (p == nullptr)
				handlerlist.pop_front();
			else
			{
				handlerlist.remove(p);
			}
			return 1;

		}

		int TcpClient::Connect()
		{
			boost::asio::ip::tcp::resolver::query q(
					boost::asio::ip::tcp::v4(),
					hostname_,
					std::to_string(port_));
			boost::asio::ip::tcp::resolver resolver(io_s);/**/
			auto endpoint_iterator = resolver.resolve(q);
			Csocket.async_connect(endpoint, boost::bind(&TcpClient::handle_connect,
				this, boost::asio::placeholders::error));
			/*boost::asio::async_connect(Csocket, endpoint_iterator,
				[this](boost::system::error_code ec,tcp::resolver::iterator)
			{
				handle_connect(ec);
			});*/
			return 0;
		}

		void TcpClient::run()
		{
			io_s.reset();
			io_s.run();
		}

		void sansi::protocol::TcpClient::handle_connect(const boost::system::error_code & error)
		{
			if (!error)
			{
				std::cout << "Connecting to: " << endpoint << std::endl;
				determine_connect = 0;
				connecttime.expires_at(boost::posix_time::pos_infin);
				return;
			}
			determine_connect = 1;
			if (error.value() != boost::system::errc::operation_canceled)
			{
				std::cerr << "error msg" << boost::system::system_error(error).what() << std::endl;
			}
			
			//Read();
		}


		int sansi::protocol::TcpClient::SetDealineTime(int Sec)//设置超时时间
		{
			boost::system::error_code error; // 这个ec 是否需要看情况而定
			if (Sec > 0)
			{
				DealineTime = Sec;
				deadline.expires_from_now(boost::posix_time::seconds(DealineTime));
				deadline.async_wait(boost::bind(&sansi::protocol::TcpClient::check_deadline, this, error));
				return 0;
			}
			else
			{
				DealineTime = Sec;
				deadline.expires_at(boost::asio::deadline_timer::traits_type::now());
				deadline.cancel();
			}
			return 1;
		}

		void sansi::protocol::TcpClient::check_deadline(const boost::system::error_code error)
		{
			if (!error)
			{
				if (deadline.expires_at() <= boost::asio::deadline_timer::traits_type::now())
				{// 真正的等待超时
					cout << "*********************time out! **********************" << endl;//onerror
					boost::system::error_code time_out = boost::asio::error::timed_out;
					//deadline.async_wait(boost::bind(&sansi::protocol::TcpClient::check_deadline, this, error));
					//deadline.expires_at(boost::posix_time::pos_infin);// 定时器 设定为永不超时/不可用状态
					deadline.cancel();
					if (!handlerlist.empty())
						handlerlist.front()->onError(time_out, this);

					return;
				}
				// 如果不是真正的超时，定是其操作已成功/用户重新设置了定时器
				// 重新启动定时器
				deadline.async_wait(boost::bind(&sansi::protocol::TcpClient::check_deadline, this, error));
			}
		}

		int TcpClient::SetHostname(const std::string & hostname, unsigned short port)
		{
			tcp::resolver  resolver(io_s);
			boost::system::error_code ec;
			port_ = port;
			tcp::resolver::query query(hostname, boost::lexical_cast<std::string, unsigned short>(port));
			tcp::resolver::iterator iter = resolver.resolve(query, ec);
			tcp::resolver::iterator end;
			hostname_ = hostname;
			// pick the first endpoint
			if (iter != end && ec == 0)
			{
				endpoint = *iter;
				return 1;
			}
			return 0;
		}

		void sansi::protocol::TcpClient::CheckConnectTime()
		{
			if (connecttime.expires_at() <= boost::asio::deadline_timer::traits_type::now())
			{// 真正的等待超时
				cout << "connection time out " << endl;//onerror
				connecttime.expires_at(boost::posix_time::pos_infin);// 定时器 设定为永不超时/不可用状态
				boost::system::error_code error = boost::asio::error::timed_out;
				if (!handlerlist.empty())
					handlerlist.front()->onError(error, this);

				return;
			}
		}

		/****************************Tcp  service ***************************************/
		TcpAccept::TcpAccept(const std::string & hostname, unsigned short port,
			boost::asio::io_service &io) :
			ser_io(io),
			deadline_(io),
			acceptor(ser_io, tcp::endpoint(address::from_string(hostname), port)),
			DealineTime(1), accept_temp(1)
		{
			start();
		}


		TcpAccept::TcpAccept(unsigned short port, boost::asio::io_service & io):
			ser_io(io),deadline_(io),
			acceptor(ser_io, tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
			DealineTime(1), accept_temp(1)
		{
			start();
		}

		void TcpAccept::start()
		{
			sock_pt sock(new tcp::socket(ser_io)); //智能指针
			acceptor.async_accept(*sock, bind(&TcpAccept::accept_handler, this,
				boost::asio::placeholders::error, sock)); //异步侦听服务 // 多一个this叫做成员指针
														  //            boost::system::error_code error;
														  //            acceptor.listen(10000,error);
		}

		
		void TcpAccept::Close(sock_pt delete_socket)
		{
			auto my = sock_list.begin();
            int i=0;
			if (delete_socket != nullptr)
			{
                for (;i<sock_list.size(); ++i)
                {
                    if ((*my)== delete_socket)
                    {
                        sock_list.erase(my) ;
                        delete_socket->close();

                        cout<<"client close success"<<endl;
                    }
                    ++my;
                }
			}
			else if (delete_socket == nullptr)
			{
				for (; my != sock_list.end(); ++my)
				{
					if (!(*my)->is_open())
						(*my)->close();
				}
				sock_list.clear();
				acceptor.close();
				RemoveHander();
				accept_temp = 0;//表示正常关闭
			}

		}

		void TcpAccept::run()
		{
			ser_io.reset();
			ser_io.run();
		}
	

		void TcpAccept::accept_handler(const boost::system::error_code& ec, sock_pt sock)
		{
			if (ec)
			{
				if (accept_temp == 1)
					cout << "accept_handler error." << boost::system::system_error(ec).what() << endl;
				else
					cout << "acceptor close" << endl;
				return;
			}
			else if (ec&&accept_temp == 0)
			{

			}//vsockt[vsockt.size];
			sock_list.push_back(sock);
			temp_socket = sock.get();

			boost::system::error_code error;
			
			cout << "client address:" << sock->remote_endpoint().address().to_string() << endl;
			cout << "port" << sock->remote_endpoint().port() << endl;
			if (0 != handlerlist.size())
				handlerlist.front()->onAccept(this, sock);
            Read(sock,0);
			if (acceptor.is_open())
			{
				accept_temp = 1;
				start();
			}
		}

	
		int TcpAccept::RemoveHander(SerDisposeHandler* p)
		{
			if (p == nullptr)
				if (!handlerlist.empty())
					handlerlist.pop_front();
				else
				{
					handlerlist.remove(p);
				}
			return 1;
		}

		bool TcpAccept::CheckTime()
		{
			if (DealineTime > 1)
			{
				deadline_.expires_from_now(boost::posix_time::seconds(DealineTime));
				return true;
			}
			else
			{
				return false;
			}
		}

		void TcpAccept::readclear()
		{
			read_frame_.data_buf_.clear();
			memset(m_data, '\0', MAXBUFSIZE);
		}

		int  TcpAccept::isAccept()
		{
			if (acceptor.is_open())
				return 0;
			else
				return 1;

		}

		int TcpAccept::write(sock_pt m_socket,uint8_t * tm, int length)
		{
			m_socket->async_write_some(boost::asio::buffer(tm, length),
				[this, m_socket,tm, length](boost::system::error_code ec, std::size_t wlength) {
				if (ec == boost::asio::error::eof
					|| ec == boost::asio::error::bad_descriptor
					|| ec == boost::asio::error::connection_aborted
					|| ec == boost::asio::error::connection_reset)
				{
					cout << " disconnected: " << ec.message();

					if (!handlerlist.empty())
						handlerlist.front()->onError(ec, m_socket,this);
				}
				else
				{
					if (!handlerlist.empty())
					{
						if (!handlerlist.empty())
							handlerlist.front()->onWriteDone(tm, length, m_socket, this);
						//CheckTime();
					}

					deadline_.expires_from_now(boost::posix_time::seconds(DealineTime));

				}
			});
			return 0;
		}

		int TcpAccept::Read(sock_pt m_socket, size_t bytes_transferred)
		{
			deadline_.expires_from_now(boost::posix_time::seconds(DealineTime));
			m_socket->async_receive(boost::asio::buffer(m_data),
				[this, m_socket, bytes_transferred](boost::system::error_code ec, std::size_t length) {
				std::cout << "async receive length" << length << std::endl;
				if (ec == boost::asio::error::eof
					|| ec == boost::asio::error::bad_descriptor
					|| ec == boost::asio::error::connection_aborted
					|| ec == boost::asio::error::connection_reset)
				{
					std::cout << "Mix Read disconnected: " << ec.message();
					if (!handlerlist.empty())
						handlerlist.front()->onError(ec, m_socket ,this);
				}
				else
				{
					deadline_.expires_from_now(boost::posix_time::seconds(DealineTime));
					//AlreadyRead(m_data,length)  这里是将缓冲区的内容传给handler 由handler 进行处理 数据内容
					if (handlerlist.empty())
						cout << "please add handler" << endl;
					else
						handlerlist.front()->onMsgArrived(m_data, length, m_socket,this);
					//Read(m_socket,bytes_transferred)
				}
				
			});
			return 0;
		}

	}

}


