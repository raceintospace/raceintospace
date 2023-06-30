#ifndef RIS_SETTINGS
#define RIS_SETTINGS

#include "serialize.h"

/**
 * Audio configuration settings used for recording and loading user
 * preferences configurable in-game. These are distinct from "options",
 * which represent configuration settings inaccessible from the main
 * game.
 *
 * This class is intended for use with user menus and initializing
 * audio settings, not for being consulted in-game. RIS should prefer
 * calls to music.h in-game.
 */
struct AudioConfig {
    struct Channel {
        bool muted;
        int volume;

        template<class Archive>
        void serialize(Archive &ar, uint32_t const version)
        {
            ar(CEREAL_NVP(muted));
            ar(CEREAL_NVP(volume));

            ASSERT(volume >= 0 && volume <= 100);
        }
    };

    Channel master, music, soundFX;

    template<class Archive>
    void serialize(Archive &ar, uint32_t const version)
    {
        ar(CEREAL_NVP(master));
        ar(CEREAL_NVP(music));
        ar(CEREAL_NVP(soundFX));
    }
};


AudioConfig LoadAudioSettings();
void SaveAudioSettings(const AudioConfig &settings);


#endif // RIS_SETTINGS
