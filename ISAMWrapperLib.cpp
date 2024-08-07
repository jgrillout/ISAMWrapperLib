//Version: 7.7.24.17.34
// File: ISAMWrapperLib.cpp
// Defines the functions for the static library.
#include "ISAMWrapperLib.h"
// Constructor implementation
ISAMWrapperLib::ISAMWrapperLib(const std::string& dbPath, const std::string& pkColumn) : primaryKeyColumn(pkColumn) {
    // Open the database connection
    int rc = sqlite3_open(dbPath.c_str(), &db);
    if (rc) {
        // Handle error
        std::cerr << "Error opening SQLite database: " << sqlite3_errmsg(db) << std::endl;
    }
}

// Destructor implementation
ISAMWrapperLib::~ISAMWrapperLib() {
    // Close the database connection
    sqlite3_close(db);
}
// Execute SQL query
int ISAMWrapperLib::executeQuery(const std::string& query) {
    char* errMsg = nullptr;
    int rc = sqlite3_exec(db, query.c_str(), NULL, 0, &errMsg);
    if (errMsg != nullptr) {
        sqlite3_free(errMsg);
    }
    return rc;
}


// Create a table
int ISAMWrapperLib::createTable(const std::string& tableName, const std::vector<std::string>& columns) {
    std::cerr << "inside createTable." << std::endl;
    if (columns.empty()) {
        std::cerr << "Error: Columns vector is empty." << std::endl;
        return SQLITE_ERROR;
    }
    std::string query = "CREATE TABLE IF NOT EXISTS " + tableName + " (" + columns[0] + " PRIMARY KEY, ";
    for (size_t i = 1; i < columns.size(); ++i) {
        query += columns[i];
        if (i != columns.size() - 1) query += ", ";
    }
    query += ")";
    std::cout << "Executing query: " << query << std::endl;
    return executeQuery(query);
}

int ISAMWrapperLib::insert(const std::string& tableName, std::vector<std::string> fieldNames,const std::vector<std::string>& fieldValues) {
    if (fieldValues.empty()) {
        // No fieldValues provided, return an error code
        return SQLITE_ERROR;
    }

    std::string query = "INSERT INTO " + tableName + " VALUES (";
    for (size_t i = 0; i < fieldNames.size(); ++i) {
        query += "'" + fieldValues[i] + "'";
        if (i != fieldNames.size() - 1) {
            query += ", ";
        }
    }
    query += ")";

    return executeQuery(query);
}

int ISAMWrapperLib::updateRow(const std::string& tableName, const std::vector<std::string>& columnNames, const std::vector<std::string>& newValues, const std::string& condition) {
    if (newValues.empty()) {
        // Invalid input, return an error code
        return SQLITE_ERROR;
    }

    std::string query = "UPDATE " + tableName + " SET ";
    for (size_t i = 0; i < columnNames.size(); ++i) {
        query += columnNames[i] + " = '" + newValues[i] + "'";
        if (i != columnNames.size() - 1) {
            query += ", ";
        }
    }
    if (!condition.empty()) {
        query += " WHERE " + condition;
    }

    return executeQuery(query);
}

// Delete a row
int ISAMWrapperLib::deleteRow(const std::string& tableName, const std::string& condition) {
    std::string query = "DELETE FROM " + tableName;
    if (!condition.empty()) {
        query += " WHERE " + condition;
    }

    return executeQuery(query);
}

// Select rows
std::vector<std::tuple<std::string, std::string>> ISAMWrapperLib::selectRows(const std::string& tableName, const std::string& condition, const std::string& orderBy) {
    std::string query = "SELECT * FROM " + tableName;
    if (!condition.empty()) {
        query += " WHERE " + condition;
    }
    if (!orderBy.empty()) {
        query += " ORDER BY " + orderBy;
    }

    sqlite3_stmt* stmt;
    std::vector<std::tuple<std::string, std::string>> results;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL) == SQLITE_OK) {
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            std::string id =   reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
            std::string name = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 1));
            results.emplace_back(id, name);
        }
        sqlite3_finalize(stmt);
    }
    return results;
}
bool ISAMWrapperLib::getRecordByCondition(const std::string& table, const std::string& condition, std::vector<std::string>& record) {
    std::string query = "SELECT * FROM " + table + " WHERE " + condition;
    sqlite3_stmt* stmt;
    if (sqlite3_prepare_v2(db, query.c_str(), -1, &stmt, NULL) != SQLITE_OK) {
        return false; // Query preparation failure
    }

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int columns = sqlite3_column_count(stmt);
        for (int i = 0; i < columns; ++i) {
            const unsigned char* colValue = sqlite3_column_text(stmt, i);
            record.push_back(colValue ? reinterpret_cast<const char*>(colValue) : "");
        }
        sqlite3_finalize(stmt);
        return true; // Record found and populated
    }

    sqlite3_finalize(stmt);
    record.clear(); // Clear any existing data in the record vector
    return false; // No record found
}

