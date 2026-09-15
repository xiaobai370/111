#include"include/config.h"
#include"core/search.h"
#include"core/sstd.h"

VOID demossdt()
{
    SSDT_INFO ssdt_info = { 0 };
    sstdlocate(&ssdt_info);
    sstdgetfunctionaddress(ssdt_info.ssdtTable->ServiceTableBase, 0x26);
    sstdgetshadowaddress(ssdt_info.shadowTable, 0);
}

VOID demolde()
{



}




// 驱动卸载函数（设备管理器/命令行卸载驱动触发）
VOID DriverUnload(_In_ PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);

    DbgPrint("===== 驱动开始卸载 =====\n");

    // 在这里写资源清理逻辑：
    // 1. 释放之前申请的堆内存
    // 2. 关闭句柄、销毁线程、删除设备对象
    // 3. 取消注册的回调、删除定时器等

    DbgPrint("===== 驱动卸载完成 =====\n");
}



NTSTATUS DriverEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PUNICODE_STRING RegistryPath
)
{
    NTSTATUS status = STATUS_SUCCESS;

    // 消除未使用参数警告
    UNREFERENCED_PARAMETER(RegistryPath);

    DbgPrint("===== 驱动加载成功 =====\n");

    // 注册卸载回调，驱动卸载时自动执行DriverUnload
    DriverObject->DriverUnload = DriverUnload;

    // 在这里写你的初始化逻辑：
    // 1. 特征扫描函数调用测试
    // 2. 创建线程、注册回调、分配内存等






    return status;
}

