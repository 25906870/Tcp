#pragma once
#ifndef DISPOSE_HPP_
#define DISPOSE_HPP_

#include"toolkit/string/string.h"
#include"toolkit/cast/cast.h"
#include "TcpBase.h"
#include<list>
namespace cast = qingpei::toolkit::cast;
#define READ_HEADEND 0
#define READ_ACCORDINGLENGTH 1

/*功能：求模式串值
*参数：ptn：模式串
*nextval：保存模式串值的数组
*/
void get_nextval(const unsign_str &ptn, int *nextval);

/*功能：实现KMP算法
*参数：src：源串
*      patn：模式串
*      nextval：模式串值
*      pos：源串开始的位置
*返回值：若匹配成功，则返回下标；若出错或匹配不成功，则返回-1
*/
int kmp_search(const uint8_t *src, int srclength, const unsign_str &patn, int pos = 0);
class DisposeRecivedMsg {
public:
	/*
	*@param: Head_,End_
	*		 type 选择初始化类型选择之后不可改变
	*			  READ_HEADEND 根据头尾来进行读取
	*			  READ_ACCORDINGLENGTH 根据长度来读取
	* 
	
	LengthSite_ 帧头到长度 DesLength_长度字节 supplementlength
	*/
	DisposeRecivedMsg(const unsign_str &Head_, const unsign_str &End_, int type,
		int LengthSite_ = 9, int DesLength_ = 4, int supplementlength = 3);
	DisposeRecivedMsg();
	enum RETURN
	{
		WAIT_DATA=0,
		ANALYZE_SUCCESS
	};

	/*传入数据解析出完整的数据帧
	*param: data 传入收到的数据
	*		Readlength 传入数据的长度
	*	    frames 按要求解析出来的数据帧 只进行了数据格式上的拼接（如有校验需要使用者自行进行）
	*return 0继续读取 1完整的数据
	*/
	int ReadFrame(uint8_t *data, int Readlength, std::vector<std::vector<uint8_t>>& frames);
private:
	/*
	根据长度读取数据
	1.当tcp收到消息时会将信息传入dispose 在传入的数据中寻找 Head标志位 将缓冲区内head标志位前的数据丢掉
	2.结束标准读到需要长度时 判断尾部是否符合标准 若不符合 在缓冲区中后移一位继续读取或者直接判断 等到读到符合要求的数据
	3.读到完整的一帧 就返回1 否则为0继续读取
	*/
	int ReadFrameAccordinglength(uint8_t *data, int Readlength, std::vector<std::vector<uint8_t>>& frames);
	/*
	根据头尾读取数据
	1.当tcp收到消息时会将信息传入dispose 在传入的数据中寻找 Head标志位 将缓冲区内head标志位前的数据丢掉
	2.直至读到 end为止
	*/
	int ReadFrameHeadEnd(uint8_t *data, int Readlength, std::vector<std::vector<uint8_t>>& frames);


private:
	/*
	读取数据时候 查找Head并将不匹配的数据去除
	*/
	int FindHead(int *preendpos = NULL);

	int FindEnd();

	/*
	*return : -1 找不到头
			  0  表示已经有了一帧的开头无需再找
	*/
	int FindHeadAccordinglength();
	/*
	当数据读取完毕时进行m_frame 的检查
	*/
	int CheckEnd(uint8_t End_);

	/*
	根据长度读取数据
	*/
	int PushData(uint8_t *data, int Readlength);
	/*
	计算数据的长度
	return: -1数据内容长度不够
	*/
	int	FrameLength();

	/*
	获取到完整的一帧
	*/
	int GetFrame(int endpos = -1);
private:
	/*
	缓冲区用来存储读来的数据头为Head
	*/
	std::list<uint8_t> m_FrameBuff;
	std::vector<uint8_t> m_vecFrameBuff; //vec缓存区
	bool m_tag_head;
	//buf_ptr framebuf;
public:
	/*
	当数据读取完毕时 放到这当中 传给用户
	*/
	std::vector<uint8_t> m_frame;

private:
	int m_LengthSite;
	uint8_t Head;
	uint8_t End;

	unsign_str m_Headstr;
	unsign_str m_Endstr;

	//  aa0011223344556677  00000005  001122334455     crc     55 
	//  head m_LengthSite		  m_DesLength			     m_supplement  end
	int m_DesAndSite;
	int m_DesLength;//描述长度 对根据长短来读的数据进行描述长度的字段占几个字节的设置
	int m_supplement;//对给定长度也无法完全描述的给予补充长度
	int m_frame_type;
};



#endif // !DISPOSE_HPP_

