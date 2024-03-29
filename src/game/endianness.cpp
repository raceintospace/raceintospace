#include "endianness.h"

#include <cassert>

#include "Buzz_inc.h"
#include "game_main.h"


// Need these functions to always exist
uint32_t _Swap32bit(uint32_t value)
{
    return (value >> 24)
           | ((value >> 8) & 0x0000FF00)
           | ((value << 8) & 0x00FF0000)
           | (value << 24);
}

uint16_t _Swap16bit(uint16_t value)
{
    return (value << 8) | (value >> 8);
}

float _SwapFloat(float value)
{
    union {
        float f;
        unsigned char b[4];
    } dat1, dat2;

    dat1.f = value;
    dat2.b[0] = dat1.b[3];
    dat2.b[1] = dat1.b[2];
    dat2.b[2] = dat1.b[1];
    dat2.b[3] = dat1.b[0];
    return dat2.f;
}

// This will swap all the player structures
void _SwapGameDat(void)
{
    int16_t i, j;

    Data->Checksum = _Swap32bit(Data->Checksum);

    for (j = 0; j < 28; j++) {
        Data->Prestige[j].Points[0] =
            _Swap16bit(Data->Prestige[j].Points[0]);
        Data->Prestige[j].Points[1] =
            _Swap16bit(Data->Prestige[j].Points[1]);
    }

    for (j = 0; j < 2; j++) {
        Data->P[j].Cash = _Swap16bit(Data->P[j].Cash);
        Data->P[j].Budget = _Swap16bit(Data->P[j].Budget);
        Data->P[j].Prestige = _Swap16bit(Data->P[j].Prestige);

        for (i = 0; i < 5; i++) {
            Data->P[j].PrestHist[i][0] =
                _Swap16bit(Data->P[j].PrestHist[i][0]);
            Data->P[j].PrestHist[i][1] =
                _Swap16bit(Data->P[j].PrestHist[i][1]);

            Data->P[j].PresRev[i] = _Swap16bit(Data->P[j].PresRev[i]);

            Data->P[j].Spend[i][0] = _Swap16bit(Data->P[j].Spend[i][0]);
            Data->P[j].Spend[i][1] = _Swap16bit(Data->P[j].Spend[i][1]);
            Data->P[j].Spend[i][2] = _Swap16bit(Data->P[j].Spend[i][2]);
            Data->P[j].Spend[i][3] = _Swap16bit(Data->P[j].Spend[i][3]);
        }

        Data->P[j].tempPrestige[0] = _Swap16bit(Data->P[j].tempPrestige[0]);
        Data->P[j].tempPrestige[1] = _Swap16bit(Data->P[j].tempPrestige[1]);

        for (i = 0; i < 40; i++) {
            Data->P[j].BudgetHistory[i] =
                _Swap16bit(Data->P[j].BudgetHistory[i]);
            Data->P[j].BudgetHistoryF[i] =
                _Swap16bit(Data->P[j].BudgetHistoryF[i]);
        }

        for (i = 0; i < 65; i++) {
            Data->P[j].Pool[i].Prestige =
                _Swap16bit(Data->P[j].Pool[i].Prestige);
        }

        for (i = 0; i < 100; i++) {
            Data->P[j].History[i].result =
                _Swap16bit(Data->P[j].History[i].result);
            Data->P[j].History[i].spResult =
                _Swap16bit(Data->P[j].History[i].spResult);
            Data->P[j].History[i].Prestige =
                _Swap16bit(Data->P[j].History[i].Prestige);
        }

        Data->P[j].PastMissionCount = _Swap16bit(Data->P[j].PastMissionCount);

        for (i = 0; i < 30; i++) {
            Data->P[j].PastIntel[i].num =
                _Swap16bit(Data->P[j].PastIntel[i].num);
        }

    }

    _SwapEquipment();
    // End of GameDatSwap
}

void _SwapEquipment(void)
{
    int plr = 0;
    int i = 0;

    for (plr = 0; plr < 2; ++plr) {
        for (i = 0; i < 3; i++) {
            Data->P[plr].Probe[i].InitCost =
                _Swap16bit(Data->P[plr].Probe[i].InitCost);
            Data->P[plr].Probe[i].UnitWeight =
                _Swap16bit(Data->P[plr].Probe[i].UnitWeight);
            Data->P[plr].Probe[i].MaxPay =
                _Swap16bit(Data->P[plr].Probe[i].MaxPay);
            Data->P[plr].Probe[i].Safety =
                _Swap16bit(Data->P[plr].Probe[i].Safety);
            Data->P[plr].Probe[i].MisSaf =
                _Swap16bit(Data->P[plr].Probe[i].MisSaf);
            Data->P[plr].Probe[i].MSF =
                _Swap16bit(Data->P[plr].Probe[i].MSF);
            Data->P[plr].Probe[i].Steps =
                _Swap16bit(Data->P[plr].Probe[i].Steps);
            Data->P[plr].Probe[i].Failures =
                _Swap16bit(Data->P[plr].Probe[i].Failures);
        }

        for (i = 0; i < 5; i++) {
            Data->P[plr].Rocket[i].InitCost =
                _Swap16bit(Data->P[plr].Rocket[i].InitCost);
            Data->P[plr].Rocket[i].UnitWeight =
                _Swap16bit(Data->P[plr].Rocket[i].UnitWeight);
            Data->P[plr].Rocket[i].MaxPay =
                _Swap16bit(Data->P[plr].Rocket[i].MaxPay);
            Data->P[plr].Rocket[i].Safety =
                _Swap16bit(Data->P[plr].Rocket[i].Safety);
            Data->P[plr].Rocket[i].MisSaf =
                _Swap16bit(Data->P[plr].Rocket[i].MisSaf);
            Data->P[plr].Rocket[i].MSF =
                _Swap16bit(Data->P[plr].Rocket[i].MSF);
            Data->P[plr].Rocket[i].Steps =
                _Swap16bit(Data->P[plr].Rocket[i].Steps);
            Data->P[plr].Rocket[i].Failures =
                _Swap16bit(Data->P[plr].Rocket[i].Failures);
        }

        for (i = 0; i < 7; i++) {
            Data->P[plr].Manned[i].InitCost =
                _Swap16bit(Data->P[plr].Manned[i].InitCost);
            Data->P[plr].Manned[i].UnitWeight =
                _Swap16bit(Data->P[plr].Manned[i].UnitWeight);
            Data->P[plr].Manned[i].MaxPay =
                _Swap16bit(Data->P[plr].Manned[i].MaxPay);
            Data->P[plr].Manned[i].Safety =
                _Swap16bit(Data->P[plr].Manned[i].Safety);
            Data->P[plr].Manned[i].MisSaf =
                _Swap16bit(Data->P[plr].Manned[i].MisSaf);
            Data->P[plr].Manned[i].MSF =
                _Swap16bit(Data->P[plr].Manned[i].MSF);
            Data->P[plr].Manned[i].Steps =
                _Swap16bit(Data->P[plr].Manned[i].Steps);
            Data->P[plr].Manned[i].Failures =
                _Swap16bit(Data->P[plr].Manned[i].Failures);
        }

        for (i = 0; i < 7; i++) {
            Data->P[plr].Misc[i].InitCost =
                _Swap16bit(Data->P[plr].Misc[i].InitCost);
            Data->P[plr].Misc[i].UnitWeight =
                _Swap16bit(Data->P[plr].Misc[i].UnitWeight);
            Data->P[plr].Misc[i].MaxPay =
                _Swap16bit(Data->P[plr].Misc[i].MaxPay);
            Data->P[plr].Misc[i].Safety =
                _Swap16bit(Data->P[plr].Misc[i].Safety);
            Data->P[plr].Misc[i].MisSaf =
                _Swap16bit(Data->P[plr].Misc[i].MisSaf);
            Data->P[plr].Misc[i].MSF =
                _Swap16bit(Data->P[plr].Misc[i].MSF);
            Data->P[plr].Misc[i].Steps =
                _Swap16bit(Data->P[plr].Misc[i].Steps);
            Data->P[plr].Misc[i].Failures =
                _Swap16bit(Data->P[plr].Misc[i].Failures);
        }
    }
}

void _SwapPatchHdr(PatchHdr *hdr)
{
    assert(hdr);

    hdr->w = _Swap16bit(hdr->w);
    hdr->h = _Swap16bit(hdr->h);
    hdr->size = _Swap16bit(hdr->size);
    hdr->offset = _Swap32bit(hdr->offset);
}

void _SwapPatchHdrSmall(PatchHdrSmall *hdr)
{
    assert(hdr);

    hdr->size = _Swap16bit(hdr->size);
    hdr->offset = _Swap32bit(hdr->offset);
}

/* vim: set noet ts=4 sw=4 tw=77: */
