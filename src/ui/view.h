#pragma once
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

#include "../app/use_cases.h"

namespace menu {
class Menu;
}

namespace app {
class UseCases;
}

namespace ui {
namespace detail {

struct AddBookParams {
    std::string title;
    std::string author_id;
    int publication_year = 0;
};

}  // namespace detail

class View {
public:
    View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output);

private:
    bool AddAuthor(std::istream& cmd_input) const;
    bool AddBook(std::istream& cmd_input) const;
    bool AddBookTags(const std::string& book_id) const;
    bool ShowAuthors() const;
    bool ShowBooks() const;
    bool ShowAuthorBooks() const;
    bool DeleteAuthor(std::istream& cmd_input) const;
    bool EditAuthor(std::istream& cmd_input) const;
    bool DeleteBook(std::istream& cmd_input) const;
    bool EditBook(std::istream& cmd_input) const;
    std::string GetAuthorName() const;
    bool ShowBook(std::istream& cmd_input) const;
    void PrintBooks(const std::vector<items::BookInfo>& books) const;
    void PrintAuthorBooks(const std::vector<items::BookInfo>& books) const;
    void PrintAuthors(const std::vector<items::AuthorInfo>& authors) const;
    void PrintBook(const items::BookInfo& book, const std::string& book_tags) const;
    std::optional<items::BookInfo> SelectBookFromList(std::vector<items::BookInfo>& books) const;

    std::optional<detail::AddBookParams> GetBookParams(std::istream& cmd_input) const;
    std::optional<std::string> AddBookAuthor() const;
    std::optional<std::string> SelectAuthor() const;
    std::optional<items::BookInfo> SelectBook() const;
    std::vector<items::AuthorInfo> GetAuthors() const;
    std::vector<items::BookInfo> GetBooks() const;
    std::vector<items::BookInfo> GetAuthorBooks(const std::string& author_id) const;
    std::vector<std::string> GetTags(const std::string& curr_tags = "") const;
    void GetNewBookInfo(items::BookInfo& curr_info) const;

    menu::Menu& menu_;
    app::UseCases& use_cases_;
    std::istream& input_;
    std::ostream& output_;
};

}  // namespace ui