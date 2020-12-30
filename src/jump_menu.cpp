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
		if (client->client->menu)
		{
		    PMenu_Close(client);
		}
		client->client->showscores = true;

		std::vector<user_time_record> highscores;
		int total_completions = 0;
		GetHighscoresForMap(level.mapname, highscores, total_completions);

		// TODO fresh time

		// Sideways arrow symbol
		char symbol_arrow = 13;

		std::stringstream ss;
		ss << "xv 0 yv 0 string2 \"No   Player                       Date \" ";

		for (int i = 0; i < MAX_HIGHSCORES; ++i)
		{
			if (i < highscores.size())
			{
				// There is a time set for this highscore position
				std::string username = RemoveFileExtension(RemovePathFromFilename(highscores[i].filepath));
				std::string time = GetCompletionTimeDisplayString(highscores[i].time_ms);
				std::string date = highscores[i].date.substr(0, highscores[i].date.find_first_of(' '));

				// TODO fresh time!!
				ss << "yv " << (i * 10) + 16 << " string \"" << std::setw(2) << std::right << i + 1;
				ss << symbol_arrow << "  " << std::setw(16) << std::left << username;
				ss << std::setw(11) << std::right << time;
				ss << "  " << date << "\" ";
			}
			else
			{
				// No time set for this position yet
				ss << "yv " << (i * 10) + 16 << " string \"" << std::setw(2) << i + 1 << " \" ";
			}
		}
		ss << "yv " << (MAX_HIGHSCORES * 10) + 24 << " string \"    " << highscores.size();
		ss << " players completed map " << total_completions << " times\" ";
		gi.WriteByte(svc_layout);
		gi.WriteString(const_cast<char*>(ss.str().c_str()));
		//gi.WriteString(string);
		gi.unicast(client, true);
	}
}