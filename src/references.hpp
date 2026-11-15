#pragma once
#include <array>
#include <string_view>

namespace biblioteca {
inline constexpr int referencePageWidth=760,referencePageHeight=820;
inline constexpr float referenceLinkY(int row){return row==0?.465f:.818f;}

struct Citation {
    std::string_view text;
    const char* url;
    std::string_view title;
};

inline constexpr std::array<std::array<Citation,4>,2> references{{
    {{
        {"Cermak, A. (2024b, noviembre 6). Pioneer 10 - NASA Science. NASA Science.",
         "https://science.nasa.gov/mission/pioneer-10/","Pioneer 10 - NASA Science"},
        {"Nasa. (2019). Kepler / K2 - NASA Science. Nasa. Science.Nasa.Gov.",
         "https://science.nasa.gov/mission/kepler/","Kepler / K2 - NASA Science"},
        {"NASA. (2019). Mars Exploration Rovers: Spirit and Opportunity - NASA Science. NASA. Science.Nasa.Gov.",
         "https://science.nasa.gov/mission/mars-exploration-rovers-spirit-and-opportunity/",
         "Mars Exploration Rovers: Spirit and Opportunity - NASA Science"},
        {"NASA. (2024, noviembre 3). InSight - NASA science. Science.Nasa.Gov.",
         "https://science.nasa.gov/mission/insight/","InSight - NASA science"}
    }},
    {{
        {"NASA. (2019b). NASA - NSSDCA - spacecraft - details. Nasa.Gov.",
         "https://nssdc.gsfc.nasa.gov/nmc/spacecraft/display.action?id=1969-059C","NASA - NSSDCA - spacecraft - details"},
        {"NASA - NSSDCA - spacecraft - details. (s. f.). Nssdc.Gsfc.Nasa.Gov. Recuperado 15 de noviembre de 2026, de",
         "https://nssdc.gsfc.nasa.gov/nmc/spacecraft/display.action?id=1971-063C","NASA - NSSDCA - spacecraft - details"},
        {"NASA - NSSDCA - spacecraft - details. (s. f.-b). Nssdc.Gsfc.Nasa.Gov. Recuperado 15 de noviembre de 2026, de",
         "https://nssdc.gsfc.nasa.gov/nmc/spacecraft/display.action?id=1969-099C","NASA - NSSDCA - spacecraft - details"},
        {"Surveyor 3 - NASA science. (s. f.). Science.Nasa.Gov. Recuperado 15 de noviembre de 2026, de",
         "https://science.nasa.gov/mission/surveyor-3/","Surveyor 3 - NASA science"}
    }}
}};
}
