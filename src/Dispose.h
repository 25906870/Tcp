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

/*���ܣ���ģʽ��ֵ
*������ptn��ģʽ��
*nextval������ģʽ��ֵ������
*/
void get_nextval(const unsign_str &ptn, int *nextval);

/*���ܣ�ʵ��KMP�㷨
*������src��Դ��
*      patn��ģʽ��
*      nextval��ģʽ��ֵ
*      pos��Դ����ʼ��λ��
*����ֵ����ƥ��ɹ����򷵻��±ꣻ�������ƥ�䲻�ɹ����򷵻�-1
*/
int kmp_search(const uint8_t *src, int srclength, const unsign_str &patn, int pos = 0);
class DisposeRecivedMsg {
public:
	/*
	*@param: Head_,End_
	*		 type ѡ���ʼ������ѡ��֮�󲻿ɸı�
	*			  READ_HEADEND ����ͷβ�����ж�ȡ
	*			  READ_ACCORDINGLENGTH ���ݳ�������ȡ
	* 
	
	LengthSite_ ֡ͷ������ DesLength_�����ֽ� supplementlength
	*/
	DisposeRecivedMsg(const unsign_str &Head_, const unsign_str &End_, int type,
		int LengthSite_ = 9, int DesLength_ = 4, int supplementlength = 3);
	DisposeRecivedMsg();
	enum RETURN
	{
		WAIT_DATA=0,
		ANALYZE_SUCCESS
	};

	/*�������ݽ���������������֡
	*param: data �����յ�������
	*		Readlength �������ݵĳ���
	*	    frames ��Ҫ���������������֡ ֻ���������ݸ�ʽ�ϵ�ƴ�ӣ�����У����Ҫʹ�������н��У�
	*return 0������ȡ 1����������
	*/
	int ReadFrame(uint8_t *data, int Readlength, std::vector<std::vector<uint8_t>>& frames);
private:
	/*
	���ݳ��ȶ�ȡ����
	1.��tcp�յ���Ϣʱ�Ὣ��Ϣ����dispose �ڴ����������Ѱ�� Head��־λ ����������head��־λǰ�����ݶ���
	2.������׼������Ҫ����ʱ �ж�β���Ƿ���ϱ�׼ �������� �ڻ������к���һλ������ȡ����ֱ���ж� �ȵ���������Ҫ�������
	3.����������һ֡ �ͷ���1 ����Ϊ0������ȡ
	*/
	int ReadFrameAccordinglength(uint8_t *data, int Readlength, std::vector<std::vector<uint8_t>>& frames);
	/*
	����ͷβ��ȡ����
	1.��tcp�յ���Ϣʱ�Ὣ��Ϣ����dispose �ڴ����������Ѱ�� Head��־λ ����������head��־λǰ�����ݶ���
	2.ֱ������ endΪֹ
	*/
	int ReadFrameHeadEnd(uint8_t *data, int Readlength, std::vector<std::vector<uint8_t>>& frames);


private:
	/*
	��ȡ����ʱ�� ����Head������ƥ�������ȥ��
	*/
	int FindHead(int *preendpos = NULL);

	int FindEnd();

	/*
	*return : -1 �Ҳ���ͷ
			  0  ��ʾ�Ѿ�����һ֡�Ŀ�ͷ��������
	*/
	int FindHeadAccordinglength();
	/*
	�����ݶ�ȡ���ʱ����m_frame �ļ��
	*/
	int CheckEnd(uint8_t End_);

	/*
	���ݳ��ȶ�ȡ����
	*/
	int PushData(uint8_t *data, int Readlength);
	/*
	�������ݵĳ���
	return: -1�������ݳ��Ȳ���
	*/
	int	FrameLength();

	/*
	��ȡ��������һ֡
	*/
	int GetFrame(int endpos = -1);
private:
	/*
	�����������洢����������ͷΪHead
	*/
	std::list<uint8_t> m_FrameBuff;
	std::vector<uint8_t> m_vecFrameBuff; //vec������
	bool m_tag_head;
	//buf_ptr framebuf;
public:
	/*
	�����ݶ�ȡ���ʱ �ŵ��⵱�� �����û�
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
	int m_DesLength;//�������� �Ը��ݳ������������ݽ����������ȵ��ֶ�ռ�����ֽڵ�����
	int m_supplement;//�Ը�������Ҳ�޷���ȫ�����ĸ��貹�䳤��
	int m_frame_type;
};



#endif // !DISPOSE_HPP_

