#pragma once
#include <pqxx/pqxx>

#include "app/use_cases_impl.h"
#include "postgres/postgres.h"

namespace bookypedia {

struct AppConfig {
    std::string db_url;
};

class Application {
public:
    explicit Application(const AppConfig& config);

    void Run();

private:
    postgres::Database db_;
    std::unique_ptr<app::UnitOfWorkFactory> factory_ =
            std::make_unique<postgres::UnitOfWorkFactoryImpl>(db_.GetConnection());
    app::UseCasesImpl use_cases_{factory_.get()};
};

}  // namespace bookypedia
