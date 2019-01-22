#pragma once
#include "basicObj.h"
#include "NetStreamMaker.h"

/**
 * H264�Ľṹ��
 * 00 00 00 01/00 00 01->nal(1bytes)->slice->���->�˶�����������
 * ���h264��body�г�����ǰ׺����00 00 00 01/00 00 01��Ϊ00 03 00 00 01/00 03 00 01.
 */

/** H264ƬԪ���� */
enum NalType
{
    unknow  = 0,  // ����H264����
    b_Nal   = 1,  // B Slice,�ǹؼ�֡
    dpa_Nal = 2,
    dpb_Nal = 3,
    pdc_Nal = 4,
    idr_Nal = 5,  // IDR ,�ؼ�֡
    sei_Nal = 6,  // SEI,������ǿ��Ϣ
    sps_Nal = 7,  // SPS,���в�����
    pps_Nal = 8,  // PPS,ͼ�������
    aud_Nal = 9,
    filler_Nal = 12,
    other,        // ��������
	reserve_end = 23,
	STAP_A = 24,  //24 ��һʱ�����ϰ�
	STAP_B,       //25 ��һʱ�����ϰ�
	MTAP16,       //26 ���ʱ�����ϰ�
	MTAP24,       //27 ���ʱ�����ϰ�
	FU_A,         //28 ��Ƭ�ĵ�Ԫ
	FU_B,         //29 ��Ƭ�ĵ�Ԫ
};

/**
 * Nal��Ԫͷ�ṹ 1���ֽ�
 */
typedef struct nal_unit_header
{
    unsigned char nal_type : 5;     // 4-8λ  ���NALU��Ԫ������ NalType
    unsigned char nal_ref_idc : 2;  // 2-3λ  nal_ref_idc��ȡ00~11���ƺ�ָʾ���NALU����Ҫ��, ��00��NALU���������Զ���������Ӱ��ͼ��Ļطš�����һ������²�̫�����������
    unsigned char for_bit : 1;      // ��һλ forbidden_zero_bit����H.264�淶�й涨����һλ����Ϊ0��

}nal_unit_header_t;

typedef struct nal_unit_header_rtp {
	unsigned char ex_type : 5;      // 4-8λ  ���NALU��Ԫ������ FU_A
    unsigned char nal_ref_idc : 2;  // 2-3λ  nal_ref_idc��ȡ00~11���ƺ�ָʾ���NALU����Ҫ��, ��00��NALU���������Զ���������Ӱ��ͼ��Ļطš�����һ������²�̫�����������
    unsigned char for_bit : 1;      // ��һλ forbidden_zero_bit����H.264�淶�й涨����һλ����Ϊ0��

	unsigned char nal_type : 5;     // 4-8λ  ���NALU��Ԫ������ NalType
	unsigned char R : 1;            //
	unsigned char E : 1;            //
	unsigned char S : 1;
}nal_unit_header_rtp_t;

bool inline is_h264_slice(char* buff) {
	nal_unit_header* pNalUnit = (nal_unit_header*)buff;
	if(pNalUnit->nal_type == NalType::FU_A) {
		return true;
	}
	return false;
}

bool inline is_h264_header(char* buff) {
	nal_unit_header* pNalUnit = (nal_unit_header*)buff;
	bool s = 1;
	NalType nal = (NalType)pNalUnit->nal_type;
	if(NalType::FU_A == pNalUnit->nal_type) {
		s = 0;
		nal_unit_header_rtp_t* pNalUnit2 = (nal_unit_header_rtp_t*)buff;
		nal = (NalType)pNalUnit2->nal_type;
		s = pNalUnit2->S;
	}
    if ( NalType::sps_Nal == nal
      || NalType::pps_Nal == nal
      || NalType::sei_Nal == nal
      || NalType::idr_Nal == nal
      || NalType::b_Nal   == nal) {
		  if(s)
			return true;
	}
	return false;
}

bool inline is_h264_end(char* buff) {
	nal_unit_header* pNalUnit = (nal_unit_header*)buff;
	bool e = 1;
	NalType nal = (NalType)pNalUnit->nal_type;
	if(NalType::FU_A == pNalUnit->nal_type) {
		e = 0;
		nal_unit_header_rtp_t* pNalUnit2 = (nal_unit_header_rtp_t*)buff;
		nal = (NalType)pNalUnit2->nal_type;
		e = pNalUnit2->E;
	}
    if ( NalType::sps_Nal == nal
      || NalType::pps_Nal == nal
      || NalType::sei_Nal == nal
      || NalType::idr_Nal == nal
      || NalType::b_Nal   == nal) {
		  if(e)
			return true;
	}
	return false;
}

NalType inline h264_naltype(char* buff) {
	nal_unit_header* pNalUnit = (nal_unit_header*)buff;
	NalType nal = (NalType)pNalUnit->nal_type;
	if(NalType::FU_A == pNalUnit->nal_type) {
		nal_unit_header_rtp_t* pNalUnit2 = (nal_unit_header_rtp_t*)buff;
		nal = (NalType)pNalUnit2->nal_type;
	}
	return nal;
}

class CH264
{
public:
    CH264(CBasicObj* pObj);
    ~CH264();

    /**
     * ������������
     */
    int InputBuffer(char *pBuf, uint32_t nLen);

    /** ��ȡ���� */
    NalType NaluType(){return m_eNaluType;}

    /** ��ȡ��������λ��(ȥ����001��0001) */
    char* DataBuff(uint32_t& nLen){nLen=m_nDataLen;return m_pDataBuff;}

    /** ��ȡsps�����õ���������Ϣ */
    uint32_t Width(){return m_nWidth;}
    uint32_t Height(){return m_nHeight;}
    double Fps(){return m_nFps;}

private:
    /**
     * ��������
     */
    void ParseNalu();

    /**
     * ����SPS,��ȡ��Ƶͼ�������Ϣ 
     * @return �ɹ��򷵻�true , ʧ���򷵻�false
     */ 
    bool DecodeSps();

    uint32_t Ue(uchar *pBuff, uint32_t nLen, uint32_t &nStartBit);

    int Se(uchar *pBuff, uint32_t nLen, uint32_t &nStartBit);

    /**
     * ��λ���������л�ȡֵ
     * @param buf[in] ������
     * @param nStartBit[inout] ��ʼ��λ,��������ƶ����¸�����
     * @param BitCount[in] ֵռ�õ�λ��
     * @return ָ��λ����ֵ
     */
    uint32_t u(uint32_t BitCount,uchar* buf,uint32_t &nStartBit);

    /**
     * ���ݻ�ԭ�������е�0031��00031��ԭλ001��0001
     */
    void de_emulation_prevention(uchar* buf,uint32_t* buf_size);

private:
    char*       m_pNaluBuff;    //< ��������
    uint32_t    m_nBuffLen;     //< ���ݳ���
    char*       m_pDataBuff;    //< ȥ����001��0001�������
    uint32_t    m_nDataLen;     //< �������ݵĳ���
    NalType     m_eNaluType;    //< ����

    CNetStreamMaker    *m_pSPS;            // ����SPS
    CNetStreamMaker    *m_pPPS;            // ����PPS
    CNetStreamMaker    *m_pFullBuff;       // ����h264���� 7 8 5 1 1 1 1 ... 1
    bool               m_bFirstKey;        // �Ѿ������һ���ؼ�֡
    bool               m_bDecode;          // �Ƿ��Ѿ�����sps
    CBasicObj*          m_pObj;

    /** sps�е����� */
    int32_t     m_nWidth;
    int32_t     m_nHeight;
    double      m_nFps;
};