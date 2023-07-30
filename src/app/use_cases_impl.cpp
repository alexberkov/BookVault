#include "use_cases_impl.h"

#include "../domain/author.h"
#include "../domain/book.h"

namespace app {
using namespace domain;

void UseCasesImpl::EndTransaction() {
    factory_->GetUnitOfWork()->Commit();
    factory_->DeleteUnitOfWork();
}

void UseCasesImpl::CancelTransaction() {
    factory_->GetUnitOfWork()->Reset();
    factory_->DeleteUnitOfWork();
}

std::optional<std::string> UseCasesImpl::AddAuthor(const std::string& name) {
    return factory_->GetUnitOfWork()->AddAuthor(name);
}

std::optional<std::string> UseCasesImpl::AddBook(const std::string &title, size_t year, std::string author_id) {
    return factory_->GetUnitOfWork()->AddBook(title, year, author_id);
}

std::vector<items::AuthorInfo> UseCasesImpl::GetAuthors() {
    return factory_->GetUnitOfWork()->GetAuthors();
}

std::vector<items::BookInfo> UseCasesImpl::GetBooks() {
    return factory_->GetUnitOfWork()->GetBooks();
}

std::vector<items::BookInfo> UseCasesImpl::GetAuthorBooks(const std::string& author_id) {
    return factory_->GetUnitOfWork()->GetAuthorBooks(author_id);
}

std::optional<items::AuthorInfo> UseCasesImpl::FindAuthorByName(const std::string& author_name) {
    return factory_->GetUnitOfWork()->FindAuthorByName(author_name);
}

std::optional<items::AuthorInfo> UseCasesImpl::FindAuthorById(const std::string &author_id) {
    return factory_->GetUnitOfWork()->FindAuthorById(author_id);
}

std::vector<items::BookInfo> UseCasesImpl::FindBookByTitle(const std::string& book_title) {
    return factory_->GetUnitOfWork()->FindBookByTitle(book_title);
}

void UseCasesImpl::AddBookTags(const std::string &book_id, const std::vector<std::string> &book_tags) {
    factory_->GetUnitOfWork()->AddBookTags(book_id, book_tags);
}

void UseCasesImpl::DeleteAuthor(const std::string &author_id) {
    factory_->GetUnitOfWork()->DeleteAuthorBooks(author_id);
    factory_->GetUnitOfWork()->DeleteAuthor(author_id);
}

void UseCasesImpl::EditAuthor(const std::string &author_id, const std::string &new_author_name) {
    factory_->GetUnitOfWork()->EditAuthor(author_id, new_author_name);
}

std::optional<items::AuthorInfo> UseCasesImpl::GetBookAuthor(const std::string &book_id) {
    auto author_info = factory_->GetUnitOfWork()->GetBookAuthor(book_id);
    if (author_info.has_value())
        return factory_->GetUnitOfWork()->FindAuthorById(author_info.value().id);
    return std::nullopt;
}

std::vector<std::string> UseCasesImpl::GetBookTags(const std::string &book_id) {
    return factory_->GetUnitOfWork()->GetBookTags(book_id);
}

void UseCasesImpl::DeleteBook(const std::string &book_id) {
    factory_->GetUnitOfWork()->DeleteBookTags(book_id);
    factory_->GetUnitOfWork()->DeleteBook(book_id);
}

void UseCasesImpl::EditBook(const items::BookInfo &book) {
    factory_->GetUnitOfWork()->EditBook(book);
}

void UseCasesImpl::EditBookTags(const std::string &book_id, const std::vector<std::string> &new_tags) {
    factory_->GetUnitOfWork()->EditBookTags(book_id, new_tags);
}

}  // namespace app
