#pragma once
#include "stories.hpp"
#include "child_text.hpp"
#include "adult_books.hpp"
#include "imported_books.hpp"
#include "technical_notes.hpp"

namespace biblioteca {
inline bool englishLanguage=true;
inline bool isEnglish(){return englishLanguage;}
inline void setEnglish(bool enabled){englishLanguage=enabled;}
const Chapter& chapterEnglish(int book,int spread);
const AdultSpread& childSpreadEnglish(int book,int spread);
const std::array<AdultBook,originalAdultCount>& adultBooksEnglish();
const std::array<ImportedBook,importedChildren+importedAdults>& importedBooksEnglish();
std::string_view technicalNoteEnglish(Destination place,int page);
}
