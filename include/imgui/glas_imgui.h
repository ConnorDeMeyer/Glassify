#pragma once

#include "glas_imgui_config.h"
#include "../glas_config.h"
#include "../glas_impl.h"

#include <type_traits>
#include <imgui.h>
#include <imgui_internal.h>


namespace ImGui
{
	template <typename T>
	constexpr void FillInfo(glas::TypeInfo& info)
	{
		info.ImGuiRenderer = [](const char* label, void* data) -> bool { return GlasAuto(label, *static_cast<T*>(data)); };
	}

}

/** HELPER FUNCTIONS */

inline const void* VoidOffset(const void* data, size_t offset)
{
	return static_cast<const uint8_t*>(data) + offset;
}

inline void* VoidOffset(void* data, size_t offset)
{
	return static_cast<uint8_t*>(data) + offset;
}

constexpr bool IsCStringEmpty(const char* str)
{
	return str[0] == '\0';
}


/** GLAS */
namespace ImGui
{
	inline bool GlasAuto(const char* label, glas::TypeId& type)
	{
		bool returnValue{};

		if (BeginCombo(label, type.GetId() ? std::string(type.GetInfo().Name).c_str() : ""))
		{
			auto& typeInfos = glas::GetAllTypeInfo();
			for (auto& [id, info] : typeInfos)
			{
				bool selected{ glas::TypeId{id} == type };
				if (Selectable(std::string(info.Name).c_str(), &selected) && id != type)
				{
					type = id;
					returnValue = true;
				}
			}
			EndCombo();
		}

		return returnValue;
	}

	//inline bool GlasAuto(const char* label, glas::VariableId& type);
	//inline bool GlasAuto(const char* label, glas::FunctionId& type);
}

/** STRING */
#if defined(GLAS_IMGUI_STRING) || defined(_STRING_)
#include <misc/cpp/imgui_stdlib.h>
namespace ImGui
{
	inline bool GlasAuto(const char* label, std::string& value)
	{
		return ImGui::InputText(label, &value);
	}
}
#endif

/** STRING VIEW */
#if defined(GLAS_IMGUI_STRING_VIEW) || defined(_STRING_VIEW_)
#include <string_view>
namespace ImGui
{
	inline bool GlasAuto(const char* label, std::string_view& string)
	{
		if (label && IsCStringEmpty(label))
		{
			ImGui::TextUnformatted(label);
			ImGui::SameLine();
			ImGui::TextUnformatted(" : ");
			ImGui::SameLine();
		}
		ImGui::TextUnformatted(string.data(), string.data() + string.size());
		return false;
	}
}
#endif

///** CONTAINER HELPERS */
//#pragma region ContainerHelpers
//namespace ImGui
//{
//	template <typename Container>
//	bool GlasAutoContainer(Container& container)
//	{
//		stream << '[';
//
//		int counter{};
//		for (auto& val : container)
//		{
//			if (counter++ != 0)
//				stream << ',';
//
//			GlasAutoType(stream, val);
//		}
//
//		stream << ']';
//	}
//
//	template <typename Container>
//	bool GlasAutoMapContainer(Container& container)
//	{
//		stream << '{';
//
//		int counter{};
//		for (auto& [key, val] : container)
//		{
//			if (counter++ != 0)
//				stream << ',';
//
//			GlasAutoType(stream, key);
//			stream << ": ";
//			GlasAutoType(stream, val);
//		}
//
//		stream << '}';
//	}
//}
//#pragma endregion

/** VECTOR */
#if defined(GLAS_IMGUI_VECTOR) || defined(_VECTOR_)
namespace ImGui
{
	template <typename T>
	bool GlasAuto(const char* label, std::vector<T>& value)
	{
		bool returnVal{};

		TextUnformatted(label);

		if (Button("+")) value.emplace_back(); SameLine();
		if (Button("-")) value.pop_back();

		//BeginChild(GetID(&value), {0, 300}, true, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_MenuBar);
		if (BeginTable(label, 1, ImGuiTableFlags_BordersOuter | ImGuiTableFlags_RowBg | ImGuiTableFlags_NoBordersInBody | ImGuiTableFlags_SizingStretchProp))
		{
			
			for (auto& element : value)
			{
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				PushID(&element);
				returnVal |= GlasAuto("", element);

				if (&element != &value.back())
					Separator();

				PopID();
			}

			EndTable();
		}
		//EndChild();

		return returnVal;
	}
}
#endif

///** ARRAY */
//#if defined(GLAS_IMGUI_ARRAY) || defined(_ARRAY_)
//namespace ImGui
//{
//	template <typename T, size_t size>
//	bool GlasAuto(const char* /*label*/, std::array<T, size>& /*value*/)
//	{
//		//GlasAutoContainer(stream, value);
//	}
//}
//#endif

///** DEQUE */
//#if defined(GLAS_IMGUI_DEQUE) || defined(_DEQUE_)
//namespace ImGui
//{
//	template <typename T>
//	bool GlasAuto(std::deque<T>& value)
//	{
//		GlasAutoContainer(stream, value);
//	}
//}
//#endif
//
///** FORWARD LIST */
//#if defined(GLAS_IMGUI_FORWARD_LIST) || defined(_FORWARD_LIST_)
//namespace ImGui
//{
//	template <typename T>
//	bool GlasAuto(std::forward_list<T>& value)
//	{
//		GlasAutoContainer(stream, value);
//	}
//}
//#endif
//
///** LIST */
//#if defined(GLAS_IMGUI_LIST) || defined(_LIST_)
//namespace ImGui
//{
//	template <typename T>
//	bool GlasAuto(std::list<T>& value)
//	{
//		GlasAutoContainer(stream, value);
//	}
//}
//#endif
//
///** SET & MULTI SET */
//#if defined(GLAS_IMGUI_SET) || defined(_SET_)
//#include <set>
//namespace ImGui
//{
//	template <typename T>
//	bool GlasAuto(std::set<T>& value)
//	{
//		GlasAutoContainer(stream, value);
//	}
//
//	template <typename T>
//	bool GlasAuto(std::multiset<T>& value)
//	{
//		GlasAutoContainer(stream, value);
//	}
//}
//#endif
//
///** UNORDERED SET */
//#if defined(GLAS_IMGUI_UNORDERED_SET) || defined(_UNORDERED_SET_)
//#include <unordered_set>
//namespace ImGui
//{
//	template <typename T>
//	bool GlasAuto(std::unordered_set<T>& value)
//	{
//		GlasAutoContainer(stream, value);
//	}
//	
//	template <typename T>
//	bool GlasAuto(std::unordered_multiset<T>& value)
//	{
//		GlasAutoContainer(stream, value);
//	}
//}
//#endif
//
///** MAP */
//#if defined(GLAS_IMGUI_MAP) || defined(_MAP_)
//#include <map>
//namespace ImGui
//{
//	template <typename Key, typename Value>
//	bool GlasAuto(std::map<Key, Value>& value)
//	{
//		GlasAutoMapContainer(stream, value);
//	}
//
//	template <typename Key, typename Value>
//	bool GlasAuto(std::multimap<Key, Value>& value)
//	{
//		GlasAutoMapContainer(stream, value);
//	}
//}
//#endif
//
///** UNORDERED MAP */
//#if defined(GLAS_IMGUI_MAP) || defined(_UNORDERED_MAP_)
//#include <unordered_map>
//namespace ImGui
//{
//	template <typename Key, typename Value>
//	bool GlasAuto(std::unordered_map<Key, Value>& value)
//	{
//		GlasAutoMapContainer(stream, value);
//	}
//
//	template <typename Key, typename Value>
//	bool GlasAuto(std::unordered_multimap<Key, Value>& value)
//	{
//		GlasAutoMapContainer(stream, value);
//	}
//}
//#endif

///** MEMORY */
//#if defined(GLAS_IMGUI_MEMORY) || defined(_MEMORY_)
//#include <memory>
//namespace ImGui
//{
//	template <typename T>
//	bool GlasAuto(std::unique_ptr<T>& value)
//	{
//		stream << static_cast<bool>(value);
//		if (value)
//		{
//			stream << ": ";
//			GlasAutoType(stream, *value);
//		}
//	}
//}
//#endif
//
///** OPTIONAL */
//#if defined(GLAS_IMGUI_OPTIONAL) || defined(_OPTIONAL_)
//#include <optional>
//namespace ImGui
//{
//	template <typename T>
//	bool GlasAuto(std::optional<T>& value)
//	{
//		stream << value.has_value();
//		if (value.has_value())
//		{
//			stream << ": ";
//			GlasAutoType(stream, *value);
//		}
//	}
//
//}
//#endif

/** UTILITY */
#if defined(GLAS_IMGUI_UTILITY) || defined(_UTILITY_)
namespace ImGui
{
	template <typename T1, typename T2>
	bool GlasAuto(const char* label, std::pair<T1, T2>& value)
	{
		ImGui::TextUnformatted(label);
		GlasAuto("", value.first);
		ImGui::SameLine();
		GlasAuto("", value.second);
	}

	template <size_t Index, typename... Ts>
	bool GlasAuto(std::tuple<Ts...>& value)
	{
		return GlasAuto("", std::get<Index>(value));
	}

	template <typename... Ts>
	bool GlasAuto(const char* label, std::tuple<Ts...>& value)
	{
		bool returnVal{};
		ImGui::TextUnformatted(label);
		ImGui::BeginChild(&value, {}, true, ImGuiWindowFlags_AlwaysAutoResize);
		returnVal |= GlasAuto<0, Ts...>(value);
		ImGui::EndChild();
		return returnVal;
	}
}
#endif

/** FUNDAMENTAL TYPES*/
namespace ImGui
{
	template <typename T>
	constexpr ImGuiDataType GetDataType()
	{
		static_assert(std::is_integral_v<T>);
		if constexpr (std::is_signed_v<T>)
		{
			switch (sizeof(T))
			{
			case 1: return ImGuiDataType_S8;
			case 2: return ImGuiDataType_S16;
			case 4: return ImGuiDataType_S32;
			case 8: return ImGuiDataType_S64;
			}
		}
		else
		{
			switch (sizeof(T))
			{
			case 1: return ImGuiDataType_U8;
			case 2: return ImGuiDataType_U16;
			case 4: return ImGuiDataType_U32;
			case 8: return ImGuiDataType_U64;
			}
		}
		return 0;
	}

	template <typename T>
	bool GlasAuto(const char* label, T& value) requires std::is_integral_v<T>
	{
		return ImGui::DragScalar(label, GetDataType<T>(), &value);
	}

	bool GlasAuto(const char* label, float& value)
	{
		return ImGui::DragScalar(label, ImGuiDataType_Float, &value);
	}

	bool GlasAuto(const char* label, double& value)
	{
		return ImGui::DragScalar(label, ImGuiDataType_Double, &value);
	}

	bool GlasAuto(const char* label, bool& value)
	{
		return ImGui::Checkbox(label, &value);
	}
}

#ifdef GLAS_STORAGE
namespace ImGui
{
	inline bool GlasAuto(const char* label, glas::Storage::TypeStorage& value)
	{
		bool returnVal{};

		glas::TypeId type = value.GetType();

		if (GlasAuto("Type", type))
		{
			value = glas::Storage::TypeStorage(type);
		}

		TextUnformatted("Value:");

		Indent();

		if (value.GetData())
		{
			returnVal |= type.GetInfo().ImGuiRenderer(label, value.GetData());
		}

		Unindent();

		return returnVal;
	}

	//inline bool GlasAuto(const char* label, glas::Storage::TypeTuple& value)
	//{
	//	stream << "{ ";
	//
	//	stream << value.GetSize();
	//
	//	stream << " {";
	//	const auto variableIds = value.GetVariableIds();
	//
	//	for (size_t i{}; i < variableIds.size(); ++i)
	//	{
	//		if (i != 0)
	//			stream << ',';
	//
	//		stream << variableIds[i];
	//	}
	//
	//	stream << " },{";
	//
	//	size_t variableStreamed{};
	//	for (size_t i{}; i < variableIds.size(); ++i)
	//	{
	//		if (!variableIds[i].IsRefOrPointer())
	//		{
	//			if (variableStreamed++ != 0)
	//				stream << ',';
	//
	//			GlasAuto(stream, value.GetVoid(i), variableIds[i].GetTypeId());
	//		}
	//	}
	//
	//	stream << " }}";
	//}
#endif

}

/** DEFAULT*/
namespace ImGui
{
	bool GlasAuto(const char* label, void* data, glas::TypeId type)
	{
		bool returnValue{};
		bool emptyLabel = label[0] == '\0';

		if (emptyLabel || TreeNode(label))
		{
			auto& info = GetTypeInfo(type);

			auto& members = info.Members;
			for (auto& member : members)
			{
				if (!member.VariableId.IsRefOrPointer())
				{
					returnValue |= member.VariableId.GetTypeId().GetInfo().ImGuiRenderer(std::string(member.Name).c_str(), VoidOffset(data, member.Offset));
				}
			}
			if (!emptyLabel)
				TreePop();
		}

		return returnValue;
	}

	template <typename T>
	bool GlasAuto(const char* label, T& value)
	{
		return GlasAuto(label, &value, glas::TypeId::Create<T>());
	}
}