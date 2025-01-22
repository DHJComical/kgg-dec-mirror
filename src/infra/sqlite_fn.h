#pragma once

#include "sqlite_base.h"

/**
 * @brief Opens a SQLite database file with extended options.
 *
 * @param filename The name of the database file to be opened (UTF-8 encoded).
 * @param ppDb A pointer to a pointer that will receive the SQLite database
 *             handle upon successful opening.
 * @param flags Flags that control the behavior of the database connection.
 * @param zVfs The name of the VFS (Virtual File System) module to use.
 *             If NULL, the default VFS is used.
 * @return Returns SQLITE_OK on success or an error code on failure.
 */
typedef int (*sqlite3_open_v2_t)(const char* filename, sqlite3** ppDb, int flags, const char* zVfs);

/**
 * @brief Compiles an SQL statement into a prepared statement.
 *
 * @param db Database handle.
 * @param zSql SQL statement, UTF-8 encoded.
 * @param n Maximum length of zSql in bytes.
 * @param ppStmt OUT: Statement handle.
 * @param pzTail OUT: Pointer to unused portion of zSql.
 * @return Returns SQLITE_OK on success or an error code on failure.
 */
typedef int (*sqlite3_prepare_v2_t)(sqlite3* db, const char* zSql, int n, sqlite3_stmt** ppStmt, const char** pzTail);

/**
 * @brief Evaluates a prepared statement.
 *
 * @param stmt Prepared statement.
 * @return Returns SQLITE_ROW, SQLITE_DONE, or an error code.
 */
typedef int (*sqlite3_step_t)(sqlite3_stmt* stmt);

/**
 * @brief Returns the text value of a column in the current row of a result set.
 *
 * @param stmt Prepared statement.
 * @param iCol Column index.
 * @return Text value of the column.
 */
typedef const unsigned char* (*sqlite3_column_text_t)(sqlite3_stmt* stmt, int iCol);

/**
 * @brief Destroys a prepared statement object.
 *
 * @param stmt Prepared statement.
 * @return Returns SQLITE_OK on success or an error code on failure.
 */
typedef int (*sqlite3_finalize_t)(sqlite3_stmt* stmt);

/**
 * @brief Closes a database connection and invalidates all prepared statements.
 *
 * @param db Database handle.
 * @return Returns SQLITE_OK on success or an error code on failure.
 */
typedef int (*sqlite3_close_v2_t)(sqlite3* db);

/**
 * @brief Sets the encryption key for a database.
 *
 * @param db Database to be keyed.
 * @param pKey The key.
 * @param nKey The length of the key in bytes.
 * @return Returns SQLITE_OK on success or an error code on failure.
 */
typedef int (*sqlite3_key_t)(sqlite3* db, const void* pKey, int nKey);
