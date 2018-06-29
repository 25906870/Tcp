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
根据长度读取数据
1.当tcp收到消息时会将信息传入dispose 在传入的数据中寻找 Head标志位 将缓冲区内head标志位前的数据丢掉
2.结束标准读到需要长度时 判断尾部是否符合标准 若不符合 在缓冲区中后移一位继续读取或者直接判断 等到读到符合要求的数据
3.读到完整的一帧 就返回1 否则为0继续读取
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
			int end = FrameLen + m_DesAndSite + m_supplement - m_Headstr.size();//m_DesAndSite保函了头的长度
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
根据头尾读取数据
1.当tcp收到消息时会将信息传入dispose 在传入的数据中寻找 Head标志位 将缓冲区内head标志位前的数据丢掉
2.直至读到 end为止
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
读取数据时候 查找Head并将不匹配的数据去除
*/
int DisposeRecivedMsg::FindHead(int *preendpos)
{
	int tag = 0;

	auto it = m_vecFrameBuff.begin();
	auto it0 = it;
	int pos = kmp_search(m_vecFrameBuff.data(), m_vecFrameBuff.size(), m_Headstr);
	if (pos == -1)return -1;
	if (preendpos != NULL && *preendpos < pos)return -1;//判断是否越界
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
*return : -1 找不到头
0  表示已经有了一帧的开头无需再找
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
根据长度读取数据
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
计算数据的长度
return: -1数据内容长度不够
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
获取到完整的一帧
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


/*功能：求模式串值
*参数：ptn：模式串
*nextval：保存模式串值的数组
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

/*功能：实现KMP算法
*参数：src：源串
*      patn：模式串
*      nextval：模式串值
*      pos：源串开始的位置
*返回值：若匹配成功，则返回下标；若出错或匹配不成功，则返回-1
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
			//当匹配失效时，直接用p[j_next]与s[i]比较
			//下面阐述怎么求这个值，即匹配失效后的下一次匹配的位置
		}
	}
	if (j >= plen)
	{
		delete[]nextval;
		nextval = NULL;
		return i - plen;//返回下标，从0开始
	}
	else
	{
		delete[]nextval;
		nextval = NULL;
		return -1;
	}
}



