#pragma once

#include <nlohmann/json.hpp>

#include <string_view>

namespace tf2_bot_detector
{
	template<typename T>
	static bool try_get_to(const nlohmann::json& j, const std::string_view& name, T& value)
	{
		if (auto found = j.find(name); found != j.end())
		{
			found->get_to(value);
			return true;
		}

		return false;
	}
}
