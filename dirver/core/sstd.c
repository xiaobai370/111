#include "sstd.h"
#include "search.h"

const CHAR* g_ssdtpattern = "";

NTSTATUS fundcsrssprocess(PEPROCESS* process)
{
    PEPROCESS currentprocess = NULL;
    UNICODE_STRING csrssname = { 0 };
    NTSTATUS status = NULL;
    RtlInitUnicodeString(&csrssname, L"csrss.exe");
    for (size_t i = 0; i < 1000; i++)
    {
        status = PsLookupProcessByProcessId(i, &currentprocess);
        if (NT_SUCCESS(status))
        {
            PUCHAR imagename = PsGetProcessImageFileName(currentprocess);
            if (imagename != NULL && _stricmp(imagename, "csrss.exe")==0)
            {
                *process=currentprocess;
                return STATUS_SUCCESS;
            }
            ObDereferenceObject(currentprocess);
        }
    }
    *process = NULL;
    return STATUS_UNSUCCESSFUL;
}

NTSTATUS sstdlocate(PSSDT_INFO info)
{
    PVOID matchaddr = NULL;
    NTSTATUS status = STATUS_NOT_FOUND;
    LONG disp = 0;
    PUCHAR instraddr = NULL;

    if (info == NULL)
    {
        return STATUS_UNSUCCESSFUL;
    }
    RtlZeroMemory(info, sizeof(SSDT_INFO));

    status= searchfindpatterninmodule(NULL, g_ssdtpattern, &matchaddr);
    if (!NT_SUCCESS(status))
    {
        LOG_FAIL("sstdlocate: searchfindpatterninmodule failed");
        return STATUS_NOT_FOUND;
    }
    LOG_SUCCESS("ssdtlocate: searchfindpatterninmodule success");
    instraddr = (PUCHAR)matchaddr;
    disp = *(PLONG)(instraddr + 3);
    info->ssdtTable=(PKSERVICE_TABLE_DESCRIPTOR)(instraddr + disp+7);
    LOG_SUCCESS("ssdtlocate: ssdtTable address is 0x%p", info->ssdtTable);

    disp = *(PLONG)(instraddr + 10);
    info->shadowTable = (PKSERVICE_TABLE_DESCRIPTOR)(instraddr + disp + 14);
    LOG_SUCCESS("ssdtlocate: shadowTable address is 0x%p", info->shadowTable);

    return STATUS_SUCCESS;
}

/*
ssdt结构
31                    4   3   2   1   0
┌───────────────────────┬───────────────┐
│     高28bit压缩偏移   │低4bit栈参数字节数│
│    entry >> 4 的值    │    entry & 0xF │
└───────────────────────┴───────────────┘
*/

PVOID sstdgetfunctionaddress(PLONG servicetablebase, ULONG index)
{
    LONG entry = 0;
    if (servicetablebase == NULL)
    {
        return NULL;
    }
    entry = servicetablebase[index];
    return (PVOID)((ULONG_PTR)servicetablebase+(entry>>4));
}

PVOID sstdgetshadowaddress(PKSERVICE_TABLE_DESCRIPTOR shadowtable, ULONG index)
{
    PEPROCESS csrssprocess = NULL;
    KAPC_STATE apcstate = {0};
    PKSERVICE_TABLE_DESCRIPTOR win32ktable = NULL;
    PVOID funaddr= NULL;
    NTSTATUS status = NULL;
    if (shadowtable == NULL)
    {
        return NULL;
    }
    status = fundcsrssprocess(&csrssprocess);
    if (!NT_SUCCESS(status))
    {
        LOG_FAIL("sstdgetshadowaddress: fundcsrssprocess failed");
        return NULL;
    }
    win32ktable=&shadowtable[1];
    KeStackAttachProcess(csrssprocess, &apcstate);
    if (win32ktable->ServiceTableBase != NULL && index < win32ktable->NumberOfServices)
    {
        LONG entry=win32ktable->ServiceTableBase[index];
        funaddr = (PVOID)((ULONG_PTR)win32ktable->ServiceTableBase + (entry >> 4));
    }
    else
    {
        LOG_FAIL("sstdgetshadowaddress: win32ktable->ServiceTableBase is NULL or index out of range");
    }
    KeUnstackDetachProcess(&apcstate);
    ObDereferenceObject(csrssprocess);
    return funaddr;
}