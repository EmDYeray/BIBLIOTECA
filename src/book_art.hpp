#pragma once
#include "imported_books.hpp"
#include "paint.hpp"

namespace biblioteca {
Illustration illustrationFor(int book);
void illustrate(Paint& paint,Rect panel,Illustration art,int page,bool technical);
}
