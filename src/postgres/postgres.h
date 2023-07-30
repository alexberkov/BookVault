#pragma once
#include <pqxx/connection>
#include <pqxx/transaction>
#include <pqxx/pqxx>

#include "../domain/author.h"
#include "../domain/book.h"
#include "../app/use_cases.h"

namespace postgres {

class UnitOfWorkImpl : public app::UnitOfWork {
public:
    explicit UnitOfWorkImpl(pqxx::connection& connection): connection_{connection} {
        work_ = std::make_unique<pqxx::work>(connection_);
    }
    std::optional<std::string> AddAuthor(const std::string& name) override;
    std::optional<std::string> AddBook(const std::string& title, size_t year, std::string author_id) override;
    void AddBookTags(const std::string& book_id, const std::vector<std::string>& book_tags) override;
    std::vector<items::AuthorInfo> GetAuthors() override;
    std::vector<items::BookInfo> GetBooks() override;
    std::vector<items::BookInfo> GetAuthorBooks(const std::string& author_id) override;
    std::optional<items::AuthorInfo> FindAuthorByName(const std::string& author_name) override;
    std::vector<items::BookInfo> FindBookByTitle(const std::string& book_title) override;
    void DeleteAuthor(const std::string& author_id) override;
    void DeleteAuthorBooks(const std::string& author_id) override;
    void DeleteBookTags(const std::string& book_id) override;
    void EditAuthor(const std::string& author_id, const std::string& new_author_name) override;
    void DeleteBook(const std::string& book_id) override;
    void EditBook(const items::BookInfo& book) override;
    std::optional<items::AuthorInfo> GetBookAuthor(const std::string& book_id) override;
    std::optional<items::AuthorInfo> FindAuthorById(const std::string& author_id) override;
    std::vector<std::string> GetBookTags(const std::string& book_id) override;
    void EditBookTags(const std::string& book_id, const std::vector<std::string>& new_tags_str) override;
    void Commit() override;
    void Reset() override;
private:
    pqxx::connection& connection_;
    std::unique_ptr<pqxx::work> work_;
};

class UnitOfWorkFactoryImpl: public app::UnitOfWorkFactory {
public:
    explicit UnitOfWorkFactoryImpl(pqxx::connection& connection): connection_{connection}  {}
    std::unique_ptr<app::UnitOfWork>& GetUnitOfWork() override {
        if (unit_of_work_ == nullptr)
            unit_of_work_ = std::make_unique<UnitOfWorkImpl>(connection_);
        return unit_of_work_;
    }
    void DeleteUnitOfWork() override {
        unit_of_work_.reset();
    }
private:
    pqxx::connection& connection_;
    std::unique_ptr<app::UnitOfWork> unit_of_work_ = nullptr;
};

class Database {
public:
    explicit Database(pqxx::connection connection);
    pqxx::connection& GetConnection() {
        return connection_;
    }

private:
    pqxx::connection connection_;
};

}  // namespace postgres