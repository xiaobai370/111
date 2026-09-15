#pragma once
#include"..\include/config.h"

typedef struct _PATTERN_DATA
{
    UCHAR bytes[MAX_PATTERN_LEN];		//字符值
    UCHAR mask[MAX_PATTERN_LEN];		//掩码 0xFF=精确匹配，0x00=模糊匹配
    ULONG length;						//特征码长度
}PATTERN_DATA, * PPATTERN_DATA;


typedef struct _MODULE_INFO
{
    PVOID imagebase;					//模块基址
    ULONG imagesize;					//模块大小
    CHAR name[256];						//模块名

}MODULE_INFO, * PMODULE_INFO;

NTSTATUS searchgetmoduleinfo(const CHAR* modulename, MODULE_INFO* moduleinfo);

NTSTATUS searchparsepattern(const char* patternstr, PATTERN_DATA* patterndata);

NTSTATUS searchscanpattern(MODULE_INFO moduleinfo, PPATTERN_DATA patterndata, PVOID* result);

NTSTATUS searchfindpattern(MODULE_INFO moduleinfo, const CHAR* patternstr, PVOID* result);

NTSTATUS searchfindpatterninmodule(const CHAR* modulename, const CHAR* patternstr, PVOID* result);