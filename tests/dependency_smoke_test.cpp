#include <catch2/catch_test_macros.hpp>
#include <curl/curl.h>
#include <nlohmann/json.hpp>
#include <sqlite3.h>

#include <memory>
#include <string>

TEST_CASE("SQLite preserves JSON source evidence", "[smoke][persistence]") {
    sqlite3* raw_database = nullptr;
    const int open_result = sqlite3_open(":memory:", &raw_database);
    const std::unique_ptr<sqlite3, decltype(&sqlite3_close)> database(raw_database, sqlite3_close);
    REQUIRE(open_result == SQLITE_OK);
    REQUIRE(sqlite3_exec(database.get(), "CREATE TABLE evidence (payload TEXT NOT NULL)", nullptr,
                         nullptr, nullptr) == SQLITE_OK);

    const nlohmann::json evidence = {
        {"title", "Unknown manufacturer's instrument"}, {"model", nullptr}, {"source_id", "00123"}};
    const std::string payload = evidence.dump();

    sqlite3_stmt* raw_insert = nullptr;
    const int insert_result = sqlite3_prepare_v2(
        database.get(), "INSERT INTO evidence (payload) VALUES (?)", -1, &raw_insert, nullptr);
    const std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> insert(raw_insert,
                                                                            sqlite3_finalize);
    REQUIRE(insert_result == SQLITE_OK);
    REQUIRE(sqlite3_bind_text(insert.get(), 1, payload.c_str(), -1, SQLITE_TRANSIENT) == SQLITE_OK);
    REQUIRE(sqlite3_step(insert.get()) == SQLITE_DONE);

    sqlite3_stmt* raw_select = nullptr;
    const int select_result = sqlite3_prepare_v2(database.get(), "SELECT payload FROM evidence", -1,
                                                 &raw_select, nullptr);
    const std::unique_ptr<sqlite3_stmt, decltype(&sqlite3_finalize)> select(raw_select,
                                                                            sqlite3_finalize);
    REQUIRE(select_result == SQLITE_OK);
    REQUIRE(sqlite3_step(select.get()) == SQLITE_ROW);
    const auto* stored = reinterpret_cast<const char*>(sqlite3_column_text(select.get(), 0));
    REQUIRE(stored != nullptr);
    REQUIRE(nlohmann::json::parse(stored) == evidence);
    REQUIRE(sqlite3_step(select.get()) == SQLITE_DONE);
}

TEST_CASE("libcurl can initialize without network access", "[smoke][http]") {
    REQUIRE(curl_global_init(CURL_GLOBAL_DEFAULT) == CURLE_OK);
    {
        const std::unique_ptr<CURL, decltype(&curl_easy_cleanup)> handle(curl_easy_init(),
                                                                         curl_easy_cleanup);
        CHECK(handle != nullptr);
    }
    curl_global_cleanup();
}
