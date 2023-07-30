#include "view.h"

#include <boost/algorithm/string.hpp>
#include <cassert>
#include <iostream>
#include <set>
#include "../menu/menu.h"

using namespace std::literals;
namespace ph = std::placeholders;

namespace ui {
namespace detail {

std::ostream& operator<<(std::ostream& out, const items::AuthorInfo& author) {
    out << author.name;
    return out;
}

std::ostream& operator<<(std::ostream& out, const items::BookInfo& book) {
    out << book.title << ", " << book.publication_year;
    return out;
}

}  // namespace detail

View::View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output)
    : menu_{menu}
    , use_cases_{use_cases}
    , input_{input}
    , output_{output} {
    menu_.AddAction("AddAuthor"s, "name"s, "Adds author"s, std::bind(&View::AddAuthor, this, ph::_1));
    menu_.AddAction("AddBook"s, "<pub year> <title>"s, "Adds book"s, std::bind(&View::AddBook, this, ph::_1));
    menu_.AddAction("ShowAuthors"s, {}, "Show authors"s, std::bind(&View::ShowAuthors, this));
    menu_.AddAction("ShowBooks"s, {}, "Show books"s, std::bind(&View::ShowBooks, this));
    menu_.AddAction("ShowAuthorBooks"s, {}, "Show author books"s, std::bind(&View::ShowAuthorBooks, this));
    menu_.AddAction("DeleteAuthor"s, {}, "Delete author"s, std::bind(&View::DeleteAuthor, this, ph::_1));
    menu_.AddAction("EditAuthor"s, {}, "Edit author"s, std::bind(&View::EditAuthor, this, ph::_1));
    menu_.AddAction("ShowBook"s, {}, "Show book"s, std::bind(&View::ShowBook, this, ph::_1));
    menu_.AddAction("DeleteBook"s, {}, "Delete book"s, std::bind(&View::DeleteBook, this, ph::_1));
    menu_.AddAction("EditBook"s, {}, "Edit book"s, std::bind(&View::EditBook, this, ph::_1));
}

void View::PrintAuthors(const std::vector<items::AuthorInfo> &authors) const {
    int author_num = 1;
    for (auto & author: authors)
        output_ << author_num++ << " " << author.name << std::endl;
}

void View::PrintBooks(const std::vector<items::BookInfo> &books) const {
    int book_num = 1;
    for (auto & book: books)
        output_ << book_num++ << " " << book.title << " by " << book.author_name << ", " << book.publication_year << std::endl;
}

void View::PrintAuthorBooks(const std::vector<items::BookInfo> &books) const {
    int book_num = 1;
    for (auto & book: books)
        output_ << book_num++ << " " << book.title << ", " << book.publication_year << std::endl;
}

void View::PrintBook(const items::BookInfo &book, const std::string &book_tags) const {
    output_ << "Title: " << book.title << "\nAuthor: " << book.author_name <<
        "\nPublication year: " << book.publication_year << std::endl;
    if (!book_tags.empty())
        output_ << "Tags: " << book_tags << std::endl;
}

bool View::AddAuthor(std::istream& cmd_input) const {
    try {
        std::string name;
        std::getline(cmd_input, name);
        boost::algorithm::trim(name);
        if (name.empty())
            throw std::invalid_argument("Invalid author name");
        auto add_res = use_cases_.AddAuthor(name);
        if (!add_res.has_value())
            throw std::runtime_error("Failed to add author");
        use_cases_.EndTransaction();
    } catch (const std::exception&) {
        output_ << "Failed to add author"sv << std::endl;
        use_cases_.CancelTransaction();
    }
    return true;
}

bool View::AddBook(std::istream& cmd_input) const {
    try {
        if (auto params = GetBookParams(cmd_input)) {
            auto book_id = use_cases_.AddBook(params->title, params->publication_year, params->author_id);
            if (book_id.has_value()) {
                AddBookTags(book_id.value());
                use_cases_.EndTransaction();
            } else
                throw std::runtime_error("Failed to add book to db");
        } else
            throw std::runtime_error("params: ");
    } catch (const std::exception& e) {
        std::string error = e.what(), cancel = "cancel";
        if (error != cancel)
            output_ << "Failed to add book: "sv << e.what() << std::endl;
        use_cases_.CancelTransaction();
    }
    return true;
}

std::vector<std::string> remove_duplicates(std::vector<std::string>&& vec) {
    std::set<std::string> s{vec.begin(), vec.end()};
    std::vector<std::string> res;
    for (auto & tag: s)
        if (!tag.empty())
            res.push_back(tag);
    return res;
}

std::vector<std::string> View::GetTags(const std::string& curr_tags) const {
    if (curr_tags.empty())
        output_ << "Enter tags (comma separated):" << std::endl;
    else
        output_ << "Enter tags (current tags: " << curr_tags << "):" << std::endl;
    std::string tags_string;
    std::getline(input_, tags_string);
    std::vector<std::string> tags{};
    if (tags_string.empty())
        return tags;
    boost::algorithm::split(tags, tags_string, boost::is_any_of(","));
    for (auto & tag: tags)
        boost::algorithm::trim(tag);
    return remove_duplicates(std::move(tags));
}

bool View::AddBookTags(const std::string& book_id) const {
    try {
        auto tags = GetTags();
        if (!tags.empty()) {
            use_cases_.AddBookTags(book_id, tags);
            use_cases_.EndTransaction();
        }
    } catch (const std::exception& e) {
        use_cases_.CancelTransaction();
        std::string error = e.what();
        throw std::runtime_error(error);
    }
    return true;
}

bool View::ShowAuthors() const {
    auto authors = GetAuthors();
    PrintAuthors(authors);
    return true;
}

bool View::ShowBooks() const {
    auto books = GetBooks();
    std::sort(books.begin(), books.end(), [](auto &l, auto &r){
        if (l.title == r.title) {
            char new_c_l = std::tolower(l.author_name[0]), new_c_r = std::tolower(r.author_name[0]);
            std::string new_name_l, new_name_r;
            new_name_l += new_c_l;
            new_name_l += l.author_name.substr(1);
            new_name_r += new_c_r;
            new_name_r += r.author_name.substr(1);
            return new_name_l < new_name_r;
        }
        return l.title < r.title;
    });
    PrintBooks(books);
    return true;
}

bool View::ShowAuthorBooks() const {
    try {
        if (auto author_id = SelectAuthor()) {
            auto author_books = GetAuthorBooks(*author_id);
            PrintAuthorBooks(author_books);
        }
    } catch (const std::exception& e) {
        throw std::runtime_error("Failed to Show Books");
    }
    return true;
}

bool View::DeleteAuthor(std::istream& cmd_input) const {
    std::string name;
    std::getline(cmd_input, name);
    boost::algorithm::trim(name);
    if (name.empty()) {
        try {
            auto author_id = SelectAuthor();
            if (author_id.has_value()) {
                use_cases_.DeleteAuthor(author_id.value());
                use_cases_.EndTransaction();
            } else
                throw std::runtime_error("Invalid author id");
        } catch (const std::exception& e) {
            output_ << "Failed to delete author: " << e.what() << std::endl;
            use_cases_.CancelTransaction();
        }
    } else {
        try {
            auto author = use_cases_.FindAuthorByName(name);
            if (author.has_value()) {
                use_cases_.DeleteAuthor(author->id);
                use_cases_.EndTransaction();
            }
            else
                throw std::runtime_error("Author does not exist");
        } catch (const std::exception& e) {
            output_ << "Failed to delete author: " << e.what() << std::endl;
            use_cases_.CancelTransaction();
        }
    }
    return true;
}

std::string View::GetAuthorName() const {
    std::string new_name;
    output_ << "Enter new name: " << std::endl;
    std::getline(input_, new_name);
    boost::algorithm::trim(new_name);
    if (new_name.empty())
        throw std::runtime_error("Author name is empty");
    return new_name;
}

void View::GetNewBookInfo(items::BookInfo& curr_info) const {
    std::string title, year;
    try {
        output_ << "Enter new title or empty line to use current one ("
                << curr_info.title << "):" << std::endl;
        std::getline(input_, title);
        if (!title.empty()) {
            boost::algorithm::trim(title);
            curr_info.title = title;
        }
        output_ << "Enter publication year or empty line to use the current one ("
                << curr_info.publication_year << "):" << std::endl;
        std::getline(input_, year);
        if (!year.empty()) {
            int publication_year = std::stoi(year);
            curr_info.publication_year = publication_year;
        }
    } catch (const std::exception& e) {
        output_ << "Failed to edit book: " << e.what() << std::endl;
    }
}

bool View::EditAuthor(std::istream& cmd_input) const {
    std::string name;
    std::getline(cmd_input, name);
    boost::algorithm::trim(name);
    if (name.empty()) {
        try {
            auto author_id = SelectAuthor();
            if (author_id.has_value()) {
                use_cases_.EditAuthor(author_id.value(), GetAuthorName());
                use_cases_.EndTransaction();
            } else
                throw std::runtime_error("Invalid author id");
        } catch (const std::exception& e) {
            output_ << "Failed to edit author: " << e.what() << std::endl;
            use_cases_.CancelTransaction();
        }
    } else {
        try {
            auto author = use_cases_.FindAuthorByName(name);
            if (author.has_value()) {
                use_cases_.EditAuthor(author->id, GetAuthorName());
                use_cases_.EndTransaction();
            }
            else
                throw std::runtime_error("Author does not exist");
        } catch (const std::exception& e) {
            output_ << "Failed to edit author: " << e.what() << std::endl;
            use_cases_.CancelTransaction();
        }
    }
    return true;
}

std::string TagsToString(const std::vector<std::string>& tags) {
    std::string tag_str;
    for (auto & tag: tags) {
        if (tag.empty())
            continue;
        tag_str += (tag + ", ");
    }
    if (!tag_str.empty())
        tag_str = tag_str.substr(0, tag_str.size() - 2);
    return tag_str;
}

bool View::ShowBook(std::istream &cmd_input) const {
    std::string title;
    std::getline(cmd_input, title);
    boost::algorithm::trim(title);
    if (title.empty()) {
        try {
            auto book = SelectBook();
            if (book.has_value()) {
                auto book_tags = use_cases_.GetBookTags(book.value().id);
                auto book_tags_str = TagsToString(book_tags);
                PrintBook(book.value(), book_tags_str);
            }
        } catch (const std::exception& e) {
            output_ << "Failed to find book: " << e.what() << std::endl;
        }
    } else {
        try {
            auto books = use_cases_.FindBookByTitle(title);
            if (books.empty())
                return true;
            if (books.size() == 1) {
                auto book = books[0];
                auto book_tags = use_cases_.GetBookTags(book.id);
                auto book_tags_str = TagsToString(book_tags);
                PrintBook(book, book_tags_str);
            } else {
                auto book = SelectBookFromList(books);
                if (book.has_value()) {
                    auto book_tags = use_cases_.GetBookTags(book.value().id);
                    auto book_tags_str = TagsToString(book_tags);
                    PrintBook(book.value(), book_tags_str);
                }
            }
        } catch (const std::exception& e) {
            output_ << "Failed to find book: " << e.what() << std::endl;
        }
    }
    return true;
}

bool View::DeleteBook(std::istream &cmd_input) const {
    std::string title;
    std::getline(cmd_input, title);
    boost::algorithm::trim(title);
    if (title.empty()) {
        try {
            auto book = SelectBook();
            if (book.has_value()) {
                use_cases_.DeleteBook(book.value().id);
                use_cases_.EndTransaction();
            }
        } catch (const std::exception& e) {
            output_ << "Failed to delete book: " << e.what() << std::endl;
            use_cases_.CancelTransaction();
        }
    } else {
        try {
            auto books = use_cases_.FindBookByTitle(title);
            if (books.empty())
                return true;
            if (books.size() == 1) {
                auto book = books[0];
                use_cases_.DeleteBook(book.id);
                use_cases_.EndTransaction();
            } else {
                auto book = SelectBookFromList(books);
                if (book.has_value()) {
                    use_cases_.DeleteBook(book.value().id);
                    use_cases_.EndTransaction();
                }
            }
        } catch (const std::exception& e) {
            output_ << "Failed to delete book: " << e.what() << std::endl;
            use_cases_.CancelTransaction();
        }
    }
    return true;
}

bool View::EditBook(std::istream &cmd_input) const {
    std::string title;
    std::getline(cmd_input, title);
    boost::algorithm::trim(title);
    if (title.empty()) {
        try {
            auto book = SelectBook();
            if (book.has_value()) {
                GetNewBookInfo(book.value());
                use_cases_.EditBook(book.value());
                auto book_tags = use_cases_.GetBookTags(book->id);
                auto book_tags_str = TagsToString(book_tags);
                auto new_tags = GetTags(book_tags_str);
                use_cases_.EditBookTags(book.value().id, new_tags);
                use_cases_.EndTransaction();
            } else
                throw std::runtime_error("Book not found");
        } catch (const std::exception& e) {
            output_ << e.what() << std::endl;
            use_cases_.CancelTransaction();
        }
    } else {
        try {
            auto books = use_cases_.FindBookByTitle(title);
            if (books.empty())
                throw std::runtime_error("Book not found");
            if (books.size() == 1) {
                auto book = books[0];
                GetNewBookInfo(book);
                use_cases_.EditBook(book);
                auto book_tags = use_cases_.GetBookTags(book.id);
                auto book_tags_str = TagsToString(book_tags);
                auto new_tags = GetTags(book_tags_str);
                use_cases_.EditBookTags(book.id, new_tags);
                use_cases_.EndTransaction();
            } else {
                auto book = SelectBookFromList(books);
                if (book.has_value()) {
                    GetNewBookInfo(book.value());
                    use_cases_.EditBook(book.value());
                    auto book_tags = use_cases_.GetBookTags(book->id);
                    auto book_tags_str = TagsToString(book_tags);
                    auto new_tags = GetTags(book_tags_str);
                    use_cases_.EditBookTags(book.value().id, new_tags);
                    use_cases_.EndTransaction();
                } else
                    throw std::runtime_error("Book not found");
            }
        } catch (const std::exception& e) {
            output_ << e.what() << std::endl;
            use_cases_.CancelTransaction();
        }
    }
    return true;
}

std::optional<detail::AddBookParams> View::GetBookParams(std::istream& cmd_input) const {
    detail::AddBookParams params;

    cmd_input >> params.publication_year;
    std::getline(cmd_input, params.title);
    boost::algorithm::trim(params.title);

    auto author_id = AddBookAuthor();
    if (not author_id.has_value())
        return std::nullopt;
    else {
        params.author_id = author_id.value();
        return params;
    }
}

std::optional<std::string> View::AddBookAuthor() const {
    output_ << "Enter author name or empty line to select from list:" << std::endl;
    std::string author_name;
    std::getline(input_, author_name);
    if (author_name.empty()) {
        auto author_id = SelectAuthor();
        if (author_id.has_value())
            return author_id;
        else
            throw std::runtime_error("cancel");
    } else {
        boost::algorithm::trim(author_name);
        auto author = use_cases_.FindAuthorByName(author_name);
        if (author.has_value())
            return author.value().id;
        else {
            output_ << "No author found. Do you want to add " << author_name << " (y/n)?" << std::endl;
            std::string answer;
            std::getline(input_, answer);
            if (answer == "y" || answer == "Y") {
                try {
                    return use_cases_.AddAuthor(author_name);
                } catch (const std::exception& e) {
                    output_ << "Failed to add author" << std::endl;
                }
            }
            return std::nullopt;
        }
    }
}

std::optional<std::string> View::SelectAuthor() const {
    output_ << "Select author:" << std::endl;
    auto authors = GetAuthors();
    PrintAuthors(authors);
    output_ << "Enter author # or empty line to cancel" << std::endl;

    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return std::nullopt;
    }

    int author_idx;
    try {
        author_idx = std::stoi(str);
    } catch (std::exception const&) {
        throw std::runtime_error("Invalid author num");
    }

    --author_idx;
    if (author_idx < 0 or author_idx >= authors.size()) {
        throw std::runtime_error("Invalid author num");
    }

    return authors[author_idx].id;
}

std::optional<items::BookInfo> View::SelectBook() const {
    auto books = GetBooks();
    std::sort(books.begin(), books.end(), [](auto &l, auto &r){
        if (l.title == r.title) {
            char new_c_l = std::tolower(l.author_name[0]), new_c_r = std::tolower(r.author_name[0]);
            std::string new_name_l, new_name_r;
            new_name_l += new_c_l;
            new_name_l += l.author_name.substr(1);
            new_name_r += new_c_r;
            new_name_r += r.author_name.substr(1);
            return new_name_l < new_name_r;
        }
        return l.title < r.title;
    });
    PrintBooks(books);
    output_ << "Enter book # or empty line to cancel" << std::endl;

    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return std::nullopt;
    }

    int book_idx;
    try {
        book_idx = std::stoi(str);
    } catch (std::exception const&) {
        throw std::runtime_error("Invalid book num");
    }

    --book_idx;
    if (book_idx < 0 or book_idx >= books.size()) {
        throw std::runtime_error("Invalid book num");
    }

    return books[book_idx];
}

std::optional<items::BookInfo> View::SelectBookFromList(std::vector<items::BookInfo>& books) const {
    std::sort(books.begin(), books.end(), [](auto &l, auto &r){
        if (l.title == r.title) {
            char new_c_l = std::tolower(l.author_name[0]), new_c_r = std::tolower(r.author_name[0]);
            std::string new_name_l, new_name_r;
            new_name_l += new_c_l;
            new_name_l += l.author_name.substr(1);
            new_name_r += new_c_r;
            new_name_r += r.author_name.substr(1);
            return new_name_l < new_name_r;
        }
        return l.title < r.title;
    });
    PrintBooks(books);
    output_ << "Enter book # or empty line to cancel" << std::endl;

    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return std::nullopt;
    }

    int book_idx;
    try {
        book_idx = std::stoi(str);
    } catch (std::exception const&) {
        throw std::runtime_error("Invalid book num");
    }

    --book_idx;
    if (book_idx < 0 or book_idx >= books.size()) {
        throw std::runtime_error("Invalid book num");
    }

    return books[book_idx];
}

std::vector<items::AuthorInfo> View::GetAuthors() const {
    return use_cases_.GetAuthors();
}

std::vector<items::BookInfo> View::GetBooks() const {
    return use_cases_.GetBooks();
}

std::vector<items::BookInfo> View::GetAuthorBooks(const std::string& author_id) const {
    return use_cases_.GetAuthorBooks(author_id);
}

}  // namespace ui
