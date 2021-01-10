#include "jump_menu.h"
#include <sstream>
#include "jump_scores.h"
#include <iomanip>
#include <filesystem>
#include "jump_utils.h"

namespace Jump
{
    void BestTimesScoreboardMessage(edict_t* client)
	{
		std::vector<user_time_record> highscores;
		int players = 0;
		int total_completions = 0;
		GetHighscoresForMap(level.mapname, highscores, players, total_completions);

		// Sideways arrow symbol
		char symbol_arrow = 13;

		std::stringstream ss;
		ss << "xv 0 yv 0 string2 \"No   Player                       Date \" ";

		for (int i = 0; i < MAX_HIGHSCORES; ++i)
		{
			if (i < highscores.size())
			{
				std::string username = RemoveFileExtension(RemovePathFromFilename(highscores[i].filepath));
				std::string time = GetCompletionTimeDisplayString(highscores[i].time_ms);
				std::string date = highscores[i].date.substr(0, highscores[i].date.find_first_of(' '));

				// Position the text vertically, make it a white string
				ss << "yv " << (i * 10) + 16 << " string \"";

				// Show the highscore number
				ss << std::setw(2) << std::right << i + 1;

				// Show the arrow symbol (indicates a replay is available for this time)
				// TODO: can remove this
				ss << symbol_arrow;

				// Show the fresh time symbol or not
				bool fresh_time = jump_server.fresh_times.find(AsciiToLower(username)) != jump_server.fresh_times.end();
				if (fresh_time)
				{
					ss << " *";
				}
				else
				{
					ss << "  ";
				}

				// Show the username
				ss << std::setw(16) << std::left << username;

				// Show the completion time
				ss << std::setw(11) << std::right << time;

				// Show the date of completion
				ss << "  " << date << "\" ";
			}
			else
			{
				// No time set for this position yet
				ss << "yv " << (i * 10) + 16 << " string \"" << std::setw(2) << i + 1 << " \" ";
			}
		}
		ss << "yv " << (MAX_HIGHSCORES * 10) + 24 << " string \"    " << players;
		ss << " players completed map " << total_completions << " times\" "; // TODO: "player" if only one player has completed
		gi.WriteByte(svc_layout);
		assert(ss.str().size() < 1024);
		gi.WriteString(const_cast<char*>(ss.str().c_str()));
		gi.unicast(client, true);
	}

	void ActiveClientsScoreboardMessage(edict_t* ent)
	{
		// There is a hard limit of 1024 bytes for the scoreboard message, so we can only show so many players.
		// TODO: split into multiple menus or implement a separate shorter one
		const int MAX_PLAYERS_DISPLAYED = 14;

		std::vector<edict_t*> clients_hard;
		std::vector<edict_t*> clients_easy;
		std::vector<edict_t*> clients_spec;

		for (int i = 0; i < game.maxclients; ++i)
		{
			edict_t* ent = g_edicts + 1 + i;
			if (!ent->inuse)
			{
				continue;
			}
			if (ent->client->jumpdata->team == TEAM_HARD)
			{
				clients_hard.push_back(ent);
			}
			else if (ent->client->jumpdata->team == TEAM_EASY)
			{
				clients_easy.push_back(ent);
			}
			else
			{
				clients_spec.push_back(ent);
			}
		}

		std::stringstream ss;
		ss << "xv 0 yv 0 string2 \"Ping Player                Best Comp Maps     %  Team\" ";
		ss << "yv 16 ";

		int yv_offset = 16;
		int players_added = 0;
		//for (size_t i = 0; i < clients_hard.size(); ++i)
		for (size_t i = 0; i < 4; ++i)
		{
			players_added++;
			if (players_added >= MAX_PLAYERS_DISPLAYED)
			{
				break;
			}
			//edict_t* player = clients_hard[i];

			//std::string ping = std::to_string(player->client->ping);
			//std::string username = player->client->pers.netname;
			//if (player == ent)
			//{
			//	ping = GetGreenConsoleText(ping);
			//	username = GetGreenConsoleText(username);
			//}

			int yv = yv_offset + (i * 10);
			ss << "yv " << yv << " string \"";
			//ss << std::right << std::setw(4) << ping << " ";
			//ss << std::left << std::setw(15) << username << " ";
			ss << std::right << std::setw(4) << i * 20 << " ";
			ss << std::left << std::setw(15) << "SomeName" << " ";
			ss << std::right << std::setw(10) << "56.023" << " "; // TODO completion time
			ss << std::right << std::setw(4) << "23" << " "; // TODO completions
			ss << std::right << std::setw(4) << "2760" << "  "; // TODO maps
			ss << std::right << std::setw(4) << "97.1" << "  "; // TODO percentage
			ss << "Hard" << "\" ";
		}

		// If there are no hard players, shift team easy players to the top, else add padding between the groups.
		if (players_added > 0)
		{
			yv_offset = 16 + (players_added * 10) + 10;
		}
		else
		{
			yv_offset = 16;
		}

		//for (size_t i = 0; i < clients_easy.size(); ++i)
		for (size_t i = 0; i < 4; ++i)
		{
			players_added++;
			if (players_added >= MAX_PLAYERS_DISPLAYED)
			{
				break;
			}
			//edict_t* player = clients_easy[i];

			//std::string ping = std::to_string(player->client->ping);
			//std::string username = player->client->pers.netname;
			//if (player == ent)
			//{
			//	ping = GetGreenConsoleText(ping);
			//	username = GetGreenConsoleText(username);
			//}

			int yv = yv_offset + (i * 10);
			ss << "yv " << yv << " string \"";
			//ss << std::right << std::setw(4) << ping << " ";
			//ss << std::left << std::setw(15) << username << " ";
			ss << std::right << std::setw(4) << i * 20 << " ";
			ss << std::left << std::setw(15) << "SomeName" << " ";
			ss << std::right << std::setw(10) << "" << " ";
			ss << std::right << std::setw(4) << "" << " ";
			ss << std::right << std::setw(4) << "" << "  ";
			ss << std::right << std::setw(4) << "" << "  ";
			ss << "Easy" << "\" ";
		}

		if (players_added == 0)
		{
			yv_offset = 16 + 16 + 10;	// only spectators, we want empty space to show that there are no players
		}
		else
		{
			yv_offset = 16 + 10 + (players_added * 10);
			//if (!clients_hard.empty() && !clients_easy.empty())
			{
				yv_offset += 10;	// add in the space between the hard and easy team sections
			}
		}

		if (players_added < MAX_PLAYERS_DISPLAYED)
		{
			ss << "yv " << yv_offset << " string2 \"Spectators\" ";
		}

		yv_offset += 8;
		//for (size_t i = 0; i < clients_spec.size(); ++i)
		for (size_t i = 0; i < 3; ++i)
		{
			players_added++;
			if (players_added >= MAX_PLAYERS_DISPLAYED)
			{
				break;
			}
			//edict_t* player = clients_spec[i];

			//std::string ping = std::to_string(player->client->ping);
			//std::string username = player->client->pers.netname;
			//if (player == ent)
			//{
			//	ping = GetGreenConsoleText(ping);
			//	username = GetGreenConsoleText(username);
			//}

			int yv = yv_offset + (i * 8);
			ss << "yv " << yv << " string \"";
			//ss << std::right << std::setw(4) << ping << " ";
			//ss << std::left << std::setw(15) << username << " ";
			ss << std::right << std::setw(4) << i * 20 << " ";
			ss << std::left << std::setw(15) << "SomeName" << " ";
			// TODO: idle, who you are speccing, what replay you are watching
			ss << "\" ";
		}

		assert(ss.str().size() < 1024);
		std::string str = ss.str(); // TODO remove
		gi.WriteByte(svc_layout);
		gi.WriteString(const_cast<char*>(ss.str().c_str()));
		gi.unicast(ent, true);
	}

	void ExtendedActiveClientsScoreboardMessage(edict_t* ent)
	{
		// There is a hard limit of 1024 bytes for the scoreboard message, so we can only show so many players.
		const int MAX_PLAYERS_DISPLAYED = 23;

		std::vector<edict_t*> clients_hard;
		std::vector<edict_t*> clients_easy;
		std::vector<edict_t*> clients_spec;

		for (int i = 0; i < game.maxclients; ++i)
		{
			edict_t* ent = g_edicts + 1 + i;
			if (!ent->inuse)
			{
				continue;
			}
			if (ent->client->jumpdata->team == TEAM_HARD)
			{
				clients_hard.push_back(ent);
			}
			else if (ent->client->jumpdata->team == TEAM_EASY)
			{
				clients_easy.push_back(ent);
			}
			else
			{
				clients_spec.push_back(ent);
			}
		}

		std::stringstream ss;
		ss << "xv 0 ";							// 5
		ss << "yv 0 string2 \"Team: Hard\" ";	// 26

		int yv_offset = 10;
		int players_added = 0;
		//for (size_t i = 0; i < clients_hard.size(); ++i)
		for (size_t i = 0; i < 10; ++i)
		{
			players_added++;
			if (players_added >= MAX_PLAYERS_DISPLAYED)
			{
				break;
			}
			//edict_t* player = clients_hard[i];

			//std::string ping = std::to_string(player->client->ping);
			//std::string username = player->client->pers.netname;
			//if (player == ent)
			//{
			//	ping = GetGreenConsoleText(ping);
			//	username = GetGreenConsoleText(username);
			//}

			int yv = yv_offset + (i * 8);
			ss << "yv " << yv << " string \"";	// 15
			//ss << std::right << std::setw(4) << ping << " ";
			//ss << std::left << std::setw(15) << username << " ";
			ss << std::right << std::setw(4) << i * 20 << " ";	// 5
			ss << std::left << std::setw(15) << "SomeName" << " ";	// 16
			ss << std::right << std::setw(10) << "56.023" << "\" "; // 12, TODO completion time
		}

		yv_offset = 10 + (players_added * 8) + 8;
		ss << "yv " << yv_offset << " string2 \"Team: Easy\" ";	// 28
		yv_offset += 10;

		//for (size_t i = 0; i < clients_easy.size(); ++i)
		for (size_t i = 0; i < 10; ++i)
		{
			players_added++;
			if (players_added >= MAX_PLAYERS_DISPLAYED)
			{
				break;
			}
			//edict_t* player = clients_easy[i];

			//std::string ping = std::to_string(player->client->ping);
			//std::string username = player->client->pers.netname;
			//if (player == ent)
			//{
			//	ping = GetGreenConsoleText(ping);
			//	username = GetGreenConsoleText(username);
			//}

			int yv = yv_offset + (i * 8);
			ss << "yv " << yv << " string \"";	// 15
			//ss << std::right << std::setw(4) << ping << " ";
			//ss << std::left << std::setw(15) << username << " ";
			ss << std::right << std::setw(4) << i * 20 << " ";	// 5
			ss << std::left << std::setw(15) << "SomeName" << "\" "; // 17
		}

		yv_offset = 10 + (players_added * 8) + 8 + 8 + 8;
		ss << "yv " << yv_offset << " string2 \"Spectators\" ";	// 28
		yv_offset += 10;

		//for (size_t i = 0; i < clients_spec.size(); ++i)
		for (size_t i = 0; i < 10; ++i)
		{
			players_added++;
			if (players_added >= MAX_PLAYERS_DISPLAYED)
			{
				break;
			}
			//edict_t* player = clients_spec[i];

			//std::string ping = std::to_string(player->client->ping);
			//std::string username = player->client->pers.netname;
			//if (player == ent)
			//{
			//	ping = GetGreenConsoleText(ping);
			//	username = GetGreenConsoleText(username);
			//}

			int yv = yv_offset + (i * 8);
			ss << "yv " << yv << " string \"";	// 15
			//ss << std::right << std::setw(4) << ping << " ";
			//ss << std::left << std::setw(15) << username << " ";
			ss << std::right << std::setw(4) << i * 20 << " ";	// 5
			ss << std::left << std::setw(15) << "SomeName" << "\" "; // 17
		}

		assert(ss.str().size() < 1024);
		std::string str = ss.str(); // TODO remove
		gi.WriteByte(svc_layout);
		gi.WriteString(const_cast<char*>(ss.str().c_str()));
		gi.unicast(ent, true);
	}
}