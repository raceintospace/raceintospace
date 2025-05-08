/*
    Copyright (C) 2005 Michael K. McCarty & Fritz Bronner

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
//****************************************************************
//*Interplay's BUZZ ALDRIN's RACE into SPACE                     *
//*                                                              *
//*Formerly -=> LiftOff : Race to the Moon :: IBM version MCGA   *
//*Copyright 1991 by Strategic Visions, Inc.                     *
//*Designed by Fritz Bronner                                     *
//*Programmed by Michael K McCarty                               *
//*                                                              *
//****************************************************************

// This file handles the main Spaceport screen.

#include "port.h"

#include <cstdio>

#include <boost/shared_ptr.hpp>

#include "display/graphics.h"
#include "display/surface.h"
#include "display/image.h"

#include "admin.h"
#include "ast0.h"
#include "ast2.h"
#include "ast3.h"
#include "ast4.h"
#include "budget.h"
#include "Buzz_inc.h"
#include "draw.h"
#include "filesystem.h"
#include "game_main.h"
#include "gr.h"
#include "intel.h"
#include "mc.h"
#include "mission_util.h"
#include "museum.h"
#include "pace.h"
#include "place.h"
#include "radar.h"
#include "rdplex.h"
#include "review.h"
#include "rush.h"
#include "sdlhelper.h"
#include "spot.h"
#include "utils.h"
#include "vab.h"

LOG_DEFAULT_CATEGORY(LOG_ROOT_CAT)

#define LET_A   0x09
#define LET_M   0x0A
#define LET_O   0x0B
#define LET_V   0x0C
#define LET_R   0x08

#define SPOT_ON 1 /**< turn off until everything else works - pace */
#define pNOREDRAW 0
#define pREDRAW 1
#define pEXIT 2
#define pQUIT 3
#define pNOFADE 4
#define pNEWMUSIC 5

#define S_QTY 43
#define S_MOBJ 35


struct BOUND {
    int16_t x1;
    int16_t y1;
    int16_t x2;
    int16_t y2;

    template<class Archive>
    void serialize(Archive & ar) {
        ar(CEREAL_NVP(x1));
        ar(CEREAL_NVP(y1));
        ar(CEREAL_NVP(x2));
        ar(CEREAL_NVP(y2));
    }
};

struct REGION {
    char qty;
    BOUND CD[4];
    char iNum;
    char sNum;
    char PreDraw;

    template<class Archive>
    void serialize(Archive & ar) {
        ar(CEREAL_NVP(qty));
        ar(CEREAL_NVP(CD));
        ar(CEREAL_NVP(iNum));
        ar(CEREAL_NVP(sNum));
        ar(CEREAL_NVP(PreDraw));
    }
};

struct MOBJ {
    std::string Name;
    char qty;
    std::string Help;
    REGION Reg[4];

    template<class Archive>
    void serialize(Archive & ar) {
        ar(CEREAL_NVP(Name)); 
        ar(CEREAL_NVP(qty));
        ar(CEREAL_NVP(Help)); 
        ar(CEREAL_NVP(Reg));
    }
};

struct IMG {
	int32_t Size;         // Size of Image (bytes)
	char Comp;            // Type of Compression Used
	int16_t Width;        // Width of Image
	int16_t Height;       // Height of Image
	int16_t PlaceX;       // Where to Place Img:X
	int16_t PlaceY;       // Where to Place Img:Y
	
	template<class Archive>
	void serialize(Archive & ar) {
        ar(Size);
        ar(Comp);
        ar(Width);
        ar(Height);
        ar(PlaceX);
        ar(PlaceY);
	}  
};

struct OUTLINE {
      uint16_t Count;
      std::vector<uint16_t> bone;
      
      template<class Archive>
      void serialize(Archive & ar) {
        ar(CEREAL_NVP(Count));
        ar(CEREAL_NVP(bone));
      }
};

struct PORTOUTLINE {
    uint16_t loc;
    char val;
};

int Vab_Spot; // Global variable

namespace // Local global variables
{	
	std::vector<MOBJ> MObj(S_MOBJ);
	std::vector<IMG> Img(S_QTY);
	std::vector<OUTLINE> pOutline(55);
	
	PORTOUTLINE *pPortOutlineRestore;
	
	/** These are the valid hotkeys */
	char HotKeyList[] = "AIMRPVCQETBHLS\0";
	char RUSH;
	int FCtr;

	boost::shared_ptr<display::PalettizedSurface> flaggy;

	enum MissionReadyStatus {
		MISSIONS_NONE = 0,
		MISSIONS_UNSTAGED,
		MISSIONS_UNSCHEDULED,
		MISSIONS_READY
	};
};  // End of namespace


void WaveFlagSetup(void);
void WaveFlagDel(void);
void LoadImg(int plr, int indx);
void PortText(int x, int y, std::string txt, char col);
void UpdatePortOverlays(void);
void DoCycle(void);
bool EndTurnOk(int plr);
int MissionStatus(int plr);
void PortOutLine(unsigned int Count, uint16_t *buf, char mode);
void PortRestore(unsigned int Count);
int MapKey(char plr, int key, int old) ;
void Port(char plr);
char PortSel(char plr, char loc);
char Request(char plr, const char *s, char md);
int SpaceportAnimationEntry(int plr);
int SpaceportAnimationOngoing(int plr);
void LoadPOutline(int plr); 


void WaveFlagSetup(char plr)
{
    char filename[256];
    snprintf(filename, sizeof(filename), "images/port/flag.seq.%d.png", plr);
    flaggy = boost::shared_ptr<display::PalettizedSurface>(Filesystem::readImage(filename));
}

void WaveFlagDel(void)
{
    flaggy.reset();
}


/**
 * Display a port building/location image.
 *
 * Image composition is:
 *     int32_t Size      -- Size of Image (bytes)
 *     char Comp         -- Type of Compression Used
 *     int16_t Width     -- Width of Image
 *     int16_t Height    -- Height of Image
 *     int16_t PlaceX    -- Where to Place Img:X
 *     int16_t PlaceY    -- Where to Place Img:Y
 *
 * \param plr    the player (usa/sov)_port.json image file to open.
 * \param indx   index to the image position data in the Img vector.
 */
void LoadImg(int plr, int indx)
{
    std::string filename;
    
    // Load port sprites
	if (plr == 0) {
		filename = "images/port/usa_port.dat." + std::to_string(indx) + ".png";
	} else {
		filename = "images/port/sov_port.dat." + std::to_string(indx) + ".png";
	}
    
    boost::shared_ptr<display::PalettizedSurface> image;
    if (image = Filesystem::readImage(filename)) {
		TRACE2("load image `%s'", filename.c_str());
    } else {
		throw std::runtime_error(filename + " could not be loaded.");
    }    
    
    //image->exportPalette(Img[indx].Width, Img[indx].Height);
    image->exportPalette();
    
    display::graphics.screen()->draw(image, Img[indx].PlaceX, Img[indx].PlaceY);
}


/**
 * Loads the port palette into the global display palette, overwriting
 * the current palette.
 *
 * The Port palette is a 256-color palette, differing slightly per side.
 * The first 27 colors, however, are the same.
 *
 * \param plr  The player palette to use (0 for USA, 1 for USSR)
 */
void PortPal(char plr)
{   
    std::string filename;
	filename = (plr == 0) ? "usa_port.json" : "sov_port.json";
    
    // Deserialize palette
    std::vector<uint8_t> palette;
    std::ifstream file(locate_file(filename.c_str(), FT_DATA));
    if (!file) {
		throw std::runtime_error(filename + " could not be opened.");
	}
    cereal::JSONInputArchive ar(file);
    ar(CEREAL_NVP(palette));
        
    // Display p and copy data to p.pal
    display::AutoPal p(display::graphics.legacyScreen());
    for (size_t i= 0; i < sizeof(p.pal); i++) {  // the limit is Autopal p.pal size
        p.pal[i] = static_cast<char>(palette[i]);
    }
}


void DrawSpaceport(char plr)
{
	std::string filename;
	filename = (plr == 0) ? "usa_port.json" : "sov_port.json";
	
	// Deserialize Img and MObj
	{
		std::ifstream file(locate_file(filename.c_str(), FT_DATA));
		if (!file) {
			throw std::runtime_error(filename + " could not be opened.");
		}
		
		cereal::JSONInputArchive ar(file);
		ar(CEREAL_NVP(MObj));
		ar(CEREAL_NVP(Img));
    }
    
    // Draw the main port image
	LoadImg(plr, 0);

    UpdatePortOverlays();

    if (xMODE & xMODE_CLOUDS) {
        LoadImg(plr, 1);    // Clouds
    }

    // Pads
    for (int i = 0; i < MAX_LAUNCHPADS; i++) {
        Data->P[plr].Port[PORT_LaunchPad_A + i] = 1;  // Draw launch pad

        if (Data->P[plr].Mission[i].MissionCode) {
            Data->P[plr].Port[PORT_LaunchPad_A + i] = 2;  // Draw damaged launch pad
        } else if (Data->P[plr].LaunchFacility[i] >= LAUNCHPAD_DAMAGED_MARGIN) {
            Data->P[plr].Port[PORT_LaunchPad_A + i] = 3;
        } else if (Data->P[plr].LaunchFacility[i] == LAUNCHPAD_NOT_BUILT) {  // No launch facility
            Data->P[plr].Port[PORT_LaunchPad_A + i] = 0;
        }
    }

    if (Vab_Spot == 1 && Data->P[plr].Port[PORT_VAB] == 2) {
        Data->P[plr].Port[PORT_LaunchPad_A] = plr;
    }

    if (Data->P[plr].AstroCount > 0) {
        LoadImg(plr, 16 - plr * 4);  // Draw CPX
        HotKeyList[9] = 'T';
        HotKeyList[10] = 'B';
    } else {    // No manned program hotkeys
        HotKeyList[9] = '\0';
        HotKeyList[10] = '\0';
    }

    if (Data->P[plr].Pool[0].Active >= 1) {
        LoadImg(plr, 17 - plr * 4);    // Draw TRN
    }

    if (Data->P[plr].Port[PORT_Research] > 1) {
        LoadImg(plr, 13 + 15 * plr);    // RD Stuff
    }

    if (Data->P[plr].Port[PORT_Research] > 2) {
        LoadImg(plr, 14 + 15 * plr);
    }

    if (Data->P[plr].Port[PORT_Research] == 3) {
        LoadImg(plr, 15 + 15 * plr);
    }

    for (int fm = 0; fm < S_MOBJ; fm++) {
        int idx = Data->P[plr].Port[fm];  // Current Port Level for MObj

        if (MObj[fm].Reg[idx].PreDraw > 0) {  // PreDrawn Shape
            LoadImg(plr, MObj[fm].Reg[idx].PreDraw);
        }

        if (MObj[fm].Reg[idx].iNum > 0) {  // Actual Shape
            LoadImg(plr, MObj[fm].Reg[idx].iNum);
        }
    }

    ShBox(0, 190, 319, 199);            // Base Box :: larger

    display::graphics.setForegroundColor(0);
    draw_string(257, 197, "CASH:");
    draw_megabucks(285, 197, Data->P[plr].Cash);
    display::graphics.setForegroundColor(11);
    draw_string(256, 196, "CASH:");
    draw_megabucks(284, 196, Data->P[plr].Cash);

    display::graphics.setForegroundColor(0);

    if (Data->Season == 0) {
        draw_string(166, 197, "SPRING 19");
    } else {
        draw_string(166, 197, "FALL 19");
    }

    draw_number(0, 0, Data->Year);

    display::graphics.setForegroundColor(11);

    if (Data->Season == 0) {
        draw_string(165, 196, "SPRING 19");
    } else {
        draw_string(165, 196, "FALL 19");
    }

    draw_number(0, 0, Data->Year);

    if (plr == 0) {
        display::graphics.screen()->draw(flaggy, FCtr * 23, 0, 23, 22, 49, 121);
    } else {
        display::graphics.screen()->draw(flaggy, FCtr * 23, 0, 23, 22, 220, 141);
    }
}


void PortText(int x, int y, std::string txt, char col)
{
    fill_rectangle(1, 192, 160, 198, 3);
    display::graphics.setForegroundColor(0);
    draw_string(x + 1, y + 1, txt.c_str());
    display::graphics.setForegroundColor(col);
    draw_string(x, y, txt.c_str());
}


void UpdatePortOverlays(void)
{
    for (int8_t i = 0; i < NUM_PLAYERS; i++) {  // Programs
        for (int8_t j = 0; j < 5; j++) {
            Data->P[i].Port[PORT_Mercury - j] = (Data->P[i].Manned[j].Num >= 0) ? 1 : 0;
        }

#ifdef DEADCODE
        // Zond thingy -- this was never implemented and was available after 6 manned seasons
        // if (i==1 && Data->P[i].Manned[MANNED_HW_THREE_MAN_CAPSULE].Seas>6) Data->P[i].Port[PORT_Zond]=1;
#endif

        if (Data->P[i].Probe[PROBE_HW_ORBITAL].Num >= 0 || Data->P[i].Probe[PROBE_HW_INTERPLANETARY].Num >= 0 ||
            Data->P[i].Probe[PROBE_HW_LUNAR].Num >= 0) {
            Data->P[i].Port[PORT_Satellite] = 1;
        }

        if (Data->P[i].Manned[MANNED_HW_TWO_MAN_MODULE].Num >= 0 || Data->P[i].Manned[MANNED_HW_ONE_MAN_MODULE].Num >= 0) {
            Data->P[i].Port[PORT_LM] = 1;
        }

        // Museum
        if (Data->Prestige[Prestige_MannedOrbital].Goal[i] > 0) {
            Data->P[i].Port[PORT_Museum] = MAX(Data->P[i].Port[PORT_Museum], 1);    // Mus:1
        }

        if (Data->Prestige[Prestige_LunarFlyby].Goal[i] > 0) {
            Data->P[i].Port[PORT_Museum] = MAX(Data->P[i].Port[PORT_Museum], 2);    // Mus:2
        }

        if (Data->Prestige[Prestige_MannedLunarOrbit].Goal[i] > 0) {
            Data->P[i].Port[PORT_Museum] = MAX(Data->P[i].Port[PORT_Museum], 3);    // Mus:3
        }

        // R&D
        if (Data->P[i].Budget >= 85) {
            Data->P[i].Port[PORT_Research] = MAX(Data->P[i].Port[PORT_Research], 1);    // RD:1
        }

        if (Data->P[i].Budget >= 112) {
            Data->P[i].Port[PORT_Research] = MAX(Data->P[i].Port[PORT_Research], 2);    // RD:2
        }

        if (Data->P[i].Budget >= 150) {
            Data->P[i].Port[PORT_Research] = MAX(Data->P[i].Port[PORT_Research], 3);    // RD:3
        }

        // VAB

        if (Data->Prestige[Prestige_Duration_B].Goal[i] > 0) {
            Data->P[i].Port[PORT_VAB] = MAX(Data->P[i].Port[PORT_VAB], 1);    // VAB:1
        }

        if (Data->P[i].Budget > 115) {
            Data->P[i].Port[PORT_VAB] = MAX(Data->P[i].Port[PORT_VAB], 2);    // VAB:2
        }

        // Admin
        if (Data->P[i].AstroLevel >= 2) {
            Data->P[i].Port[PORT_Admin] = MAX(Data->P[i].Port[PORT_Admin], 1);    // Adm:1
        }

        if (Data->P[i].AstroLevel >= 4) {
            Data->P[i].Port[PORT_Admin] = MAX(Data->P[i].Port[PORT_Admin], 2);    // Adm:2
        }

        if (Data->Prestige[Prestige_OnePerson].Goal[i] > 0) {
            Data->P[i].Port[PORT_Tracking] = MAX(Data->P[i].Port[PORT_Tracking], 1);    // Trk:1
        }

        if (Data->Prestige[Prestige_MannedLunarPass].Goal[i] > 0) {
            Data->P[i].Port[PORT_MissionControl] = MAX(Data->P[i].Port[PORT_MissionControl], 1);    // MC:1
        }

        if (Data->P[i].AstroCount > 0) {
            Data->P[i].Port[PORT_AstroComplex] = Data->P[i].Port[PORT_BasicTraining] = 1;
        }

        if (Data->P[i].Pool[0].Active > 0) {     // Astros
            Data->P[i].Port[PORT_Helipad] = Data->P[i].Port[PORT_Pool] = Data->P[i].Port[PORT_Planetarium] = 1;
            Data->P[i].Port[PORT_Centrifuge] = Data->P[i].Port[PORT_MedicalCtr] = Data->P[i].Port[PORT_Airfield] = 1;
        }
    }
}


void Master(char plr)
{
    helpText = "i000";
    keyHelpText = "i000";
    WaveFlagSetup(plr);
    Vab_Spot = 0;
	
	/*
    // TODO: Is there a point to this loop? Can it just be removed?
    // Can any Mission modification be moved to start-of-turn upkeep?
    for (int i = 0; i < 3; i++) {
        Data->P[plr].Mission[i].Joint =
            GetMissionPlan(Data->P[plr].Mission[i].MissionCode).Jt;
    }
	*/
	
    // Entering screen for the first time so fade out and in.
    FadeOut(2, 10, 0, 0);
    DrawSpaceport(plr);
    FadeIn(2, 10, 0, 0);

    SpotInit();

#if SPOT_ON

    int animation = SpaceportAnimationEntry(plr);
    SpotLoad(animation);

#endif
	
    Port(plr);
    helpText = "i000";
    keyHelpText = "i000";
    WaveFlagDel();

    SpotClose();
}


/**
 * Get the mouse or keyboard input.
 *
 * A non-blocking check of mouse/keyboard input based on GetMouse_fast
 * which updates flagpole, USA water, and spot animations for the Port
 * screen.
 */
void GetMse(char plr, char fon)
{
    static double last_wave_step;
    double now;

    now = get_time();

    if (now - last_wave_step > .125) {
        last_wave_step = now;

        if (plr == 0) {
            DoCycle();
        }

        if (fon != 1) {
            goto done;
        }

#if SPOT_ON
        SpotAdvance();
#endif
        FCtr = FCtr % 5;

        if (plr == 0) {
            display::graphics.screen()->draw(flaggy, FCtr * 23, 0, 23, 22, 49, 121);
        } else {
            display::graphics.screen()->draw(flaggy, FCtr * 23, 0, 23, 22, 220, 141);
        }

done:
        FCtr++;
    }

    GetMouse_fast();
}


void DoCycle(void)                   // Three ranges of color cycling
{
    int i, tmp1, tmp2, tmp3, j;
    display::AutoPal p(display::graphics.legacyScreen());

    j = 384;

    i = 0;
    tmp1 = p.pal[j + 3 * i + 0];
    tmp2 = p.pal[j + 3 * i + 1];
    tmp3 = p.pal[j + 3 * i + 2];

    for (; i < 3; i++) {
        p.pal[j + i * 3 + 0] = p.pal[j + (i + 1) * 3 + 0];
        p.pal[j + i * 3 + 1] = p.pal[j + (i + 1) * 3 + 1];
        p.pal[j + i * 3 + 2] = p.pal[j + (i + 1) * 3 + 2];
    }

    p.pal[j + 3 * i] = tmp1;
    p.pal[j + 3 * i + 1] = tmp2;
    p.pal[j + 3 * i + 2] = tmp3;

    i = 4;

    tmp1 = p.pal[j + 3 * i + 0];
    tmp2 = p.pal[j + 3 * i + 1];
    tmp3 = p.pal[j + 3 * i + 2];

    for (; i < 11; i++) {
        p.pal[j + i * 3 + 0] = p.pal[j + (i + 1) * 3 + 0];
        p.pal[j + i * 3 + 1] = p.pal[j + (i + 1) * 3 + 1];
        p.pal[j + i * 3 + 2] = p.pal[j + (i + 1) * 3 + 2];
    }

    p.pal[j + 3 * i] = tmp1;
    p.pal[j + 3 * i + 1] = tmp2;
    p.pal[j + 3 * i + 2] = tmp3;

    i = 12;

    tmp1 = p.pal[j + 3 * i + 0];
    tmp2 = p.pal[j + 3 * i + 1];
    tmp3 = p.pal[j + 3 * i + 2];

    for (; i < 15; i++) {
        p.pal[j + i * 3 + 0] = p.pal[j + (i + 1) * 3 + 0];
        p.pal[j + i * 3 + 1] = p.pal[j + (i + 1) * 3 + 1];
        p.pal[j + i * 3 + 2] = p.pal[j + (i + 1) * 3 + 2];
    }

    p.pal[j + 3 * i] = tmp1;
    p.pal[j + 3 * i + 1] = tmp2;
    p.pal[j + 3 * i + 2] = tmp3;
}


/**
 * Check if the player is cleared to end their turn.
 *
 * \param plr  the currently active player.
 */
bool EndTurnOk(int plr)
{
    int status = MissionStatus(plr);
    bool enableTutorial = true;  // Implement option to disable advice

    if (status == MISSIONS_UNSTAGED) {
        Help("i005");
        return false;
    } else if (status == MISSIONS_UNSCHEDULED) {
        Help("i103");
        return false;
    }

    if (enableTutorial) {

        // Warn player that they have no missions scheduled for next
        // turn, except on the first turn of the game, or if you can't
        // buy rockets next turn. -Leon
        if (Data->P[plr].Rocket[0].Delay < 1 &&
            !(Data->Year == 57 && Data->Season == 0)) {
            bool future = false;

            for (int i = 0; i < MAX_MISSIONS; i++) {
                if (Data->P[plr].Future[i].MissionCode) {
                    future = true;
                    break;
                }
            }

            if (!future && Help("i161") <= 0) {
                return false;
            }
        }
    }

    if (status == MISSIONS_NONE) {
        return Request(plr, "END TURN", 1);
    } else {  // status == MISSIONS_READY
        return Help("i004") >= 1;
    }
}


/**
 * Report on the status of the missions planned for the current turn.
 */
int MissionStatus(int plr)
{
    bool prepped = false;

    // Check to see if missions are good to go
    for (int i = 0; i < MAX_MISSIONS; i++) {
        if (Data->P[plr].Mission[i].MissionCode &&
            Data->P[plr].Mission[i].Hard[Mission_PrimaryBooster] == 0) {
            return MISSIONS_UNSTAGED;
        }

        prepped |= (Data->P[plr].Mission[i].MissionCode != Mission_None);
    }

    if (!prepped) {
        return MISSIONS_NONE;
    } else {
        return RUSH ? MISSIONS_READY : MISSIONS_UNSCHEDULED;
    }
}


/** ???
 *
 * \param mode ...  0 = ?   1 = copy stored outline ?
 */
void PortOutLine(unsigned int Count, uint16_t *outline, char mode)
{
    int min_x = MAX_X, min_y = MAX_Y, max_x = 0, max_y = 0;
    unsigned int i;

    pPortOutlineRestore = (PORTOUTLINE *)xrealloc(pPortOutlineRestore,
                          sizeof(PORTOUTLINE) * Count);

    for (i = 0; i < Count; i++) {
        if (mode == 1) {
            // Save value from the screen
            pPortOutlineRestore[i].loc = outline[i];    // Offset of the outline into the buffer
            pPortOutlineRestore[i].val = display::graphics.legacyScreen()->pixels()[outline[i]];    // Save original pixel value
        } else {                   // dunno
            outline[i] = pPortOutlineRestore[i].loc;
        }

        display::graphics.legacyScreen()->pixels()[outline[i]] = 11;   // Color the outline index 11, which should be Yellow
        min_x = MIN(min_x, outline[i] % MAX_X);
        min_y = MIN(min_y, outline[i] / MAX_X);
        max_x = MAX(max_x, outline[i] % MAX_X);
        max_y = MAX(max_y, outline[i] / MAX_X);
    }
}

void PortRestore(unsigned int Count)
{
    int min_x = MAX_X, min_y = MAX_Y, max_x = 0, max_y = 0;
    unsigned int i;
    int loc;

    for (i = 0; i < Count; i++) {
        loc = pPortOutlineRestore[i].loc;
        display::graphics.legacyScreen()->pixels()[loc] = pPortOutlineRestore[i].val;
        min_x = MIN(min_x, loc % MAX_X);
        min_y = MIN(min_y, loc / MAX_X);
        max_x = MAX(max_x, loc % MAX_X);
        max_y = MAX(max_y, loc / MAX_X);
    }

    free(pPortOutlineRestore);
    pPortOutlineRestore = NULL;
}


/** Map a keypress to a spaceport building selection
 */
int MapKey(char plr, int key, int old)
{
    int val, found = 0;
    char high = -1, low = -1;

    for (int j = 0; j < S_MOBJ; j++) {
        if (MObj[j].Reg[Data->P[plr].Port[j]].sNum > 0) {
            if (low == -1) {
                low = j;
            }

            high = j;
        }
    }

    val = old;

    switch (key) {
    case 'A':
        if (MObj[6].Reg[Data->P[plr].Port[PORT_Admin]].sNum > 0) {
            val = PORT_Admin;
        }

        mousebuttons = 1;
        break;

    case 'H':
        if (MObj[8].Reg[Data->P[plr].Port[PORT_MedicalCtr]].sNum > 0) {
            val = PORT_MedicalCtr;
        }

        mousebuttons = 1;
        break;

    case 'I':
        if (MObj[1].Reg[Data->P[plr].Port[PORT_Pentagon]].sNum > 0) {
            val = PORT_Pentagon;
        }

        mousebuttons = 1;
        break;

    case 'L':
        val = PORT_Moon;

        mousebuttons = 1;
        break;

    case 'M':
        if (MObj[5].Reg[Data->P[plr].Port[PORT_Museum]].sNum > 0) {
            val = PORT_Museum;
        }

        mousebuttons = 1;
        break;

    case 'R':
        if (MObj[22].Reg[Data->P[plr].Port[PORT_Research]].sNum > 0) {
            val = PORT_Research;
        }

        mousebuttons = 1;
        break;

    case 'S':
        val = PORT_Satellite;

        mousebuttons = 1;
        break;

    case 'P':
        if (MObj[2].Reg[Data->P[plr].Port[PORT_Capitol]].sNum > 0) {
            val = PORT_Capitol;
        }

        mousebuttons = 1;
        break;

    case 'V':
        if (MObj[4].Reg[Data->P[plr].Port[PORT_VAB]].sNum > 0) {
            val = PORT_VAB;
        }

        mousebuttons = 1;
        break;

    case 'C':
        if (MObj[26].Reg[Data->P[plr].Port[PORT_MissionControl]].sNum > 0) {
            val = PORT_MissionControl;
        }

        mousebuttons = 1;
        break;

    case 'Q':
        if (MObj[29].Reg[Data->P[plr].Port[PORT_Gate]].sNum > 0) {
            val = PORT_Gate;
        }

        mousebuttons = 1;
        break;

    case 'E':
        if (MObj[28].Reg[Data->P[plr].Port[PORT_FlagPole]].sNum > 0) {
            val = PORT_FlagPole;
        }

        mousebuttons = 1;
        break;

    case 'T':
        if (MObj[7].Reg[Data->P[plr].Port[PORT_AstroComplex]].sNum > 0) {
            val = PORT_AstroComplex;
        }

        mousebuttons = 1;
        break;

    case 'B':
        if (MObj[9].Reg[Data->P[plr].Port[PORT_BasicTraining]].sNum > 0) {
            val = PORT_BasicTraining;
        }

        mousebuttons = 1;
        break;

    case UP_ARROW:
        if (old == high) {
            old = 0;
        } else {
            old = old + 1;
        }

        found = 0;

        for (int j = old; j < high + 1; j++) {
            if (MObj[j].Reg[Data->P[plr].Port[j]].sNum > 0) {
                if (found == 0) {
                    val = j;
                    found = 1;
                }
            }
        }

        break;

    case DN_ARROW:
        if (old == low) {
            old = 34;
        } else {
            old = old - 1;
        }

        found = 0;

        for (int j = old; j > low - 1; j--) {
            if (MObj[j].Reg[Data->P[plr].Port[j]].sNum > 0) {
                if (found == 0) {
                    val = j;
                    found = 1;
                }
            }
        }

        break;

    default:
        val = old;
        break;
    }

    return val;
}

void Port(char plr)
{
    int i, j, kMode, kEnt, k;
    char good, res;
    int kPad, pKey, index;
    uint16_t Count, *bone;

    helpText = "i043";
    keyHelpText = "k043";
    bone = (uint16_t *) buffer;
    
	LoadPOutline(plr);
	
    if (plr == 0 && Data->Year > 65) {
        PortText(5, 196, "CAPE KENNEDY", 12);
    } else if (plr == 0) {
        PortText(5, 196, "THE CAPE", 12);
    } else {
        PortText(5, 196, "BAIKONUR", 12);
    }

    pKey = 0;

    music_start((plr == 0) ? M_USPORT : M_SVPORT);
    kMode = kPad = kEnt = 0;
    i = 0;  // this is used to loop through all the selection regions on the port

    while (1) {
        av_block();

        if (kMode == 0) {
            i = 0;
        } else if (kMode == 1) {
            kEnt = 0;
        }

        do {
#if BABYSND
            UpdateAudio();
#endif

            if (pKey == 0) {
                key = 0;
                GetMse(plr, 1);
            } else {
                pKey = 0;
            }

            if (kPad > 0 && kMode == 1) {
                key = kPad;
            }

            if (kMode == 1 && !(x == 319 && y == 199)) {
                kMode = 0;
            }

            if (key > 0) {  // this was only looking for the low byte
                i = MapKey(plr, key, i);  // Get Port offset for keyboard input

                if (key == K_ESCAPE) {
                    kMode = 0;
                    i = 0;
                } else {
                    kMode = 1;
                }

                if (MObj[i].Reg[Data->P[plr].Port[i]].sNum > 0) {
                    x = MObj[i].Reg[Data->P[plr].Port[i]].CD[0].x1;
                    y = MObj[i].Reg[Data->P[plr].Port[i]].CD[0].y1;
                    kEnt = i;
                }
            }

            if (kMode == 1 && kEnt == i) {
                x = MObj[i].Reg[Data->P[plr].Port[i]].CD[0].x1;
                y = MObj[i].Reg[Data->P[plr].Port[i]].CD[0].y1;
            } else if (kMode == 1 && kEnt != i) {
                x = -1;
                y = -1;
            }

            for (j = 0; j < MObj[(kMode == 0) ? i : kEnt].Reg[Data->P[plr].Port[(kMode == 0) ? i : kEnt]].qty; j++) {
                if (x >= MObj[(kMode == 0) ? i : kEnt].Reg[Data->P[plr].Port[(kMode == 0) ? i : kEnt]].CD[j].x1 &&
                    y >= MObj[(kMode == 0) ? i : kEnt].Reg[Data->P[plr].Port[(kMode == 0) ? i : kEnt]].CD[j].y1 &&
                    x <= MObj[(kMode == 0) ? i : kEnt].Reg[Data->P[plr].Port[(kMode == 0) ? i : kEnt]].CD[j].x2 &&
                    y <= MObj[(kMode == 0) ? i : kEnt].Reg[Data->P[plr].Port[(kMode == 0) ? i : kEnt]].CD[j].y2) {
                    PortText(5, 196, MObj[i].Name, 11);

                    if (MObj[i].Reg[Data->P[plr].Port[i]].sNum > 0) {
                        index = MObj[i].Reg[Data->P[plr].Port[i]].sNum;
                        Count = pOutline[index].Count;
                        bone = new uint16_t[pOutline[index].bone.size()];
                        std::copy(pOutline[index].bone.begin(), pOutline[index].bone.end(), bone);
      
                        PortOutLine(Count, bone, 1);
                        delete [] bone;
                        strncpy(&helpText[1], (MObj[i].Help).c_str(), 3);
                    }

                    good = 0;

                    // Search hotkey string for valid selection
                    for (k = 0; k < (int)strlen(HotKeyList); k++) {
                        if (HotKeyList[k] == ((char)(0x00ff & key))) {
                            good = 1;
                        }
                    }

                    while (x >= MObj[i].Reg[Data->P[plr].Port[i]].CD[j].x1 &&
                           y >= MObj[i].Reg[Data->P[plr].Port[i]].CD[j].y1 &&
                           x <= MObj[i].Reg[Data->P[plr].Port[i]].CD[j].x2 &&
                           y <= MObj[i].Reg[Data->P[plr].Port[i]].CD[j].y2) {
                        av_block();
#if BABYSND
                        UpdateAudio();
#endif
                        kPad = 0;
                        key = 0;
                        GetMse(plr, 1);

                        if (kMode == 0 && key > 0) {
                            x = y = 0;
                            pKey = key;
                        }

                        if (kMode == 1 && !(x == 319 && y == 199)) {
                            kMode = 0;
                        }

                        if (kMode == 1) {
                            x = MObj[i].Reg[Data->P[plr].Port[i]].CD[0].x1;
                            y = MObj[i].Reg[Data->P[plr].Port[i]].CD[0].y1;
                        }

                        if (key > 0 && kMode == 1)  // got a keypress
                            if (key != K_ENTER) {  // not return
                                x = -1;
                                y = -1;
                                kPad = key;
                            }

                        if (good == 1 || (kMode == 0 && mousebuttons == 1) || (kMode == 1 && key == K_ENTER)
                            || (kMode == 0 && key == K_ENTER)) {
                            PortRestore(Count);
                            Count = 0;

                            // || i==33

                            if (!(i == PORT_FlagPole || i == PORT_Gate ||
                                  i == PORT_Monument || i == PORT_SovMonumentAlt ||
                                  (Data->Year == 57 || (Data->Year == 58 && Data->Season == 0)))) {
#if SPOT_ON
                                SpotKill();
#endif
                                music_stop();
                            } else {
                                SpotPause();
                            }

                            res = PortSel(plr, i);

                            switch (res) {
                            case pNOREDRAW:
                                PortText(5, 196, MObj[i].Name, 11);
                                break;

                            case pREDRAW:
                            case pNEWMUSIC:
                            case pNOFADE:
                                if (res != pNOFADE) {
                                    music_stop();
                                    music_start((plr == 0) ? M_USPORT : M_SVPORT);
                                }

                                SpotKill();

                                // Returning to spaceport so fade between redraws
                                if (res == pREDRAW) {
                                    FadeOut(2, 10, 0, 0);
                                }

                                DrawSpaceport(plr);

                                if (res == pREDRAW) {
                                    FadeIn(2, 10, 0, 0);
                                }

#if SPOT_ON
                                SpotRefresh();
                                SpotLoad(SpaceportAnimationOngoing(plr));
#endif
                                Vab_Spot = 0;
#ifdef DEADCODE
                                // I'm not sure why we're redrawing the outlines here;
                                // commenting it out for now.  If no problems are seen
                                // with the port outlines then restore this
                                //   if (pPortOutlineRestore)
                                //      PortOutLine(Count,bone,0);
#endif
                                PortText(5, 196, MObj[i].Name, 11);
                                break;

                            case pEXIT:
                                FadeOut(2, 10, 0, 0);
                                //fclose(fin);
#if BABYSND

                                if (i == 28 || i == 29) {
                                    SpotResume();
                                }

#endif
                                SpotKill();
                                music_stop();
                                autosave_game("AUTOSAVE.SAV");
                                return;

                            case pQUIT:
                                FadeOut(2, 10, 0, 0);
#if BABYSND

                                if (i == PORT_FlagPole || i == PORT_Gate) {
                                    SpotResume();
                                }

#endif
                                SpotKill();
                                music_stop();
                                return;
                            }  // switch

                            kMode = good = 0;
                            SpotResume();

                            if (MObj[i].Reg[Data->P[plr].Port[i]].sNum > 0) {
                                
                        		index = MObj[i].Reg[Data->P[plr].Port[i]].sNum;
				                Count = pOutline[index].Count;
				                bone = new uint16_t[pOutline[index].bone.size()];
                    			std::copy(pOutline[index].bone.begin(), pOutline[index].bone.end(), bone);                                

                                PortOutLine(Count, bone, 1);
                                delete [] bone;
                            }

                            while (mousebuttons == 1) {
                                GetMse(plr, 1);
                            }
                        }  // if
                    }  //while

                    if (plr == 0 && Data->Year > 65) {
                        PortText(5, 196, "CAPE KENNEDY", 12);
                    } else if (plr == 0) {
                        PortText(5, 196, "THE CAPE", 12);
                    } else {
                        PortText(5, 196, "BAIKONUR", 12);
                    }

                    PortRestore(Count);
                    Count = 0;
                    helpText = "i043";
                    keyHelpText = "k043";
                }  // if
            }

            if (kMode == 0) {
                i++;
            }

            if (kMode == 1) {
                kEnt++;
            }
        } while ((kMode == 0 && i < S_MOBJ && i >= 0) 
    	  || (kMode == 1 && kEnt < S_MOBJ && kEnt >= 0));
    }  // while
}


/**
 * This is the code that controls the jumpoff point from the Spaceports
 * to the various areas.  It basically assigns a help message, then
 * makes a call into the module - which would have its own event loop.
 */
char PortSel(char plr, char loc)
{
    int MisOK;
    Vab_Spot = 0;  // clear the damn thing.

    switch (loc) {
    case PORT_Monument:
        Help((plr == 0) ? "i023" : "i022");
        keyHelpText = "k022";
        return pNOREDRAW;  // Monuments

    case PORT_Pentagon:
        if (Data->Year == 57 || (Data->Year == 58 && Data->Season == 0)) {
            Help("i108");
            return pNOREDRAW;
        }

        helpText = "i027\0";
        Intel(plr);
        return pNEWMUSIC;

    case PORT_Capitol:
        helpText = (plr == 0) ? "i021" : "i532";
        keyHelpText = (plr == 0) ? "k021" : "k532";
        Review(plr);
        return pREDRAW;

    case PORT_Cemetery:
        helpText = "i020";
        Cemetery(plr);
        return pREDRAW;

    case PORT_VAB:
        if (Option != -1) {
            put_serial(LET_V);
            put_serial(LET_V);
            put_serial(LET_V);
        }

        helpText = "i015";
        VAB(plr);
        return pREDRAW;

    case PORT_Museum:
        if (Option != -1) {
            put_serial(LET_M);
            put_serial(LET_M);
            put_serial(LET_M);
        }

        helpText = "i027";
        Museum(plr);
        return pNEWMUSIC;

    case PORT_Admin:
        if (Option != -1) {
            put_serial(LET_A);
            put_serial(LET_A);
            put_serial(LET_A);
        }

        helpText = "i027";
        Admin(plr);

        if (LOAD == 1) {
            return pEXIT;
        } else if (QUIT == 1) {
            return pQUIT;
        } else {
            if (plr == 0) {
                Vab_Spot = 4;
            }

            return pNEWMUSIC;
        }

    case PORT_AstroComplex:
        helpText = "i039";
        Limbo(plr);
        return pREDRAW;  // Astro Complex

    case PORT_MedicalCtr:
        helpText = "i041";
        Hospital(plr);
        return pREDRAW;

    case PORT_BasicTraining:
        helpText = "i038";
        Train(plr, 0);
        return pREDRAW;

    case PORT_Helipad:
        helpText = "i037";
        Train(plr, 2);
        return pREDRAW;

    case PORT_Pool:
        helpText = "i037";
        Train(plr, 3);
        return pREDRAW;

    case PORT_Planetarium:
        helpText = "i037";
        Train(plr, 4);
        return pREDRAW;

    case PORT_Centrifuge:
        helpText = "i037";
        Train(plr, 5);
        return pREDRAW;

    case PORT_Airfield:
        helpText = "i037";
        Train(plr, 1);
        return pREDRAW;

    case PORT_Satellite:
        helpText = "i019";
        SatBld(plr);
        return pREDRAW;

    case PORT_LM:
        helpText = "i044";
        keyHelpText = "k209";
        LMBld(plr);
        return pREDRAW;  // LM Program

    case PORT_Jupiter:
        helpText = "i036";
        Programs(plr, 5);
        return pREDRAW;

    case PORT_XMS:
        helpText = "i036";
        Programs(plr, 4);
        return pREDRAW;

    case PORT_Apollo:
        helpText = "i036";
        Programs(plr, 3);
        return pREDRAW;

    case PORT_Gemini:
        helpText = "i036";
        Programs(plr, 2);
        return pREDRAW;

    case PORT_Mercury:
        helpText = "i036";
        Programs(plr, 1);
        return pREDRAW;

    case PORT_Research:
        if (Option != -1) {
            put_serial(LET_R);
            put_serial(LET_R);
            put_serial(LET_R);
        }

        helpText = "i009";
        RD(plr);

        if (plr == 1) {
            Vab_Spot = 2;
        }

        return pREDRAW;

    case PORT_LaunchPad_A:
    case PORT_LaunchPad_B:
    case PORT_LaunchPad_C:
        helpText = "i028";
        ShowPad(plr, loc - 23);
        return pREDRAW;

    case PORT_MissionControl:
        helpText = "i018";
        keyHelpText = "k018";
        MisOK = MissionStatus(plr);

        if (MisOK == MISSIONS_NONE) {
            Help("i104");
        } else if (MisOK == MISSIONS_UNSTAGED) {
            Help("i005");
        } else {
            Rush(plr);
            RUSH = 1;
            return pREDRAW;
        }

        return pNOFADE;

    case PORT_ViewingStand:
        helpText = "i017";
        keyHelpText = "k017";
        Viewing(plr);
        return pREDRAW;

    case PORT_FlagPole:  // Flagpole : End turn

        if (EndTurnOk(plr)) {
            if (Option != -1 && MissionStatus(plr) == MISSIONS_READY) {
                put_serial(LET_O);
                put_serial(LET_O);
                put_serial(LET_O);
            }

            RUSH = 0;
            return pEXIT;
        }

        return pNOREDRAW;

    case PORT_Gate: // Security Gate: Quit
        QUIT = Request(plr, "QUIT", 1);

        if (QUIT) {
            if (Option != -1) {
                DoModem(2);
            }

            return pQUIT;
        }

        return pNOREDRAW;

    case PORT_Moon:
        helpText = "i029";
        Moon(plr);
        return pREDRAW;  // Moon

    case PORT_SovMonumentAlt:
        Help("i025");
        return pNOREDRAW;  // Sov Mon #2

    case PORT_Zond:
        helpText = "i036";
        Programs(plr, 3);
        return pREDRAW;  // Zond

    case PORT_Tracking:
        if (Option != -1) {
            MesCenter();    // Tracking
            Vab_Spot = 3;
            return pREDRAW;
        } else {
            Help("i042");
            Vab_Spot = 3;
            return pNOFADE;
        }

    case PORT_SVHQ:
        return pNOREDRAW;  // SV

    default:
        return pNOREDRAW;
    }
}


char Request(char plr, const char *s, char md)
{
    char i;
    display::LegacySurface local(196, 84);

    if (md > 0) {  // Save Buffer
        local.copyFrom(display::graphics.legacyScreen(), 85, 52, 280, 135);
    }

    i = strlen(s) >> 1;
    display::graphics.setForegroundColor(0);
    ShBox(85, 52, 249, 135);
    IOBox(170, 103, 243, 130);
    IOBox(91, 103, 164, 130);
    InBox(92, 58, 243, 97);
    display::graphics.setForegroundColor(1);
    draw_heading(111, 110, "YES", 0, 0);
    draw_heading(194, 110, "NO", 0, 0);

    display::graphics.setForegroundColor(11);

    if (md == 6)  {
        draw_string(166 - i * 7, 65, s);
    } else {
        draw_heading(166 - i * 10, 65, &s[0], 0, -1);
    }

    draw_string(135, 94, "ARE YOU SURE?");

    while (1) {
        if (md != 6) {
            GetMse(plr, 1);
        } else {
            GetMouse();
        }

        if (mousebuttons == 0) {
            break;
        }
    }

    i = 2;

    while (i == 2) {
        if (md != 6) {
            GetMse(plr, 1);
        } else {
            GetMouse();
        }

        if ((x >= 172 && y >= 105 && x <= 241 && y <= 128 && mousebuttons != 0) || (key == 'N')) {
            InBox(172, 105, 241, 128);
            i = 0;
            delay(50);
            key = 0;
        }

        if ((x > 93 && y >= 105 && x <= 162 && y <= 128 && mousebuttons != 0) || (key == 'Y')) {
            InBox(93, 105, 162, 128);
            i = 1;
            delay(50);
            key = 0;
        }

        delay(50);
    }  /* End while */

    if (md > 0) {
        display::graphics.legacyScreen()->palette().copy_from(local.palette());
        display::graphics.screen()->draw(local, 85, 52);
    }

    return i;
}


/**
 * Confirm all scheduled missions are a go.
 *
 * Launches a pop-up to review the missions scheduled for launch this
 * turn. A more advaned version of the existing setup, providing a
 * summary of the missions and access to Mission Control.
 *
 * TODO: This is an unfinished function that is never called.
 *
 * \param plr
 * \return  0 or 1 (Unsure which is proceed and which not, b/c unfinished).
 */
char MisReq(char plr)
{
    int i, num = 0;
    display::LegacySurface local(184, 132);

    local.copyFrom(display::graphics.legacyScreen(), 53, 29, 236, 160);

    for (i = 0; i < 3; i++) {
        if ((Data->P[plr].Mission[i].MissionCode) &&
            (Data->P[plr].Mission[i].Hard[Mission_PrimaryBooster] == 0)) {
            num++;
        }
    }

    ShBox(53, 29, 236, 160);
    ShBox(60, 34, 229, 44);
    InBox(60, 47, 229, 120);
    // fill_rectangle(61,48,228,119,0);
    ShBox(63, 50, 226, 117);

    if (num == 0) {
        IOBox(60, 141, 141, 155);
        IOBox(148, 141, 229, 155);
        display::graphics.setForegroundColor(1);
        draw_string(70, 129, "CONFIRM LAUNCH SCHEDULE OR");
        draw_string(80, 136, "CHOOSE TO REVIEW IT.");
        display::graphics.setForegroundColor(8);
        draw_string(85, 150, "C");
        display::graphics.setForegroundColor(1);
        draw_string(0, 0, "ONFIRM");
        display::graphics.setForegroundColor(8);
        draw_string(179, 150, "R");
        display::graphics.setForegroundColor(1);
        draw_string(0, 0, "EVIEW");
    } else {
        IOBox(60, 141, 229, 155);
        display::graphics.setForegroundColor(1);
        draw_string(62, 129, "MISSIONS DO NOT ALL HAVE");
        draw_string(62, 136, "ASSIGNMENTS. NO COMMIT POSSIBLE");
        draw_string(120, 150, "REVIEW MISSIONS");
    }

    display::graphics.setForegroundColor(10);

    draw_string(94, 41, "LAUNCH CONFIRMATION");

    display::graphics.setForegroundColor(1);

    for (i = 0; i < 3; i++) {
        draw_string(68, 59 + 20 * i, "PAD ");
        draw_character(0x41 + i);
        draw_string(0, 0, ": ");

        if (Data->P[plr].Mission[i].MissionCode) {
            draw_string(0, 0, &Data->P[plr].Mission[i].Name[0]);

            if (Data->P[plr].Mission[i].Men > 0) {
                draw_string(86, 65 + 20 * i, "MANNED MISSION");
            } else {
                draw_string(86, 65 + 20 * i, "UNMANNED MISSION");
            }

            if (Data->P[plr].Mission[i].Hard[Mission_PrimaryBooster] == 0) {
                display::graphics.setForegroundColor(9);
                draw_string(86, 71 + 20 * i, "HARDWARE UNASSIGNED");
            } else {
                draw_string(86, 71 + 20 * i, "HARDWARE ASSIGNED");
            }

            display::graphics.setForegroundColor(1);
        }
    }

    while (1) {
        if (plr == 0) {
            GetMse(plr, 1);
        } else {
            GetMouse();
        }

        if (mousebuttons == 0) {
            break;
        }
    }

    i = 2;

    while (i == 2) {
        if (plr == 0) {
            GetMse(plr, 1);
        } else {
            GetMouse();
        }

        if ((x >= 62 && y >= 143 && x <= 139 && y <= 153 && mousebuttons != 0 && num == 0)
            || (key == 'C' && num == 0)) {
            InBox(62, 143, 139, 153);
            i = 1;
            delay(50);
            key = 0;
            Rush(plr);
        }

        if ((x > 150 && y >= 143 && x <= 227 && y <= 153 && mousebuttons != 0 && num == 0)
            || (key == 'R' && num == 0)) {
            InBox(150, 143, 227, 153);
            i = 0;
            delay(50);
            key = 0;
        }

        if ((x > 62 && y >= 143 && x <= 227 && y <= 153 && mousebuttons != 0 && num > 0)
            || (key == 'R' && num > 0)) {
            InBox(62, 143, 227, 153);
            i = 0;
            delay(50);
            key = 0;
        }
    }  /* End while */

    local.copyTo(display::graphics.legacyScreen(), 53, 29);

    return i;
}


int SpaceportAnimationEntry(int plr)
{
    bool helipadBuilt = Data->P[plr].Port[PORT_Helipad] > 0;
    bool crewInTraining = false;

    for (int i = 0; i < Data->P[plr].AstroCount; i++) {
        if (Data->P[plr].Pool[i].Status >= AST_ST_TRAIN_BASIC_2) {
            crewInTraining = true;
        }
    }

    int roll = brandom(1000);

    if (xMODE & xMODE_CLOUDS) {
        if (plr == 0 && Data->P[plr].Port[PORT_VAB] == 0) {
            return USA_STORM_CLOUDS;
        } else if (plr == 1) {
            return SOV_STORM_CLOUDS;
        }
    } else if ((xMODE & xMODE_SPOT_ANIM) && helipadBuilt) {
        xMODE &= ~xMODE_SPOT_ANIM;
        return plr == 0 ? USA_LM_CRASH : SOV_LM_CRASH;
    } else if (crewInTraining && helipadBuilt) {
        return plr == 0 ? USA_LM_TEST : SOV_LM_TEST;
    } else if (roll < 150) {
        if (plr == 1 && Data->P[plr].Port[PORT_Airfield] == 1) {
            return SOV_NEW_PLANE;
        } else {
            return plr == 0 ? USA_PLANE_FLY_BY : SOV_PLANE_FLY_BY;
        }
    } else if (roll >= 850) {
        return plr == 0 ? USA_HELICOPTER : SOV_HELICOPTER;
    }

    return SPOT_NONE;
}


int SpaceportAnimationOngoing(int plr)
{
    int roll = brandom(100);

    if (Vab_Spot == 1 && Data->P[plr].Port[PORT_VAB] == 2) {
        Data->P[plr].Port[PORT_LaunchPad_A] = 1;

        if (plr == 0) {
            if (roll <= 60) {
                return USA_ROCKET_TO_PAD;
            } else {
                return USA_ROCKET_TO_VAB;
            }
        } else if (plr == 1) {
            return SOV_ROCKET_TO_PAD;
        }
    } else if (Vab_Spot == 4 && plr == 0 && Data->P[plr].Port[PORT_VAB] == 0) {
        return USA_ROTATING_CRANE;
    } else if (Vab_Spot == 2 && plr == 1) {
        return SOV_GATE;
    } else if (Vab_Spot == 3) {
        if (plr == 1) {
            return SOV_TRACKING;
        } else if (plr == 0) {
            return USA_TRACKING;
        }
    } else if (roll < 30) {
        if (plr == 1 && Data->P[plr].Port[PORT_MedicalCtr] == 1) {
            return SOV_NEW_PLANE;
        } else {
            return plr == 0 ? USA_PLANE_FLY_BY : SOV_PLANE_FLY_BY;
        }
    } else if (plr == 1 && roll < 40) {
        return SOV_GATE;
    } else if (roll < 60) {
        return plr == 0 ? USA_HELICOPTER : SOV_HELICOPTER;
    }

    return SPOT_NONE;
}


// Deserialize Count and bone data in pOutline vector
void LoadPOutline(int plr) {
	
	std::string filename;
	filename = (plr == 0) ? "usa_port.json" : "sov_port.json";
	std::ifstream file(locate_file(filename.c_str(), FT_DATA));
	  if (!file) {
	  throw std::runtime_error(filename + " could not be opened.");  
	}

	cereal::JSONInputArchive ar(file);
	ar(CEREAL_NVP(pOutline));
	INFO1("pOutline succesfully uploaded.");
}


// Edit r settings {{{
// ex: ts=4 noet sw=2
// ex: foldmethod=marker
// }}}
