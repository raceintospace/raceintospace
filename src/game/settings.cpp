/*
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


#include "settings.h"

#include "fs.h"
#include "ioexception.h"
#include "logging.h"
#include "options.h"


namespace
{

void ResetAudioSettings(AudioConfig &audio);

};


/**
 * Reads the audio configuration file to get the current audio settings.
 *
 * This checks the options.want_audio configuration setting, so it
 * should not be called until 'options' have been loaded.
 */
AudioConfig LoadAudioSettings()
{
    AudioConfig audio;

    char *configFileName = locate_file("settings.json", FT_SAVE_CHECK);

    if (configFileName != NULL) {
        DESERIALIZE_JSON_FILE(&audio, configFileName);
    } else {
        CNOTICE3(filesys,
                 "Could not find audio configuration file %s,"
                 " supplying defaults",
                 "settings.json");

        ResetAudioSettings(audio);

        if (!options.want_audio) {
            audio.master.muted = true;
            audio.music.muted = true;
            audio.soundFX.muted = true;
        }

        try {
            SaveAudioSettings(audio);
        } catch (const IOException &err) {
            CERROR3(filesys,
                    "Could not create audio configuration file %s",
                    "settings.json");
        } catch (const cereal::Exception &err) {
            CERROR3(filesys,
                    "Could not export audio configuration data to %s",
                    "settings.json");
        }
    }

    free(configFileName);

    if (!options.want_audio) {
        audio.master.muted = true;
        audio.music.muted = true;
        audio.soundFX.muted = true;
    }

    return audio;
}


/**
 * Writes a set of audio configurations to the audio settings file.
 *
 * \throws IOException  if unable to create audio settings file.
 * \throws cereal::Exception  if cereal cannot write to the JSON file.
 */
void SaveAudioSettings(const AudioConfig &settings)
{
    char *configFileName = locate_file("settings.json", FT_SAVE_CHECK);

    if (configFileName == NULL) {
        FILE *file = sOpen("settings.json", "wb", FT_SAVE);

        if (file == NULL) {
            free(configFileName);
            throw IOException("Unable to create config file "
                              "settings.json");
        }

        fclose(file);

        configFileName = locate_file("settings.json", FT_SAVE_CHECK);

        if (configFileName == NULL) {
            throw IOException("Tried to create config file "
                              "settings.json"
                              " and could not find afterwards.");
        }
    }

    SERIALIZE_JSON_FILE(settings, configFileName);

    // Func locate_file requires user to free memory.
    free(configFileName);
}


//----------------------------------------------------------------------


namespace
{

void ResetAudioSettings(AudioConfig &audio)
{
    audio.master.muted = audio.music.muted = audio.soundFX.muted = false;
    audio.master.volume = audio.music.volume = audio.soundFX.volume = 100;
}


};  // End of anonymous namespace
