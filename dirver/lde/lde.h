#pragma once

#include "..\include\config.h"

// LDE 反汇编函数指针类型
typedef int (*LDE_DISASM)(void *p, int dw);

/**
 * @brief 初始化 LDE 引擎，分配内存并加载 shellcode
 * @param pLde 输出 LDE 函数指针
 * @return TRUE 成功，FALSE 失败
 */
BOOLEAN LdeInit(LDE_DISASM *pLde);

/**
 * @brief 释放 LDE 引擎资源
 * @param lde LDE 函数指针
 */
VOID LdeUninit(LDE_DISASM lde);

/**
 * @brief 计算 Hook 目标地址处需要覆盖的最小指令长度
 * @details 从目标地址开始逐条解码指令，累计长度直到 >= shellCodeLen
 * @param hookAddress 目标函数地址
 * @param shellCodeLen 跳板所需最小字节数（通常为 14）
 * @return 实际需要搬移的字节数，0 表示失败
 */
ULONG LdeGetHookLength(PVOID hookAddress, ULONG_PTR shellCodeLen);
