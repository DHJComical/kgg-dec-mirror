#include "infra.h"
#include "sqlite_error.h"
#include "sqlite_fn.h"

#include <windows.h>

namespace Infra {

SqliteDB::SqliteDB(const std::filesystem::path& infra_dll_path) {
    ok_ = InitInfraDll(infra_dll_path);
}

bool SqliteDB::Open(const std::filesystem::path& db_path, std::string_view key) {
    if (infra_ == nullptr) {
        return false;
    }

    auto db_path_u8 = db_path.generic_u8string();
    int rc = sqlite3_open_v2_(reinterpret_cast<const char*>(db_path_u8.c_str()), &db_, SQLITE_OPEN_READONLY, nullptr);
    if (rc != SQLITE_OK) {
        return false;
    }

    if (!key.empty()) {
        rc = sqlite3_key_(db_, key.data(), static_cast<int>(key.size()));
        if (rc != SQLITE_OK) {
            sqlite3_close_v2_(db_);
            db_ = nullptr;
            return false;
        }
    }

    return true;
}

void SqliteDB::Close() {
    if (db_) {
        sqlite3_close_v2_(db_);
        db_ = nullptr;
    }
}
void SqliteDB::FreeInfraDll() {
    if (infra_ != nullptr) {
        FreeLibrary(reinterpret_cast<HMODULE>(infra_));
        infra_ = nullptr;
    }

    sqlite3_open_v2_ = nullptr;
    sqlite3_key_ = nullptr;
    sqlite3_prepare_v2_ = nullptr;
    sqlite3_step_ = nullptr;
    sqlite3_column_text_ = nullptr;
    sqlite3_close_v2_ = nullptr;
    sqlite3_finalize_ = nullptr;
}

bool SqliteDB::InitInfraDll(const std::filesystem::path& infra_dll_path) {
    auto path_unicode = infra_dll_path.wstring();
    HMODULE hMod = LoadLibraryW(path_unicode.c_str());
    infra_ = hMod;
    if (hMod == nullptr) {
        return false;
    }

    sqlite3_open_v2_ = reinterpret_cast<sqlite3_open_v2_t>(GetProcAddress(hMod, "sqlite3_open_v2"));
    sqlite3_key_ = reinterpret_cast<sqlite3_key_t>(GetProcAddress(hMod, "sqlite3_key"));
    sqlite3_prepare_v2_ = reinterpret_cast<sqlite3_prepare_v2_t>(GetProcAddress(hMod, "sqlite3_prepare_v2"));
    sqlite3_step_ = reinterpret_cast<sqlite3_step_t>(GetProcAddress(hMod, "sqlite3_step"));
    sqlite3_column_text_ = reinterpret_cast<sqlite3_column_text_t>(GetProcAddress(hMod, "sqlite3_column_text"));
    sqlite3_close_v2_ = reinterpret_cast<sqlite3_close_v2_t>(GetProcAddress(hMod, "sqlite3_close_v2"));
    sqlite3_finalize_ = reinterpret_cast<sqlite3_finalize_t>(GetProcAddress(hMod, "sqlite3_finalize"));

    if (!sqlite3_open_v2_ || !sqlite3_key_ || !sqlite3_prepare_v2_ || !sqlite3_step_ || !sqlite3_column_text_ ||
        !sqlite3_close_v2_ || !sqlite3_finalize_) {
        infra_ = nullptr;
        return false;
    }

    return true;
}

KugouDb::KugouDb(const std::filesystem::path& infra_dll_path, const std::filesystem::path& db_path)
    : SqliteDB(infra_dll_path) {
    int rc{-1};
    if (!IsInfraOk()) {
        return;
    }

    Open(db_path);
}

kgm_ekey_db_t KugouDb::dump_ekey(int& error) {
    if (!IsOpen()) {
        error = SQLITE_ERROR;
        return {};
    }

    int rc{-1};
    sqlite3_stmt* stmt{nullptr};

    rc = sqlite3_prepare_v2_(db_,
                             "select EncryptionKeyId, EncryptionKey from ShareFileItems"
                             " where EncryptionKey != ''",
                             -1, &stmt, nullptr);
    if (rc != SQLITE_OK) {
        error = rc;
        return {};
    }

    kgm_ekey_db_t result{};
    while ((rc = sqlite3_step_(stmt)) == SQLITE_ROW) {
        const auto* ekey_id = reinterpret_cast<const char*>(sqlite3_column_text_(stmt, 0));
        const auto* ekey = reinterpret_cast<const char*>(sqlite3_column_text_(stmt, 1));
        result[ekey_id] = ekey;
    }

    if (rc != SQLITE_DONE) {
        error = rc;
    }

    sqlite3_finalize_(stmt);
    error = 0;
    return result;
}

KugouDb::~KugouDb() {
    if (db_ != nullptr) {
        sqlite3_close_v2_(db_);
        db_ = nullptr;
    }
}

bool KugouDb::Open(const std::filesystem::path& db_path) {
    return SqliteDB::Open(db_path, {"7777B48756BA491BB4CEE771A3E2727E"});
}

}  // namespace Infra
