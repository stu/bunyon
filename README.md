# Bunyon v0.5
by Stu George

About
=====
Bunyon is an interpreter designed to run Scott Adams games
as well as homebrew games that are written for the TI99/4
computer.

The TI99/4 datafiles for Scott Adams games is a binary format
not compatible with the more common ScottFree/TRS-80 format.

Games
=====
This interpreter can play the first known 12 games to completion
	Adventureland
	Pirate Isle
	Mission Impossible
	Voodoo Castle
	The Count
	Strange Odyssey
	Mystery Fun House
	Pyramid of Doom
	Ghost Town
	Savage Island part 1
	Savage Island Part 2
	Golden Voyage

Currently, only the unix console version has all the options enabled,
so you cant replay the scripts or generate the game source with the
win32 version yet.

The final 4 games (Hulk, Spiderman, Buckaroo Banzai and Sorcerer of
Claymorgue Castle) were purported to have been written for the TI99,
but I have never seen or found them.

If you have copies of these, please contact me.

Aside from Scott Adams authored games, there was an editor that could
produce games in the same format, The first version was the Weiand Adventure
Editor from Germany, which was later relicensed as the Tex-Comp Adventure
Editor in America.

Of these games, there is a version of Colossal Cave, Knight Ironheart,
Searching for the Loch Ness monster etc.


Decompiling
===========
The decompiler outputs source compatible with ScottCom, and I have
decompiled, recompiled and tested against the scripts and achieved
the same result with;

* Adventureland
* Mission Impossible

Does not work with select_rv commands (adventure 6), as ScottCom
does not support that command yet, nor with Pirate Adventure as
ScottCom does not allow numbers as nouns/verbs.

Known Bugs
==========
Does not handle unknown opcode F1..


Scott's Bugs
============
Just some things I have noticed while playing and testing;

Adventureland
* You do not need have the blue ox when you say bunyon in the quicksand,
  it will still leave with the axe.
* "take sign" - Results in a message even if no sign is present.
* The fish can escape back to the lake even if your not in that room...
* Eat is a synonym for Drink, so you can  'Drink Fruit' and 'Drink Honey'.

Pirate Adventure
* Has strange command "go kqf", which is synonymous with walk 30 paces.

Savage Island I
* look watch - I've been here -1 moves. (do it again says 0 moves....)
  (This is a result of the system setting all variables to -1 to start...)


Interesting Tid Bits
====================
The original games did not use the Adventure Editor [Wieand Adventure Editor/
Licensed in USA as Tex-Comp Adventure Editor]

However, when Scott Adams was given a copy of the Editor, he did port
his four newest games at the time: Spiderman, Buckaroo Banzi, Hulk
and Claymorgue Castle, to the TI.  His comment was that it made writing
games for the TI quite a bit easier than the way he was doing it before
he had the Adventure Editor.


To Do List
==========
Figure out opcode F1... Only used once in adventure 08...

	act auto 100
	{
		set (flag_16);
		exists (itm_009);	# Lit flashlite
		swap itm_009, itm_008;	# Lit flashlite, Unlit flashlite
		--0xF1--;
	}

Ghost Town also reports incorrect number of moves it takes to complete
the game, which is frustrating to figure out.


Scripts
=======
Random seed of 1
- Adventureland
- Pirate Isle
- Mission Impossible
- Voodoo Castle
- The Count
- Strange Odyssey
- Mystery Fun House
- Ghost Town
- Golden Voyage
- Savage Island Part 2

Random Seed of 3
- Pyramid of Doom
- Savage Island part 1


Building
========
Bunyon is built using rant, a ruby build tool. So in order to build the
source you will need a copy of ruby and rant (the easiest way to install
rant is via rubygems).

You will also need a version of GLK.

currently the unix version compiles against glkterm
and the win32 version uses winglk-1.26

See config.h for build options.


Thanks
======
Big thanks to Kevin Venzke for helping find and report bugs and
to David Schweinsberg.


Revision History
================
v0.5 - 20220923
	* hey its old code I need to stick on github!
	* quick cleanup of typedefs
	* quick change to simple Makefile pulling pkg-config for garglk
	* GPL it!

v0.4 - 20060603
	* Decompile now puts semicolon on eq(); codes
	* No more need to define if system does not have strupr/lower
	* Changed struct packing for gcc >= 4.
	* Fixed divide by zero when typing score and there are no treasures

v0.3 - 20051202
    * Fixed big endian compile error [reported David Schweinsberg]
    * Fixed Rantfile for building on unix + win32
    * Fixed missing 'Cant do that' messages. [reported by Kevin Venzke]
    * Light_Time was incorrect on decompiled games
    * Lots of fixes to the decompiler
    * Split window to look like original Scott Adams drivers
    * Fixed "nothing" message display when no items in room. [reported Kevin Venzke]
    * Replaced 'your' and 'you' with 'I' to be consistent.
    * Fixed 'le' to be <= instead of <, tested against real TI99/4A driver
    * Fixed printing of \ as space.
    * Now prints OK when you pickup something.
    * WinGLK version now uses same colours as TI driver (except success/death)
    * Changes to some game text to match TI driver

v0.2 - 20051030
    + Fixed "ANY" word (word 0). Fixes non-scott games that have noun 0
      something else.
    + Upgraded win32 to WinGLK 1.26
    + Fixed VS6 workspace release and debug
    + Removed unix_glkterm dependancy in win32 version
    + Fixed inventory count from <= to <...
    + Fixed "get" opcode. Action fails if "get" fails.
    + Added some internal debug commands
    + Fixed way "try" opcode nesting works
    + Fixed initial light count
    + Fixed light out
    + Fixed off by 1 on items for save/load game state

v0.1 - 20050823
    + Initial release# Bunyon v0.5
by Stu George

About
=====
Bunyon is an interpreter designed to run Scott Adams games
as well as homebrew games that are written for the TI99/4
computer.

The TI99/4 datafiles for Scott Adams games is a binary format
not compatible with the more common ScottFree/TRS-80 format.

Games
=====
This interpreter can play the first known 12 games to completion
	Adventureland
	Pirate Isle
	Mission Impossible
	Voodoo Castle
	The Count
	Strange Odyssey
	Mystery Fun House
	Pyramid of Doom
	Ghost Town
	Savage Island part 1
	Savage Island Part 2
	Golden Voyage

Currently, only the unix console version has all the options enabled,
so you cant replay the scripts or generate the game source with the
win32 version yet.

The final 4 games (Hulk, Spiderman, Buckaroo Banzai and Sorcerer of
Claymorgue Castle) were purported to have been written for the TI99,
but I have never seen or found them.

If you have copies of these, please contact me.

Aside from Scott Adams authored games, there was an editor that could
produce games in the same format, The first version was the Weiand Adventure
Editor from Germany, which was later relicensed as the Tex-Comp Adventure
Editor in America.

Of these games, there is a version of Colossal Cave, Knight Ironheart,
Searching for the Loch Ness monster etc.


Decompiling
===========
The decompiler outputs source compatible with ScottCom, and I have
decompiled, recompiled and tested against the scripts and achieved
the same result with;

* Adventureland
* Mission Impossible

Does not work with select_rv commands (adventure 6), as ScottCom
does not support that command yet, nor with Pirate Adventure as
ScottCom does not allow numbers as nouns/verbs.

Known Bugs
==========
Does not handle unknown opcode F1..


Scott's Bugs
============
Just some things I have noticed while playing and testing;

Adventureland
* You do not need have the blue ox when you say bunyon in the quicksand,
  it will still leave with the axe.
* "take sign" - Results in a message even if no sign is present.
* The fish can escape back to the lake even if your not in that room...
* Eat is a synonym for Drink, so you can  'Drink Fruit' and 'Drink Honey'.

Pirate Adventure
* Has strange command "go kqf", which is synonymous with walk 30 paces.

Savage Island I
* look watch - I've been here -1 moves. (do it again says 0 moves....)
  (This is a result of the system setting all variables to -1 to start...)


Interesting Tid Bits
====================
The original games did not use the Adventure Editor [Wieand Adventure Editor/
Licensed in USA as Tex-Comp Adventure Editor]

However, when Scott Adams was given a copy of the Editor, he did port
his four newest games at the time: Spiderman, Buckaroo Banzi, Hulk
and Claymorgue Castle, to the TI.  His comment was that it made writing
games for the TI quite a bit easier than the way he was doing it before
he had the Adventure Editor.


To Do List
==========
Figure out opcode F1... Only used once in adventure 08...

	act auto 100
	{
		set (flag_16);
		exists (itm_009);	# Lit flashlite
		swap itm_009, itm_008;	# Lit flashlite, Unlit flashlite
		--0xF1--;
	}

Ghost Town also reports incorrect number of moves it takes to complete
the game, which is frustrating to figure out.


Scripts
=======
Random seed of 1
- Adventureland
- Pirate Isle
- Mission Impossible
- Voodoo Castle
- The Count
- Strange Odyssey
- Mystery Fun House
- Ghost Town
- Golden Voyage
- Savage Island Part 2

Random Seed of 3
- Pyramid of Doom
- Savage Island part 1


Building
========
Bunyon is built using rant, a ruby build tool. So in order to build the
source you will need a copy of ruby and rant (the easiest way to install
rant is via rubygems).

You will also need a version of GLK.

currently the unix version compiles against glkterm
and the win32 version uses winglk-1.26

See config.h for build options.


Thanks
======
Big thanks to Kevin Venzke for helping find and report bugs and
to David Schweinsberg.


Revision History
================
v0.5 - 20220923
- hey its old code I need to stick on github!
- quick cleanup of typedefs
- quick change to simple Makefile pulling pkg-config for garglk
- GPL it!

v0.4 - 20060603
- Decompile now puts semicolon on eq(); codes
- No more need to define if system does not have strupr/lower
- Changed struct packing for gcc >= 4.
- Fixed divide by zero when typing score and there are no treasures

v0.3 - 20051202
- Fixed big endian compile error [reported David Schweinsberg]
- Fixed Rantfile for building on unix + win32
- Fixed missing 'Cant do that' messages. [reported by Kevin Venzke]
- Light_Time was incorrect on decompiled games
- Lots of fixes to the decompiler
- Split window to look like original Scott Adams drivers
- Fixed "nothing" message display when no items in room. [reported Kevin Venzke]
- Replaced 'your' and 'you' with 'I' to be consistent.
- Fixed 'le' to be <= instead of <, tested against real TI99/4A driver
- Fixed printing of \ as space.
- Now prints OK when you pickup something.
- WinGLK version now uses same colours as TI driver (except success/death)
- Changes to some game text to match TI driver

v0.2 - 20051030
- Fixed "ANY" word (word 0). Fixes non-scott games that have noun 0 something else.
- Upgraded win32 to WinGLK 1.26
- Fixed VS6 workspace release and debug
- Removed unix_glkterm dependancy in win32 version
- Fixed inventory count from <= to <...
- Fixed "get" opcode. Action fails if "get" fails.
- Added some internal debug commands
- Fixed way "try" opcode nesting works
- Fixed initial light count
- Fixed light out
- Fixed off by 1 on items for save/load game state

v0.1 - 20050823
- Initial release

