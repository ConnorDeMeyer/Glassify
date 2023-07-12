#include "ImGui_GlasDemo.h"

#include <vector>

#include <backends/imgui_impl_glfw.h>
#include <backends/imgui_impl_opengl3.h>

#include "GLFW/glfw3.h"

#include "glassify.h"
#include "ImGuiEditor/ImGuiWrappers.h"

struct GlasDemoApplication
{
	std::vector<glas::Storage::TypeStorage> GlobalVars;
};

GLAS_STORAGE_DISABLE_COPY(decltype(GlasDemoApplication::GlobalVars));
GLAS_STORAGE_DISABLE_COPY(GlasDemoApplication)





GLAS_TYPE(GlasDemoApplication)
GLAS_MEMBER(GlasDemoApplication, GlobalVars)

GlasDemoApplication g_Application{};

void InitializeImgui(GLFWwindow* window)
{
	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
	io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

	// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
	ImGuiStyle& style = ImGui::GetStyle();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		style.WindowRounding = 0.0f;
		style.Colors[ImGuiCol_WindowBg].w = 1.0f;
	}

	const char* glsl_version = "#version 130";

	ImGui_ImplGlfw_InitForOpenGL(window, true);
	ImGui_ImplOpenGL3_Init(glsl_version);
}

void DestroyImgui()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();

	ImGui::DestroyContext();
}

void ShowDockSpace()
{
	ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;
	ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;

	const ImGuiViewport* viewport = ImGui::GetMainViewport();
	ImGui::SetNextWindowPos(viewport->WorkPos);
	ImGui::SetNextWindowSize(viewport->WorkSize);
	ImGui::SetNextWindowViewport(viewport->ID);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
	window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
	window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
	ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));

	ImGui::Begin("Main DockSpace", nullptr, window_flags);

	ImGui::DockSpace(420, ImVec2(0.0f, 0.0f), dockspace_flags);
	ImGui::PopStyleVar(3);

	ImGui::End();
}

void DrawTypeIds();
void DrawGlobalVariables();
void DrawGlobalFunctions();

void DrawImgui()
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ShowDockSpace();

	DrawTypeIds();
	DrawGlobalVariables();
	DrawGlobalFunctions();
	//ImGui::ShowDemoWindow();

	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

	ImGuiIO& io = ImGui::GetIO();
	if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
	{
		GLFWwindow* backup_current_context = glfwGetCurrentContext();
		ImGui::UpdatePlatformWindows();
		ImGui::RenderPlatformWindowsDefault();
		glfwMakeContextCurrent(backup_current_context);
	}
}

void DrawTypeIds()
{
	ImGui::Begin("Type Information", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	{
		auto& typeInfos = glas::GetAllTypeInfo();

		for (auto& typeInfo : typeInfos)
		{
			ImGui::PushID(static_cast<int32_t>(typeInfo.first.GetId()));
			std::string nameString = std::string(typeInfo.second.Name);
			if (ImGui::CollapsingHeader(nameString.c_str()))
			{
				ImGui::Text("Size: [%u]", typeInfo.second.Size);
				ImGui::Text("Alignment: [%u]", typeInfo.second.Align);
				
				ImGui::Indent();
				for (auto& member : typeInfo.second.Members)
				{
					std::string memberString = std::string(member.Name);
					if (ImGui::TreeNode(memberString.c_str()))
					{
						ImGui::Text("Offset: [%u]", member.Offset);
						ImGui::Text("Type: [%s]", member.VariableId.ToString().c_str());
						ImGui::TreePop();
					}
				}
				ImGui::Unindent();
			}
			ImGui::PopID();
		}
	}
	ImGui::End();
}

void DrawGlobalVariables()
{
	ImGui::Begin("Global Variables", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	{
		ImGui::GlasAuto("Global Variables", g_Application.GlobalVars);
	}
	ImGui::End();
}

void DrawGlobalFunctions()
{
	ImGui::Begin("Global Functions", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
	{
		ImGui::TextUnformatted("Check Console for outputs");

		static std::unordered_map<glas::FunctionId, glas::Storage::TypeTuple> functionParameters;

		auto& functions = glas::GetGlobalData().FunctionInfoMap;

		for (auto& [functionId, functionInfo] : functions)
		{
			ImGui::PushID(static_cast<int>(functionId.GetId()));
			if (ImGui::CollapsingHeader(functionInfo.Name.data()))
			{
				auto it = functionParameters.find(functionId);
				if (it == functionParameters.end())
				{
					it = functionParameters.emplace(functionId, glas::Storage::TypeTuple::CreateNoReferences(functionInfo.ParameterTypes)).first;
				}

				ImGui::GlasAuto("", it->second);

				if (ImGui::Button("Call"))
				{
					glas::Storage::TypeStorage storage{};

					if (functionInfo.ReturnType.GetTypeId().IsValid() &&
						functionInfo.ReturnType.GetTypeId() != glas::TypeId::Create<void>() &&
						!functionInfo.ReturnType.IsRefOrPointer())
					{
						storage = glas::Storage::TypeStorage(functionInfo.ReturnType.GetTypeId());
					}

					functionInfo.Call(it->second, storage.GetData());

					if (storage.GetData())
					{
						glas::Serialization::Serialize(std::cout, storage.GetData(), storage.GetType());
						std::cout << '\n';
					}
				}
			}
			ImGui::PopID();

			ImGui::Separator();
		}
	}
	ImGui::End();
}

void HelloWorld()
{
	std::cout << "Hello World!\n";
}
GLAS_FUNCTION(HelloWorld);

void PrintString(const std::string& text)
{
	std::cout << text << '\n';
}
GLAS_FUNCTION(PrintString);

auto ReturnComplexValue()
{
	return std::tuple<int, double, float, bool>{5, 52.12, 12.66f, true};
}
GLAS_FUNCTION(ReturnComplexValue);



