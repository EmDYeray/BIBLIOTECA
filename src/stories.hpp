#pragma once
#include <string_view>

namespace biblioteca {
enum class Figure { Satellite, Imager, Rocket, Station, Dish, Rover, Probe, Telescope,
                    Mars, Europa, Moon, Saturn, Earth, Sun,
                    MarsQuarry, EuropaCrevasse, MoonOutpost, SaturnCrossing, EarthNight, SolarCorona, OrbitalBuild,
                    EarthFire, EuropaDescent, OrbitalDock, MarsStorm, MoonCapsule, SaturnMoons, SolarFlare, MarsSummit,
                    ApolloEagle, ApolloIntrepid, ApolloFalcon, KeplerWatch, KeplerTransit, PioneerJupiter, Insight, TwinRovers,
                    PioneerLaunch, PioneerBelt, PioneerPlaque, PioneerSignal, KeplerBalance, KeplerField,
                    ApolloSamples, ApolloNetwork, ApolloSurveyor, ApolloImpact, ApolloHadley, ApolloDrop,
                    InsightWaves, InsightDrill, InsightDust, MarsTraverse, OrbitalSystems, ApolloEasep, InsightLayers, Count };
enum class Destination { Earth, Europa, Orbit, Mars, Moon, Saturn, Sun };
struct DestinationArt {std::string_view name,planet,light,shadow;};
Destination destination(int book);
const DestinationArt& destinationArt(int book);
struct Chapter {
    std::string_view title,left,right;
    Figure model;
};
const Chapter& chapter(int book,int spread);
bool wordlessBinding(int book);
}
