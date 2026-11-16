#define chapter chapterEnglish
#define destinationArt destinationArtEnglish
#define wordlessBinding wordlessBindingEnglish
#include "stories.hpp"
#include <array>

namespace biblioteca {
namespace {
using F=Figure;
using Story=std::array<Chapter,8>;
constexpr std::array<Story,7> stories{{
    Story{{
        {"The First Image","The observatory awoke about the Pacific. His camera distinguished a column of smoke at the edge of his saw.",
         "Ines requested another pass. If the smoke remained attached to the ground, the fire had begun before dawn.",F::Earth},
        {"Two images of fire","The repeated satellite received two photographs that seemed identical. Ines turned on his thermal channel to compare them.",
         "The glow had moved 3 km up. The wind brought fire to the village.",F::Satellite},
        {"The shadow of the cloud","The cloud covered the valley in the next passing. The team sought an earlier measurement as a reference.",
         "The probe brought an infrared band. Under the cloud came a hot front following the dry river bed.",F::Probe},
        {"Contact minutes","The mountain station had only seven minutes before losing the satellite after the horizon.",
         "Ines first sent his light map. The fire department was able to see him while the complete data continued to come.",F::Dish},
        {"The missing part","A monitoring vehicle found the bridge cut by trees. The suggested path from space ended up there.",
         "The team crossed the map with the car's images and opened a way across the river.",F::EarthFire},
        {"The Last Delivery","From the orbital station a crew member took an oblique photo showing smoke on the dashboard that the map hidden.",
         "The image came in time to move the trucks. At dusk the new path remained clear.",F::Station},
        {"Medir lo perdido","Three days later, Ines requested the same cover from the first morning. The comparison showed where the scar was about to end.",
         "At the file she held both shots. The ranger could walk step by step the advance of the fire.",F::EarthNight},
        {"A different valley","The following year, the instrument looked back at the valley. The slope was still dark, but the river was already bringing water.",
         "Ines added a cape to his map. Where earlier I had computed fire velocity, she now noted the return of plants.",F::Telescope}
    }},
    Story{{
        {"The blue cracks","Mateo saw a dark line on the ice of Europe. He was changing his thickness just where two fractures were found.",
         "He set the crossing for a second passing. From the probe, the relief was a frozen river.",F::Europa},
        {"The second passing","The camera lost its frame as it turned onto the moon. He still had a narrow fringe of his first image.",
         "Mateo compared both shots and found his line. The light from another angle confirmed that there was a crest.",F::Imager},
        {"A listening antenna","The antenna received an incomplete map of the ground. The fracking that was shaded was missing.",
         "The team first requested an elevation of that shore. They needed to know how far from the plane of descent.",F::EuropaDescent},
        {"Shadow on the ice","The probe lowered its passing angle to follow both crest at once. A bright spot moved with the Sun.",
         "Mateo ruled out his reflection and preserved the rest of his image. The actual fracture remained set at both maps.",F::Probe},
        {"Test","A car on Earth tested the path about an ice model. The first slope sent him to the border.",
         "To change the entrance a few degrees caused the wheels to cross through a broad stretch. The essay was noted.",F::Rover},
        {"The Replay","The satellite's taken one of the day's last transmissions. The crest was close to an area of exposed rock.",
         "Mateo added that reference to his path. They could use it even if the bright ground hidden the relief.",F::Satellite},
        {"The map that travels","The rocket that the next team would carry was waiting with the map and test images loaded.",
         "The copy of the map was leading both routes. One followed the crack and the other moved apart if they lost their link.",F::Rocket},
        {"The light fringe","They saw Europe once again as the Sun reached the edge of the fracture. The ice returned a blue fringe.",
         "Mateo put his place next to his plans. From the probe bridge you saw how the ice was broken.",F::EuropaCrevasse}
    }},
    Story{{
        {"They hear me, right?","Aurora deployed his panels after his eclipse. The current was right, but the beacon was eight seconds late.",
         "Vera repeated his warrant and noted his hour. The delay was too regular to blame the silence of space.",F::Satellite},
        {"One hour at the roof","The main antenna was busy. Vera went up to the roof and lit a parabolic that had been unused for months.",
         "The second station was also delayed. The wrong watch was on the satellite.",F::Dish},
        {"A photo with a contras","The orbital observatory took a photo of the singing team. A loose plate scattering the sensor that stars were after.",
         "Aurora rectified his orientation every eight seconds and paused his beacon by doing so. The jam was mechanical.",F::Telescope},
        {"A short move","Vera tested a short turn with the probe's replica. The light pitted the sensor from a side that the plate did not cover.",
         "He asked for the same orbit turn. For the first time in 6 hours, the beacon arrived without delay.",F::Probe},
        {"A piece with his hands","The crew at the station received a copy of the plate. When she vibrates, she pitted the support as she did in the image.",
         "An engineer bent his tab at the assembly bench and repeated his test. Vera saved the correction.",F::OrbitalBuild},
        {"The final review","The new vehicle came up with an increased tab and two brands that gave up any bending.",
         "The inspection camera took images while they were deployed. The brands have been keeping their distance.",F::Rocket},
        {"To look at the Sunrise","The thermal sensor followed the piece through two light lights. The metal changed its temperature but preserved its shape.",
         "Aurora repeated the test maneuver and regained his direction in a single attempt.",F::Imager},
        {"Tomorrow's beacon","Vera left the automatic station as the first light came out. From the parking lot, she heard an announcement about a passing without mistakes.",
         "He saved his audio. The new piece came out in the service capsule headed to the port of the station.",F::OrbitalDock}
    }},
    Story{{
        {"The first roll","Nadia saw the first Rover prints on Mars. The low light painted with an orange crest in front of the wheels.",
         "The Rover stopped at a crack. The next day I'll try to cross a more flat range.",F::Mars},
        {"The deviation","The orbital observer showed that the ground was falling into steps behind the border. The Rover's camera was only about to see his first.",
         "Nadia set up a rough ground rodeo. There was an open area that also kept his antenna in light of the sky.",F::Imager},
        {"Wheel at the slopes","The Rover advanced short and lifted up a cloud of fine powder. A wheel slipped as it passed onto a dark rock.",
         "Nadia put him back. The second attempt tipped the rock and left 6 clear prints on the coast.",F::Mars},
        {"The radio window","The repeater passed above the crater with a few minutes of link. The Rover first prepared the smaller photos.",
         "One of them had clear layers on his wall. The following order changed his destination at the foot of his stratum.",F::Satellite},
        {"A different rock","The vehicle spread the camera near the ground. A rock with thin and clear bands appeared on the red powder.",
         "The drill took the sample at the foot of the stratum. Nadia put his place next to his ravine photo.",F::MarsQuarry},
        {"The powder's coming.","A dark fringe covered the sky at noon. Nadia put the team to rest on an outstanding ground.",
         "When the powder settled, the camera recognized the crest. The wheels were still about to come back.",F::MarsStorm},
        {"The voice of the Earth","With a short link, the Earth's antenna retrieved the images that the Rover had preserved from the storm.",
         "The last photo shown an intact path. They could leave without touching the marks that guided their return.",F::Dish},
        {"The Red Way","At dawn, the Rover returned to the high end. He stopped where the ridge cut the sky onto the crater.",
         "Nadia left the complete path in his file. The prints were the most direct record they had found.",F::MarsSummit}
    }},
    Story{{
        {"A Shadow On The Moon","Iris followed his descent to the Moon as a fragment appeared on his intended path to his relief.",
         "The module was in orbit. A second take showed that the object was nearing his path.",F::Moon},
        {"Unknown size","The monitoring probe measured the fragment from two angles. He was small and brilliant and turned too fast.",
         "Iris calculated a bigger area of uncertainty. I'd rather put the ship back in time than figure out the size.",F::Probe},
        {"Wait for notice","An antenna was tipped up minutes before the change. From the valley they called to confirm the package.",
         "The ship charged the receipt. Iris put his earlier path on his map to compare his separation.",F::Dish},
        {"The Night shift camera","The observatory caught the fragment after a light from the station. A little corner shone three times.",
         "The flash was repeated in the next cycle. They were able to estimate how she turned and how far she moved.",F::Imager},
        {"Corregir rumbo","The station turned its engines on for a few seconds. No need to change the schedule for the next docks.",
         "Iris's been waiting for his reading. The fragment would pass after the lab with plenty of space.",F::Station},
        {"Reg crew","The launch vehicle received a new path. His crew found the station exactly where I was expecting them.",
         "The maneuver ended with an almost intact deposit. They have been able to keep the fuel for their turn.",F::MoonCapsule},
        {"Volver a medir","The repeated satellite gave measurements of two observatories. Together they shown that the fragment would no longer cross the path.",
         "Iris put an end to his alert but held his turn with all the points that had led to his decision.",F::Satellite},
        {"The descend at dawn","Once the path was clear, the module came down to the sunny edge of the crater. One of his legs touched before the other.",
         "Iris got his first photo next to his polar stand. The dome and prints and both antennas were ready.",F::MoonOutpost}
    }},
    Story{{
        {"The entrance to the rings","Leo got a pulse from his probe before his meeting with SaturN. The navigation part was incomplete.",
         "On SaturN's disc, the probe saw a thin shade of the rings. He continued to have data collection during his contact.",F::Saturn},
        {"The face of the planet","The probe passed with SaturN with his camera on. I had less than a minute to get an approach.",
         "The shooter responded to his second attempt. The light border entered completely into the frame.",F::Probe},
        {"The broken file","A repeater received a copy of the first measures. A memory malfunction had broken the file into two.",
         "Leo checked his time. He could assemble his reading without inventing his lost intervals.",F::Satellite},
        {"When the sun went","The ship was shaded across the planet. Leo pulled from his tail every command they could have been expecting.",
         "The exit maneuver consumed less energy than intended. The camera held a final display.",F::Station},
        {"A spark in orbit","The monitoring telescope saw the ship's flash coming out of the eclipse. It was a point at the end of the ring.",
         "The photo confirmed his path. The probe was still with his home-oriented antenna.",F::Telescope},
        {"A column of ice","The detailed image came in several pieces. A clear line was curved behind the light border.",
         "Leo pointed at the curve near one of the moons. That was their first lead from scattering ice.",F::SaturnMoons},
        {"Backup","A next rocket would have a more memory camera. Leo taught the team how his image had been rebuilt.",
         "The new design would put a copy of the hour in each row. If something broke, they would still know where to look.",F::Rocket},
        {"sign back","The antennae just got his photo after his clearance. The rings appeared golden on one side and dark on the other.",
         "Leo sent his photo with his first pulse. At an edge, the probe's antenna crossed the ring's shadow.",F::SaturnCrossing}
    }},
    Story{{
        {"The storm at the crown","Amalia's observatory spotted a solar rash. Several radio links were discontinued soon after.",
         "Amalia activated the monitoring protocol. The activity of the Sun and the status of links would be put into the same report.",F::Sun},
        {"A loaned antenna","At the end of the valley, a school held a parabolic at the roof. Her cable was cracked by the Sun.",
         "The teacher replaced him and tipped his phone. At 6 p.m. they were expecting the first package of the satellite.",F::Dish},
        {"The wrong message","A sensor sent coordinates from a bridge that no longer existed. The map I was using was four.",
         "Amalia asked for new images, signaled the purified bridge and moved his surrender point to the school yard.",F::Imager},
        {"A box with wheels","The delivery vehicle went through the yard with an onboard communications module. His camera read his code in his box.",
         "The code coincided with the teacher's list. They could start passing without registering another equipment.",F::Rover},
        {"Two countries at the same time","The orbital station delivered the package during a passing over the border. The sign came at once to two valleys.",
         "The messages reached both schools. The teachers compared the hour with that of the solar pounding.",F::SolarFlare},
        {"Test Replay","Amalia asked the test probe to copy the package if the main station failed.",
         "They put out the transmitter for a minute. Schools continued to hear full notice.",F::Probe},
        {"The next launch","A replacement equipment was currently at the platform. Amalia corrected the map and put it next to the mission's file.",
         "Each station had an extra frequency for the next grown up. That would have been two ways from day one.",F::Rocket},
        {"Last note","When the Earth's network returned, the observatory took another image of the Sun. The area of the eruption was clear.",
         "Amalia saved his image in his technical file. The observatory's shield was still oriented to the crown. Amalia archived his reading with the drawing of the valley.",F::SolarCorona}
    }}
}};
}

const Chapter& chapterEnglish(int book,int spread){return stories[book%stories.size()][spread];}
}
