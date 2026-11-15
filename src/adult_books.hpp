#pragma once
#include "library.hpp"

namespace biblioteca {
struct AdultSpread {std::string_view title,left,right;};
struct AdultBook {
    const char* title;
    const char* genre;
    const char* description;
    const char* binding;
    std::array<AdultSpread,adultSpreads> chapters;
};
const std::array<AdultBook,originalAdultCount>& adultBooks();
}
