/*
  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*/

#include <set>
#include <vector>

#include "JSSQLite.h"
#include "D2BS.h"
#include "File.h"
#include "Helpers.h"

using namespace std;

struct SqliteDB;
struct DBStmt;
typedef std::set<DBStmt*> StmtList;

// open is true exactly when db is a live handle; a closed or never-opened database keeps db NULL.
struct SqliteDB {
    sqlite3* db;
    bool open;
    wchar_t* path;
    StmtList stmts;
};

// Every statement object holds one reference (db.statements builds more objects for the same statement), and the
// last release deletes it. A listed statement's assoc_db is its database; unlisting it or closing the database NULLs it.
struct DBStmt {
    sqlite3_stmt* stmt;
    bool open, canGet;
    SqliteDB* assoc_db;
    // Rooted while non-NULL; row_rt is the runtime holding that root, so finalizers can drop it without a context.
    JSObject* current_row;
    JSRuntime* row_rt;
    int refs;
};

void close_db_stmt(DBStmt* stmt);
bool clean_sqlite_db(SqliteDB* db);

// The cached getObject() row must leave the runtime's root table before its statement moves to another row, closes
// or is deleted: the root is the address of this field, so a deleted DBStmt would leave the GC reading freed memory.
static void drop_current_row(DBStmt* stmt) {
    if (stmt->current_row) {
        JS_RemoveObjectRootRT(stmt->row_rt, &stmt->current_row);
        stmt->current_row = NULL;
        stmt->row_rt = NULL;
    }
}

// Closing leaves db NULL and open false, so a closed database never hands sqlite a freed handle and open() can
// reopen it. A failed sqlite3_close keeps the handle live, so the state is left as it was.
bool clean_sqlite_db(SqliteDB* db) {
    if (db && db->open) {
        // Statement objects can outlive their database object, so each statement leaves it closed and detached.
        for (StmtList::iterator it = db->stmts.begin(); it != db->stmts.end(); it++) {
            close_db_stmt(*it);
            (*it)->assoc_db = NULL;
        }
        db->stmts.clear();
        if (SQLITE_OK != sqlite3_close(db->db))
            return false;
        db->db = NULL;
        db->open = false;
    }
    return true;
}

void close_db_stmt(DBStmt* stmt) {
    // go(), close(), db.close() and the last release all end here, so the cached row and ready state end here too.
    drop_current_row(stmt);
    stmt->canGet = false;
    if (stmt->stmt && stmt->open) {
        sqlite3_finalize(stmt->stmt);
        stmt->stmt = NULL;
        stmt->open = false;
    }
}

static void unlist_db_stmt(DBStmt* stmt) {
    if (stmt->assoc_db) {
        stmt->assoc_db->stmts.erase(stmt);
        stmt->assoc_db = NULL;
    }
}

static void release_db_stmt(DBStmt* stmt) {
    if (--stmt->refs > 0)
        return;
    // close_db_stmt drops the cached row's root, which must happen before the DBStmt holding it is deleted.
    close_db_stmt(stmt);
    unlist_db_stmt(stmt);
    delete stmt;
}

// Thrown by every statement method that needs a live statement after close(), go() or the database's close().
static const char STMT_CLOSED[] = "The statement is closed";

static const char* db_stmt_errmsg(DBStmt* stmt) {
    return stmt->assoc_db ? sqlite3_errmsg(stmt->assoc_db->db) : STMT_CLOSED;
}

// sqlite and SQL text can hold '%', so it travels as a "%s" argument and never becomes JS_ReportError's format.
static JSBool report_sqlite_error(JSContext* cx, const char* prefix, const char* detail) {
    JS_ReportError(cx, "%s%s", prefix ? prefix : "", detail ? detail : "unknown error");
    return JS_FALSE;
}

enum BindStatus { BIND_OK, BIND_BAD_TYPE, BIND_FAILED };

// Binds one script value to parameter idx. Text binds SQLITE_TRANSIENT because the UTF-8 copy is freed before
// sqlite3_step reads it in a UTF-8 database; booleans bind as 1/0 and null as NULL. BIND_FAILED means the error is
// already reported (by the engine on OOM, or here for a sqlite3_bind_* failure); BIND_BAD_TYPE means nothing was
// reported, so the caller throws.
static BindStatus bind_js_value(JSContext* cx, sqlite3_stmt* stmt, int idx, jsval v) {
    int rc = SQLITE_OK;
    if (JSVAL_IS_NULL(v) || JSVAL_IS_VOID(v))
        rc = sqlite3_bind_null(stmt, idx);
    else if (JSVAL_IS_BOOLEAN(v))
        rc = sqlite3_bind_int(stmt, idx, JSVAL_TO_BOOLEAN(v) ? 1 : 0);
    else if (JSVAL_IS_INT(v))
        rc = sqlite3_bind_int(stmt, idx, JSVAL_TO_INT(v));
    else if (JSVAL_IS_DOUBLE(v))
        rc = sqlite3_bind_double(stmt, idx, JSVAL_TO_DOUBLE(v));
    else if (JSVAL_IS_STRING(v)) {
        char* szText = JS_EncodeStringToUTF8(cx, JSVAL_TO_STRING(v));
        if (!szText)
            return BIND_FAILED; // the engine has reported the failure
        rc = sqlite3_bind_text(stmt, idx, szText, -1, SQLITE_TRANSIENT);
        JS_free(cx, szText);
    } else
        return BIND_BAD_TYPE;

    if (rc != SQLITE_OK) {
        JS_ReportError(cx, "Could not bind parameter %d: %s", idx, sqlite3_errstr(rc));
        return BIND_FAILED;
    }
    return BIND_OK;
}

JSAPI_FUNC(my_sqlite_version) {
    if (!StoreString(vp, JS_InternString(cx, sqlite3_version)))
        return JS_FALSE;
    return JS_TRUE;
}

JSAPI_FUNC(my_sqlite_memusage) {
    JS_SET_RVAL(cx, vp, JS_NumberValue((jsdouble)sqlite3_memory_used()));
    return JS_TRUE;
}

EMPTY_CTOR(sqlite_stmt)

JSAPI_FUNC(sqlite_ctor) {
    if (argc > 0 && !JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]))
        THROW_ERROR(cx, "Invalid parameters in SQLite constructor");
    wchar_t* path;

    if (argc > 0) {
        const jschar* name = JS_GetStringCharsZ(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[0]));
        if (!name)
            return JS_FALSE;
        path = _wcsdup(name);
    } else
        path = _wcsdup(L":memory:");
    if (!path) {
        JS_ReportOutOfMemory(cx);
        return JS_FALSE;
    }

    // if the path is not a special placeholder (:memory:, et. al.), sandbox it
    if (path[0] != ':') {
        if (!isValidPath(path)) {
            free(path);
            THROW_ERROR(cx, "Invalid characters in database name");
        }

        // isValidPath refuses any name whose join with the script folder would not fit this buffer.
        wchar_t* tmp = (wchar_t*)malloc(sizeof(wchar_t) * (_MAX_PATH + _MAX_FNAME));
        if (!tmp) {
            free(path);
            JS_ReportOutOfMemory(cx);
            return JS_FALSE;
        }
        swprintf_s(tmp, _MAX_PATH + _MAX_FNAME, L"%s\\%s", Vars.szScriptPath, path);
        free(path);
        path = tmp;
    }

    // The class registers no constructor arguments, so argv[1] holds the script's value only when argc > 1.
    bool autoOpen = true;
    if (argc > 1 && JSVAL_IS_BOOLEAN(JS_ARGV(cx, vp)[1]))
        autoOpen = !!JSVAL_TO_BOOLEAN(JS_ARGV(cx, vp)[1]);

    sqlite3* db = NULL;
    // A failed open still hands back a handle holding the error message: report it, then close the handle.
    if (autoOpen && SQLITE_OK != sqlite3_open16(path, &db)) {
        JSBool rv = report_sqlite_error(cx, "Could not open database: ", sqlite3_errmsg(db));
        sqlite3_close(db);
        free(path);
        return rv;
    }

    SqliteDB* dbobj = new SqliteDB;
    dbobj->db = db;
    dbobj->open = autoOpen;
    dbobj->path = path;

    JSObject* jsdb = BuildObject(cx, &sqlite_db, sqlite_methods, sqlite_props, dbobj);
    if (!jsdb) {
        sqlite3_close(db);
        free(dbobj->path);
        delete dbobj;
        THROW_ERROR(cx, "Could not create the sqlite object");
    }
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(jsdb));

    return JS_TRUE;
}

JSAPI_FUNC(sqlite_execute) {
    JSObject* obj = JS_THIS_OBJECT(cx, vp);
    // A foreign or prototype this has no database.
    SqliteDB* dbobj = (SqliteDB*)JS_GetInstancePrivate(cx, obj, &sqlite_db, NULL);
    if (!dbobj) {
        JS_SET_RVAL(cx, vp, JSVAL_FALSE);
        return JS_TRUE;
    }
    if (dbobj->open != true)
        THROW_ERROR(cx, "Database must first be opened!");
    if (argc < 1 || argc > 1 || !JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]))
        THROW_ERROR(cx, "Invalid parameters in SQLite.execute");

    char* sql = JS_EncodeStringToUTF8(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[0]));
    if (!sql)
        return JS_FALSE; // the engine has reported the failure
    char* err = NULL;
    if (SQLITE_OK != sqlite3_exec(dbobj->db, sql, NULL, NULL, &err)) {
        // err is NULL only when sqlite could not allocate it; sqlite3_errmsg then reports that out-of-memory error.
        report_sqlite_error(cx, "", err ? err : sqlite3_errmsg(dbobj->db));
        sqlite3_free(err);
        JS_free(cx, sql);
        return JS_FALSE;
    }
    JS_free(cx, sql);
    JS_SET_RVAL(cx, vp, JSVAL_TRUE);
    return JS_TRUE;
}

JSAPI_FUNC(sqlite_query) {
    JSObject* obj = JS_THIS_OBJECT(cx, vp);
    // A foreign or prototype this has no database.
    SqliteDB* dbobj = (SqliteDB*)JS_GetInstancePrivate(cx, obj, &sqlite_db, NULL);
    if (!dbobj) {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;
    }
    if (dbobj->open != true)
        THROW_ERROR(cx, "Database must first be opened!");
    if (argc < 1 || !JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]))
        THROW_ERROR(cx, "Invalid parameters to SQLite.query");

    char* sql = JS_EncodeStringToUTF8(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[0]));
    if (!sql)
        return JS_FALSE; // the engine has reported the failure
    sqlite3_stmt* stmt;
    if (SQLITE_OK != sqlite3_prepare_v2(dbobj->db, sql, strlen(sql), &stmt, NULL)) {
        JS_free(cx, sql);
        return report_sqlite_error(cx, "", sqlite3_errmsg(dbobj->db));
    }
    if (stmt == NULL) {
        JS_free(cx, sql);
        THROW_ERROR(cx, "Statement has no effect");
    }

    for (uint i = 1; i < argc; i++) {
        BindStatus st = bind_js_value(cx, stmt, (int)i, JS_ARGV(cx, vp)[i]);
        if (st == BIND_OK)
            continue;
        sqlite3_finalize(stmt);
        JS_free(cx, sql);
        if (st == BIND_BAD_TYPE) {
            char msg[1024];
            sprintf_s(msg, sizeof(msg), "Invalid bound parameter %i", i);
            THROW_ERROR(cx, msg);
        }
        return JS_FALSE; // bind_js_value reported the error
    }

    DBStmt* dbstmt = new DBStmt;
    dbstmt->stmt = stmt;
    dbstmt->open = true;
    dbstmt->canGet = false;
    dbstmt->assoc_db = dbobj;
    dbstmt->current_row = NULL;
    dbstmt->row_rt = NULL;
    dbstmt->refs = 1;

    JSObject* row = BuildObject(cx, &sqlite_stmt, sqlite_stmt_methods, sqlite_stmt_props, dbstmt);
    if (!row) {
        sqlite3_finalize(stmt);
        delete dbstmt;
        JS_free(cx, sql);
        THROW_ERROR(cx, "Could not create the sqlite row object");
    }
    // The database closes every statement in its list, so a statement joins it only once its object exists.
    dbobj->stmts.insert(dbstmt);
    JS_free(cx, sql);
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(row));
    return JS_TRUE;
}

JSAPI_FUNC(sqlite_close) {
    JSObject* obj = JS_THIS_OBJECT(cx, vp);
    // A foreign or prototype this has no database.
    SqliteDB* dbobj = (SqliteDB*)JS_GetInstancePrivate(cx, obj, &sqlite_db, NULL);
    if (!dbobj) {
        JS_SET_RVAL(cx, vp, JSVAL_FALSE);
        return JS_TRUE;
    }
    if (!clean_sqlite_db(dbobj))
        return report_sqlite_error(cx, "Could not close database: ", sqlite3_errmsg(dbobj->db));
    JS_SET_RVAL(cx, vp, JSVAL_TRUE);
    return JS_TRUE;
}

JSAPI_FUNC(sqlite_open) {
    JSObject* obj = JS_THIS_OBJECT(cx, vp);
    // A foreign or prototype this has no database.
    SqliteDB* dbobj = (SqliteDB*)JS_GetInstancePrivate(cx, obj, &sqlite_db, NULL);
    if (!dbobj) {
        JS_SET_RVAL(cx, vp, JSVAL_FALSE);
        return JS_TRUE;
    }
    if (!dbobj->open) {
        // A failed open still hands back a handle holding the error message: report it, then close the handle.
        sqlite3* db = NULL;
        if (SQLITE_OK != sqlite3_open16(dbobj->path, &db)) {
            JSBool rv = report_sqlite_error(cx, "Could not open database: ", sqlite3_errmsg(db));
            sqlite3_close(db);
            return rv;
        }
        dbobj->db = db;
        dbobj->open = true;
    }
    JS_SET_RVAL(cx, vp, JSVAL_TRUE);
    return JS_TRUE;
}

JSAPI_PROP(sqlite_getProperty) {
    SqliteDB* dbobj = (SqliteDB*)JS_GetInstancePrivate(cx, obj, &sqlite_db, NULL);
    if (!dbobj) {
        vp.setUndefined();
        return JS_TRUE;
    }

    jsval ID;
    if (!JS_IdToValue(cx, id, &ID))
        return JS_FALSE;
    switch (JSVAL_TO_INT(ID)) {
    case SQLITE_PATH:
        if (!StoreString(vp, JS_NewUCStringCopyZ(cx, dbobj->path)))
            return JS_FALSE;
        break;
    case SQLITE_OPEN:
        vp.setBoolean(dbobj->open);
        break;
    case SQLITE_LASTROWID:
        // A database that is not open has no handle and reports 0. A double keeps rowids above 2^31 (exact to 2^53).
        vp.set(JS_NumberValue(dbobj->open ? (jsdouble)sqlite3_last_insert_rowid(dbobj->db) : 0));
        break;
    case SQLITE_STMTS: {
        // Pinned from a copy of the list: building an object can run the GC, whose statement finalizers release and
        // unlist statements.
        std::vector<DBStmt*> list(dbobj->stmts.begin(), dbobj->stmts.end());
        for (size_t i = 0; i < list.size(); i++)
            list[i]->refs++;
        JSBool ok = JS_TRUE;
        JS_BeginRequest(cx);
        // Only open statements are listed, packed from index 0: go() leaves a finished statement in the database's list.
        JSObject* stmts = JS_NewArrayObject(cx, 0, NULL);
        if (!stmts)
            ok = JS_FALSE;
        else
            vp.set(OBJECT_TO_JSVAL(stmts)); // roots the array while the statement objects are built
        uint32_t count = 0;
        for (size_t i = 0; ok && i < list.size(); i++) {
            if (!list[i]->open)
                continue;
            JSObject* stmt = BuildObject(cx, &sqlite_stmt, sqlite_stmt_methods, sqlite_stmt_props, list[i]);
            if (!stmt) {
                ok = JS_FALSE;
                break;
            }
            list[i]->refs++;
            // Defined, not set: an inherited Array.prototype index setter would take the statement and run script mid-build.
            if (!JS_DefineElement(cx, stmts, count++, OBJECT_TO_JSVAL(stmt), NULL, NULL, JSPROP_ENUMERATE)) {
                ok = JS_FALSE;
                break;
            }
        }
        for (size_t i = 0; i < list.size(); i++)
            release_db_stmt(list[i]);
        JS_EndRequest(cx);
        if (!ok)
            return JS_FALSE;
    } break;
    case SQLITE_CHANGES:
        // Int-tagged when integral, so the count passes the integer checks of skip() and bind().
        vp.set(JS_NumberValue(dbobj->open ? (jsdouble)sqlite3_changes(dbobj->db) : 0));
        break;
    }
    return JS_TRUE;
}

void sqlite_finalize(JSFreeOp* fop, JSObject* obj) {
    SqliteDB* dbobj = (SqliteDB*)JS_GetPrivate(obj);
    JS_SetPrivate(obj, NULL);
    if (dbobj) {
        clean_sqlite_db(dbobj);
        free(dbobj->path); //_wcsdup reqires free not delete
        delete dbobj;
    }
}

// JSBool sqlite_equal(JSContext *cx, JSObject *obj, jsval v, JSBool *bp)
//{
//	SqliteDB* dbobj = (SqliteDB*)JS_GetInstancePrivate(cx, obj, &sqlite_db, NULL);
//	if(!JSVAL_IS_OBJECT(v))
//		return JS_TRUE;
//	JSObject *obj2 = JSVAL_TO_OBJECT(v);
//	if(!obj2 || JS_GET_CLASS(cx, obj2) != JS_GET_CLASS(cx, obj))
//		return JS_TRUE;
//
//	SqliteDB* dbobj2 = (SqliteDB*)JS_GetPrivate(cx, obj2);
//	if(dbobj2->db != dbobj->db)
//		return JS_TRUE;
//
//	*bp = JS_TRUE;
//	return JS_TRUE;
//}

JSAPI_FUNC(sqlite_stmt_getobject) {
    JSObject* obj = JS_THIS_OBJECT(cx, vp);
    DBStmt* stmtobj = (DBStmt*)JS_GetInstancePrivate(cx, obj, &sqlite_stmt, NULL);
    if (!stmtobj) {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;
    }
    if (!stmtobj->stmt)
        THROW_ERROR(cx, STMT_CLOSED);
    sqlite3_stmt* stmt = stmtobj->stmt;

    if (!stmtobj->canGet) {
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        return JS_TRUE;
    }
    if (stmtobj->current_row) {
        JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(stmtobj->current_row));
        return JS_TRUE;
    }

    int cols = sqlite3_column_count(stmt);
    if (cols == 0) {
        JS_SET_RVAL(cx, vp, JSVAL_TRUE);
        return JS_TRUE;
    }
    JSObject* obj2 = JS_NewObject(cx, NULL, NULL, NULL);
    if (!obj2)
        return JS_FALSE; // the engine has reported the failure
    for (int i = 0; i < cols; i++) {
        if (stmtobj->stmt != stmt)
            THROW_ERROR(cx, STMT_CLOSED);
        const char* colnam = sqlite3_column_name(stmt, i);
        // i is below the column count, so sqlite returns no name only when it ran out of memory.
        if (!colnam) {
            JS_ReportOutOfMemory(cx);
            return JS_FALSE;
        }
        jsval val = JSVAL_NULL;
        switch (sqlite3_column_type(stmt, i)) {
        case SQLITE_INTEGER:
            // jsdouble == double, so this conversion is no problem
            val = JS_NumberValue((jsdouble)sqlite3_column_int64(stmt, i));
            break;
        case SQLITE_FLOAT:
            val = JS_NumberValue(sqlite3_column_double(stmt, i));
            break;
        case SQLITE_TEXT: {
            // Only a failed conversion makes a TEXT value's text NULL, and AnsiToUnicode cannot take NULL.
            const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
            if (!text) {
                JS_ReportOutOfMemory(cx);
                return JS_FALSE;
            }
            wchar_t* wText = AnsiToUnicode(text);
            bool stored = StoreString(&val, JS_NewUCStringCopyZ(cx, wText));
            delete[] wText;
            if (!stored)
                return JS_FALSE;
            break;
        }
        case SQLITE_BLOB:
            // currently not supported
            THROW_ERROR(cx, "Blob type not supported (yet)");
            break;
        case SQLITE_NULL:
            break; // val is already null
        }
        // A GC while this row is built can finalize the database and with it this statement; colnam lives in the statement.
        if (stmtobj->stmt != stmt)
            THROW_ERROR(cx, STMT_CLOSED);
        // sqlite column names are UTF-8 and a char* property name is read as Latin-1, so the key is decoded the
        // same way getColumnName decodes the name.
        wchar_t* wName = AnsiToUnicode(colnam);
        if (!wName) {
            JS_ReportOutOfMemory(cx);
            return JS_FALSE;
        }
        JSBool defined = JS_DefineUCProperty(cx, obj2, (const jschar*)wName, wcslen(wName), val, NULL, NULL, JSPROP_ENUMERATE);
        delete[] wName;
        if (!defined)
            return JS_FALSE; // the engine has reported the failure
    }
    // The last property store can GC too, and a closed statement must not cache a row.
    if (stmtobj->stmt != stmt)
        THROW_ERROR(cx, STMT_CLOSED);
    // Rooted before it is cached, so current_row is never a cached object the GC can't see.
    if (!JS_AddRoot(cx, &stmtobj->current_row))
        return JS_FALSE;
    stmtobj->current_row = obj2;
    stmtobj->row_rt = JS_GetRuntime(cx);
    JS_SET_RVAL(cx, vp, OBJECT_TO_JSVAL(obj2));
    return JS_TRUE;
}

JSAPI_FUNC(sqlite_stmt_colcount) {
    JSObject* obj = JS_THIS_OBJECT(cx, vp);
    DBStmt* stmtobj = (DBStmt*)JS_GetInstancePrivate(cx, obj, &sqlite_stmt, NULL);
    if (!stmtobj) {
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(0));
        return JS_TRUE;
    }
    if (!stmtobj->stmt)
        THROW_ERROR(cx, STMT_CLOSED);
    sqlite3_stmt* stmt = stmtobj->stmt;

    if (!stmtobj->canGet)
        THROW_ERROR(cx, "Statement is not ready");

    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(sqlite3_column_count(stmt)));
    return JS_TRUE;
}

JSAPI_FUNC(sqlite_stmt_colval) {
    JSObject* obj = JS_THIS_OBJECT(cx, vp);
    DBStmt* stmtobj = (DBStmt*)JS_GetInstancePrivate(cx, obj, &sqlite_stmt, NULL);
    if (!stmtobj) {
        JS_SET_RVAL(cx, vp, JSVAL_VOID);
        return JS_TRUE;
    }
    if (!stmtobj->stmt)
        THROW_ERROR(cx, STMT_CLOSED);
    sqlite3_stmt* stmt = stmtobj->stmt;

    if (!stmtobj->canGet)
        THROW_ERROR(cx, "Statement is not ready");

    if (argc < 1 || argc > 1 || !JSVAL_IS_INT(JS_ARGV(cx, vp)[0]))
        THROW_ERROR(cx, "Invalid parameter for SQLiteStatement.getColumnValue");
    jsval rval = JS_RVAL(cx, vp);
    int i = JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);
    switch (sqlite3_column_type(stmt, i)) {
    case SQLITE_INTEGER:
        // jsdouble == double, so this conversion is no problem
        rval = JS_NumberValue((jsdouble)sqlite3_column_int64(stmt, i));
        JS_SET_RVAL(cx, vp, rval);
        break;
    case SQLITE_FLOAT:
        rval = JS_NumberValue(sqlite3_column_double(stmt, i));
        JS_SET_RVAL(cx, vp, rval);
        break;
    case SQLITE_TEXT: {
        // Only a failed conversion makes a TEXT value's text NULL, and AnsiToUnicode cannot take NULL.
        const char* text = reinterpret_cast<const char*>(sqlite3_column_text(stmt, i));
        if (!text) {
            JS_ReportOutOfMemory(cx);
            return JS_FALSE;
        }
        wchar_t* wText = AnsiToUnicode(text);
        bool stored = StoreString(vp, JS_NewUCStringCopyZ(cx, wText));
        delete[] wText;
        if (!stored)
            return JS_FALSE;
        break;
    }
    case SQLITE_BLOB:
        // currently not supported
        THROW_ERROR(cx, "Blob type not supported (yet)");
        break;
    case SQLITE_NULL:
        JS_SET_RVAL(cx, vp, JSVAL_NULL);
        break;
    }
    return JS_TRUE;
}

JSAPI_FUNC(sqlite_stmt_colname) {
    JSObject* obj = JS_THIS_OBJECT(cx, vp);
    DBStmt* stmtobj = (DBStmt*)JS_GetInstancePrivate(cx, obj, &sqlite_stmt, NULL);
    if (!stmtobj) {
        JS_SET_RVAL(cx, vp, JSVAL_VOID);
        return JS_TRUE;
    }
    if (!stmtobj->stmt)
        THROW_ERROR(cx, STMT_CLOSED);
    sqlite3_stmt* stmt = stmtobj->stmt;

    if (!stmtobj->canGet)
        THROW_ERROR(cx, "Statement is not ready");

    if (argc < 1 || argc > 1 || !JSVAL_IS_INT(JS_ARGV(cx, vp)[0]))
        THROW_ERROR(cx, "Invalid parameter for SQLiteStatement.getColumnName");

    int i = JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);
    if (i < 0 || i >= sqlite3_column_count(stmt))
        THROW_ERROR(cx, "Invalid column index");
    // In range, sqlite returns no name only when it ran out of memory converting it.
    const char* name = sqlite3_column_name(stmt, i);
    if (!name) {
        JS_ReportOutOfMemory(cx);
        return JS_FALSE;
    }
    wchar_t* wname = AnsiToUnicode(name);
    bool stored = StoreString(vp, JS_NewUCStringCopyZ(cx, wname));
    delete[] wname;
    if (!stored)
        return JS_FALSE;
    return JS_TRUE;
}

JSAPI_FUNC(sqlite_stmt_execute) {
    JSObject* obj = JS_THIS_OBJECT(cx, vp);
    DBStmt* stmtobj = (DBStmt*)JS_GetInstancePrivate(cx, obj, &sqlite_stmt, NULL);
    if (!stmtobj) {
        JS_SET_RVAL(cx, vp, JSVAL_FALSE);
        return JS_TRUE;
    }
    if (!stmtobj->stmt)
        THROW_ERROR(cx, STMT_CLOSED);

    // A step leaves the current row whatever it returns, so the cached row and ready state go first.
    drop_current_row(stmtobj);
    stmtobj->canGet = false;
    int res = sqlite3_step(stmtobj->stmt);

    if (SQLITE_ROW != res && SQLITE_DONE != res)
        return report_sqlite_error(cx, "", db_stmt_errmsg(stmtobj));
    close_db_stmt(stmtobj);
    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL((SQLITE_DONE == res)));
    return JS_TRUE;
}

JSAPI_FUNC(sqlite_stmt_bind) {
    JSObject* obj = JS_THIS_OBJECT(cx, vp);
    DBStmt* stmtobj = (DBStmt*)JS_GetInstancePrivate(cx, obj, &sqlite_stmt, NULL);
    if (!stmtobj) {
        JS_SET_RVAL(cx, vp, JSVAL_FALSE);
        return JS_TRUE;
    }
    if (!stmtobj->stmt)
        THROW_ERROR(cx, STMT_CLOSED);
    sqlite3_stmt* stmt = stmtobj->stmt;
    if (argc < 2 || argc > 2 || !(JSVAL_IS_STRING(JS_ARGV(cx, vp)[0]) || JSVAL_IS_INT(JS_ARGV(cx, vp)[0])))
        THROW_ERROR(cx, "Invalid parameters for SQLiteStatement.bind");

    int colnum = -1;
    if (JSVAL_IS_INT(JS_ARGV(cx, vp)[0]))
        colnum = JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);
    else {
        char* szName = JS_EncodeStringToUTF8(cx, JSVAL_TO_STRING(JS_ARGV(cx, vp)[0]));
        if (!szName)
            return JS_FALSE; // the engine has reported the failure
        colnum = sqlite3_bind_parameter_index(stmt, szName);
        JS_free(cx, szName);
    }

    if (colnum == 0)
        THROW_ERROR(cx, "Invalid parameter number, parameters start at 1");

    switch (bind_js_value(cx, stmt, colnum, JS_ARGV(cx, vp)[1])) {
    case BIND_OK:
        break;
    case BIND_BAD_TYPE:
        THROW_ERROR(cx, "Invalid bound parameter");
    default:
        return JS_FALSE; // bind_js_value reported the error
    }

    JS_SET_RVAL(cx, vp, JSVAL_TRUE);
    return JS_TRUE;
}

JSAPI_FUNC(sqlite_stmt_next) {
    JSObject* obj = JS_THIS_OBJECT(cx, vp);
    DBStmt* stmtobj = (DBStmt*)JS_GetInstancePrivate(cx, obj, &sqlite_stmt, NULL);
    if (!stmtobj) {
        JS_SET_RVAL(cx, vp, JSVAL_FALSE);
        return JS_TRUE;
    }
    if (!stmtobj->stmt)
        THROW_ERROR(cx, STMT_CLOSED);

    // A step leaves the current row whatever it returns, so the cached row and ready state go first.
    drop_current_row(stmtobj);
    stmtobj->canGet = false;
    int res = sqlite3_step(stmtobj->stmt);

    if (SQLITE_ROW != res && SQLITE_DONE != res)
        return report_sqlite_error(cx, "", db_stmt_errmsg(stmtobj));

    stmtobj->canGet = (SQLITE_ROW == res);
    JS_SET_RVAL(cx, vp, BOOLEAN_TO_JSVAL((SQLITE_ROW == res)));
    return JS_TRUE;
}

JSAPI_FUNC(sqlite_stmt_skip) {
    // this is read before any JS_SET_RVAL: the rval slot holds the callee that JS_THIS_OBJECT may need.
    JSObject* obj = JS_THIS_OBJECT(cx, vp);
    DBStmt* stmtobj = (DBStmt*)JS_GetInstancePrivate(cx, obj, &sqlite_stmt, NULL);
    if (!stmtobj) {
        JS_SET_RVAL(cx, vp, INT_TO_JSVAL(0));
        return JS_TRUE;
    }
    if (!stmtobj->stmt)
        THROW_ERROR(cx, STMT_CLOSED);
    if (argc < 1 || !JSVAL_IS_INT(JS_ARGV(cx, vp)[0]))
        THROW_ERROR(cx, "Invalid parameter to SQLiteStatement.skip");
    int rows = JSVAL_TO_INT(JS_ARGV(cx, vp)[0]);
    int skipped = 0;
    if (rows > 0) {
        drop_current_row(stmtobj);
        // Stops at the end of the results: a further step would auto-reset the statement to its first row.
        for (; skipped < rows; skipped++) {
            int res = sqlite3_step(stmtobj->stmt);
            if (res == SQLITE_ROW) {
                stmtobj->canGet = true;
                continue;
            }
            stmtobj->canGet = false;
            if (res == SQLITE_DONE)
                break;
            return report_sqlite_error(cx, "", db_stmt_errmsg(stmtobj));
        }
    }
    JS_SET_RVAL(cx, vp, INT_TO_JSVAL(skipped));
    return JS_TRUE;
}

JSAPI_FUNC(sqlite_stmt_reset) {
    JSObject* obj = JS_THIS_OBJECT(cx, vp);
    DBStmt* stmtobj = (DBStmt*)JS_GetInstancePrivate(cx, obj, &sqlite_stmt, NULL);
    if (!stmtobj) {
        JS_SET_RVAL(cx, vp, JSVAL_FALSE);
        return JS_TRUE;
    }
    if (!stmtobj->stmt)
        THROW_ERROR(cx, STMT_CLOSED);
    // sqlite3_reset rewinds the statement even when it reports the last step's error.
    int res = sqlite3_reset(stmtobj->stmt);
    stmtobj->canGet = false;
    drop_current_row(stmtobj);
    if (SQLITE_OK != res)
        return report_sqlite_error(cx, "", db_stmt_errmsg(stmtobj));
    JS_SET_RVAL(cx, vp, JSVAL_TRUE);
    return JS_TRUE;
}

JSAPI_FUNC(sqlite_stmt_close) {
    JSObject* obj = JS_THIS_OBJECT(cx, vp);
    DBStmt* stmtobj = (DBStmt*)JS_GetInstancePrivate(cx, obj, &sqlite_stmt, NULL);
    if (!stmtobj) {
        JS_SET_RVAL(cx, vp, JSVAL_FALSE);
        return JS_TRUE;
    }
    // The object keeps its DBStmt until its finalizer, so a closed statement stays distinct from a prototype
    // or foreign this; closing again does nothing, and other objects for it see it closed.
    close_db_stmt(stmtobj);
    unlist_db_stmt(stmtobj);
    JS_SET_RVAL(cx, vp, JSVAL_TRUE);
    return JS_TRUE;
}

JSAPI_PROP(sqlite_stmt_getProperty) {
    DBStmt* stmtobj = (DBStmt*)JS_GetInstancePrivate(cx, obj, &sqlite_stmt, NULL);
    if (!stmtobj) {
        vp.setUndefined();
        return JS_TRUE;
    }

    jsval ID;
    if (!JS_IdToValue(cx, id, &ID))
        return JS_FALSE;
    switch (JSVAL_TO_INT(ID)) {
    case SQLITE_STMT_SQL: {
        // A closed statement has no SQL text (sqlite3_sql returns NULL), and AnsiToUnicode leaves a NULL
        // input's buffer unterminated. Getters don't throw: statement properties are enumerable.
        const char* sql = sqlite3_sql(stmtobj->stmt);
        if (!sql) {
            vp.setUndefined();
            break;
        }
        wchar_t* wText = AnsiToUnicode(sql);
        bool stored = StoreString(vp, JS_NewUCStringCopyZ(cx, wText));
        delete[] wText;
        if (!stored)
            return JS_FALSE;
        break;
    }
    case SQLITE_STMT_READY:
        vp.setBoolean(stmtobj->stmt != NULL && stmtobj->canGet);
        break;
    }
    return JS_TRUE;
}

void sqlite_stmt_finalize(JSFreeOp* fop, JSObject* obj) {
    DBStmt* stmtobj = (DBStmt*)JS_GetPrivate(obj);
    JS_SetPrivate(obj, NULL);
    if (stmtobj)
        release_db_stmt(stmtobj);
}
