#pragma once
/*����Լ����
�������������ĸ��д�ķ�ʽ��������GwBaseClient
�ṹ��������С�շ巨��systemPara,playSchedule
ö������������ĸ��д�ķ�ʽ������materialType,
����������С�շ巨����showImage()
��������Ҫ���ִ�д��ĸ������֮����_��������play_schedule
��ĳ�Ա������m_��ͷ����m_id��m_name���ṹ��ĳ�Ա������Ҫ��m_��ͷ

ȫ�ֱ�����ȫ�ֺ���һ�㶨����Myinclude.h�С�ȫ�ֺ����Ǻ������Ŀ��صĺ���
ȫ�ֽṹ�嶨�����gs_����gs_systemPara                                                (g��ʾȫ��,s��ʾ�ṹ��)
ȫ�ֱ�����g_��ͷ����g_system_para                                                     (g��ʾȫ��)
ȫ�ֺ�����gf_��ͷ����string gf_getCurrentDirectory()                                  (g��ʾȫ��,f��ʾ����)

����ṹ�壺���������Ŀ�õ���������ĿҲ�����õ��Ľṹ�壬������ShareStruct.h��
����ṹ���������shs_��ͷ����shs_pngPara                                              (sh��ʾ����,s��ʾ�ṹ��)
�����������������Ŀ�õ���������ĿҲ�����õ��ĺ�����������ShareFuc.h��
��������������shf_��ͷ����shf_string2Byte                                            (sh��ʾ����,f��ʾ����)
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
	//sock �����Ͷ�Ӧ�ͻ��˽���ͨ��
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
	* ����ʹ�øú��� ����д���ݵ��ڴ���类�ͷ� д��־���ݱ���ʼ��
	*/
	int Write(sock_ptr sk_ptr, writebuf_ptr buf, int length);

	int Read(sock_ptr sk_ptr, buf_ptr read_buffer);
	int SetHandler(TcpBaseHandler *ptr);
	int DestroyHandler();

public:
	TcpBaseHandler *m_handler;
};


#endif