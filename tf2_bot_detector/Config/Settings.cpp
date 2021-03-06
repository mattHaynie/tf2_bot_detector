#include "Settings.h"
#include "IPlayer.h"
#include "JSONHelpers.h"
#include "Log.h"
#include "PlayerListJSON.h"

#include <mh/text/case_insensitive_string.hpp>
#include <mh/text/string_insertion.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <iomanip>
#include <regex>

using namespace tf2_bot_detector;
using namespace std::string_literals;
using namespace std::string_view_literals;

static std::filesystem::path s_SettingsPath("cfg/settings.json");

namespace tf2_bot_detector
{
	void to_json(nlohmann::json& j, const Settings::Theme::Colors& d)
	{
		j =
		{
			{ "scoreboard_cheater", d.m_ScoreboardCheater },
			{ "scoreboard_suspicious", d.m_ScoreboardSuspicious },
			{ "scoreboard_exploiter", d.m_ScoreboardExploiter },
			{ "scoreboard_racism", d.m_ScoreboardRacist },
			{ "scoreboard_you", d.m_ScoreboardYou },
			{ "scoreboard_connecting", d.m_ScoreboardConnecting },
			{ "friendly_team", d.m_FriendlyTeam },
			{ "enemy_team", d.m_EnemyTeam },
		};
	}

	void to_json(nlohmann::json& j, const Settings::Theme& d)
	{
		j =
		{
			{ "colors", d.m_Colors }
		};
	}

	void from_json(const nlohmann::json& j, Settings::Theme::Colors& d)
	{
		try_get_to(j, "scoreboard_cheater", d.m_ScoreboardCheater);
		try_get_to(j, "scoreboard_suspicious", d.m_ScoreboardSuspicious);
		try_get_to(j, "scoreboard_exploiter", d.m_ScoreboardExploiter);
		try_get_to(j, "scoreboard_racism", d.m_ScoreboardRacist);
		try_get_to(j, "scoreboard_you", d.m_ScoreboardYou);
		try_get_to(j, "scoreboard_connecting", d.m_ScoreboardConnecting);
		try_get_to(j, "friendly_team", d.m_FriendlyTeam);
		try_get_to(j, "enemy_team", d.m_EnemyTeam);
	}

	void from_json(const nlohmann::json& j, Settings::Theme& d)
	{
		d.m_Colors = j.at("colors");
	}
}

Settings::Settings()
{
	// Immediately load and resave to normalize any formatting
	LoadFile();
	SaveFile();
}

bool Settings::LoadFile()
{
	nlohmann::json json;
	{
		std::ifstream file(s_SettingsPath);
		if (!file.good())
		{
			Log(std::string(__FUNCTION__ ": Failed to open ") << s_SettingsPath, { 1, 0.5, 0, 1 });
			return false;
		}

		try
		{
			file >> json;
		}
		catch (const nlohmann::json::exception& e)
		{
			Log(std::string(__FUNCTION__ ": Failed to parse JSON from ") << s_SettingsPath << ": " << e.what(), { 1, 0.25, 0, 1 });
			return false;
		}
	}

	if (auto found = json.find("general"); found != json.end())
	{
		try_get_to(*found, "local_steamid", m_LocalSteamID);
		try_get_to(*found, "sleep_when_unfocused", m_SleepWhenUnfocused);
		try_get_to(*found, "auto_temp_mute", m_AutoTempMute);

		if (auto foundDir = found->find("tf_game_dir"); foundDir != found->end())
			m_TFDir = foundDir->get<std::string_view>();
	}

	try_get_to(json, "theme", m_Theme);

	return true;
}

bool Settings::SaveFile() const
{
	nlohmann::json json =
	{
		{ "$schema", "./schema/settings.schema.json" },
		{ "theme", m_Theme },
		{ "general",
			{
				{ "tf_game_dir", m_TFDir.string() },
				{ "sleep_when_unfocused", m_SleepWhenUnfocused },
				{ "auto_temp_mute", m_AutoTempMute },
			}
		}
	};

	if (m_LocalSteamID.IsValid())
		json["general"]["local_steamid"] = m_LocalSteamID;

	// Make sure we successfully serialize BEFORE we destroy our file
	auto jsonString = json.dump(1, '\t', true);
	{
		std::filesystem::create_directories(std::filesystem::path(s_SettingsPath).remove_filename());
		std::ofstream file(s_SettingsPath);
		if (!file.good())
		{
			LogError(std::string(__FUNCTION__ ": Failed to open settings file for writing: ") << s_SettingsPath);
			return false;
		}

		file << jsonString << '\n';
		if (!file.good())
		{
			LogError(std::string(__FUNCTION__ ": Failed to write settings to ") << s_SettingsPath);
			return false;
		}
	}

	return true;
}
