#pragma once

#define SQLITE_OK (0)

/* beginning-of-error-codes */
#define SQLITE_ERROR 1       /* Generic error */
#define SQLITE_INTERNAL 2    /* Internal logic error in SQLite */
#define SQLITE_PERM 3        /* Access permission denied */
#define SQLITE_ABORT 4       /* Callback routine requested an abort */
#define SQLITE_BUSY 5        /* The database file is locked */
#define SQLITE_LOCKED 6      /* A table in the database is locked */
#define SQLITE_NOMEM 7       /* A malloc() failed */
#define SQLITE_READONLY 8    /* Attempt to write a readonly database */
#define SQLITE_INTERRUPT 9   /* Operation terminated by sqlite3_interrupt()*/
#define SQLITE_IOERR 10      /* Some kind of disk I/O error occurred */
#define SQLITE_CORRUPT 11    /* The database disk image is malformed */
#define SQLITE_NOTFOUND 12   /* Unknown opcode in sqlite3_file_control() */
#define SQLITE_FULL 13       /* Insertion failed because database is full */
#define SQLITE_CANTOPEN 14   /* Unable to open the database file */
#define SQLITE_PROTOCOL 15   /* Database lock protocol error */
#define SQLITE_EMPTY 16      /* Internal use only */
#define SQLITE_SCHEMA 17     /* The database schema changed */
#define SQLITE_TOOBIG 18     /* String or BLOB exceeds size limit */
#define SQLITE_CONSTRAINT 19 /* Abort due to constraint violation */
#define SQLITE_MISMATCH 20   /* Data type mismatch */
#define SQLITE_MISUSE 21     /* Library used incorrectly */
#define SQLITE_NOLFS 22      /* Uses OS features not supported on host */
#define SQLITE_AUTH 23       /* Authorization denied */
#define SQLITE_FORMAT 24     /* Not used */
#define SQLITE_RANGE 25      /* 2nd parameter to sqlite3_bind out of range */
#define SQLITE_NOTADB 26     /* File opened that is not a database file */
#define SQLITE_NOTICE 27     /* Notifications from sqlite3_log() */
#define SQLITE_WARNING 28    /* Warnings from sqlite3_log() */
#define SQLITE_ROW 100       /* sqlite3_step() has another row ready */
#define SQLITE_DONE 101      /* sqlite3_step() has finished executing */
/* end-of-error-codes */

inline const char* sqlite_get_error(int rc) {
    switch (rc) {
        case SQLITE_ERROR:
            return "SQLITE_ERROR: Generic error";
        case SQLITE_INTERNAL:
            return "SQLITE_INTERNAL: Internal logic error in SQLite";
        case SQLITE_PERM:
            return "SQLITE_PERM: Access permission denied";
        case SQLITE_ABORT:
            return "SQLITE_ABORT: Callback routine requested an abort";
        case SQLITE_BUSY:
            return "SQLITE_BUSY: The database file is locked";
        case SQLITE_LOCKED:
            return "SQLITE_LOCKED: A table in the database is locked";
        case SQLITE_NOMEM:
            return "SQLITE_NOMEM: A malloc() failed";
        case SQLITE_READONLY:
            return "SQLITE_READONLY: Attempt to write a readonly database";
        case SQLITE_INTERRUPT:
            return "SQLITE_INTERRUPT: Operation terminated by sqlite3_interrupt()";
        case SQLITE_IOERR:
            return "SQLITE_IOERR: Some kind of disk I/O error occurred";
        case SQLITE_CORRUPT:
            return "SQLITE_CORRUPT: The database disk image is malformed";
        case SQLITE_NOTFOUND:
            return "SQLITE_NOTFOUND: Unknown opcode in sqlite3_file_control()";
        case SQLITE_FULL:
            return "SQLITE_FULL: Insertion failed because database is full";
        case SQLITE_CANTOPEN:
            return "SQLITE_CANTOPEN: Unable to open the database file";
        case SQLITE_PROTOCOL:
            return "SQLITE_PROTOCOL: Database lock protocol error";
        case SQLITE_EMPTY:
            return "SQLITE_EMPTY: Internal use only";
        case SQLITE_SCHEMA:
            return "SQLITE_SCHEMA: The database schema changed";
        case SQLITE_TOOBIG:
            return "SQLITE_TOOBIG: String or BLOB exceeds size limit";
        case SQLITE_CONSTRAINT:
            return "SQLITE_CONSTRAINT: Abort due to constraint violation";
        case SQLITE_MISMATCH:
            return "SQLITE_MISMATCH: Data type mismatch";
        case SQLITE_MISUSE:
            return "SQLITE_MISUSE: Library used incorrectly";
        case SQLITE_NOLFS:
            return "SQLITE_NOLFS: Uses OS features not supported on host";
        case SQLITE_AUTH:
            return "SQLITE_AUTH: Authorization denied";
        case SQLITE_FORMAT:
            return "SQLITE_FORMAT: Not used";
        case SQLITE_RANGE:
            return "SQLITE_RANGE: 2nd parameter to sqlite3_bind out of range";
        case SQLITE_NOTADB:
            return "SQLITE_NOTADB: File opened that is not a database file";
        case SQLITE_NOTICE:
            return "SQLITE_NOTICE: Notifications from sqlite3_log()";
        case SQLITE_WARNING:
            return "SQLITE_WARNING: Warnings from sqlite3_log()";
        default:
            return "<unknown>";
    }
}
