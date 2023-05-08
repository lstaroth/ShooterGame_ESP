#pragma once
#include "imgui.h"

namespace Render
{
    void Init();
    void NewFrame();
    void ShowFrame();
    void Release();

    void Line(ImVec2 a, ImVec2 b, ImColor color, float thickness);
    void DrawBox(ImColor color, int x, int y, int w, int h);
}