#define childSpread childSpreadEnglish
#include "child_text.hpp"
#include "stories.hpp"

namespace biblioteca {
const AdultSpread& childSpread(int book,int spread){
    using Story=std::array<AdultSpread,spreads>;
    static constexpr std::array<Story,7> stories{{
        Story{{
            {"Smoke above the valley","Ines saw smoke near a mountain. The photo came from a camera in space.","He asked for another photo. He wanted to know if the smoke came out of the ground."},
            {"Two Photos","Both photos looked the same. Ines used a camera that could see the heat.","The fire went up the mountain. The wind was pushing him into town."},
            {"Under the cloud","A cloud covered the valley. The team found photos from earlier.","The heat chamber gave another lead. There was fire at the dry river."},
            {"A radio map","The satellite was about to be hidden behind the mountain. That's a little while to send the map.","Ines first sent a small copy. The fire department was clear and clear."},
            {"The closed bridge","A car with a camera saw a tree fall onto the bridge. That's a bad place to get through.","The team found another way across the river."},
            {"Another View","A photo was taken from a ship. He was showing smoke where nobody had seen him before.","The trucks could have moved their way. At late afternoon, the new path remained free."},
            {"The mark of fire","Three days later, Ines saw the mountain again. The burned ground looks dark.","He put his photos back and forth. That's how far the fire had come."},
            {"The river back","The following year, water returned to the river. Some plants were already growing.","Ines drew the sprouts on his map. That's how his life came back."}
        }},
        Story{{
            {"A river of ice","Mateo saw a dark line in Europe, a moon of Jupiter. It felt like a frozen river.","He put his place on the map. I wanted to look at the crack again."},
            {"Two Viewpoints","The ship turned and the crack came from the photo. Luckily, there was a little bit at the edge.","Mateo put together two photos. At second place, a shadow shown that the ice was up there."},
            {"The dark shore","The antenna only got half the photo. The shadow part was still missing.","The team requested another hearing. I needed to know if a robot could come down without falling."},
            {"A glow that moves","A light point changed place as the ship turned. He looked like a mirror under the sun.","Mateo compared his photos. The glow moved. The crack remained at the same place."},
            {"Ice Under Test","On Earth they did a path with test ice. The car slipped near the edge.","They tried for a bigger share. The tires have been passing with care this time."},
            {"A rock on the map","The satellite sent a final photo. At the end of the crack was a dark rock.","Mateo tagged her. The rock could help find the way."},
            {"Two Routes","The rocket would have a copy of the map to the next team.","There was a path by the crack and another way. They could choose to see the ice."},
            {"The blue fringe","The sun came up onto the edge. The ice shines with blue.","Mateo put up his photo. Now I could see the crack from one shore to another."}
        }},
        Story{{
            {"Do you hear me?","Aurora was a satellite. When she came out of the shade, her panels came back to light.","Vera heard a beep eight seconds late. He looked at his watch and tested again."},
            {"On the roof","Vera's up to the roof. That's another dish-shaped antenna.","He heard the same beeping late. I knew the first antenna was not the cause."},
            {"A loose piece","Another ship took a photo from his side. One piece moved next to Aurora's camera.","The piece covered up the stars Aurora used to figure out where she was looking."},
            {"A small turn","On Earth, Vera turned a copy of Aurora. He found a place where light came in.","He asked the ship to do the same turn. The beep's back in time!"},
            {"The piece at the table","An engineer tested a copy of the piece at his table. He moved her and tipped his edge.","He tipped his end a little and tested again. That time there was space between the pieces."},
            {"Two Marks","The rocket had a new piece with two painted stripes.","The camera looked at his stripes. If they parted, they would know that the piece had been bent."},
            {"Sunlight and Shadow","The piece passed from the sun's heat to the cold shade. The stripes followed his place.","Aurora could have been looking at the stars with nothing to do with his camera."},
            {"A beep in time","Vera turned the radio on as the sun came up. Aurora's beep came at his hour.","He put it away to remember how a piece so small could change the whole journey."}
        }},
        Story{{
            {"First Tracks","Nadia studied the rover's tire tracks. Sunlight painted the stones orange.","The rover stopped beside a crack. Nadia searched for a flatter route across."},
            {"Turn around","A photo from above shown rock steps that the robot couldn't see.","Nadia drew a ride up a firmer ground."},
            {"A Slipping Wheel","On his way up, a wheel slipped and put up powder. Below was a dark stone.","Nadia put the robot back. In his second attempt, he surrounded the stone."},
            {"Photos by Radio","A ship passed onto the robot. I could send photos to Earth for a few minutes.","Nadia received a photo with clear stripes on the rock. He wanted to see her closely."},
            {"The striped stone","The robot's arm put his camera down to a Red Stone. He had colorful stripes.","His drill pulled a little bit. Nadia saved a photo of his place."},
            {"A cloud of dust","The sky became dark. Nadia let the robot wait at a flat place.","When the powder fell, the camera saw the prints of the road again."},
            {"The Earth hears","An antenna on Earth received the photos that the robot had saved.","In a photo, the way back was still clear."},
            {"The Red Route","At dawn, the robot went up again. From above you saw the edge of a big hole.","Nadia put up his map. The prints told where his robot had been."}
        }},
        Story{{
            {"A glow by the Moon","Iris saw a little bit of rock near a ship's path.","The ship waited while Iris requested another photo. The rock was coming his way."},
            {"A rock that turns","A camera looked at the rock from both sides. He was small and brilliant and turned so fast.","Iris chose to put the ship away. It was safer than guessing how big a rock was."},
            {"A Radio Bulletin","An antenna sent the new way to the ship. From Earth they have been expecting his answer.","When the answer came, Iris put together his old and new path."},
            {"Three Blinks","The camera saw three lights. It was the light of the Sun on a corner of the rock.","When we saw another flash, the team could have known where it was going."},
            {"A soft push","The ship turned its engines on for a few seconds to get a little back.","Iris looks at new photos. The rock would come from behind with plenty of space."},
            {"The friendly ship","Another ship came with a new crew. He had the path Iris had drawn.","The two ships met and they still had fuel to get back."},
            {"The rock goes far","Two cameras measured where the rock was. They both saw his way back.","Iris got his notice. He saved his photos to explain how they had known."},
            {"Four legs on the moon","The ship came down with a big light hole. One foot touched the powder before the other.","Iris got a photo from the ground. They saw prints, a round house and two antennas."}
        }},
        Story{{
            {"Saturn Up Close","Leo got a message from his ship. He was about to come to SaturN, the planet of great rings.","The ship saw how the rings drew a thin shade onto the planet."},
            {"A quick photo","The ship had little time to take a photo of Saturn. He pointed his camera and waited.","The second attempt, the photo went well. He saw the whole light border."},
            {"A photo in parts","The first photo came in two parts. At first we didn't know what was earlier.","Leo looked at the time written at every place. That's how she could put them back together."},
            {"Behind the planet","On the other side of Saturne, the ship was dark. Leo put out some equipment.","With the remaining power, the ship was able to save another photo."},
            {"A spark","Reeling to the Sun, the ship shone as a point by the ring.","Leo compared his photo with his map. The ship was still on its way."},
            {"A cloud of ice","A new image saw a bright fringe next to a Saturn moon.","Leo put the place up. He wanted to know if that moon was throwing some ice."},
            {"Saving Two Copies","The next rocket would have a camera that could have more photos.","Leo asked to put his time at every place. That's how they could get them together if they parted again."},
            {"The photo of the morning","The last photo came at dawn. One side of the rings was golden and the other was dark.","Leo sent her with his hour. Near the border was the shade of a ring."}
        }},
        Story{{
            {"The Sun Stirs","Amalia saw a big flash at the Sun. Shortly after, several radios stopped listening.","He wrote down his hour of flash and his hour of cuts. He wanted to know if they had happened together."},
            {"The Rooftop Antenna","A school had an antenna on its roof. His cable was broken by the sun's heat.","The teacher put up a new cable. At nightfall they have been expecting a message from the satellite."},
            {"A bridge that's gone","The map sent messages to a bridge that no longer existed. That was an old map.","Amalia requested a new photo. He drew the school as his new meeting place."},
            {"A box in the yard","A small car brought a radio box to the yard. His camera read his label.","The teacher saw that was the right box. They could send messages by now."},
            {"Two schools listen","A ship passed over the valley and sent an announcement to two schools.","The two teachers read the message and put their time in their notebook."},
            {"Another way to message","Amalia asked another ship to copy the notice. He wanted to prove his help.","He turned his first radio off for a minute. The schools continued to hear the message."},
            {"Replacement equipment","Ground they prepared a rocket with another radio box. Amalia put his map right next to him.","The next announcement could have been a two-way ride if one stopped functioning."},
            {"The Sun Calms Down","When the radios returned, the ship took a new photo of the Sun. The bright area had been calm.","Amalia put together the photo and history of both schools."}
        }}
    }};
    return stories[static_cast<int>(destination(book))][spread];
}
}
