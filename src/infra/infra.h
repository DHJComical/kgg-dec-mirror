#pragma once

#include <filesystem>
#include <unordered_map>

#include "sqlite_base.h"
#include "sqlite_fn.h"

namespace Infra {

typedef std::unordered_map<std::string, std::string> kgm_ekey_db_t;
extern bool g_init_sqlite_ok;

class SqliteDB {
   public:
    explicit SqliteDB(const std::filesystem::path& infra_dll_path);
    bool Open(const std::filesystem::path& db_path, std::string_view key);
    void Close();
    [[nodiscard]] bool IsInfraOk() const { return ok_; }
    [[nodiscard]] bool IsOpen() const { return db_ != nullptr; }

   private:
    bool InitInfraDll(const std::filesystem::path& infra_dll_path);
    bool ok_{false};

   protected:
    void FreeInfraDll();

    void* infra_{nullptr};
    sqlite3_open_v2_t sqlite3_open_v2_{nullptr};
    sqlite3_key_t sqlite3_key_{nullptr};
    sqlite3_prepare_v2_t sqlite3_prepare_v2_{nullptr};
    sqlite3_step_t sqlite3_step_{nullptr};
    sqlite3_column_text_t sqlite3_column_text_{nullptr};
    sqlite3_close_v2_t sqlite3_close_v2_{nullptr};
    sqlite3_finalize_t sqlite3_finalize_{nullptr};
    sqlite3* db_{nullptr};
};

class KugouDb : public SqliteDB {
   public:
    explicit KugouDb(const std::filesystem::path& infra_dll_path, const std::filesystem::path& db_path);
    ~KugouDb();

    bool Open(const std::filesystem::path& db_path);
    kgm_ekey_db_t dump_ekey(int& error);
};

}  // namespace Infra
