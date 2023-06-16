#pragma once

#include <cassert>

#define GLAS_IMGUI_STRING
#define GLAS_IMGUI_STRING_VIEW
#define GLAS_IMGUI_VECTOR
#define GLAS_IMGUI_ARRAY
#define GLAS_IMGUI_DEQUE
#define GLAS_IMGUI_FORWARD_LIST
#define GLAS_IMGUI_LIST
#define GLAS_IMGUI_SET
#define GLAS_IMGUI_UNORDERED_SET
#define GLAS_IMGUI_MAP
#define GLAS_IMGUI_MEMORY
#define GLAS_IMGUI_OPTIONAL
#define GLAS_IMGUI_UTILITY

namespace glas
{
	struct TypeInfo;
}

namespace ImGui
{
	template <typename T>
	constexpr void FillInfo(glas::TypeInfo& info);
}

/** GLAS */
namespace ImGui
{
	inline bool GlasAuto(const char* label, glas::TypeId& type);
	//inline bool GlasAuto(const char* label, glas::VariableId& type);
	//inline bool GlasAuto(const char* label, glas::FunctionId& type);
}

/** STRING */
#if defined(GLAS_IMGUI_STRING) || defined(_STRING_)
#include <string>
namespace ImGui
{
	inline bool GlasAuto(const char* label, std::string& string);
}
#endif

/** STRING VIEW */
#if defined(GLAS_IMGUI_STRING_VIEW) || defined(_STRING_VIEW_)
#include <string_view>
namespace ImGui
{
	inline bool GlasAuto(const char* label, std::string_view& string);
}
#endif

/** VECTOR */
#if defined(GLAS_IMGUI_VECTOR) || defined(_VECTOR_)
#include <vector>
namespace ImGui
{
	template <typename T>
	bool GlasAuto(const char* label, std::vector<T>& value);
}
#endif

///** ARRAY */
//#if defined(GLAS_IMGUI_ARRAY) || defined(_ARRAY_)
//#include <array>
//namespace ImGui
//{
//	template <typename T, size_t size>
//	bool GlasAuto(const char* label, std::array<T, size>& value);
//}
//#endif

///** DEQUE */
//#if defined(GLAS_IMGUI_DEQUE) || defined(_DEQUE_)
//#include <deque>
//namespace ImGui
//{
//	template <typename T>
//	bool GlasAuto(const char* label, std::deque<T>& value);
//}
//#endif
//
///** FORWARD LIST */
//#if defined(GLAS_IMGUI_FORWARD_LIST) || defined(_FORWARD_LIST_)
//#include <forward_list>
//namespace ImGui
//{
//	template <typename T>
//	bool GlasAuto(const char* label, std::forward_list<T>& value);
//}
//#endif
//
///** LIST */
//#if defined(GLAS_IMGUI_LIST) || defined(_LIST_)
//#include <list>
//namespace ImGui
//{
//	template <typename T>
//	bool GlasAuto(const char* label, std::list<T>& value);
//}
//#endif
//
///** SET & MULTI SET */
//#if defined(GLAS_IMGUI_SET) || defined(_SET_)
//#include <set>
//namespace ImGui
//{
//	template <typename T>
//	bool GlasAuto(const char* label, std::set<T>& value);
//
//	template <typename T>
//	bool GlasAuto(const char* label, std::multiset<T>& value);
//}
//#endif
//
///** UNORDERED SET */
//#if defined(GLAS_IMGUI_UNORDERED_SET) || defined(_UNORDERED_SET_)
//#include <unordered_set>
//namespace ImGui
//{
//	template <typename T>
//	bool GlasAuto(const char* label, std::unordered_set<T>& value);
//
//	template <typename T>
//	bool GlasAuto(const char* label, std::unordered_multiset<T>& value);
//}
//#endif
//
///** MAP */
//#if defined(GLAS_IMGUI_MAP) || defined(_MAP_)
//#include <map>
//namespace ImGui
//{
//	template <typename Key, typename Value>
//	bool GlasAuto(const char* label, std::map<Key, Value>& value);
//
//	template <typename Key, typename Value>
//	bool GlasAuto(const char* label, std::multimap<Key, Value>& value);
//}
//#endif
//
///** UNORDERED MAP */
//#if defined(GLAS_IMGUI_MAP) || defined(_UNORDERED_MAP_)
//#include <unordered_map>
//namespace ImGui
//{
//	template <typename Key, typename Value>
//	bool GlasAuto(const char* label, std::unordered_map<Key, Value>& value);
//
//	template <typename Key, typename Value>
//	bool GlasAuto(const char* label, std::unordered_multimap<Key, Value>& value);
//}
//#endif

///** MEMORY */
//#if defined(GLAS_IMGUI_MEMORY) || defined(_MEMORY_)
//#include <memory>
//namespace ImGui
//{
//	template <typename T>
//	bool GlasAuto(const char* label, std::unique_ptr<T>& value);
//}
//#endif
//
///** OPTIONAL */
//#if defined(GLAS_IMGUI_OPTIONAL) || defined(_OPTIONAL_)
//#include <optional>
//namespace ImGui
//{
//	template <typename T>
//	bool GlasAuto(const char* label, std::optional<T>& value);
//}
//#endif

/** PAIR */
#if defined(GLAS_IMGUI_UTILITY) || defined(_UTILITY_)
#include <utility>
namespace ImGui
{
	template <typename T1, typename T2>
	bool GlasAuto(const char* label, std::pair<T1,T2>& value);

	template <typename... Ts>
	bool GlasAuto(const char* label, std::tuple<Ts...>& value);
}
#endif

namespace ImGui
{
	template <typename T>
	bool GlasAuto(const char* label, T& value) requires std::is_integral_v<T>;
	
	bool GlasAuto(const char* label, float& value);
	bool GlasAuto(const char* label, double& value);
	bool GlasAuto(const char* label, bool& value);
}

#ifdef GLAS_STORAGE
#include "../storage/glas_storage_config.h"
namespace ImGui
{
	inline bool GlasAuto(const char* label, glas::Storage::TypeStorage& value);

	inline bool GlasAuto(const char* label, glas::Storage::TypeTuple& value);
}
#endif

/** DEFAULT*/
namespace ImGui
{
	template <typename T>
	bool GlasAuto(const char* label, T& value);
}