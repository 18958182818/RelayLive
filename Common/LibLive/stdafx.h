// stdafx.h : ��׼ϵͳ�����ļ��İ����ļ���
// ���Ǿ���ʹ�õ��������ĵ�
// �ض�����Ŀ�İ����ļ�
//

#pragma once

#include <stdio.h>
#include <tchar.h>
#include <stdlib.h>   
#include <iostream>
#include <string>
#include <sstream>


// TODO: �ڴ˴����ó�����Ҫ������ͷ�ļ�  
#include "common.h"
#include "Endian.h"

using namespace std;



enum STREAM_TYPE {
	STREAM_UNKNOW = 0,
	STREAM_PS,
	STREAM_H264
};
extern STREAM_TYPE g_stream_type;