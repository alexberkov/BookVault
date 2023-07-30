#pragma once
#include <string>

#include "../util/tagged_uuid.h"
#include "author.h"

namespace domain {

namespace detail {
struct BookTag {};
}  // namespace detail

using BookId = util::TaggedUUID<detail::BookTag>;

class Author;

class Book {
public:
    Book(BookId id, size_t year, std::string title, std::string author_id)
            : id_(std::move(id))
            , author_id_(AuthorId::FromString(author_id))
            , title_(std::move(title))
            , year_(year) {}

    const BookId& GetId() const noexcept {
        return id_;
    }

    const AuthorId& GetAuthorId() const noexcept {
        return author_id_;
    }

    const std::string& GetTitle() const noexcept {
        return title_;
    }

    const size_t& GetYear() const noexcept {
        return year_;
    }

private:
    AuthorId author_id_;
    BookId id_;
    std::string title_;
    size_t year_;
};

}  // namespace domain

