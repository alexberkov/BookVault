#pragma once
#include "../domain/author_fwd.h"
#include "use_cases.h"

namespace app {

class UseCasesImpl : public UseCases {
public:
    explicit UseCasesImpl(UnitOfWorkFactory* factory) {
        factory_ = factory;
    }

    std::optional<std::string> AddAuthor(const std::string& name) override;
    std::optional<std::string> AddBook(const std::string& title, size_t year, std::string author_id) override;
    void AddBookTags(const std::string& book_id, const std::vector<std::string>& book_tags) override;
    std::vector<items::AuthorInfo> GetAuthors() override;
    std::vector<items::BookInfo> GetBooks() override;
    std::vector<items::BookInfo> GetAuthorBooks(const std::string& author_id) override;
    std::optional<items::AuthorInfo> FindAuthorByName(const std::string& author_name) override;
    std::optional<items::AuthorInfo> FindAuthorById(const std::string& author_id) override;
    std::vector<items::BookInfo> FindBookByTitle(const std::string& book_title) override;
    void DeleteAuthor(const std::string& author_id) override;
    void EditAuthor(const std::string& author_id, const std::string& new_author_name) override;
    void DeleteBook(const std::string& book_id) override;
    void EditBook(const items::BookInfo& book) override;
    std::optional<items::AuthorInfo> GetBookAuthor(const std::string& book_id) override;
    std::vector<std::string> GetBookTags(const std::string& book_id) override;
    void EditBookTags(const std::string& book_id, const std::vector<std::string>& new_tags) override;
    void EndTransaction() override;
    void CancelTransaction() override;

private:
    UnitOfWorkFactory* factory_;
};



}  // namespace app
