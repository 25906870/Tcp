#include "TcpBase.h"

int TcpBase::Write(sock_ptr sk_ptr, uint8_t *buf, int length)
{
	boost::asio::async_write(*sk_ptr, boost::asio::buffer(buf, length),
		[this, sk_ptr, buf, length](const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if (ec == boost::asio::error::eof
			|| ec == boost::asio::error::bad_descriptor
			|| ec == boost::asio::error::connection_aborted
			|| ec == boost::asio::error::connection_reset)
		{
			cout << " disconnected: " << ec.message();

			if (m_handler != NULL)
				m_handler->onError(ec, sk_ptr, this);
		}
		else
		{

			if (m_handler != NULL)
				m_handler->onWriteDone(buf, length, sk_ptr, this);
			//deadline_.expires_from_now(boost::posix_time::seconds(DealineTime));
		}
	}
	);
	return 0;
}
/*
* 建议使用该函数 避免写数据的内存过早被释放 写日志内容被初始化
*/
int TcpBase::Write(sock_ptr sk_ptr, writebuf_ptr buf, int length)
{
	boost::asio::async_write(*sk_ptr, boost::asio::buffer(buf->data(), length),
		[this, sk_ptr, buf, length](const boost::system::error_code& ec, size_t bytes_transferred)
	{
		if (ec == boost::asio::error::eof
			|| ec == boost::asio::error::bad_descriptor
			|| ec == boost::asio::error::connection_aborted
			|| ec == boost::asio::error::connection_reset)
		{
			cout << " disconnected: " << ec.message();

			if (m_handler != NULL)
				m_handler->onError(ec, sk_ptr, this);
		}
		else
		{

			if (m_handler != NULL)
				m_handler->onWriteDone(buf, length, sk_ptr, this);
			//deadline_.expires_from_now(boost::posix_time::seconds(DealineTime));
		}
	}
	);
	return 0;
}

int TcpBase::Read(sock_ptr sk_ptr, buf_ptr read_buffer)
{
	//auto read_buffer = std::make_shared<boost::array<uint8_t,1055>>();
	sk_ptr->async_receive(boost::asio::buffer(*read_buffer),//boost::asio::buffer(*read_buffer),
		[this, sk_ptr, read_buffer](boost::system::error_code ec, std::size_t length) {
		if (ec/* == boost::asio::error::eof
			  || ec == boost::asio::error::bad_descriptor
			  || ec == boost::asio::error::connection_aborted
			  || ec == boost::asio::error::connection_reset*/)
		{
			std::cout << "error message : " << ec.message();
			if (m_handler != NULL)
				m_handler->onError(ec, sk_ptr, this);
			sk_ptr->close();
		}
		else
		{
			if (m_handler == NULL)
				cout << "please add handler" << endl;
			else
				m_handler->onMsgArrived(read_buffer, length, sk_ptr, this);
			//Read(m_socket,bytes_transferred)
		}
		if (sk_ptr->is_open())
			Read(sk_ptr, read_buffer);
	});
	return 0;
}
int TcpBase::SetHandler(TcpBaseHandler *ptr)
{
	m_handler = ptr;
	return 0;
}
int TcpBase::DestroyHandler()
{
	if (m_handler == NULL)
	{
		m_handler->Destory();
		m_handler = NULL;
	}
	return 0;
}



/********************************************************************/
class handler_allocator
	: private boost::noncopyable
{
public:
	handler_allocator()
		: in_use_(false)
	{
	}

	void* allocate(std::size_t size)
	{
		if (!in_use_ && size < storage_.size)
		{
			in_use_ = true;
			return storage_.address();
		}
		else
		{
			return ::operator new(size);
		}
	}

	void deallocate(void* pointer)
	{
		if (pointer == storage_.address())
		{
			in_use_ = false;
		}
		else
		{
			::operator delete(pointer);
		}
	}

private:
	// Storage space used for handler-based custom memory allocation.
	boost::aligned_storage<1024> storage_;

	// Whether the handler-based custom allocation storage has been used.
	bool in_use_;
};

// Wrapper class template for handler objects to allow handler memory
// allocation to be customised. Calls to operator() are forwarded to the
// encapsulated handler.
template <typename Handler>
class custom_alloc_handler
{
public:
	custom_alloc_handler(handler_allocator& a, Handler h)
		: allocator_(a),
		handler_(h)
	{
	}

	template <typename Arg1>
	void operator()(Arg1 arg1)
	{
		handler_(arg1);
	}

	template <typename Arg1, typename Arg2>
	void operator()(Arg1 arg1, Arg2 arg2)
	{
		handler_(arg1, arg2);
	}

	friend void* asio_handler_allocate(std::size_t size,
		custom_alloc_handler<Handler>* this_handler)
	{
		return this_handler->allocator_.allocate(size);
	}

	friend void asio_handler_deallocate(void* pointer, std::size_t /*size*/,
		custom_alloc_handler<Handler>* this_handler)
	{
		this_handler->allocator_.deallocate(pointer);
	}

private:
	handler_allocator& allocator_;
	Handler handler_;
};

// Helper function to wrap a handler object to add custom allocation.
template <typename Handler>
inline custom_alloc_handler<Handler> make_custom_alloc_handler(
	handler_allocator& a, Handler h)
{
	return custom_alloc_handler<Handler>(a, h);
}

class session
	: public boost::enable_shared_from_this<session>
	, public TcpBase
{
public:
	session(io_service &io_) :
		socket_(std::make_shared<tcp::socket>(io_))
	{
		data_ = std::make_shared<boost::array<uint8_t, MAXCACHESIZE>>();
	}

	sock_ptr &socket()
	{
		return socket_;
	}

	void start()
	{
		Read(socket_, data_);
	}



	void write(uint8_t *buf, int length)
	{
		Write(socket_, buf, length);
	}

private:
	// The socket used to communicate with the client.
	sock_ptr socket_;

	// Buffer used to store data received from the client.

	buf_ptr data_;
	// The allocator to use for handler-based custom memory allocation.
	//handler_allocator allocator_;
};

