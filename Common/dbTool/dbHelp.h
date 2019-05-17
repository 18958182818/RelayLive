/**
 * ��װһ�����ݿ���롢���µȲ�����Ƶ�������������ݣ�����Ҫ�����󶨵Ľӿ�
 */
#pragma once

namespace dbTool
{

#define SQLT_CHR 1 
#define SQLT_INT 3 
#define SQLT_FLT 4 
#define SQLT_LNG 8 
#define SQLT_UIN 68 
#define SQLT_BLOB 113 
#define SQLT_ODT 156 

struct bindParam
{
    char*       bindName;    //< �󶨱������
    uint32_t    columnType;  //< ������
    uint32_t    maxLen;      //< ��󳤶ȣ��������ַ���ʱ��������
    bool        nullable;    //< �Ƿ��Ϊ��
    char*       default;     //< Ĭ��ֵ��ż����Ҫ
};

struct helpHandle;

/**
 * ����һ���������
 * @param tag ���ӳر�ǩ
 * @param sql ִ�е�sql
 * @param rowNum ÿ�����󶨵���������,���ݴﵽ���ֵ�ᴥ��ִ��sql
 * @param interval ��������룬�������ݵ�����rowNumʱ�ᴥ��sql
 * @param binds ����Ϣ�����飬��bindName=NULL������bindName��Ҫ��sql�еİ󶨱�Ƕ�Ӧ
 */
helpHandle* CreateHelp(string tag, string sql, int rowNum, int interval, bindParam* binds);

/**
 * �ͷ�һ���������
 * @param clean true����Ҫ���Ѿ�����ļ�¼��Ϣȫ���������ݿ⣻false��������������ֱ���ͷ�
 */
void FreeHelp(helpHandle* h, bool clean = false);

/**
 * ͨ��������������ݿ��в���һ�м�¼��
 */
void AddRow(helpHandle* h, vector<string> row);

}