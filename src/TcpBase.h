#pragma once
/*命名约定：
类的名称以首字母大写的方式命名，如GwBaseClient
结构体命名以小驼峰法，systemPara,playSchedule
枚举命名以首字母大写的方式命名，materialType,
函数命名以小驼峰法，如showImage()
变量名不要出现大写字母，单词之间用_隔开，如play_schedule
类的成员变量以m_开头，如m_id，m_name；结构体的成员变量不要以m_开头

全局变量和全局函数一般定义在Myinclude.h中。全局函数是和这个项目相关的函数
全局结构体定义加上gs_，如gs_systemPara                                                (g表示全局,s表示结构体)
全局变量以g_开头，如g_system_para                                                     (g表示全局)
全局函数以gf_开头，如string gf_getCurrentDirectory()                                  (g表示全局,f表示函数)

共享结构体：除了这个项目用到，其他项目也可能用到的结构体，定义在ShareStruct.h中
共享结构体的名称以shs_开头，如shs_pngPara                                              (sh表示共享,s表示结构体)
共享函数：除了这个项目用到，其他项目也可能用到的函数，定义在ShareFuc.h中
共享函数的名称以shf_开头，如shf_string2Byte                                            (sh表示共享,f表示函数)
*/

#ifndef TCPBASE_H_
#define TCPBASE_H_

#include <stdlib.h>
#include <stdint.h>
#include <memory>
#include <boost/asio.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/bind.hpp>
#include <boost/asio/io_service.hpp>
#include <boost/aligned_storage.hpp>
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#if defined __GNUC__
#include <unistd.h>
#define Sleep(x) usleep(x##000)
#elif defined _MSC_VER
#include <windows.h>
#endif

#define MAXCACHESIZE 1044

using boost::asio::io_service;
using boost::asio::ip::tcp;
using boost::asio::ip::address;

using namespace std;

typedef std::shared_ptr<tcp::socket> sock_ptr;
typedef std::shared_ptr<boost::array<uint8_t, MAXCACHESIZE>> buf_ptr;//std::shared_ptr<boost::array<uint8_t, 1055>>
typedef std::shared_ptr<vector<uint8_t>> writebuf_ptr;
typedef vector<string> vct_str;
typedef vector<uint8_t> unsign_str;
//typedef std::shared_ptr<boost::array<uint8_t, MAXBUFSIZE>> arry_prt;
enum HanderFun{
	OnConnect=0,
	OnAccepet,
	OnWriteDone,
	OnMsgArrived
};

class TcpBaseHandler
{
public:
	virtual ~TcpBaseHandler() {}
	virtual int onWriteDone(const uint8_t *frame, int framelength, sock_ptr ptr,void *ser) = 0;
	virtual int onWriteDone(writebuf_ptr frame, int framelength, sock_ptr ptr, void *ser) = 0;
	//sock 用来和对应客户端进行通信
	virtual int onMsgArrived(buf_ptr buf,int msglength,sock_ptr ptr, void *ser) = 0;
	virtual int onAccept(sock_ptr ptr,void *ser) = 0;
	virtual int onError(const boost::system::error_code& error, sock_ptr ptr, void *ser) = 0;
	virtual int onConnect(sock_ptr ptr, void *ser) = 0;

	//static Handler* _stdcall CreateHandle(){ return nullptr; };
	void Destory() { delete this; }
};



class TcpBase
{
public:
	int Write(sock_ptr sk_ptr, uint8_t *buf, int length);
	/*
	* 建议使用该函数 避免写数据的内存过早被释放 写日志内容被初始化
	*/
	int Write(sock_ptr sk_ptr, writebuf_ptr buf, int length);

	int Read(sock_ptr sk_ptr, buf_ptr read_buffer);
	int SetHandler(TcpBaseHandler *ptr);
	int DestroyHandler();

public:
	TcpBaseHandler *m_handler;
};


#endif