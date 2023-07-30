#pragma once

#include <string>
#include <vector>
#include <optional>
#include <memory>

namespace items {

struct AuthorInfo {
    std::string id;
    std::string name;
    AuthorInfo(std::string&& _id, std::string&& _name): id(std::move(_id)), name(std::move(_name)) {}
};

struct BookInfo {
    std::string title, id, author_id, author_name;
    int publication_year;
    BookInfo(std::string&& _title, std::string&& _id, std::string&& _author_id, std::string&& _author_name, int year):
            title(std::move(_title)),
            publication_year(year),
            id(std::move(_id)),
            author_id(std::move(_author_id)),
            author_name(std::move(_author_name)) {
    }
};

} // namespace items

namespace app {

class UseCases {
public:
    virtual std::optional<std::string> AddAuthor(const std::string& name) = 0;
    virtual std::optional<std::string> AddBook(const std::string& title, size_t year, std::string author_id) = 0;
    virtual void AddBookTags(const std::string& book_id, const std::vector<std::string>& book_tags) = 0;
    virtual std::vector<items::AuthorInfo> GetAuthors() = 0;
    virtual std::vector<items::BookInfo> GetBooks() = 0;
    virtual std::vector<items::BookInfo> GetAuthorBooks(const std::string& author_id) = 0;
    virtual std::optional<items::AuthorInfo> FindAuthorByName(const std::string& author_name) = 0;
    virtual std::vector<items::BookInfo> FindBookByTitle(const std::string& book_title) = 0;
    virtual std::optional<items::AuthorInfo> FindAuthorById(const std::string& author_id) = 0;
    virtual void DeleteAuthor(const std::string& author_id) = 0;
    virtual void EditAuthor(const std::string& author_id, const std::string& new_author_name) = 0;
    virtual void DeleteBook(const std::string& book_id) = 0;
    virtual void EditBook(const items::BookInfo& book) = 0;
    virtual std::optional<items::AuthorInfo> GetBookAuthor(const std::string& book_id) = 0;
    virtual std::vector<std::string> GetBookTags(const std::string& book_id) = 0;
    virtual void EditBookTags(const std::string& book_id, const std::vector<std::string>& new_tags) = 0;
    virtual void EndTransaction() = 0;
    virtual void CancelTransaction() = 0;

protected:
    ~UseCases() = default;
};

class UnitOfWork {
public:
    virtual std::optional<std::string> AddAuthor(const std::string& name) = 0;
    virtual std::optional<std::string> AddBook(const std::string& title, size_t year, std::string author_id) = 0;
    virtual void AddBookTags(const std::string& book_id, const std::vector<std::string>& book_tags) = 0;
    virtual std::vector<items::AuthorInfo> GetAuthors() = 0;
    virtual std::vector<items::BookInfo> GetBooks() = 0;
    virtual std::vector<items::BookInfo> GetAuthorBooks(const std::string& author_id) = 0;
    virtual std::optional<items::AuthorInfo> FindAuthorByName(const std::string& author_name) = 0;
    virtual std::vector<items::BookInfo> FindBookByTitle(const std::string& book_title) = 0;
    virtual void DeleteAuthor(const std::string& author_id) = 0;
    virtual void DeleteAuthorBooks(const std::string& author_id) = 0;
    virtual void DeleteBookTags(const std::string& book_id) = 0;
    virtual void EditAuthor(const std::string& author_id, const std::string& new_author_name) = 0;
    virtual void DeleteBook(const std::string& book_id) = 0;
    virtual void EditBook(const items::BookInfo& book) = 0;
    virtual std::optional<items::AuthorInfo> GetBookAuthor(const std::string& book_id) = 0;
    virtual std::optional<items::AuthorInfo> FindAuthorById(const std::string& author_id) = 0;
    virtual std::vector<std::string> GetBookTags(const std::string& book_id) = 0;
    virtual void EditBookTags(const std::string& book_id, const std::vector<std::string>& new_tags_str) = 0;
    virtual void Commit() = 0;
    virtual void Reset() = 0;
    virtual ~UnitOfWork() = default;
};

class UnitOfWorkFactory {
public:
    virtual std::unique_ptr<UnitOfWork>& GetUnitOfWork() = 0;
    virtual void DeleteUnitOfWork() = 0;
    virtual ~UnitOfWorkFactory() = default;
};

}  // namespace app
