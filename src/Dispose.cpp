#include "Dispose.h"

DisposeRecivedMsg::DisposeRecivedMsg(const unsign_str &Head_, const unsign_str &End_, int type,
		int LengthSite_, int DesLength_, int supplementlength) :
		m_LengthSite(LengthSite_), m_Headstr(Head_), m_Endstr(End_), m_tag_head(false),
		m_supplement(supplementlength), m_frame_type(type), m_DesLength(DesLength_)
	{
		m_DesAndSite = m_LengthSite + m_DesLength;
	}
DisposeRecivedMsg::DisposeRecivedMsg() {}


int DisposeRecivedMsg::ReadFrame(uint8_t *data, int Readlength, std::vector<std::vector<uint8_t>>& frames)
{
	if (m_frame_type == READ_HEADEND)
	{
		return ReadFrameHeadEnd(data, Readlength, frames);
	}
	else
	{
		return ReadFrameAccordinglength(data, Readlength, frames);
	}
}
/*
���ݳ��ȶ�ȡ����
1.��tcp�յ���Ϣʱ�Ὣ��Ϣ����dispose �ڴ����������Ѱ�� Head��־λ ����������head��־λǰ�����ݶ���
2.������׼������Ҫ����ʱ �ж�β���Ƿ���ϱ�׼ �������� �ڻ������к���һλ������ȡ����ֱ���ж� �ȵ���������Ҫ�������
3.����������һ֡ �ͷ���1 ����Ϊ0������ȡ
*/
int DisposeRecivedMsg::ReadFrameAccordinglength(uint8_t *data, int Readlength, std::vector<std::vector<uint8_t>>& frames)
{
	PushData(data, Readlength);
	frames.clear();

	while (true && !m_vecFrameBuff.empty())
	{
		int tag = FindHeadAccordinglength();
		int FrameLen = FrameLength();

		if (FrameLen > 1048)
		{
			m_tag_head = false;
			return RETURN::WAIT_DATA;
		}

		if (tag == -1 || FrameLen == -1)
		{
			return RETURN::WAIT_DATA;
		}
		if (FrameLen != -1)
		{
			int end = FrameLen + m_DesAndSite + m_supplement - m_Headstr.size();//m_DesAndSite������ͷ�ĳ���
			if (end > m_vecFrameBuff.size())
			{
				return RETURN::WAIT_DATA;
			}
			//int endpos = FindEnd();
			auto it = m_vecFrameBuff.begin();
			for (size_t i = 0; i < end - m_Endstr.size() && m_vecFrameBuff.end() != it; i++)
			{
				it++;
			}
			bool tag_frame = true;
			for (auto ch : m_Endstr)
			{
				if (*it != ch)
				{
					m_tag_head = false;
					tag_frame = false;
					continue;
				}
				if (it != m_vecFrameBuff.end())
				{
					it++;
				}
			}
			if (tag_frame)
			{
				GetFrame(end);
				frames.push_back(m_frame);
			}


		}

		if (m_vecFrameBuff.size()<m_Headstr.size())
		{
			break;
		}
	}

	return RETURN::ANALYZE_SUCCESS;
}

/*
����ͷβ��ȡ����
1.��tcp�յ���Ϣʱ�Ὣ��Ϣ����dispose �ڴ����������Ѱ�� Head��־λ ����������head��־λǰ�����ݶ���
2.ֱ������ endΪֹ
*/
int DisposeRecivedMsg::ReadFrameHeadEnd(uint8_t *data, int Readlength, std::vector<std::vector<uint8_t>>& frames)
{
	/*for (size_t i = 0; i < length; i++)
	{

	}*/
	PushData(data, Readlength);
	frames.clear();

	while (true && !m_vecFrameBuff.empty())
	{

		int tag = 0, tagend = 0;
		if (m_tag_head == false)
		{
			tag = FindHead();
		}

		if (tag == -1)
		{
			return RETURN::WAIT_DATA;
		}
		tagend = FindEnd();

		if (tagend == -1)
		{
			return RETURN::WAIT_DATA;
		}
		else
		{
			while (FindHead(&tagend) != -1)
			{

			}
			GetFrame(tagend);
			frames.push_back(m_frame);
		}
	}
	return RETURN::ANALYZE_SUCCESS;
}

/*
��ȡ����ʱ�� ����Head������ƥ�������ȥ��
*/
int DisposeRecivedMsg::FindHead(int *preendpos)
{
	int tag = 0;

	auto it = m_vecFrameBuff.begin();
	auto it0 = it;
	int pos = kmp_search(m_vecFrameBuff.data(), m_vecFrameBuff.size(), m_Headstr);
	if (pos == -1)return -1;
	if (preendpos != NULL && *preendpos < pos)return -1;//�ж��Ƿ�Խ��
	m_tag_head = true;
	for (int i = 0; i < pos; ++it0, ++i)
	{

	}
	for (size_t i = 0; i < m_Headstr.size(); i++, it0++)
	{

	}
	tag = pos;
	if (preendpos != NULL)
	{
		*preendpos -= m_Headstr.size();
		*preendpos -= pos; //20180518
	}
	m_vecFrameBuff.erase(it, it0);

	return pos == -1 ? pos : 0;
}

int DisposeRecivedMsg::FindEnd()
{
	int tag = 0;

	auto it = m_vecFrameBuff.begin();
	auto it0 = it;
	int pos = kmp_search(m_vecFrameBuff.data(), m_vecFrameBuff.size(), m_Endstr);
	//if (pos == -1)break;
	for (int i = 0; i < pos; ++it0, i++)
	{

	}

	return pos;
}

/*
*return : -1 �Ҳ���ͷ
0  ��ʾ�Ѿ�����һ֡�Ŀ�ͷ��������
*/
int DisposeRecivedMsg::FindHeadAccordinglength()
{

	if (m_tag_head)
	{
		return 0;
	}
	int pos = -1;
	if (m_vecFrameBuff.size()>m_Headstr.size())
	{
		pos = FindHead();
		if (pos != -1)
		{
			m_tag_head = true;
		}
	}
	return pos;
}


/*
���ݳ��ȶ�ȡ����
*/
int DisposeRecivedMsg::PushData(uint8_t *data, int Readlength)
{
	for (size_t i = 0; i < Readlength; i++)
	{
		m_vecFrameBuff.push_back(data[i]);
	}

	return 0;
}
/*
�������ݵĳ���
return: -1�������ݳ��Ȳ���
*/
int	DisposeRecivedMsg::FrameLength()
{
	uint8_t temp[4];
	auto tempframe = m_vecFrameBuff.begin();
	if (m_vecFrameBuff.size() < m_DesAndSite - m_Headstr.size())return -1;
	for (int i = 0; i < m_LengthSite - m_Headstr.size(); i++)
		tempframe++;
	for (int i = 0; i < m_DesLength; i++)
	{
		temp[i] = *tempframe;
		tempframe++;
	}
	int FrameLength = qingpei::toolkit::cast::bytes_to_num<uint16_t>(temp, m_DesLength);
	return FrameLength;
}

/*
��ȡ��������һ֡
*/
int DisposeRecivedMsg::GetFrame(int endpos)
{
	if (endpos == -1)
	{
		m_frame = m_Headstr;
		for (auto ch : m_vecFrameBuff)
		{
			m_frame.push_back(ch);
		}
		m_vecFrameBuff.clear();
	}
	else
	{

		m_frame = m_Headstr;
		auto it = m_vecFrameBuff.begin();
		auto it0 = it;
		for (size_t i = 0; i < endpos; i++, it++)
		{
			m_frame.push_back(*it);
		}
		m_vecFrameBuff.erase(it0, it);
	}
	m_tag_head = false;
	return 0;
}


/*���ܣ���ģʽ��ֵ
*������ptn��ģʽ��
*nextval������ģʽ��ֵ������
*/
void get_nextval(const unsign_str &ptn, int *nextval)
{
	int i = 0;
	nextval[0] = -1;
	int j = -1;
	int plen = ptn.size() - 1;

	if (plen == 0 || nextval == NULL)
	{
		return;
	}
	while (i < plen)
	{
		if (j == -1 || ptn[i] == ptn[j])
		{
			++i;
			++j;
			if (ptn[i] != ptn[j])
			{
				nextval[i] = j;
			}
			else
			{
				nextval[i] = nextval[j];
			}
		}
		else
		{
			j = nextval[j];
		}
	}
}

/*���ܣ�ʵ��KMP�㷨
*������src��Դ��
*      patn��ģʽ��
*      nextval��ģʽ��ֵ
*      pos��Դ����ʼ��λ��
*����ֵ����ƥ��ɹ����򷵻��±ꣻ�������ƥ�䲻�ɹ����򷵻�-1
*/
int kmp_search(const uint8_t *src, int srclength, const unsign_str &patn, int pos)
{
	int *nextval = new int[patn.size()];
	get_nextval(patn, nextval);
	int i = pos;
	int j = 0;
	if (srclength == 0 || patn.size() == 0)
	{
		delete[]nextval;
		nextval = NULL;
		return -1;
	}
	int slen = srclength;
	int plen = patn.size();

	if (pos < 0 || pos > slen)
	{
		delete[]nextval;
		nextval = NULL;
		return -1;
	}

	while (i < slen && j < plen)
	{
		if (j == -1 || src[i] == patn[j])
		{
			++i;
			++j;
		}
		else
		{
			j = nextval[j];
			//��ƥ��ʧЧʱ��ֱ����p[j_next]��s[i]�Ƚ�
			//���������ô�����ֵ����ƥ��ʧЧ�����һ��ƥ���λ��
		}
	}
	if (j >= plen)
	{
		delete[]nextval;
		nextval = NULL;
		return i - plen;//�����±꣬��0��ʼ
	}
	else
	{
		delete[]nextval;
		nextval = NULL;
		return -1;
	}
}



