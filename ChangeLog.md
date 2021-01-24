# Race into Space changes

## Version 2.0

### BACK-END CHANGES

- Game brought over from Sourceforge to GitHub
- Versioning system converted from CVS to GIT
- Code converted from C to C++
- Code cleanup to make the game easier to modify
- Many of the data files and images have been changed to use discrete image files rather than being 
  embedded into the code
- Vestigial copy protection code finally removed (#493)

Since transitioning to C++ on GIT, the game has been brought up to where we left off
 (that is, bugs introduced by the transition have been fixed).
Below are improvements that have been made over what the game offered in v.1.1.

### BUG FIXES

- The duration penalty system has been fixed, and is working right for the first time ever (#246)
   - This makes a final solution to what has sometimes been called the "Duration B Bug"
- The newscast that announces all manned missions will be canceled this turn no longer cancels unmanned 
  missions as well (#164)
- 'Nauts killed in a newscast (train accident, etc.) are now removed immediately (#240)
   - Before, they wouldn't be removed until the start of the following turn, making it possible for 
     the dead crew member to fly on a mission<br/><br/>
- The newscast announcing that a storm front has blocked a launch window now specifies which pad (#169)
   - Before, it had called it "the H launch facility"
- When a component faces a Safety penalty unless the player pays to fix it, there was sometimes a 
  mismatch between newscast and actual numbers (#204, 243)<br/>
- When a component faces a Safety penalty unless the player pays to fix it, it didn't make sure you had 
  enough cash to pay, allowing you to have a negative number of MBs (#268) [not sure if this was in 1.1]<br/>
- The second of the two newscasts announcing that the primary crew caught cold and will be replaced 
  with the backup crew never worked; now it does (#286)<br/>
- The newscast stating that an astronaut left to become a test pilot or has been transferred for 
  reeducation instead sent them to the hospital/infirmary (#290)<br/>
- The LM failure "Engine explodes upon ignition. Spacecraft crew is killed." used to kill all three 
  crew members instead of just the Commander and LM Pilot (#476)<br/>
- Because of the nature of the variable for 'naut mood, after a successful mission it could roll over
  above 100, which would be converted to a very low number, potentially leading to two entire flight 
  crews resigning from sudden bitterness during their moment of triumph (#484)<br/>
- The newscast that delayed a mission on the pad used to delay missions that couldn't have been 
  delayed a turn (such as Mars/Jupiter/Saturn flybys).<br/>
- There were three newscasts which were supposed to reduce your budget by 20, 25, or 30 MBs, or by 
  20%, 25%, or 30% of your budget, whichever is less.  Instead, if 20/25/30% was less than 20/25/30 MB,
  it reduced your budget TO 20/25/30% of what it had been, which caused your budget to drop like a rock.
  (#394)<br/>
- There was also a newscast that was supposed to increase your budget by 20MB, but the conditions under 
  which it was allowed to happen effectively prevented it ever happening (#397)<br/>
- In the Capsule Screen, if you tried to break an empty crew, it would tell you the crew was assigned 
  as the backup crew for a current mission (#348)<br/>
- Jupiter flybys were set to take 7 turns to arrive, the same as Saturn - now they're set to 4 (#171)
- Removed visual artifacts next to astronaut wings and medals in Astronaut/Cosmonaut History 
  (#299, from #192)<br/>
- Better centering of astronaut wings in Astro/Cosmonaut history for anyone who's flown in space.<br/>
- Fixed a bug with technology transfer for the Titan/Proton rocket: the technology bonus from multiple 
  rockets at 75%+ was not being applied (#310)<br/>
- Fixed a bug in Future Missions: if you chose a mission and proceeded to where you select a capsule 
  or crew, then cancel - then press Continue again (either after choosing a different mission or 
  deciding to go ahead with the same one), when you select capsule or crew, when you go back to 
  Future Missions you find the mission canceled (#222)<br/>
- Fixed a bug in the VAB/VIB: with some payloads, the color of the rocket/booster combination wasn't 
  changing to reflect whether it could lift the currently selected payload (#298)<br/>
   - This was noticed particularly with an XMS-2 Lunar Pass using B/Titan and a B-Kicker.  When you 
     cycled to B/TITAN, the Current Payload displayed in dark red to show that a boosted Titan 
     couldn't lift that combo, but "B/TITAN" under Rocket still showed in white, sometimes leading 
     the player to purchase hardware for a mission he couldn't fly.<br/>
- The VAB/VIB previously wasn't properly reporting damage to rockets (where you're required to pay xMB 
  or suffer y% Safety loss on its next mission).<br/>
- Fixed an array overflow error that used to occasionally crash toward the end of the game (#40, #200)<br/>
- When you're flying docking tests, the game used to get stuck sometimes at 2 DMs. For the rest of 
  the game you always had two docking modules and didn't have to buy them again (#319) [not sure if this was in 1.1]<br/>
- An "A-OK" failure on an LEM Thrust Test (that is, one that let the test continue successfully) 
  used to not award you LM Point(s) for the test (#441)<br/>
- Certain mission launch steps involving a minishuttle defaulted to the capsule failure cases.<br/>
- The game used to allow you to assign Duration missions with an illegally low duration (#257, #278)<br/>
- If you scheduled an (Orbit) docking mission (which uses a DM left in orbit by a previous mission) 
  and the following turn there was no DM in orbit, the game was supposed to tell you that it was 
  downgrading the mission to remove the docking part, but that message didn't exist (#288)<br/>
- Scrubbing a mission in the launch pad screen failed to free up any hardware you had purchased for 
  it (292)<br/>
- When the capsule's emergency-eject system has to be used, the mission would often continue right 
  after, or say it was used successfully but all the crew shows as dead (#244)<br/>
- The image for a damaged/destroyed Soviet launch pad instead showed a functioning pad with a rocket
  ready for launch. It now shows a damaged pad. The damaged-pad icon for the US wasn't too obviously 
  damaged either, so now it's more so.<br/>
- BARIS allowed you to use PgUp and PgDn to scroll through the newscast more efficiently, both at the 
  start of your turn and later in the Viewing Stand.  That functionality was lost in RIS because DOS 
  has a peculiar way of processing those keys, but it's now been restored - along with the Home and 
  End keys.<br/>
- Resolved excessive astronaut/cosmonaut retirement checks to match original intent (#428)
- Resolved inconsistencies between the code and the comments when the game checks to see if people
  should retire.<br/>
- In 'naut recruitment, if you clicked and held on the up or down arrow, it would take 3 or 4 seconds 
  before the list would start to scroll.<br/>
- In Preferences, the Human/AI button nestled into the world map never looked quite right on the 
  Soviet side.  That's been fixed. (#353)<br/>
 
### GAME CHANGES

- Since the duration penalty system now works properly, we've made it possible to downgrade to a 
  lower Duration level in Mission Control (#255)<br/>
   - e.g., if you have a Duration C ready to go and schedule a Dur D mission for next turn, but 
     something goes wrong and the flight doesn't accomplish Duration C, next turn you can downgrade 
     the upcoming D mission to Dur C to avoid the -5 penalty for skipping a level.<br/>
- We have added back Play By Email.  It worked in the CD version of BARIS, but was disabled in RIS. 
  (#131) [However, it has been disabled by default.]<br/>
- If you have no missions planned for next turn when you hit the Flagpole, you now receive a warning 
  about it. (#166)<br/>
- The VAB/VIB now has a Delay button to let you put a launch back a turn rather than having to scrub 
  it and reschedule. (#167)<br/>
- In R&D and Purchasing, clicking on the image of the hardware item you're currently on will now pop 
  up a help message giving information about the item (#467)<br/>
- In Future Missions, the part of the screen that shows the safety risk is now a button that will 
  bring up a popup that breaks it down between milestone penalties, duration penalties, and new 
  mission penalties.  It also shows penalties on the LM steps of lunar landings.  (#168, #373)<br/>
- In Future Missions, the name box is now also a button that brings up a description of the currently
  selected mission (#492)<br/>
- Mission Control also now includes a penalty pop-up, activated by clicking on the image of the pad
  on the left. (#168)<br/><br/>
- When the computer player wins the game, you now see its Moon landing play out like other prestige 
  firsts.<br/>
- Increased height of Prestige First window so one more prestige first can be listed - which also 
  better centers the window on the screen<br/>
- The game now distinguishes between a damaged and a destroyed pad. Previously, the Launch Facility 
  window showed "DESTROYED" while other windows showed them as "DAMAGED". (#426)<br/>
- It also shows a different image for damaged vs. destroyed launch pads. (#466)<br/>
- The newscast, and the help pop-up when you press F1, now highlight the up and down arrows when 
  there is more text to scroll up or down to. (#372)<br/>
- The R&D building now greys out research teams you can't afford.  Also, if you are within 5 points 
  of Max R&D, teams will be greyed out so you can't assign more teams than there are points of Safety 
  you can gain by research (#318)<br/>
- In the Capsule Screen, crews assigned to a mission now show in a different color. Those assigned 
  to a mission for this turn show in blue, those for next turn in green; primary crews show in a 
  darker shade. And next to the capsule pic it shows that they're Primary or Backup crew for this 
  turn or next. (#173)<br/>
- In the Capsule Screen, you can now bring up the Compatibility pop-up by clicking on the astronaut's
  name: you don't have to click exactly on the morale box.<br/>
- To keep track of how much time each person has spent in space, the game used to automatically 
  assign the maximum for each flight's duration level (A=2 days, B=5, C=7, D=12, E=16, F=20), rather 
  than choosing a number of days within each range.  As a result, your fellow on the suborbital (a 15-
  minute flight!) showed as having spent two days in space.  Now it assigns a number of days depending
  on the type of mission flown (or just random days in the range, for E & F). (#300)<br/>
- The game now makes some use of the Endurance skill: it contributes to the Duration steps of Duration
  flights<br/>
- When Mission Control recommends a mission scrub, if you hit the button to do so, you're now prompted 
  for confirmation so you won't cancel it accidentally (#475)<br/>
- You can now cancel out of recruitment if you change your mind (#295)<br/>
- There's now an option in Advanced Preferences that will let you recruit female 'nauts without having 
  to receive the newscast which requires you to recruit them (#329)<br/>
- The Max R&D figures for Mercury, Gemini, and Apollo have been reset to their levels in the floppy 
  version (Apollo was different only in the Basic Model) (#216)<br/>
- Player now has more mission downgrade options - in particular:<br/>
   - It is now possible to downgrade a Lunar Landing to a Lunar Orbital LM Test<br/>
- Future Missions now shows the Duration Level (B-F) of manned missions above level A (#251)<br/>
- Mission Control now shows a figure for REQUIREMENT PENALTIES: the total of any milestone penalties, 
  duration penalties, and new mission penalty, if any. This way, if say your mission has a milestone 
  or duration penalty you can see that, and can see if downgrading the mission will remove the 
  penalty. (#320)<br/>
- In Mission Control, mission downgrade options now include an option to scrub the flight<br/>
- The opening screen now defaults to the Historical Model rather than the Basic Model (#221)<br/>
- The country insignia at the start of a player's turn is now circular; it used to look oval, kind 
  of squashed (see #51)<br/>
- Question marks now look a little better (see middle of #207)<br/>
- Save games now save under file names that reflect the name you assign to the save (#438)<br/>
- In R&D, removed the percent sign from the RESEARCH STRONG: +1%/TEAM line, since the research news 
  events change the dice being rolled (7-sided instead of 6-sided, etc.) rather than rolling 6-sided 
  dice and then applying a plus or minus (#321)<br/>
- Small updates to the 'naut roster (removal of duplicates, etc.)<br/>
- Small updates to the endgame text to improve justification on the righthand side<br/>
- Small updates to the newscast text (e.g., "cannot deliver any capsules or 4-person capsule/modules" 
  was confusing)<br/>
- The Soviet newscast now plays the Soviet newscast music (#214)<br/>
   - Starting with the CD version of BARIS, and in RIS through v.1.1, it played the US newscast music
   - The Soviet music improves the timing so that the movie's video better matches the voice of the 
     newscaster<br/>
- Clarifications have been added to space tenure in 'naut History and to tenure in Basic and Advanced 
  Training: the number is now followed by "seasons" or "days" respectively (#300)<br/>
- The Saturn V has been renamed simply "Saturn" - to include other rockets in the Saturn family, such 
  as the Saturn IB.<br/>
- Images of some rockets have had small improvements, both in the VAB/VIB and the intelligence screen.<br/>
- In Future Missions, the cash figure is now centered under "CASH:"<br/>
- Better centering of program names containing "I" (Gemini, Jupiter) in Astronaut Complex / Cosmonaut 
  Center and Future Missions (#345, #361)<br/>
- Added PgUp, PgDn, Home, and End key functionality in the recruitment screen - especially helpful when
  female candidates are available (#206)<br/>
- In recruitment, the Continue button is disabled until you've selected a full complement of 'nauts.
  Now it also pops up a message telling you you haven't selected enough. (#462)<br/>
- The number of spots to fill in recruitment now shows immediately instead of after you've selected 
  one to recruit.<br/>
- Added Home and End key functionality to Basic and Advanced Training (#508)<br/>
- Added Home, End, PgUp, and PgDn functionality to the Prestige Summary and Mission Records screens 
  (#351)<br/>
- In Hardware Efficiency (off the Museum), the flag in the top left has always been a hidden button 
  linking it to the Prestige Points screen (and vice versa).  The flag is now decorated and animated 
  as a button in those screens so it's no longer hidden.<br/>
- In Preferences, the custom roster and movies/stills options have been disabled (#254)<br/>
- The Soviet player's default name has been changed from KOROLOV to KOROLYOV.<br/>
- Improvements to the help file: grammar corrections, readability, more needed clarifications, and in 
  a few places updates to reflect additions we've made to the game.<br/>
- When you assign crews to a mission, if you don't have any available crews, the game still popped up 
  a message telling you you don't have enough crews to assign a primary and a backup.  The old game 
  used to require a backup crew, and the message reflected that.  Now it will still show that message 
  if you set the game to require backups, but if not you will now be told you don't have any crews 
  available, instead. (#381)<br/>
- Credits now shows a screen for those who've worked on RIS, as opposed to original BARIS (#263)<br/>
- All the Credits now in a different color to be easier to read against the background (#211)<br/>
- Added names for people who were mentioned in a text file on the CD version but didn't actually 
  make it into the credits for that version<br/>
- The result text in the Step Failure screen ("Catastrophic Failure", "All Systems Are Go", etc.) is 
  now centered in its box.  It used to be left-justified.<br/>
- When playing against the AI, at the end of the game, if you clicked on STATS, it showed your 
  opponent's name as COMPUTER# (e.g., COMPUTER2), the # representing which strategy the computer 
  player used.  Now it gives the name of the AI player and tells the strategy number. (#323)<br/>
- If any of the crew who completes the Moon landing is female, they now show in a different color 
  in the History section of the endgame.<br/>
