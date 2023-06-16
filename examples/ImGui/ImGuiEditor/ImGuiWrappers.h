#pragma once
#include "glassify.h"
#include "imgui.h"

GLAS_TYPE(ImVec2)
GLAS_MEMBER(ImVec2, x)
GLAS_MEMBER(ImVec2, y)

struct ImGuiWrapper_InputText final
{
	std::string Label{};
	std::string Hint{};
	std::string Content{};
};

GLAS_TYPE(ImGuiWrapper_InputText)
GLAS_MEMBER(ImGuiWrapper_InputText, Label)
GLAS_MEMBER(ImGuiWrapper_InputText, Hint)
GLAS_MEMBER(ImGuiWrapper_InputText, Content)

struct ImGuiWrapper_Button final
{
	std::string Label{};
	ImVec2 Size{};
};

GLAS_TYPE(ImGuiWrapper_Button)
GLAS_MEMBER(ImGuiWrapper_Button, Label)
GLAS_MEMBER(ImGuiWrapper_Button, Size)

struct ImGuiWrapper_CheckBox final
{
	std::string Label{};
	bool Value{};
};

GLAS_TYPE(ImGuiWrapper_CheckBox)
GLAS_MEMBER(ImGuiWrapper_CheckBox, Label)
GLAS_MEMBER(ImGuiWrapper_CheckBox, Value)

struct ImGuiWrapper_RadioButton final
{
	std::string Label{};
	bool Active{};
};

GLAS_TYPE(ImGuiWrapper_RadioButton)
GLAS_MEMBER(ImGuiWrapper_RadioButton, Label)
GLAS_MEMBER(ImGuiWrapper_RadioButton, Active)

