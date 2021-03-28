#pragma once

#include "sqlite3.h"
#include <string>
#include <vector>

namespace Jump
{

class LocalDatabase
{
public:
    static LocalDatabase& Instance();

    void Init();
    void Close();

    void MigrateAll();
private:
    // Disallow
    LocalDatabase() {}
    LocalDatabase(const LocalDatabase&) {}
    LocalDatabase& operator=(const LocalDatabase&) {}

    void CreateTableMaps();
    void CreateTableUsers();
    void CreateTableMapTimes();

    void AddMap(const std::string& mapname);
    void AddMapList(const std::vector<std::string>& maps);

    void AddUserOrUpdateSeen(const std::string& username);

    // Migrate from old highscores
    void ClearAllTables();
    void MigrateUsers(const std::string& userFile);
    void MigrateMaplist(const std::string& maplistFile);
    void MigrateMapTimes(const std::string& folder);
    void MigrateMSets(const std::string& folder);
    void MigrateEnts(const std::string& folder);
    void MigrateReplays(const std::string& folder);

    sqlite3* m_db = nullptr;
};

}