#include "postgres.h"

#include <pqxx/zview.hxx>

namespace postgres {

using namespace std::literals;
using pqxx::operator"" _zv;

void UnitOfWorkImpl::Commit() {
    if (work_ != nullptr) {
        work_->commit();
        work_.reset();
    }
}

void UnitOfWorkImpl::Reset() {
   if (work_ != nullptr) {
       work_->abort();
       work_.reset();
   }
}

std::optional<std::string> UnitOfWorkImpl::AddAuthor(const std::string &name) {
    try {
        auto author_id = domain::AuthorId::New().ToString();
        work_->exec_params(R"(INSERT INTO authors (id, name) VALUES ($1, $2);)"_zv, author_id, name);
        return author_id;
    } catch (const std::exception& e) {
        return std::nullopt;
    }
}

std::optional<std::string> UnitOfWorkImpl::AddBook(const std::string &title, size_t year, std::string author_id) {
    try {
        auto book_id = domain::BookId::New().ToString();
        work_->exec_params(
                R"(INSERT INTO books (id, author_id, title, publication_year) VALUES ($1, $2, $3, $4))"_zv,
                    book_id, author_id, title, year);
        return book_id;
    } catch (const std::exception& e) {
        return std::nullopt;
    }
}

void UnitOfWorkImpl::AddBookTags(const std::string &book_id, const std::vector<std::string> &book_tags) {
    for (auto& tag: book_tags)
        work_->exec_params(R"(INSERT INTO book_tags (book_id, tag) VALUES ($1, $2);)"_zv, book_id, tag);
}

std::optional<items::AuthorInfo> UnitOfWorkImpl::FindAuthorByName(const std::string &author_name) {
    auto res = work_->exec_params(R"(SELECT * FROM authors WHERE name = $1)"_zv, author_name);
    if (res.empty())
        return std::nullopt;
    auto author = res.begin();
    return {{to_string(author.at("id")), to_string(author.at("name"))}};
}

std::vector<items::BookInfo> UnitOfWorkImpl::FindBookByTitle(const std::string& book_title) {
    std::vector<items::BookInfo> books;
    auto res = work_->exec_params(R"(SELECT * FROM books WHERE title = $1;)"_zv, book_title);
    for (auto row: res) {
        auto author_id = to_string(row.at("author_id"));
        auto res_author = work_->exec_params(R"(SELECT * FROM authors WHERE id = $1;)"_zv, author_id);
        if (res_author.empty() || res_author.size() > 1)
            continue;
        auto author_name = to_string(res_author.begin().at("name"));
        books.emplace_back(std::move(to_string(row.at("title"))),
                           std::move(to_string(row.at("id"))),
                           std::move(author_id),
                           std::move(author_name),
                           row.at("publication_year").as<int>());
    }
    return books;
}

std::vector<items::AuthorInfo> UnitOfWorkImpl::GetAuthors() {
    std::vector<items::AuthorInfo> authors;
    for (auto [id, name]: work_->query<std::string,std::string>("SELECT * FROM authors ORDER BY name;"_zv)) {
        authors.emplace_back(std::move(id), std::move(name));
    }
    return authors;
}

std::vector<items::BookInfo> UnitOfWorkImpl::GetBooks() {
    std::vector<items::BookInfo> books;
    auto res = work_->exec(R"(SELECT * FROM books ORDER BY title;)"_zv);
    for (auto row: res) {
        auto author_id = to_string(row.at("author_id"));
        auto res_author = work_->exec_params(R"(SELECT * FROM authors WHERE id = $1;)"_zv, author_id);
        if (res_author.empty() || res_author.size() > 1)
            continue;
        auto author_name = to_string(res_author.begin().at("name"));
        books.emplace_back(std::move(to_string(row.at("title"))),
                           std::move(to_string(row.at("id"))),
                           std::move(author_id),
                           std::move(author_name),
                           row.at("publication_year").as<int>());
    }
    return books;
}

std::vector<items::BookInfo> UnitOfWorkImpl::GetAuthorBooks(const std::string& author_id) {
    std::vector<items::BookInfo> books;
    auto res = work_->exec_params(R"(SELECT * FROM books WHERE author_id = $1 ORDER BY publication_year;)"_zv, author_id);
    auto res_author = work_->exec_params(R"(SELECT * FROM authors WHERE id = $1;)"_zv, author_id);
    if (res_author.empty() || res_author.size() > 1)
        return books;
    for (auto row: res) {
        auto author_name = to_string(res_author.begin().at("name"));
        books.emplace_back(std::move(to_string(row.at("title"))),
                           std::move(to_string(row.at("id"))),
                           std::move(to_string(row.at("author_id"))),
                           std::move(author_name),
                           row.at("publication_year").as<int>());
    }
    return books;
}

void UnitOfWorkImpl::DeleteAuthor(const std::string &author_id) {
    work_->exec_params(R"(DELETE FROM authors WHERE id = $1;)"_zv, author_id);
}

void UnitOfWorkImpl::DeleteBook(const std::string &book_id) {
    work_->exec_params(R"(DELETE FROM books WHERE id = $1;)"_zv, book_id);
}

void UnitOfWorkImpl::DeleteAuthorBooks(const std::string &author_id) {
    auto res = work_->exec_params(R"(SELECT * FROM books WHERE author_id = $1;)"_zv, author_id);
    for (auto row: res)
        DeleteBookTags(to_string(row.at("id")));
    work_->exec_params(R"(DELETE FROM books WHERE author_id = $1;)"_zv, author_id);
}

void UnitOfWorkImpl::DeleteBookTags(const std::string &book_id) {
    work_->exec_params(R"(DELETE FROM book_tags WHERE book_id = $1;)"_zv, book_id);
}

void UnitOfWorkImpl::EditAuthor(const std::string &author_id, const std::string &new_author_name) {
    work_->exec_params(R"(UPDATE authors SET name = $2 WHERE id = $1;)"_zv, author_id, new_author_name);
}

std::optional<items::AuthorInfo> UnitOfWorkImpl::GetBookAuthor(const std::string &book_id) {
    auto res = work_->exec_params(R"(SELECT * FROM books WHERE id = $1;)"_zv, book_id);
    if (res.empty())
        return std::nullopt;
    auto author_id = to_string(res.begin().at("author_id"));
    return {{std::move(author_id), "NULL"}};
}

std::optional<items::AuthorInfo> UnitOfWorkImpl::FindAuthorById(const std::string &author_id) {
    auto res = work_->exec_params(R"(SELECT * FROM authors WHERE id = $1;)"_zv, author_id);
    if (res.empty())
        return std::nullopt;
    auto author = res.begin();
    auto id = to_string(author.at("id")), name = to_string(author.at("name"));
    return {{std::move(id), std::move(name)}};
}

std::vector<std::string> UnitOfWorkImpl::GetBookTags(const std::string &book_id) {
    auto res = work_->exec_params(R"(SELECT * FROM book_tags WHERE book_id = $1;)"_zv, book_id);
    std::vector<std::string> tags;
    for (auto row: res)
        tags.push_back(to_string(row.at("tag")));
    return tags;
}

void UnitOfWorkImpl::EditBook(const items::BookInfo &book) {
    work_->exec_params(R"(UPDATE books SET title = $2, publication_year = $3 WHERE id = $1;)"_zv,
                       book.id, book.title, book.publication_year);
}

void UnitOfWorkImpl::EditBookTags(const std::string &book_id, const std::vector<std::string> &new_tags) {
    work_->exec_params(R"(DELETE FROM book_tags WHERE book_id = $1;)"_zv, book_id);
    for (auto& tag: new_tags)
        work_->exec_params(R"(INSERT INTO book_tags (book_id, tag) VALUES ($1, $2);)"_zv, book_id, tag);
}

Database::Database(pqxx::connection connection)
    : connection_{std::move(connection)} {
    pqxx::work work{connection_};
    work.exec(R"(
CREATE TABLE IF NOT EXISTS authors (
    id UUID CONSTRAINT author_id_constraint PRIMARY KEY,
    name varchar(100) UNIQUE NOT NULL
);
)"_zv);
    work.exec(R"(
CREATE TABLE IF NOT EXISTS books (
    id UUID CONSTRAINT book_id_constraint PRIMARY KEY,
    author_id UUID NOT NULL,
    title varchar(100) NOT NULL,
    publication_year integer NOT NULL
);
)"_zv);
    work.exec(R"(
CREATE TABLE IF NOT EXISTS book_tags (
    book_id UUID NOT NULL,
    tag varchar(30)
);
)"_zv);
    work.commit();
}

}  // namespace postgres