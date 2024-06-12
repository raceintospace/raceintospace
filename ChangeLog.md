# Race into Space changes

## Version 2.0

### BACK-END CHANGES

* Game brought over from Sourceforge to GitHub
* Versioning system converted from CVS to GIT
* Code converted from C to C++
* Code cleanup to make the game easier to modify (#623, #626, #639, #644, #645, #783, among others)
* Parts of the code brought into line with standards (#624, #783)
* Improved readability of code in spaceport animation (#800)
* Removed legacy code for compressing save games (#777)
* Many of the data files and images have been changed to use discrete image files rather than being embedded into the code (#51)
* Vestigial copy protection code finally removed (#493)
* We've added some JSON serialization for game data and save games. (#478)
* Deserialization of JSON files has been moved to the initialization of the game, improving its performance (#774)
* Removed redundant code in the spaceport menus that redrew the date and cash values (#773)
* The abbreviation for the sound to play in the Intelligence screens was "INTELLEG"&mdash;now "INTELLIG" (#523, #539)
* In the config file, "altasLunar" has been corrected to "atlasLunar" (#520)
* The music files have been moved to a separate repository for copyright reasons.
* Fixed several out-of-bounds memory reads in the AI and astronaut code, which may have led to inconsistent AI behavior, or doing things it shouldn't (#825)
* Removed several unused variables and renames others given nonsensical names (#846)
* Added an optional alignment argument to the draw_string method to make text centering easier (#842)

Since transitioning to C++ on GIT, the game has been brought up to where we left off (that is, bugs introduced by the transition have been fixed).

Below are improvements that have been made over what the game offered in v.1.1.

---
### BUG FIXES AND PROBLEMS SOLVED

**LARGER ITEMS**
* The **duration penalty system** has been fixed, and is working right for the first time ever (#246)
   * This makes a final solution to what has sometimes been called the "Duration B Bug", where your first duration mission (or sometimes all of them) would be assessed a duration penalty even if you weren't skipping a level
* The newscast that announces all manned missions will be canceled this turn no longer cancels unmanned 
  missions as well (#164)
* 'Nauts killed in a newscast (train accident, etc.) are now removed immediately (#240)
   * Before, they wouldn't be removed until the start of the following turn, making it possible for the dead crew member to fly on a mission
* If you flew a Docking Duration mission and the Docking step failed, it would cancel the remainder of the mission without attempting Duration (#322)
* If you flew a Lunar Orbital LM Test and the LM Test part failed, the game didn't give you credit for having flown the Lunar Orbital, even though the lunar orbital part was successful. (#332)
* When the LM failed to lift off saying "Engine explodes upon ignition. Spacecraft crew is killed." it used to kill _all_ crew members, including the one in orbit (#476)
* If the LM lifts off from the Moon but fails to develop full thrust and makes an emergency landing back on the surface, stranding the crew, the Docking Specialist up in orbit died too (but managed to bring the capsule home anyway) (#287)
---
**SMALLER ITEMS**
* **Newscast issues**
   * The Soviet newscast now plays the Soviet newscast music (#214)
      * Starting with the CD version of BARIS, and in RIS through v.1.1, it played the US newscast music
      * The Soviet music improves the timing so that the movie's video better matches the voice of the newscaster
   * The newscast announcing that a storm front has blocked a launch window now specifies which pad (#169)
      * Before, it had called it "the H launch facility" (since h is used as a variable).
   * When a component faces a Safety penalty unless the player pays to fix it, there was sometimes a mismatch between newscast and actual numbers (#204, #243)
   * The VAB/VIB wasn't properly reporting damage to rockets (where you're required to pay xMB or suffer y% Safety loss on its next mission).
   * When a component faces a Safety penalty unless the player pays to fix it, it didn't make sure you had enough cash to pay, allowing you to have a negative number of MBs (#268) [not sure if this was in 1.1]
   * The newscast saying a given piece of hardware has lost x% Safety could sometimes claim it lost more than it actually had (#569)
   * The second of the two newscasts announcing that the primary crew is sick and will be replaced with the backup crew never worked before; now it does (#286, #325)
   * The newscast stating that an astronaut left to become a test pilot, or has been transferred for reeducation, instead sent them to the hospital/infirmary (#290)
   * There were three newscasts which were supposed to reduce your budget by 20, 25, or 30 MBs, or by 20%, 25%, or 30% of your budget, whichever is less.  Instead, if 20/25/30% was less than 20/25/30 MB, it reduced your budget *****to***** 20/25/30% of what it had been, causing your budget to drop like a rock. (#394)
   * The newscast that delayed a mission on the pad used to delay missions that couldn't have been put back a turn, such as Mars/Jupiter/Saturn flybys (fixed as part of #167)
   * There was also a newscast that was supposed to increase your budget by 20MB, but the conditions under which it was allowed effectively prevented it ever happening (#397)
   * When a newscast gives you three Air Force pilots, they now come with skills. They used to start out in Basic Training with 0s in everything (#541) 
   * Air Force Astronauts didn't show a Group. In Basic Training it was blank, and in History they showed as Group X. Now in Basic they show as Group A.F. (for Air Force). In Astronaut History they show as Group USAF; in Cosmonaut History they show as Group VVS, the abbreviation for the Soviet Air Force. (#669)
   * When the newscast reported that a recruit in Basic Training was a physical washout, it used to send them to the Hospital, then return them to service instead of turning them out (#315)
   * Mission injuries that led to immediate retirement would retire someone without telling you. Now the retirement is announced in the newscast. (#568)
* **Mission Failures**
   * On a mission where the rocket explodes but the crew escapes thanks to the Launch Escape System pulling the capsule to safety, you still got credit for Manned Space Mission.  Now you won't get credit unless the mission reaches the Reentry step. (#330)
   * On a lunar landing, "Permanent communications loss with ground control. Mission is scrubbed." would ask who should be first on the Moon, but it shouldn't have because no one would be getting out. (#514)
   * When the capsule's emergency-eject system has to be used, the mission would often continue right after, or say it was used successfully but all the crew shows as dead (#244)
   * Fixed a bug where a rocket suffers a major fire on the pad, destroying it; the crew survives, but without any mention of the emergency-eject system being used (#547)
   * If your lunar landing failed with the message "Maneuvering failure, craft lands in rough terrain causing damage", the mission would go straight to Lunar Orbital Activities, skipping the liftoff (#505)
   * A Joint lunar mission that manages to get the unmanned component into orbit would give credit for performing a manned lunar landing, even if the manned part of the mission failed. (#678)
   * An "A-OK" failure on an LEM Thrust Test (that is, one that let the test continue successfully) used to not award you LM Point(s) for the test (#441)
   * If an emergency EVA was needed to get back to the capsule after the Moon landing, the game wouldn't give you credit for the landing (#741)
   * A launch failure on a manned mission could sometimes activate the emergency ejection system, show the capsule jetting away successfully, but then crashing. The text, however, would erroneously tell you that the capsule was pulled down by the boosters. (#725)
   * On the lunar landing mission, if the LM failed to dock with the capsule (but crew spacewalked successfully to the capsule), the game would deny you credit for the landing (#741)
* **Game Crashes**
   * Fixed an out-of-bounds write in the VAB when reassembling hardware, which led to corrupt data and an assert failure upon autosave. (#704)
   * Fixed an array overflow error that used to occasionally crash toward the end of the game (#40, #200)
   * Fixed a bug that caused the game to crash with a black screen, if the AI can't find an available crew to send on the mission it's about to fly. This happened most often when it was flying direct ascent. (#552)
   * There were two bugs that would cause the game to do a core dump and close if your LM made a fiery impact on the lunar surface.  One of them has been fixed (#501)
   * The game would sometimes crash during intelligence report generation if the player was flying the Soyuz Lunar Landing (mission 57), because it wasn't set to accommodate missions above 56. (#675)
   * The event counter's "sanity check" was firing one turn too early, causing the game to crash in Fall 1977 (#779)
   * Fixed a crash when GetEquipment was unable to find a docking module for a Joint mission because the DM was on the other pad (#778)
   * The game would sometimes crash when a Soyuz Lunar Landing mission was looking for LM hardware (#776)
   * Fixed bugs that caused the game to crash on a code 9 failure on launch, or sometimes on other mission steps (#687 and #663)
   * An LM test with Apollo/Soyuz could lead to docking steps not being carried out, leading to an infinite loop that made the game crash (#721)
   * There were some problems with downgrades.  Some downgrades were to missions that had more hardware than the original, and downgrading a Soyuz LL to an LM Test was problematic.  Both could make the game crash (#726)
   * A catastrophic failure on a docking test, where the capsule crashed into the DM, could sometimes assign a Max R&D value to the docking module (#730)
* **PBEM**
   * When starting a PBEM game when the previous game had had the US played by the AI, the AI flags would not reset, causing the game to crash. (#706)
   * DM safety was sometimes raised twice in PBEM games (#701)
   * There was an error in the awarding of prestige for capsule firsts which were not properly recorded in PBEM games (#700)
   * There was a bug causing a duration step to not be awarded in PBEM games (#695)
   * Fixed an error when autosaving in PBEM games which led to some data used for array indexing containing unsafe values. This could lead to a buffer overflow showing strange behavior in the Preferences screen (#699)
* **Interface Fixes**
   * Fixed a bug in Future Missions: if you chose a mission and proceeded to where you select a capsule or crew, then cancel&mdash;then press Continue again (either after choosing a different mission or deciding to go ahead with the same one), and you select capsule or crew, when you go back to Future Missions you find the mission canceled (#222)
   * Fixed a bug in the VAB/VIB: with some payloads, the color of the rocket/booster combination wasn't changing to reflect whether it could lift the currently selected payload (#298)
     * This was noticed with an XMS-2 Lunar Pass using B/Titan and a B-Kicker.  When you cycled to B/TITAN, the Current Payload displayed in dark red to show that a boosted Titan couldn't lift that combo, but "B/TITAN" under Rocket still showed in white, sometimes leading the player to purchase hardware that couldn't do the job.
   * The Jupiter Flyby was set to take 7 turns to arrive, the same as Saturn&mdash;now it's set to 4, as it should be (#171)
   * The game used to allow you to assign Duration missions with an illegally low duration (#257, #278)
   * BARIS allowed you to use PgUp and PgDn to scroll through the newscast more easily, both at the start of your turn and later in the Viewing Stand.  That functionality was lost in RIS because DOS has a peculiar way of processing those keys, but it's now been restored&mdash;along with the Home and End keys.
   * The Viewing Stand used to show the game's first newscast twice, for both Spring and Fall 1957, so it never showed the player's second newscast of the game. That's fixed now. (#696)
   * In recruitment, if you clicked and held on the up or down arrow, it would take 3 or 4 seconds before the list would start to scroll.
   * The Step Failure screen didn't show the correct safety factor (#578, #648)
   * The Purchase button in Hardware Purchasing formerly didn't animate if you pressed P, though it would if you clicked on the button. (#841)
* **Esthetic Fine-Tuning**
   * Removed visual artifacts next to astronaut wings and medals in Astronaut/Cosmonaut History (#299, from #192)
   * Better centering of astronaut wings in Astro/Cosmonaut history.
   * In the screen that asks who will be first on the Moon, the names have been centered in their respective buttons. Longer names were bleeding out past the edge of the button. (#535)
   * In the Capsule Screen, if you tried to break an empty crew, it would tell you the crew was assigned as the backup crew for a current mission&mdash;now it says it's an empty crew (#348)
   * The image for a damaged/destroyed Soviet launch pad instead showed a functioning pad with a rocket ready for launch. It now shows a damaged pad. The damaged-pad icon for the US wasn't too obviously damaged either, so now it's more so. (#412)
   * Addressed an oversight: in recruitment, if you dismissed a candidate, the number of available spots didn't show any higher on the right. (#581)
   * The number of spots to fill in recruitment now shows when you first open recruitment, rather than after you've selected one to recruit.
   * In recruitment, the Continue button didn't look quite right when clicked on (off by 1px).
   * In Preferences, the Human/AI button nestled into the world map never looked quite right on the Soviet side.  Now it does. (#353)
   * Player names with maximum length used to go off the right margin in the endgame (#719) 
   * Small updates to the endgame text to prevent text going off the righthand side
* There was a rarely encountered bug with damaged hardware (pay xMB or lose y% Safety) where paying to fix it didn't repair the damage (#737)
* Fixed a bug with technology transfer for the Titan/Proton rocket: the technology bonus from multiple rockets at 75%+ was not being applied (#310)
* Because of the nature of the variable for 'naut mood, after a successful mission it could roll over above 100, which would result in a very low number, potentially leading to two entire flight crews resigning out of sudden bitterness during their moment of triumph (#484)
* Resolved excessive astronaut/cosmonaut retirement checks to match game's original intent (#428)
* The analog clocks in Mission Control were behaving erratically, skipping from 10:30 directly to 12:00 (#796)
* When you're flying docking tests, the game used to get stuck sometimes at 2 DMs in your inventory. For the rest of the game you always had two docking modules and didn't have to buy them again (#319)
  [not sure if this was in 1.1]
* If you have a craft that does not require a docking module to dock with an LM (i.e., Apollo/Soyuz), the docking step was rolled against the safety of the LM and not against the docking module (#715)
* Certain mission launch steps involving a minishuttle defaulted to the capsule failure cases.
* Fixed a bug that led the game to not count Duration levels achieved on Jt Orbital Docking Duration missions (#690)
* Penalties were doubled if hardware was used twice on Joint missions (#709)
* Scrubbing a mission in the launch pad screen failed to free up any hardware you had purchased for it (#292)
* If you had two missions ready to launch, and the first was successful and flown with hardware at Max Safety, if the second used the same hardware, that hardware would fly at 1 point _above_ Max Safety (#614)
* Resolved inconsistencies between the code and the comments when the game checks to see if people should retire.
* Sometimes mission reply data wasn't saved, so replays in Space History didn't work (#768)
* The bottom bar in the Space History screen would sometimes get messed up (#768)
* When a save is loaded in the Fall, the game would create an extra new intelligence briefing. This could potentially have created an array overflow and overwriting of variables in games that were saved multiple times (#217)
* On the lunar landing mission, the Photo Recon check was rolling against the Safety of the wrong device (#482)
* Director rankings were being calculated improperly (#580)
* Mission Control and the Launch Facility screen used to play the other side's music: Soviet instead of US, US instead of Soviet (#258)
* The event counting system had a bug when the AI played as the US, which could cause its director ranking to tank even when its prestige was high (#683)
* The terminal window would sometimes give the error that the LM should have 1 crew, but only found 0 (#607)
* Fixed some memory leaks in the game (#845)

---
### GAME CHANGES

**LARGER ITEMS**
* Since the **duration penalty system** now works properly, we've made it possible to downgrade to a lower Duration level in Mission Control (#255)
   * e.g., if you have a Duration C ready to go and schedule a Duration D for next turn, but something goes wrong and the first flight doesn't accomplish Duration C, next turn you can downgrade the D mission to C to avoid the -5 penalty for skipping a level.
* We have added back **Play By Email** (PBEM).  It worked in the CD version of BARIS, but was removed from RIS. (#131) 
* We've changed the turn order for PBEM games to AB|BA|AB|B..., where A=US and B=USSR and the pipe denotes the end of the turn (called "snake mode"). This has the advantage of letting each user play two turns before having to send the PBEM file to their opponent. (#691)
* If you have no missions planned for next turn when you hit the Flagpole, you now receive a warning about it. (#166)
* When you open R&D, you will receive a warning if (#166):
   * You've prototyped an LM or Kicker-C but haven't started docking modules yet
   * You've prototyped a landing craft (LM, Kicker-C, direct ascent) but haven't started EVA suits yet
   * You've prototyped Gemini/Voskhod or a minishuttle, but haven't started a kicker yet
* If your cash is at or below 60MB (20MB before your first capsule) and you have mission(s) scheduled for this turn but haven't assembled them yet, the R&D screen will now pop up a reminder to assemble the mission(s) before spending your money on research (#511)
* The VAB/VIB now has a **Delay** button to let you put a launch back a turn rather than having to scrub it and reschedule. (#167)
* The Astronaut Complex / Cosmonaut Center now has a SEND ALL button that sends everyone (except those who have announced retirement or have very low morale) to the capsule/shuttle program you select (#811)
* The Max R&D figures for Mercury and Gemini in the Historical Model have been reset to their levels in the floppy version (#216)
---
**SMALLER ITEMS**
* **VISUAL CUES**
   * In R&D and Purchasing, clicking on the image of the currently displayed hardware item will now pop up a help message giving information about it (#467)
   * Similarly, in Future Missions, the name box is now a button that brings up a description of the currently selected mission. (#492)
   * In Future Missions, the part of the screen that shows the safety risk is now a button that will launch a pop-up breaking down your requirement penalty into milestone, duration, and new mission penalties.  It also shows penalties on the LM steps of lunar landings. (#168, #373)
   * In Future Missions, when you select crews, now skills are highlighted only when they'll be used on the mission.  LM lights up only on missions that use a lunar module, for instance. 
      * LM Tests without EVA now highlight EV, but less brightly, because an emergency EVA may be necessary if docking the LM back to the capsule fails.
   * When you assign crews to a Duration mission, their members' Endurance is now highlighted along with the skills related to their roles in the crew&mdash;since Endurance now helps out on Duration missions.  They will not highlight if use of endurance is turned off or the game is set to Classic mode.  (#586)
   * Mission Control now shows a figure for requirement penalties: the total of any milestone penalties, duration penalties, and new mission penalty, if any (same as in Future Missions). This way, if say your mission has a milestone or duration penalty you can see it there, and can try downgrading it to remove the penalty. (#320)
   * In Mission Control the requirement penalties line is now color-coded green/yellow/red to match what Future Missions would show in the Milestone Risk bar (where it shows A-OK!/CAUTION!/DANGER!).
   * Clicking on the image of the pad in Mission Control will now pop up a window that breaks down those requirement penalties. (#168)
   * In the VAB/VIB, any hardware whose Safety is below Max R&D will now show in yellow, so you can go back into R&D to research it more.  It was too easy to miss that before. (#166)
   * The VAB/VIB also now shows the Safety of your EVA suits on missions that will use them, if they're below Max R&D or have been damaged (#166)
      *  If you haven't prototyped them it all, that button will now say so.
   * The last-minute launch confirmation screen now also shows hardware Safety percentages in yellow if they're below Max R&D. (#166)
   * The last-minute launch confirmation screen now shows a + to indicate when a piece of hardware holds an Avoid Fail card (#473)
   * The newscast, and the help pop-up when you press F1, now highlight the up and down arrows when there is more text to scroll up or down to. (#372)
   * The arrow keys in the Time Capsule highlight if you can go up/down
   * The arrow keys in the Library at the CIA/KGB also highlight if you can go up/down
   * The R&D building now greys out research teams you can't afford.  Also, if you are within 5 points of Max R&D, teams will be greyed out so you can't assign more teams than there are points of Safety you can gain by research (#318)
   * In the Capsule Screen, crews assigned to a mission now show in a different color. Those assigned to a mission for this turn show in blue, those for next turn in green; primary crews show in a darker shade. And next to the capsule pic it says that they're Primary or Backup crew for this turn or next. (#173)
   * When you click on Recruitment and it asks to confirm whether to recruit spacepeople, the game now shows how much money you have to help you decide whether to go ahead with it.
   * Recruitment now shows more candidate skills. In addition to Capsule and Endurance, Docking also shows at Astronaut Level 2, and at Level 1, all skills show. This can be returned to the way it used to be with the show_recruit_stats setting in Advanced Preferences, or by setting the game to Classic mode. (#176)
   * Since crews who have been in Mercury/Vostok for 6+ turns start to lose morale, the Capsule Screen now shows (OBSOLETE) after the capsule name if anyone has been there that long.
   * If you get the newscast saying you won't be able to purchase rockets for a year, or the equivalent one for capsules, Future Missions will now warn you so you don't accidentally schedule missions you'd just have to cancel next turn. (#611)
   * Future Missions now shows the Duration Level (B-F) of manned missions above level A (#251)
   * If you scheduled an (Orbit) docking mission (which uses a DM left in orbit by a previous mission) and the following turn there was no DM in orbit, the game was supposed to tell you that it was downgrading the mission to remove the docking part, but that message didn't exist (#288)
   * Any time you schedule an (Orbit) mission, Future Missions will now pop up a warning that you will need to launch a DM this turn or next. (#451)
   * Clarifications have been added to time in space in 'naut History and to tenure in Basic and Advanced Training: the number is now followed by "seasons" or "days" respectively (#300)
   * In the VAB/VIB, if you hit Autopurchase but the rocket combo won't lift the current payload, the game now warns you so you don't accidentally buy the wrong hardware. (#516)
   * In Hardware Efficiency (off the Museum), the flag in the top left has always been a hidden button linking it to the Prestige Points screen (and vice versa).  The flag is now decorated and animated as a button there so as to stand out better.
   * The Step Failure screen used to underline the crew member who failed a roll, which wasn't very self-explanatory.  Now it shows their name in parentheses next to the dice roll results (#474)
   * The Step Failure screen used to show the dice roll as Safety vs. Roll (e.g., 85 VS 92), which seemed backward. Now it shows Roll vs. Safety (e.g., 92 VS. 85).
   * At the end of a single-player game, if you clicked on STATS, it showed your opponent's name as COMPUTER# (e.g., COMPUTER2), the # representing which strategy the computer player used.  Now it gives the name of the AI player and shows the number of its (randomly) chosen strategy. (#323)
   * If any of the crew on the Moon landing is female, they now show in a different color in the History section of the endgame.
   * In the Lunar Module screen, the square at the bottom showing TOTAL LM POINTS is now a button that triggers a pop-up explaining the LM Point system (#506)
   * In recruitment, the Continue button is disabled until you've selected a full complement of 'nauts.  Now if you hit Continue before selecting enough, it also pops up a message saying so. (#462)
   * The Hospital/Infirmary now shows how much longer each patient will be there (#673)
* **KEYBOARD OPTIONS**
   * In the main menu, if you hit New Game by mistake, you can now press Esc to cancel back out to the main screen (#289)
   * You can now Esc out of the Prestige First video, rather than having to keep pressing keys while it shows the whole mission movie (#247)
   * You can now use the Escape or Enter key to cancel out of more screens, including the Administration and Museum of Space History (#723)
   * R&D and Purchasing can now be closed with the Esc key as well as Enter.
   * When you select a mission and are at the window to select the type of spacecraft, you can cancel out of there by pressing Esc.  Previously you had to hit the Cancel button, but now you can do either.
   * The Esc key now cancels you out of the Time Capsule, same as pressing PLAY. 
   * In the Time Capsule, if you hit Save game but change your mind, pressing Esc will cancel the save (#620)
   * Added Home and End key functionality to Basic and Advanced Training (#508)
   * Added Home, End, PgUp, and PgDn functionality to the Prestige Summary and Mission Records screens (#351)
   * Added Home, End, PgUp, and PgDn functionality to the Help pop-up (#468)
   * Added PgUp, PgDn, Home, and End key functionality in the recruitment screen&mdash;especially helpful when female candidates are available (#206)
   * Added Home, End, PgUp, and PgDn functionality to the Time Capsule
   * Added PgUp, PgDn, Home, and End key functionality to the CIA/KGB Library
* **LAUNCH PADS**
   * The game now distinguishes between a damaged and a destroyed pad. Previously, the Launch Facility window always showed "DESTROYED" while other windows showed it as "DAMAGED"&mdash;regardless what the game had _said_ happened to the pad. (#426)
   * It also shows a different image for damaged versus destroyed launch pads. (#466)
   * Damaged pads are always 10 or 15MB to repair. Destroyed pads cost 20 or 25MB to fix. (#412)
   * If a launch pad has been damaged or destroyed, the VAB/VIB and Future Launch Selection Screen now show not only how much that will cost, but also how much cash you currently have.
   * The mission name on the pad is now moved slightly to the left&mdash;some mission types were crowding the right margin (#270)
* **ESTHETIC IMPROVEMENTS**
   * The country insignia at the start of a player's turn is now circular; it used to look oval, kind of squashed (see #51)
   * In R&D and Purchasing, the Atlas rocket now looks more like an Atlas rocket (#556) 
   * In R&D and Purchasing, the Soyuz capsule now looks more like the real thing. 
   * The Boosted Atlas in the VAB looked short and fat, so it's been slimmed a bit, and it's now the same height as a nonboosted one (#735)
   * The LK lander now looks more like the real thing, in R&D and Purchasing, the LM Program building, the VIB, and the CIA/KGB report, though not in mission movies or the Step Failure screen (#588)
   * Images of some rockets have had small improvements, both in the VAB/VIB and the intelligence screen: in particular, the N1 looks more historical, the Saturn and Nova have better-looking fins, and the Atlas doesn't look so phallic.
   * The Saturn rocket now looks a bit better - there was no fin on the right side before, and the top looked like it curved over to the left.
   * In the VAB/VIB and the intelligence screen, the nozzles on Soviet rockets used to look a little ragged or uneven&mdash;now they look a bit better.
   * The VIB image for the Lapot shuttle was too long, causing an error message (invisible unless you ran the program from the command prompt) saying it can't fit the lunar module&mdash;and sure enough, you didn't see it on the righthand side in the VIB. That's been fixed. (#418)
   * Question marks now look a little better (see middle of #207)
   * Space History, for dummy tests, now shows an image of the Earth or Moon with the capsule, to show whether it was an Earth-orbital or lunar dummy test (#790)
   * Small updates to the newscast text (e.g., "cannot deliver any capsules or 4-person capsule/modules" was confusing)
   * The VAB (but not the VIB) showed a weird stripe up the middle of the rocket fairing, caused by transparency issues in the color palette. (#515)
   * Some hardware (the US B-Kicker, the Lapot, and Soyuz) were showing odd black spots caused by transparency issues in the color palette (#513)
   * The XMS-2 was badly off center in the VAB (#743)
   * The tops of the direct ascent rockets now look a bit better in the VAB/VIB.  The Nova doesn't look so blocky, and the central part of the UR-700 isn't so narrow.
   * In Future Missions, the cash figure is now centered under "CASH:"
   * Better centering of program names containing "I" (Gemini, Jupiter) in Astronaut Complex / Cosmonaut Center and Future Missions (#345, #361)
   * The result text in the Step Failure screen ("Catastrophic Failure", "All Systems Are Go", etc.) is now centered in its box.  It used to be left-justified.
   * The Credits are now in a different color to be easier to read against the background (#211)
   * In R&D, removed the percent sign from the RESEARCH STRONG: +1%/TEAM line, since the research news events change the dice being rolled (7-sided instead of 6-sided, etc.) rather than rolling 6-sided dice and then applying a plus or minus (#321)
* **INTERFACE IMPROVEMENTS**
   * You can now change player names after a game has begun.  In two-player games, you can change your name but not your opponent's. (#591, #705)
   * Lots of improvements to the help file: grammar corrections, readability, more clarifications, and in a few places updates to reflect changes we've made to the game.
   * In single-player games, there is now a cheat in the Purchasing screen: pressing **$** will add 100MB to your cash on hand.  This can be disabled in the config file with no_money_cheat, which is also a Classic setting. (#597)
   * In the Capsule Screen, you can now bring up the Compatibility pop-up by clicking on the astronaut's name&mdash;you don't have to click exactly on the morale box.
   * In the main endgame screen, the buttons at the bottom now highlight the letter that activates them.
   * When Mission Control recommends you scrub a mission, if you hit the button to do so, you're now prompted for confirmation so you won't cancel it accidentally, plus it shows more prominently so you're less likely to miss it. (#475)
      * For the rest of the mission, the hardware will suffer a temporary Safety penalty.  So this screen now mentions that fact above the button for scrubbing it. (#589)
   * You can now cancel out of astronaut/cosmonaut recruitment if you change your mind (#295)
   * Player now has more mission downgrade options&mdash;in particular, it is now possible to downgrade a Lunar Landing to a Lunar Orbital LM Test
   * Added a context-sensitive help item for the Launch Confirmation screen (#786)
   * In Mission Control, your downgrade options now include an option to scrub the flight.
   * In Future Missions, when you select a capsule program with no (or insufficient) crews to assign to the mission, it now takes you back to the main Future Missions screen rather than taking you on to the (empty) Select Primary Crew window. (#509)
   * The opening screen now defaults to the Historical Model rather than the Basic Model (#221)
   * In Preferences, the movies/stills option has been disabled&mdash;missions now always show movies (#254)
   * When you assign crews to a mission, if you don't have enough available crews, the game still popped up a message saying you don't have enough crews to assign a primary *and a backup*.  The old game used to require a backup crew, and the message reflected that.  Now it will still show that message if you set the game to require backups, but if not it will say just that you don't have any crews available. (#381)
   * Credits now shows a screen for those who've worked on RIS, in addition to those who worked on the original BARIS (#263)
   * Credits also now show names for people who were mentioned in a text file on the CD version but didn't actually make it into the credits for that version.
* When the AI wins the game, you now see its Moon landing play out like other prestige firsts.
* Increased height of Prestige First window so one more prestige first can be listed&mdash;which also better centers the window on the screen.
* Saved games are now stored on disk with the name you give them, rather than as BUZZ1.SAV, BUZZ2.SAV, etc.
* The security of saved games has been significantly improved. (#655)
* Before, using a capsule type on a mission got you only +1 Safety even if you did a mission that flies two capsules such as the MOL.  Now, if you fly a mission that uses two capsules of the same type, it can get +2 Safety.  This applies mainly to Joint Orbitals and MOL missions. (#328)
* The Service category has been added to the 'naut roster after all these years, so astronaut candidates now show what branch of the military, if any, they're serving in (#165)
* Race has been added to the 'naut roster, so black astronauts and Asian cosmonauts no longer show white faces (#293)
* To keep track of how much time each person has spent in space, the game used to automatically assign the maximum for each flight's duration level (A=2 days, B=5, C=7, D=12, E=16, F=20), rather than choosing a number of days within each range.  As a result, your fellow on the suborbital got credit for two days in space for a 15-minute flight.  Now it assigns a number depending on the type of mission flown (or a random number of days in the range, for E & F). (#300)
* The game now makes some use of the Endurance skill (#384)
   * It contributes to the Duration steps of Duration flights
   * When the newscast randomly selects a spaceperson to be injured/retired, the higher their Endurance the more likely they are to go to be injured rather than retire altogether.  
   * Also, if injured, those with higher EN are more likely to go for 1 turn instead of 2: 25% chance per point above 1, and EN0 has a 25% chance of going for 3 turns (#384)
* There's now an option in Advanced Preferences (female_nauts) that will let you recruit women without having to receive the newscast which requires you to recruit them (#329)
* Another option in Advanced Preferences (game_style) is the Classic setting, which makes the game behave more like the original BARIS: you're required to assign backup crews, etc.
* Small updates to the 'naut roster (removal of duplicates, etc.)
* The Saturn V has been renamed simply "Saturn"&mdash;to include other rockets in its family, such as the Saturn IB (#216)
* The Soviet player's default name has been changed from KOROLOV to KOROLYOV, which is a closer transliteration (#262)
* People in Advanced Training now don't suffer the usual 4-point/turn loss of morale, since they're presumably being groomed for something special (#279) 
