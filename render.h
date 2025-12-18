#pragma once
#include "global.h"

// 渲染/资源清理相关函数声明（在 render.cpp 中实现）
void InitProcessName();
void CleanupRenderResources();
void CleanupRenderResources_NoInput();
void FinalCleanupAll();
