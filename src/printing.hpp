#pragma once
#include "native.hpp"
#include <string_view>
namespace biblioteca {
struct Color;
Color bindingColor(int book);
GLuint printSpine(int book);
GLuint printCover(int book,bool shelf=false);
GLuint printPage(int book,int page);
GLuint printRoomPlaque(std::string_view title);
GLuint printReferencePage(int page);
std::string_view pageEdge(int book);
}
