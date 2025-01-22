#pragma once

// SQLite
#define SQLITE_OPEN_READONLY 0x00000001  /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_READWRITE 0x00000002 /* Ok for sqlite3_open_v2() */
#define SQLITE_OPEN_CREATE 0x00000004    /* Ok for sqlite3_open_v2() */

typedef struct sqlite3 sqlite3;
typedef struct sqlite3_stmt sqlite3_stmt;
