#pragma once

#include "geometry.hpp"

namespace biblioteca {
void hearthInlay(MeshData& out);
void ceilingInstrument(MeshData& out);
void pendant(MeshData& out,Vec at);
void chartRelief(MeshData& out,Vec at,bool technical,int layout);
}
