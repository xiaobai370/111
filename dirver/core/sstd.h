#pragma once
#include "../include/config.h"

typedef struct _KSERVICE_TABLE_DESCRIPTOR
{
    PLONG ServiceTableBase;         //函数偏移表
    PVOID CounterTableBase;         //计数表
    ULONG NumberOfServices;         //服务函数数量
    PUCHAR ParamTableBase;          //参数表
}KSERVICE_TABLE_DESCRIPTOR,*PKSERVICE_TABLE_DESCRIPTOR;

typedef struct _SSDT_INFO
{
    PKSERVICE_TABLE_DESCRIPTOR ssdtTable;
    PKSERVICE_TABLE_DESCRIPTOR shadowTable;
}SSDT_INFO,*PSSDT_INFO;

NTSTATUS fundcsrssprocess(PEPROCESS* process);

NTSTATUS sstdlocate(PSSDT_INFO info);

PVOID sstdgetfunctionaddress(PLONG servicetablebase, ULONG index);

PVOID sstdgetshadowaddress(PKSERVICE_TABLE_DESCRIPTOR shadowtable, ULONG index);