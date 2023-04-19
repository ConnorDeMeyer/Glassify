#pragma once

#include <imgui.h>

struct GLFWwindow;

void InitializeImgui(GLFWwindow* window);
void DestroyImgui();
void DrawImgui();