#pragma once

#include "sqlite3.h"
#include <string>

namespace Jump
{

class LocalDatabase
{
public:
    static LocalDatabase& Instance();

    void Init();
    void Close();
private:
    // Disallow
    LocalDatabase() {}
    LocalDatabase(const LocalDatabase&) {}
    LocalDatabase& operator=(const LocalDatabase&) {}

    void CreateTableMaps();
    void CreateTableUsers();
    void CreateTableMapTimes();

    void AddNewMap(const std::string& mapname);

    sqlite3* m_Handle = nullptr;
};

}