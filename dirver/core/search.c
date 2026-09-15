#include"search.h"
// 原型和 strtoul 完全对应：
// ULONG strtoul(const char *nptr, char **endptr, int base);
typedef PCSTR* PPCSTR;
ULONG KtStrtoul(
    _In_ PCSTR  nptr,
    _Out_ PPCSTR endptr,
    _In_ INT    base
)
{
    ULONG result = 0;
    PCSTR p = nptr;

    // 跳过空白
    while (*p != '\0' && (*p == ' ' || *p == '\t'))
        p++;

    // 处理进制前缀（仅支持 10 / 16，特征码只用16）
    if (base == 16)
    {
        if (p[0] == '0' && (p[1] == 'x' || p[1] == 'X'))
            p += 2;
    }

    for (; *p != '\0'; p++)
    {
        UCHAR digit;
        if (*p >= '0' && *p <= '9')
            digit = *p - '0';
        else if (base == 16 && *p >= 'A' && *p <= 'F')
            digit = *p - 'A' + 10;
        else if (base == 16 && *p >= 'a' && *p <= 'f')
            digit = *p - 'a' + 10;
        else
            break; // 非法字符，停止解析

        result = (result << base / 2) | digit;
    }

    // 输出停止位置
    if (endptr != NULL)
        *endptr = p;

    return result;
}


NTSTATUS searchgetmoduleinfo(const CHAR* modulename, MODULE_INFO* moduleinfo)
{
    NTSTATUS status = NULL;
    ULONG buffersize = 0;
    PRTL_PROCESS_MODULES modulelist = NULL;
    if (moduleinfo == NULL)
    {
        return STATUS_INVALID_PARAMETER;
    }

    status = ZwQuerySystemInformation(SystemModuleInformation, NULL, 0, &buffersize);
    if (status != STATUS_INFO_LENGTH_MISMATCH)
    {
        return status;
    }
    modulelist = (PRTL_PROCESS_MODULES)ExAllocatePoolWithTag(NonPagedPool, buffersize, POOL_TAG);
    if (modulelist == NULL)
    {
        return STATUS_INSUFFICIENT_RESOURCES;
    }
    status = ZwQuerySystemInformation(SystemModuleInformation, modulelist, buffersize, &buffersize);
    if (!NT_SUCCESS(status))
    {
        ExFreePoolWithTag(modulelist, POOL_TAG);
        return status;
    }
    if (modulename == NULL)
    {
        if (modulelist->NumberOfModules > 0)
        {
            moduleinfo->imagebase = modulelist->Modules[0].ImageBase;
            moduleinfo->imagesize = modulelist->Modules[0].ImageSize;
            RtlCopyMemory(moduleinfo->name, (char*)modulelist->Modules[0].FullPathName, 255);
            ExFreePoolWithTag(modulelist, POOL_TAG);

            return STATUS_SUCCESS;
        }
        ExFreePoolWithTag(modulelist, POOL_TAG);

        return STATUS_NOT_FOUND;
    }
    for (ULONG i = 0; i < modulelist->NumberOfModules; i++)
    {
        CHAR* filename = (char*)modulelist->Modules[i].FullPathName + modulelist->Modules[i].OffsetToFileName;
        if (_stricmp(filename, modulename) == 0)
        {
            moduleinfo->imagebase = modulelist->Modules[i].ImageBase;
            moduleinfo->imagesize = modulelist->Modules[i].ImageSize;
            RtlCopyMemory(moduleinfo->name, filename, 255);
            ExFreePoolWithTag(modulelist, POOL_TAG);
            return STATUS_SUCCESS;
        }
    }
    ExFreePoolWithTag(modulelist, POOL_TAG);
    return STATUS_NOT_FOUND;
}

NTSTATUS searchparsepattern(const char* patternstr, PATTERN_DATA* patterndata)
{
    ULONG index= 0;
    const char* p = patternstr;
    if (!patterndata || !patternstr)
    {
        return STATUS_INVALID_PARAMETER;
    }
    RtlZeroMemory(patterndata, sizeof(PATTERN_DATA));
    while (*p != '\0'&& index < MAX_PATTERN_LEN)
    {
        while(*p == ' '|| *p == '\t') p++;
        if(*p=='\0') break;
        if (p[0] == '?' && p[1] == '?')
        {
            patterndata->bytes[index] = 0x00;
            patterndata->mask[index] = 0x00;
            p += 2;
        }
        else
        {
            patterndata->bytes[index] = (UCHAR)KtStrtoul(p, &p, 16);
            patterndata->mask[index] = 0xFF;
        }
        index++;
    }
    patterndata->length = index;
    if (index == 0)
    {
        return STATUS_INVALID_PARAMETER;
    }
    return STATUS_SUCCESS;
}

NTSTATUS searchscanpattern(MODULE_INFO moduleinfo, PPATTERN_DATA patterndata, PVOID* result)
{
    UCHAR* start = (UCHAR*)moduleinfo.imagebase;
    ULONG scansize = moduleinfo.imagesize - patterndata->length+1;
    if (!start || !patterndata || !result)
    {
        return STATUS_INVALID_PARAMETER;
    }
    if (moduleinfo.imagesize < patterndata->length)
    {
        return STATUS_INVALID_PARAMETER;
    }
    *result = NULL;
    for (SIZE_T i = 0; i < scansize; i++)
    {
        BOOLEAN matched = TRUE;
        for (SIZE_T j = 0; j < patterndata->length; j++)
        {
            if (patterndata->mask[j] == 0x00)
            {
                continue;
            }
            if (start[i + j] != patterndata->bytes[j])
            {
                matched = FALSE;
                break;
            }
        }
        if (matched == TRUE)
        {
            *result = start + i;
            return STATUS_SUCCESS;
        }
    }
    return STATUS_NOT_FOUND;
}

NTSTATUS searchfindpattern(MODULE_INFO moduleinfo, const CHAR* patternstr, PVOID* result)
{
    PATTERN_DATA patterndata = {0};
    NTSTATUS status = NULL;
    status=searchparsepattern(patternstr, &patterndata);
    if (!NT_SUCCESS(status))
    {
        LOG_FAIL("searchparsepattern failed");
        return status;
    }
    status = searchscanpattern(moduleinfo, &patterndata, result);
    if (!NT_SUCCESS(status))
    {
        LOG_FAIL("searchscanpattern failed");
    }
    return status;
}

NTSTATUS searchfindpatterninmodule(const CHAR* modulename, const CHAR* patternstr, PVOID* result)
{
    NTSTATUS status = NULL;
    MODULE_INFO moduleinfo = { 0 };

    status = searchgetmoduleinfo(modulename, &moduleinfo);
    if (!NT_SUCCESS(status))
    {
        LOG_FAIL("searchgetmoduleinfo failed");
        return status;
    }
    LOG_INFO("moduleinfo.name:%s", moduleinfo.name);
    LOG_INFO("moduleinfo.imagebase:%p", moduleinfo.imagebase);
    LOG_INFO("moduleinfo.imagesize:%d", moduleinfo.imagesize);
    status = searchfindpattern(moduleinfo, patternstr, result);

    if (!NT_SUCCESS(status))
    {
        LOG_FAIL("searchfindpattern failed");
    }
    else
    {
        LOG_SUCCESS("特征码提取成功！");
        LOG_INFO("find pattern:%p", *result);
    }
    return status;
}




















