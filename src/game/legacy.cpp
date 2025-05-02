/**
 * Copyright (C) 2005 Michael K. McCarty & Fritz Bronner,
 * Copyright (C) 2020 Hendrik Weimer
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */
// Interplay's BUZZ ALDRIN's RACE into SPACE
//
// Formerly -=> LiftOff : Race to the Moon :: IBM version MCGA
// Copyright 1991 by Strategic Visions, Inc.
// Designed by Fritz Bronner
// Programmed by Michael K McCarty

// This file handles legacy save games.

#include "admin.h"
#include "data.h"
#include "endianness.h"
#include "game_main.h"
#include "legacy.h"
#include "pace.h"

void _SwapEquipment(struct LegacyPlayers *pData)
{
    int plr = 0;
    int i = 0;

    for (plr = 0; plr < 2; ++plr) {
        for (i = 0; i < 3; i++) {
            pData->P[plr].Probe[i].InitCost =
                _Swap16bit(pData->P[plr].Probe[i].InitCost);
            pData->P[plr].Probe[i].UnitWeight =
                _Swap16bit(pData->P[plr].Probe[i].UnitWeight);
            pData->P[plr].Probe[i].MaxPay =
                _Swap16bit(pData->P[plr].Probe[i].MaxPay);
            pData->P[plr].Probe[i].Safety =
                _Swap16bit(pData->P[plr].Probe[i].Safety);
            pData->P[plr].Probe[i].MisSaf =
                _Swap16bit(pData->P[plr].Probe[i].MisSaf);
            pData->P[plr].Probe[i].MSF =
                _Swap16bit(pData->P[plr].Probe[i].MSF);
            pData->P[plr].Probe[i].Steps =
                _Swap16bit(pData->P[plr].Probe[i].Steps);
            pData->P[plr].Probe[i].Failures =
                _Swap16bit(pData->P[plr].Probe[i].Failures);
        }

        for (i = 0; i < 5; i++) {
            pData->P[plr].Rocket[i].InitCost =
                _Swap16bit(pData->P[plr].Rocket[i].InitCost);
            pData->P[plr].Rocket[i].UnitWeight =
                _Swap16bit(pData->P[plr].Rocket[i].UnitWeight);
            pData->P[plr].Rocket[i].MaxPay =
                _Swap16bit(pData->P[plr].Rocket[i].MaxPay);
            pData->P[plr].Rocket[i].Safety =
                _Swap16bit(pData->P[plr].Rocket[i].Safety);
            pData->P[plr].Rocket[i].MisSaf =
                _Swap16bit(pData->P[plr].Rocket[i].MisSaf);
            pData->P[plr].Rocket[i].MSF =
                _Swap16bit(pData->P[plr].Rocket[i].MSF);
            pData->P[plr].Rocket[i].Steps =
                _Swap16bit(pData->P[plr].Rocket[i].Steps);
            pData->P[plr].Rocket[i].Failures =
                _Swap16bit(pData->P[plr].Rocket[i].Failures);
        }

        for (i = 0; i < 7; i++) {
            pData->P[plr].Manned[i].InitCost =
                _Swap16bit(pData->P[plr].Manned[i].InitCost);
            pData->P[plr].Manned[i].UnitWeight =
                _Swap16bit(pData->P[plr].Manned[i].UnitWeight);
            pData->P[plr].Manned[i].MaxPay =
                _Swap16bit(pData->P[plr].Manned[i].MaxPay);
            pData->P[plr].Manned[i].Safety =
                _Swap16bit(pData->P[plr].Manned[i].Safety);
            pData->P[plr].Manned[i].MisSaf =
                _Swap16bit(pData->P[plr].Manned[i].MisSaf);
            pData->P[plr].Manned[i].MSF =
                _Swap16bit(pData->P[plr].Manned[i].MSF);
            pData->P[plr].Manned[i].Steps =
                _Swap16bit(pData->P[plr].Manned[i].Steps);
            pData->P[plr].Manned[i].Failures =
                _Swap16bit(pData->P[plr].Manned[i].Failures);
        }

        for (i = 0; i < 7; i++) {
            pData->P[plr].Misc[i].InitCost =
                _Swap16bit(pData->P[plr].Misc[i].InitCost);
            pData->P[plr].Misc[i].UnitWeight =
                _Swap16bit(pData->P[plr].Misc[i].UnitWeight);
            pData->P[plr].Misc[i].MaxPay =
                _Swap16bit(pData->P[plr].Misc[i].MaxPay);
            pData->P[plr].Misc[i].Safety =
                _Swap16bit(pData->P[plr].Misc[i].Safety);
            pData->P[plr].Misc[i].MisSaf =
                _Swap16bit(pData->P[plr].Misc[i].MisSaf);
            pData->P[plr].Misc[i].MSF =
                _Swap16bit(pData->P[plr].Misc[i].MSF);
            pData->P[plr].Misc[i].Steps =
                _Swap16bit(pData->P[plr].Misc[i].Steps);
            pData->P[plr].Misc[i].Failures =
                _Swap16bit(pData->P[plr].Misc[i].Failures);
        }
    }
}

// This will swap all the player structures
void _SwapGameDat(struct LegacyPlayers *pData)
{
    int16_t i, j;

    pData->Checksum = _Swap32bit(pData->Checksum);

    for (j = 0; j < 28; j++) {
        pData->Prestige[j].Points[0] =
            _Swap16bit(pData->Prestige[j].Points[0]);
        pData->Prestige[j].Points[1] =
            _Swap16bit(pData->Prestige[j].Points[1]);
    }

    for (j = 0; j < 2; j++) {
        pData->P[j].Cash = _Swap16bit(pData->P[j].Cash);
        pData->P[j].Budget = _Swap16bit(pData->P[j].Budget);
        pData->P[j].Prestige = _Swap16bit(pData->P[j].Prestige);

        for (i = 0; i < 5; i++) {
            pData->P[j].PrestHist[i][0] =
                _Swap16bit(pData->P[j].PrestHist[i][0]);
            pData->P[j].PrestHist[i][1] =
                _Swap16bit(pData->P[j].PrestHist[i][1]);

            pData->P[j].PresRev[i] = _Swap16bit(pData->P[j].PresRev[i]);

            pData->P[j].Spend[i][0] = _Swap16bit(pData->P[j].Spend[i][0]);
            pData->P[j].Spend[i][1] = _Swap16bit(pData->P[j].Spend[i][1]);
            pData->P[j].Spend[i][2] = _Swap16bit(pData->P[j].Spend[i][2]);
            pData->P[j].Spend[i][3] = _Swap16bit(pData->P[j].Spend[i][3]);
        }

        pData->P[j].tempPrestige[0] = _Swap16bit(pData->P[j].tempPrestige[0]);
        pData->P[j].tempPrestige[1] = _Swap16bit(pData->P[j].tempPrestige[1]);

        for (i = 0; i < 40; i++) {
            pData->P[j].BudgetHistory[i] =
                _Swap16bit(pData->P[j].BudgetHistory[i]);
            pData->P[j].BudgetHistoryF[i] =
                _Swap16bit(pData->P[j].BudgetHistoryF[i]);
        }

        for (i = 0; i < 65; i++) {
            pData->P[j].Pool[i].Prestige =
                _Swap16bit(pData->P[j].Pool[i].Prestige);
        }

        for (i = 0; i < 100; i++) {
            pData->P[j].History[i].result =
                _Swap16bit(pData->P[j].History[i].result);
            pData->P[j].History[i].spResult =
                _Swap16bit(pData->P[j].History[i].spResult);
            pData->P[j].History[i].Prestige =
                _Swap16bit(pData->P[j].History[i].Prestige);
        }

        pData->P[j].PastMissionCount = _Swap16bit(pData->P[j].PastMissionCount);

        for (i = 0; i < 30; i++) {
            pData->P[j].PastIntel[i].num =
                _Swap16bit(pData->P[j].PastIntel[i].num);
        }

    }

    _SwapEquipment(pData);
    // End of GameDatSwap
}

/**
 * Load function for old save game formats.
 */
void LegacyLoad(SaveFileHdr header, FILE *fin, size_t fileLength)
{
    LEGACY_REPLAY *load_buffer = NULL;
    uint16_t dataSize, compSize;
    int i, j;
    const int legacySize = 38866;
    struct LegacyPlayers *legacyData;

    legacyData = new struct LegacyPlayers;
    memset(legacyData, 0x00, sizeof(struct LegacyPlayers));

    dataSize = *(uint16_t *) header.dataSize;
    compSize = *(uint16_t *)(header.dataSize + 2);

    // Determine Endian Swap, 31663 is for pre-PBEM save games
    bool endianSwap = (dataSize != legacySize && dataSize != 31663);

    if (endianSwap) {
        compSize = _Swap16bit(compSize);
        dataSize = _Swap16bit(dataSize);

        if (dataSize !=  legacySize && dataSize != 31663) {
            // TODO: Feels like BadFileType() should be launched by
            // FileAccess, which runs the interface. Throw an
            // exception or return an error code?
            fclose(fin);
            BadFileType();
            return;
        }
    }

    size_t readLen = compSize;
    load_buffer = (LEGACY_REPLAY *)malloc(readLen);
    fread(load_buffer, 1, readLen, fin);
    RLED((char *) load_buffer, (char *)legacyData, compSize);
    free(load_buffer);

    // Swap Players' Data
    if (endianSwap) {
        _SwapGameDat(legacyData);
    }

    // Serialize legacy data to JSON and deserialize it into the new struct
    stringstream stream;
    {
        cereal::JSONOutputArchive oarchive(stream);
        oarchive(cereal::make_nvp("Data", *legacyData));
    }
    {
        cereal::JSONInputArchive iarchive(stream);
        iarchive(cereal::make_nvp("Data", *Data));
    }
    
    delete legacyData;

    // Read the Replay Data
    load_buffer = (LEGACY_REPLAY *)malloc((sizeof(LEGACY_REPLAY)) * MAX_REPLAY_ITEMS);
    fread(load_buffer, 1, sizeof(LEGACY_REPLAY) * MAX_REPLAY_ITEMS, fin);

    if (endianSwap) {
        LEGACY_REPLAY *r = NULL;
        r = load_buffer;

        for (int j = 0; j < MAX_REPLAY_ITEMS; j++) {
            for (int k = 0; k < r->Qty; k++) {
                r[j].Off[k] = _Swap16bit(r[j].Off[k]);
            }
        }
    }

    std::vector<std::string> seq;
    std::map<int, std::string> fseq;

    DESERIALIZE_JSON_FILE(&seq, locate_file("legacy-seq.json", FT_DATA));
    DESERIALIZE_JSON_FILE(&fseq, locate_file("legacy-fseq.json", FT_DATA));

    for (i = 0; i < MAX_REPLAY_ITEMS; i++) {
        interimData.tempReplay.at(i).clear();
        assert(load_buffer[i].Qty <= 35);

        for (int j = 0; j < load_buffer[i].Qty; j++) {

            int code = load_buffer[i].Off[j];

            if (code < 1000) {
                interimData.tempReplay.at(i).push_back({false, seq.at(code)});
            } else {
                interimData.tempReplay.at(i).push_back({true, fseq.at(code)});
            }

        }
    }

    free(load_buffer);

    size_t eventSize = fileLength - ftell(fin);

    // Read the Event Data
    load_buffer = (LEGACY_REPLAY *)malloc(eventSize);
    fread(load_buffer, 1, eventSize, fin);
    fclose(fin);

    if (endianSwap) {
        // File Structure is 84 longs 42 per side
        for (int j = 0; j < 84; j++) {
            OLDNEWS *on = (OLDNEWS *) load_buffer + (j * sizeof(OLDNEWS));

            if (on->offset) {
                on->offset = _Swap32bit(on->offset);
                on->size = _Swap16bit(on->size);
            }
        }
    }

    // Save Event information

    for (int j = 0; j < MAX_NEWS_ITEMS; j++) {
        OLDNEWS *on = (OLDNEWS *) load_buffer + j;
        char *text = (char *) load_buffer + on->offset;
        interimData.tempEvents.at(j) = "";

        for (int k = 0; k < on->size; k++) {
            interimData.tempEvents.at(j).push_back(text[k]);
        }
    }

    free(load_buffer);

    // MSF now holds MaxRDBase (from 1.0.0)
    CheckMSF();
}
