#include <cassert>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>

#include <iostream>

#include <catch2/catch_test_macros.hpp>

#include "input_iterator.hpp"
#include "sqlite3.h"
#include "time_series.hpp"

using sqlite_val_t = std::variant<std::int64_t, double, std::string, std::vector<std::byte>, std::nullptr_t>;

struct sqlite_ts {
    ::sqlite3 *db = nullptr;
    std::string tb;

    // TODO: forbid copy, replace shared ptr with unique ptr?
    // TODO: moved from state?
    struct const_iterator {
        ::sqlite3 *db = nullptr;
        std::shared_ptr<::sqlite3_stmt> stmt;

        const_iterator() = default;
        const_iterator(const const_iterator &) = default;
        const_iterator(const_iterator &&) noexcept = default;
        explicit const_iterator(::sqlite3 *d, const std::string &sql) : db(d)
        {
            ::sqlite3_stmt *pstmt = nullptr;

            const auto ret = ::sqlite3_prepare_v2(db, sql.c_str(), -1, &pstmt, nullptr);

            if (ret != SQLITE_OK) [[unlikely]] {
                throw std::runtime_error(::sqlite3_errstr(ret));
            }

            try {
                stmt.reset(pstmt, [](::sqlite3_stmt *s) { ::sqlite3_finalize(s); });
            } catch (...) {
                ::sqlite3_finalize(pstmt);
                throw;
            }

            // NOTE: trigger the first evaluation of the statement, this must always
            // be done after the preparation of the statement. This will bring us
            // to the first row of the result of the query.
            ++*this;
        }
        const_iterator &operator=(const const_iterator &) = delete;
        const_iterator &operator=(const_iterator &&) noexcept = default;
        ~const_iterator() = default;

        void operator++()
        {
            assert(db != nullptr);

            const auto ret = ::sqlite3_step(stmt.get());

            if (ret == SQLITE_DONE) {
                // NOTE: finished, reset the iterator to the
                // end() state.
                db = nullptr;
                stmt.reset();
                return;
            }

            if (ret != SQLITE_ROW) [[unlikely]] {
                throw std::runtime_error(::sqlite3_errstr(ret));
            }
        }

        std::pair<double, std::vector<sqlite_val_t>> operator*() const
        {
            assert(db != nullptr);

            // Reduce typing.
            auto *st = stmt.get();

            // Fetch the total number of columns. This is always nonzero.
            const auto ncols = ::sqlite3_column_count(st);
            assert(ncols > 0);

            // Fetch the time key.
            if (::sqlite3_column_type(st, 0) != SQLITE_FLOAT) [[unlikely]] {
                throw std::runtime_error("Invalid type for the time column");
            }
            const auto tm_val = ::sqlite3_column_double(st, 0);

            // Build the vector of values.
            std::vector<sqlite_val_t> vals;
            vals.reserve(static_cast<decltype(vals.size())>(ncols - 1));
            for (auto i = 1; i < ncols; ++i) {
                switch (::sqlite3_column_type(st, 0)) {
                    case SQLITE_INTEGER:
                        vals.emplace_back(::sqlite3_column_int64(st, i));
                        break;
                    case SQLITE_FLOAT:
                        vals.emplace_back(::sqlite3_column_double(st, i));
                        break;
                    case SQLITE_TEXT: {
                        const auto str_size = ::sqlite3_column_bytes(st, i);
                        const auto *text_ptr = ::sqlite3_column_text(st, i);
                        vals.emplace_back(std::string(text_ptr, text_ptr + str_size));

                        break;
                    }
                    case SQLITE_BLOB: {
                        const auto blob_size = ::sqlite3_column_bytes(st, i);
                        const auto *blob_ptr = ::sqlite3_column_blob(st, i);

                        if (blob_ptr == nullptr) {
                            vals.emplace_back(std::vector<std::byte>{});
                        } else {
                            vals.emplace_back(
                                std::vector<std::byte>(reinterpret_cast<const std::byte *>(blob_ptr),
                                                       reinterpret_cast<const std::byte *>(blob_ptr) + blob_size));
                        }

                        break;
                    }
                    case SQLITE_NULL:
                        vals.emplace_back(nullptr);

                        break;
                    default:
                        throw std::runtime_error("Unknown value type detected");
                }
            }

            return std::make_pair(tm_val, std::move(vals));
        }

        // NOTE: make only end() iterators compare equal to each other.
        [[nodiscard]] friend bool operator==(const const_iterator &a, const const_iterator &b) noexcept
        {
            return a.db == nullptr && b.db == nullptr;
        }
    };

    sqlite_ts() = delete;
    sqlite_ts(const sqlite_ts &) = delete;
    sqlite_ts(sqlite_ts &&other) noexcept : db(other.db), tb(std::move(other.tb))
    {
        other.db = nullptr;
    }
    // NOLINTNEXTLINE(bugprone-easily-swappable-parameters)
    explicit sqlite_ts(const std::string &filename, std::string table) : tb(std::move(table))
    {
        const auto ret = ::sqlite3_open_v2(filename.c_str(), &db, SQLITE_OPEN_READONLY, nullptr);

        if (ret != SQLITE_OK) [[unlikely]] {
            throw std::runtime_error(::sqlite3_errstr(ret));
        }
    }
    sqlite_ts &operator=(const sqlite_ts &) = delete;
    sqlite_ts &operator=(sqlite_ts &&) noexcept = delete;
    ~sqlite_ts()
    {
        if (db != nullptr) {
            [[maybe_unused]] const auto ret = ::sqlite3_close_v2(db);
            assert(ret == SQLITE_OK);
        }
    }

    [[nodiscard]] const_iterator begin() const
    {
        // NOTE: we assume here the time value is always on the first column.
        return const_iterator(db, "SELECT * FROM " + tb + " ORDER BY 1");
    }
    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    [[nodiscard]] const_iterator end() const
    {
        return const_iterator{};
    }
};

// NOLINTBEGIN(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)

TEST_CASE("basic")
{
    sqlite_ts ts{TANUKI_SQLITE_FILE, "my_table"};
    // auto its = facade::make_input_ts(std::move(ts));
    // facade::make_input_iterator(ts.begin());
    using iter_t = decltype(ts.begin());
    facade::input_iterator<facade::detail::deduce_iter_value_t<iter_t>, std::iter_reference_t<iter_t>,
                           std::iter_rvalue_reference_t<iter_t>>(ts.begin());
    // facade::make_input_range(std::move(ts));

    // auto it = ts.begin();
    // std::cout << (*it).first << '\n';
    // ++it;
    // std::cout << (*it).first << '\n';
    // ++it;
    // std::cout << (*it).first << '\n';
    // ++it;
    // std::cout << (*it).first << '\n';
    // ++it;
}

// NOLINTEND(cert-err58-cpp,misc-use-anonymous-namespace,cppcoreguidelines-avoid-do-while)
