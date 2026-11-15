#pragma once
#include "geometry.hpp"
#include "stories.hpp"
#include <filesystem>

namespace biblioteca {
constexpr float paperGap=pageInner+.012f,foldedDepth=.009f;
constexpr float popStart=.64f,popTurnStart=.29f,popTurnEnd=.71f;
constexpr int popupCount=int(Figure::Count);
struct PopupAsset {std::string_view file,name;};
const std::array<PopupAsset,popupCount>& popupAssets();
int popupIndex(int book,int spread);
std::filesystem::path assetsPath();
enum class PopupMotion { Flight, Launch, Spin, Ringed, Drift, Roll, Beacon, Scan, Drop, Pulse, Slide };
struct PopupActor { PopupMotion motion; Vec pivot; MeshData mesh; };
struct PopupScene { MeshData resting; std::vector<PopupActor> actors; };
bool dynamicPopup(PopupMotion motion);
PopupScene loadPopup(const std::filesystem::path& obj);
std::array<MeshData,2> foldablePanels(const PopupScene& scene,bool includeRocket=true);
Mat animatePopup(const PopupActor& actor,float strength,float seconds);
float motionStrength(float opening,float progress,bool flipping);

struct ReadingState {
    int left,right;
    float deploy,leaf;
    bool turning;
    int attachedBookSpread;
    int restingBookSpread;
};
ReadingState readingState(int spread,int target,float progress,float opening);
std::array<MeshData,2> splitPaper(const MeshData& scene);
Mat paperPanel(float deployed,bool compact,int side);
MeshData attachToSheet(const MeshData& panel,float t,bool compact,int side);
}
