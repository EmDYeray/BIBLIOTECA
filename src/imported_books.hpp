#pragma once
#include "adult_books.hpp"

namespace biblioteca {
enum class Illustration { Eagle,Intrepid,Falcon,Kepler,Pioneer,Insight,Rovers,Earth,Europa,Orbit,Saturn,Sun };
struct ImportedBook {
    AdultBook text;
    Illustration art;
};
const std::array<ImportedBook,importedChildren+importedAdults>& importedBooks();
}
