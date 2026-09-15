#pragma once

#include<ntifs.h>
//字符串操作头文件
#include<ntstrsafe.h>

//内存池标签
#define POOL_TAG 'ccx0'

//调试信息输出
#define LOG_PREFIX "[KernelSearch]"
#define LOG_SUCCESS(fmt,...) DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, LOG_PREFIX "[+]" fmt "\r\n", ##__VA_ARGS__)
#define LOG_FAIL(fmt,...) DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, LOG_PREFIX "[-]" fmt "\r\n", ##__VA_ARGS__)
#define LOG_WARN(fmt,...) DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, LOG_PREFIX "[!]" fmt "\r\n", ##__VA_ARGS__)
#define LOG_INFO(fmt,...) DbgPrintEx(DPFLTR_IHVVIDEO_ID, DPFLTR_ERROR_LEVEL, LOG_PREFIX "[*]" fmt "\r\n", ##__VA_ARGS__)

//特征码字符串最大长度
#define MAX_PATTERN_LEN 128

//通配符标记值
#define PATTERN_WILDCARD 0xcc



//结构体定义
typedef struct _RTL_PROCESS_MODULE_INFORMATION {
    HANDLE Section;                // 该模块对应的Section句柄
    PVOID MappedBase;              // 模块加载虚拟基址
    PVOID ImageBase;               // PE头基址（一般等于MappedBase）
    ULONG ImageSize;               // PE镜像大小
    ULONG Flags;
    USHORT LoadOrderIndex;
    USHORT InitOrderIndex;
    USHORT LoadCount;              // 引用计数
    USHORT OffsetToFileName;       // FileName字符串相对本结构的偏移
    UCHAR FullPathName[256];       // 完整路径，OffsetToFileName指向此处模块名
} RTL_PROCESS_MODULE_INFORMATION, * PRTL_PROCESS_MODULE_INFORMATION;

// 模块数组头部结构
typedef struct _RTL_PROCESS_MODULES {
    ULONG NumberOfModules;                      // 当前进程模块总数
    RTL_PROCESS_MODULE_INFORMATION Modules[1];  // 变长数组，后面紧跟所有模块
} RTL_PROCESS_MODULES, * PRTL_PROCESS_MODULES;

//导出函数声明
typedef enum _SYSTEM_INFORMATION_CLASS {
    SystemBasicInformation,                  // 0
    SystemPerformanceInformation,            // 1
    SystemTimeOfDayInformation,              // 2
    SystemPathInformation,                   // 3
    SystemProcessInformation,                // 5
    SystemCallCountInformation,              // 6
    SystemDeviceInformation,                 // 7
    SystemProcessorPerformanceInformation,   // 8
    SystemModuleInformation,                 // 11
    SystemHandleInformation,                 // 16
    SystemThreadInformation,                 // 17
    SystemFileCacheInformation,              // 21
    SystemPoolTagInformation,                // 22
    SystemBigPoolInformation,                // 66
    // 还有大量扩展值，不同Windows版本新增
} SYSTEM_INFORMATION_CLASS;

NTSTATUS
ZwQuerySystemInformation(
    _In_      SYSTEM_INFORMATION_CLASS SystemInformationClass,
    _Out_     PVOID                    SystemInformation,
    _In_      ULONG                    SystemInformationLength,
    _Out_opt_ PULONG                   ReturnLength
);

PUCHAR PsGetProcessImageFileName(PEPROCESS Process);