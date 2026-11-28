# Summary

The ids and flag values the API uses (areas, skills, item types, UI flags, out-of-game locations, collision flags, Box opacity, text colors and fonts) are listed in the [Reference tables](#reference-tables) at the end.

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`Unit`](#classUnit)` `[`getUnit`](#group__globalFunctions_1ga6073f506327123ebf40219400e4d3f70)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` type,String name,uint32_t mode,uint32_t nUnitId)`            | Get a unit by type, name, mode and nUnitId.
`public `[`Unit`](#classUnit)` `[`getUnit`](#group__globalFunctions_1ga22c01b5a8480e8fbf540e43bad5ae2f8)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` type,uint32_t classId,uint32_t mode,uint32_t nUnitId)`            | Get a unit by type, classId, mode and nUnitId.
`public object[] `[`getPath`](#group__globalFunctions_1ga7913f6338480cc180c015198b6c18024)`(uint32_t Area,uint32_t srcX,uint32_t srcY,uint32_t dstX,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` dstY,uint32_t reductionType,uint32_t Radius)`            | Creates a path (walking or teleporting) from the source to the destination.
`public object[] `[`getPath`](#group__globalFunctions_1ga051897c1e76c0ceeccd946dbedc6ea54)`(uint32_t Area,uint32_t srcX,uint32_t srcY,uint32_t dstX,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` dstY,uint32_t reductionType,uint32_t Radius,bool(*)(int32_t x, int32_t y) reject,bool(*)(object curPt, `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac) i, object[] pts)` reduce,int32_t[](*)(int32_t x, int32_t y) mutate)`            | Creates a path (walking or teleporting) from the source to the destination.
`public unsigned short `[`getCollision`](#group__globalFunctions_1gac3a89f2d8c4940ae49dce43fbe2f626b)`(uint32_t nLevelId,int32_t nX,int32_t nY)`            | Get the collision flags at a given point in a given area.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getMercHP`](#group__globalFunctions_1gac4217ed30da8eb07d8addca4cdb51a9d)`()`            | Get the health of the controlled unit's merc, as a percentage.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getCursorType`](#group__globalFunctions_1gaa0bc4f7877d6992600202b8dce442787)`()`            | Get the state of the in-game hand cursor.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getCursorType`](#group__globalFunctions_1ga9b3530843295a8ee8fa59a3130ae7409)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nType)`            | Get the hand-cursor state, or the item-panel mode when nType is 1.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getSkillByName`](#group__globalFunctions_1gaee860cd7482aff8e89519314a0f86955)`(String skillName)`            | Get skill ID by name.
`public String `[`getSkillById`](#group__globalFunctions_1ga14cda78dea3cf554c8ebeeb0834039c9)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` skillId)`            | Get skill name by ID.
`public String `[`getLocaleString`](#group__globalFunctions_1gaf2065d798cd6a0337fb94f7bae718283)`(uint16_t localeId)`            | Get the String in the current locale that corresponds to the given id.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getTextSize`](#group__globalFunctions_1ga19d918388535584b388c00c57d76ab47)`(String string,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` font,bool asObject)`            | Get the width and height of the given text in the given font.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getThreadPriority`](#group__globalFunctions_1ga6ab5869695c97ce6e4fbc6899e16a75d)`()`            | Get the priority of the current thread.
`public bool `[`getUIFlag`](#group__globalFunctions_1gac259c30bc648650e021f45f1c020e412)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nUIId)`            | Get whether or not a UI flag is set.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getTradeInfo`](#group__globalFunctions_1gaedebe009d3f6c0afeab5cb544f430767)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode)`            | Get the current player-trade state.
`public bool `[`getWaypoint`](#group__globalFunctions_1ga4561d06e77835b04e0794758844bd987)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nWaypointId)`            | Get whether the controlled unit has the given waypoint.
`public `[`D2BSScript`](#classD2BSScript)` `[`getScript`](#group__globalFunctions_1ga8340268c49de1e5f34588c01b0cf971d)`(bool currentScript)`            | Get the current or first context.
`public `[`D2BSScript`](#classD2BSScript)` `[`getScript`](#group__globalFunctions_1ga02ad0d69faff60c7061a98a3e8a2b53b)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` threadId)`            | Get a script by thread id.
`public `[`D2BSScript`](#classD2BSScript)` `[`getScript`](#group__globalFunctions_1gac202e144c149fa59378fd01585e6a2bd)`(String name)`            | Get a script by filename.
`public `[`D2BSScript`](#classD2BSScript)` `[`getScript`](#group__globalFunctions_1ga822bdc4cbce00ca342a05bc29ffae76e)`()`            | Get the first script.
`public `[`Room`](#classRoom)` `[`getRoom`](#group__globalFunctions_1ga0ec5e4d9e09bf924c1f96ce417290417)`(uint32_t levelId)`            | Get the first room in area given by level id.
`public `[`Room`](#classRoom)` `[`getRoom`](#group__globalFunctions_1ga6c10567ae9eac427ef0770edb2ab8c21)`(uint32_t levelId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y)`            | Get the room that the given point is in from the given level id.
`public `[`Room`](#classRoom)` `[`getRoom`](#group__globalFunctions_1gacc763e01907b73ca5c405469cc58cf12)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y)`            | Get the room that the given point is in.
`public `[`Room`](#classRoom)` `[`getRoom`](#group__globalFunctions_1gafc051829ad79986ed98043fd46bd9a2c)`()`            | Get the first room in the current area.
`public `[`Party`](#classParty)` `[`getParty`](#group__globalFunctions_1gae1a5223caa888f3cc2429fad719fc7eb)`()`            | Get the first party.
`public `[`Party`](#classParty)` `[`getParty`](#group__globalFunctions_1ga6a0dc4f8e9bd582b1c8c4fc4da5e2f47)`(String name)`            | Get the party line for the player with the given name.
`public `[`Party`](#classParty)` `[`getParty`](#group__globalFunctions_1ga3fa567c623c83fe382918a7c6fb49978)`(uint32_t playerId)`            | Get the party line for the player with the given player id.
`public `[`Party`](#classParty)` `[`getParty`](#group__globalFunctions_1gab98faccffab8b30337ba357504c145c3)`(`[`Unit`](#classUnit)` player)`            | Get the party line that corresponds to the given unit.
`public `[`PresetUnit`](#classPresetUnit)` `[`getPresetUnit`](#group__globalFunctions_1ga6bf8c5a903fcf00dcd2504e8f4dc4ff7)`(uint32_t levelId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nType,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nClassId)`            | Get the first [PresetUnit](#classPresetUnit) of the given type and class id.
`public `[`PresetUnit`](#classPresetUnit)` `[`getPresetUnits`](#group__globalFunctions_1gaf77d2fb8bcab572d328c5764155be78d)`(uint32_t levelId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nType,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nClassId)`            | Get an array of [PresetUnit](#classPresetUnit) s of the given type and class id.
`public `[`Area`](#classArea)` `[`getArea`](#group__globalFunctions_1ga096676595eb6e9e2361dffe28acd8397)`()`            | Get the [Area](#classArea) where the controlled unit currently resides.
`public `[`Area`](#classArea)` `[`getArea`](#group__globalFunctions_1gae59e910620c20fb4fae45fc06d7cc351)`(int32_t nArea)`            | Get the [Area](#classArea) for area ID nArea.
`public String `[`getBaseStat`](#group__globalFunctions_1gab4ebba6bce777cb9705e199af1d08bc8)`(String szTableName,int32_t nClassId,String szStatName)`            | Get the base stat from the given table with the given class ID and stat name.
`public String `[`getBaseStat`](#group__globalFunctions_1ga79338ba3a14fe002f38d987fed2809b3)`(int32_t nBaseStat,int32_t nClassId,String szStatName)`            | Get the base stat from the given table with the given class ID and stat name.
`public String `[`getBaseStat`](#group__globalFunctions_1gac86c8437c691e21b5593552ab9def9ba)`(String szTableName,int32_t nClassId,int32_t nStat)`            | Get the base stat from the given table with the given class ID and stat ID.
`public String `[`getBaseStat`](#group__globalFunctions_1gafcfdd2f8d3eb3a2c30436ddfc4add78b)`(int32_t nBaseStat,int32_t nClassId,int32_t nStat)`            | Get the base stat from the given table with the given class ID and stat ID.
`public `[`Control`](#classControl)` `[`getControl`](#group__globalFunctions_1gaad0e54fb02bc1ce440b96204167da80f)`(int32_t nType,int32_t nX,int32_t nY,int32_t nXSize,int32_t nYSize)`            | Get the control specified by type, location and size.
`public bool `[`getPlayerFlag`](#group__globalFunctions_1gaa18d78f3e5bb312f53b602247ec66fd3)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nFirstUnitId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSecondUnitId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nFlag)`            | Get the relation between two players, anywhere in the game.
`public double `[`getTickCount`](#group__globalFunctions_1gad6eff12ded45cc505954f4763a71eae3)`()`            | Get the tick count.
`public `[`Unit`](#classUnit)` `[`getInteractedNPC`](#group__globalFunctions_1ga3338c7af7db3773f41edb61bf9c5873c)`()`            | Get the [Unit](#classUnit) that the controlled [Unit](#classUnit) is currently interacting with.
`public void `[`print`](#group__globalFunctions_1ga428513e838de3670e09b242cde1f0f6b)`(...)`            | Print the String representation of the arguments to the console.
`public void `[`delay`](#group__globalFunctions_1ga95cd2552eeef0300ab5bae0ee284f82d)`(uint32_t nDelay)`            | Delay for nDelay milliseconds.
`public bool `[`load`](#group__globalFunctions_1gabb55e1126cb1e7626356bd529f6bc965)`(String file,...)`            | Load file and start a new thread that runs its main function.
`public bool `[`isIncluded`](#group__globalFunctions_1gae9d2eb16bc4dc333da24e3d968897f67)`(String file)`            | Determine whether a file has been included yet or not.
`public bool `[`include`](#group__globalFunctions_1ga41df84bb09ce63ef47fd30ddf980294b)`(String file)`            | Include file from the scriptDir\libs directory.
`public void `[`stop`](#group__globalFunctions_1ga8c8dfb6d9bcdee82b208da3033068b81)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` stop)`            | Conditionally stop current script.
`public void `[`stop`](#group__globalFunctions_1gaaff9311fc25289798786b3c364291a28)`(bool stop)`            | Conditionnaly stop current script.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`rand`](#group__globalFunctions_1ga6cbf7046bc4629083c0d50f6c5ce8843)`(int32_t low,int32_t high)`            | Return a random number between low and high.
`public bool `[`revealLevel`](#group__globalFunctions_revealLevel)`(bool drawPresets = false)`    | Reveal the player's current level on the automap.
`public bool `[`copy`](#group__globalFunctions_1gaca5b8f5b2f8d97182bab3979920352e4)`(String txt)`            | Copy txt to the clipboard; returns whether it was placed.
`public String `[`paste`](#globalFuncs_8h_1a17b290203f07cb10374e5a6a9c052508)`()`            | Get the text from the clipboard.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`sendCopyData`](#group__globalFunctions_1gad4e03d1abafaa3beab0e1cbb90ca8e37)`(String windowClassName,String windowName,int32_t nModeId,String data)`            | Send data to another window by means of the WM_COPYDATA message.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`sendCopyData`](#group__globalFunctions_1ga3dc20441410e40f237213140436a68d3)`(HWND hWnd,String dummy,int32_t nModeId,String data)`            | Send data to another window by means of the WM_COPYDATA message.
`public String `[`sendDDE`](#group__globalFunctions_1ga1b7c45afc2b2302e309c1376d3ecf83f)`(int32_t mode,String pszDDEServer,String pszTopic,String pszItem,String pszData)`            | Send data to a DDE server by means of DdeClientTransaction.
`public bool `[`keystate`](#group__globalFunctions_1ga8758a45d0a7e6bb5c42258ea99e7da07)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` vKey)`            | Get whether or not a key is pressed.
`public void `[`addEventListener`](#group__globalFunctions_1gaef5f2e6f72ff80cc13fd4ab84ab34810)`(String event,Function eventHandler)`            | Add an event handler to the list of listeners for the given event.
`public void `[`removeEventListener`](#group__globalFunctions_1gaee02cb255ac39d16c7035fb1b58c70ed)`(String event,Function eventHandler)`            | Removes a previously added event from the listener list.
`public void `[`clearEvent`](#group__globalFunctions_1ga185f0dafe7fd58fd565b8ce3b460541b)`(String event)`            | Clear the event handler list for given event.
`public void `[`clearAllEvents`](#group__globalFunctions_1ga229f0f1b263c13dc1a2c5bdcb51a5320)`()`            | Clear all event handlers (from all events); timers keep running.
`public int `[`setTimeout`](#group__globalFunctions_setTimeout)`(Function callback,double delay,...)`            | Call a function once, after a delay.
`public int `[`setInterval`](#group__globalFunctions_setInterval)`(Function callback,double period,...)`            | Call a function every period until the timer is cleared or its script ends.
`public void `[`clearTimeout`](#group__globalFunctions_clearTimeout)`(int id)`            | Cancel a timer by id.
`public void `[`clearInterval`](#group__globalFunctions_clearInterval)`(int id)`            | Cancel a timer by id; the same function as clearTimeout().
`public int `[`clearAllTimers`](#group__globalFunctions_clearAllTimers)`()`            | Cancel every timer of the calling script.
`public bool `[`js_strict`](#group__globalFunctions_1ga7b204d249e4e2d8274bec860070f1be3)`()`            | Get whether or not the JSOPTION_STRICT flag is set.
`public void `[`js_strict`](#group__globalFunctions_1gae9bcfbfe5b3392c7533b960c25720b4f)`(bool setStrictFlag)`            | Set or clear the JSOPTION_STRICT flag.
`public String `[`version`](#group__globalFunctions_1ga8972d7cb64b90764c110de40e134cfbc)`()`            | Get the D2BS version string.
`public void `[`version`](#group__globalFunctions_1ga8e60448ae3ce3cd28e7f438cfa9e11eb)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` a)`            | Print the D2BS version string.
`public void `[`scriptBroadcast`](#group__globalFunctions_1ga2dea3ce2441ba8f7a7e4c5bdf1be7bba)`(...)`            | Send a message to every running script that has a scriptmsg listener, the calling script included.
`public String `[`sqlite_version`](#group__globalFunctions_1ga182b9023c5495a885298b7f981ee8919)`()`            | Get the sqlite version string (from sqlite3_version).
`public double `[`sqlite_memusage`](#group__globalFunctions_1gae497b684eaeacd9196d73700e4d57d52)`()`            | Return the sqlite memory usage.
`public `[`Folder`](#classFolder)` `[`dopen`](#group__globalFunctions_1ga9a23806a759b9bace15e57adbdb90f91)`(String name)`            | Get the [Folder](#classFolder) object for the directory scriptPath\name.
`public void `[`debugLog`](#group__globalFunctions_1gacab2f81ca665988be5ca4ceb60503073)`(...)`            | Log the String representation of the arguments.
`public void `[`showConsole`](#group__globalFunctions_1ga084bae5a15c2ac3f27ea93f3a03ca0ce)`()`            | Show the console.
`public void `[`hideConsole`](#group__globalFunctions_1gaaaafa71c8fdd6db81a134ae4aaa355db)`()`            | Hide the console.
`public void `[`login`](#group__globalFunctions_1gaf76b7b46958dabf5e4ee9a492f0ec3fa)`()`            | Loads the current profile (set by the -profile launch argument, a D2Bot copydata message, or an earlier login(profile)) from d2bs.ini and gets to the lobby (for battle.net chars) or into a game (for single player characters).
`public void `[`login`](#group__globalFunctions_1gad9588235f179db8af822d1356337b021)`(String profile)`            | Loads the profile from d2bs.ini and gets to the lobby (for battle.net chars) or into a game (for single player characters).
`public bool `[`selectCharacter`](#group__globalFunctions_1ga5bec5be39d3ffe649ab5a32fa328d680)`(String profile)`            | Select the character from the given profile.
`public Object[] `[`getCharacterList`](#group__globalFunctions_getCharacterList)`()`            | Get every character on the character select screen in one call, in the order the screen lists them.
`public bool `[`focusCharacter`](#group__globalFunctions_focusCharacter)`(String name)`            | Select a character on the character select screen by name and scroll the list to show it.
`public void `[`createGame`](#group__globalFunctions_1ga73e647c911a4b4a29fa03498a86873f8)`(String name,String pass,int32_t diff)`            | Create a game with the given name and password in the given difficulty.
`public void `[`joinGame`](#group__globalFunctions_1gae7e03a89a0c5ce69e71e7489b5ff2d28)`(String name,String pass)`            | Join a game with the given name and password.
`public void `[`addProfile`](#group__globalFunctions_1ga198630d3b1155cad54add069502b17a8)`(String profile,String mode,String gateway,String username,String password,String charname,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` spDifficulty)`            | Create a profile with the given profile name, mode, gateway, username, password, character name and single player difficulty.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getLocation`](#group__globalFunctions_1ga21d43545897ce3af3692562593053480)`()`            | Get the current OOG location.
`public void `[`loadMpq`](#group__globalFunctions_1ga9add41a51f249d6a0d6fd3ed51fd7825)`(String path)`            | Load an mpq file.
`public bool `[`submitItem`](#group__globalFunctions_1gaaf6e11417ccd95f45faea57945684689)`()`            | Submit the item on the cursor to the open screen (like the add sockets screen).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getMouseCoords`](#group__globalFunctions_1gadf9310ea4d054d7013f93e4e8e840af4)`(bool nFlag,bool nReturn)`            | Get the mouse coordinates (in screen or map space) and return as an array.
`public `[`Unit`](#classUnit)` `[`copyUnit`](#group__globalFunctions_1ga8db0ed764b74081b12b217918f5f2f94)`(`[`Unit`](#classUnit)` other)`            | Copy the cached data associated with given unit.
`public bool `[`clickMap`](#group__globalFunctions_1gaa5bf89869bc19b084e30faf73fd9b5b3)`(uint16_t nClickType,uint16_t nShift,`[`Unit`](#classUnit)` toClick)`            | Click on the map with the given click type, with or without Shift held, at the given unit.
`public bool `[`clickMap`](#group__globalFunctions_1ga751d442b1f8e8a2a0f02c3d484d4e43c)`(uint16_t nClickType,bool nShift,`[`Unit`](#classUnit)` toClick)`            | Click on the map with the given click type, with or without Shift held, at the given unit.
`public bool `[`clickMap`](#group__globalFunctions_1ga192a8f3a562aa9801ccc9e90e48b132d)`(uint16_t nClickType,uint16_t nShift,uint16_t nX,uint16_t nY)`            | Click on the map with the given click type, with or without Shift held, at the given point.
`public bool `[`clickMap`](#group__globalFunctions_1gaf77b0ae23278cf2e0d3ab86c53a38f2d)`(uint16_t nClickType,bool nShift,uint16_t nX,uint16_t nY)`            | Click on the map with the given click type, with or without Shift held, at the given point.
`public bool `[`acceptTrade`](#group__globalFunctions_1ga1d54436fac5efdab779706bb6cce5144)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` action)`            | Read the state of the player-trade Accept button, or press it.
`public bool `[`acceptTrade`](#group__globalFunctions_1ga2a996039f7583fccf5900a6d6d12dee7)`()`            | Press your Accept button in a player trade: accept the trade, or withdraw your acceptance if you have already accepted.
`public void `[`tradeOk`](#group__globalFunctions_1ga1d2f580b3ec43e5e2d09e0ea3b417154)`()`            | Hit the OK button to open the trade window.
`public object[] `[`getDialogLines`](#group__globalFunctions_1ga337232421eadff983fd70603bc5211c9)`()`            | Get the lines of dialog of whatever you're talking to.
`public void `[`beep`](#group__globalFunctions_1gaa673a64fed7b36fcbbfaa63d92249019)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nBeepId)`            | Beep with beep ID nBeepId.
`public void `[`clickItem`](#group__globalFunctions_1gac5192a4a78c0610586d6878e0503be80)`(`[`Unit`](#classUnit)` item)`            | Click an item.
`public void `[`clickItem`](#group__globalFunctions_1ga59b940335f9d7a19a6d9294b3cf8959a)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nClickType,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nBodyLoc)`            | Click on a body location with a certain click type.
`public void `[`clickItem`](#group__globalFunctions_1ga49309d493f0b9601e12aa4f73db8165e)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nClickType,`[`Unit`](#classUnit)` item)`            | Click on a given item with the given click type.
`public void `[`clickItem`](#group__globalFunctions_1gad906e16da3dc0445fe6babdd9903a3c6)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nClickType,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nX,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nY,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nLoc)`            | Click an item location with the given click type.
`public double `[`getDistance`](#group__globalFunctions_1gae31399c1fa4cc70ad1235bfb1790556a)`(`[`Unit`](#classUnit)` a)`            | Get the euclidean distance from me to a.
`public double `[`getDistance`](#group__globalFunctions_1ga8d30dbe020059f12ee58f99e69c131ec)`(`[`Unit`](#classUnit)` a,`[`Unit`](#classUnit)` b)`            | Get the euclidean distance from a to b.
`public double `[`getDistance`](#group__globalFunctions_1gaa79333fe4ec69dc11bffd905431db402)`(`[`Unit`](#classUnit)` a,int32_t bx,int32_t by)`            | Get the euclidean distance from a to b.
`public double `[`getDistance`](#group__globalFunctions_1ga099a9099c3a32df0346fa40c496964b9)`(int32_t ax,int32_t ay)`            | Get the euclidean distance from me to a.
`public double `[`getDistance`](#group__globalFunctions_1gaf62a14b80beae3219199c0f05c1ea44b)`(int32_t ax,int32_t ay,`[`Unit`](#classUnit)` b)`            | Get the euclidean distance from a to b.
`public double `[`getDistance`](#group__globalFunctions_1ga5b44bdbe481beae0bd0a5fb322820864)`(int32_t ax,int32_t ay,int32_t bx,int32_t by)`            | Get the euclidean distance from a to b.
`public void `[`gold`](#group__globalFunctions_1ga43cd766ec6751004726dd28e5f5714d7)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nGold,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode)`            | Drop, offer in a trade, deposit or withdraw gold.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`checkCollision`](#group__globalFunctions_1gacbe9f546980a44ff675f8c5b77d617d3)`(`[`Unit`](#classUnit)` a,`[`Unit`](#classUnit)` b,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nBitMask)`            | Check if two units collide.
`public bool `[`playSound`](#group__globalFunctions_1gaa759ea651473dc19c7af1bde87f126c9)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSoundId)`            | Play d2 sound by id.
`public void `[`quit`](#group__globalFunctions_1ga2463a3acef2df4c951ce942a3229e44e)`()`            | Quit the game.
`public void `[`quitGame`](#group__globalFunctions_1gaa5cd0b13e9579fbd9b3f538ebae81d99)`()`            | Quit Diablo II.
`public bool `[`say`](#group__globalFunctions_1ga88ff9ae5c22b23bd4de2a7025c3cf107)`(...)`            | Say the string equivalent of the each of the arguments.
`public bool `[`clickParty`](#group__globalFunctions_1ga40e19c4f23164e0119973be3b66ec581)`(`[`Party`](#classParty)` player,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode)`            | Click one of the buttons in the party screen for the given player.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`weaponSwitch`](#group__globalFunctions_1gab9c5eb66ccf79108ee26a7f73943d0bc)`(int32_t dummy)`            | Get which weapon switch is being used.
`public void `[`transmute`](#group__globalFunctions_1ga66b26223d5346aaf653562d27cdfe889)`()`            | Hit the transmute button.
`public void `[`useStatPoint`](#group__globalFunctions_1ga895a78a9e6fbafc50cc592a81f891bb6)`(uint16_t statType,uint32_t count)`            | Use a stat point.
`public void `[`useSkillPoint`](#group__globalFunctions_1ga2316220fcf3f65233c46a23881f5702e)`(uint16_t skill,uint32_t count)`            | Use a skill point.
`public void `[`takeScreenshot`](#group__globalFunctions_1gab872a4402833e45312ccdde4004b4f96)`()`            | Take a screenshot.
`public bool `[`moveNPC`](#globalFuncs_8h_1a76152e5a307aa9fab69c1243453e51a7)`(`[`Unit`](#classUnit)` npc,uint32_t x,uint32_t y)`            | Move NPC to a new location.
`public bool `[`getPacket`](#globalFuncs_8h_1ae457d3d8c69ab22efeefa2bc07e51d5e)`(...)`            | Pass packets to the client as if the server had sent them.
`public bool `[`sendPacket`](#globalFuncs_8h_1a4082e1ce7c6ac097efd964d6397d197d)`(...)`            | Send a packet to the server.
`public String `[`getIP`](#globalFuncs_8h_1a7386e4db6f3be302ddd7a159d66774b1)`()`            | Get the external IP by going to "http://automation.whatismyip.com/n09230945.asp" and parsing the result.
`public Object `[`screenToAutomap`](#group__globalFunctions_1ga252bace45245bc8c4794f27d31f8ca45)`(Object arg)`            | Convert a point from screen coordinates to automap coordinates.
`public Object `[`screenToAutomap`](#group__globalFunctions_1gadb899fcbf6b0520f61c6ac2134b7bddd)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` ix,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` iy)`            | Convert a point from screen coordinates to automap coordinates.
`public Object `[`automapToScreen`](#group__globalFunctions_1ga8430f6eb530edcbdc6518f0449235cae)`(Object arg)`            | Convert a point from automap coordinates to screen coordinates.
`public Object `[`automapToScreen`](#group__globalFunctions_1ga12b95062f34ffb0b121981df69f6de66)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` ix,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` iy)`            | Convert a point from automap coordinates to screen coordinates.
`public String `[`md5`](#group__globalFunctions_1ga5aefa76b292174617e0020a21484ddc3)`(String str)`            | Takes the md5 hash.
`public String `[`sha1`](#group__globalFunctions_1gab3300abd7aaa5487c52f78452e32926b)`(String str)`            | Takes the sha1 hash.
`public String `[`sha256`](#group__globalFunctions_1gacbf583814bf77aa15bf6b10be1bc2921)`(String str)`            | Takes the sha256 hash.
`public String `[`sha384`](#group__globalFunctions_1ga16efc8fdeb82a91dde92253fd4f81f6b)`(String str)`            | Takes the sha384 hash.
`public String `[`sha512`](#group__globalFunctions_1gafe6cf810a65b58a3685657dfde830367)`(String str)`            | Takes the sha512 hash.
`public String `[`md5_file`](#group__globalFunctions_1ga84736bab060acfa890403db4fff9e25f)`(String file)`            | Take the md5 hash of a file.
`public String `[`sha1_file`](#group__globalFunctions_1ga2a0768372027c28a64fcb0780f7ff47b)`(String file)`            | Take the sha1 hash of a file.
`public String `[`sha256_file`](#group__globalFunctions_1gacd44b56a400b27db81deb0edd542a81e)`(String file)`            | Take the sha256 hash of a file.
`public String `[`sha384_file`](#group__globalFunctions_1gaec5638f75198f07622c214371a2bbef7)`(String file)`            | Take the sha384 hash of a file.
`public String `[`sha512_file`](#group__globalFunctions_1gad562af84fe5dd55bf5d5172861b2a28f)`(String file)`            | Take the sha512 hash of a file.
`namespace `[`ProfileType`](#namespaceProfileType) | [ProfileType](#namespaceProfileType) enum.
`class `[`Area`](#classArea) | Represent an [Area](#classArea) of the map.
`class `[`Box`](#classBox) | A [Box](#classBox) screen hook.
`class `[`Control`](#classControl) | This class represents out of game (OOG) [Control](#classControl) s.
`class `[`D2BSScript`](#classD2BSScript) | Represents a script (a thread), created by either loading a file, or typing something in the console.
`class `[`DBStatement`](#classDBStatement) | A prepared SQL statement on an SQLite database.
`class `[`Exit`](#classExit) | Represents a method of getting from one [Area](#classArea) to another.
`class `[`File`](#classFile) | This class handles the opening, reading and writing of files.
`class `[`FileTools`](#classFileTools) | A collection of tools to do things like move files and determine whether files exist.
`class `[`Folder`](#classFolder) | Represents a folder.
`class `[`Frame`](#classFrame) | This class represents a screen hook for a [Frame](#classFrame).
`class `[`Image`](#classImage) | This class represents a screen hook for an [Image](#classImage).
`class `[`Line`](#classLine) | A 2d line.
`class `[`Me`](#classMe) | The class that represents the controlled unit.
`class `[`Party`](#classParty) | Represents the items in the list in the [Party](#classParty) screen.
`class `[`PresetUnit`](#classPresetUnit) | Represents a [PresetUnit](#classPresetUnit) (a type of [Unit](#classUnit) that is available as soon as the [Area](#classArea) data is available).
`class `[`Profile`](#classProfile) | This class represents the details needed to get a player to a game or lobby.
`class `[`Room`](#classRoom) | Represents a "Room" in the game.
`class `[`Socket`](#classSocket) | A TCP connection to a host listed in d2bs.ini `Hosts`.
`class `[`SQLite`](#classSQLite) | Class for dealing with an [SQLite](#classSQLite) database.
`class `[`Text`](#classText) | A [Text](#classText) screen hook.
`class `[`Unit`](#classUnit) | This class represents a unit (monster, wp, npc, character, etc) in the game.

## Members

#### `public `[`Unit`](#classUnit)` `[`getUnit`](#group__globalFunctions_1ga6073f506327123ebf40219400e4d3f70)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` type,String name,uint32_t mode,uint32_t nUnitId)` 

Get a unit by type, name, mode and nUnitId.

#### Parameters
* `type` The type of the unit. Set to -1 to get any unit. 100 gets the cursor item, 101 gets the selected unit.

* `name` The name of the unit to look for.

* `mode` Either the mode being searched for, or a bitmask with bit 29 set and bits 0-28 corresponding to modes of units being searched for.

* `nUnitId` The unique id of the unit.

#### Returns
The first unit found that matches the description.

#### `public `[`Unit`](#classUnit)` `[`getUnit`](#group__globalFunctions_1ga22c01b5a8480e8fbf540e43bad5ae2f8)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` type,uint32_t classId,uint32_t mode,uint32_t nUnitId)` 

Get a unit by type, classId, mode and nUnitId.

#### Parameters
* `type` The type of the unit. Set to -1 to get any unit. 100 gets the cursor item, 101 gets the selected unit.

* `classId` The class id of the unit.

* `mode` Either the mode being searched for, or a bitmask with bit 29 set and bits 0-28 corresponding to modes of units being searched for.

* `nUnitId` The unique id of the unit.

#### Returns
The first unit found that matches the description.

#### `public object[] `[`getPath`](#group__globalFunctions_1ga7913f6338480cc180c015198b6c18024)`(uint32_t Area,uint32_t srcX,uint32_t srcY,uint32_t dstX,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` dstY,uint32_t reductionType,uint32_t Radius)` 

Creates a path (walking or teleporting) from the source to the destination.

Returns an array of points that form a path from the source to the destination. Takes [Area](#classArea) ID(s), source and destination points, walking or teleporting and range as parameters.

#### Parameters
* `[Area](#classArea)` The [Area](#classArea) ID to path through.

* `srcX` The source X coordinate.

* `srcY` The source Y coordinate.

* `dstX` The destination X coordinate.

* `dstY` The destination Y coordinate.

* `reductionType` The type of reduction: {0 - walking, 1 - teleport, 2 - none}

* `Radius` The distance between each point.

#### Returns
The path as an array of objects with x and y properties. Waits while an area or act loads, also one that begins before it looks the area up, except in a blocking listener or while your script holds the game lock. Throws an error when the area is not in the act the player is in, or when a load that begins as the call runs leaves its lookup empty past the wait (at once in a blocking listener or while your script holds the game lock).

#### `public object[] `[`getPath`](#group__globalFunctions_1ga051897c1e76c0ceeccd946dbedc6ea54)`(uint32_t Area,uint32_t srcX,uint32_t srcY,uint32_t dstX,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` dstY,uint32_t reductionType,uint32_t Radius,bool(*)(int32_t x, int32_t y) reject,bool(*)(object curPt, `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac) i, object[] pts)` reduce,int32_t[](*)(int32_t x, int32_t y) mutate)` 

Creates a path (walking or teleporting) from the source to the destination.

Returns an array of points that form a path from the source to the destination. Takes [Area](#classArea) ID, source and destination points, walking or teleporting, range and reduction or not as parameters.

#### Parameters
* `[Area](#classArea)` The [Area](#classArea) ID to path through.

* `srcX` The source X coordinate.

* `srcY` The source Y coordinate.

* `dstX` The destination X coordinate.

* `dstY` The destination Y coordinate.

* `reductionType` The type of reduction: {0 - walking, 1 - teleport, 2 - none, 3 - user specified}

* `Radius` The distance between each point. Reduction type 3 does not use it, but it must be passed so the functions are the 8th to 10th arguments.

* `reject` Called at most once for each point, with (x, y): for the start and the end before the search, then for each neighbouring point the search considers, except the end. A truthy return blocks the point. A point outside the current act's levels is blocked without a call.

* `reduce` Called once for each point of the path found, in order, with the point ({x, y}), its index and the array of all the points. A truthy return keeps the point in the result.

* `mutate` Called with (x, y) for the start or the end when it is blocked, before the search. Return [x, y] to move that point (fractions are truncated); a return that isn't an object keeps it. A start or end that is outside the current act's levels once mutate returns, moved there or kept, has no path, so getPath returns an empty path without calling reject or mutate again. An object whose elements 0 and 1 don't convert to numbers in the 32-bit integer range makes getPath throw a TypeError.

Reduction type 3 needs all three functions, or getPath throws. They are called with getPath's this (the global object for a plain call). An error one of them throws, or the script being stopped while one runs, ends the getPath call with it. When the script already holds the game lock as getPath starts, for example in a gamepacketsent listener that runs inside clickMap(), they run with the game paused and the D2BS log notes it once for that call; otherwise they run without the game lock, so the game keeps running while one waits, for example in delay(). They may call getCollision() and getPath(). Inside a listener the game is already waiting on, the game also stays paused until getPath returns or the game stops waiting for that listener (after 500 ms for a packet or chat blocker, 1 s for keyupblocker or keydownblocker, 3 s for a hook's click handler). After each reject or mutate call that returns normally, getPath checks that the player is still in the same game, act and area; if not, it calls none of the three again and returns an empty path. A trip away and back to the same area within one call may not be noticed. reduce runs after the search, so the path it reduces is still returned.

#### Returns
The path as an array of objects with x and y properties. Waits while an area or act loads, also one that begins before it looks the area up, except in a blocking listener or while your script holds the game lock. Throws an error when the area is not in the act the player is in, or when a load that begins as the call runs leaves its lookup empty past the wait (at once in a blocking listener or while your script holds the game lock).

#### `public unsigned short `[`getCollision`](#group__globalFunctions_1gac3a89f2d8c4940ae49dce43fbe2f626b)`(uint32_t nLevelId,int32_t nX,int32_t nY)` 

Get the collision flags at a given point in a given area.

#### Parameters
* `nLevelId` The level id of the area in question.

* `nX` The x coordinate of the point in question.

* `nY` The y coordinate of the point in question.

#### Returns
The collision flags. See [Collision flags](#collision-flags) in the reference tables. The area only picks the act: a point in another area of that act returns that area's flags. A room that is not loaded is loaded for the call and unloaded again. Returns 65535 (all bits set) for a point that lies in no room of the act's areas or in a room whose collision data can't be loaded, and for barricade-tower spots in areas 74, 111, 112 and 117. Waits while an area or act loads, also one that begins before it looks the area up, except in a blocking listener or while your script holds the game lock. Throws an error when the area is not in the act the player is in, or when a load that begins as the call runs leaves its lookup empty past the wait (at once in a blocking listener or while your script holds the game lock).

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getMercHP`](#group__globalFunctions_1gac4217ed30da8eb07d8addca4cdb51a9d)`()` 

Get the health of the controlled unit's merc, as a percentage.

#### Returns
The merc's HP percentage, 0 while the merc is dead, undefined when there is no merc, or false when the game is not ready.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getCursorType`](#group__globalFunctions_1gaa0bc4f7877d6992600202b8dce442787)`()` 

Get the state of the in-game hand cursor.

The value is the cursor's animation state, not a cursor image id:

| Value | Meaning |
|---|---|
| 1 | Normal pointer. |
| 2 | The hand opening after 5 seconds without a mouse move or click. |
| 3 | The hand closing again. While an item is held, a mouse move gives 3, and a click can then give 5 and then 1. |
| 4 | The open hand idling. Also set when an item is put on the cursor, until the mouse moves. |
| 5 | The click animation while the mouse button is down (hand cursor only, not in states 6 and 7). |
| 6 | Item-target cursor, for example after using an identify scroll or tome. |
| 7 | Shop action cursor, after the NPC shop's Buy, Sell or Repair button is clicked. |

The value does not reliably tell whether an item is held: use me.itemoncursor for that. There is no game-ready check, so outside a game the call returns whatever value the game last held.

#### Returns
The hand-cursor state.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getCursorType`](#group__globalFunctions_1ga9b3530843295a8ee8fa59a3130ae7409)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nType)` 

Get the hand-cursor state, or the item-panel mode when nType is 1.

With nType 1 the call returns the mode shared by the NPC shop, player trade, stash, Horadric Cube and submit-item panels. With any other value it returns the hand-cursor state described for getCursorType(). The argument is converted to an integer first, so "1" and 1.9 count as 1; a conversion that throws ends the call with that exception. There is no game-ready check, so outside a game the call returns whatever value the game last held.

Item-panel modes (nType 1). Values 0, 1, 12 and 14 were seen in a live game; the meanings of 5, 6, 7 and 19 are less certain than the others:

| Value | Meaning |
|---|---|
| 0 | No item panel mode (cleared when a panel closes). |
| 1 | NPC shop open, no action selected. |
| 2 | Buy button active. |
| 3 | Sell button active. |
| 4 | Repair button active. |
| 5 | A buy, sell or repair of an item has been requested. |
| 6 | The transaction was sent; waiting for the server. |
| 7 | The NPC answered with a transaction result message; the mode then returns to 1. |
| 10 | Player trade: the trade request prompt is open. |
| 11 | Player trade: the trade window is open. |
| 12 | Stash open. |
| 13 | A stash sub-state; meaning unknown. It turns back to 12 when a gold-amount dialog opens. |
| 14 | Horadric Cube open. |
| 19 | Submit-item panel open for the Horadric Staff orifice (other uses of the submit-item panel were not checked). |

#### Parameters
* `nType` 1 - the item-panel mode, any other value - the hand-cursor state.

#### Returns
The item-panel mode or the hand-cursor state.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getSkillByName`](#group__globalFunctions_1gaee860cd7482aff8e89519314a0f86955)`(String skillName)` 

Get skill ID by name.

#### Parameters
* `skillName` The skill name.

#### Returns
The skill id (0 is Attack), or undefined when the name is not in D2BS's built-in English skill-name table or the argument is not a string. The name is matched case-insensitively against that table (ids 0-155 and 219-280), not against the Skills.txt names listed under [Skill ids](#skill-ids) in the reference tables (some of them differ, for example Werewolf, not Wearwolf) and not against the game's localized names.

#### `public String `[`getSkillById`](#group__globalFunctions_1ga14cda78dea3cf554c8ebeeb0834039c9)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` skillId)` 

Get skill name by ID.

#### Parameters
* `skillId` The skill ID.

#### Returns
The skill's in-game name in the client's language (its skilldesc string), which is not always a name getSkillByName() and setSkill() accept. Returns "Unknown" for an id outside the game's skill table and for a skill that has no skill description (for example 156), and undefined when the argument is missing or is not an integer.

#### `public String `[`getLocaleString`](#group__globalFunctions_1gaf2065d798cd6a0337fb94f7bae718283)`(uint16_t localeId)` 

Get the String in the current locale that corresponds to the given id.

#### Parameters
* `localeId` The id of the String.

#### Returns
The String in the current locale.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getTextSize`](#group__globalFunctions_1ga19d918388535584b388c00c57d76ab47)`(String string,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` font,bool asObject)` 

Get the width and height of the given text in the given font.

#### Parameters
* `string` The string to get the size of.

* `font` The game font, 0-13 (see [Fonts](#fonts) in the reference tables). D2BS does not check the value; 14 or more makes the game read past its font table and can crash it.

* `asObject` False to return as an array.

#### Returns
The width and height of the text. 0 - width, 1 - height.

#### Parameters
* `string` The string to get the size of.

* `font` The game font, 0-13 (see [Fonts](#fonts) in the reference tables). D2BS does not check the value; 14 or more makes the game read past its font table and can crash it.

* `asObject` True to return as an object.

#### Returns
The width and height of the text. Has properties .width and .height.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getThreadPriority`](#group__globalFunctions_1ga6ab5869695c97ce6e4fbc6899e16a75d)`()` 

Get the priority of the current thread.

#### Returns
The priority.

#### `public bool `[`getUIFlag`](#group__globalFunctions_1gac259c30bc648650e021f45f1c020e412)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nUIId)` 

Get whether or not a UI flag is set.

Valid ids are 0 to 37. See [UI flags](#ui-flags) in the reference tables for what each id tracks.

Any other id (38 or more, or a negative id) makes the game exit immediately. D2BS does not check the range.

#### Parameters
* `nUIId` The ID of the UI in question, 0 to 37.

#### Returns
true when the panel or state is open or active, false when it is not. Returns undefined when the argument is missing or is not an integer (1.5 or a string), and false (with a 'Game not ready' warning) outside a ready game. While a game is loading the call can first wait for it to become ready.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getTradeInfo`](#group__globalFunctions_1gaedebe009d3f6c0afeab5cb544f430767)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode)` 

Get the current player-trade state.

With nMode 1, meant to get the name of the player you are trading with; it always returns null.

Modes 0 and 2 return the same number:

| Value | Meaning |
|---|---|
| 0 | No trade. |
| 1 | You asked another player to trade and are waiting for the answer (a box with only a Cancel button shows). |
| 2 | Another player asked to trade with you and the accept/decline prompt is showing. |
| 3 | The trade window is open, or about to open, and you have not accepted. |
| 5 | The trade window is open and the other player has accepted; you have not. |
| 7 | You have accepted. Also used for 30 seconds after a trade request was declined or cancelled (by you, or automatically by the game), with no trade window open; the state then returns to 0. |

The game declines new trade requests on its own whenever the state is not 0.

#### Parameters
* `nMode` 0 or 2 - the trade state. Both return the same value.

#### Returns
The trade state. An integer mode other than 0, 1 and 2 returns false. With no argument the call returns false at once; with a non-integer argument it waits for the game, then returns false. Outside a ready game it returns false (with a 'Game not ready' warning).

#### Parameters
* `nMode` 1.

#### Returns
null.

#### `public bool `[`getWaypoint`](#group__globalFunctions_1ga4561d06e77835b04e0794758844bd987)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nWaypointId)` 

Get whether the controlled unit has the given waypoint.

#### Parameters
* `nWaypointId` The waypoint's index in the game's waypoint list (the Waypoint column of Levels.txt: 0 for the Rogue Encampment, 1 for the Cold Plains), not an area id. An index above 40 is read as 0.

#### Returns
Whether the controlled unit has that waypoint. Returns false when the argument is missing or is not an integer, and false (with a 'Game not ready' warning) outside a ready game.

#### `public `[`D2BSScript`](#classD2BSScript)` `[`getScript`](#group__globalFunctions_1ga8340268c49de1e5f34588c01b0cf971d)`(bool currentScript)` 

Get the current or first context.

#### Parameters
* `currentScript` Whether to get the current script (true) or the first (false).

#### Returns
A [D2BSScript](#classD2BSScript) object for the request script.

#### `public `[`D2BSScript`](#classD2BSScript)` `[`getScript`](#group__globalFunctions_1ga02ad0d69faff60c7061a98a3e8a2b53b)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` threadId)` 

Get a script by thread id.

#### Parameters
* `threadId` The thread id of the script to get.

#### Returns
A [D2BSScript](#classD2BSScript) object for the script from the given thread.

#### `public `[`D2BSScript`](#classD2BSScript)` `[`getScript`](#group__globalFunctions_1gac202e144c149fa59378fd01585e6a2bd)`(String name)` 

Get a script by filename.

#### Parameters
* `name` The filename of the script to find.

#### Returns
A [D2BSScript](#classD2BSScript) object for the script.

#### `public `[`D2BSScript`](#classD2BSScript)` `[`getScript`](#group__globalFunctions_1ga822bdc4cbce00ca342a05bc29ffae76e)`()` 

Get the first script.

#### Returns
A [D2BSScript](#classD2BSScript) representing the first script in the runtime.

#### `public `[`Room`](#classRoom)` `[`getRoom`](#group__globalFunctions_1ga0ec5e4d9e09bf924c1f96ce417290417)`(uint32_t levelId)` 

Get the first room in area given by level id.

#### Parameters
* `levelId` The id of the area to get the room from, or 0 to get the room the controlled unit is in. Any number or numeric string is read as an area id; undefined, null and values that convert to NaN read as 0.

#### Returns
The first room in the area, or the controlled unit's room for 0. Waits while an area or act loads, and again when a load that begins as the call runs leaves its lookup empty; undefined when the area is not in the player's act, when the game ends as the call runs, or when a load that begins as the call runs leaves its lookup empty past the wait (at once in a blocking listener or while your script holds the game lock). Throws "Get Room Game not ready" out of game, when a load under way as the call starts outlasts the wait, or at once while the game is busy in a blocking listener or while your script holds the game lock.

#### `public `[`Room`](#classRoom)` `[`getRoom`](#group__globalFunctions_1ga6c10567ae9eac427ef0770edb2ab8c21)`(uint32_t levelId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y)` 

Get the room that the given point is in from the given level id.

#### Parameters
* `levelId` The level id to look in.

* `x` The x coordinate to find a room for.

* `y` The y coordinate to find a room for.

#### Returns
The room at the given point, or the area's first room when no room holds it; undefined when a coordinate is 0, when the area is not in the player's act, when the game ends as the call runs, or when a load that begins as the call runs leaves its lookup empty past the wait (at once in a blocking listener or while your script holds the game lock). Throws "Get Room Game not ready" out of game, when a load under way as the call starts outlasts the wait, or at once while the game is busy in a blocking listener or while your script holds the game lock.

#### `public `[`Room`](#classRoom)` `[`getRoom`](#group__globalFunctions_1gacc763e01907b73ca5c405469cc58cf12)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y)` 

Get the room that the given point is in.

#### Parameters
* `x` The x coordinate to find a room for.

* `y` The y coordinate to find a room for.

#### Returns
The room at the given point, or the current area's first room when no room holds it; undefined when a coordinate is 0, when the game ends as the call runs, or when a load that begins as the call runs leaves its lookup empty past the wait (at once in a blocking listener or while your script holds the game lock). Throws "Get Room Game not ready" out of game, when a load under way as the call starts outlasts the wait, or at once while the game is busy in a blocking listener or while your script holds the game lock.

#### `public `[`Room`](#classRoom)` `[`getRoom`](#group__globalFunctions_1gafc051829ad79986ed98043fd46bd9a2c)`()` 

Get the first room in the current area.

#### Returns
The first room in the current area; undefined when the game ends as the call runs, or when a load that begins as the call runs leaves its lookup empty past the wait (at once in a blocking listener or while your script holds the game lock). Throws "Get Room Game not ready" out of game, when a load under way as the call starts outlasts the wait, or at once while the game is busy in a blocking listener or while your script holds the game lock.

#### `public `[`Party`](#classParty)` `[`getParty`](#group__globalFunctions_1gae1a5223caa888f3cc2429fad719fc7eb)`()` 

Get the first party.

#### Returns
The first party.

#### `public `[`Party`](#classParty)` `[`getParty`](#group__globalFunctions_1ga6a0dc4f8e9bd582b1c8c4fc4da5e2f47)`(String name)` 

Get the party line for the player with the given name.

#### Parameters
* `name` The name of the player to search for

#### Returns
The party line

#### `public `[`Party`](#classParty)` `[`getParty`](#group__globalFunctions_1ga3fa567c623c83fe382918a7c6fb49978)`(uint32_t playerId)` 

Get the party line for the player with the given player id.

#### Parameters
* `playerId` 

#### Returns
The party line

#### `public `[`Party`](#classParty)` `[`getParty`](#group__globalFunctions_1gab98faccffab8b30337ba357504c145c3)`(`[`Unit`](#classUnit)` player)` 

Get the party line that corresponds to the given unit.

#### Parameters
* `player` A player [Unit](#classUnit); any other unit gives undefined.

#### Returns
The party line

#### `public `[`PresetUnit`](#classPresetUnit)` `[`getPresetUnit`](#group__globalFunctions_1ga6bf8c5a903fcf00dcd2504e8f4dc4ff7)`(uint32_t levelId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nType,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nClassId)` 

Get the first [PresetUnit](#classPresetUnit) of the given type and class id.

#### Parameters
* `levelId` The area id of the level to look for a [PresetUnit](#classPresetUnit) in.

* `nType` The type of the [PresetUnit](#classPresetUnit). See getUnit for type codes. 0 or omitted matches any type, and a value that converts to NaN matches none.

* `nClassId` The class id to find. See: [http://forums.d2botnet.org/viewtopic.php?f=18&t=986](http://forums.d2botnet.org/viewtopic.php?f=18&t=986) and [http://forums.d2botnet.org/viewtopic.php?f=18&t=985](http://forums.d2botnet.org/viewtopic.php?f=18&t=985) . 0 or omitted matches any class id, and a value that converts to NaN matches none.

#### Returns
The first [PresetUnit](#classPresetUnit) found of type nType and class ID nClassId. Waits while an area or act loads, also one that begins before it looks the area up, except in a blocking listener or while your script holds the game lock. Throws an error when the area is not in the act the player is in, or when a load that begins as the call runs leaves its lookup empty past the wait (at once in a blocking listener or while your script holds the game lock).

#### `public `[`PresetUnit`](#classPresetUnit)` `[`getPresetUnits`](#group__globalFunctions_1gaf77d2fb8bcab572d328c5764155be78d)`(uint32_t levelId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nType,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nClassId)` 

Get an array of [PresetUnit](#classPresetUnit) s of the given type and class id.

#### Parameters
* `levelId` The area id of the level to look for [PresetUnit](#classPresetUnit) s in.

* `nType` The type of the [PresetUnit](#classPresetUnit). See getUnit for type codes. 0 or omitted matches any type, and a value that converts to NaN matches none.

* `nClassId` The class id to find. See: [http://forums.d2botnet.org/viewtopic.php?f=18&t=986](http://forums.d2botnet.org/viewtopic.php?f=18&t=986) and [http://forums.d2botnet.org/viewtopic.php?f=18&t=985](http://forums.d2botnet.org/viewtopic.php?f=18&t=985) . 0 or omitted matches any class id, and a value that converts to NaN matches none.

#### Returns
An array of [PresetUnit](#classPresetUnit) s found of type nType and class ID nClassId. Waits while an area or act loads, also one that begins before it looks the area up, except in a blocking listener or while your script holds the game lock. Throws an error when the area is not in the act the player is in, or when a load that begins as the call runs leaves its lookup empty past the wait (at once in a blocking listener or while your script holds the game lock).

#### `public `[`Area`](#classArea)` `[`getArea`](#group__globalFunctions_1ga096676595eb6e9e2361dffe28acd8397)`()` 

Get the [Area](#classArea) where the controlled unit currently resides.

#### Returns
An [Area](#classArea) object for the current object. Waits while an area or act loads, also one that begins before it looks the area up, except in a blocking listener or while your script holds the game lock. False when a load that begins as the call runs leaves its lookup empty past the wait (at once in a blocking listener or while your script holds the game lock).

#### `public `[`Area`](#classArea)` `[`getArea`](#group__globalFunctions_1gae59e910620c20fb4fae45fc06d7cc351)`(int32_t nArea)` 

Get the [Area](#classArea) for area ID nArea.

#### Parameters
* `nArea` The area ID.

#### Returns
An [Area](#classArea) object. Waits while an area or act loads, also one that begins before it looks the area up, except in a blocking listener or while your script holds the game lock. False when the area is not in the act the player is in, or when a load that begins as the call runs leaves its lookup empty past the wait (at once in a blocking listener or while your script holds the game lock).

#### `public String `[`getBaseStat`](#group__globalFunctions_1gab4ebba6bce777cb9705e199af1d08bc8)`(String szTableName,int32_t nClassId,String szStatName)` 

Get the base stat from the given table with the given class ID and stat name.

#### Parameters
* `szTableName` The name of the table to look in.

* `nClassId` The class ID of the ...

* `szStatName` The stat name.

Tables, by index (either the name or the index selects a table): 0 items, 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2, 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp, 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets, 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable, 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level column is not stored, since the row is the level. In experience, row 0 holds each class's maximum level, row L (from 1 to the maximum level) the experience that reaches level L, and the row after that the experience cap.

#### Returns
The stat, or undefined when the table, row or column does not exist.

#### Parameters
* `szTableName` The name of the table to look in.

* `nClassId` The class ID of the ...

* `szStatName` The stat name.

Tables, by index (either the name or the index selects a table): 0 items, 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2, 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp, 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets, 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable, 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level column is not stored, since the row is the level. In experience, row 0 holds each class's maximum level, row L (from 1 to the maximum level) the experience that reaches level L, and the row after that the experience cap.

#### Returns
The stat, or undefined when the table, row or column does not exist.

#### Parameters
* `szTableName` The name of the table to look in.

* `nClassId` The class ID of the ...

* `szStatName` The stat name.

Tables, by index (either the name or the index selects a table): 0 items, 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2, 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp, 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets, 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable, 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level column is not stored, since the row is the level. In experience, row 0 holds each class's maximum level, row L (from 1 to the maximum level) the experience that reaches level L, and the row after that the experience cap.

#### Returns
The stat, or undefined when the table, row or column does not exist.

#### Parameters
* `szTableName` The name of the table to look in.

* `nClassId` The class ID of the ...

* `szStatName` The stat name.

Tables, by index (either the name or the index selects a table): 0 items, 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2, 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp, 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets, 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable, 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level column is not stored, since the row is the level. In experience, row 0 holds each class's maximum level, row L (from 1 to the maximum level) the experience that reaches level L, and the row after that the experience cap.

#### Returns
The stat, or undefined when the table, row or column does not exist.

#### `public String `[`getBaseStat`](#group__globalFunctions_1ga79338ba3a14fe002f38d987fed2809b3)`(int32_t nBaseStat,int32_t nClassId,String szStatName)` 

Get the base stat from the given table with the given class ID and stat name.

#### Parameters
* `nBaseStat` The stat to look up.

* `nClassId` The class ID of the ...

* `szStatName` The stat name.

Tables, by index (either the name or the index selects a table): 0 items, 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2, 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp, 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets, 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable, 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level column is not stored, since the row is the level. In experience, row 0 holds each class's maximum level, row L (from 1 to the maximum level) the experience that reaches level L, and the row after that the experience cap.

#### Returns
The stat, or undefined when the table, row or column does not exist.

#### Parameters
* `nBaseStat` The stat to look up.

* `nClassId` The class ID of the ...

* `szStatName` The stat name.

Tables, by index (either the name or the index selects a table): 0 items, 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2, 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp, 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets, 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable, 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level column is not stored, since the row is the level. In experience, row 0 holds each class's maximum level, row L (from 1 to the maximum level) the experience that reaches level L, and the row after that the experience cap.

#### Returns
The stat, or undefined when the table, row or column does not exist.

#### Parameters
* `nBaseStat` The stat to look up.

* `nClassId` The class ID of the ...

* `szStatName` The stat name.

Tables, by index (either the name or the index selects a table): 0 items, 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2, 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp, 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets, 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable, 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level column is not stored, since the row is the level. In experience, row 0 holds each class's maximum level, row L (from 1 to the maximum level) the experience that reaches level L, and the row after that the experience cap.

#### Returns
The stat, or undefined when the table, row or column does not exist.

#### Parameters
* `nBaseStat` The stat to look up.

* `nClassId` The class ID of the ...

* `szStatName` The stat name.

Tables, by index (either the name or the index selects a table): 0 items, 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2, 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp, 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets, 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable, 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level column is not stored, since the row is the level. In experience, row 0 holds each class's maximum level, row L (from 1 to the maximum level) the experience that reaches level L, and the row after that the experience cap.

#### Returns
The stat, or undefined when the table, row or column does not exist.

#### `public String `[`getBaseStat`](#group__globalFunctions_1gac86c8437c691e21b5593552ab9def9ba)`(String szTableName,int32_t nClassId,int32_t nStat)` 

Get the base stat from the given table with the given class ID and stat ID.

#### Parameters
* `szTableName` The name of the table to look in.

* `nClassId` The class ID of the ...

* `nStat` The stat ID.

Tables, by index (either the name or the index selects a table): 0 items, 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2, 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp, 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets, 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable, 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level column is not stored, since the row is the level. In experience, row 0 holds each class's maximum level, row L (from 1 to the maximum level) the experience that reaches level L, and the row after that the experience cap.

#### Returns
The stat, or undefined when the table, row or column does not exist.

#### Parameters
* `szTableName` The name of the table to look in.

* `nClassId` The class ID of the ...

* `nStat` The stat ID.

Tables, by index (either the name or the index selects a table): 0 items, 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2, 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp, 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets, 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable, 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level column is not stored, since the row is the level. In experience, row 0 holds each class's maximum level, row L (from 1 to the maximum level) the experience that reaches level L, and the row after that the experience cap.

#### Returns
The stat, or undefined when the table, row or column does not exist.

#### Parameters
* `szTableName` The name of the table to look in.

* `nClassId` The class ID of the ...

* `nStat` The stat ID.

Tables, by index (either the name or the index selects a table): 0 items, 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2, 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp, 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets, 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable, 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level column is not stored, since the row is the level. In experience, row 0 holds each class's maximum level, row L (from 1 to the maximum level) the experience that reaches level L, and the row after that the experience cap.

#### Returns
The stat, or undefined when the table, row or column does not exist.

#### Parameters
* `szTableName` The name of the table to look in.

* `nClassId` The class ID of the ...

* `nStat` The stat ID.

Tables, by index (either the name or the index selects a table): 0 items, 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2, 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp, 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets, 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable, 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level column is not stored, since the row is the level. In experience, row 0 holds each class's maximum level, row L (from 1 to the maximum level) the experience that reaches level L, and the row after that the experience cap.

#### Returns
The stat, or undefined when the table, row or column does not exist.

#### `public String `[`getBaseStat`](#group__globalFunctions_1gafcfdd2f8d3eb3a2c30436ddfc4add78b)`(int32_t nBaseStat,int32_t nClassId,int32_t nStat)` 

Get the base stat from the given table with the given class ID and stat ID.

#### Parameters
* `nBaseStat` The stat to look up.

* `nClassId` The class ID of the ...

* `nStat` The stat ID.

Tables, by index (either the name or the index selects a table): 0 items, 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2, 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp, 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets, 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable, 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level column is not stored, since the row is the level. In experience, row 0 holds each class's maximum level, row L (from 1 to the maximum level) the experience that reaches level L, and the row after that the experience cap.

#### Returns
The stat, or undefined when the table, row or column does not exist.

#### Parameters
* `nBaseStat` The stat to look up.

* `nClassId` The class ID of the ...

* `nStat` The stat ID.

Tables, by index (either the name or the index selects a table): 0 items, 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2, 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp, 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets, 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable, 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level column is not stored, since the row is the level. In experience, row 0 holds each class's maximum level, row L (from 1 to the maximum level) the experience that reaches level L, and the row after that the experience cap.

#### Returns
The stat, or undefined when the table, row or column does not exist.

#### Parameters
* `nBaseStat` The stat to look up.

* `nClassId` The class ID of the ...

* `nStat` The stat ID.

Tables, by index (either the name or the index selects a table): 0 items, 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2, 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp, 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets, 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable, 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level column is not stored, since the row is the level. In experience, row 0 holds each class's maximum level, row L (from 1 to the maximum level) the experience that reaches level L, and the row after that the experience cap.

#### Returns
The stat, or undefined when the table, row or column does not exist.

#### Parameters
* `nBaseStat` The stat to look up.

* `nClassId` The class ID of the ...

* `nStat` The stat ID.

Tables, by index (either the name or the index selects a table): 0 items, 1 monstats, 2 skilldesc, 3 skills, 4 objects, 5 missiles, 6 monstats2, 7 itemstatcost, 8 levels, 9 leveldefs, 10 lvlmaze, 11 lvlsub, 12 lvlwarp, 13 lvlprest, 14 lvltypes, 15 charstats, 16 setitems, 17 uniqueitems, 18 sets, 19 itemtypes, 20 runes, 21 cubemain, 22 gems, 23 experience, 24 pettable, 25 superuniques, 26 monlvl. In monlvl the row is the monster level and the columns are MonLvl.txt's 30 value columns, for example "L-DM(H)"; its Level column is not stored, since the row is the level. In experience, row 0 holds each class's maximum level, row L (from 1 to the maximum level) the experience that reaches level L, and the row after that the experience cap.

#### Returns
The stat, or undefined when the table, row or column does not exist.

#### `public `[`Control`](#classControl)` `[`getControl`](#group__globalFunctions_1gaad0e54fb02bc1ce440b96204167da80f)`(int32_t nType,int32_t nX,int32_t nY,int32_t nXSize,int32_t nYSize)` 

Get the control specified by type, location and size.

-1 indicates parameter not to be matched against.

Any negative value, and any argument that is not an integer (such as the string "6" or 6.5), also skips that field silently. With no arguments, or with every argument -1 or not an integer, the call returns the first control on the screen. When every field is skipped and at least one of them by a negative value other than -1, nothing matches and the call returns undefined. Controls exist only at the out-of-game menus. Pass at most five arguments: an integer in a sixth or later argument is written past the end of D2BS's argument storage and can crash the game.

nType is the control's type, the same value [Control](#classControl).type returns:

| Type | Name (D2BS source) | Control |
|---|---|---|
| 1 | CONTROL_EDITBOX | Single-line text input (account, password, game name). |
| 2 | CONTROL_IMAGE | Image. |
| 3 | CONTROL_UNUSED | Never constructed in 1.14d; matches nothing. |
| 4 | CONTROL_TEXTBOX | Multi-line text box or label (read with getText). |
| 5 | CONTROL_SCROLLBAR | Scroll bar. |
| 6 | CONTROL_BUTTON | Button. |
| 7 | CONTROL_LIST | List box. |
| 8 | CONTROL_TIMER | Timer (invisible). |
| 9 | CONTROL_SMACK | Video player. |
| 10 | CONTROL_PROGRESSBAR | Progress bar. |
| 11 | CONTROL_POPUP | Popup. |
| 12 | CONTROL_ACCOUNTLIST | Account list. |

#### Parameters
* `nType` The type of control (see the table above).

* `nX` The x coordinate of the control to find.

* `nY` The y coordinate of the control to find.

* `nXSize` The width of the control.

* `nYSize` The height of the control.

#### Returns
A [Control](#classControl) object for the first control that matches every given field, or undefined when none matches or when the client is not at the out-of-game menus. Throws 'Failed to build control!' only when the [Control](#classControl) object cannot be created.

#### `public bool `[`getPlayerFlag`](#group__globalFunctions_1gaa18d78f3e5bb312f53b602247ec66fd3)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nFirstUnitId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSecondUnitId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nFlag)` 

Get the relation between two players. It reads the party roster, so it answers for any player in the game, near this one or not.

nFlag is a bit mask; combine bits with | to test several at once. The bits are the party screen's per-player buttons, set by clicking them (or with clickParty) and also by the server's relation updates:

| Bit | Name | Meaning |
|---|---|---|
| 1 | loot | The loot button (allow corpse looting); available only in hardcore games (clickParty mode 0). |
| 2 | ignore | The ignore toggle next to squelch, as kolbot names it (clickParty mode 4). |
| 4 | squelch | The squelch toggle (clickParty mode 5). |
| 8 | hostile | Hostile (clickParty mode 1). The game lets you declare hostility only toward a player of level 9 or more. |

The flags read are the ones the first player holds toward the second. Do not rely on getPlayerFlag(me.gid, other.gid, 8) to tell whether the other player has declared hostility toward you.

#### Parameters
* `nFirstUnitId` The party-roster id (me.gid or a [Party](#classParty)'s gid) of the player holding the relation.

* `nSecondUnitId` The party-roster id of the player the relation is toward.

* `nFlag` The relation bits to test (see the table above).

#### Returns
true when any bit of nFlag is set, otherwise false: also when both ids are the same, when the first player is not in the party roster, when not exactly three numbers are passed, and when the game closes meanwhile. Outside a ready game it returns false (with a 'Game not ready' warning).

#### `public double `[`getTickCount`](#group__globalFunctions_1gad6eff12ded45cc505954f4763a71eae3)`()` 

Get the tick count.

Milliseconds since Windows started (GetTickCount64); it keeps counting past 2^32 (49.7 days), except on Windows XP.

#### Returns
The tick count.

#### `public `[`Unit`](#classUnit)` `[`getInteractedNPC`](#group__globalFunctions_1ga3338c7af7db3773f41edb61bf9c5873c)`()` 

Get the [Unit](#classUnit) that the controlled [Unit](#classUnit) is currently interacting with.

#### Returns
The [Unit](#classUnit) currently being interacted with.

#### `public void `[`print`](#group__globalFunctions_1ga428513e838de3670e09b242cde1f0f6b)`(...)` 

Print the String representation of the arguments to the console.

#### `public void `[`delay`](#group__globalFunctions_1ga95cd2552eeef0300ab5bae0ee284f82d)`(uint32_t nDelay)` 

Delay for nDelay milliseconds.

This is done by suspending the context, sleeping, then resuming the context for short (<50 ms) delays. For longer delays the context is suspended, 50ms are slept away, the context is resumed, sequence repeats until time slept is greater then nDelay ms.

#### Parameters
* `nDelay` Milliseconds to sleep.

#### `public bool `[`load`](#group__globalFunctions_1gabb55e1126cb1e7626356bd529f6bc965)`(String file,...)` 

Load file and start a new thread that runs its main function.

[File](#classFile) taken from scriptDir\file. All remaining parameters are copied to the new script's main function; a value that can't be copied (a function, or a game object such as a Unit) throws an error and starts nothing. Loading a script that is already running ends the running copy; the new copy starts once that copy has ended, or after half a second (at once when a script loads its own file), and getScript() and getScripts() then list only the new copy.

#### Parameters
* `file` The filename of the script.

#### Returns
true when the script's thread started, false when the file was not found or the thread could not start. A syntax error in the file still ends that script on its own thread after load() returned true; the calling script and the game keep running. A missing, empty, non-string or too-long file name throws an error.

#### `public bool `[`isIncluded`](#group__globalFunctions_1gae9d2eb16bc4dc333da24e3d968897f67)`(String file)` 

Determine whether a file has been included yet or not.

[File](#classFile) taken from scriptDir\libs\file

#### Parameters
* `file` Filename of the file that might have been included.

Throws if file is not a string.

#### Returns
Whether or not the file has been included.

#### `public bool `[`include`](#group__globalFunctions_1ga41df84bb09ce63ef47fd30ddf980294b)`(String file)` 

Include file from the scriptDir\libs directory.

#### Parameters
* `file` Filename of the file to include.

Throws if file is not a string.

#### Returns
Whether the inclusion was successful.

#### `public void `[`stop`](#group__globalFunctions_1ga8c8dfb6d9bcdee82b208da3033068b81)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` stop)` 

Conditionally stop current script.

#### Parameters
* `stop` 1 if current script should be stopped.

#### `public void `[`stop`](#group__globalFunctions_1gaaff9311fc25289798786b3c364291a28)`(bool stop)` 

Conditionnaly stop current script.

#### Parameters
* `stop` True if current script should be stopped.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`rand`](#group__globalFunctions_1ga6cbf7046bc4629083c0d50f6c5ce8843)`(int32_t low,int32_t high)` 

Return a random number between low and high.

Draws from the operating system's random generator (the C runtime's rand_s), in and out of game; it never touches the game's own random seed and never waits for the game.

#### Parameters
* `low` The lowest possible integer to return.

* `high` The highest possible integer to return.

#### Returns
A random integer in the range [low,high].

#### `public bool `[`revealLevel`](#group__globalFunctions_revealLevel)`(bool drawPresets = false)` 

Reveal the player's current level on the automap.

#### Parameters
* `drawPresets` True to also draw preset icons (shrines, special NPCs, level names). Optional; read only when it is the only argument, and false when omitted or not a boolean.

#### Returns
True once the player's level was walked and revealed (a room the game cannot load stays hidden), false when the game is not ready.

#### `public bool `[`copy`](#group__globalFunctions_1gaca5b8f5b2f8d97182bab3979920352e4)`(String txt)` 

Copy txt to the clipboard as Unicode text. Throws when txt is missing or not a string.

#### Parameters
* `txt` [Text](#classText) to copy to the clipboard.

#### Returns
true when the text was placed on the clipboard, false otherwise.

#### `public String `[`paste`](#globalFuncs_8h_1a17b290203f07cb10374e5a6a9c052508)`()` 

Get the text from the clipboard.

#### Returns
The text on the clipboard, or an empty string when it holds no text.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`sendCopyData`](#group__globalFunctions_1gad4e03d1abafaa3beab0e1cbb90ca8e37)`(String windowClassName,String windowName,int32_t nModeId,String data)` 

Send data to another window by means of the WM_COPYDATA message.

Uses FindWindow to get the HWND and SendMessage to send the message.

#### Parameters
* `windowClassName` The class name of the receiving window.

* `windowName` The window name of the receiving window.

* `nModeId` Data to be sent as the dwData member of the copy data.

* `data` Data to be send as the lpData member of the copy data.

#### Returns
The result of SendMessage.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`sendCopyData`](#group__globalFunctions_1ga3dc20441410e40f237213140436a68d3)`(HWND hWnd,String dummy,int32_t nModeId,String data)` 

Send data to another window by means of the WM_COPYDATA message.

Uses SendMessage to send the message to hWnd.

#### Parameters
* `hWnd` The handle to the receiving window.

* `dummy` Ignored.

* `nModeId` Data to be sent as the dwData member of the copy data.

* `data` Data to be send as the lpData member of the copy data.

#### Returns
The result of SendMessage.

#### `public String `[`sendDDE`](#group__globalFunctions_1ga1b7c45afc2b2302e309c1376d3ecf83f)`(int32_t mode,String pszDDEServer,String pszTopic,String pszItem,String pszData)` 

Send data to a DDE server by means of DdeClientTransaction.

#### Parameters
* `mode` The mode of transaction to use.

0 - Do command given by pszItem with an XTYP_REQUEST and get the resulting data.

1 - Send pszData to pszItem with an XTYP_POKE.

2 - Send pszData as an XTYP_EXECUTE command.

#### Parameters
* `pszDDEServer` The service name of the DDE server to connect to.

* `pszTopic` The topic with which to start the connection.

* `pszItem` The item to request (mode 0) or poke (mode 1).

* `pszData` The data to poke (mode 1) or execute (mode 2).

Arguments after the server are optional. mode must be an integer; sendDDE throws when mode or the server is missing.

#### Returns
Result data (at most 255 bytes) if mode was 0, otherwise false. A server that is not running gives "" in mode 0. Apart from the argument errors, it throws only when DDE itself cannot start.

#### `public bool `[`keystate`](#group__globalFunctions_1ga8758a45d0a7e6bb5c42258ea99e7da07)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` vKey)` 

Get whether or not a key is pressed.

Returns the value of GetAsyncKeyState.

#### Parameters
* `vKey` The virtual key code for the key in question. See: [http://msdn.microsoft.com/en-us/library/dd375731%28v=VS.85%29.aspx](http://msdn.microsoft.com/en-us/library/dd375731%28v=VS.85%29.aspx)

#### Returns
Whether not the key is pressed.

#### `public void `[`addEventListener`](#group__globalFunctions_1gaef5f2e6f72ff80cc13fd4ab84ab34810)`(String event,Function eventHandler)` 

Add an event handler to the list of listeners for the given event. A function already listening to that event is not added again, so it runs once per event and one removeEventListener() call removes it. Listeners run in the order they were added. A listener removed while an event is being delivered (with removeEventListener(), clearEvent() or clearAllEvents(), also from inside a listener) is not called for that event if it has not run yet (unless it is added again before its turn), and one added meanwhile runs from the next event. An error a listener throws is printed when it is thrown, the event's other listeners still run, and the error never blocks the event or reaches the rest of the script. Once the script is stopped (or, for an in-game script, its game ends), no further listener of the event runs.

#### Parameters
* `event` The name of the event. Can be:

melife (uint32_t dwLife)

memana (uint32_t dwMana)

keyup (uint32_t key)

keydown (uint32_t key)

bool keyupblocker (uint32_t key) - Returns true to block event from Diablo II. Blocking a key-up whose key-down reached the game leaves that key held down in the game until a later key-up for it passes.

bool keydownblocker (uint32_t key) - Returns true to block event from Diablo II

playerassign (uint32_t unitId)

mouseclick (uint32_t button, uint32_t x, uint32_t y, bool bUp)

mousemove (uint32_t x, uint32_t y)

scriptmsg (...)

golddrop (uint32_t gId, uint32_t Mode)

chatmsg (String lpszNick, String lpszMsg)

bool chatmsgblocker (String lpszNick, String lpszMsg) - Returns true to block event from Diablo II

whispermsg (String lpszNick, String lpszMsg)

bool whispermsgblocker (String lpszNick, String lpszMsg) - Returns true to block event from Diablo II

copydata (int32_t dwMode, String lpszMsg)

itemaction (uint32_t gId, uint32_t mode, String code, Bool global)

gameevent (BYTE mode, DWORD param1, DWORD param2, String name1, String name2)

From [http://www.blizzhackers.cc/viewtopic.php?t=392307](http://www.blizzhackers.cc/viewtopic.php?t=392307) Dark_Mage-

mode:

0x00 - "%Name1(%Name2) dropped due to time out." 0x01 - "%Name1(%Name2) dropped due to errors." 0x02 - "%Name1(%Name2) joined our world. Diablo's minions grow stronger." 0x03 - "%Name1(%Name2) left our world. Diablo's minions weaken." 0x04 - "%Name1 is not in the game." 0x05 - "%Name1 is not logged in."

0x06 - "%Name1 was Slain by %Name2" BYTE Param2 = [Unit](#classUnit) Type of Slayer (0x00 = Player, 0x01 = NPC) if Type = Player, Name2 = Slayer Character Name & DWORD Param1 = Slayer Character Type if Type = NPC, DWORD Param1 = Monster Id Code from MPQ (points to string for Name2) if Type = NPC & Monster is Unique, Name2 = Unique Monster Id

0x07 - Player Relations (Bottom Left [Text](#classText)) DWORD Param1 = Player Id Player Id = Pointer to Character Name BYTE Param2 = Action Taken Actions: 0x01 - "%Player permits you to loot his corpse." 0x02 - "%Player permits you to loot her corpse." 0x03 - "%Player has declared hostility towards you." 0x04 - "%Player is no longer hostile towards you." 0x05 - "%Player invites you to ally against the forces of evil." 0x06 - "%Player has canceled party invite." 0x07 - "%Player has joined your party to fight the forces of evil." 0x08 - "You are now allied with %Player." 0x09 - "%Player has left your party." 0x0a - "%Player no longer allows you to access his corpse." 0x0b - "%Player no longer allows you to access her corpse."

0x08 - "%Name1 is busy." 0x09 - "You must wait a short time to trade with that player."

0x0a - "%Name1 has items in his box." if Name1 = 0x00, "You have items in your box."

0x0b - Guild master kicked, from an unused guild feature (the English text is blank). No known server sends it. 0x0c - Not handled by the game client; meaning unknown. No known server sends it. 0x0d - "%Name1 is not listening to you." 0x0e - Received on 'Not enough mana' speech. 0x0f - "Realm going down in %Param1 minutes." 0x10 - "You must wait a short time to declare hostility with that player." 0x11 - "%Param1 Stones of Jordan Sold to Merchants" 0x12 - "Diablo Walks the Earth"

A listener whose result can block its event (`keyupblocker`, `keydownblocker`, `chatmsgblocker`, `chatinputblocker`, `whispermsgblocker`, `gamepacket`, `gamepacketsent`, `realmpacket`, and a hook's click handler) runs while the game, or the script that sent the packet, may be waiting for its answer. A call in it that needs a ready game returns its "Game not ready" result at once while the game is busy (an area or act loading, or the moment the player moves between rooms), rather than waiting. In the same way, `getPath()`, `getCollision()`, `getRoom()`, `getArea()`, `getPresetUnit()` and `getPresetUnits()` give their not-found result at once when their lookup comes up empty because the game turned busy after the call started. A `gamepacketsent` listener for a packet its own script sent runs inside that send instead; its calls wait as usual unless the send was made while the script holds the game lock or from inside a listener whose result can block its event, when they get "Game not ready" at once.

#### Parameters
* `eventHandler` A function with the signature listed above that handles the event.

Throws "Invalid arguments" if fewer than two arguments are passed.

#### `public void `[`removeEventListener`](#group__globalFunctions_1gaee02cb255ac39d16c7035fb1b58c70ed)`(String event,Function eventHandler)` 

Removes a previously added event from the listener list.

#### Parameters
* `event` The event name (the one used to add the event).

* `eventHandler` The handler to remove.

Throws "Invalid arguments" if fewer than two arguments are passed.

#### `public void `[`clearEvent`](#group__globalFunctions_1ga185f0dafe7fd58fd565b8ce3b460541b)`(String event)` 

Clear the event handler list for given event.

#### Parameters
* `event` The name of the event to clear the handler list for.

Throws "Invalid arguments" if no event is passed.

#### `public void `[`clearAllEvents`](#group__globalFunctions_1ga229f0f1b263c13dc1a2c5bdcb51a5320)`()` 

Clear all event handlers (from all events). Timers keep running: they stop with clearTimeout(), clearInterval(), clearAllTimers() or the end of their script.

#### `public int `[`setTimeout`](#group__globalFunctions_setTimeout)`(Function callback,double delay,...)` 

Call a function once, after a delay. An error the callback throws is reported when it happens, as one in main() is.

#### Parameters
* `callback` The function to call.

* `delay` The delay in milliseconds; a fraction is cut to whole milliseconds, and a negative or NaN delay calls it at once.

* `...` Extra arguments, passed on to the callback.

#### Returns
The timer's id, a whole number from 1 up, for clearTimeout(). Throws an error the script can catch when the callback is not a function, when the delay is missing or not a number, or when Windows could not create or set the timer.

#### `public int `[`setInterval`](#group__globalFunctions_setInterval)`(Function callback,double period,...)` 

Call a function every period until the timer is cleared or its script ends. A period of 0 (or a negative or NaN one) repeats every millisecond. A call is not started while the previous one is still running (for example inside delay()), and periods missed while the script was busy give one call. An error the callback throws is reported and the interval keeps running; clear it to stop it.

#### Parameters
* `callback` The function to call.

* `period` The period in milliseconds; a fraction is cut to whole milliseconds.

* `...` Extra arguments, passed on to the callback.

#### Returns
The timer's id, a whole number from 1 up, for clearInterval(). Throws an error the script can catch when the callback is not a function, when the period is missing or not a number, or when Windows could not create or set the timer.

#### `public void `[`clearTimeout`](#group__globalFunctions_clearTimeout)`(int id)` 

Cancel a timer by id. clearTimeout() and clearInterval() are the same function: either cancels a timer of either kind, and of any script. With no id, undefined or null it does nothing; a fraction is cut to a whole number (1.5 cancels timer 1); a number that is no timer's id (0, or a timer already cancelled) cancels nothing; any other value throws an error the script can catch.

#### Parameters
* `id` The id setTimeout() or setInterval() returned.

#### `public void `[`clearInterval`](#group__globalFunctions_clearInterval)`(int id)` 

Cancel a timer by id; the same function as clearTimeout(), with the same handling of undefined, null, fractions and other values.

#### Parameters
* `id` The id setTimeout() or setInterval() returned.

#### `public int `[`clearAllTimers`](#group__globalFunctions_clearAllTimers)`()` 

Cancel every timer of the calling script, including an interval whose callback is running.

#### Returns
How many timers it cancelled.

#### `public bool `[`js_strict`](#group__globalFunctions_1ga7b204d249e4e2d8274bec860070f1be3)`()` 

Get whether or not the JSOPTION_STRICT flag is set.

#### Returns
Whether or not the strict flag is set.

#### `public void `[`js_strict`](#group__globalFunctions_1gae9bcfbfe5b3392c7533b960c25720b4f)`(bool setStrictFlag)` 

Set or clear the JSOPTION_STRICT flag.

#### Parameters
* `setStrictFlag` The new JSOPTION_STRICT state.

#### `public String `[`version`](#group__globalFunctions_1ga8972d7cb64b90764c110de40e134cfbc)`()` 

Get the D2BS version string.

#### Returns
The D2BS version string.

#### `public void `[`version`](#group__globalFunctions_1ga8e60448ae3ce3cd28e7f438cfa9e11eb)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` a)` 

Print the D2BS version string.

#### `public void `[`scriptBroadcast`](#group__globalFunctions_1ga2dea3ce2441ba8f7a7e4c5bdf1be7bba)`(...)` 

Send a message to every running script that has a scriptmsg listener, the calling script included.

D2BS calls each such script's scriptmsg listeners with copies of the values passed in. The values are copied once, so getters on them run once however many scripts listen, and not at all when none does. When a script listens, a value that can't be copied (a function, or a game object such as a Unit) throws an error and no script gets the message. A paused script, or one that has not started yet, does not get it; script.send() reaches those.

#### `public String `[`sqlite_version`](#group__globalFunctions_1ga182b9023c5495a885298b7f981ee8919)`()` 

Get the sqlite version string (from sqlite3_version).

#### Returns
The sqlite version string.

#### `public double `[`sqlite_memusage`](#group__globalFunctions_1gae497b684eaeacd9196d73700e4d57d52)`()` 

Return the number of bytes of memory SQLite currently holds.

#### Returns
The memory SQLite holds, in bytes (0 before SQLite is first used).

#### `public `[`Folder`](#classFolder)` `[`dopen`](#group__globalFunctions_1ga9a23806a759b9bace15e57adbdb90f91)`(String name)` 

Get the [Folder](#classFolder) object for the directory scriptPath\name, creating the folder when it is missing (its parent must exist).

#### Parameters
* `name` The name of the [Folder](#classFolder) to open, relative to the script folder. It can't leave the script folder or make a path longer than Windows allows.

#### Returns
A [Folder](#classFolder) object representing the [Folder](#classFolder) name, or undefined when no name, or more than one argument, is passed. A name that isn't a string or isn't a valid folder name throws an error.

#### `public void `[`debugLog`](#group__globalFunctions_1gacab2f81ca665988be5ca4ceb60503073)`(...)` 

Log the String representation of the arguments.

#### `public void `[`showConsole`](#group__globalFunctions_1ga084bae5a15c2ac3f27ea93f3a03ca0ce)`()` 

Show the console.

#### `public void `[`hideConsole`](#group__globalFunctions_1gaaaafa71c8fdd6db81a134ae4aaa355db)`()` 

Hide the console.

#### `public void `[`login`](#group__globalFunctions_1gaf76b7b46958dabf5e4ee9a492f0ec3fa)`()` 

Loads the current profile (set by the -profile launch argument, a D2Bot copydata message, or an earlier login(profile)) from d2bs.ini and gets to the lobby (for battle.net chars) or into a game (for single player characters).

#### Exceptions
* `String` May throw one of the following:

"invalid character name" - If it fails to select the character.

"Failed to click the Single button?" - If it fails to click the single player button.

"Failed to click the 'Battle.net' button?" - If it fails to click the battle.net button.

"Failed to click the 'Other Multiplayer' button?" - If it fails to click the Other Multiplayer button.

"Failed to click the 'Open Battle.net' button?" - If it fails to click the Open battle.net button.

"Failed to click the 'TCP/IP' button?" - If it fails to click the TCP/IP button.

"Failed to click the 'Host Game' button?" - If it fails to click the Host Game (TCP/IP) button.

"Failed to click the 'Join Game' button?" - If it fails to click the Join Game (TCP/IP) button.

"Failed to click the OK button" - If it fails to click the OK button (TCP/IP enter IP address).

"Failed to find the 'Host IP Address' text-edit box." - If it can't find the host IP address (TCP/IP) text box.

"Could not get the IP address from the profile in the d2bs.ini file." - If the user failed to specify the IP address in the d2bs.ini file.

"Failed to click the exit button?" - If it is login and fails to click the exit button.

"Failed to set the 'Username' text-edit box.' - If it can't find the username control.

"Failed to set the 'Password' text-edit box." - If it can't find the password control.

"Failed to click the 'Log in' button?" - If it can't click the login button.

"Failed to click the 'Normal Difficulty' button?" - If it can't click the normal button in single player game creation.

"Failed to click the 'Nightmare Difficulty' button?" - If it can't click the nightmare button in single player game creation.

"Failed to click the 'Hell Difficulty' button?" - If it can't click the hell button in single player game creation.

"Invalid single player difficulty level specified!" - If the difficulty level is invalid.

"Unable to connect" - If the unable to connect screen is shown.

"CD-Key in use" - If the cdkey in use screen is shown.

"Bad account or password" - If the username/password wrong screen is shown.

"Realm Down" - If the realm down message is shown.

"Unhandled login location" - If the game is in an unsupported location.

#### `public void `[`login`](#group__globalFunctions_1gad9588235f179db8af822d1356337b021)`(String profile)` 

Loads the profile from d2bs.ini and gets to the lobby (for battle.net chars) or into a game (for single player characters).

#### Parameters
* `profile` The profile to load.

#### Exceptions
* `String` May throw one of the following:

"invalid character name" - If it fails to select the character.

"Failed to click the Single button?" - If it fails to click the single player button.

"Failed to click the 'Battle.net' button?" - If it fails to click the battle.net button.

"Failed to click the 'Other Multiplayer' button?" - If it fails to click the Other Multiplayer button.

"Failed to click the 'Open Battle.net' button?" - If it fails to click the Open battle.net button.

"Failed to click the exit button?" - If it is login and fails to click the exit button.

"Failed to set the 'Username' text-edit box.' - If it can't find the username control.

"Failed to set the 'Password' text-edit box." - If it can't find the password control.

"Failed to click the 'Log in' button?" - If it can't click the login button.

"Failed to click the 'Normal Difficulty' button?" - If it can't click the normal button in single player game creation.

"Failed to click the 'Nightmare Difficulty' button?" - If it can't click the nightmare button in single player game creation.

"Failed to click the 'Hell Difficulty' button?" - If it can't click the hell button in single player game creation.

"Invalid single player difficulty level specified!" - If the difficulty level is invalid.

"Unable to connect" - If the unable to connect screen is shown.

"CD-Key in use" - If the cdkey in use screen is shown.

"Bad account or password" - If the username/password wrong screen is shown.

"Realm Down" - If the realm down message is shown.

"Unhandled login location" - If the game is in an unsupported location.

#### `public bool `[`selectCharacter`](#group__globalFunctions_1ga5bec5be39d3ffe649ab5a32fa328d680)`(String profile)` 

Select the character from the given profile.

Keeps all details except the character name from old profile.

#### Parameters
* `profile` The profile to get the character name from.

#### Returns
Whether or not character swap was successful.

#### `public Object[] `[`getCharacterList`](#group__globalFunctions_getCharacterList)`()` 

Get every character on the character select screen in one call, in the order the screen lists them.

Only the expansion character select screen is supported. On a realm the list holds the characters the realm sent: when an account has more, the screen's "more characters" row is left out.

#### Returns
false when getLocation() does not report 12 (only the expansion character select screen does, and a popup such as the difficulty choice changes the location), a realm is still sending its list, or D2BS is still starting up. A screen with no characters reports 42, as it can while a list is still loading. Otherwise an array with one object per character: name, realm (empty for single player), classid (0 amazon to 6 assassin), level, hardcore, expansion, ladder, dead (a hardcore character that died and can't be played) and expires (the realm expiry as Unix time in seconds, 0 when there is none, as in single player).

#### `public bool `[`focusCharacter`](#group__globalFunctions_focusCharacter)`(String name)` 

Select a character on the character select screen by name and scroll the list to show it.

Selecting does not log in: click OK or press Enter after it returns. A character slot click or arrow key in between changes the selection again. Only the expansion character select screen is supported.

#### Parameters
* `name` The character name, not case sensitive. Anything but a string throws.

#### Returns
true when the character is selected; false when it is not in the list, getLocation() does not report 12, a realm is still sending its list, a character is being deleted, a popup such as the delete confirmation or the gateway list is open, or D2BS is still starting up.

#### `public void `[`createGame`](#group__globalFunctions_1ga73e647c911a4b4a29fa03498a86873f8)`(String name,String pass,int32_t diff)` 

Create a game with the given name and password in the given difficulty.

#### Parameters
* `name` The game name.

* `pass` The game password.

* `diff` The game difficulty. 0 - normal, 1 - nightmare, 2 - hell, 3 - hardest difficulty available.

#### `public void `[`joinGame`](#group__globalFunctions_1gae7e03a89a0c5ce69e71e7489b5ff2d28)`(String name,String pass)` 

Join a game with the given name and password.

#### Parameters
* `name` The game name.

* `pass` The game password.

#### `public void `[`addProfile`](#group__globalFunctions_1ga198630d3b1155cad54add069502b17a8)`(String profile,String mode,String gateway,String username,String password,String charname,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` spDifficulty)` 

Create a profile with the given profile name, mode, gateway, username, password, character name and single player difficulty.

#### Parameters
* `profile` The profile name.

* `mode` The mode (single player/battle.net/etc).

* `gateway` The realm gateway.

* `username` Account username.

* `password` Account password.

* `charname` Character name (case sensitive).

* `spDifficulty` The difficulty to create games for a single player character.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getLocation`](#group__globalFunctions_1ga21d43545897ce3af3692562593053480)`()` 

Get the current OOG location.

D2BS detects the out-of-game screen by the buttons and text boxes it shows. The values (0 to 44) are listed in [Out-of-game locations](#out-of-game-locations) in the reference tables. A popup the detection knows reports the popup; an unknown popup can report the screen under it, or 0.

#### Returns
The current OOG location, 0 when on the menus but no known screen matched, or null while not on the out-of-game menus: in a game, while a game loads, and in the moments between screens when no controls exist.

#### `public void `[`loadMpq`](#group__globalFunctions_1ga9add41a51f249d6a0d6fd3ed51fd7825)`(String path)` 

Load an mpq file.

#### Parameters
* `path` The mpq file to load

Throws if path is not a string.

#### `public bool `[`submitItem`](#group__globalFunctions_1gaaf6e11417ccd95f45faea57945684689)`()` 

Submit the item on the cursor to the open screen (like the add sockets screen).

#### Returns
Whether or not it was successful.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getMouseCoords`](#group__globalFunctions_1gadf9310ea4d054d7013f93e4e8e840af4)`(bool nFlag,bool nReturn)` 

Get the mouse coordinates (in screen or map space) and return as an array.

Get the mouse coordinates (in screen or map space) and return as an Object.

#### Parameters
* `nFlag` Whether to return in map space (true) or screen space (false).

* `nReturn` Whether to return as an array or as an Object, false for array format.

#### Returns
The mouse coordinates in an array. 0 - x, 1 - y.

#### Parameters
* `nFlag` Whether to return in map space (true) or screen space (false).

* `nReturn` Whether to return as an array or as an Object, true for Object format.

#### Returns
The mouse coordinates in an array. Has properties .x and .y.

#### `public `[`Unit`](#classUnit)` `[`copyUnit`](#group__globalFunctions_1ga8db0ed764b74081b12b217918f5f2f94)`(`[`Unit`](#classUnit)` other)` 

Copy the cached data associated with given unit.

This is things like type and unit id.

The copy is a new [Unit](#classUnit) object for the same game unit. It holds other's unit id, type and getUnit/getNext search filters (and, for an item reached through getItem, its owner). It is not a snapshot: like the original, it reads live game data on every access and returns false or undefined once the unit has left the game. The difference is that getNext() moves only the object it is called on, so a copy stays on its unit while the original keeps iterating.

Use it to keep the units found in a getUnit/getNext loop, for example list.push(copyUnit(unit)). Pushing unit itself stores the same object every time, so every entry ends up on the unit the loop finished on.

#### Parameters
* `other` The [Unit](#classUnit) to copy.

#### Returns
The new copy of the unit, or undefined when other is missing or is not a [Unit](#classUnit). Throws 'Couldn't copy unit' only when the new object cannot be created.

#### `public bool `[`clickMap`](#group__globalFunctions_1gaa5bf89869bc19b084e30faf73fd9b5b3)`(uint16_t nClickType,uint16_t nShift,`[`Unit`](#classUnit)` toClick)` 

Click on the map with the given click type, with or without Shift held, at the given unit.

nClickType picks the mouse button and phase; a normal click is a down followed by an up:

0 - Left button down

1 - Left button held

2 - Left button up

3 - Right button down

4 - Right button held

5 - Right button up

Any other value does nothing. The value is read as a 16-bit number first (wrapping modulo 65536), so 65536 counts as 0 and -65535 as 1.

The click lands on the unit's position and targets that unit, for example to attack it, talk to it or pick it up. When toClick is your own character, the click is a plain map click at your position and targets no unit. The game ignores the click while UI flag 0x09 (the Esc menu) is set.

#### Parameters
* `nClickType` The click type to do (see above).

* `nShift` Whether or not to hold down shift, read as a 16-bit number (wrapping modulo 65536): 0 - don't hold shift, any other value - hold shift. With shift held, D2BS also adds the flag that the game's Always Run setting adds; the game applies it only while the character has stamina.

* `toClick` The unit to click on.

#### Returns
true once the click is handed to the game, even when the game ignores it (a click type above 5, the Esc menu open, or a click the game is still handling). false when fewer than three arguments are given, the arguments match neither form (a non-integer number such as 1.5 included), toClick is not a [Unit](#classUnit) or no longer exists, or the player is not in a game or the game does not become ready in time (with a 'Game not ready' warning). It does not throw.

#### `public bool `[`clickMap`](#group__globalFunctions_1ga751d442b1f8e8a2a0f02c3d484d4e43c)`(uint16_t nClickType,bool nShift,`[`Unit`](#classUnit)` toClick)` 

Click on the map with the given click type, with or without Shift held, at the given unit.

nClickType and the click itself work as in the first clickMap overload.

#### Parameters
* `nClickType` The click type to do: 0 left down, 1 left held, 2 left up, 3 right down, 4 right held, 5 right up.

* `nShift` Whether or not to be holding down shift. With shift held, D2BS also adds the flag that the game's Always Run setting adds; the game applies it only while the character has stamina.

* `toClick` The unit to click on.

#### Returns
true once the click is handed to the game, even when the game ignores it. false when the arguments are invalid, toClick is not a [Unit](#classUnit) or no longer exists, or the player is not in a game (with a 'Game not ready' warning). It does not throw.

#### `public bool `[`clickMap`](#group__globalFunctions_1ga192a8f3a562aa9801ccc9e90e48b132d)`(uint16_t nClickType,uint16_t nShift,uint16_t nX,uint16_t nY)` 

Click on the map with the given click type, with or without Shift held, at the given point.

nClickType works as in the first clickMap overload. nX and nY are game map coordinates (the same as unit.x and unit.y), not screen pixels, and the point does not need to be on screen. Use the [Unit](#classUnit) form to interact with a unit.

#### Parameters
* `nClickType` The click type to do: 0 left down, 1 left held, 2 left up, 3 right down, 4 right held, 5 right up.

* `nShift` Whether or not to hold down shift, read as a 16-bit number (wrapping modulo 65536): 0 - don't hold shift, any other value - hold shift. With shift held, D2BS also adds the flag that the game's Always Run setting adds; the game applies it only while the character has stamina.

* `nX` The x coordinate to click at (map coordinates; negative values wrap).

* `nY` The y coordinate to click at (map coordinates; negative values wrap).

#### Returns
true once the click is handed to the game, even when the game ignores it (a click type above 5, the Esc menu open, or a click the game is still handling). false when fewer than three arguments are given, the arguments match neither form (a non-integer number such as 1.5 included), or the player is not in a game or the game does not become ready in time (with a 'Game not ready' warning). It does not throw.

#### `public bool `[`clickMap`](#group__globalFunctions_1gaf77b0ae23278cf2e0d3ab86c53a38f2d)`(uint16_t nClickType,bool nShift,uint16_t nX,uint16_t nY)` 

Click on the map with the given click type, with or without Shift held, at the given point.

nClickType works as in the first clickMap overload, and nX and nY as in the third.

#### Parameters
* `nClickType` The click type to do: 0 left down, 1 left held, 2 left up, 3 right down, 4 right held, 5 right up.

* `nShift` Whether or not to be holding down shift. With shift held, D2BS also adds the flag that the game's Always Run setting adds; the game applies it only while the character has stamina.

* `nX` The x coordinate to click at (map coordinates).

* `nY` The y coordinate to click at (map coordinates).

#### Returns
true once the click is handed to the game, even when the game ignores it. false when the arguments are invalid or the player is not in a game (with a 'Game not ready' warning). It does not throw.

#### `public bool `[`acceptTrade`](#group__globalFunctions_1ga1d54436fac5efdab779706bb6cce5144)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` action)` 

Read the state of the player-trade Accept button, or press it.

action is converted to an integer the way action | 0 does: the fraction is dropped and the value wraps at 2^32, so "2" counts as 2, true and 1.5 count as 1, and 4294967297 counts as 1. When the result is anything other than 1, 2 or 3 (a non-numeric string, undefined and other NaN values become 0), the call presses the button exactly as acceptTrade() does.

#### Parameters
* `action` What action to perform.

1 - Get whether you have pressed Accept (the trade screen's own Accept flag).

2 - Get the trade state number (the same value as getTradeInfo(0)).

3 - Get whether the accept check is red (the server has disabled the Accept button).

#### Returns
The value specified by action: true or false for 1 and 3, a number for 2. Outside a ready game it returns false (with a 'Game not ready' warning). A value whose conversion throws ends the call with that exception.

#### `public bool `[`acceptTrade`](#group__globalFunctions_1ga2a996039f7583fccf5900a6d6d12dee7)`()` 

Press your Accept button in a player trade: accept the trade, or withdraw your acceptance if you have already accepted.

It acts like a click on the trade screen's Accept button. When acceptTrade(1) is false it accepts (getTradeInfo(0) becomes 7). Otherwise it withdraws your acceptance: the state returns to 3 and the trade window stays open; it does not cancel the trade.

It acts only when getTradeInfo(0) is 3, 5 or 7. State 7 is also the 30-second pause after a trade request was declined or cancelled, with no trade window open. A call in that pause sends an accept (or an un-accept, if acceptTrade(1) is still true) with no trade open; the state then stays at 7 (or 3) instead of returning to 0, so the game keeps declining new trade requests until the server ends the trade. Check that the trade window is open before calling it.

#### Returns
true when the button was pressed or released; false, with nothing done, while the accept check is red (acceptTrade(3) is true). Throws "Invalid parameter passed to acceptTrade!" when getTradeInfo(0) is not 3, 5 or 7, although the call has no parameter at fault. Outside a ready game it returns false (with a 'Game not ready' warning).

#### `public void `[`tradeOk`](#group__globalFunctions_1ga1d2f580b3ec43e5e2d09e0ea3b417154)`()` 

Hit the OK button to open the trade window.

Throws an error if client not in a proper state to click ok.

#### `public object[] `[`getDialogLines`](#group__globalFunctions_1ga337232421eadff983fd70603bc5211c9)`()` 

Get the lines of dialog of whatever you're talking to.

Each object has a text property which gives the text and a selectable property which is a boolean as to whether you can click on that line or not.

#### Returns
An array of objects of the lines, or undefined if no dialog up.

#### `public void `[`beep`](#group__globalFunctions_1gaa673a64fed7b36fcbbfaa63d92249019)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nBeepId)` 

Beep with beep ID nBeepId.

#### Parameters
* `nBeepId` ID of the beep to make. See: [http://msdn.microsoft.com/en-us/library/ms680356%28VS.85%29.aspx](http://msdn.microsoft.com/en-us/library/ms680356%28VS.85%29.aspx) .

#### `public void `[`clickItem`](#group__globalFunctions_1gac5192a4a78c0610586d6878e0503be80)`(`[`Unit`](#classUnit)` item)` 

Click an item.

#### Parameters
* `item` The item to click on.

#### `public void `[`clickItem`](#group__globalFunctions_1ga59b940335f9d7a19a6d9294b3cf8959a)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nClickType,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nBodyLoc)` 

Click on a body location with a certain click type.

#### Parameters
* `nClickType` The click type. {0 - left click, 4 - left click merc body locations}

* `nBodyLoc` The body location.

0 - Not equipped

1 - Helmet

2 - Amulet

3 - Armor

4 - Right hand slot 1

5 - Left hand slot 1

6 - Right ring

7 - Left ring

8 - Belt

9 - Boots

10 - Gloves

11 - Right hand slot 2

12 - Left hand slot 2

#### `public void `[`clickItem`](#group__globalFunctions_1ga49309d493f0b9601e12aa4f73db8165e)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nClickType,`[`Unit`](#classUnit)` item)` 

Click on a given item with the given click type.

#### Parameters
* `nClickType` The click type. {0 - left click, 1 - right click, 4 - left click merc location}

* `item` The item to click.

* `nClickType`

#### `public void `[`clickItem`](#group__globalFunctions_1gad906e16da3dc0445fe6babdd9903a3c6)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nClickType,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nX,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nY,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nLoc)` 

Click an item location with the given click type.

#### Parameters
* `nClickType` The click type. {0 - left click, 1 - right click, 2 - shift left click, 4 - left click merc location}

* `nX` The x coordinate of the location to click.

* `nY` The y coordinate of the location to click.

* `nLoc` The game location of the item.

2 - Belt

3 - Inventory

4 - Player trade

6 - Cube

7 - Stash

#### `public double `[`getDistance`](#group__globalFunctions_1gae31399c1fa4cc70ad1235bfb1790556a)`(`[`Unit`](#classUnit)` a)` 

Get the euclidean distance from me to a.

#### Parameters
* `a` The second [Unit](#classUnit).

#### Returns
The euclidean distance from a.

#### `public double `[`getDistance`](#group__globalFunctions_1ga8d30dbe020059f12ee58f99e69c131ec)`(`[`Unit`](#classUnit)` a,`[`Unit`](#classUnit)` b)` 

Get the euclidean distance from a to b.

#### Parameters
* `a` The first [Unit](#classUnit).

* `b` The second [Unit](#classUnit).

#### Returns
The euclidean distance from a to b.

#### `public double `[`getDistance`](#group__globalFunctions_1gaa79333fe4ec69dc11bffd905431db402)`(`[`Unit`](#classUnit)` a,int32_t bx,int32_t by)` 

Get the euclidean distance from a to b.

#### Parameters
* `a` The first [Unit](#classUnit), or any object with x and y.

* `bx` The x coordinate of point b.

* `by` The y coordinate of point b.

#### Returns
The euclidean distance from a to b, or undefined when a Unit argument has left the game.

#### `public double `[`getDistance`](#group__globalFunctions_1ga099a9099c3a32df0346fa40c496964b9)`(int32_t ax,int32_t ay)` 

Get the euclidean distance from me to a.

#### Parameters
* `ax` The x coordinate of point a.

* `ay` The y coordinate of point a.

#### Returns
The euclidean distance from me to a.

#### `public double `[`getDistance`](#group__globalFunctions_1gaf62a14b80beae3219199c0f05c1ea44b)`(int32_t ax,int32_t ay,`[`Unit`](#classUnit)` b)` 

Get the euclidean distance from a to b.

#### Parameters
* `ax` The x coordinate of point a.

* `ay` The y coordinate of point a.

* `b` The second [Unit](#classUnit), or any object with x and y.

#### Returns
The euclidean distance from a to b, or undefined when a Unit argument has left the game.

#### `public double `[`getDistance`](#group__globalFunctions_1ga5b44bdbe481beae0bd0a5fb322820864)`(int32_t ax,int32_t ay,int32_t bx,int32_t by)` 

Get the euclidean distance from a to b.

#### Parameters
* `ax` The x coordinate of point a.

* `ay` The y coordinate of point a.

* `bx` The x coordinate of point b.

* `by` The y coordinate of point b.

#### Returns
The euclidean distance from a to b.

#### `public void `[`gold`](#group__globalFunctions_1ga43cd766ec6751004726dd28e5f5714d7)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nGold,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode)` 

Drop, offer in a trade, deposit or withdraw gold.

nMode picks what happens to nGold:

| Mode | Action |
|---|---|
| 0, 1 | Drop nGold gold on the ground (1 is the default). |
| 2 | Set the gold you offer in the trade window, capped at the gold you carry; a negative nGold offers all of it. If the other player has already accepted, the offer is not changed: the call sends the same request as un-pressing Accept instead, and the trade state returns to 3. |
| 3 | Move nGold from your inventory into the stash (with the stash open). |
| 4 | Move nGold from the stash into your inventory (with the stash open). |

Any other mode does nothing, and an amount of 0 does nothing except in mode 2. The call checks neither that a trade or the stash is open nor the amount in modes 0, 3 and 4. It first closes the game's own gold dialog; if one was open, the amount typed in it is used instead of nGold.

It returns undefined; outside a ready game it returns false (with a 'Game not ready' warning).

#### Parameters
* `nGold` The amount of gold to move around. Must be an integer in the 32-bit range; any other value (1.5, 1e10, the string "100") is replaced by 0.

* `nMode` What to do with the gold (see the table above). Must be an integer; any other value is replaced by 1.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`checkCollision`](#group__globalFunctions_1gacbe9f546980a44ff675f8c5b77d617d3)`(`[`Unit`](#classUnit)` a,`[`Unit`](#classUnit)` b,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nBitMask)` 

Check if two units collide.

#### Parameters
* `a` [Unit](#classUnit) a.

* `b` [Unit](#classUnit) b.

* `nBitMask` Bit mask to check collision of a and b with.

#### Returns
The collision result for nBitMask, or nBitMask itself (reads as blocked) when either unit has left the game; undefined for wrong arguments.

#### `public bool `[`playSound`](#group__globalFunctions_1gaa759ea651473dc19c7af1bde87f126c9)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSoundId)` 

Play d2 sound by id.

#### Returns
true

#### `public void `[`quit`](#group__globalFunctions_1ga2463a3acef2df4c951ce942a3229e44e)`()` 

Quit the game.

#### `public void `[`quitGame`](#group__globalFunctions_1gaa5cd0b13e9579fbd9b3f538ebae81d99)`()` 

Quit Diablo II.

Allows the core to shutdown, than terminates the process.

#### `public bool `[`say`](#group__globalFunctions_1ga88ff9ae5c22b23bd4de2a7025c3cf107)`(...)` 

Say the string equivalent of the each of the arguments.

#### Returns
true if at least one parameter was sent in.

#### `public bool `[`clickParty`](#group__globalFunctions_1ga40e19c4f23164e0119973be3b66ec581)`(`[`Party`](#classParty)` player,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode)` 

Click one of the buttons in the party screen for the given player.

#### Parameters
* `player` The player who's button you wish to click.

* `nMode` Which button to click.

0 - Allow loot corpse

1 - Hostile player

2 - Join party

3 - Leave party (any player still in the game can be passed in)

#### Returns
true if the button was clicked; false if it doesn't apply, or once the player has left or the game has ended.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`weaponSwitch`](#group__globalFunctions_1gab9c5eb66ccf79108ee26a7f73943d0bc)`(int32_t dummy)` 

Get which weapon switch is being used.

Switch the weapons.

#### Parameters
* `dummy` Any int other than 0.

#### Returns
Which weapon switch is active.

#### Parameters
* `dummy` 0 to use this mode.

#### Returns
true

#### `public void `[`transmute`](#group__globalFunctions_1ga66b26223d5346aaf653562d27cdfe889)`()` 

Hit the transmute button.

#### `public void `[`useStatPoint`](#group__globalFunctions_1ga895a78a9e6fbafc50cc592a81f891bb6)`(uint16_t statType,uint32_t count)` 

Use a stat point.

The points are spent one at a time, half a second apart, and the script waits meanwhile. If you have fewer unspent stat points than count, nothing is spent; count 0 (or a negative count) spends nothing.

BE CAREFUL! The call does not check that statType is valid. statType is converted to a 16-bit number, so a non-numeric value such as "dexterity" becomes 0 (strength) and -1 becomes 65535. The game's own stat button uses the high byte of this value to spend extra points in one packet, so a value above 255 asks for more points than count. Use only 0 to 3; an invalid stat might get you flagged/banned.

It returns undefined; outside a ready game it returns false (with a 'Game not ready' warning).

#### Parameters
* `statType` The type of stat to add the points to: 0 - strength, 1 - energy, 2 - dexterity, 3 - vitality (the same ids getStat uses).

* `count` The number of points to add

#### `public void `[`useSkillPoint`](#group__globalFunctions_1ga2316220fcf3f65233c46a23881f5702e)`(uint16_t skill,uint32_t count)` 

Use a skill point.

The points are spent one at a time, half a second apart, and the script waits meanwhile. If you have fewer unspent skill points than count, nothing is spent; count 0 (or a negative count) spends nothing.

BE CAREFUL! The call does not check that your character can learn the skill. The game's own skill tree checks the required level and the prerequisite skills before it spends a point; this call skips those checks. skill is converted to a 16-bit number, so a non-numeric value becomes 0 (Attack). An invalid skill might get you flagged/banned.

It returns undefined; outside a ready game it returns false (with a 'Game not ready' warning).

#### Parameters
* `skill` The skill to add the points to: a skill id, the same number getSkill uses. See [Skill ids](#skill-ids) in the reference tables. Only class skills can be learned: Amazon 6-35, Sorceress 36-65, Necromancer 66-95, Paladin 96-125, Barbarian 126-155, Druid 221-250, Assassin 251-280, each only by its own class.

* `count` The number points to spend

#### `public void `[`takeScreenshot`](#group__globalFunctions_1gab872a4402833e45312ccdde4004b4f96)`()` 

Take a screenshot.

Performs the action that pressing print screen (by default) would do

#### `public bool `[`moveNPC`](#globalFuncs_8h_1a76152e5a307aa9fab69c1243453e51a7)`(`[`Unit`](#classUnit)` npc,uint32_t x,uint32_t y)` 

Move NPC to a new location.

This is an exploit function.

#### Parameters
* `npc` NPC to move.

* `x` X coord to move to.

* `y` Y coord to move to.

#### Returns
true once the move request is sent; false with a warning when EnableUnsupported is off or the game is not ready. Throws if fewer than three arguments are passed or npc is not a monster Unit. x and y are converted like any JS number.

#### `public bool `[`getPacket`](#globalFuncs_8h_1ae457d3d8c69ab22efeefa2bc07e51d5e)`(...)` 

Pass packets to the client as if the server had sent them.

Takes either one ArrayBuffer holding one or more whole packets, or pairs of arguments: size, value, size, value, ... where each size (1 to 4) is the number of bytes its value (an integer) adds to the packet, little endian. Every packet must be one the game can read, 1 to 516 bytes long and complete. An empty ArrayBuffer is refused. Pass a typed array's `.buffer`, not the typed array; the whole buffer is used, so for a typed array that covers only part of its buffer pass `view.buffer.slice(view.byteOffset, view.byteOffset + view.byteLength)`. The packets are checked together: if one fails, none is passed on.

This is an exploit function.

#### Returns
true when the packets were passed to the game's incoming packet queue; false outside a game, and false with a warning when the input is refused. A value that throws while it is converted stops the call with its error.

#### `public bool `[`sendPacket`](#globalFuncs_8h_1a4082e1ce7c6ac097efd964d6397d197d)`(...)` 

Send a packet to the server.

Takes either one ArrayBuffer holding the packet, or pairs of arguments: size, value, size, value, ... where each size (1 to 4) is the number of bytes its value (an integer) adds to the packet, little endian. The packet must be 1 to 516 bytes long. Pass a typed array's `.buffer`, not the typed array; the whole buffer is used, so for a typed array that covers only part of its buffer pass `view.buffer.slice(view.byteOffset, view.byteOffset + view.byteLength)`.

This is an exploit function.

#### Returns
true when the game took the packet; false when it did not (outside a game, for example, or a `gamepacketsent` listener blocked it, with no warning), and false with a warning when the input is refused. A value that throws while it is converted stops the call with its error.

#### `public String `[`getIP`](#globalFuncs_8h_1a7386e4db6f3be302ddd7a159d66774b1)`()` 

Get the external IP by going to "http://automation.whatismyip.com/n09230945.asp" and parsing the result.

#### `public Object `[`screenToAutomap`](#group__globalFunctions_1ga252bace45245bc8c4794f27d31f8ca45)`(Object arg)` 

Convert a point from screen coordinates to automap coordinates.

#### Parameters
* `arg` An object with x and y parameters specifying the point.

#### Returns
A new object with x and y, containing the automap coordinate. The argument is not changed.

#### `public Object `[`screenToAutomap`](#group__globalFunctions_1gadb899fcbf6b0520f61c6ac2134b7bddd)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` ix,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` iy)` 

Convert a point from screen coordinates to automap coordinates.

#### Parameters
* `ix` The x coordinate.

* `iy` The y coordinate.

#### Returns
An object with x and y, containing the automap coordinate.

#### `public Object `[`automapToScreen`](#group__globalFunctions_1ga8430f6eb530edcbdc6518f0449235cae)`(Object arg)` 

Convert a point from automap coordinates to screen coordinates.

#### Parameters
* `arg` An object with x and y parameters specifying the point.

#### Returns
A new object with x and y, containing the screen coordinate. The argument is not changed.

#### `public Object `[`automapToScreen`](#group__globalFunctions_1ga12b95062f34ffb0b121981df69f6de66)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` ix,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` iy)` 

Convert a point from automap coordinates to screen coordinates.

#### Parameters
* `ix` The x coordinate.

* `iy` The y coordinate.

#### Returns
An object with x and y, containing the screen coordinate.

#### `public String `[`md5`](#group__globalFunctions_1ga5aefa76b292174617e0020a21484ddc3)`(String str)` 

Takes the md5 hash.

#### Parameters
* `str` The string to hash.

#### Returns
The md5 hash of the string.

#### `public String `[`sha1`](#group__globalFunctions_1gab3300abd7aaa5487c52f78452e32926b)`(String str)` 

Takes the sha1 hash.

#### Parameters
* `str` The string to hash.

#### Returns
The sha1 hash of the string.

#### `public String `[`sha256`](#group__globalFunctions_1gacbf583814bf77aa15bf6b10be1bc2921)`(String str)` 

Takes the sha256 hash.

#### Parameters
* `str` The string to hash.

#### Returns
The sha256 hash of the string.

#### `public String `[`sha384`](#group__globalFunctions_1ga16efc8fdeb82a91dde92253fd4f81f6b)`(String str)` 

Takes the sha384 hash.

#### Parameters
* `str` The string to hash.

#### Returns
The sha384 hash of the string.

#### `public String `[`sha512`](#group__globalFunctions_1gafe6cf810a65b58a3685657dfde830367)`(String str)` 

Takes the sha512 hash.

#### Parameters
* `str` The string to hash.

#### Returns
The sha512 hash of the string.

#### `public String `[`md5_file`](#group__globalFunctions_1ga84736bab060acfa890403db4fff9e25f)`(String file)` 

Take the md5 hash of a file.

#### Parameters
* `file` Filename relative to script path.

#### Returns
The md5 hash of the file, or null when the file can't be read.

#### `public String `[`sha1_file`](#group__globalFunctions_1ga2a0768372027c28a64fcb0780f7ff47b)`(String file)` 

Take the sha1 hash of a file.

#### Parameters
* `file` Filename relative to script path.

#### Returns
The sha1 hash of the file, or null when the file can't be read.

#### `public String `[`sha256_file`](#group__globalFunctions_1gacd44b56a400b27db81deb0edd542a81e)`(String file)` 

Take the sha256 hash of a file.

#### Parameters
* `file` Filename relative to script path.

#### Returns
The sha256 hash of the file, or null when the file can't be read.

#### `public String `[`sha384_file`](#group__globalFunctions_1gaec5638f75198f07622c214371a2bbef7)`(String file)` 

Take the sha384 hash of a file.

#### Parameters
* `file` Filename relative to script path.

#### Returns
The sha384 hash of the file, or null when the file can't be read.

#### `public String `[`sha512_file`](#group__globalFunctions_1gad562af84fe5dd55bf5d5172861b2a28f)`(String file)` 

Take the sha512 hash of a file.

#### Parameters
* `file` Filename relative to script path.

#### Returns
The sha512 hash of the file, or null when the file can't be read.

# namespace `ProfileType` 

[ProfileType](#namespaceProfileType) enum.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`enum `[`ProfileType`](#namespaceProfileType_1a52d9eb951de02b75f7ca78be5cfd4f3d)            | 

## Members

#### `enum `[`ProfileType`](#namespaceProfileType_1a52d9eb951de02b75f7ca78be5cfd4f3d) 

 Values                         | Descriptions                                
--------------------------------|---------------------------------------------
singlePlayer            | 
battleNet            | 
openBattleNet            | 
tcpIpHost            | 
tcpIpJoin            | 

# class `Area` 

Represent an [Area](#classArea) of the map. Reading a property waits while an area or act loads. It throws an error while the area is not in the act the player is in (for example after a waypoint to another act) or the game cannot build its rooms, and when the game isn't ready: out of game, when the wait for an area or act load times out, or at once while the game is busy in a blocking listener or while your script holds the game lock.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`Exit`](#classExit)` `[`exits`](#classArea_1a56bbc920b4b5ca70d070a8157824a1de) | Get an array of the [Area](#classArea)'s exits.
`public String `[`name`](#classArea_1add56e46cfd14437bb4dc88fa80a5b90c) | The name of the [Area](#classArea).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classArea_1a7b57ea354fbe94f9480d28e07857e4c3) | The x coordinate of the left side of the map.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`xsize`](#classArea_1ac030cf51b89f3c58b1672b84f57d8573) | The x size (width) of the map.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classArea_1adca338a7f47c9e2cba5f8a4b6cb862a5) | The y coordinate of the bottom (smallest y) of the map.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`ysize`](#classArea_1ade6c62f10571d7318680fe9e5e28496f) | The y size (height) of the map.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`id`](#classArea_1a7871c593963912cebd55208e71b3d16f) | The area id.

## Members

#### `public `[`Exit`](#classExit)` `[`exits`](#classArea_1a56bbc920b4b5ca70d070a8157824a1de) 

Get an array of the [Area](#classArea)'s exits.

This includes arrays where you can walk between two areas and warp [Unit](#classUnit) s.

#### `public String `[`name`](#classArea_1add56e46cfd14437bb4dc88fa80a5b90c) 

The name of the [Area](#classArea).

Its Levels.txt LevelName, which for 25 areas is not the name the game shows ("Rocky Summit" for the in-game Arreat Summit, area 120). See [Area ids](#area-ids) in the reference tables.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classArea_1a7b57ea354fbe94f9480d28e07857e4c3) 

The x coordinate of the left side of the map.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`xsize`](#classArea_1ac030cf51b89f3c58b1672b84f57d8573) 

The x size (width) of the map.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classArea_1adca338a7f47c9e2cba5f8a4b6cb862a5) 

The y coordinate of the bottom (smallest y) of the map.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`ysize`](#classArea_1ade6c62f10571d7318680fe9e5e28496f) 

The y size (height) of the map.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`id`](#classArea_1a7871c593963912cebd55208e71b3d16f) 

The area id.

See [Unit::area](#classUnit_1af5f2e11d4f116084ffd807b578078d7d).

# class `Box` 

A [Box](#classBox) screen hook.

A [Box](#classBox) is a filled rectangle in one palette color, drawn with the draw mode set by `opacity` (see [Box opacity](#box-opacity) in the reference tables). Create one with `new Box(x, y, xsize, ysize, color, opacity, align, automap, click, hover)`; every argument is optional (defaults 0, 0, 0, 0, 0, 0, 0, false, none, none). Coordinates are game-window pixels, x to the right and y down. A [Box](#classBox) made by an out-of-game script draws only at the menus; one made in game (or from the console) draws only in game. With `automap` true, (x, y) and (x + xsize, y + ysize) are game map coordinates (the units of unit.x and unit.y), and the [Box](#classBox) is the screen-aligned rectangle between those two points on the automap: it shows only while the automap is open, and only while xsize is greater than ysize and xsize + ysize is greater than 0 (otherwise nothing is drawn). `automap` is a constructor argument only; there is no automap property.

Hooks of every script and every hook class are drawn in ascending `zorder` (default 1), so a higher value draws on top. Set `visible` to hide or show the [Box](#classBox), and call remove() to delete it. Keep a reference to every hook: one whose object is garbage-collected is removed. When the script that made a hook ends, its hooks leave the screen (with the default `DisableCache=true` in d2bs.ini; with `DisableCache=false` a script that ended normally keeps its hooks drawn, but not clickable, until the script cache is flushed).

Number arguments and number properties take any finite number and drop its fraction (the same result as `value | 0` in script); any other value is ignored and the default or current value stays. `color`, `opacity` and `zorder` keep only the low 16 bits, so -1 reads back 65535. `automap` and `visible` take only true or false; any other value (1 included) is ignored.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classBox_1a180c7a450afc2e8cfe4dddb50eeaa4f2) | The x coordinate of the [Box](#classBox).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classBox_1a1b13c5067471d1052c6f3191f260c785) | The y coordinate (top) of the [Box](#classBox).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`xsize`](#classBox_1a26d7213ca75ae43589b05354efe153d6) | The xsize (width) of the [Box](#classBox).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`ysize`](#classBox_1a81988ee738d5c278c72bb1016addd712) | The ysize (height) of the [Box](#classBox).
`public bool `[`visible`](#classBox_1ae2d7733c544cc0ededa905045062a37d) | Whether or not the [Box](#classBox) is visible.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`align`](#classBox_1a1703dbd7e3fdce5b0fd8f9250cc99ade) | The horizontal alignment.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`zorder`](#classBox_1a3f51df7eadec72ac84ca1bede15ea008) | The z-order of the [Box](#classBox) (what it covers up and is covered by).
`public ClickHandler `[`click`](#classBox_1a660b86951ae4edaa3b4f4e93dc682b62) | The click handler that gets called when the [Box](#classBox) is clicked; null or undefined removes it.
`public HoverHandler `[`hover`](#classBox_1aa6504cde7640e62a2a4853677bfe0d5e) | A hover handler for the [Box](#classBox); kept but never called.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`color`](#classBox_1aa9c08a5f737f77a0d4a6d61192dadeb6) | The color of the [Box](#classBox).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`opacity`](#classBox_1af39fa1fbcc7398a6334531934f85a878) | The draw mode of the [Box](#classBox) (default 0).
`public  `[`Box`](#classBox_1af8e43dea86c924a91316bd0f61099cc3)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x2,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y2,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` color,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` opacity,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` align,bool automap,ClickHandler click,HoverHandler hover)` | Create a [Box](#classBox) hook with the given parameters.
`public void `[`remove`](#classBox_1a6257b65ed22242f4660d3fa357a5ec5b)`()` | Remove the [Box](#classBox) from the screen for good.

## Members

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classBox_1a180c7a450afc2e8cfe4dddb50eeaa4f2) 

The x coordinate of the [Box](#classBox).

It is the left edge with align 0 and the centre with align 2; with align 1 the [Box](#classBox) starts xsize/2 to the right of it. A map coordinate when the [Box](#classBox) was made with automap true. While x or y is -1 the [Box](#classBox) is not drawn.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classBox_1a1b13c5067471d1052c6f3191f260c785) 

The y coordinate (top) of the [Box](#classBox).

A map coordinate when the [Box](#classBox) was made with automap true. While x or y is -1 the [Box](#classBox) is not drawn.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`xsize`](#classBox_1a26d7213ca75ae43589b05354efe153d6) 

The xsize (width) of the [Box](#classBox).

In screen pixels a zero or negative width draws nothing. With automap true it is a map-unit offset (see the class text for when an automap [Box](#classBox) draws).

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`ysize`](#classBox_1a81988ee738d5c278c72bb1016addd712) 

The ysize (height) of the [Box](#classBox).

In screen pixels a zero or negative height draws nothing. With automap true it is a map-unit offset (see the class text for when an automap [Box](#classBox) draws).

#### `public bool `[`visible`](#classBox_1ae2d7733c544cc0ededa905045062a37d) 

Whether or not the [Box](#classBox) is visible.

Defaults to true. A hidden [Box](#classBox) is not drawn and gets no clicks. Only true or false is accepted.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`align`](#classBox_1a1703dbd7e3fdce5b0fd8f9250cc99ade) 

The horizontal alignment.

0 - Left: the [Box](#classBox) starts at x

1 - the [Box](#classBox) is shifted right by half its width (not right-aligned)

2 - Center: the [Box](#classBox) is centred on x

Other values are stored as given and draw as 0. The click area always starts at x, whatever the alignment.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`zorder`](#classBox_1a3f51df7eadec72ac84ca1bede15ea008) 

The z-order of the [Box](#classBox) (what it covers up and is covered by).

Defaults to 1. Hooks are drawn from the lowest zorder to the highest, so a higher value covers a lower one; hooks with equal values draw in no set order. A click reaches every hook under it, whatever its zorder.

#### `public ClickHandler `[`click`](#classBox_1a660b86951ae4edaa3b4f4e93dc682b62) 

The click handler that gets called when the [Box](#classBox) is clicked. Assign null or undefined to remove it; a value that is not a function is ignored.

It is called as click(button, x, y) when a mouse button is pressed over the [Box](#classBox): button 0 left, 1 right, 2 middle; x and y are window pixels. Only a boolean `true` return keeps the press from the game; the release always reaches the game. The handler runs when the owning script next processes events, and the game gets the press anyway if the handler has not answered within 3 seconds. A paused or ended script's hooks get no clicks.

The [Box](#classBox) gets clicks while its `visible` property is true, even when it is not drawn at the moment (other game state, automap closed, x or y of -1). The click area runs from x to x + xsize and y to y + ysize (edges excluded) on the raw values: alignment and the automap conversion are not applied.

#### `public HoverHandler `[`hover`](#classBox_1aa6504cde7640e62a2a4853677bfe0d5e) 

A hover handler for the [Box](#classBox). D2BS keeps it and returns it from `hover`, but never calls it. Assign null or undefined to remove it; a value that is not a function is ignored.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`color`](#classBox_1aa9c08a5f737f77a0d4a6d61192dadeb6) 

The color of the [Box](#classBox).

An index (0-255) into the game's 256-color palette (0 is black). Only the low 8 bits are drawn, so 256 draws as 0, though the property reads back the low 16 bits.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`opacity`](#classBox_1af39fa1fbcc7398a6334531934f85a878) 

The draw mode of the [Box](#classBox) (default 0).

Modes 0-2 blend the color with what is underneath; 3, 4 and 6 ignore the color (3 draws nothing, 4 draws black, 6 darkens what is underneath); 5 and 7 draw the color solid. Only 0-7 are supported; D2BS does not check the value, so use nothing else. See [Box opacity](#box-opacity) in the reference tables.

#### `public  `[`Box`](#classBox_1af8e43dea86c924a91316bd0f61099cc3)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x2,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y2,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` color,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` opacity,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` align,bool automap,ClickHandler click,HoverHandler hover)` 

Create a [Box](#classBox) hook with the given parameters.

#### Parameters
* `x` The x coordinate of the [Box](#classBox): its left edge with align 0, its centre with align 2; with align 1 the [Box](#classBox) starts xsize/2 to the right of x. A map coordinate when automap is true.

* `y` The y coordinate (top) of the [Box](#classBox); a map coordinate when automap is true.

* `x2` The xsize (width) of the [Box](#classBox). In screen pixels a zero or negative width draws nothing. With automap true it is a map-unit offset to the far corner, and the [Box](#classBox) draws only when x2 is greater than y2 and x2 + y2 is greater than 0.

* `y2` The ysize (height) of the [Box](#classBox). In screen pixels a zero or negative height draws nothing. With automap true it is a map-unit offset to the far corner.

* `color` The color of the [Box](#classBox): an index into the game's 256-color palette; only the low 8 bits are drawn.

* `opacity` The draw mode, 0-7 (default 0). See [Box opacity](#box-opacity) in the reference tables.

* `align` The horizontal alignment.

0 - Left

1 - shifted right by half the width (not right-aligned)

2 - Center

#### Parameters
* `automap` true to place the [Box](#classBox) at map coordinates on the automap, where it shows only while the automap is open (see the class text). Only true or false is accepted.

* `click` A function to call when the [Box](#classBox) is clicked; any other value sets no handler.

* `hover` A hover function for the [Box](#classBox), kept but never called; any other value sets no handler.

#### `public void `[`remove`](#classBox_1a6257b65ed22242f4660d3fa357a5ec5b)`()` 

Remove the [Box](#classBox) from the screen for good.

The object stays and its click and hover handlers are released. Its properties then read undefined, writes to them are ignored, and calling remove() again does nothing. Returns undefined.

# class `Control` 

This class represents out of game (OOG) [Control](#classControl) s.

A Control whose control is gone (the screen changed, or the game left the menu) reads every property as undefined and ignores writes; getNext() then returns false and click() and setText() do nothing.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public String `[`text`](#classControl_1ad8a40f413495015a31061fbd9438f7b8) | The text of the [Control](#classControl).
`public double `[`x`](#classControl_1ab67c4321deb0a7ab71cc3ab9bfa60cef) | The x coordinate of the [Control](#classControl).
`public double `[`y`](#classControl_1af0d0d577a0b49e54118be2dbb70857c9) | The y coordinate of the [Control](#classControl).
`public double `[`xsize`](#classControl_1a0319dfc08d4f17914371dc93b55d28b1) | The xsize (width) of the [Control](#classControl).
`public double `[`ysize`](#classControl_1afc787bd3d4d6b176b02333ccd662ceea) | The ysize (height) of the [Control](#classControl).
`public double `[`state`](#classControl_1aeb22d5c21c492b4d08e64acc6181f4f7) | The [Control](#classControl)'s display state: its flag word (the value disabled reads) minus 2, read as an unsigned number.
`public bool `[`password`](#classControl_1aebd2dddca40bfc123cb86b307368e134) | Return whether or not the [Control](#classControl) holds a password (starred out text).
`public double `[`type`](#classControl_1acf59ef5830cbbf3da75babd82f47a93a) | The type of control.
`public double `[`cursorpos`](#classControl_1a46fb8326d65d4a4aef170978e2ce1373) | The position of the text cursor in an edit box.
`public double `[`selectstart`](#classControl_1acfa6b74b35e85f4b806bed082eade450) | On an edit box, the character index of the selection's moving end.
`public double `[`selectend`](#classControl_1a435440d08525dcdf317a55d1b96b1086) | On an edit box, the character index where the current selection began (its fixed end).
`public double `[`disabled`](#classControl_1a8b89abb5d91642620cc5ffe5173a6619) | The [Control](#classControl)'s raw flag word; its bits are listed under state.
`public `[`Control`](#classControl)` `[`getNext`](#classControl_1a4894f8dd6595425524cd468938ed327c)`()` | Gets the next [Control](#classControl) from the linked list.
`public void `[`click`](#classControl_1a399dc8ad5e1e8fb6c4944e448a154c0e)`()` | Click [Control](#classControl) in the center of the [Control](#classControl).
`public void `[`click`](#classControl_1aa6025f9c6027722b94351e24f98c4fa4)`(uint32_t x,uint32_t y)` | Click [Control](#classControl) at the given location.
`public void `[`setText`](#classControl_1a794772a6cde7d2956c68556f5a7065fa)`(String text)` | Set the [Control](#classControl)'s text.
`public String[] `[`getText`](#classControl_1afab7c6e5349104c77cf2f6b91bf1461a)`()` | Get the texts from a label [Control](#classControl).

## Members

#### `public String `[`text`](#classControl_1ad8a40f413495015a31061fbd9438f7b8) 

The text of the [Control](#classControl).

#### `public double `[`x`](#classControl_1ab67c4321deb0a7ab71cc3ab9bfa60cef) 

The x coordinate of the [Control](#classControl).

#### `public double `[`y`](#classControl_1af0d0d577a0b49e54118be2dbb70857c9) 

The y coordinate of the [Control](#classControl).

#### `public double `[`xsize`](#classControl_1a0319dfc08d4f17914371dc93b55d28b1) 

The xsize (width) of the [Control](#classControl).

#### `public double `[`ysize`](#classControl_1afc787bd3d4d6b176b02333ccd662ceea) 

The ysize (height) of the [Control](#classControl).

#### `public double `[`state`](#classControl_1aeb22d5c21c492b4d08e64acc6181f4f7) 

The [Control](#classControl)'s display state: its flag word (the value disabled reads) minus 2, read as an unsigned number.

Common reads: 3 = shown and enabled, 2 = shown but greyed, 11 or 10 = the same on the popup layer. A hidden control reads 1 or 0 (7 or 6 on the popup layer), or 4294967295 or 4294967294 when its flag word is below 2.

The flag word is a set of bits: 0x01 enabled (takes clicks; a button without it is drawn greyed), 0x04 shown (drawn and clickable), 0x08 popup layer (drawn after every other control; popups have it), and 0x02, a transient bit the game sets while a button is being released.

Setting 0-3 replaces the whole flag word with state + 2: 3 shows and enables the control, 2 shows it greyed, and 1 and 0 hide it. Setting it also removes the popup-layer bit. States 0 and 1 also set a transient bit the game uses while a button is being released, with effects that are not fully known, so prefer 2 and 3. Any other 32-bit integer throws "Invalid state value"; a non-integer value, or an integer outside the signed 32-bit range, is ignored.

Reads undefined once the control is gone.

#### `public bool `[`password`](#classControl_1aebd2dddca40bfc123cb86b307368e134) 

Return whether or not the [Control](#classControl) holds a password (starred out text).

#### `public double `[`type`](#classControl_1acf59ef5830cbbf3da75babd82f47a93a) 

The type of control.

| Value | Type | Notes |
|---|---|---|
| 1 | Edit box | Single-line text input (account, password, game name); its text is writable |
| 2 | Image | Image or picture |
| 3 | (unused) | Never constructed in 1.14d |
| 4 | Text box | Multi-line text or label box; read it with getText() |
| 5 | Scroll bar | |
| 6 | Button | |
| 7 | List | List box |
| 8 | Timer | Invisible timer control |
| 9 | Video | Smacker video player |
| 10 | Progress bar | |
| 11 | Popup | |
| 12 | Account list | |

#### `public double `[`cursorpos`](#classControl_1a46fb8326d65d4a4aef170978e2ce1373) 

The position of the text cursor in an edit box. Reading it returns the current position, and setting it stores a new one. On any other control type it reads as undefined and ignores writes.

#### `public double `[`selectstart`](#classControl_1acfa6b74b35e85f4b806bed082eade450) 

On an edit box, the character index of the selection's moving end.

It follows the cursor while the selection is extended with Shift or the mouse. Despite its name it is not where the selection began; that is selectend. The selection is the text between selectstart and selectend, in whichever order they fall; nothing is selected when selectend is 4294967295 or equals selectstart. It reads 0 once the selection is cleared (moving the cursor with the keyboard without Shift clears it), so it is meaningful only while a selection exists.

Read-only. On other control types the value has no selection meaning. Reads undefined once the control is gone.

#### `public double `[`selectend`](#classControl_1a435440d08525dcdf317a55d1b96b1086) 

On an edit box, the character index where the current selection began (its fixed end).

Reads 4294967295 when no selection has been started. Nothing is selected either when selectend equals selectstart. The selection is the text between selectend and selectstart, in whichever order they fall. Select all gives selectend 0 and selectstart equal to the text length.

Read-only. On other control types the value has no selection meaning. Reads undefined once the control is gone.

#### `public double `[`disabled`](#classControl_1a8b89abb5d91642620cc5ffe5173a6619) 

The [Control](#classControl)'s raw flag word; its bits are listed under state. Despite its name it is not a boolean: a shown, enabled control reads 5 and a shown, greyed one reads 4 (13 and 12 on the popup layer). Read-only: an assignment is ignored, and throws a TypeError in strict code. Reads undefined once the control is gone.

#### `public `[`Control`](#classControl)` `[`getNext`](#classControl_1a4894f8dd6595425524cd468938ed327c)`()` 

Gets the next [Control](#classControl) from the linked list.

This is done by finding the old [Control](#classControl) based on type, location and size.

#### `public void `[`click`](#classControl_1a399dc8ad5e1e8fb6c4944e448a154c0e)`()` 

Click [Control](#classControl) in the center of the [Control](#classControl).

#### `public void `[`click`](#classControl_1aa6025f9c6027722b94351e24f98c4fa4)`(uint32_t x,uint32_t y)` 

Click [Control](#classControl) at the given location.

This is equivalent with clicking at the given location.

#### Parameters
* `x` The x coordinate of the point to click.

* `y` The y coordinate of the point to click.

#### `public void `[`setText`](#classControl_1a794772a6cde7d2956c68556f5a7065fa)`(String text)` 

Set the [Control](#classControl)'s text.

#### Parameters
* `text` The text to set the [Control](#classControl)'s text field to.

Throws "Invalid arguments" if no text is passed to a control that is still on screen.

#### `public String[] `[`getText`](#classControl_1afab7c6e5349104c77cf2f6b91bf1461a)`()` 

Get the texts from a label [Control](#classControl).

Only works for labels.

#### Returns
An array of all the [Control](#classControl)'s texts.

# class `D2BSScript` 

Represents a script (a thread), created by either loading a file, or typing something in the console.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public String `[`name`](#classD2BSScript_1ac7b3b72a5f100891f7e194a3cd4dce27) | The relative filename of the script.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`type`](#classD2BSScript_1aee04c264935a168134c967506123d49c) | The type of script.
`public bool `[`running`](#classD2BSScript_1a384dc30cf52c68c260aa77f7ae9d8708) | Whether or not the script is running.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`threadid`](#classD2BSScript_1a85c1a8f5ec4bef0563d07209a1f1d2a1) | The thread id.
`public bool `[`getNext`](#classD2BSScript_1a9f47bf9a5ebd40e20561a133f9594b7b)`()` | Get the next script.
`public void `[`pause`](#classD2BSScript_1af541fc2916b40774e2af4eaf947b0c85)`()` | Pause the script.
`public void `[`resume`](#classD2BSScript_1ae16e135f6467d23dfb8a67582c817485)`()` | Resume the script.
`public void `[`join`](#classD2BSScript_1a943c172af9bbbc4eedecb74653e1d146)`()` | Wait for the script to finish.
`public void `[`stop`](#classD2BSScript_1ab4933bbbe30752c142be6df995226967)`()` | Stop the script.
`public void `[`send`](#classD2BSScript_1ab615c8319b0575b135a8f127a3cf61cf)`(...)` | Send some values to the script.

## Members

#### `public String `[`name`](#classD2BSScript_1ac7b3b72a5f100891f7e194a3cd4dce27) 

The relative filename of the script.

If the script is from the console the filename will be reported as "Command Line".

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`type`](#classD2BSScript_1aee04c264935a168134c967506123d49c) 

The type of script.

0 - InGame

1 - OutOfGame or Command

#### `public bool `[`running`](#classD2BSScript_1a384dc30cf52c68c260aa77f7ae9d8708) 

Whether or not the script is running.

False before the script has started (while it compiles and, after a reload, while it waits up to half a second for the old copy to end), while it is paused, after stop(), and after it has ended. A stopped or ended script stays listed until its thread ends (with DisableCache=false, until the cache is flushed or the file is loaded again). Once it is no longer listed, its name is an empty string, running stays false, its other members are undefined and its methods do nothing.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`threadid`](#classD2BSScript_1a85c1a8f5ec4bef0563d07209a1f1d2a1) 

The thread id.

#### `public bool `[`getNext`](#classD2BSScript_1a9f47bf9a5ebd40e20561a133f9594b7b)`()` 

Get the next script.

#### Returns
Whether there was another script or not.

#### `public void `[`pause`](#classD2BSScript_1af541fc2916b40774e2af4eaf947b0c85)`()` 

Pause the script.

Messages sent to it with send(), and its setTimeout/setInterval callbacks, wait until resume() (or, in game, run while it waits inside a game call such as a skill change); each pending timer runs once then, however many periods passed. Game events that arrive while it is paused are not delivered.

#### `public void `[`resume`](#classD2BSScript_1ae16e135f6467d23dfb8a67582c817485)`()` 

Resume the script.

#### `public void `[`join`](#classD2BSScript_1a943c172af9bbbc4eedecb74653e1d146)`()` 

Wait for the script to finish.

Returns at once when it has already ended. Throws an error when called on the calling script itself.

#### `public void `[`stop`](#classD2BSScript_1ab4933bbbe30752c142be6df995226967)`()` 

Stop the script. D2BS prints a line such as `Script default.dbj ended` once for each script other than the console, when it finishes or is stopped (a script that fails to start prints "failed to start" instead); stop() on a script that has already ended prints no second line.

#### `public void `[`send`](#classD2BSScript_1ab615c8319b0575b135a8f127a3cf61cf)`(...)` 

Send some values to the script.

Has the effect of calling any listeners to scriptmsg with the parameters provided. The message reaches a script that is running or paused and has a scriptmsg listener, or one that has not started yet (just loaded, or waiting for the copy it replaces to end). A script that has not started usually gets the message at its first delay() or event, so a listener its top level adds before then receives it; an engine interrupt during its top level (a garbage collection, for example) can deliver it earlier, before such a listener exists. A paused script gets it once it resumes (or, in game, while it waits inside a game call such as a skill change). Nothing is copied or sent when the script has ended or is stopping, or has started (running or paused) without a scriptmsg listener. A value that can't be copied (a function, or a game object such as a Unit) throws an error and nothing is sent.

# class `DBStatement` 

A prepared SQL statement on an SQLite database.

SQLite.query(sql, ...) creates one, binding any extra arguments to its parameters in order, and SQLite.statements lists the ones still open; a [DBStatement](#classDBStatement) cannot be constructed directly (`new DBStatement()` throws "Invalid Operation"). query() prepares only the first SQL statement in the string, and throws "Statement has no effect" for an empty one.

Step through the result rows with [next()](#classDBStatement_1a571c59f80ee8940aaedd2b6f6abc9bd4), which returns true while a row is ready and false at the end, and read the current row with [getObject()](#classDBStatement_1ad4f6860effd1caa57653e8c1fd3dd4c4), getColumnValue() or [getColumnName()](#classDBStatement_1a849c6076cb4e5e4a7436aebb3ca7dee7). Calling next() again after it returned false runs the statement again from its first row. Use [go()](#classDBStatement_1a0d92705f6226d3bfea6a1d99d797e7be) instead to run a statement such as an INSERT or UPDATE once: it returns true when the statement ran to completion, and false when its step produced a row (a SELECT run with go() returns false when it finds a row and true when it finds none; either way the statement is closed after that one step). bind() sets a parameter (1-based index, or a name including its prefix such as ":id"), [skip()](#classDBStatement_1a52b6771be336a56bbc8f02bcb99e55c7) steps over rows and [reset()](#classDBStatement_1a7c08cde3f29a7e5158e4ff290f9c6e25) rewinds the statement so it can run again.

The statement is closed after [close()](#classDBStatement_1a1d8293b4debace6e99c0dbede2677ee7), after a [go()](#classDBStatement_1a0d92705f6226d3bfea6a1d99d797e7be) that returns, and after the database's close(): every method except close() throws "The statement is closed", `sql` reads undefined and `ready` reads false. When go()'s step reports an error, go() throws and the statement stays open, not ready.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public String `[`sql`](#classDBStatement_1ac1de9e1a1ad1938c08682caac8984dad) | The statement string, or undefined once the statement is closed.
`public bool `[`ready`](#classDBStatement_1a254934b06af2d3a12349add04f338c2e) | Whether a row can be read now; false once the statement is closed.
`public Object `[`getObject`](#classDBStatement_1ad4f6860effd1caa57653e8c1fd3dd4c4)`()` | An object with property names from the column names and property values coming from the data.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getColumnCount`](#classDBStatement_1acd872f8af18dccfa9e6a65d0e7451d25)`()` | Get the column count.
`public double `[`getColumnValue`](#classDBStatement_1a186d710e040e011b31ab49add001cdda)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` colIndex)` | Get the value from the column specified by index.
`public String `[`getColumnValue`](#classDBStatement_1aa177441f9b780c947e1522a07ffdf666)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` colIndex)` | Get the value from the column specified by index.
`public  `[`getColumnName`](#classDBStatement_1a849c6076cb4e5e4a7436aebb3ca7dee7)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` colIndex)` | Get the name of the column at the given index.
`public bool `[`go`](#classDBStatement_1a0d92705f6226d3bfea6a1d99d797e7be)`()` | Call sqlite3_step and close_db_stmt on the statement.
`public bool `[`next`](#classDBStatement_1a571c59f80ee8940aaedd2b6f6abc9bd4)`()` | Call sqlite3_step on the statement.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`skip`](#classDBStatement_1a52b6771be336a56bbc8f02bcb99e55c7)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` rows)` | Skip a certain number of rows in the results.
`public bool `[`reset`](#classDBStatement_1a7c08cde3f29a7e5158e4ff290f9c6e25)`()` | Call sqlite3_reset.
`public bool `[`close`](#classDBStatement_1a1d8293b4debace6e99c0dbede2677ee7)`()` | Close the statement.
`public bool `[`bind`](#classDBStatement_1aa7f4478203eaa27a68d33fa18a1ab9b4)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` colnum,double val)` | Bind the given data to the given parameter.
`public bool `[`bind`](#classDBStatement_1a289bdcf455c3d15c1defc949fed8ff8b)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` colnum,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` val)` | Bind the given data to the given parameter.
`public bool `[`bind`](#classDBStatement_1ab817aa527c6c877e4bcc0410080b9b82)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` colnum,String val)` | Bind the given data to the given parameter.
`public bool `[`bind`](#classDBStatement_1a42ddfab63e2f5438e6a971bf96e6cdab)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` colnum,bool val)` | Bind the given boolean to the given parameter as the integer 1 (true) or 0 (false).

## Members

#### `public String `[`sql`](#classDBStatement_1ac1de9e1a1ad1938c08682caac8984dad) 

The statement string, or undefined once the statement is closed.

#### `public bool `[`ready`](#classDBStatement_1a254934b06af2d3a12349add04f338c2e) 

Whether a row can be read now; false once the statement is closed.

#### `public Object `[`getObject`](#classDBStatement_1ad4f6860effd1caa57653e8c1fd3dd4c4)`()` 

An object with property names from the column names and property values coming from the data. The same object is returned until the statement moves to another row or closes.

#### Returns
The row object, or null when the statement is not ready.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getColumnCount`](#classDBStatement_1acd872f8af18dccfa9e6a65d0e7451d25)`()` 

Get the column count.

#### Returns
The column count.

#### `public double `[`getColumnValue`](#classDBStatement_1a186d710e040e011b31ab49add001cdda)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` colIndex)` 

Get the value from the column specified by index.

#### Parameters
* `colIndex` The index of the column to get data from.

Throws "Invalid column index" if colIndex is not a column of the statement.

#### Returns
The data.

#### `public String `[`getColumnValue`](#classDBStatement_1aa177441f9b780c947e1522a07ffdf666)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` colIndex)` 

Get the value from the column specified by index.

#### Parameters
* `colIndex` The index of the column to get data from.

Throws "Invalid column index" if colIndex is not a column of the statement.

#### Returns
The data.

#### `public  `[`getColumnName`](#classDBStatement_1a849c6076cb4e5e4a7436aebb3ca7dee7)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` colIndex)` 

Get the name of the column at the given index.

#### Parameters
* `colIndex` The index of the column to get data from.

Throws "Invalid column index" if colIndex is not a column of the statement.

#### Returns
The column name.

#### `public bool `[`go`](#classDBStatement_1a0d92705f6226d3bfea6a1d99d797e7be)`()` 

Call sqlite3_step and close_db_stmt on the statement.

Runs the statement once and closes it: ready is false afterwards and the statement can't be stepped again. When the step reports an error, go() throws and the statement stays open, not ready.

#### Returns
true when the statement ran to completion; false when its step produced a row.

#### `public bool `[`next`](#classDBStatement_1a571c59f80ee8940aaedd2b6f6abc9bd4)`()` 

Call sqlite3_step on the statement.

Moves to the next row of the results; ready tells whether there is one.

#### Returns
true when a row is ready; false at the end of the results. A further call after false runs the statement again from its first row.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`skip`](#classDBStatement_1a52b6771be336a56bbc8f02bcb99e55c7)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` rows)` 

Skip a certain number of rows in the results. Stops at the end of the results; ready is true when it stopped on a row. A count of 0 or less skips nothing.

#### Parameters
* `rows` The number of rows to skip.

#### Returns
The number of rows actually skipped, which is less than rows at the end of the results.

#### `public bool `[`reset`](#classDBStatement_1a7c08cde3f29a7e5158e4ff290f9c6e25)`()` 

Call sqlite3_reset.

Starts the results again from the first row; ready is false until next() or skip() lands on a row.

#### Returns
true. After a failed next(), skip() or go() it throws that step's error; the statement is still rewound and not ready.

#### `public bool `[`close`](#classDBStatement_1a1d8293b4debace6e99c0dbede2677ee7)`()` 

Close the statement.

#### Returns
true, also when the statement was already closed.

#### `public bool `[`bind`](#classDBStatement_1aa7f4478203eaa27a68d33fa18a1ab9b4)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` colnum,double val)` 

Bind the given data to the given parameter.

#### Parameters
* `colnum` The parameter to bind to: its 1-based number, or its name including the prefix (for example ":id"). An unknown name or 0 throws "Invalid parameter number, parameters start at 1".

* `val` The data to bind.

#### Returns
true when the value was bound. Throws when colnum is out of range, when val is an object or a function, or when the statement has been stepped with next() or skip() since it was created or last reset().

#### `public bool `[`bind`](#classDBStatement_1a289bdcf455c3d15c1defc949fed8ff8b)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` colnum,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` val)` 

Bind the given data to the given parameter.

#### Parameters
* `colnum` The parameter to bind to: its 1-based number, or its name including the prefix (for example ":id"). An unknown name or 0 throws "Invalid parameter number, parameters start at 1".

* `val` The data to bind.

#### Returns
true when the value was bound. Throws when colnum is out of range, when val is an object or a function, or when the statement has been stepped with next() or skip() since it was created or last reset().

#### `public bool `[`bind`](#classDBStatement_1ab817aa527c6c877e4bcc0410080b9b82)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` colnum,String val)` 

Bind the given data to the given parameter.

#### Parameters
* `colnum` The parameter to bind to: its 1-based number, or its name including the prefix (for example ":id"). An unknown name or 0 throws "Invalid parameter number, parameters start at 1".

* `val` The data to bind.

#### Returns
true when the value was bound. Throws when colnum is out of range, when val is an object or a function, or when the statement has been stepped with next() or skip() since it was created or last reset().

#### `public bool `[`bind`](#classDBStatement_1a42ddfab63e2f5438e6a971bf96e6cdab)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` colnum,bool val)` 

Bind the given boolean to the given parameter as the integer 1 (true) or 0 (false). A bind() of null or undefined binds NULL.

#### Parameters
* `colnum` The parameter to bind to: its 1-based number, or its name including the prefix (for example ":id"). An unknown name or 0 throws "Invalid parameter number, parameters start at 1".

* `val` The data to bind.

#### Returns
true when the value was bound. Throws when colnum is out of range, when val is an object or a function, or when the statement has been stepped with next() or skip() since it was created or last reset().

# class `Exit` 

Represents a method of getting from one [Area](#classArea) to another.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classExit_1ad611f082650a2cb50c55e45a22aeea77) | The x coordinate of the exit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classExit_1a46d25b3b696f875747de1f4ddfbbf1a5) | The y coordinate of the exit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`target`](#classExit_1a6980668d676c2c6c4c465f05a65fbe20) | The [Area](#classArea) ID of the target level.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`type`](#classExit_1ad79ae2186aea695de8bba1cb25147b6b) | The [Exit](#classExit) type.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`tileid`](#classExit_1a173070014dcf8334a334f8005035d8b5) | The class ID of the [Exit](#classExit) if it is a stairs type [Exit](#classExit), 0 otherwise.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`level`](#classExit_1ad626cb965b1345961c7c98808a5c4cd9) | The [Area](#classArea) ID of the source side of the [Exit](#classExit).

## Members

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classExit_1ad611f082650a2cb50c55e45a22aeea77) 

The x coordinate of the exit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classExit_1a46d25b3b696f875747de1f4ddfbbf1a5) 

The y coordinate of the exit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`target`](#classExit_1a6980668d676c2c6c4c465f05a65fbe20) 

The [Area](#classArea) ID of the target level.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`type`](#classExit_1ad79ae2186aea695de8bba1cb25147b6b) 

The [Exit](#classExit) type.

1 - A walk through [Exit](#classExit)

2 - A stairs type [Exit](#classExit)

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`tileid`](#classExit_1a173070014dcf8334a334f8005035d8b5) 

The class ID of the [Exit](#classExit) if it is a stairs type [Exit](#classExit), 0 otherwise.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`level`](#classExit_1ad626cb965b1345961c7c98808a5c4cd9) 

The [Area](#classArea) ID of the source side of the [Exit](#classExit).

# class `File` 

This class handles the opening, reading and writing of files.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public bool `[`readable`](#classFile_1a0415c7cad271ddfd0931223c210f1ddd) | Whether or not the file can be read from.
`public bool `[`writeable`](#classFile_1ae27c16eecaebba584c6edbfdfcc82a98) | Whether or not the file is writeable.
`public bool `[`seekable`](#classFile_1a28141c09589502aa71c4bb8eca9dd2d8) | Whether or not the file is seekable.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`mode`](#classFile_1ad69e4f31f332da4ce62d4dbbf905fc3b) | The mode the file was opened with.
`public bool `[`binaryMode`](#classFile_1aa14287c782b3732a65678c84ca9fa20d) | Whether the file was opened as a binary file or not.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`length`](#classFile_1a29b6bf8632786b651f99746906cefafb) | The file length.
`public String `[`path`](#classFile_1a3c225ed52bdd0630736490e8ffd47dab) | The relative path to the [File](#classFile) (the filename/path given for opening).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`position`](#classFile_1a336836eda3d033e4005283f2f1604cb3) | The position in the [File](#classFile) (where the next read or write will occur).
`public bool `[`eof`](#classFile_1a391472ffe19feb34d5e88325bdf17f23) | Whether or not the [File](#classFile) is at end of file.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`accessed`](#classFile_1a5a63aa7adeaaffddbd52b7c15aea8c87) | The last accessed time of the [File](#classFile).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`created`](#classFile_1a7ff0ed452a129324a9a5fcbee803a373) | The creation time of the [File](#classFile).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`modified`](#classFile_1af946c3c5984c1e0580db67bd80c24215) | The last modified time of the [File](#classFile).
`public bool `[`autoflush`](#classFile_1abb41241dc0a11a7725345da82c460496) | Whether or not to autoflush the [File](#classFile).
`public `[`File`](#classFile)` `[`close`](#classFile_1ab8f676c747765ad97655928f1c2baf04)`()` | Close the current file and then return it.
`public `[`File`](#classFile)` `[`reopen`](#classFile_1a6fe4ccb2fea82b46e091aa7c46df4dbe)`()` | Reopen the file, equivalent to calling open with the same parameters as were used the first time.
`public String `[`read`](#classFile_1a2a09f2249cb161ee348e9198b054df0d)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` count)` | Read count chars from the file.
`public int[] `[`read`](#classFile_1a8b86be0df9dca624871c726b2eb96a44)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` count)` | Read count 32-bit integers from the file.
`public String `[`readLine`](#classFile_1a1f9845e5268c8d9ac944235f99538320)`()` | Read a line from the file and return it.
`public String[] `[`readAllLines`](#classFile_1a2da4d7f22372013563a6d2301c3217bb)`()` | Read all the lines from the file into a String array.
`public String `[`readAll`](#classFile_1a59de24f17a4fce1bdffb20d63ed63bc7)`()` | Read the entire file and return as a String.
`public `[`File`](#classFile)` `[`write`](#classFile_1a90860f54a694ad17207855c4ff0c6053)`(...)` | Write the string representation (for standard files) or the binary representation (for binary files) of each of the arguments to the [File](#classFile).
`public `[`File`](#classFile)` `[`seek`](#classFile_1ad3b8e538df678492e7f5e0330c613809)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` n)` | Seek n bytes from the current location.
`public `[`File`](#classFile)` `[`seek`](#classFile_1ad88c69806c5f39b110cc8277cb7dc7f6)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` n,bool isLines,bool fromStart)` | Seek n bytes or lines (depending on isLines) from the current location or the start of file (depending on fromStart).
`public `[`File`](#classFile)` `[`flush`](#classFile_1a53a7ee7193e77c0fe8c139f0965e3720)`()` | Flush the buffers for the [File](#classFile).
`public `[`File`](#classFile)` `[`reset`](#classFile_1ae367f8c9c2b0fa514d9a15feea63cfe5)`()` | Seek back to the beginning of the [File](#classFile).
`public `[`File`](#classFile)` `[`end`](#classFile_1ae1809bb15e04ae6085b444f3e8fbc943)`()` | Seek to the end of the [File](#classFile).

## Members

#### `public bool `[`readable`](#classFile_1a0415c7cad271ddfd0931223c210f1ddd) 

Whether or not the file can be read from.

Based on whether the [File](#classFile) is open, whether it is at the end of file, and whether there is an error pending.

#### `public bool `[`writeable`](#classFile_1ae27c16eecaebba584c6edbfdfcc82a98) 

Whether or not the file is writeable.

Based on whether the [File](#classFile) is open, whether there is an error pending and whether the [File](#classFile) was opened for writing or not.

#### `public bool `[`seekable`](#classFile_1a28141c09589502aa71c4bb8eca9dd2d8) 

Whether or not the file is seekable.

Based on whether the [File](#classFile) is open and whether there is an error pending.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`mode`](#classFile_1ad69e4f31f332da4ce62d4dbbf905fc3b) 

The mode the file was opened with.

Compare with FILE_READ, FILE_WRITE and FILE_APPEND.

#### `public bool `[`binaryMode`](#classFile_1aa14287c782b3732a65678c84ca9fa20d) 

Whether the file was opened as a binary file or not.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`length`](#classFile_1a29b6bf8632786b651f99746906cefafb) 

The file length.

#### `public String `[`path`](#classFile_1a3c225ed52bdd0630736490e8ffd47dab) 

The relative path to the [File](#classFile) (the filename/path given for opening).

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`position`](#classFile_1a336836eda3d033e4005283f2f1604cb3) 

The position in the [File](#classFile) (where the next read or write will occur).

#### `public bool `[`eof`](#classFile_1a391472ffe19feb34d5e88325bdf17f23) 

Whether or not the [File](#classFile) is at end of file.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`accessed`](#classFile_1a5a63aa7adeaaffddbd52b7c15aea8c87) 

The last accessed time of the [File](#classFile).

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`created`](#classFile_1a7ff0ed452a129324a9a5fcbee803a373) 

The creation time of the [File](#classFile).

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`modified`](#classFile_1af946c3c5984c1e0580db67bd80c24215) 

The last modified time of the [File](#classFile).

#### `public bool `[`autoflush`](#classFile_1abb41241dc0a11a7725345da82c460496) 

Whether or not to autoflush the [File](#classFile).

#### `public `[`File`](#classFile)` `[`close`](#classFile_1ab8f676c747765ad97655928f1c2baf04)`()` 

Close the current file and then return it.

#### Returns
The same [File](#classFile) object.

#### `public `[`File`](#classFile)` `[`reopen`](#classFile_1a6fe4ccb2fea82b46e091aa7c46df4dbe)`()` 

Reopen the file, equivalent to calling open with the same parameters as were used the first time.

#### Returns
The same [File](#classFile) object.

#### `public String `[`read`](#classFile_1a2a09f2249cb161ee348e9198b054df0d)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` count)` 

Read count chars from the file.

Used when the file was not opened in binary mode.

#### Parameters
* `count` The number of characters to read.

#### Returns
The characters that were read. Throws if the file is not open (it was closed).

#### `public int[] `[`read`](#classFile_1a8b86be0df9dca624871c726b2eb96a44)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` count)` 

Read count 32-bit integers from the file.

Used when the file was opened in binary mode.

#### Parameters
* `count` The number of 4-byte little-endian integers to read.

#### Returns
One integer when count is 1 (0 at the end of the file), otherwise an array of the integers read, which is shorter than count at the end of the file. Binary write stores integers as 4 bytes but other numbers as 8-byte doubles and booleans as 1 byte, so only integers read back unchanged. Throws if the file is not open (it was closed).

#### `public String `[`readLine`](#classFile_1a1f9845e5268c8d9ac944235f99538320)`()` 

Read a line from the file and return it.

#### Returns
The line. Throws if the file is not open (it was closed).

#### `public String[] `[`readAllLines`](#classFile_1a2da4d7f22372013563a6d2301c3217bb)`()` 

Read all the lines from the file into a String array.

#### Returns
An array of all the lines in the file. Throws if the file is not open (it was closed).

#### `public String `[`readAll`](#classFile_1a59de24f17a4fce1bdffb20d63ed63bc7)`()` 

Read the entire file and return as a String.

Includes a seek to start from the beginning of the file, regardless of where the current position was.

#### Returns
The entire file in a String. Throws if the file is 256 MB or larger or is not open (it was closed).

#### `public `[`File`](#classFile)` `[`write`](#classFile_1a90860f54a694ad17207855c4ff0c6053)`(...)` 

Write the string representation (for standard files) or the binary representation (for binary files) of each of the arguments to the [File](#classFile).

#### Returns
The [File](#classFile).

#### `public `[`File`](#classFile)` `[`seek`](#classFile_1ad3b8e538df678492e7f5e0330c613809)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` n)` 

Seek n bytes from the current location.

#### Parameters
* `n` Number of bytes to move..

#### Returns
The [File](#classFile).

#### `public `[`File`](#classFile)` `[`seek`](#classFile_1ad88c69806c5f39b110cc8277cb7dc7f6)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` n,bool isLines,bool fromStart)` 

Seek n bytes or lines (depending on isLines) from the current location or the start of file (depending on fromStart).

#### Parameters
* `n` Number of bytes or lines to move.

* `isLines` Whether n is a number of lines, or just bytes.

* `fromStart` Whether to start from the start of file, or the current location.

#### Returns
The [File](#classFile).

#### `public `[`File`](#classFile)` `[`flush`](#classFile_1a53a7ee7193e77c0fe8c139f0965e3720)`()` 

Flush the buffers for the [File](#classFile).

#### Returns
The [File](#classFile).

#### `public `[`File`](#classFile)` `[`reset`](#classFile_1ae367f8c9c2b0fa514d9a15feea63cfe5)`()` 

Seek back to the beginning of the [File](#classFile).

#### Returns
The [File](#classFile).

#### `public `[`File`](#classFile)` `[`end`](#classFile_1ae1809bb15e04ae6085b444f3e8fbc943)`()` 

Seek to the end of the [File](#classFile).

#### Returns
The [File](#classFile).

# class `FileTools` 

A collection of tools to do things like move files and determine whether files exist.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public bool `[`writeText`](#classFileTools_1a6d7dea2b6426a01de14576827af4c606)`(String file,...)` | Write the string representation (for standard files) or the binary representation (for binary files) of each of the arguments to the file.
`public bool `[`appendText`](#classFileTools_1a4f2c1b01653f22bb40d66d8cc7c16c6f)`(String file,...)` | Append the string representation (for standard files) or the binary representation (for binary files) of each of the arguments to the file.
`public String readText(String file)` | Read the contents of a file into a String; throws if the file is 256 MB or larger.
`public bool copy(String orig,String newName,bool overwrite)` | Copy a file byte for byte; an existing newName is replaced only when overwrite is true.
`public bool remove(String file)` | Delete a file.
`public bool rename(String orig,String newName)` | Rename a file; an existing newName is not replaced.

## Members

#### `public bool `[`writeText`](#classFileTools_1a6d7dea2b6426a01de14576827af4c606)`(String file,...)` 

Write the string representation (for standard files) or the binary representation (for binary files) of each of the arguments to the file.

#### Parameters
* `file` The filename of the file to write to.

#### Returns
Whether or not the operation was successful.

#### `public bool `[`appendText`](#classFileTools_1a4f2c1b01653f22bb40d66d8cc7c16c6f)`(String file,...)` 

Append the string representation (for standard files) or the binary representation (for binary files) of each of the arguments to the file.

#### Parameters
* `file` The filename of the file to write to.

#### Returns
Whether or not the operation was successful.

#### `public String readText(String file)` 

Read the contents of a file into a String.

#### Parameters
* `file` The filename of the file to read.

#### Returns
The contents of the file. Throws if the file is 256 MB or larger.

#### `public bool copy(String orig,String newName,bool overwrite)` 

Copy a file byte for byte.

#### Parameters
* `orig` The original filename.
* `newName` The filename of the new file.
* `overwrite` Replace newName if it already exists. Defaults to false.

#### Returns
true if the copy was made. false if it was not: the source is missing, newName exists and overwrite is not true, the two names are the same file, or reading or writing failed (no partial file is left). With overwrite true, a copy that fails while reading or writing also leaves no newName (the old file was already replaced). An invalid file name throws.

#### `public bool remove(String file)` 

Delete a file.

#### Returns
true if the file was deleted, false if it could not be (for example it does not exist). An invalid file name throws.

#### `public bool rename(String orig,String newName)` 

Rename a file.

#### Returns
true if the file was renamed. false if it was not, for example when orig is missing or newName already exists. An invalid file name throws.

# class `Folder` 

Represents a folder.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public String `[`name`](#classFolder_1a1b7bc0e644f873164b1d9c17085cf178) | The path name to the directory.
`public `[`Folder`](#classFolder)` `[`create`](#classFolder_1a3a0bac848c1e110eb40dde3cd48393b8)`(String name)` | Create a new directory.
`public bool `[`remove`](#classFolder_1aca893daac6c6747895d50987cf9cf34c)`()` | Remove the directory.
`public String[] `[`getFiles`](#classFolder_1a811c76e8bd576486d01311bde02a79b3)`(String search)` | Get the filename for each of the files in the directory.
`public String[] `[`getFolders`](#classFolder_1ad738dfd1f4c322347a1caa549d4b3eec)`(String search)` | Get the name for each of the sub-directories in the current directory.

## Members

#### `public String `[`name`](#classFolder_1a1b7bc0e644f873164b1d9c17085cf178) 

The path name to the directory.

#### `public `[`Folder`](#classFolder)` `[`create`](#classFolder_1a3a0bac848c1e110eb40dde3cd48393b8)`(String name)` 

Create a new directory.

#### Parameters
* `name` The name of the directory to create under the current directory.

Throws "No path passed to dir.create()" if name is not a string, and "Invalid folder name" if it is invalid or too long.

#### Returns
The new [Folder](#classFolder) created.

#### `public bool `[`remove`](#classFolder_1aca893daac6c6747895d50987cf9cf34c)`()` 

Remove the directory.

Must be empty to work properly.

#### Returns
true.

#### `public String[] `[`getFiles`](#classFolder_1a811c76e8bd576486d01311bde02a79b3)`(String search)` 

Get the filename for each of the files in the directory.

#### Parameters
* `search` Optional wildcard pattern for names in this folder (default `*.*`). It can't contain a backslash, a slash or a colon, or be only dots and spaces.

#### Returns
An array of all the filenames.

#### `public String[] `[`getFolders`](#classFolder_1ad738dfd1f4c322347a1caa549d4b3eec)`(String search)` 

Get the name for each of the sub-directories in the current directory.

#### Parameters
* `search` Optional wildcard pattern for names in this folder (default `*.*`). It can't contain a backslash, a slash or a colon, or be only dots and spaces.

#### Returns
An array of all the folder names.

# class `Frame` 

This class represents a screen hook for a [Frame](#classFrame).

A [Frame](#classFrame) is the game's decorative border (edge and corner pieces) drawn around a rectangle. It has no color or opacity and does not fill its inside; put a [Box](#classBox) under it for a background. Create one with `new Frame(x, y, xsize, ysize, align, automap, click, hover)`; every argument is optional (defaults 0, 0, 0, 0, 0, false, none, none). Coordinates are game-window pixels, x to the right and y down. A [Frame](#classFrame) draws only in game, never at the menus. With `automap` true the [Frame](#classFrame) still uses screen pixels, but shows only while the automap is open; `automap` is a constructor argument only. The four corner pieces are always drawn, so a [Frame](#classFrame) too small for its border (even `new Frame()` with its default size of 0), or with a negative size, still draws its corners.

Hooks of every script and every hook class are drawn in ascending `zorder` (default 1), so a higher value draws on top. Set `visible` to hide or show the [Frame](#classFrame), and call remove() to delete it. Keep a reference to every hook: one whose object is garbage-collected is removed. When the script that made a hook ends, its hooks leave the screen (with the default `DisableCache=true` in d2bs.ini; with `DisableCache=false` a script that ended normally keeps its hooks drawn, but not clickable, until the script cache is flushed).

Number arguments and number properties take any finite number and drop its fraction (the same result as `value | 0` in script); any other value is ignored and the default or current value stays. `zorder` keeps only the low 16 bits, so -1 reads back 65535. `automap` and `visible` take only true or false; any other value is ignored.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classFrame_1a0e43afae34b6d1fb613a42fa5e027d01) | The x coordinate of the [Frame](#classFrame).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classFrame_1a465bb636563d27695e1505e778e08c57) | The y coordinate (top) of the [Frame](#classFrame).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`xsize`](#classFrame_1a3217f35f7ca8516a5f636edc928badbf) | The xsize (width) of the [Frame](#classFrame).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`ysize`](#classFrame_1adb6e9d30c3b3452aae2b3744701b4b89) | The ysize (height) of the [Frame](#classFrame).
`public bool `[`visible`](#classFrame_1a863ec6abf9fd37fa6a33e63d1d58718f) | Whether or not the [Frame](#classFrame) is visible.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`align`](#classFrame_1a7344aa54a7eeaa68fdf7fa42b0fa0a29) | The horizontal alignment.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`zorder`](#classFrame_1a7b6c33df644c92758f289fd76e905b89) | The z-order of the [Frame](#classFrame) (what it covers up and is covered by).
`public ClickHandler `[`click`](#classFrame_1a2e9a32aed91d5f9c7ffb77d93f39a047) | The click handler that gets called when the [Frame](#classFrame) is clicked; null or undefined removes it.
`public HoverHandler `[`hover`](#classFrame_1a62ee67d27035fd2178031b0eee2116f4) | A hover handler for the [Frame](#classFrame); kept but never called.
`public  `[`Frame`](#classFrame_1a659ccea661a5f12de6baefb5bfa9b732)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x2,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y2,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` align,bool automap,ClickHandler click,HoverHandler hover)` | Create a [Frame](#classFrame) hook with the given parameters.
`public void `[`remove`](#classFrame_1ab6188f670788ea98d8a9d3fd65211d8f)`()` | Remove the [Frame](#classFrame) from the screen for good.

## Members

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classFrame_1a0e43afae34b6d1fb613a42fa5e027d01) 

The x coordinate of the [Frame](#classFrame).

In window pixels, even when the [Frame](#classFrame) was made with automap true. It is the left edge with align 0 and the centre with align 2; with align 1 the [Frame](#classFrame) starts xsize/2 to the right of it. While x or y is -1 the [Frame](#classFrame) is not drawn.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classFrame_1a465bb636563d27695e1505e778e08c57) 

The y coordinate (top) of the [Frame](#classFrame).

In window pixels. While x or y is -1 the [Frame](#classFrame) is not drawn.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`xsize`](#classFrame_1a3217f35f7ca8516a5f636edc928badbf) 

The xsize (width) of the [Frame](#classFrame).

In pixels. A width too small for the border, or a negative one, draws only the four corner pieces.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`ysize`](#classFrame_1adb6e9d30c3b3452aae2b3744701b4b89) 

The ysize (height) of the [Frame](#classFrame).

In pixels. A height too small for the border, or a negative one, draws only the four corner pieces.

#### `public bool `[`visible`](#classFrame_1a863ec6abf9fd37fa6a33e63d1d58718f) 

Whether or not the [Frame](#classFrame) is visible.

Defaults to true. A hidden [Frame](#classFrame) is not drawn and gets no clicks. Only true or false is accepted.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`align`](#classFrame_1a7344aa54a7eeaa68fdf7fa42b0fa0a29) 

The horizontal alignment.

0 - Left: the [Frame](#classFrame) starts at x

1 - the [Frame](#classFrame) is shifted right by half its width (not right-aligned)

2 - Center: the [Frame](#classFrame) is centred on x

Other values are stored as given and draw as 0. The click area always starts at x, whatever the alignment.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`zorder`](#classFrame_1a7b6c33df644c92758f289fd76e905b89) 

The z-order of the [Frame](#classFrame) (what it covers up and is covered by).

Defaults to 1. Hooks are drawn from the lowest zorder to the highest, so a higher value covers a lower one; hooks with equal values draw in no set order. A click reaches every hook under it, whatever its zorder.

#### `public ClickHandler `[`click`](#classFrame_1a2e9a32aed91d5f9c7ffb77d93f39a047) 

The click handler that gets called when the [Frame](#classFrame) is clicked. Assign null or undefined to remove it; a value that is not a function is ignored.

It is called as click(button, x, y) when a mouse button is pressed over the [Frame](#classFrame): button 0 left, 1 right, 2 middle; x and y are window pixels. Only a boolean `true` return keeps the press from the game; the release always reaches the game. The handler runs when the owning script next processes events, and the game gets the press anyway if the handler has not answered within 3 seconds. A paused or ended script's hooks get no clicks.

The [Frame](#classFrame) gets clicks while its `visible` property is true, even when it is not drawn at the moment (at the menus, automap closed, x or y of -1). The click area runs from x to x + xsize and y to y + ysize (edges excluded) on the raw values: alignment is not applied.

#### `public HoverHandler `[`hover`](#classFrame_1a62ee67d27035fd2178031b0eee2116f4) 

A hover handler for the [Frame](#classFrame). D2BS keeps it and returns it from `hover`, but never calls it. Assign null or undefined to remove it; a value that is not a function is ignored.

#### `public  `[`Frame`](#classFrame_1a659ccea661a5f12de6baefb5bfa9b732)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x2,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y2,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` align,bool automap,ClickHandler click,HoverHandler hover)` 

Create a [Frame](#classFrame) hook with the given parameters.

#### Parameters
* `x` The x coordinate of the [Frame](#classFrame) in window pixels: its left edge with align 0, its centre with align 2; with align 1 the [Frame](#classFrame) starts xsize/2 to the right of x.

* `y` The y coordinate (top) of the [Frame](#classFrame), in window pixels.

* `x2` The xsize (width) of the [Frame](#classFrame). A width too small for the border, or a negative one, draws only the four corner pieces.

* `y2` The ysize (height) of the [Frame](#classFrame). A height too small for the border, or a negative one, draws only the four corner pieces.

* `align` The horizontal alignment.

0 - Left

1 - shifted right by half the width (not right-aligned)

2 - Center

#### Parameters
* `automap` When true, the [Frame](#classFrame) shows only while the automap is open; its coordinates stay screen pixels. Only true or false is accepted.

* `click` A function to call when the [Frame](#classFrame) is clicked; any other value sets no handler.

* `hover` A hover function for the [Frame](#classFrame), kept but never called; any other value sets no handler.

#### `public void `[`remove`](#classFrame_1ab6188f670788ea98d8a9d3fd65211d8f)`()` 

Remove the [Frame](#classFrame) from the screen for good.

The object stays and its click and hover handlers are released. Its properties then read undefined, writes to them are ignored, and calling remove() again does nothing. Returns undefined.

# class `Image` 

This class represents a screen hook for an [Image](#classImage).

An [Image](#classImage) draws a picture from an 8-bit BMP file on the game screen or the automap. Create one with `new Image(file, x, y, color, align, automap, click, hover)`; every argument is optional (defaults: no file, 0, 0, 0, 0, false, none, none; without a file the [Image](#classImage) stays blank). The BMP's own palette is ignored: each pixel is drawn as that entry of the game's current palette, pixels of index 0 are transparent, and pixels of index 255 take the palette index given by `color`. The picture is centred vertically on y; `align` puts x at its left edge (0), right edge (1) or centre (2). An [Image](#classImage) made by an out-of-game script draws only at the menus; one made in game (or from the console) draws only in game. With `automap` true (a constructor argument only), x and y are game map coordinates (the units of unit.x and unit.y) and the [Image](#classImage) is drawn only while the automap is open; only align 2 then puts the picture on its map point, while 0 and 1 shift it sideways.

Hooks of every script and every hook class are drawn in ascending `zorder` (default 1), so a higher value draws on top. Set `visible` to hide or show the [Image](#classImage), and call remove() to delete it. Keep a reference to every hook: one whose object is garbage-collected is removed. When the script that made a hook ends, its hooks leave the screen (with the default `DisableCache=true` in d2bs.ini; with `DisableCache=false` a script that ended normally keeps its hooks drawn, but not clickable, until the script cache is flushed).

Number arguments and number properties take any finite number and drop its fraction (the same result as `value | 0` in script); any other value is ignored and the default or current value stays. `zorder` keeps only the low 16 bits, so a `zorder` of -1 reads back 65535; `color` is a palette index and only its low 8 bits are used. `automap` and `visible` take only true or false; any other value is ignored.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classImage_1a7f8f4530212c93856e611030e46c82af) | The x coordinate of the [Image](#classImage).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classImage_1a9b03b7d8dd6f69cb5a444bdd0edd786e) | The y coordinate of the [Image](#classImage)'s vertical centre.
`public bool `[`visible`](#classImage_1addd9aead76cc50bf9891d4096195c18d) | Whether or not the [Image](#classImage) is visible.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`align`](#classImage_1a943ad156731bd86b94d77e4f617e0fe8) | The horizontal alignment.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`zorder`](#classImage_1a8f7cef488ff6a72dde9dea8a5c14e39b) | The z-order of the [Image](#classImage) (what it covers up and is covered by).
`public ClickHandler `[`click`](#classImage_1a69ec2e746b9688ac033064f69a132a35) | The click handler that gets called when the [Image](#classImage) is clicked; null or undefined removes it.
`public HoverHandler `[`hover`](#classImage_1ad60232f651bec42eab0f73f2344e4bf0) | A hover handler for the [Image](#classImage); kept but never called.
`public String `[`location`](#classImage_1af4f3e489b1fb5cfa4ffdbe52ea4aee51) | Location of the file to load for display.
`public  `[`Image`](#classImage_1ae6297ffbf18a41caa7971e58571250db)`(String szText,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` color,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` align,bool automap,ClickHandler click,HoverHandler hover)` | Create an [Image](#classImage) hook with the given parameters.
`public void `[`remove`](#classImage_1a777c1f1bf88cf556aa256c6559bd3401)`()` | Remove the [Image](#classImage) from the screen for good.

## Members

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classImage_1a7f8f4530212c93856e611030e46c82af) 

The x coordinate of the [Image](#classImage).

It is the picture's left edge with align 0, its right edge with align 1 and its centre with align 2. A map coordinate when the [Image](#classImage) was made with automap true. While x or y is -1 the [Image](#classImage) is not drawn.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classImage_1a9b03b7d8dd6f69cb5a444bdd0edd786e) 

The y coordinate of the [Image](#classImage)'s vertical centre.

A map coordinate when the [Image](#classImage) was made with automap true. While x or y is -1 the [Image](#classImage) is not drawn.

#### `public bool `[`visible`](#classImage_1addd9aead76cc50bf9891d4096195c18d) 

Whether or not the [Image](#classImage) is visible.

Defaults to true. A hidden [Image](#classImage) is not drawn and gets no clicks. Only true or false is accepted.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`align`](#classImage_1a943ad156731bd86b94d77e4f617e0fe8) 

The horizontal alignment.

0 - Left: x is the picture's left edge

1 - Right: x is the picture's right edge

2 - Center: x is the picture's centre

Other values are stored as given and draw as 2. With automap true only 2 (or another value that draws as 2) puts the picture on its map point; 0 and 1 shift it sideways.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`zorder`](#classImage_1a8f7cef488ff6a72dde9dea8a5c14e39b) 

The z-order of the [Image](#classImage) (what it covers up and is covered by).

Defaults to 1. Hooks are drawn from the lowest zorder to the highest, so a higher value covers a lower one; hooks with equal values draw in no set order. A click reaches every hook under it, whatever its zorder.

#### `public ClickHandler `[`click`](#classImage_1a69ec2e746b9688ac033064f69a132a35) 

The click handler that gets called when the [Image](#classImage) is clicked. Assign null or undefined to remove it; a value that is not a function is ignored.

It is called as click(button, x, y) when a mouse button is pressed over the [Image](#classImage): button 0 left, 1 right, 2 middle; x and y are window pixels. Only a boolean `true` return keeps the press from the game; the release always reaches the game. The handler runs when the owning script next processes events, and the game gets the press anyway if the handler has not answered within 3 seconds. A paused or ended script's hooks get no clicks.

The [Image](#classImage) gets clicks while its `visible` property is true, even when it is not drawn at the moment (other game state, automap closed, x or y of -1). The click area uses the raw x and y (the automap conversion is not applied) and spans y - height/2 to y + height/2. With align 1 and 2 it matches the picture; with align 0 it is one picture-width to the right of the picture. An [Image](#classImage) whose file did not load (a blank [Image](#classImage)) gets no clicks.

#### `public HoverHandler `[`hover`](#classImage_1ad60232f651bec42eab0f73f2344e4bf0) 

A hover handler for the [Image](#classImage). D2BS keeps it and returns it from `hover`, but never calls it. Assign null or undefined to remove it; a value that is not a function is ignored.

#### `public String `[`location`](#classImage_1af4f3e489b1fb5cfa4ffdbe52ea4aee51) 

Location of the file to load for display.

It is relative to the script folder (an uncompressed 8-bit bottom-up BMP, as for new Image()). Setting a name that new Image() would refuse throws "Invalid image file path" and keeps the current file; a name that does not load leaves the Image blank. Reading it returns the name as it was given; assigning a value that is not a string is ignored.

#### `public  `[`Image`](#classImage_1ae6297ffbf18a41caa7971e58571250db)`(String szText,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` color,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` align,bool automap,ClickHandler click,HoverHandler hover)` 

Create an [Image](#classImage) hook with the given parameters.

#### Parameters
* `szText` The filename to load the image from, relative to the script folder. Only uncompressed 8-bit (256-color) bottom-up BMP files in the script folder load; a missing file or any other format leaves the Image blank, and a file fixed on disk loads the next time an Image or location names it. A name that has loaded once keeps that picture until Diablo II (Game.exe) exits, even if the file is rewritten on disk, and the same file under another spelling (letter case, `.\`, `/` instead of `\`) loads again as its own picture; a file that is deleted or cannot be opened (for example while another program holds it locked) leaves the Image blank and writes a line to the log. A name that leaves that folder (`..\` or `../`), contains any of `":?*<>|`, or makes a path longer than 515 characters when joined to the script folder's path throws "Invalid image file path". A value that is not a string, or no file at all, counts as an empty name, which loads nothing, leaves the Image blank and writes a line to the log.

* `x` The x coordinate of the [Image](#classImage)'s anchor: its left edge (align 0), right edge (align 1) or centre (align 2).

* `y` The y coordinate of the [Image](#classImage)'s vertical centre.

* `color` A game palette index (0-255; only the low 8 bits are used) for the BMP's pixels of index 255. Pixels of index 0 are transparent, and every other index is drawn as that entry of the game's current palette.

* `align` The horizontal alignment.

0 - Left

1 - Right

2 - Center

#### Parameters
* `automap` true to place the [Image](#classImage) at game map coordinates and draw it only while the automap is open; false (the default) for screen coordinates. Only true or false is accepted.

* `click` A function to call when the [Image](#classImage) is clicked; any other value sets no handler.

* `hover` A hover function for the [Image](#classImage), kept but never called; any other value sets no handler.

#### `public void `[`remove`](#classImage_1a777c1f1bf88cf556aa256c6559bd3401)`()` 

Remove the [Image](#classImage) from the screen for good.

The object stays and its click and hover handlers are released. Its properties then read undefined, writes to them are ignored, and calling remove() again does nothing. Returns undefined.

# class `Line` 

A 2d line.

A [Line](#classLine) is a straight line from (x, y) to (x2, y2) in one palette color. Create one with `new Line(x, y, x2, y2, color, automap, click, hover)`; every argument is optional (defaults 0, 0, 0, 0, 0, false, none, none). Coordinates are game-window pixels, x to the right and y down. With `automap` true both ends are game map coordinates (the units of unit.x and unit.y), and the [Line](#classLine) shows only while the automap is open; `automap` is a constructor argument only. A [Line](#classLine) made by an out-of-game script draws only at the menus; one made in game (or from the console) draws only in game. A [Line](#classLine) has no alignment and can never be clicked.

Hooks of every script and every hook class are drawn in ascending `zorder` (default 1), so a higher value draws on top. Set `visible` to hide or show the [Line](#classLine), and call remove() to delete it. Keep a reference to every hook: one whose object is garbage-collected is removed. When the script that made a hook ends, its hooks leave the screen (with the default `DisableCache=true` in d2bs.ini; with `DisableCache=false` a script that ended normally keeps its hooks drawn until the script cache is flushed).

Number arguments and number properties take any finite number and drop its fraction (the same result as `value | 0` in script); any other value is ignored and the default or current value stays. `color` and `zorder` keep only the low 16 bits, so -1 reads back 65535. `automap` and `visible` take only true or false; any other value is ignored.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classLine_1aced699d771efdaea44224fb76035ad64) | The first x coordinate of the [Line](#classLine).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classLine_1aba0b47fa036eff1c6cbc4369389a0cf8) | The first y coordinate of the [Line](#classLine).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x2`](#classLine_1a4f08d1038eba337d6f0fb32713e528f1) | The second x coordinate of the [Line](#classLine).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y2`](#classLine_1ae31c92ab0df730a2fdd34201ce6feb9d) | The second y coordinate of the [Line](#classLine).
`public bool `[`visible`](#classLine_1ad03610daf3369fe0beaaa0b63bb00826) | Whether or not the [Line](#classLine) is visible.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`color`](#classLine_1a9efd376d9b6ac16dd115bd7bfbf9c923) | The color of the [Line](#classLine).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`zorder`](#classLine_1ab5abf5a8115842ca5e99a827966a332b) | The z-order of the [Line](#classLine) (what it covers up and is covered by).
`public ClickHandler `[`click`](#classLine_1a4d07ee7727434b01299b0d41389a1e01) | The click handler for the [Line](#classLine); kept but never called, since a click never hits a [Line](#classLine). Null or undefined removes it.
`public HoverHandler `[`hover`](#classLine_1a290a03210ee18d9644db5679e931f7a3) | A hover handler for the [Line](#classLine); kept but never called.
`public  `[`Line`](#classLine_1afcf6affa1a4e375b37e61669b1103f12)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x2,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y2,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` color,bool automap,ClickHandler click,HoverHandler hover)` | Create a [Line](#classLine) hook with the given parameters.
`public void `[`remove`](#classLine_1a33c4869ac6002c11dcace8c4e5d47c60)`()` | Remove the [Line](#classLine) from the screen for good.

## Members

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classLine_1aced699d771efdaea44224fb76035ad64) 

The first x coordinate of the [Line](#classLine).

A map coordinate when the [Line](#classLine) was made with automap true. While x or y is -1 the [Line](#classLine) is not drawn (x2 and y2 of -1 do not hide it).

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classLine_1aba0b47fa036eff1c6cbc4369389a0cf8) 

The first y coordinate of the [Line](#classLine).

A map coordinate when the [Line](#classLine) was made with automap true. While x or y is -1 the [Line](#classLine) is not drawn.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x2`](#classLine_1a4f08d1038eba337d6f0fb32713e528f1) 

The second x coordinate of the [Line](#classLine).

An absolute coordinate, not a width; a map coordinate when the [Line](#classLine) was made with automap true.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y2`](#classLine_1ae31c92ab0df730a2fdd34201ce6feb9d) 

The second y coordinate of the [Line](#classLine).

An absolute coordinate, not a height; a map coordinate when the [Line](#classLine) was made with automap true.

#### `public bool `[`visible`](#classLine_1ad03610daf3369fe0beaaa0b63bb00826) 

Whether or not the [Line](#classLine) is visible.

Defaults to true. Only true or false is accepted.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`color`](#classLine_1a9efd376d9b6ac16dd115bd7bfbf9c923) 

The color of the [Line](#classLine).

An index (0-255) into the game's 256-color palette. Only the low 8 bits are drawn, so 256 draws as 0, though the property reads back the low 16 bits.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`zorder`](#classLine_1ab5abf5a8115842ca5e99a827966a332b) 

The z-order of the [Line](#classLine) (what it covers up and is covered by).

Defaults to 1. Hooks are drawn from the lowest zorder to the highest, so a higher value covers a lower one; hooks with equal values draw in no set order.

#### `public ClickHandler `[`click`](#classLine_1a4d07ee7727434b01299b0d41389a1e01) 

The click handler for the [Line](#classLine). D2BS keeps it and returns it from `click`, but never calls it: a click never hits a [Line](#classLine). Assign null or undefined to remove it; a value that is not a function is ignored.

#### `public HoverHandler `[`hover`](#classLine_1a290a03210ee18d9644db5679e931f7a3) 

A hover handler for the [Line](#classLine). D2BS keeps it and returns it from `hover`, but never calls it. Assign null or undefined to remove it; a value that is not a function is ignored.

#### `public  `[`Line`](#classLine_1afcf6affa1a4e375b37e61669b1103f12)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x2,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y2,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` color,bool automap,ClickHandler click,HoverHandler hover)` 

Create a [Line](#classLine) hook with the given parameters.

#### Parameters
* `x` The x coordinate of the first end of the [Line](#classLine).

* `y` The y coordinate of the first end of the [Line](#classLine).

* `x2` The x coordinate of the second end of the [Line](#classLine).

* `y2` The y coordinate of the second end of the [Line](#classLine).

* `color` The color of the [Line](#classLine): an index into the game's 256-color palette; only the low 8 bits are drawn.

* `automap` With true, all four coordinates are map coordinates and the [Line](#classLine) shows only while the automap is open. Only true or false is accepted.

* `click` A click function for the [Line](#classLine), kept but never called (a click never hits a [Line](#classLine)); any other value sets no handler.

* `hover` A hover function for the [Line](#classLine), kept but never called; any other value sets no handler.

#### `public void `[`remove`](#classLine_1a33c4869ac6002c11dcace8c4e5d47c60)`()` 

Remove the [Line](#classLine) from the screen for good.

The object stays and its click and hover handlers are released. Its properties then read undefined, writes to them are ignored, and calling remove() again does nothing. Returns undefined.

# class `Me` 

The class that represents the controlled unit.

Technically, also called [Unit](#classUnit), but since C++ doesn't support class overloading, called it [Me](#classMe) here instead.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`type`](#classMe_1a52c5b4db63a7b8446dc8107fc07cdf3d) | The type of the unit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`classId`](#classMe_1afc46c25e98a712ddbab9fe5b1895c581) | The class id of the object.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`mode`](#classMe_1aaaa649505660616f54b0ee92776a25b8) | The mode of the unit.
`public String `[`name`](#classMe_1a7a79209dc7ba4388f1a0b0948e1809c2) | The name of the unit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`mapid`](#classMe_1aeca5382b1726287c31235f5cc9d4d22d) | The seed used to create the map.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`act`](#classMe_1a9ac2b33c84c6cd3b15afa9b42929fc3f) | The act where the unit is currently located.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`gid`](#classMe_1a1c9269b25de87db3987394c129ef4416) | The unit unique id.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classMe_1a6e1986e643022cf0768b14b83f14d01b) | The x location of the unit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classMe_1aa554298f46bbfaf83a341b5e3d86b2e5) | The y location of the unit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`targetx`](#classMe_1ae57bbfa40548f63d807ec5ead0301024) | The target x location of the unit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`targety`](#classMe_1afe98390a5d4f85c8acd10b6312616c7f) | The target y location of the unit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`area`](#classMe_1a20c97fb4d7a65f1125b2ef94edc93382) | The id of the area (level) the unit is located in.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`hp`](#classMe_1ada4675cf13005bce0c1562c903dc44c5) | The unit's current health.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`hpmax`](#classMe_1a76936aa550e766f03719a350a0166a08) | The unit's maximum health.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`mp`](#classMe_1a2818fdee3d22e50df4d044294978efc4) | The unit's current mana.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`mpmax`](#classMe_1aff3b5b0dd48925f742d96385f0cc3391) | The unit's maximum mana.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`stamina`](#classMe_1ab53e765f014b5a962dc0cf0374d61047) | The unit's current stamina.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`staminamax`](#classMe_1a27dc070bdfcf26014c8e05f8c3d45d54) | The unit's maximum stamina.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`charlvl`](#classMe_1ad1de73d82c060287cddb8c5e111c4763) | The character level.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`itemcount`](#classMe_1ad1ccf5dc94d072c2ceb7b505868e2058) | The number of items in inventory.
`public double `[`owner`](#classMe_1a3322befee99b6afa964c5f8ee52983d2) | The unit id of the owner of the unit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`ownertype`](#classMe_1a6f9f7bbef6962920fe9306a620f204b6) | The type of owner.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`spectype`](#classMe_1ad244e2f81cefed6a58bc2b3aed36b9f3) | The special type of the unit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`direction`](#classMe_1a6ee239b2452d3c06e33de7e1725be36a) | The direction the unit is facing, from 0 to 63.
`public String `[`code`](#classMe_1aa90bcab94a360d2e8380323d8b4640cf) | The three digit code for an item.
`public String `[`prefix`](#classMe_1ab68062a31f250f249221a399c541b5e8) | The magic prefix of the item.
`public String `[`suffix`](#classMe_1a3633bb215e73eda5916b9c0f372d1f80) | The magic suffix of the item.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`prefixnum`](#classMe_1ae54c6d4c87d2ad0f7f1c9a6b1f4381fc) | The id code for the magic prefix.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`suffixnum`](#classMe_1a3d523157c13fcf3229ecc1b9d7724367) | The id code for the magic suffix.
`public String `[`fname`](#classMe_1a756f0b6b2b2b5f2439d1afe17e9d23ca) | The full name of an item.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`quality`](#classMe_1ace5f9a9c6a8139fbe30ca258e2f1bdc5) | The item quality.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`node`](#classMe_1a71440306ac4fa9414da959b615f24200) | Which kind of inventory grid holds the item.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`location`](#classMe_1a8932e926ae2bd373885fd45b2ef78907) | The location of the item (ground, inventory, stash, etc.).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`sizex`](#classMe_1a2a1356d1bb2c3fdbda515446a06b626f) | The x size of the item.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`sizey`](#classMe_1ad5a059256862e2a27e4473f299c1c2a0) | The y size of the item.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`itemType`](#classMe_1a5cbdd81b5607c8098994228fe5ab7151) | The type of the item.
`public String `[`description`](#classMe_1a52e690a6c8b7786a7bc56f400a96c4f4) | The description of the item.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`bodylocation`](#classMe_1aa895c745c9f4f909986ddbacd7d31e73) | The equipped location of the item.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`ilvl`](#classMe_1a742c6b149eeaafd2e947662a2a7a3c61) | Item level.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`runwalk`](#classMe_1ab3fd8214e6809542ea1b99f0a2cd2929) | Whether the controlled character is in the always run mode or not.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`weaponswitch`](#classMe_1adb2a8033d013320f821b7afefab5749a) | Set the weapon switch.
`public String `[`account`](#classMe_1a73c79f2cf49747d6171827f6d6bcbe24) | The account name used to log on to the account.
`public String `[`charname`](#classMe_1a3aafeeae1d507aa6492fdfd14d44fc6a) | The character name.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`diff`](#classMe_1adddc240680097fd939d1afba7a1661cf) | The difficulty of the game.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`maxdiff`](#classMe_1afc952d96b2b29fbee19791b2e6234e97) | The maximum difficulty available.
`public String `[`gamename`](#classMe_1a82b4d94b4cf1d7310bb910919347eefd) | The name of the game.
`public String `[`gamepassword`](#classMe_1aa872a1fbaa9f8aa85d01963f71813324) | The password of the game.
`public String `[`gameserverip`](#classMe_1a3debf45fa879b24fad68925e53256c2b) | A string representation of the IPv4 address of the game server.
`public double `[`gamestarttime`](#classMe_1a2fed13a602e6882f51553b8d4c3042be) | When the character entered the current game, as a getTickCount() value.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`gametype`](#classMe_1ad31342341ab64e1444961ff1059c8a05) | The game type, expansion or not.
`public bool `[`itemoncursor`](#classMe_1abd16276ae4f171422e7185fdc3bb49cc) | Whether or not there is an item on the cursor.
`public bool `[`ladder`](#classMe_1ab9d7b4028e92dc9e8e8a124ae434ca82) | Whether or not the game is a ladder game.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`ping`](#classMe_1aee8c765131a0ce6a04a5a78198d90910) | The ping in milliseconds.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`fps`](#classMe_1a26cbb5b87c79ba3d8fd43e56cce260c7) | The frame rate.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`playertype`](#classMe_1a8cf7291feb3bc8ebeb35dd5baf373427) | Whether or not the gametype is hardcore.
`public String `[`realm`](#classMe_1a9d5e5a2fec0a033316b6d49073b961d5) | The realm name.
`public String `[`realmshort`](#classMe_1aa8bdeb8ae3ba0db767a64f84ba489094) | The realm name, normally the same value as me.realm.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`mercrevivecost`](#classMe_1a913adbc2fce146ef6c9f5268a76ebfc5) | The cost to revive the merc.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`chickenhp`](#classMe_1a215c51bfd214de4bff2eeefdc5ecd5ab) | The health at or below which to chicken, outside town; 0 turns it off.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`chickenmp`](#classMe_1a7e95aede9d9b33b217e2c74b0aed9865) | The mana at or below which to chicken, outside town; 0 turns it off.
`public bool `[`quitonhostile`](#classMe_1a2b52680cdbc8a340cf1b83a11f288e9a) | Whether to chicken if another player hostiles the controlled character.
`public bool `[`blockKeys`](#classMe_1a32b0fe4966c613767100bca03d4cff54) | Whether keyboard input to the game window is blocked.
`public bool `[`blockMouse`](#classMe_1aa9360f70984b69d889ca7744a977d031) | Whether mouse input to the game window is blocked.
`public bool `[`gameReady`](#classMe_1adbf3d1e8b502d34c5f6895bb2c40f1d8) | Whether or not the game window is in the game.
`public String `[`profile`](#classMe_1a5321127db8f2c6ca21d1428fd753157d) | The profile currently being used.
`public bool `[`nopickup`](#classMe_1a0a3973ed5dae29cbfe95dc7745646004) | Whether the game is set to not pick up items or not.
`public double `[`pid`](#classMe_1ad10c9c74976842a1f9634410ad833744) | The process id of Diablo II.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`screensize`](#classMe_1acf69e064892375059446599a1494fea6) | The game's resolution mode.
`public String `[`windowtitle`](#classMe_1ae1fb424ddea108a0894dddafae0febbd) | The window title.
`public bool `[`ingame`](#classMe_1a92d8763c52f3ac2ec1d680bd026e2404) | True if the game is not at menu.
`public bool `[`quitonerror`](#classMe_1a57806876c33f36d3bcc1ca4599fe60be) | Whether a script error makes the client leave the current game.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`maxgametime`](#classMe_1ad4f57667eb1a3bd8e45288c9f3fe923a) | The max time in milliseconds to stay in game before the game auto-quits.
`public bool `[`getNext`](#classMe_1ac38694e80aec8caa323c50785816bcbd)`()` | Get the next unit that matches the originally given name or class id and originally specified mode.
`public bool `[`getNext`](#classMe_1a978176a65b1a7d182cee4fb9384cb626)`(String szName)` | Get the next unit that matches the given name and originally specified mode.
`public bool `[`getNext`](#classMe_1afb7f7bf695192c13ab56b2fbe29f743e)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` dwClassId)` | Get the next unit that matches the given class id and originally specified mode.
`public bool `[`getNext`](#classMe_1a02f641e81f28741b2bef50c16d3372da)`(String szName,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` dwMode)` | Get the next unit that matches the given name and mode.
`public bool `[`getNext`](#classMe_1a12feab5ac7792369015564b222441acd)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` dwClassId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` dwMode)` | Get the next unit that matches the given class id and mode.
`public void `[`cancel`](#classMe_1a3d6d506ed810276b50ba86f9c3f4dd98)`()` | Cancel some form of interaction.
`public bool `[`repair`](#classMe_1ab3612cb5a103004b87a9321b5a7aec50)`()` | Try to repair.
`public bool `[`useMenu`](#classMe_1aa29c5eccaa458901d4ec804e4fa167ca)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` menuItem)` | Use an NPC menu.
`public bool `[`interact`](#classMe_1a3341b12f791fa69c7ba048cc90778f20)`()` | Interact with the unit.
`public bool `[`interact`](#classMe_1a4742f918ff80aa898da57756b1a95fbd)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` destId)` | Interact with a waypoint.
`public `[`Unit`](#classUnit)` `[`getItem`](#classMe_1ab6277b88eedb3985e94a7b2ec2d0ed5e)`(String name,uint32_t mode,uint32_t nUnitId)` | Get an item from inventory by name, mode and nUnitId.
`public `[`Unit`](#classUnit)` `[`getItem`](#classMe_1a1678e3178379630e2b4208e011129094)`(uint32_t classId,uint32_t mode,uint32_t nUnitId)` | Get an item from inventory by classId, mode and nUnitId.
`public `[`Unit`](#classUnit)` `[`getItems`](#classMe_1aa3a172ada3c1e477299cf697f3722076)`()` | Get all items from inventory.
`public `[`Unit`](#classUnit)` `[`getMerc`](#classMe_1a12c1ae747f947cc96bcda92d517e4fd4)`()` | Get a player's merc.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getMercHP`](#classMe_1a8b9d83bd2cc1d39c89c47c2f3d97f398)`()` | Get the merc's HP.
`public String `[`getSkill`](#classMe_1a449dad9a671272e94c7df7ab07277043)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` hand)` | Get skill name from hand.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getSkill`](#classMe_1a482e347e3d2131af87a6aca7019bf4fd)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` hand)` | Get skill id from hand.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getSkill`](#classMe_1a68d6118cef94eb973cfa1591ea890e3f)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` dummy)` | Get all skills.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getSkill`](#classMe_1a8319dc883867a8e9f6cc101ecbee0aa5)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSkillId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nExt)` | Get the base or total level of a skill.
`public `[`Unit`](#classUnit)` `[`getParent`](#classMe_1a211afa5299fc94407dc2a6d78ebadd8c)`()` | Gets the parent of a unit.
`public String `[`getParent`](#classMe_1a4019ee0bef41f98dd4170a205de08cd7)`()` | Gets the parent of a unit.
`public void `[`overhead`](#classMe_1a6f5c4067c0304c88ab5105b93e787f7d)`(Object message)` | Puts the string equivalent of message over the unit.
`public void `[`revive`](#classMe_1a7e53744fc9cd8233451fa8562e4ce7e9)`()` | Revive the character.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getFlags`](#classMe_1a2901a55f39537ba49f5c90cfb8efe1a3)`()` | Returns the item flags.
`public double `[`getStat`](#classMe_1a1d7999f11e2a07f70dc69432b06ba782)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nStat,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSubIndex)` | Get a stat by stat id and sub index.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getStat`](#classMe_1a34c73953d580d9eae5a32d67d26a7ea0)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nStat,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSubIndex)` | Get an array of the unit's listed stats.
`public Object[] `[`getStat`](#classMe_1getStat_byId)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nStat,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSubIndex)` | Get the unit's stats as an array indexed by stat id.
`public bool `[`getState`](#classMe_1a6a8f7eed0b6808691957c24f79f8e111)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nState)` | Return whether or not the unit has a given state.
`public bool `[`getEnchant`](#classMe_1a2b35f9abde10c9fe6ba08b6e2c31ee3c)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nEnchant)` | Determine whether or not a unit has a given enchant.
`public bool `[`shop`](#classMe_1a3469ed7606e573d7e09e197999ae494e)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` dwMode)` | Shop with a given npc, either buying or selling.
`public bool `[`setSkill`](#classMe_1aa6e655c613c557c0ef44cfb8ad536cde)`(String skillName,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nHand)` | Set the skill on the given hand to be skill with name skillName.
`public bool `[`setSkill`](#classMe_1aa8850569751822ba3088f40bfd94584a)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSkillId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nHand)` | Set the skill on the given hand to be skill with id nSkillId.
`public bool `[`setSkill`](#classMe_1af20eafc6d55f4dabcba4fdcf7f8bae40)`(String skillName,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nHand,`[`Unit`](#classUnit)` item)` | Set the skill on the given hand to be skill with name skillName.
`public bool `[`setSkill`](#classMe_1a10a595cd98b1bb7fcc54bfebdd61ac6c)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSkillId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nHand,`[`Unit`](#classUnit)` item)` | Set the skill on the given hand to be skill with id nSkillId.
`public void `[`move`](#classMe_1a497c1507b060c2fcbdf79fcb7263ebdd)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y)` | Move to the given location.
`public void `[`move`](#classMe_1a2ab43203c0909238f2b721db7fda48a5)`()` | Move to this unit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getQuest`](#classMe_1a9ba75f0259540bc166cf7b8cbdbade86)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nAct,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nQuest)` | Get the quest flag for a quest specified by act and quest number.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getMinionCount`](#classMe_1accccb22ea6e3a2618fe740270374d6e6)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nType)` | Get the number of minions of a certain type.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getRepairCost`](#classMe_1a79e916f09dcbe81785c07c3c6bb9505f)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nNpcClassId)` | Get price to repair this unit at the current npc given by nNpcClassId.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getItemCost`](#classMe_1a5589dba170b74d8af4e1c0dd5b176626)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode,`[`Unit`](#classUnit)` npc)` | Get the cost to do something (buy, sell, repair) with the given item, at the given npc.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getItemCost`](#classMe_1abcb1d466bcdac846dbb10c7ba4724caf)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode,`[`Unit`](#classUnit)` npc,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nDifficulty)` | Get the cost to do something (buy, sell, repair) with the given item, at the given npc, in the given difficulty.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getItemCost`](#classMe_1ad2deea07e13b5acaa68d0c09bbe75698)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nNpcClassId)` | Get the cost to do something (buy, sell, repair) with the given item, at the given npc.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getItemCost`](#classMe_1a81997bd2784832f7e1a88863483e0289)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nNpcClassId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nDifficulty)` | Get the cost to do something (buy, sell, repair) with the given item, at the given npc, in the given difficulty.

## Members

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`type`](#classMe_1a52c5b4db63a7b8446dc8107fc07cdf3d) 

The type of the unit.

0 - Player

1 - NPC

2 - Object

3 - Missile

4 - Item

5 - Warp

Source: botNET

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`classId`](#classMe_1afc46c25e98a712ddbab9fe5b1895c581) 

The class id of the object.

See: [http://forums.d2botnet.org/viewtopic.php?f=18&t=1002](http://forums.d2botnet.org/viewtopic.php?f=18&t=1002)

[http://forums.d2botnet.org/viewtopic.php?f=18&t=986](http://forums.d2botnet.org/viewtopic.php?f=18&t=986) and

[http://forums.d2botnet.org/viewtopic.php?f=18&t=985](http://forums.d2botnet.org/viewtopic.php?f=18&t=985)

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`mode`](#classMe_1aaaa649505660616f54b0ee92776a25b8) 

The mode of the unit.

See: [http://forums.d2botnet.org/viewtopic.php?f=18&t=988](http://forums.d2botnet.org/viewtopic.php?f=18&t=988)

#### `public String `[`name`](#classMe_1a7a79209dc7ba4388f1a0b0948e1809c2) 

The name of the unit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`mapid`](#classMe_1aeca5382b1726287c31235f5cc9d4d22d) 

The seed used to create the map.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`act`](#classMe_1a9ac2b33c84c6cd3b15afa9b42929fc3f) 

The act where the unit is currently located.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`gid`](#classMe_1a1c9269b25de87db3987394c129ef4416) 

The unit unique id.

Referred to simply as the unit id. Used along with the unit type to uniquely identify the unit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classMe_1a6e1986e643022cf0768b14b83f14d01b) 

The x location of the unit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classMe_1aa554298f46bbfaf83a341b5e3d86b2e5) 

The y location of the unit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`targetx`](#classMe_1ae57bbfa40548f63d807ec5ead0301024) 

The target x location of the unit.

Where the game is taking the unit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`targety`](#classMe_1afe98390a5d4f85c8acd10b6312616c7f) 

The target y location of the unit.

Where the game is taking the unit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`area`](#classMe_1a20c97fb4d7a65f1125b2ef94edc93382) 

The id of the area (level) the unit is located in.

The value is the Id column of Levels.txt, for example 1 = Rogue Encampment, 108 = The Chaos Sanctuary (Chaos Sanctum in Levels.txt), 109 = Harrogath. See [Area ids](#area-ids) in the reference tables.

Undefined when the game has no room for the unit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`hp`](#classMe_1ada4675cf13005bce0c1562c903dc44c5) 

The unit's current health.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`hpmax`](#classMe_1a76936aa550e766f03719a350a0166a08) 

The unit's maximum health.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`mp`](#classMe_1a2818fdee3d22e50df4d044294978efc4) 

The unit's current mana.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`mpmax`](#classMe_1aff3b5b0dd48925f742d96385f0cc3391) 

The unit's maximum mana.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`stamina`](#classMe_1ab53e765f014b5a962dc0cf0374d61047) 

The unit's current stamina.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`staminamax`](#classMe_1a27dc070bdfcf26014c8e05f8c3d45d54) 

The unit's maximum stamina.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`charlvl`](#classMe_1ad1de73d82c060287cddb8c5e111c4763) 

The character level.

The level that determines stat points, skill levels, etc.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`itemcount`](#classMe_1ad1ccf5dc94d072c2ceb7b505868e2058) 

The number of items in inventory.

#### `public double `[`owner`](#classMe_1a3322befee99b6afa964c5f8ee52983d2) 

The unit id of the owner of the unit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`ownertype`](#classMe_1a6f9f7bbef6962920fe9306a620f204b6) 

The type of owner.

[type](#classMe_1a52c5b4db63a7b8446dc8107fc07cdf3d)

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`spectype`](#classMe_1ad244e2f81cefed6a58bc2b3aed36b9f3) 

The special type of the unit.

Represents things like unique, minion, boss, etc.

1 - "Normal" Boss 2 - Champion 4 - Boss 8 - Minion

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`direction`](#classMe_1a6ee239b2452d3c06e33de7e1725be36a) 

The direction the unit is facing, from 0 to 63.

0 faces straight down the screen and the value increases clockwise: 16 faces left (west), 32 up (north) and 48 right (east). 8, 24, 40 and 56 lie between them along the game's map axes (down-left, up-left, up-right and down-right).

A player or monster turns a step at a time, so while it turns this value can trail the direction it is moving in.

Undefined when the unit has no path or its path is not in a room. For objects and items the value is currently not their facing: it can be undefined or an arbitrary number.

#### `public String `[`code`](#classMe_1aa90bcab94a360d2e8380323d8b4640cf) 

The three digit code for an item.

See: [http://forums.d2botnet.org/viewtopic.php?f=18&t=991](http://forums.d2botnet.org/viewtopic.php?f=18&t=991)

#### `public String `[`prefix`](#classMe_1ab68062a31f250f249221a399c541b5e8) 

The magic prefix of the item.

#### `public String `[`suffix`](#classMe_1a3633bb215e73eda5916b9c0f372d1f80) 

The magic suffix of the item.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`prefixnum`](#classMe_1ae54c6d4c87d2ad0f7f1c9a6b1f4381fc) 

The id code for the magic prefix.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`suffixnum`](#classMe_1a3d523157c13fcf3229ecc1b9d7724367) 

The id code for the magic suffix.

#### `public String `[`fname`](#classMe_1a756f0b6b2b2b5f2439d1afe17e9d23ca) 

The full name of an item.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`quality`](#classMe_1ace5f9a9c6a8139fbe30ca258e2f1bdc5) 

The item quality.

1 - Low quality

2 - Normal

3 - Superior

4 - Magic

5 - Set

6 - Rare

7 - Unique

8 - Crafted

Source: botNET

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`node`](#classMe_1a71440306ac4fa9414da959b615f24200) 

Which kind of inventory grid holds the item.

0 - Not in any inventory grid (on the ground or on the cursor)

1 - A storage page (inventory, cube, stash, trade or shop)

2 - The belt

3 - Equipped (body locations 1-10)

4 - Equipped in a weapon-switch slot (body locations 11 and 12)

Use location to tell storage pages apart. An item linked into an inventory without a grid cell also reads 1, so node 1 alone does not prove the item sits on a storage page.

Undefined for units that are not items, so me.node is always undefined.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`location`](#classMe_1a8932e926ae2bd373885fd45b2ef78907) 

The location of the item (ground, inventory, stash, etc.).

0 - Ground

1 - Equipped

2 - Belt

3 - Inventory

4 - Store

5 - Trade

6 - Cube

7 - Stash

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`sizex`](#classMe_1a2a1356d1bb2c3fdbda515446a06b626f) 

The x size of the item.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`sizey`](#classMe_1ad5a059256862e2a27e4473f299c1c2a0) 

The y size of the item.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`itemType`](#classMe_1a5cbdd81b5607c8098994228fe5ab7151) 

The type of the item.

The base item's primary type: its row index in ItemTypes.txt, counting data rows from 0 and skipping the "Expansion" separator row (for example 2 = shield, 4 = gold, 10 = ring, 12 = amulet, 58 = jewel, 74 = rune). Class-specific bases report their own type, so an auric shield is not 2. See [Item types](#item-types) in the reference tables.

Undefined for units that are not items, so me.itemType is always undefined.

#### `public String `[`description`](#classMe_1a52e690a6c8b7786a7bc56f400a96c4f4) 

The description of the item.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`bodylocation`](#classMe_1aa895c745c9f4f909986ddbacd7d31e73) 

The equipped location of the item.

0 - Not equipped

1 - Helmet

2 - Amulet

3 - Armor

4 - Right hand slot 1

5 - Left hand slot 1

6 - Right ring

7 - Left ring

8 - Belt

9 - Boots

10 - Gloves

11 - Right hand slot 2

12 - Left hand slot 2

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`ilvl`](#classMe_1a742c6b149eeaafd2e947662a2a7a3c61) 

Item level.

Used for things like item stat rolling.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`runwalk`](#classMe_1ab3fd8214e6809542ea1b99f0a2cd2929) 

Whether the controlled character is in the always run mode or not.

0 - Walk unless directed to run 1 - Always run

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`weaponswitch`](#classMe_1adb2a8033d013320f821b7afefab5749a) 

Set the weapon switch.

#### `public String `[`account`](#classMe_1a73c79f2cf49747d6171827f6d6bcbe24) 

The account name used to log on to the account.

#### `public String `[`charname`](#classMe_1a3aafeeae1d507aa6492fdfd14d44fc6a) 

The character name.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`diff`](#classMe_1adddc240680097fd939d1afba7a1661cf) 

The difficulty of the game.

0 - Normal

1 - Nightmare

2 - Hell

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`maxdiff`](#classMe_1afc952d96b2b29fbee19791b2e6234e97) 

The maximum difficulty available.

See [Me::diff](#classMe_1adddc240680097fd939d1afba7a1661cf) for details.

#### `public String `[`gamename`](#classMe_1a82b4d94b4cf1d7310bb910919347eefd) 

The name of the game.

#### `public String `[`gamepassword`](#classMe_1aa872a1fbaa9f8aa85d01963f71813324) 

The password of the game.

#### `public String `[`gameserverip`](#classMe_1a3debf45fa879b24fad68925e53256c2b) 

A string representation of the IPv4 address of the game server.

#### `public double `[`gamestarttime`](#classMe_1a2fed13a602e6882f51553b8d4c3042be) 

When the character entered the current game, as a getTickCount() value.

For a game D2BS was loaded into, it is when D2BS first saw that game; before the first game, when D2BS loaded; at the menu after a game, 0.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`gametype`](#classMe_1ad31342341ab64e1444961ff1059c8a05) 

The game type, expansion or not.

0 - Classic

1 - Expansion (Lord of Destruction)

It reads 0 at the menus, early while a game loads and while a game is left; it can take its real value shortly before gameReady turns true, so read it once gameReady is true. Read-only.

#### `public bool `[`itemoncursor`](#classMe_1abd16276ae4f171422e7185fdc3bb49cc) 

Whether or not there is an item on the cursor.

#### `public bool `[`ladder`](#classMe_1ab9d7b4028e92dc9e8e8a124ae434ca82) 

Whether or not the game is a ladder game.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`ping`](#classMe_1aee8c765131a0ce6a04a5a78198d90910) 

The ping in milliseconds.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`fps`](#classMe_1a26cbb5b87c79ba3d8fd43e56cce260c7) 

The frame rate.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`playertype`](#classMe_1a8cf7291feb3bc8ebeb35dd5baf373427) 

Whether or not the gametype is hardcore.

#### `public String `[`realm`](#classMe_1a9d5e5a2fec0a033316b6d49073b961d5) 

The realm name.

#### `public String `[`realmshort`](#classMe_1aa8bdeb8ae3ba0db767a64f84ba489094) 

The realm name, normally the same value as me.realm.

It is not an abbreviation: it holds the name of the Battle.net realm the client logged in to, for example "Europe". Empty in single player and TCP/IP; undefined if the client has no Battle.net data. Read-only.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`mercrevivecost`](#classMe_1a913adbc2fce146ef6c9f5268a76ebfc5) 

The cost to revive the merc.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`chickenhp`](#classMe_1a215c51bfd214de4bff2eeefdc5ecd5ab) 

The health at or below which to chicken, outside town; 0 turns it off.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`chickenmp`](#classMe_1a7e95aede9d9b33b217e2c74b0aed9865) 

The mana at or below which to chicken, outside town; 0 turns it off.

#### `public bool `[`quitonhostile`](#classMe_1a2b52680cdbc8a340cf1b83a11f288e9a) 

Whether to chicken if another player hostiles the controlled character.

#### `public bool `[`blockKeys`](#classMe_1a32b0fe4966c613767100bca03d4cff54) 

Whether keyboard input to the game window is blocked.

While true, keyboard input to the game window is discarded before it reaches the game, the D2BS console (including the Home key) or keydown/keyup event listeners. Alt+F4 still works. Keystrokes sent with sendKey() still reach the game. Defaults to false and is shared by all scripts. Only true or false is accepted, so other values are ignored. login() sets it to true while it runs and to false when it returns, whatever a script set before.

#### `public bool `[`blockMouse`](#classMe_1aa9360f70984b69d889ca7744a977d031) 

Whether mouse input to the game window is blocked.

While true, mouse input to the game window is discarded before the game handles it, and no mouse click/move events or screen-hook clicks are delivered; getMouseCoords() does not follow the real cursor meanwhile. Clicks on the title bar (and the left or top border) still pass through. Defaults to false and is shared by all scripts. Only true or false is accepted, so other values are ignored. login() sets it to true while it runs and to false when it returns, whatever a script set before.

#### `public bool `[`gameReady`](#classMe_1adbf3d1e8b502d34c5f6895bb2c40f1d8) 

Whether or not the game window is in the game.

#### `public String `[`profile`](#classMe_1a5321127db8f2c6ca21d1428fd753157d) 

The profile currently being used.

#### `public bool `[`nopickup`](#classMe_1a0a3973ed5dae29cbfe95dc7745646004) 

Whether the game is set to not pick up items or not.

#### `public double `[`pid`](#classMe_1ad10c9c74976842a1f9634410ad833744) 

The process id of Diablo II.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`screensize`](#classMe_1acf69e064892375059446599a1494fea6) 

The game's resolution mode.

A mode number, not a pixel size:

0 - 640x480 (in a game)

1 - 800x600 (at the out-of-game menus)

2 - 800x600 (in a game)

The game also maps 3 to 1344x700, but nothing in stock 1.14d is known to set it. Compare against 0 to detect 640x480 in a game; at the menus it reads 1, so a test for 2 there fails. Read-only.

#### `public String `[`windowtitle`](#classMe_1ae1fb424ddea108a0894dddafae0febbd) 

The window title.

#### `public bool `[`ingame`](#classMe_1a92d8763c52f3ac2ec1d680bd026e2404) 

True if the game is not at menu.

Unlike gameReady, it is also true while a game is loading, during act changes and on the loading screens between the menus and a game. Use gameReady to know the game can be acted on. Read-only.

#### `public bool `[`quitonerror`](#classMe_1a57806876c33f36d3bcc1ca4599fe60be) 

Whether a script error makes the client leave the current game.

When true, any script error (an uncaught exception or a compile error, not a warning) in any script, the console included, makes the client leave the current game and return to the menus; the in-game scripts end with the game as on any game exit. The error is still logged and printed. It has no effect at the menus or while the game is loading. The process keeps running. The default comes from QuitOnError in d2bs.ini (false if absent; a value starting with t, T or 1 is true). Only true or false is accepted, so other values such as 1 are ignored.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`maxgametime`](#classMe_1ad4f57667eb1a3bd8e45288c9f3fe923a) 

The max time in milliseconds to stay in game before the game auto-quits.

#### `public bool `[`getNext`](#classMe_1ac38694e80aec8caa323c50785816bcbd)`()` 

Get the next unit that matches the originally given name or class id and originally specified mode.

#### Returns
Whether another unit was found.

#### `public bool `[`getNext`](#classMe_1a978176a65b1a7d182cee4fb9384cb626)`(String szName)` 

Get the next unit that matches the given name and originally specified mode.

#### Parameters
* `szName` The name to look for.

#### Returns
Whether another unit was found.

#### `public bool `[`getNext`](#classMe_1afb7f7bf695192c13ab56b2fbe29f743e)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` dwClassId)` 

Get the next unit that matches the given class id and originally specified mode.

#### Parameters
* `dwClassId` The class id to look for.

#### Returns
Whether another unit was found.

#### `public bool `[`getNext`](#classMe_1a02f641e81f28741b2bef50c16d3372da)`(String szName,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` dwMode)` 

Get the next unit that matches the given name and mode.

#### Parameters
* `szName` The name to look for.

* `dwMode` Either the mode being searched for, or a bitmask with bit 29 set and bits 0-28 corresponding to modes of units being searched for.

#### Returns
Whether another unit was found.

#### `public bool `[`getNext`](#classMe_1a12feab5ac7792369015564b222441acd)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` dwClassId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` dwMode)` 

Get the next unit that matches the given class id and mode.

#### Parameters
* `dwClassId` The class id to look for.

* `dwMode` Either the mode being searched for, or a bitmask with bit 29 set and bits 0-28 corresponding to modes of units being searched for.

#### Returns
Whether another unit was found.

#### `public void `[`cancel`](#classMe_1a3d6d506ed810276b50ba86f9c3f4dd98)`()` 

Cancel some form of interaction.

If there is scrolling text, cancel it. If you are interacting with an NPC, stop interacting. If you have an item on your cursor, drop it. Otherwise call D2CLIENT_CloseInteract.

#### `public bool `[`repair`](#classMe_1ab3612cb5a103004b87a9321b5a7aec50)`()` 

Try to repair.

Need to be able to find unit that you're trying to repair with. That means the unit needs to still be in the server unit hash table.

#### Returns
true if successful.

#### `public bool `[`useMenu`](#classMe_1aa29c5eccaa458901d4ec804e4fa167ca)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` menuItem)` 

Use an NPC menu.

Need to be able to find unit that you're trying to repair with. That means the unit needs to still be in the server unit hash table.

#### Parameters
* `menuItem` Index of the menu item to click.

#### Returns
true if menu was found.

#### `public bool `[`interact`](#classMe_1a3341b12f791fa69c7ba048cc90778f20)`()` 

Interact with the unit.

If the unit is an item in your inventory or stash, the call uses it as a right-click does: a potion is drunk, a town portal tome or scroll opens a portal outside town, and an identify tome or scroll gives the identify cursor. An item in the belt is used the same way. Either use returns false, whether or not the game acts on it. Any other unit, an item anywhere else included, gets a left-button-down click on its map position that targets it, the same kind of click clickMap(0, 0, unit) makes, and the call returns true. Called on your own character, or on a unit that has left the game, it does nothing and returns false. Outside a ready game it returns false (with a 'Game not ready' warning).

#### `public bool `[`interact`](#classMe_1a4742f918ff80aa898da57756b1a95fbd)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` destId)` 

Interact with a waypoint.

If the unit is an object, assume it's a waypoint, and take it.

#### Parameters
* `destId` The area id of the destination to select.

#### `public `[`Unit`](#classUnit)` `[`getItem`](#classMe_1ab6277b88eedb3985e94a7b2ec2d0ed5e)`(String name,uint32_t mode,uint32_t nUnitId)` 

Get an item from inventory by name, mode and nUnitId.

#### Parameters
* `name` The name of the unit to look for.

* `mode` Either the mode being searched for, or a bitmask with bit 29 set and bits 0-28 corresponding to modes of units being searched for.

* `nUnitId` The unique id of the unit.

#### Returns
The first item found that matches the description.

#### `public `[`Unit`](#classUnit)` `[`getItem`](#classMe_1a1678e3178379630e2b4208e011129094)`(uint32_t classId,uint32_t mode,uint32_t nUnitId)` 

Get an item from inventory by classId, mode and nUnitId.

#### Parameters
* `classId` The class id of the unit.

* `mode` Either the mode being searched for, or a bitmask with bit 29 set and bits 0-28 corresponding to modes of units being searched for.

* `nUnitId` The unique id of the unit.

#### Returns
The first item found that matches the description.

#### `public `[`Unit`](#classUnit)` `[`getItems`](#classMe_1aa3a172ada3c1e477299cf697f3722076)`()` 

Get all items from inventory.

#### Returns
An array of all the items from inventory; empty when the unit has no items, has left the game, or the method is called on something that is not a unit.

#### `public `[`Unit`](#classUnit)` `[`getMerc`](#classMe_1a12c1ae747f947cc96bcda92d517e4fd4)`()` 

Get a player's merc.

The unit being operated on should be a player.

#### Returns
The merc unit, or null when none is found (the merc must be in a room the client has loaded) or the unit is not a player. Any argument is ignored: getMerc(true) returns the same unit or null, so test the result for truthiness. Outside a ready game it returns false (with a 'Game not ready' warning).

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getMercHP`](#classMe_1a8b9d83bd2cc1d39c89c47c2f3d97f398)`()` 

Get the merc's HP.

The reported HP is a percentage.

#### Returns
The merc's HP percentage, 0 while the merc is dead, undefined when there is no merc, or false when the game is not ready.

#### `public String `[`getSkill`](#classMe_1a449dad9a671272e94c7df7ab07277043)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` hand)` 

Get skill name from hand.

#### Parameters
* `hand` Hand to get the skill name off.

0 - Right hand

1 - Left hand

#### Returns
The skill's in-game name in the client's language. It is not always a name setSkill() accepts; getSkill(2) and getSkill(3) give the ids. Returns false when the unit has left the game.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getSkill`](#classMe_1a482e347e3d2131af87a6aca7019bf4fd)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` hand)` 

Get skill id from hand.

#### Parameters
* `hand` Hand to get the skill id off.

2 - Right hand

3 - Left hand

#### Returns
Skill id.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getSkill`](#classMe_1a68d6118cef94eb973cfa1591ea890e3f)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` dummy)` 

Get all skills.

Gets all skill ids, along with corresponding base and total skill levels.

#### Parameters
* `dummy` Should be 4.

#### Returns
An array of arrays of integers. Inside the inner array the 0th index is the skill id, the 1st index is the base skill level and the 2nd index is the total skill level.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getSkill`](#classMe_1a8319dc883867a8e9f6cc101ecbee0aa5)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSkillId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nExt)` 

Get the base or total level of a skill.

With two arguments, only the unit's own (non-charged) entry for the skill is read. The total adds the bonuses the game applies to that skill: +all skills, +class skills, +skill tab, +single skill, +elemental skills, oskills and temporary bonuses such as the Skill shrine. Oskills add at most 3 to a player's own class skills. Skills that come from item charges get no bonuses. The result, base or total, is never below 0 and never above 99.

Pass a boolean third argument to choose the entry: true reads the skill from an item with charges (the charge level), false reads the first entry with that id, charged or not.

#### Parameters
* `nSkillId` The skill id. See [Skill ids](#skill-ids) in the reference tables.

* `nExt` 0 for the base level (the hard points put into the skill), any other integer (usually 1) for the total level.

#### Returns
The skill level. Returns false when the unit does not have the skill, when nSkillId or nExt is not an integer, when a third argument is passed that is not a boolean, or when the unit has left the game. Outside a ready game it returns false (with a 'Game not ready' warning).

#### `public `[`Unit`](#classUnit)` `[`getParent`](#classMe_1a211afa5299fc94407dc2a6d78ebadd8c)`()` 

Gets the parent of a unit.

This function is used for monster and item units.

#### Returns
The parent unit.

#### `public String `[`getParent`](#classMe_1a4019ee0bef41f98dd4170a205de08cd7)`()` 

Gets the parent of a unit.

This function is used for object units.

#### Returns
The parent's name.

#### `public void `[`overhead`](#classMe_1a6f5c4067c0304c88ab5105b93e787f7d)`(Object message)` 

Puts the string equivalent of message over the unit.

Throws "Invalid arguments" if no message is passed for a unit that is still in the game; a null or undefined message shows nothing.

#### `public void `[`revive`](#classMe_1a7e53744fc9cd8233451fa8562e4ce7e9)`()` 

Revive the character.

BE CAREFUL! This function directly sends packets without checks. If you call this function and are not dead, you might get flagged/banned.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getFlags`](#classMe_1a2901a55f39537ba49f5c90cfb8efe1a3)`()` 

Returns the item flags.

#### Returns
Item flags:

0x00000001 - Equipped

0x00000008 - In socket

0x00000010 - Identified

0x00000040 - Weapon/shield is in the active weapon switch

0x00000080 - Weapon/shield is in the inactive weapon switch

0x00000100 - Item is broken

0x00000400 - Full rejuv

0x00000800 - Socketed

0x00002000 - In the trade or gamble screen

0x00004000 - Not in a socket

0x00010000 - Is an ear

0x00020000 - A starting item (worth only 1 gold)

0x00200000 - Rune, or potion, or mephisto's soulstone

0x00400000 - Ethereal

0x00800000 - Is an item

0x01000000 - Personalized

0x04000000 - Runeword

Source: [http://subversion.assembla.com/svn/d2bs/scripts/YAMB/libs/YAMB/common/YAM-Common.dbl](http://subversion.assembla.com/svn/d2bs/scripts/YAMB/libs/YAMB/common/YAM-Common.dbl) r1086

#### `public double `[`getStat`](#classMe_1a1d7999f11e2a07f70dc69432b06ba782)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nStat,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSubIndex)` 

Get a stat by stat id and sub index.

Life, mana and stamina (stats 6-11) come back in whole points. Experience (stats 13, 29 and 30) is read unsigned, so values of 2^31 and above stay positive. On an item, stat 92 returns the item's level requirement for your character (the same value as the itemlevelreq property) and ignores nSubIndex. On any other unit, your own character included, stat 92 does not read a stat: the game's item level-requirement calculation runs on that unit and returns a number with no fixed meaning. Any other stat returns the game's total for that stat and subindex; when that total is 0, the value of that stat and subindex in the stat list that getStat(-1) reads (not the base stats getStat(-1) adds) is returned instead (0 if it is not there). Arguments that are not integers are converted to integers.

#### Parameters
* `nStat` The stat id (the ID column of ItemStatCost.txt).

* `nSubIndex` A subindex to a certain type of stat. For instance a specific skill for the +1 to skill tab stat. Defaults to 0.

#### Returns
The stat value. Returns false when no stat id is passed or the unit has left the game. Outside a ready game it returns false (with a 'Game not ready' warning).

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getStat`](#classMe_1a34c73953d580d9eae5a32d67d26a7ea0)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nStat,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSubIndex)` 

Get an array of the unit's listed stats.

The array holds the entries of the unit's stat list plus any of the unit's base stats not already in it. It is not every stat the unit has. Values are raw: life, mana and stamina are in 1/256 points, and values of 2^31 and above (such as high experience) come back negative.

#### Parameters
* `nStat` Set to -1.

* `nSubIndex` Ignored.

#### Returns
An array of arrays. The indices of the inner array are: 0 - stat id, 1 - subindex, 2 - value. Returns false when the unit has no such stat list, even if it has base stats. Outside a ready game it returns false (with a 'Game not ready' warning).

#### `public Object[] `[`getStat`](#classMe_1getStat_byId)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nStat,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSubIndex)` 

Get the unit's stats as an array indexed by stat id.

Each element is an array indexed by subindex that holds the value, with life, mana and stamina (stats 6-11) in whole points. Other values are raw, so values of 2^31 and above (such as high experience) come back negative. The values come from the unit's base stats, its stat totals (the values the single-stat form reads) and the stat list that getStat(-1) reads, in that order; where more than one holds the same stat and subindex, the last one wins, so a value here is not always the one the single-stat form returns.

A stat that carries a skill (for example a charged skill or a chance-to-cast skill such as one cast on striking) holds an object {skill, level} instead, for skill ids 8 and up; lower skill ids come back as ordinary entries. A charged skill also gets charges and maxcharges, except on an item with a maximum of 1 charge, or of 2 charges with none left. When the same stat id occurs more than once, the element is an array of such objects. Each entry comes back once, so a stat with a single skill holds a bare object, not an array; check an element with Array.isArray. When the stat also holds an ordinary entry for a lower skill id, the element is a subindex array instead, with the skill objects after its last index. Oskill mods (stat 97) are ordinary entries: element 97 is indexed by skill id and holds the bonus level.

#### Parameters
* `nStat` Set to -2.

* `nSubIndex` Ignored.

#### Returns
A sparse array indexed by stat id. Outside a ready game it returns false (with a 'Game not ready' warning).

#### `public bool `[`getState`](#classMe_1a6a8f7eed0b6808691957c24f79f8e111)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nState)` 

Return whether or not the unit has a given state.

Objects, items and tiles always return false: the game's state check answers false for them.

#### Parameters
* `nState` The state id: the id column of States.txt, 0-183. State 184 (uberminion) exists in 1.14d but getState rejects it and returns false.

Throws "Invalid arguments" if no state id is passed. Returns false when the unit has left the game, or when nState is not an integer or is out of range. Outside a ready game it returns false (with a 'Game not ready' warning).

#### Returns
Whether or not the unit has the state.

#### `public bool `[`getEnchant`](#classMe_1a2b35f9abde10c9fe6ba08b6e2c31ee3c)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nEnchant)` 

Determine whether or not a unit has a given enchant.

#### Parameters
* `nEnchant` Enchantment id.

#### Returns
Whether or not the unit has the enchantment.

#### `public bool `[`shop`](#classMe_1a3469ed7606e573d7e09e197999ae494e)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` dwMode)` 

Shop with a given npc, either buying or selling.

#### Parameters
* `dwMode` What to do with the item. 1 - Sell, 2 - Buy, 6 - ?

#### Returns
Whether or not the shop went through.

#### `public bool `[`setSkill`](#classMe_1aa6e655c613c557c0ef44cfb8ad536cde)`(String skillName,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nHand)` 

Set the skill on the given hand to be skill with name skillName.

Asks the game to put the skill on the hand, then waits up to about one second for the hand to show it. When the hand already shows the skill, the call returns true at once. The call always acts on your own character, whichever unit it is called on.

The name is matched case-insensitively against D2BS's built-in English skill-name table, not the game's localized strings. "Shadow Master" and "Phoenix Strike" are not recognized by name; pass their ids (279 and 280) instead.

#### Parameters
* `skillName` Name of the skill to put up.

* `nHand` Hand to put the skill on: 0 - right, any other integer - left.

#### Returns
true once the hand shows the skill. Returns false when the skill name is unknown, nHand is missing or not an integer, your character does not have the skill (total level 0), you are not in a game, or the hand does not show the skill in time.

#### `public bool `[`setSkill`](#classMe_1aa8850569751822ba3088f40bfd94584a)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSkillId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nHand)` 

Set the skill on the given hand to be skill with id nSkillId.

Asks the game to put the skill on the hand, then waits up to about one second for the hand to show it. When the hand already shows the skill, the call returns true at once. The call always acts on your own character, whichever unit it is called on.

#### Parameters
* `nSkillId` Id of the skill to put up. See [Skill ids](#skill-ids) in the reference tables.

* `nHand` Hand to put the skill on: 0 - right, any other integer - left.

#### Returns
true once the hand shows the skill. Returns false when the first argument is neither a string nor an integer, nHand is missing or not an integer, your character does not have the skill (total level 0), you are not in a game, or the hand does not show the skill in time.

#### `public bool `[`setSkill`](#classMe_1af20eafc6d55f4dabcba4fdcf7f8bae40)`(String skillName,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nHand,`[`Unit`](#classUnit)` item)` 

Set the skill on the given hand to be skill with name skillName.

Asks the game to put the skill on the hand, then waits up to about one second for the hand to show it. When the hand already shows the skill, the call returns true at once. The call always acts on your own character, whichever unit it is called on.

The name is matched case-insensitively against D2BS's built-in English skill-name table, not the game's localized strings. "Shadow Master" and "Phoenix Strike" are not recognized by name; pass their ids (279 and 280) instead.

#### Parameters
* `skillName` Name of the skill to put up.

* `nHand` Hand to put the skill on: 0 - right, any other integer - left.

* `item` An item unit: the hand gets that item's charged skill. A third argument that is not an item unit is ignored, and the skill is set as a normal skill. The item is not checked to be yours.

#### Returns
true once the hand shows the skill. Returns false when the skill name is unknown, nHand is missing or not an integer, your character does not have the skill (total level 0), you are not in a game, or the hand does not show the skill in time.

#### `public bool `[`setSkill`](#classMe_1a10a595cd98b1bb7fcc54bfebdd61ac6c)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSkillId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nHand,`[`Unit`](#classUnit)` item)` 

Set the skill on the given hand to be skill with id nSkillId.

Asks the game to put the skill on the hand, then waits up to about one second for the hand to show it. When the hand already shows the skill, the call returns true at once. The call always acts on your own character, whichever unit it is called on.

#### Parameters
* `nSkillId` Id of the skill to put up. See [Skill ids](#skill-ids) in the reference tables.

* `nHand` Hand to put the skill on: 0 - right, any other integer - left.

* `item` An item unit: the hand gets that item's charged skill. A third argument that is not an item unit is ignored, and the skill is set as a normal skill. The item is not checked to be yours.

#### Returns
true once the hand shows the skill. Returns false when the first argument is neither a string nor an integer, nHand is missing or not an integer, your character does not have the skill (total level 0), you are not in a game, or the hand does not show the skill in time.

#### `public void `[`move`](#classMe_1a497c1507b060c2fcbdf79fcb7263ebdd)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y)` 

Move to the given location.

#### Parameters
* `x` The x location.

* `y` The y location.

#### `public void `[`move`](#classMe_1a2ab43203c0909238f2b721db7fda48a5)`()` 

Move to this unit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getQuest`](#classMe_1a9ba75f0259540bc166cf7b8cbdbade86)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nAct,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nQuest)` 

Get the quest flag for a quest specified by act and quest number.

#### Parameters
* `nAct` The act of the quest.

* `nQuest` The quest number.

#### Returns
The quest flag for the specified quest, or undefined when act and quest are not both integers.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getMinionCount`](#classMe_1accccb22ea6e3a2618fe740270374d6e6)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nType)` 

Get the number of minions of a certain type.

#### Parameters
* `nType` The type of the minions.

#### Returns
The number of minions of the specified type.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getRepairCost`](#classMe_1a79e916f09dcbe81785c07c3c6bb9505f)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nNpcClassId)` 

Get price to repair this unit at the current npc given by nNpcClassId.

#### Parameters
* `nNpcClassId` The class id of the npc to get the price for repair at.

#### Returns
The price to repair the given unit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getItemCost`](#classMe_1a5589dba170b74d8af4e1c0dd5b176626)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode,`[`Unit`](#classUnit)` npc)` 

Get the cost to do something (buy, sell, repair) with the given item, at the given npc.

#### Parameters
* `nMode` What to do: 0 - buy, 1 - sell, 2 - repair.

* `npc` The npc to check the price with.

#### Returns
The price.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getItemCost`](#classMe_1abcb1d466bcdac846dbb10c7ba4724caf)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode,`[`Unit`](#classUnit)` npc,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nDifficulty)` 

Get the cost to do something (buy, sell, repair) with the given item, at the given npc, in the given difficulty.

#### Parameters
* `nMode` What to do: 0 - buy, 1 - sell, 2 - repair.

* `npc` The npc to check the price with.

* `nDifficulty` The difficulty to check the price in.

#### Returns
The price.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getItemCost`](#classMe_1ad2deea07e13b5acaa68d0c09bbe75698)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nNpcClassId)` 

Get the cost to do something (buy, sell, repair) with the given item, at the given npc.

#### Parameters
* `nMode` What to do: 0 - buy, 1 - sell, 2 - repair.

* `nNpcClassId` The class id of the npc to check the price with.

#### Returns
The price.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getItemCost`](#classMe_1a81997bd2784832f7e1a88863483e0289)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nNpcClassId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nDifficulty)` 

Get the cost to do something (buy, sell, repair) with the given item, at the given npc, in the given difficulty.

#### Parameters
* `nMode` What to do: 0 - buy, 1 - sell, 2 - repair.

* `nNpcClassId` The class id of the npc to check the price with.

* `nDifficulty` The difficulty to check the price in.

#### Returns
The price.

# class `Party` 

Represents the items in the list in the [Party](#classParty) screen.

Also called a RosterUnit.

A [Party](#classParty) object follows its player and reads the current values on every access. Once that player leaves the game, or the game ends, its properties read undefined, getNext() returns false and clickParty() returns false.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classParty_1a2152cde81a80e8292d45a02cf5fd1f81) | The x coordinate of the location of the RosterUnit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classParty_1a7b27f3dc2f2cf8428bcf88369f974644) | The y coordinate of the location of the RosterUnit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`area`](#classParty_1a673a0ec1ad99bdc49b0cab8fed947876) | The [Area](#classArea) ID of the RosterUnit.
`public double `[`gid`](#classParty_1a00ed094a817469dfb8cbdebabcdd95a7) | The unique id of the RosterUnit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`life`](#classParty_1a61a88341743f372a3b9556e528c7403c) | The life of the RosterUnit as a percentage of its maximum life, 0 to 100.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`partyflag`](#classParty_1aed25455e977af2f00d59dc34bae5adab) | The state of the party button for this player, as your character sees it.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`partyid`](#classParty_1abc7e22a4373225cef75b49598ae4ef10) | The id of the party this player belongs to, or 65535 when they are in no party.
`public String `[`name`](#classParty_1a82db2ed7f8d2d3e5ed7dfb1210098d57) | The name of the RosterUnit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`classid`](#classParty_1ac5ec74e94715df07ca680b1247074627) | The class ID of the RosterUnit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`level`](#classParty_1ad32c6abaa6e2f25660b7e00a12f1c7a4) | The character level of the RosterUnit.
`public `[`Party`](#classParty)` `[`getNext`](#classParty_1a1f0c9289dac815268bc3662a477aa397)`()` | Move to the next RosterUnit.

## Members

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classParty_1a2152cde81a80e8292d45a02cf5fd1f81) 

The x coordinate of the location of the RosterUnit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classParty_1a7b27f3dc2f2cf8428bcf88369f974644) 

The y coordinate of the location of the RosterUnit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`area`](#classParty_1a673a0ec1ad99bdc49b0cab8fed947876) 

The [Area](#classArea) ID of the RosterUnit.

#### `public double `[`gid`](#classParty_1a00ed094a817469dfb8cbdebabcdd95a7) 

The unique id of the RosterUnit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`life`](#classParty_1a61a88341743f372a3b9556e528c7403c) 

The life of the RosterUnit as a percentage of its maximum life, 0 to 100.

It is kept current only for members of your party; for other players it can be stale, and your own entry is not reliable either. The roster holds neither the player's absolute life nor their maximum life, so there is no maximum-life field. For your own character, use me.hp and me.hpmax.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`partyflag`](#classParty_1aed25455e977af2f00d59dc34bae5adab) 

The state of the party button for this player, as your character sees it.

The value is a single state, not a set of bits:

| Value | Meaning |
|---|---|
| 0 | The player is in no party and has no pending invite: clickParty(party, 2) invites them |
| 1 | The player is already in a party (yours or another; compare partyid to tell which); your own entry also reads 1. clickParty(party, 2) returns false without acting |
| 2 | The player has invited you: clickParty(party, 2) accepts and you join |
| 4 | You have invited the player and they have not answered: clickParty(party, 2) cancels the invite |

The game's client also accepts 3 (clicking acts like accepting an invite); 1.14d's own server never sends it, and whether a Battle.net realm does is unknown.

clickParty(party, 2) also returns false for your own entry and for a member of your own party.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`partyid`](#classParty_1abc7e22a4373225cef75b49598ae4ef10) 

The id of the party this player belongs to, or 65535 when they are in no party.

The game numbers each party; two players are in the same party exactly when their partyid values are equal and not 65535. To list your party, compare each entry's partyid with your own (getParty(me).partyid). Your own entry reads 65535 as soon as you leave a party.

#### `public String `[`name`](#classParty_1a82db2ed7f8d2d3e5ed7dfb1210098d57) 

The name of the RosterUnit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`classid`](#classParty_1ac5ec74e94715df07ca680b1247074627) 

The class ID of the RosterUnit.

See Unit::classId.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`level`](#classParty_1ad32c6abaa6e2f25660b7e00a12f1c7a4) 

The character level of the RosterUnit.

#### `public `[`Party`](#classParty)` `[`getNext`](#classParty_1a1f0c9289dac815268bc3662a477aa397)`()` 

Move to the next RosterUnit.

#### Returns
This object, now on the next player; or false at the end of the list, or when this object's player has left or its game has ended.

# class `PresetUnit` 

Represents a [PresetUnit](#classPresetUnit) (a type of [Unit](#classUnit) that is available as soon as the [Area](#classArea) data is available).

A preset unit's position is stored relative to its room. Its game map coordinates are x: roomx * 5 + x and y: roomy * 5 + y. roomx and roomy are the room's position in room tiles (5 map units each, the same values as [Room::x](#classRoom_1a7e5d5a363ed3c93efc6b1c321f0ec957) and [Room::y](#classRoom_1a8bfbccf2843aab2175743f8d9b5ffe48)), and x and y are the unit's offset in map units from the room's top-left (minimum x and y) corner.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`type`](#classPresetUnit_1aeb44ea9243dc41da4cfa2796ae14e276) | The type ID.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`roomx`](#classPresetUnit_1ad3178939e97c6a2bcadff92ee6e8dbba) | The X position of the room the [PresetUnit](#classPresetUnit) is in, in room tiles of 5 map units (the same value as [Room::x](#classRoom_1a7e5d5a363ed3c93efc6b1c321f0ec957)).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`roomy`](#classPresetUnit_1a661012095ec30341597c37e437bc60b0) | The Y position of the room the [PresetUnit](#classPresetUnit) is in, in room tiles of 5 map units (the same value as [Room::y](#classRoom_1a8bfbccf2843aab2175743f8d9b5ffe48)).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classPresetUnit_1a52f264129d70ba87ae8e7de714264994) | The X offset in map units from the room's corner.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classPresetUnit_1a6d875aeb1be7022712bcba59ba256733) | The Y offset in map units from the room's corner.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`id`](#classPresetUnit_1af1a516a03965b9e23cf2a4ab77bf4f62) | The unique ID of the [PresetUnit](#classPresetUnit).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`level`](#classPresetUnit_1aa3ae3f2724bf489f6341dc905537623f) | The [Area](#classArea) ID of the [Area](#classArea) the [PresetUnit](#classPresetUnit) is in.

## Members

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`type`](#classPresetUnit_1aeb44ea9243dc41da4cfa2796ae14e276) 

The type ID.

See [Unit::type](#classUnit_1a45243122ae655f50179f96bc831bfb1d).

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`roomx`](#classPresetUnit_1ad3178939e97c6a2bcadff92ee6e8dbba) 

The X position of the room the [PresetUnit](#classPresetUnit) is in, in room tiles of 5 map units (the same value as [Room::x](#classRoom_1a7e5d5a363ed3c93efc6b1c321f0ec957)).

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`roomy`](#classPresetUnit_1a661012095ec30341597c37e437bc60b0) 

The Y position of the room the [PresetUnit](#classPresetUnit) is in, in room tiles of 5 map units (the same value as [Room::y](#classRoom_1a8bfbccf2843aab2175743f8d9b5ffe48)).

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classPresetUnit_1a52f264129d70ba87ae8e7de714264994) 

The X offset in map units from the room's corner.

Add roomx * 5 for the map X coordinate.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classPresetUnit_1a6d875aeb1be7022712bcba59ba256733) 

The Y offset in map units from the room's corner.

Add roomy * 5 for the map Y coordinate.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`id`](#classPresetUnit_1af1a516a03965b9e23cf2a4ab77bf4f62) 

The unique ID of the [PresetUnit](#classPresetUnit).

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`level`](#classPresetUnit_1aa3ae3f2724bf489f6341dc905537623f) 

The [Area](#classArea) ID of the [Area](#classArea) the [PresetUnit](#classPresetUnit) is in.

# class `Profile` 

This class represents the details needed to get a player to a game or lobby.

This class hides the password. It provides the [login()](#classProfile_1a0c16e26d243cf44ccac630f61d14aec5) method to use the password.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public ProfileType `[`type`](#classProfile_1a55544294c83a6be6c9cfe9910f05e48a) | The type of profile, see [ProfileType](#namespaceProfileType).
`public String `[`ip`](#classProfile_1a7e526a7e2ef6fd606ab649a1b88bde35) | The IP of the game to connect to (TCP/IP join).
`public String `[`username`](#classProfile_1a3960b8797bca1eb38131b3df66718e0c) | The username to use to connect to battle.net.
`public union Profile::@0 `[`gateway`](#classProfile_1a4329f122e8a7442ca2efbba71c060f69) | The battle.net gateway to connect to.
`public String `[`character`](#classProfile_1a51ec68f5f69972a1b1557f5499baf0e0) | The character to select.
`public int8_t `[`difficulty`](#classProfile_1aa5eea6e6a7396dd2ae9f89ad0801d544) | The difficulty to use: 0 - normal, 1 - nightmare, 2 - hell.
`public uint32_t `[`maxLoginTime`](#classProfile_1a8c0867890cacb3b91987b41a1fbaa3c1) | The number of milliseconds to wait while logging in.
`public uint32_t `[`maxCharacterSelectTime`](#classProfile_1ab3927b4008bcba2195125993048bf030) | The number of milliseconds to wait for the characters to load.
`public  `[`Profile`](#classProfile_1a4eea708cdbed0262d9e7ddbe3f1e3f89)`()` | Create a [Profile](#classProfile) for the currently set profile.
`public  `[`Profile`](#classProfile_1acdcb14b34cd75227369ff5ad2e62e766)`(String profileName)` | Create a [Profile](#classProfile) for the named profile.
`public  `[`Profile`](#classProfile_1a98db02707424bffe71d5bbec6c52cf09)`(ProfileType pt,String charName,int8_t diff)` | Create a [Profile](#classProfile) with the given information.
`public  `[`Profile`](#classProfile_1ab4663dc868fc955a8bc97fe96aae969c)`(ProfileType pt,String account,String pass,String charName,String gateway)` | Create a [Profile](#classProfile) with the given information.
`public  `[`Profile`](#classProfile_1a79565a75e65ed00a878e44e47a3ec1c4)`(ProfileType pt,String charName,String ip)` | Create a [Profile](#classProfile) with the given information.
`public void `[`login`](#classProfile_1a0c16e26d243cf44ccac630f61d14aec5)`()` | Log the profile in.

## Members

#### `public ProfileType `[`type`](#classProfile_1a55544294c83a6be6c9cfe9910f05e48a) 

The type of profile, see [ProfileType](#namespaceProfileType).

#### `public String `[`ip`](#classProfile_1a7e526a7e2ef6fd606ab649a1b88bde35) 

The IP of the game to connect to (TCP/IP join).

#### `public String `[`username`](#classProfile_1a3960b8797bca1eb38131b3df66718e0c) 

The username to use to connect to battle.net.

#### `public union Profile::@0 `[`gateway`](#classProfile_1a4329f122e8a7442ca2efbba71c060f69) 

The battle.net gateway to connect to.

#### `public String `[`character`](#classProfile_1a51ec68f5f69972a1b1557f5499baf0e0) 

The character to select.

#### `public int8_t `[`difficulty`](#classProfile_1aa5eea6e6a7396dd2ae9f89ad0801d544) 

The difficulty to use: 0 - normal, 1 - nightmare, 2 - hell.

#### `public uint32_t `[`maxLoginTime`](#classProfile_1a8c0867890cacb3b91987b41a1fbaa3c1) 

The number of milliseconds to wait while logging in.

#### `public uint32_t `[`maxCharacterSelectTime`](#classProfile_1ab3927b4008bcba2195125993048bf030) 

The number of milliseconds to wait for the characters to load.

#### `public  `[`Profile`](#classProfile_1a4eea708cdbed0262d9e7ddbe3f1e3f89)`()` 

Create a [Profile](#classProfile) for the currently set profile.

#### `public  `[`Profile`](#classProfile_1acdcb14b34cd75227369ff5ad2e62e766)`(String profileName)` 

Create a [Profile](#classProfile) for the named profile.

#### Parameters
* `profileName` Name of the profile to load.

#### `public  `[`Profile`](#classProfile_1a98db02707424bffe71d5bbec6c52cf09)`(ProfileType pt,String charName,int8_t diff)` 

Create a [Profile](#classProfile) with the given information.

#### Parameters
* `pt` The type of profile, this constructor handles singlePlayer and tcpIpHost types.

* `charName` Name of the character.

* `diff` Difficulty, 0 - normal, 1 - nightmare, 2 - hell.

#### `public  `[`Profile`](#classProfile_1ab4663dc868fc955a8bc97fe96aae969c)`(ProfileType pt,String account,String pass,String charName,String gateway)` 

Create a [Profile](#classProfile) with the given information.

#### Parameters
* `pt` The type of profile, this constructor handles battleNet and openBattleNet types.

* `account` Account name.

* `pass` Password.

* `charName` Name of the character.

* `gateway` Search string for the gateway (any substring of the desired gateway, will take first gateway that matches).

#### `public  `[`Profile`](#classProfile_1a79565a75e65ed00a878e44e47a3ec1c4)`(ProfileType pt,String charName,String ip)` 

Create a [Profile](#classProfile) with the given information.

#### Parameters
* `pt` The type of profile, this constructor handles singlePlayer and tcpIpHost types.

* `charName` Name of the character.

* `ip` IP address to connect to.

#### `public void `[`login`](#classProfile_1a0c16e26d243cf44ccac630f61d14aec5)`()` 

Log the profile in.

# class `Room` 

Represents a "Room" in the game.

A [Room](#classRoom) is a rectangular piece of the map data. Its methods reach the room's lists, such as its [PresetUnit](#classPresetUnit) s.

A [Room](#classRoom) keeps its area, position and size, so its properties keep their values while a script holds it, until getNext() moves it to the next room. Its methods look the room up again on every call, in the game the room came from and the player's current act, wherever getRoom() could find it. When they can't, getNext() returns false, getNearby() returns an empty array, getStat() returns null and the other methods return undefined. unitInRoom() compares the unit's room with this one instead: it returns false when the unit is in another room, act or game, and undefined when the unit can't be found or an act change is still loading.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`number`](#classRoom_1ace0776823076279027a43e37096f44a2) | The LvlPrest.txt row (Def column) of the preset map the [Room](#classRoom) was built from.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classRoom_1a7e5d5a363ed3c93efc6b1c321f0ec957) | Get the x coordinate of the [Room](#classRoom).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classRoom_1a8bfbccf2843aab2175743f8d9b5ffe48) | Get the y coordinate of the [Room](#classRoom).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`xsize`](#classRoom_1a7c90b0e176a112beb7e8dcc808576e8b) | Get the x size (width) of the [Room](#classRoom).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`ysize`](#classRoom_1ae4dbb7c19fc3faa8e876c03aa8ed7fb9) | Get the y size (height) of the [Room](#classRoom).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`subnumber`](#classRoom_1adf03f03ffb94895c18f291a325e51fb2) | No implementation.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`area`](#classRoom_1a0e12c557e6658e021fc94dfe950ace67) | Get the [Area](#classArea) ID for the area which the [Room](#classRoom) is in.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`level`](#classRoom_1a5d8cf5f2d71723e3244654603a5ae84f) | Get the [Area](#classArea) ID for the area which the [Room](#classRoom) is in.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`correcttomb`](#classRoom_1af1e1126e1534f1653ce6b5fcd77026a4) | The [Area](#classArea) ID (66 to 72) of the true Tal Rasha's Tomb in the current game, the one with the Orifice leading to Tal Rasha's Chamber (area 73).
`public bool `[`getNext`](#classRoom_1a3dc890fc3d954ae8bd4c6da5721ed0ac)`()` | Move to the next [Room](#classRoom).
`public bool `[`reveal`](#classRoom_1a8b1a054e31adb584622cafc321e9e064)`()` | Reveal the [Room](#classRoom) on the map.
`public `[`PresetUnit`](#classPresetUnit)` `[`getPresetUnits`](#classRoom_1a0ddb9c16273a4e9d11365b9f927e74aa)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nType,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nClass)` | Get the [PresetUnit](#classPresetUnit) s from the current [Room](#classRoom).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getCollision`](#classRoom_1af6cedb196d4c1f1ad52e4f9076593248)`()` | Get the collision data for the [Room](#classRoom).
`public `[`Room`](#classRoom)` `[`getNearby`](#classRoom_1a43d7c3ae8cf396e9883e79c5d5a4b902)`()` | Get the "nearby" Rooms from pRoom2Near member.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getStat`](#classRoom_1aa43b095fa97e00fd593d949b354ba920)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nStat)` | Get some property of the [Room](#classRoom).
`public `[`Room`](#classRoom)` `[`getFirst`](#classRoom_1ad9a6e43e97d1f9368eeecc578dcd6e26)`()` | Get the first [Room](#classRoom) of the level this [Room](#classRoom) is on.
`public bool `[`unitInRoom`](#classRoom_1aad7e5367decaea3e683b2c51a8e43bb8)`(`[`Unit`](#classUnit)` unit)` | Determine whether or not the [Unit](#classUnit) is in the [Room](#classRoom).

## Members

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`number`](#classRoom_1ace0776823076279027a43e37096f44a2) 

The LvlPrest.txt row (Def column) of the preset map the [Room](#classRoom) was built from.

This includes the pieces of randomly generated dungeons and caves. A large preset map is cut into several rooms, which all read the same number. It is -1 for rooms of outdoor areas that were not built from a preset map; an outdoor area can also hold rooms built from a preset map, so not every room of an outdoor area reads -1.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classRoom_1a7e5d5a363ed3c93efc6b1c321f0ec957) 

Get the x coordinate of the [Room](#classRoom).

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classRoom_1a8bfbccf2843aab2175743f8d9b5ffe48) 

Get the y coordinate of the [Room](#classRoom).

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`xsize`](#classRoom_1a7c90b0e176a112beb7e8dcc808576e8b) 

Get the x size (width) of the [Room](#classRoom).

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`ysize`](#classRoom_1ae4dbb7c19fc3faa8e876c03aa8ed7fb9) 

Get the y size (height) of the [Room](#classRoom).

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`subnumber`](#classRoom_1adf03f03ffb94895c18f291a325e51fb2) 

No implementation.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`area`](#classRoom_1a0e12c557e6658e021fc94dfe950ace67) 

Get the [Area](#classArea) ID for the area which the [Room](#classRoom) is in.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`level`](#classRoom_1a5d8cf5f2d71723e3244654603a5ae84f) 

Get the [Area](#classArea) ID for the area which the [Room](#classRoom) is in.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`correcttomb`](#classRoom_1af1e1126e1534f1653ce6b5fcd77026a4) 

The [Area](#classArea) ID (66 to 72) of the true Tal Rasha's Tomb in the current game, the one with the Orifice leading to Tal Rasha's Chamber (area 73).

The game picks it from the act's map seed, so every [Room](#classRoom) in Act II reads the same value. It is undefined on rooms in other acts. See [Area ids](#area-ids) in the reference tables.

#### `public bool `[`getNext`](#classRoom_1a3dc890fc3d954ae8bd4c6da5721ed0ac)`()` 

Move to the next [Room](#classRoom).

#### Returns
Whether there was another [Room](#classRoom) to move to or not.

#### `public bool `[`reveal`](#classRoom_1a8b1a054e31adb584622cafc321e9e064)`()` 

Reveal the [Room](#classRoom) on the map.

#### Returns
Whether or not the [Room](#classRoom) was successfully revealed.

#### `public `[`PresetUnit`](#classPresetUnit)` `[`getPresetUnits`](#classRoom_1a0ddb9c16273a4e9d11365b9f927e74aa)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nType,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nClass)` 

Get the [PresetUnit](#classPresetUnit) s from the current [Room](#classRoom).

#### Parameters
* `nType` The type of [Unit](#classUnit) to get, or 0 for any. See [Unit::type](#classUnit_1a45243122ae655f50179f96bc831bfb1d). A number or numeric string converts as `nType >>> 0` does; a result of 0, which a value that converts to NaN also gives, matches any.

* `nClass` The class id of the [Unit](#classUnit) (s) to find, or 0 for any. See Unit::classId. A number or numeric string converts as `nClass >>> 0` does; a result of 0, which a value that converts to NaN also gives, matches any.

#### Returns
An array of all the [PresetUnit](#classPresetUnit) s from the current [Room](#classRoom).

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getCollision`](#classRoom_1af6cedb196d4c1f1ad52e4f9076593248)`()` 

Get the collision data for the [Room](#classRoom).

The array holds one entry per map row, each an array with one value per map column: result[dy][dx] is the subtile at map offset dx in x and dy in y from the room's corner (the corner with the smallest x and y). That is the subtile at map coordinates (x * 5 + dx, y * 5 + dy), where x and y are this Room's x and y. A room whose data is not loaded is loaded for the call and unloaded again.

Each value is the game's 16-bit collision mask for that subtile, a set of flag bits. See [Collision flags](#collision-flags) in the reference tables for what each bit means and which ones getPath() treats as blocking.

#### Returns
An array with the collision data. The first dimension is the y coordinate, the second is the x coordinate. Returns undefined when the game is not ready, when the room is no longer in your current act or game, or when it has no collision data.

#### `public `[`Room`](#classRoom)` `[`getNearby`](#classRoom_1a43d7c3ae8cf396e9883e79c5d5a4b902)`()` 

Get the "nearby" Rooms from pRoom2Near member.

#### Returns
An array of the nearby Rooms.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getStat`](#classRoom_1aa43b095fa97e00fd593d949b354ba920)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nStat)` 

Get some property of the [Room](#classRoom).

#### Parameters
* `nStat` Which property to get (any number or numeric string, made a 32-bit integer as `nStat | 0` does, so 4.5 reads stat 4):

0 - xStart (room 1)

1 - yStart (room 1)

2 - xSize (room 1)

3 - ySize (room 1)

4 - posX (room 2)

5 - posY (room 2)

6 - sizeX (room 2)

7 - sizeY (room 2)

9 - posGameX (coll)

10 - posGameY (coll)

11 - sizeGameX (coll)

12 - sizeGameY (coll)

13 - posRoomX (coll)

14 - posRoomY (coll)

15 - sizeRoomX (coll)

16 - sizeRoomY (coll)

#### Returns
The stat value, or null with no argument, undefined or null, a value that converts to NaN, an unlisted stat, when the room's data can't be loaded, or for stats 9 to 16 when the room has no collision map.

#### `public `[`Room`](#classRoom)` `[`getFirst`](#classRoom_1ad9a6e43e97d1f9368eeecc578dcd6e26)`()` 

Get the first [Room](#classRoom) of the level this [Room](#classRoom) is on.

#### Returns
The first [Room](#classRoom).

#### `public bool `[`unitInRoom`](#classRoom_1aad7e5367decaea3e683b2c51a8e43bb8)`(`[`Unit`](#classUnit)` unit)` 

Determine whether or not the [Unit](#classUnit) is in the [Room](#classRoom).

#### Parameters
* `unit` The unit to check.

#### Returns
Whether or not the [Unit](#classUnit) is in the [Room](#classRoom).

# class `Socket` 

A TCP connection opened with `Socket.open(host, port)`. The constructor itself throws; use `Socket.open`.

## Hosts

`Socket.open` connects only to a host listed in the `Hosts` key of the `[settings]` section of d2bs.ini. The host must equal one whole entry, compared case-insensitively; separate entries with commas, semicolons, spaces or tabs (for example `Hosts=127.0.0.1, api.example.com`). With no `Hosts` key, every host is refused.

## Members

* `static Socket open(String host, int port)` Connect to host:port. Returns false when called with fewer than two arguments or when networking cannot start. Throws `Invalid hostname specified` when host is not a string or not listed in Hosts, `Could not convert parameter 2`, `Failed to create the socket`, `Cannot find host`, `Failed to connect` or `Failed to define the socket object`.
* `String read()` One receive of up to 10000 bytes, decoded as UTF-8; an empty string when the peer closed. Throws `Socket Error` on a network error and `Socket is closed` after close().
* `void send(String data)` Send data as UTF-8. Throws `Socket Error` when it cannot be sent, `Socket is closed` after close(), and when data is missing or not a string.
* `void close()` Close the connection; calling it again does nothing.
* `readable`, `writeable` 1 when the socket can be read or written now, 0 when not, -1 after close() or on a network error.

# class `SQLite` 

Class for dealing with an [SQLite](#classSQLite) database.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public String `[`path`](#classSQLite_1a5d99c8c883289592018dfd5ee94ea3ce) | Get the path used to open the database.
`public `[`DBStatement`](#classDBStatement)`[] `[`statements`](#classSQLite_1a30a088d06c85fa846e13dd87b1ac7ad7) | Get the statements from query() that are still open, from index 0.
`public bool `[`isOpen`](#classSQLite_1a82dd45ef884604bf40f70a16c9f6fbc6) | Whether or not the database is open.
`public double `[`lastRowId`](#classSQLite_1aff1af94823aa7b2de96f65993686feed) | Get the row id of the last inserted row (exact up to 2^53), or 0 when the database is not open.
`public double `[`changes`](#classSQLite_1a8eaeea285b896935182fdd334b605c72) | Get the number of rows modified/inserted/deleted by the last statement, or 0 when the database is not open.
`public  `[`SQLite`](#classSQLite_1a9520cbf360895c30909dfdac712a9aef)`(String path,bool autoOpen)` | Create a database object which can be used to open [SQLite](#classSQLite) databases and execute queries.
`public bool `[`execute`](#classSQLite_1a8d0c60072870fba816b974671996c2af)`(String queryString)` | Execute a query on the database.
`public `[`DBStatement`](#classDBStatement)` `[`query`](#classSQLite_1a8ebb32f52a5426ae36a7977c540feb32)`(String sql,...)` | Prepare sql on the database and bind the remaining arguments to its parameters in order: strings as text, numbers as integers or reals, true/false as 1/0, and null or undefined as NULL.
`public bool `[`open`](#classSQLite_1af4ea33f6a23d60579ee6e36d0f0276fc)`()` | Open the database.
`public bool `[`close`](#classSQLite_1ae4c723f03c88a9a11b6b7cdb7fba0e18)`()` | Close the database and every statement from query().

## Members

#### `public String `[`path`](#classSQLite_1a5d99c8c883289592018dfd5ee94ea3ce) 

Get the path used to open the database.

#### `public `[`DBStatement`](#classDBStatement)`[] `[`statements`](#classSQLite_1a30a088d06c85fa846e13dd87b1ac7ad7) 

Get the statements from query() that are still open, from index 0. A statement finished by go(), closed with close() or closed by closing the database is left out.

#### `public bool `[`isOpen`](#classSQLite_1a82dd45ef884604bf40f70a16c9f6fbc6) 

Whether or not the database is open.

#### `public double `[`lastRowId`](#classSQLite_1aff1af94823aa7b2de96f65993686feed) 

Get the row id of the last inserted row (exact up to 2^53), or 0 when the database is not open.

#### `public double `[`changes`](#classSQLite_1a8eaeea285b896935182fdd334b605c72) 

Get the number of rows modified/inserted/deleted by the last statement, or 0 when the database is not open.

#### `public  `[`SQLite`](#classSQLite_1a9520cbf360895c30909dfdac712a9aef)`(String path,bool autoOpen)` 

Create a database object which can be used to open [SQLite](#classSQLite) databases and execute queries.

#### Parameters
* `path` The path to the database to open.

* `autoOpen` Whether to open the database immediately (default true; only a boolean is read). A failed open throws "Could not open database: " followed by SQLite's message.

#### `public bool `[`execute`](#classSQLite_1a8d0c60072870fba816b974671996c2af)`(String queryString)` 

Execute a query on the database.

#### Parameters
* `queryString` The query string to execute.

#### Returns
true

#### `public `[`DBStatement`](#classDBStatement)` `[`query`](#classSQLite_1a8ebb32f52a5426ae36a7977c540feb32)`(String sql,...)` 

Prepare sql on the database and bind the remaining arguments to its parameters in order: strings as text, numbers as integers or reals, true/false as 1/0, and null or undefined as NULL.

#### Parameters
* `sql` The SQL to prepare.

#### Returns
The resulting [DBStatement](#classDBStatement). Throws when sql does not prepare or holds no statement, when an argument is an object or a function, or when there are more arguments than sql has parameters (an explicit undefined counts as an argument).

#### `public bool `[`open`](#classSQLite_1af4ea33f6a23d60579ee6e36d0f0276fc)`()` 

Open the database. Opening an open database does nothing. After close(), it opens the same path again; statements from before the close stay closed, and a ":memory:" database starts empty.

#### Returns
true. Throws "Could not open database: " followed by SQLite's message when SQLite cannot open the path.

#### `public bool `[`close`](#classSQLite_1ae4c723f03c88a9a11b6b7cdb7fba0e18)`()` 

Close the database and every statement from query(). Closing a closed database does nothing.

#### Returns
true. Throws "Could not close database: " followed by SQLite's message when SQLite cannot close it.

# class `Text` 

A [Text](#classText) screen hook.

A [Text](#classText) draws a string with one of the game's fonts on the game screen or the automap. Create one with `new Text(text, x, y, color, font, align, automap, click, hover)`; every argument is optional (defaults "", 0, 0, 0, 0, 0, false, none, none). y is the bottom of the first line. Each `\n` in the text starts a new line above the previous one, so the first line is the bottom one and later lines stack upward. Color codes in the text (the character 0xFF, written `\xFF` in a script string, followed by `c` and a code character) change the color from that point on; see `color`. A [Text](#classText) made by an out-of-game script draws only at the menus; one made in game (or from the console) draws only in game. With `automap` true (a constructor argument only), x and y are game map coordinates (the units of unit.x and unit.y) and the [Text](#classText) is drawn only while the automap is open; only align 0 then puts the text on its map point, while 1 and 2 shift it sideways.

Hooks of every script and every hook class are drawn in ascending `zorder` (default 1), so a higher value draws on top. Set `visible` to hide or show the [Text](#classText), and call remove() to delete it. Keep a reference to every hook: one whose object is garbage-collected is removed. When the script that made a hook ends, its hooks leave the screen (with the default `DisableCache=true` in d2bs.ini; with `DisableCache=false` a script that ended normally keeps its hooks drawn, but not clickable, until the script cache is flushed).

Number arguments and number properties take any finite number and drop its fraction (the same result as `value | 0` in script); any other value is ignored and the default or current value stays. `color`, `font` and `zorder` keep only the low 16 bits, so -1 reads back 65535. `automap` and `visible` take only true or false; any other value is ignored.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classText_1a0561058ba47e899900800635dbe270c2) | The x coordinate of the [Text](#classText).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classText_1a90f1e6cb7aa15a593d36c96144087179) | The y coordinate of the bottom of the [Text](#classText)'s first line.
`public bool `[`visible`](#classText_1a9d8567b4c506868eeabe14720a627096) | Whether or not the [Text](#classText) is visible.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`align`](#classText_1a4c8c8cccbf4fe4c50f9e56ff3f5e5c96) | The horizontal alignment.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`zorder`](#classText_1ab953550b21d489b9c3a686e8119f19fe) | The z-order of the [Text](#classText) (what it covers up and is covered by).
`public ClickHandler `[`click`](#classText_1a6d8b641a52b4e8dcd78b3477fe0456c4) | The click handler that gets called when the [Text](#classText) is clicked; null or undefined removes it.
`public HoverHandler `[`hover`](#classText_1ac180c2f5c657fb60f5ec3b81bd1b8cd3) | A hover handler for the [Text](#classText); kept but never called.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`color`](#classText_1a6f4e60b7e0ccd69f154dad14a48be330) | The color of the [Text](#classText).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`font`](#classText_1a4113e8ec35bb03fa7529d357c5f389b1) | The game font, 0-13.
`public String `[`text`](#classText_1a562d404f9579c7f6e04ada90126d4125) | The string that the [Text](#classText) hook displays.
`public  `[`Text`](#classText_1ac6a792e28f41717fc5a4c803ec90d41b)`(String szText,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` color,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` font,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` align,bool automap,ClickHandler click,HoverHandler hover)` | Create a [Text](#classText) hook with the given parameters.
`public void `[`remove`](#classText_1a6d32e163a5fb4eb1a2b90e85baf83600)`()` | Remove the [Text](#classText) hook from the screen for good.

## Members

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classText_1a0561058ba47e899900800635dbe270c2) 

The x coordinate of the [Text](#classText).

It is the text's left edge with align 0, its right edge with align 1 and its centre with align 2. A map coordinate when the [Text](#classText) was made with automap true. While x or y is -1 the [Text](#classText) is not drawn.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classText_1a90f1e6cb7aa15a593d36c96144087179) 

The y coordinate of the bottom of the [Text](#classText)'s first line.

Each further line (after a `\n`) is drawn above the previous one, so the block grows upward from y. A map coordinate when the [Text](#classText) was made with automap true. While x or y is -1 the [Text](#classText) is not drawn.

#### `public bool `[`visible`](#classText_1a9d8567b4c506868eeabe14720a627096) 

Whether or not the [Text](#classText) is visible.

Defaults to true. A hidden [Text](#classText) is not drawn and gets no clicks. Only true or false is accepted.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`align`](#classText_1a4c8c8cccbf4fe4c50f9e56ff3f5e5c96) 

The horizontal alignment.

0 - Left: x is the text's left edge

1 - Right: x is the text's right edge

2 - Center: x is the text's centre

Other values are stored as given and draw as 0. With automap true only 0 (or another value that draws as 0) puts the text on its map point; 1 and 2 shift it sideways.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`zorder`](#classText_1ab953550b21d489b9c3a686e8119f19fe) 

The z-order of the [Text](#classText) (what it covers up and is covered by).

Defaults to 1. Hooks are drawn from the lowest zorder to the highest, so a higher value covers a lower one; hooks with equal values draw in no set order. A click reaches every hook under it, whatever its zorder.

#### `public ClickHandler `[`click`](#classText_1a6d8b641a52b4e8dcd78b3477fe0456c4) 

The click handler that gets called when the [Text](#classText) is clicked. Assign null or undefined to remove it; a value that is not a function is ignored.

It is called as click(button, x, y) when a mouse button is pressed over the [Text](#classText): button 0 left, 1 right, 2 middle; x and y are window pixels. Only a boolean `true` return keeps the press from the game; the release always reaches the game. The handler runs when the owning script next processes events, and the game gets the press anyway if the handler has not answered within 3 seconds. A paused or ended script's hooks get no clicks.

The [Text](#classText) gets clicks while its `visible` property is true, even when it is not drawn at the moment (other game state, automap closed, x or y of -1). The click area is the text's width at its aligned x, from y minus the height of all its lines up to y (edges excluded), on the raw x and y: the automap conversion is not applied.

#### `public HoverHandler `[`hover`](#classText_1ac180c2f5c657fb60f5ec3b81bd1b8cd3) 

A hover handler for the [Text](#classText). D2BS keeps it and returns it from `hover`, but never calls it. Assign null or undefined to remove it; a value that is not a function is ignored.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`color`](#classText_1a6f4e60b7e0ccd69f154dad14a48be330) 

The color of the [Text](#classText).

0-12 select the game's text colors; see [Text colors](#text-colors) in the reference tables for their names. Other values are not defined. Color codes in the text override it from that point on: the character 0xFF (written `\xFF` in a script string) followed by `c` and one of `0`-`9`, `:`, `;` or `<` selects colors 0-12; a code character after `<` selects 0, and one before `0` gives an undefined color.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`font`](#classText_1a4113e8ec35bb03fa7529d357c5f389b1) 

The game font, 0-13.

See [Fonts](#fonts) in the reference tables for the font names. Values of 14 or more are invalid: the game reads past its font table and can crash. D2BS does not check the value.

#### `public String `[`text`](#classText_1a562d404f9579c7f6e04ada90126d4125) 

The string that the [Text](#classText) hook displays.

Each `\n` starts a new line above the previous one (the first line is the bottom one), and color codes (see `color`) change the color from that point on. Assigning a value that is not a string is ignored.

#### `public  `[`Text`](#classText_1ac6a792e28f41717fc5a4c803ec90d41b)`(String szText,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` color,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` font,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` align,bool automap,ClickHandler click,HoverHandler hover)` 

Create a [Text](#classText) hook with the given parameters.

#### Parameters
* `szText` The string that the [Text](#classText) hook displays. A value that is not a string shows nothing (pass String(n) for a number).

* `x` The x coordinate of the text's anchor: its left edge (align 0), right edge (align 1) or centre (align 2).

* `y` The y coordinate of the bottom of the first line; further lines stack upward.

* `color` The text color, 0-12 (see [Text colors](#text-colors) in the reference tables).

* `font` The game font, 0-13 (see [Fonts](#fonts) in the reference tables). Values of 14 or more are invalid and can crash the game.

* `align` The horizontal alignment.

0 - Left

1 - Right

2 - Center

#### Parameters
* `automap` true to place the [Text](#classText) at game map coordinates and draw it only while the automap is open; false (the default) for screen coordinates. Only true or false is accepted.

* `click` A function to call when the [Text](#classText) is clicked; any other value sets no handler.

* `hover` A hover function for the [Text](#classText), kept but never called; any other value sets no handler.

#### `public void `[`remove`](#classText_1a6d32e163a5fb4eb1a2b90e85baf83600)`()` 

Remove the [Text](#classText) hook from the screen for good.

The object stays and its click and hover handlers are released. Its properties then read undefined, writes to them are ignored, and calling remove() again does nothing. Returns undefined.

# class `Unit` 

This class represents a unit (monster, wp, npc, character, etc) in the game.

When the unit or item has left the game, getFlag(), getEnchant() and getSkill() return false, getFlags(), getItemCost(), getMinionCount() and getMercHP() return undefined, and getItems() returns an empty array.

## Summary

 Members                        | Descriptions                                
--------------------------------|---------------------------------------------
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`type`](#classUnit_1a45243122ae655f50179f96bc831bfb1d) | The type of the unit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`classid`](#classUnit_1ab7a6985a8d8a60137e8a469fbb5b2d04) | The class id of the object.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`mode`](#classUnit_1a0e0de49749be72180866dd31b76a0102) | The mode of the unit.
`public String `[`name`](#classUnit_1aa869e12411361f016ed04a238b3894ef) | The name of the unit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`act`](#classUnit_1afaf30997a33a9c05c7a4941d1c7bfd39) | The act where the unit is currently located.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`gid`](#classUnit_1ad6fb57eda20f91a8b4ea15eb28621f1f) | The unit unique id.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classUnit_1a356b2d8a1d6dde32b67b2c9a3e173b99) | The x location of the unit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classUnit_1a6846a7ed25c3e93b51f9bc9643576f05) | The y location of the unit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`targetx`](#classUnit_1abe6abf7822c2b691853fb58320157b0f) | The target x location of the unit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`targety`](#classUnit_1a9ee1df35e54c3a4f8756329782792400) | The target y location of the unit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`area`](#classUnit_1af5f2e11d4f116084ffd807b578078d7d) | The id of the area (level) the unit is located in.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`hp`](#classUnit_1ae9d3c95ccf18f5bc2bd6a7e68f0e7b1c) | The unit's current health.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`hpmax`](#classUnit_1a0d97cfa23ab03fa550861e9f0aac7f20) | The unit's maximum health.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`mp`](#classUnit_1abbab13fe2ae1ae93b78ea148fde7a4ff) | The unit's current mana.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`mpmax`](#classUnit_1a07b6e27b8e9e9430ff9c3be224f6090e) | The unit's maximum mana.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`stamina`](#classUnit_1a53c596913a3c96cc692df12c26a3ac7d) | The unit's current stamina.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`staminamax`](#classUnit_1ac6acb5617d4f9b525287e35b91575c31) | The unit's maximum stamina.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`charlvl`](#classUnit_1ae12b4d41735cb54cf7609bcaa772f4ae) | The character level.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`itemcount`](#classUnit_1af3532e4e568df722332b0e2d85759fb3) | The number of items in inventory.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`owner`](#classUnit_1a8616a100c244d69e7592d5b6843a0b61) | The unit id of the owner of the unit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`ownertype`](#classUnit_1ae2003fc0bc7e60523ff0c05d1f7bb1ca) | The type of owner.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`spectype`](#classUnit_1a90be3cd88e866777cffb925e3244e730) | The special type of the unit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`direction`](#classUnit_1af64fb0f78ce8cb663fe12ec8a1c5f81d) | The direction the unit is facing, from 0 to 63.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`uniqueid`](#classUnit_1aeda7422b700e357cdda91fb24312e58e) | The unique id of a boss.
`public String `[`code`](#classUnit_1a1dfbd39341316980c6c17305555e141c) | The three digit code for an item.
`public String `[`prefix`](#classUnit_1a6b00de01b009bf7b212d4c52326fa7fc) | The magic prefix of the item.
`public String `[`suffix`](#classUnit_1a871f08609370b8e5a3c91aa1cd5dae1a) | The magic suffix of the item.
`public String[] `[`prefixes`](#classUnit_1a021da0af0241b96f52cbf411c11b3d34) | The magic prefixes of the item.
`public String[] `[`suffixes`](#classUnit_1a3c72883e56ef785dfebbf1b0a86fecff) | The magic suffixes of the item.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`prefixnum`](#classUnit_1a00e6bd1941d366802e6ac571e36da8c3) | The id code for the magic prefix.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`suffixnum`](#classUnit_1ae71acd4a5d4d075f0211242f48e18d3c) | The id code for the magic suffix.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`prefixnums`](#classUnit_1a66f3e79ca4097a2c095a4f047b97f996) | The id codes for the magic prefixes.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`suffixnums`](#classUnit_1af668286ae6c3d1d87bfc531ab8d54f77) | The id codes for the magic suffixes.
`public String `[`fname`](#classUnit_1a23495fdca380dc2f19c624b5bcf3f4c8) | The full name of an item.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`quality`](#classUnit_1af7a138ffb9643f0015ba22f0230e5ce4) | The item quality.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`node`](#classUnit_1ab5da58f77c0aaa3f3ce81cccee12e057) | Which kind of inventory grid holds the item.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`location`](#classUnit_1a60b8bec5206902278dfbbebe75c3617e) | The location of the item (ground, inventory, stash, etc.).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`sizex`](#classUnit_1a1320b38ed997f4922abfeb2966fed81b) | The x size of the item.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`sizey`](#classUnit_1afe39c0f050662a28ffd252d50f94910d) | The y size of the item.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`itemType`](#classUnit_1ab0d3eddb92c27f52f7698895bcf3905b) | The type of the item.
`public String `[`description`](#classUnit_1a7c76ba13eb7e3a3f26f50282fb8b431e) | The description of the item.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`bodylocation`](#classUnit_1ab7779c44d220bab7d0d8467a6d1fd007) | The equipped location of the item.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`ilvl`](#classUnit_1aee7cd2c778a2ad23cbb2001378bfd86b) | Item level.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`lvlreq`](#classUnit_1af1677614eb990c59aa3cd43be30983e2) | Level requirement (character level required to use the item).
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`gfx`](#classUnit_1a9bd325f9e2eb378709bf785a2b4bc6ad) | Get the id for the graphic used for the item.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`runwalk`](#classUnit_1ac8a909f484cee949343474ff0cddcb5f) | Whether the controlled character is in the always run mode or not.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`weaponswitch`](#classUnit_1ac9b8c63afb5360437af31a0b0902232a) | Set the weapon switch.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`objtype`](#classUnit_1adfb606b289eeccde32b0ed79ed0020c9) | The object's interaction type.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`islocked`](#classUnit_1afa81eef36e4eddf98f8dd69c48e22875) | Whether or not the chest is locked.
`public bool `[`getNext`](#classUnit_1aa6a3842cbc14d3456f1f447153d68b53)`()` | Get the next unit that matches the originally given name or class id and originally specified mode.
`public bool `[`getNext`](#classUnit_1a3745b5481a9ede47f517caa64e5eb6d2)`(String szName)` | Get the next unit that matches the given name and originally specified mode.
`public bool `[`getNext`](#classUnit_1a16a65b85a7c7a75742cb4494bc4fbc92)`(uint32_t dwClassId)` | Get the next unit that matches the given class id and originally specified mode.
`public bool `[`getNext`](#classUnit_1ab92726734ec51ea37d30808b166dbead)`(String szName,uint32_t dwMode)` | Get the next unit that matches the given name and mode.
`public bool `[`getNext`](#classUnit_1a6c91814477954824ee590aabc2bc8287)`(uint32_t dwClassId,uint32_t dwMode)` | Get the next unit that matches the given class id and mode.
`public void `[`cancel`](#classUnit_1abdc2ecc5fb8044db1800438e876a5253)`()` | Cancel whatever's going on.
`public bool `[`repair`](#classUnit_1a6565698fe901074f3c7e61799e001430)`()` | Try to repair.
`public bool `[`useMenu`](#classUnit_1a504281991916f1881b30f9968c62526c)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` menuItem)` | Use an NPC menu.
`public bool `[`interact`](#classUnit_1a0c70c60dd71e4b039245f023a752c06e)`()` | Interact with the unit.
`public bool `[`interact`](#classUnit_1aa1a4ac5b77fc8561252cad2af1ba9ba3)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` destId)` | Interact with a waypoint.
`public `[`Unit`](#classUnit)` `[`getItem`](#classUnit_1a4ef77dc37b10ba1fa7254935549f66fc)`(String name,uint32_t mode,uint32_t nUnitId)` | Get an item from inventory by name, mode and nUnitId.
`public `[`Unit`](#classUnit)` `[`getItem`](#classUnit_1ad5dbc6824bff1d1847be8052ab36d381)`(uint32_t classId,uint32_t mode,uint32_t nUnitId)` | Get an item from inventory by classId, mode and nUnitId.
`public `[`Unit`](#classUnit)` `[`getItems`](#classUnit_1af421c40cb966dd2bef028c3ae2660e78)`()` | Get all items from inventory.
`public `[`Unit`](#classUnit)` `[`getMerc`](#classUnit_1a33171db06dfdf2cc7d7826e1ea7242e4)`(bool dummy)` | Get a player's merc; the argument is ignored.
`public `[`Unit`](#classUnit)` `[`getMerc`](#classUnit_1a765ff16754426875b9975e71c776a6ac)`()` | Get a player's merc.
`public String `[`getSkill`](#classUnit_1a85219a0b1d3856c432b6db80249ecc63)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` hand)` | Get skill name from hand.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getSkill`](#classUnit_1a94823fedd1fb16eece0fe7fd729bb4e8)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` hand)` | Get skill id from hand.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getSkill`](#classUnit_1acfc83d3e88c549e6dae3c682c3baf40f)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` dummy)` | Get all skills.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getSkill`](#classUnit_1ab14f99798787ec5b258a47824f089604)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSkillId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nExt)` | Get the base or total level of a skill.
`public `[`Unit`](#classUnit)` `[`getParent`](#classUnit_1aba1d739e034f714e07e5c66a1bcd8297)`()` | Gets the parent of a unit.
`public String `[`getParent`](#classUnit_1a59389e13f5a91193cc7c57a51372a529)`()` | Gets the parent of a unit.
`public void `[`overhead`](#classUnit_1a3b8b22982ac3eb1dcefb354102a19a30)`(Object message)` | Puts the string equivalent of message over the unit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getFlags`](#classUnit_1a01704c64338a7818f03ea9fdb0205ee6)`()` | Returns the item flags.
`public bool `[`getFlag`](#classUnit_1a66aba538357c70138e3126b022749f56)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` flags)` | Same as getFlags, but returns a boolean.
`public double `[`getStat`](#classUnit_1a273cc4dadf71bdf8c2ad9b6ee5ed122a)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nStat,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSubIndex)` | Get a stat by stat id and sub index.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getStat`](#classUnit_1a106c9cbe30bae8d80322f3ec1bc3aafa)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nStat,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSubIndex)` | Get an array of the unit's listed stats.
`public Object[] `[`getStat`](#classUnit_1getStat_byId)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nStat,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSubIndex)` | Get the unit's stats as an array indexed by stat id.
`public bool `[`getState`](#classUnit_1a8ec41cccbb61db284264cc5879686a6d)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nState)` | Return whether or not the unit has a given state.
`public bool `[`getEnchant`](#classUnit_1a082ad92039c9a817574adb3431ddb159)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nEnchant)` | Determine whether or not a unit has a given enchant.
`public bool `[`shop`](#classUnit_1aa4e63e5e27420a6965bda5d855bb8c49)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` dwMode)` | Shop with a given npc, either buying or selling.
`public bool `[`setSkill`](#classUnit_1a89e811c8b062728894b6504a8f02372d)`(String skillName,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nHand)` | Set the skill on the given hand to be skill with name skillName.
`public bool `[`setSkill`](#classUnit_1a3f88599825b6b93e1bf52f9c13df0a95)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSkillId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nHand)` | Set the skill on the given hand to be skill with id nSkillId.
`public bool `[`setSkill`](#classUnit_1setSkill_name_item)`(String skillName,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nHand,`[`Unit`](#classUnit)` item)` | Set the skill on the given hand to be skill with name skillName.
`public bool `[`setSkill`](#classUnit_1setSkill_id_item)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSkillId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nHand,`[`Unit`](#classUnit)` item)` | Set the skill on the given hand to be skill with id nSkillId.
`public void `[`move`](#classUnit_1a0b13c1bcd85e4a2fda08548feef07d0b)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y)` | Move to the given location.
`public void `[`move`](#classUnit_1a4a295cc929f3a8ccf67287d4f6f5515b)`()` | Move to this unit.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getQuest`](#classUnit_1afbb11a6c2f4457b19fd464c1df647065)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nAct,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nQuest)` | Get the quest flag for a quest specified by act and quest number.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getMinionCount`](#classUnit_1a1cf6810c2589b47f74cfa125375d75ed)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nType)` | Get the number of minions of a certain type.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getRepairCost`](#classUnit_1ae007a9f727a4f34f40e5bb4d0867897b)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nNpcClassId)` | Get price to repair this unit at the current npc given by nNpcClassId.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getItemCost`](#classUnit_1a18eb04eaed8f97b76aacbd8b2734f563)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nNpcClassId)` | Get the cost to do something (buy, sell, repair) with the given item, at the given npc.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getItemCost`](#classUnit_1ac27394b9daab7a7c1c457ab6121910f8)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nNpcClassId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nDifficulty)` | Get the cost to do something (buy, sell, repair) with the given item, at the given npc, in the given difficulty.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getItemCost`](#classUnit_1a9db71041710e126f77d3a2eb2526fd9a)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode,`[`Unit`](#classUnit)` npc,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nDifficulty)` | Get the cost to do something (buy, sell, repair) with the given item, at the given npc, in the given difficulty.
`public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getItemCost`](#classUnit_1ae375f1e4e46cdc39c29a947d7add4765)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode,`[`Unit`](#classUnit)` npc)` | Get the cost to do something (buy, sell, repair) with the given item, at the given npc.

## Members

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`type`](#classUnit_1a45243122ae655f50179f96bc831bfb1d) 

The type of the unit.

0 - Player

1 - NPC

2 - Object

3 - Missile

4 - Item

5 - Warp

Source: botNET

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`classid`](#classUnit_1ab7a6985a8d8a60137e8a469fbb5b2d04) 

The class id of the object.

See: [http://forums.d2botnet.org/viewtopic.php?f=18&t=1002](http://forums.d2botnet.org/viewtopic.php?f=18&t=1002)

[http://forums.d2botnet.org/viewtopic.php?f=18&t=986](http://forums.d2botnet.org/viewtopic.php?f=18&t=986) and

[http://forums.d2botnet.org/viewtopic.php?f=18&t=985](http://forums.d2botnet.org/viewtopic.php?f=18&t=985)

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`mode`](#classUnit_1a0e0de49749be72180866dd31b76a0102) 

The mode of the unit.

See: [http://forums.d2botnet.org/viewtopic.php?f=18&t=988](http://forums.d2botnet.org/viewtopic.php?f=18&t=988)

#### `public String `[`name`](#classUnit_1aa869e12411361f016ed04a238b3894ef) 

The name of the unit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`act`](#classUnit_1afaf30997a33a9c05c7a4941d1c7bfd39) 

The act where the unit is currently located.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`gid`](#classUnit_1ad6fb57eda20f91a8b4ea15eb28621f1f) 

The unit unique id.

Referred to simply as the unit id. Used along with the unit type to uniquely identify the unit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`x`](#classUnit_1a356b2d8a1d6dde32b67b2c9a3e173b99) 

The x location of the unit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`y`](#classUnit_1a6846a7ed25c3e93b51f9bc9643576f05) 

The y location of the unit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`targetx`](#classUnit_1abe6abf7822c2b691853fb58320157b0f) 

The target x location of the unit.

Where the game is taking the unit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`targety`](#classUnit_1a9ee1df35e54c3a4f8756329782792400) 

The target y location of the unit.

Where the game is taking the unit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`area`](#classUnit_1af5f2e11d4f116084ffd807b578078d7d) 

The id of the area (level) the unit is located in.

The value is the Id column of Levels.txt, for example 1 = Rogue Encampment, 108 = The Chaos Sanctuary (Chaos Sanctum in Levels.txt), 109 = Harrogath. See [Area ids](#area-ids) in the reference tables.

Undefined when the game has no room for the unit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`hp`](#classUnit_1ae9d3c95ccf18f5bc2bd6a7e68f0e7b1c) 

The unit's current health.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`hpmax`](#classUnit_1a0d97cfa23ab03fa550861e9f0aac7f20) 

The unit's maximum health.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`mp`](#classUnit_1abbab13fe2ae1ae93b78ea148fde7a4ff) 

The unit's current mana.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`mpmax`](#classUnit_1a07b6e27b8e9e9430ff9c3be224f6090e) 

The unit's maximum mana.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`stamina`](#classUnit_1a53c596913a3c96cc692df12c26a3ac7d) 

The unit's current stamina.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`staminamax`](#classUnit_1ac6acb5617d4f9b525287e35b91575c31) 

The unit's maximum stamina.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`charlvl`](#classUnit_1ae12b4d41735cb54cf7609bcaa772f4ae) 

The character level.

The level that determines stat points, skill levels, etc.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`itemcount`](#classUnit_1af3532e4e568df722332b0e2d85759fb3) 

The number of items in inventory.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`owner`](#classUnit_1a8616a100c244d69e7592d5b6843a0b61) 

The unit id of the owner of the unit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`ownertype`](#classUnit_1ae2003fc0bc7e60523ff0c05d1f7bb1ca) 

The type of owner.

[type](#classUnit_1a45243122ae655f50179f96bc831bfb1d)

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`spectype`](#classUnit_1a90be3cd88e866777cffb925e3244e730) 

The special type of the unit.

Represents things like unique, minion, boss, etc.

1 - "Normal" Boss 2 - Champion 4 - Boss 8 - Minion

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`direction`](#classUnit_1af64fb0f78ce8cb663fe12ec8a1c5f81d) 

The direction the unit is facing, from 0 to 63.

0 faces straight down the screen and the value increases clockwise: 16 faces left (west), 32 up (north) and 48 right (east). 8, 24, 40 and 56 lie between them along the game's map axes (down-left, up-left, up-right and down-right).

A player or monster turns a step at a time, so while it turns this value can trail the direction it is moving in.

Undefined when the unit has no path or its path is not in a room. For objects and items the value is currently not their facing: it can be undefined or an arbitrary number.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`uniqueid`](#classUnit_1aeda7422b700e357cdda91fb24312e58e) 

The unique id of a boss.

Taken from pUnit->pMonsterData->wUniqueNo. Not the same thing as the gid (unit id).

#### `public String `[`code`](#classUnit_1a1dfbd39341316980c6c17305555e141c) 

The three digit code for an item.

See: [http://forums.d2botnet.org/viewtopic.php?f=18&t=991](http://forums.d2botnet.org/viewtopic.php?f=18&t=991)

#### `public String `[`prefix`](#classUnit_1a6b00de01b009bf7b212d4c52326fa7fc) 

The magic prefix of the item.

#### `public String `[`suffix`](#classUnit_1a871f08609370b8e5a3c91aa1cd5dae1a) 

The magic suffix of the item.

#### `public String[] `[`prefixes`](#classUnit_1a021da0af0241b96f52cbf411c11b3d34) 

The magic prefixes of the item.

#### `public String[] `[`suffixes`](#classUnit_1a3c72883e56ef785dfebbf1b0a86fecff) 

The magic suffixes of the item.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`prefixnum`](#classUnit_1a00e6bd1941d366802e6ac571e36da8c3) 

The id code for the magic prefix.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`suffixnum`](#classUnit_1ae71acd4a5d4d075f0211242f48e18d3c) 

The id code for the magic suffix.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`prefixnums`](#classUnit_1a66f3e79ca4097a2c095a4f047b97f996) 

The id codes for the magic prefixes.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`suffixnums`](#classUnit_1af668286ae6c3d1d87bfc531ab8d54f77) 

The id codes for the magic suffixes.

#### `public String `[`fname`](#classUnit_1a23495fdca380dc2f19c624b5bcf3f4c8) 

The full name of an item.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`quality`](#classUnit_1af7a138ffb9643f0015ba22f0230e5ce4) 

The item quality.

1 - Low quality

2 - Normal

3 - Superior

4 - Magic

5 - Set

6 - Rare

7 - Unique

8 - Crafted

Source: botNET

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`node`](#classUnit_1ab5da58f77c0aaa3f3ce81cccee12e057) 

Which kind of inventory grid holds the item.

0 - Not in any inventory grid (on the ground or on the cursor)

1 - A storage page (inventory, cube, stash, trade or shop)

2 - The belt

3 - Equipped (body locations 1-10)

4 - Equipped in a weapon-switch slot (body locations 11 and 12)

Use location to tell storage pages apart. An item linked into an inventory without a grid cell also reads 1, so node 1 alone does not prove the item sits on a storage page.

Undefined for units that are not items.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`location`](#classUnit_1a60b8bec5206902278dfbbebe75c3617e) 

The location of the item (ground, inventory, stash, etc.).

0 - Ground

1 - Equipped

2 - Belt

3 - Inventory

4 - Store

5 - Trade

6 - Cube

7 - Stash

Source: d2botnet code

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`sizex`](#classUnit_1a1320b38ed997f4922abfeb2966fed81b) 

The x size of the item.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`sizey`](#classUnit_1afe39c0f050662a28ffd252d50f94910d) 

The y size of the item.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`itemType`](#classUnit_1ab0d3eddb92c27f52f7698895bcf3905b) 

The type of the item.

The base item's primary type: its row index in ItemTypes.txt, counting data rows from 0 and skipping the "Expansion" separator row (for example 2 = shield, 4 = gold, 10 = ring, 12 = amulet, 58 = jewel, 74 = rune). Class-specific bases report their own type, so an auric shield is not 2. See [Item types](#item-types) in the reference tables.

Undefined for units that are not items.

#### `public String `[`description`](#classUnit_1a7c76ba13eb7e3a3f26f50282fb8b431e) 

The description of the item.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`bodylocation`](#classUnit_1ab7779c44d220bab7d0d8467a6d1fd007) 

The equipped location of the item.

0 - Not equipped

1 - Helmet

2 - Amulet

3 - Armor

4 - Right hand slot 1

5 - Left hand slot 1

6 - Right ring

7 - Left ring

8 - Belt

9 - Boots

10 - Gloves

11 - Right hand slot 2

12 - Left hand slot 2

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`ilvl`](#classUnit_1aee7cd2c778a2ad23cbb2001378bfd86b) 

Item level.

Used for things like item stat rolling.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`lvlreq`](#classUnit_1af1677614eb990c59aa3cd43be30983e2) 

Level requirement (character level required to use the item).

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`gfx`](#classUnit_1a9bd325f9e2eb378709bf785a2b4bc6ad) 

Get the id for the graphic used for the item.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`runwalk`](#classUnit_1ac8a909f484cee949343474ff0cddcb5f) 

Whether the controlled character is in the always run mode or not.

0 - Walk unless directed to run 1 - Always run

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`weaponswitch`](#classUnit_1ac9b8c63afb5360437af31a0b0902232a) 

Set the weapon switch.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`objtype`](#classUnit_1adfb606b289eeccde32b0ed79ed0020c9) 

The object's interaction type.

This is a byte the server sends with the object. Its known uses:

* For a shrine or well, the shrine kind: the row index in Shrines.txt (for example 1 = refill, 2 = health, 3 = mana, 12 = skill, 15 = experience, 18 = gem). A shrine seen in game read 1 and a well read 2.

* For a portal, the id of the area it leads to (the same ids as area; see [Area ids](#area-ids) in the reference tables).

* For a chest, bit 0x80 is set while the chest is locked, so a locked chest reads 128 or more (see islocked).

Other objects read 0 in the cases seen: the stash, the waypoint, fires, torches and a chest.

Undefined for units that are not objects.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`islocked`](#classUnit_1afa81eef36e4eddf98f8dd69c48e22875) 

Whether or not the chest is locked.

#### `public bool `[`getNext`](#classUnit_1aa6a3842cbc14d3456f1f447153d68b53)`()` 

Get the next unit that matches the originally given name or class id and originally specified mode.

#### Returns
Whether another unit was found.

#### `public bool `[`getNext`](#classUnit_1a3745b5481a9ede47f517caa64e5eb6d2)`(String szName)` 

Get the next unit that matches the given name and originally specified mode.

#### Parameters
* `szName` The name to look for.

#### Returns
Whether another unit was found.

#### `public bool `[`getNext`](#classUnit_1a16a65b85a7c7a75742cb4494bc4fbc92)`(uint32_t dwClassId)` 

Get the next unit that matches the given class id and originally specified mode.

#### Parameters
* `dwClassId` The class id to look for.

#### Returns
Whether another unit was found.

#### `public bool `[`getNext`](#classUnit_1ab92726734ec51ea37d30808b166dbead)`(String szName,uint32_t dwMode)` 

Get the next unit that matches the given name and mode.

#### Parameters
* `szName` The name to look for.

* `dwMode` Either the mode being searched for, or a bitmask with bit 29 set and bits 0-28 corresponding to modes of units being searched for.

#### Returns
Whether another unit was found.

#### `public bool `[`getNext`](#classUnit_1a6c91814477954824ee590aabc2bc8287)`(uint32_t dwClassId,uint32_t dwMode)` 

Get the next unit that matches the given class id and mode.

#### Parameters
* `dwClassId` The class id to look for.

* `dwMode` Either the mode being searched for, or a bitmask with bit 29 set and bits 0-28 corresponding to modes of units being searched for.

#### Returns
Whether another unit was found.

#### `public void `[`cancel`](#classUnit_1abdc2ecc5fb8044db1800438e876a5253)`()` 

Cancel whatever's going on.

If there's scrolling text clear it. If you're interacting to an NPC, stop. If there's an item on the cursor, drop it. Otherwise close any other interaction that might be going on.

#### `public bool `[`repair`](#classUnit_1a6565698fe901074f3c7e61799e001430)`()` 

Try to repair.

Need to be able to find unit that you're trying to repair with. That means the unit needs to still be in the server unit hash table.

#### Returns
true if successful.

#### `public bool `[`useMenu`](#classUnit_1a504281991916f1881b30f9968c62526c)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` menuItem)` 

Use an NPC menu.

Need to be able to find unit that you're trying to repair with. That means the unit needs to still be in the server unit hash table.

#### Parameters
* `menuItem` Index of the menu item to click.

#### Returns
true if menu was found.

#### `public bool `[`interact`](#classUnit_1a0c70c60dd71e4b039245f023a752c06e)`()` 

Interact with the unit.

If the unit is an item in your inventory or stash, the call uses it as a right-click does: a potion is drunk, a town portal tome or scroll opens a portal outside town, and an identify tome or scroll gives the identify cursor. An item in the belt is used the same way. Either use returns false, whether or not the game acts on it. Any other unit, an item anywhere else included, gets a left-button-down click on its map position that targets it, the same kind of click clickMap(0, 0, unit) makes, and the call returns true. Called on your own character, or on a unit that has left the game, it does nothing and returns false. Outside a ready game it returns false (with a 'Game not ready' warning).

#### `public bool `[`interact`](#classUnit_1aa1a4ac5b77fc8561252cad2af1ba9ba3)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` destId)` 

Interact with a waypoint.

If the unit is an object, assume it's a waypoint, and take it.

#### Parameters
* `destId` The area id of the destination to select.

#### `public `[`Unit`](#classUnit)` `[`getItem`](#classUnit_1a4ef77dc37b10ba1fa7254935549f66fc)`(String name,uint32_t mode,uint32_t nUnitId)` 

Get an item from inventory by name, mode and nUnitId.

#### Parameters
* `name` The name of the unit to look for.

* `mode` Either the mode being searched for, or a bitmask with bit 29 set and bits 0-28 corresponding to modes of units being searched for.

* `nUnitId` The unique id of the unit.

#### Returns
The first item found that matches the description.

#### `public `[`Unit`](#classUnit)` `[`getItem`](#classUnit_1ad5dbc6824bff1d1847be8052ab36d381)`(uint32_t classId,uint32_t mode,uint32_t nUnitId)` 

Get an item from inventory by classId, mode and nUnitId.

#### Parameters
* `classId` The class id of the unit.

* `mode` Either the mode being searched for, or a bitmask with bit 29 set and bits 0-28 corresponding to modes of units being searched for.

* `nUnitId` The unique id of the unit.

#### Returns
The first item found that matches the description.

#### `public `[`Unit`](#classUnit)` `[`getItems`](#classUnit_1af421c40cb966dd2bef028c3ae2660e78)`()` 

Get all items from inventory.

#### Returns
An array of all the items from inventory; empty when the unit has no items, has left the game, or the method is called on something that is not a unit.

#### `public `[`Unit`](#classUnit)` `[`getMerc`](#classUnit_1a33171db06dfdf2cc7d7826e1ea7242e4)`(bool dummy)` 

Get a player's merc; the argument is ignored.

The unit being operated on should be a player.

#### Parameters
* `dummy` Ignored.

#### Returns
The merc unit, or null when none is found (the merc must be in a room the client has loaded) or the unit is not a player. Any argument is ignored: getMerc(true) returns the same unit or null, so test the result for truthiness. Outside a ready game it returns false (with a 'Game not ready' warning).

#### `public `[`Unit`](#classUnit)` `[`getMerc`](#classUnit_1a765ff16754426875b9975e71c776a6ac)`()` 

Get a player's merc.

The unit being operated on should be a player.

#### Returns
The merc unit, or null when none is found (the merc must be in a room the client has loaded) or the unit is not a player. Any argument is ignored: getMerc(true) returns the same unit or null, so test the result for truthiness. Outside a ready game it returns false (with a 'Game not ready' warning).

#### `public String `[`getSkill`](#classUnit_1a85219a0b1d3856c432b6db80249ecc63)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` hand)` 

Get skill name from hand.

#### Parameters
* `hand` Hand to get the skill name off.

0 - Right hand

1 - Left hand

#### Returns
The skill's in-game name in the client's language. It is not always a name setSkill() accepts; getSkill(2) and getSkill(3) give the ids. Returns false when the unit has left the game.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getSkill`](#classUnit_1a94823fedd1fb16eece0fe7fd729bb4e8)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` hand)` 

Get skill id from hand.

#### Parameters
* `hand` Hand to get the skill id off.

2 - Right hand

3 - Left hand

#### Returns
Skill id.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getSkill`](#classUnit_1acfc83d3e88c549e6dae3c682c3baf40f)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` dummy)` 

Get all skills.

Gets all skill ids, along with corresponding base and total skill levels.

#### Parameters
* `dummy` Should be 4.

#### Returns
An array of arrays of integers. Inside the inner array the 0th index is the skill id, the 1st index is the base skill level and the 2nd index is the total skill level.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getSkill`](#classUnit_1ab14f99798787ec5b258a47824f089604)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSkillId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nExt)` 

Get the base or total level of a skill.

With two arguments, only the unit's own (non-charged) entry for the skill is read. The total adds the bonuses the game applies to that skill: +all skills, +class skills, +skill tab, +single skill, +elemental skills, oskills and temporary bonuses such as the Skill shrine. Oskills add at most 3 to a player's own class skills. Skills that come from item charges get no bonuses. The result, base or total, is never below 0 and never above 99.

Pass a boolean third argument to choose the entry: true reads the skill from an item with charges (the charge level), false reads the first entry with that id, charged or not.

#### Parameters
* `nSkillId` The skill id. See [Skill ids](#skill-ids) in the reference tables.

* `nExt` 0 for the base level (the hard points put into the skill), any other integer (usually 1) for the total level.

#### Returns
The skill level. Returns false when the unit does not have the skill, when nSkillId or nExt is not an integer, when a third argument is passed that is not a boolean, or when the unit has left the game. Outside a ready game it returns false (with a 'Game not ready' warning).

#### `public `[`Unit`](#classUnit)` `[`getParent`](#classUnit_1aba1d739e034f714e07e5c66a1bcd8297)`()` 

Gets the parent of a unit.

This function is used for monster and item units.

#### Returns
The parent unit.

#### `public String `[`getParent`](#classUnit_1a59389e13f5a91193cc7c57a51372a529)`()` 

Gets the parent of a unit.

This function is used for object units.

#### Returns
The parent's name.

#### `public void `[`overhead`](#classUnit_1a3b8b22982ac3eb1dcefb354102a19a30)`(Object message)` 

Puts the string equivalent of message over the unit.

Throws "Invalid arguments" if no message is passed for a unit that is still in the game; a null or undefined message shows nothing.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getFlags`](#classUnit_1a01704c64338a7818f03ea9fdb0205ee6)`()` 

Returns the item flags.

#### Returns
Item flags:

0x00000001 - Equipped

0x00000008 - In socket

0x00000010 - Identified

0x00000040 - Weapon/shield is in the active weapon switch

0x00000080 - Weapon/shield is in the inactive weapon switch

0x00000100 - Item is broken

0x00000400 - Full rejuv

0x00000800 - Socketed

0x00002000 - In the trade or gamble screen

0x00004000 - Not in a socket

0x00010000 - Is an ear

0x00020000 - A starting item (worth only 1 gold)

0x00200000 - Rune, or potion, or mephisto's soulstone

0x00400000 - Ethereal

0x00800000 - Is an item

0x01000000 - Personalized

0x04000000 - Runeword

Source: [http://subversion.assembla.com/svn/d2bs/scripts/YAMB/libs/YAMB/common/YAM-Common.dbl](http://subversion.assembla.com/svn/d2bs/scripts/YAMB/libs/YAMB/common/YAM-Common.dbl) r1086

#### `public bool `[`getFlag`](#classUnit_1a66aba538357c70138e3126b022749f56)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` flags)` 

Same as getFlags, but returns a boolean.

Returns true if any of the flags given match the item flags.

[getFlags](#classUnit_1a01704c64338a7818f03ea9fdb0205ee6)

#### Parameters
* `flags` Flags to check.

#### Returns
true if any of the flags are set in the item flags.

#### `public double `[`getStat`](#classUnit_1a273cc4dadf71bdf8c2ad9b6ee5ed122a)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nStat,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSubIndex)` 

Get a stat by stat id and sub index.

Life, mana and stamina (stats 6-11) come back in whole points. Experience (stats 13, 29 and 30) is read unsigned, so values of 2^31 and above stay positive. On an item, stat 92 returns the item's level requirement for your character (the same value as the itemlevelreq property) and ignores nSubIndex. On any other unit, your own character included, stat 92 does not read a stat: the game's item level-requirement calculation runs on that unit and returns a number with no fixed meaning. Any other stat returns the game's total for that stat and subindex; when that total is 0, the value of that stat and subindex in the stat list that getStat(-1) reads (not the base stats getStat(-1) adds) is returned instead (0 if it is not there). Arguments that are not integers are converted to integers.

#### Parameters
* `nStat` The stat id (the ID column of ItemStatCost.txt).

* `nSubIndex` A subindex to a certain type of stat. For instance a specific skill for the +1 to skill tab stat. Defaults to 0.

#### Returns
The stat value. Returns false when no stat id is passed or the unit has left the game. Outside a ready game it returns false (with a 'Game not ready' warning).

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getStat`](#classUnit_1a106c9cbe30bae8d80322f3ec1bc3aafa)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nStat,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSubIndex)` 

Get an array of the unit's listed stats.

The array holds the entries of the unit's stat list plus any of the unit's base stats not already in it. It is not every stat the unit has. Values are raw: life, mana and stamina are in 1/256 points, and values of 2^31 and above (such as high experience) come back negative.

#### Parameters
* `nStat` Set to -1.

* `nSubIndex` Ignored.

#### Returns
An array of arrays. The indices of the inner array are: 0 - stat id, 1 - subindex, 2 - value. Returns false when the unit has no such stat list, even if it has base stats. Outside a ready game it returns false (with a 'Game not ready' warning).

#### `public Object[] `[`getStat`](#classUnit_1getStat_byId)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nStat,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSubIndex)` 

Get the unit's stats as an array indexed by stat id.

Each element is an array indexed by subindex that holds the value, with life, mana and stamina (stats 6-11) in whole points. Other values are raw, so values of 2^31 and above (such as high experience) come back negative. The values come from the unit's base stats, its stat totals (the values the single-stat form reads) and the stat list that getStat(-1) reads, in that order; where more than one holds the same stat and subindex, the last one wins, so a value here is not always the one the single-stat form returns.

A stat that carries a skill (for example a charged skill or a chance-to-cast skill such as one cast on striking) holds an object {skill, level} instead, for skill ids 8 and up; lower skill ids come back as ordinary entries. A charged skill also gets charges and maxcharges, except on an item with a maximum of 1 charge, or of 2 charges with none left. When the same stat id occurs more than once, the element is an array of such objects. Each entry comes back once, so a stat with a single skill holds a bare object, not an array; check an element with Array.isArray. When the stat also holds an ordinary entry for a lower skill id, the element is a subindex array instead, with the skill objects after its last index. Oskill mods (stat 97) are ordinary entries: element 97 is indexed by skill id and holds the bonus level.

#### Parameters
* `nStat` Set to -2.

* `nSubIndex` Ignored.

#### Returns
A sparse array indexed by stat id. Outside a ready game it returns false (with a 'Game not ready' warning).

#### `public bool `[`getState`](#classUnit_1a8ec41cccbb61db284264cc5879686a6d)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nState)` 

Return whether or not the unit has a given state.

Objects, items and tiles always return false: the game's state check answers false for them.

#### Parameters
* `nState` The state id: the id column of States.txt, 0-183. State 184 (uberminion) exists in 1.14d but getState rejects it and returns false.

Throws "Invalid arguments" if no state id is passed. Returns false when the unit has left the game, or when nState is not an integer or is out of range. Outside a ready game it returns false (with a 'Game not ready' warning).

#### Returns
Whether or not the unit has the state.

#### `public bool `[`getEnchant`](#classUnit_1a082ad92039c9a817574adb3431ddb159)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nEnchant)` 

Determine whether or not a unit has a given enchant.

#### Parameters
* `nEnchant` Enchantment id.

#### Returns
Whether or not the unit has the enchantment.

#### `public bool `[`shop`](#classUnit_1aa4e63e5e27420a6965bda5d855bb8c49)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` dwMode)` 

Shop with a given npc, either buying or selling.

#### Parameters
* `dwMode` What to do with the item. 1 - Sell, 2 - Buy, 6 - ?

#### Returns
Whether or not the shop went through.

#### `public bool `[`setSkill`](#classUnit_1a89e811c8b062728894b6504a8f02372d)`(String skillName,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nHand)` 

Set the skill on the given hand to be skill with name skillName.

Asks the game to put the skill on the hand, then waits up to about one second for the hand to show it. When the hand already shows the skill, the call returns true at once. The call always acts on your own character, whichever unit it is called on.

The name is matched case-insensitively against D2BS's built-in English skill-name table, not the game's localized strings. "Shadow Master" and "Phoenix Strike" are not recognized by name; pass their ids (279 and 280) instead.

#### Parameters
* `skillName` Name of the skill to put up.

* `nHand` Hand to put the skill on: 0 - right, any other integer - left.

#### Returns
true once the hand shows the skill. Returns false when the skill name is unknown, nHand is missing or not an integer, your character does not have the skill (total level 0), you are not in a game, or the hand does not show the skill in time.

#### `public bool `[`setSkill`](#classUnit_1a3f88599825b6b93e1bf52f9c13df0a95)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSkillId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nHand)` 

Set the skill on the given hand to be skill with id nSkillId.

Asks the game to put the skill on the hand, then waits up to about one second for the hand to show it. When the hand already shows the skill, the call returns true at once. The call always acts on your own character, whichever unit it is called on.

#### Parameters
* `nSkillId` Id of the skill to put up. See [Skill ids](#skill-ids) in the reference tables.

* `nHand` Hand to put the skill on: 0 - right, any other integer - left.

#### Returns
true once the hand shows the skill. Returns false when the first argument is neither a string nor an integer, nHand is missing or not an integer, your character does not have the skill (total level 0), you are not in a game, or the hand does not show the skill in time.

#### `public bool `[`setSkill`](#classUnit_1setSkill_name_item)`(String skillName,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nHand,`[`Unit`](#classUnit)` item)` 

Set the skill on the given hand to be skill with name skillName.

Asks the game to put the skill on the hand, then waits up to about one second for the hand to show it. When the hand already shows the skill, the call returns true at once. The call always acts on your own character, whichever unit it is called on.

The name is matched case-insensitively against D2BS's built-in English skill-name table, not the game's localized strings. "Shadow Master" and "Phoenix Strike" are not recognized by name; pass their ids (279 and 280) instead.

#### Parameters
* `skillName` Name of the skill to put up.

* `nHand` Hand to put the skill on: 0 - right, any other integer - left.

* `item` An item unit: the hand gets that item's charged skill. A third argument that is not an item unit is ignored, and the skill is set as a normal skill. The item is not checked to be yours.

#### Returns
true once the hand shows the skill. Returns false when the skill name is unknown, nHand is missing or not an integer, your character does not have the skill (total level 0), you are not in a game, or the hand does not show the skill in time.

#### `public bool `[`setSkill`](#classUnit_1setSkill_id_item)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nSkillId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nHand,`[`Unit`](#classUnit)` item)` 

Set the skill on the given hand to be skill with id nSkillId.

Asks the game to put the skill on the hand, then waits up to about one second for the hand to show it. When the hand already shows the skill, the call returns true at once. The call always acts on your own character, whichever unit it is called on.

#### Parameters
* `nSkillId` Id of the skill to put up. See [Skill ids](#skill-ids) in the reference tables.

* `nHand` Hand to put the skill on: 0 - right, any other integer - left.

* `item` An item unit: the hand gets that item's charged skill. A third argument that is not an item unit is ignored, and the skill is set as a normal skill. The item is not checked to be yours.

#### Returns
true once the hand shows the skill. Returns false when the first argument is neither a string nor an integer, nHand is missing or not an integer, your character does not have the skill (total level 0), you are not in a game, or the hand does not show the skill in time.

#### `public void `[`move`](#classUnit_1a0b13c1bcd85e4a2fda08548feef07d0b)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` x,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` y)` 

Move to the given location.

#### Parameters
* `x` The x location.

* `y` The y location.

#### `public void `[`move`](#classUnit_1a4a295cc929f3a8ccf67287d4f6f5515b)`()` 

Move to this unit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getQuest`](#classUnit_1afbb11a6c2f4457b19fd464c1df647065)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nAct,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nQuest)` 

Get the quest flag for a quest specified by act and quest number.

#### Parameters
* `nAct` The act of the quest.

* `nQuest` The quest number.

#### Returns
The quest flag for the specified quest, or undefined when act and quest are not both integers.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getMinionCount`](#classUnit_1a1cf6810c2589b47f74cfa125375d75ed)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nType)` 

Get the number of minions of a certain type.

#### Parameters
* `nType` The type of the minions.

#### Returns
The number of minions of the specified type.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getRepairCost`](#classUnit_1ae007a9f727a4f34f40e5bb4d0867897b)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nNpcClassId)` 

Get price to repair this unit at the current npc given by nNpcClassId.

#### Parameters
* `nNpcClassId` The class id of the npc to get the price for repair at.

#### Returns
The price to repair the given unit.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getItemCost`](#classUnit_1a18eb04eaed8f97b76aacbd8b2734f563)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nNpcClassId)` 

Get the cost to do something (buy, sell, repair) with the given item, at the given npc.

#### Parameters
* `nMode` What to do: 0 - buy, 1 - sell, 2 - repair.

* `nNpcClassId` The class id of the npc to check the price with.

#### Returns
The price.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getItemCost`](#classUnit_1ac27394b9daab7a7c1c457ab6121910f8)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nNpcClassId,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nDifficulty)` 

Get the cost to do something (buy, sell, repair) with the given item, at the given npc, in the given difficulty.

#### Parameters
* `nMode` What to do: 0 - buy, 1 - sell, 2 - repair.

* `nNpcClassId` The class id of the npc to check the price with.

* `nDifficulty` The difficulty to check the price in.

#### Returns
The price.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getItemCost`](#classUnit_1a9db71041710e126f77d3a2eb2526fd9a)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode,`[`Unit`](#classUnit)` npc,`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nDifficulty)` 

Get the cost to do something (buy, sell, repair) with the given item, at the given npc, in the given difficulty.

#### Parameters
* `nMode` What to do: 0 - buy, 1 - sell, 2 - repair.

* `npc` The npc to check the price with.

* `nDifficulty` The difficulty to check the price in.

#### Returns
The price.

#### `public `[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` `[`getItemCost`](#classUnit_1ae375f1e4e46cdc39c29a947d7add4765)`(`[`int`](#group__handlers_1gad45ee50356f6b4f157faf0c8e44217ac)` nMode,`[`Unit`](#classUnit)` npc)` 

Get the cost to do something (buy, sell, repair) with the given item, at the given npc.

#### Parameters
* `nMode` What to do: 0 - buy, 1 - sell, 2 - repair.

* `npc` The npc to check the price with.

#### Returns
The price.

# Reference tables

These tables list the ids and flag values that the script API takes and returns. The area, skill and item-type tables come from the game's own 1.14d data tables.

## Area ids

Area (level) ids, as `unit.area`, `me.area`, `getArea()`, `getRoom()`, `getPresetUnit()`, `getPresetUnits()`, `getPath()`, `getCollision()` and a waypoint's `interact()` use them. `getWaypoint()` takes a waypoint index instead (the Levels.txt Waypoint column). An area id is the `Id` column of the game's Levels.txt; the table lists every 1.14d area. Name is the name the game shows, from its English string tables; Levels.txt LevelName is the key it is looked up by, which differs for 25 areas (120 Rocky Summit is the in-game Arreat Summit, and 125-127 Hell1-3 are Abaddon, Pit of Acheron and Infernal Pit). Scripts get the LevelName, not Name: `getArea(120).name` and `getBaseStat("levels", 120, "LevelName")` both return "Rocky Summit". The seven Tal Rasha's Tomb areas share one name; the internal name tells them apart, and `correcttomb` of any Act II room gives the id of the one whose Orifice leads to Tal Rasha's Chamber (73) in the current game. Act is the act the area belongs to (1-5; - for id 0). The calls that look up an area know only the areas the client has created for the player's act. In a new game some are missing at first (in a new Act 1 game, 21-25 and 29-38 were still missing 5 seconds after the game was ready), and such an area counts as not in the player's act.

Id | Name | Levels.txt LevelName | Internal name (Levels.txt Name) | Act
---|---|---|---|---
0 | (none) | - | Null | -
1 | Rogue Encampment | Rogue Encampment | Act 1 - Town | 1
2 | Blood Moor | Blood Moor | Act 1 - Wilderness 1 | 1
3 | Cold Plains | Cold Plains | Act 1 - Wilderness 2 | 1
4 | Stony Field | Stony Field | Act 1 - Wilderness 3 | 1
5 | Dark Wood | Dark Wood | Act 1 - Wilderness 4 | 1
6 | Black Marsh | Black Marsh | Act 1 - Wilderness 5 | 1
7 | Tamoe Highland | Tamoe Highland | Act 1 - Wilderness 6 | 1
8 | Den of Evil | Den of Evil | Act 1 - Cave 1 | 1
9 | Cave Level 1 | Cave Level 1 | Act 1 - Cave 2 | 1
10 | Underground Passage Level 1 | Underground Passage Level 1 | Act 1 - Cave 3 | 1
11 | Hole Level 1 | Hole Level 1 | Act 1 - Cave 4 | 1
12 | Pit Level 1 | Pit Level 1 | Act 1 - Cave 5 | 1
13 | Cave Level 2 | Cave Level 2 | Act 1 - Cave 2 Treasure | 1
14 | Underground Passage Level 2 | Underground Passage Level 2 | Act 1 - Cave 3 Treasure | 1
15 | Hole Level 2 | Hole Level 2 | Act 1 - Cave 4 Treasure | 1
16 | Pit Level 2 | Pit Level 2 | Act 1 - Cave 5 Treasure | 1
17 | Burial Grounds | Burial Grounds | Act 1 - Graveyard | 1
18 | Crypt | Crypt | Act 1 - Crypt 1 A | 1
19 | Mausoleum | Mausoleum | Act 1 - Crypt 2 A | 1
20 | Forgotten Tower | Forgotten Tower | Act 1 - Tower 2 | 1
21 | Tower Cellar Level 1 | Tower Cellar Level 1 | Act 1 - Crypt 3 A | 1
22 | Tower Cellar Level 2 | Tower Cellar Level 2 | Act 1 - Crypt 3 B | 1
23 | Tower Cellar Level 3 | Tower Cellar Level 3 | Act 1 - Crypt 3 C | 1
24 | Tower Cellar Level 4 | Tower Cellar Level 4 | Act 1 - Crypt 3 D | 1
25 | Tower Cellar Level 5 | Tower Cellar Level 5 | Act 1 - Crypt 3 E | 1
26 | Monastery Gate | Monastery Gate | Act 1 - Monastery | 1
27 | Outer Cloister | Outer Cloister | Act 1 - Courtyard 1 | 1
28 | Barracks | Barracks | Act 1 - Barracks | 1
29 | Jail Level 1 | Jail Level 1 | Act 1 - Jail 1 | 1
30 | Jail Level 2 | Jail Level 2 | Act 1 - Jail 2 | 1
31 | Jail Level 3 | Jail Level 3 | Act 1 - Jail 3 | 1
32 | Inner Cloister | Inner Cloister | Act 1 - Courtyard 2 | 1
33 | Cathedral | Cathedral | Act 1 - Cathedral | 1
34 | Catacombs Level 1 | Catacombs Level 1 | Act 1 - Catacombs 1 | 1
35 | Catacombs Level 2 | Catacombs Level 2 | Act 1 - Catacombs 2 | 1
36 | Catacombs Level 3 | Catacombs Level 3 | Act 1 - Catacombs 3 | 1
37 | Catacombs Level 4 | Catacombs Level 4 | Act 1 - Catacombs 4 | 1
38 | Tristram | Tristram | Act 1 - Tristram | 1
39 | The Secret Cow Level | Moo Moo Farm | Act 1 - Moo Moo Farm | 1
40 | Lut Gholein | Lut Gholein | Act 2 - Town | 2
41 | Rocky Waste | Rocky Waste | Act 2 - Desert 1 | 2
42 | Dry Hills | Dry Hills | Act 2 - Desert 2 | 2
43 | Far Oasis | Far Oasis | Act 2 - Desert 3 | 2
44 | Lost City | Lost City | Act 2 - Desert 4 | 2
45 | Valley of Snakes | Valley of Snakes | Act 2 - Desert 5 | 2
46 | Canyon of the Magi | Canyon of the Magi | Act 2 - Valley of the Kings | 2
47 | Sewers Level 1 | Sewers Level 1 | Act 2 - Sewer 1 A | 2
48 | Sewers Level 2 | Sewers Level 2 | Act 2 - Sewer 1 B | 2
49 | Sewers Level 3 | Sewers Level 3 | Act 2 - Sewer 1 C | 2
50 | Harem Level 1 | Harem Level 1 | Act 2 - Harem | 2
51 | Harem Level 2 | Harem Level 2 | Act 2 - Corrupt Harem 1 | 2
52 | Palace Cellar Level 1 | Palace Cellar Level 1 | Act 2 - Basement 1 | 2
53 | Palace Cellar Level 2 | Palace Cellar Level 2 | Act 2 - Basement 2 | 2
54 | Palace Cellar Level 3 | Palace Cellar Level 3 | Act 2 - Basement 3 | 2
55 | Stony Tomb Level 1 | Stony Tomb Level 1 | Act 2 - Tomb 1 A | 2
56 | Halls of the Dead Level 1 | Halls of the Dead Level 1 | Act 2 - Tomb 2 A | 2
57 | Halls of the Dead Level 2 | Halls of the Dead Level 2 | Act 2 - Tomb 2 B | 2
58 | Claw Viper Temple Level 1 | Claw Viper Temple Level 1 | Act 2 - Tomb 3 A | 2
59 | Stony Tomb Level 2 | Stony Tomb Level 2 | Act 2 - Tomb 1 Treasure | 2
60 | Halls of the Dead Level 3 | Halls of the Dead Level 3 | Act 2 - Tomb 2 Treasure | 2
61 | Claw Viper Temple Level 2 | Claw Viper Temple Level 2 | Act 2 - Tomb 3 Treasure | 2
62 | Maggot Lair Level 1 | Maggot Lair Level 1 | Act 2 - Lair 1 A | 2
63 | Maggot Lair Level 2 | Maggot Lair Level 2 | Act 2 - Lair 1 B | 2
64 | Maggot Lair Level 3 | Maggot Lair Level 3 | Act 2 - Lair 1 Treasure | 2
65 | Ancient Tunnels | Ancient Tunnels | Act 2 - Sewer 2 A | 2
66 | Tal Rasha's Tomb | Tal Rasha's Tomb | Act 2 - Tomb Tal 1 | 2
67 | Tal Rasha's Tomb | Tal Rasha's Tomb | Act 2 - Tomb Tal 2 | 2
68 | Tal Rasha's Tomb | Tal Rasha's Tomb | Act 2 - Tomb Tal 3 | 2
69 | Tal Rasha's Tomb | Tal Rasha's Tomb | Act 2 - Tomb Tal 4 | 2
70 | Tal Rasha's Tomb | Tal Rasha's Tomb | Act 2 - Tomb Tal 5 | 2
71 | Tal Rasha's Tomb | Tal Rasha's Tomb | Act 2 - Tomb Tal 6 | 2
72 | Tal Rasha's Tomb | Tal Rasha's Tomb | Act 2 - Tomb Tal 7 | 2
73 | Tal Rasha's Chamber | Duriel's Lair | Act 2 - Duriel's Lair | 2
74 | Arcane Sanctuary | Arcane Sanctuary | Act 2 - Arcane | 2
75 | Kurast Docks | Kurast Docktown | Act 3 - Town | 3
76 | Spider Forest | Spider Forest | Act 3 - Jungle 1 | 3
77 | Great Marsh | Great Marsh | Act 3 - Jungle 2 | 3
78 | Flayer Jungle | Flayer Jungle | Act 3 - Jungle 3 | 3
79 | Lower Kurast | Lower Kurast | Act 3 - Kurast 1 | 3
80 | Kurast Bazaar | Kurast Bazaar | Act 3 - Kurast 2 | 3
81 | Upper Kurast | Upper Kurast | Act 3 - Kurast 3 | 3
82 | Kurast Causeway | Kurast Causeway | Act 3 - Kurast 4 | 3
83 | Travincal | Travincal | Act 3 - Travincal | 3
84 | Arachnid Lair | Spider Cave | Act 3 - Spider 1 | 3
85 | Spider Cavern | Spider Cavern | Act 3 - Spider 2 | 3
86 | Swampy Pit Level 1 | Swampy Pit Level 1 | Act 3 - Dungeon 1 A | 3
87 | Swampy Pit Level 2 | Swampy Pit Level 2 | Act 3 - Dungeon 1 B | 3
88 | Flayer Dungeon Level 1 | Flayer Dungeon Level 1 | Act 3 - Dungeon 2 A | 3
89 | Flayer Dungeon Level 2 | Flayer Dungeon Level 2 | Act 3 - Dungeon 2 B | 3
90 | Swampy Pit Level 3 | Swampy Pit Level 3 | Act 3 - Dungeon 1 Treasure | 3
91 | Flayer Dungeon Level 3 | Flayer Dungeon Level 3 | Act 3 - Dungeon 2 Treasure | 3
92 | Sewers Level 1 | Sewers Level 1 | Act 3 - Sewer 1 | 3
93 | Sewers Level 2 | Sewers Level 2 | Act 3 - Sewer 2 | 3
94 | Ruined Temple | Ruined Temple | Act 3 - Temple 1 | 3
95 | Disused Fane | Disused Fane | Act 3 - Temple 2 | 3
96 | Forgotten Reliquary | Forgotten Reliquary | Act 3 - Temple 3 | 3
97 | Forgotten Temple | Forgotten Temple | Act 3 - Temple 4 | 3
98 | Ruined Fane | Ruined Fane | Act 3 - Temple 5 | 3
99 | Disused Reliquary | Disused Reliquary | Act 3 - Temple 6 | 3
100 | Durance of Hate Level 1 | Durance of Hate Level 1 | Act 3 - Mephisto 1 | 3
101 | Durance of Hate Level 2 | Durance of Hate Level 2 | Act 3 - Mephisto 2 | 3
102 | Durance of Hate Level 3 | Durance of Hate Level 3 | Act 3 - Mephisto 3 | 3
103 | The Pandemonium Fortress | The Pandemonium Fortress | Act 4 - Town | 4
104 | Outer Steppes | Outer Steppes | Act 4 - Mesa 1 | 4
105 | Plains of Despair | Plains of Despair | Act 4 - Mesa 2 | 4
106 | City of the Damned | City of the Damned | Act 4 - Mesa 3 | 4
107 | River of Flame | River of Flame | Act 4 - Lava 1 | 4
108 | The Chaos Sanctuary | Chaos Sanctum | Act 4 - Diablo 1 | 4
109 | Harrogath | Harrogath | Act 5 - Town | 5
110 | Bloody Foothills | Bloody Foothills | Act 5 - Siege 1 | 5
111 | Frigid Highlands | Rigid Highlands | Act 5 - Barricade 1 | 5
112 | Arreat Plateau | Arreat Plateau | Act 5 - Barricade 2 | 5
113 | Crystalline Passage | Crystalized Cavern Level 1 | Act 5 - Ice Cave 1 | 5
114 | Frozen River | Cellar of Pity | Act 5 - Ice Cave 1A | 5
115 | Glacial Trail | Crystalized Cavern Level 2 | Act 5 - Ice Cave 2 | 5
116 | Drifter Cavern | Echo Chamber | Act 5 - Ice Cave 2A | 5
117 | Frozen Tundra | Tundra Wastelands | Act 5 - Barricade Snow | 5
118 | The Ancients' Way | Glacial Caves Level 1 | Act 5 - Ice Cave 3 | 5
119 | Icy Cellar | Glacial Caves Level 2 | Act 5 - Ice Cave 3A | 5
120 | Arreat Summit | Rocky Summit | Act 5 - Mountain Top | 5
121 | Nihlathak's Temple | Nihlathaks Temple | Act 5 - Temple Entrance | 5
122 | Halls of Anguish | Halls of Anguish | Act 5 - Temple 1 | 5
123 | Halls of Pain | Halls of Death's Calling | Act 5 - Temple 2 | 5
124 | Halls of Vaught | Halls of Vaught | Act 5 - Temple Boss | 5
125 | Abaddon | Hell1 | Act 5 - Hell 1 | 5
126 | Pit of Acheron | Hell2 | Act 5 - Hell 2 | 5
127 | Infernal Pit | Hell3 | Act 5 - Hell 3 | 5
128 | Worldstone Keep Level 1 | The Worldstone Keep Level 1 | Act 5 - Baal Temple 1 | 5
129 | Worldstone Keep Level 2 | The Worldstone Keep Level 2 | Act 5 - Baal Temple 2 | 5
130 | Worldstone Keep Level 3 | The Worldstone Keep Level 3 | Act 5 - Baal Temple 3 | 5
131 | Throne of Destruction | Throne of Destruction | Act 5 - Throne Room | 5
132 | The Worldstone Chamber | The Worldstone Chamber | Act 5 - World Stone | 5
133 | Matron's Den | Pandemonium Run 1 | Act 5 - Pandemonium 1 | 5
134 | Forgotten Sands | Pandemonium Run 2 | Act 5 - Pandemonium 2 | 5
135 | Furnace of Pain | Pandemonium Run 3 | Act 5 - Pandemonium 3 | 5
136 | Tristram | Tristram | Act 5 - Pandemonium Finale | 5

## Skill ids

Skill ids, as `getSkill()`, `setSkill()`, `useSkillPoint()` and `getSkillById()` use them and `getSkillByName()` returns them: the `Id` column of the game's Skills.txt. Names are the Skills.txt names; the in-game names can differ. `getSkillByName()` and `setSkill()` with a name take D2BS's own English names instead, which differ from some of these (for example Werewolf, Fissure, Burst of Speed). Class skills are Amazon 6-35, Sorceress 36-65, Necromancer 66-95, Paladin 96-125, Barbarian 126-155, Druid 221-250 and Assassin 251-280; a character can spend skill points only on its own class's skills. The other ids are shared, monster and item skills (for example 217-220, the identify and town portal scrolls and tomes).

Id | Skill (Skills.txt skill) | Class
---|---|---
0 | Attack | -
1 | Kick | -
2 | Throw | -
3 | Unsummon | -
4 | Left Hand Throw | -
5 | Left Hand Swing | -
6 | Magic Arrow | Amazon
7 | Fire Arrow | Amazon
8 | Inner Sight | Amazon
9 | Critical Strike | Amazon
10 | Jab | Amazon
11 | Cold Arrow | Amazon
12 | Multiple Shot | Amazon
13 | Dodge | Amazon
14 | Power Strike | Amazon
15 | Poison Javelin | Amazon
16 | Exploding Arrow | Amazon
17 | Slow Missiles | Amazon
18 | Avoid | Amazon
19 | Impale | Amazon
20 | Lightning Bolt | Amazon
21 | Ice Arrow | Amazon
22 | Guided Arrow | Amazon
23 | Penetrate | Amazon
24 | Charged Strike | Amazon
25 | Plague Javelin | Amazon
26 | Strafe | Amazon
27 | Immolation Arrow | Amazon
28 | Dopplezon | Amazon
29 | Evade | Amazon
30 | Fend | Amazon
31 | Freezing Arrow | Amazon
32 | Valkyrie | Amazon
33 | Pierce | Amazon
34 | Lightning Strike | Amazon
35 | Lightning Fury | Amazon
36 | Fire Bolt | Sorceress
37 | Warmth | Sorceress
38 | Charged Bolt | Sorceress
39 | Ice Bolt | Sorceress
40 | Frozen Armor | Sorceress
41 | Inferno | Sorceress
42 | Static Field | Sorceress
43 | Telekinesis | Sorceress
44 | Frost Nova | Sorceress
45 | Ice Blast | Sorceress
46 | Blaze | Sorceress
47 | Fire Ball | Sorceress
48 | Nova | Sorceress
49 | Lightning | Sorceress
50 | Shiver Armor | Sorceress
51 | Fire Wall | Sorceress
52 | Enchant | Sorceress
53 | Chain Lightning | Sorceress
54 | Teleport | Sorceress
55 | Glacial Spike | Sorceress
56 | Meteor | Sorceress
57 | Thunder Storm | Sorceress
58 | Energy Shield | Sorceress
59 | Blizzard | Sorceress
60 | Chilling Armor | Sorceress
61 | Fire Mastery | Sorceress
62 | Hydra | Sorceress
63 | Lightning Mastery | Sorceress
64 | Frozen Orb | Sorceress
65 | Cold Mastery | Sorceress
66 | Amplify Damage | Necromancer
67 | Teeth | Necromancer
68 | Bone Armor | Necromancer
69 | Skeleton Mastery | Necromancer
70 | Raise Skeleton | Necromancer
71 | Dim Vision | Necromancer
72 | Weaken | Necromancer
73 | Poison Dagger | Necromancer
74 | Corpse Explosion | Necromancer
75 | Clay Golem | Necromancer
76 | Iron Maiden | Necromancer
77 | Terror | Necromancer
78 | Bone Wall | Necromancer
79 | Golem Mastery | Necromancer
80 | Raise Skeletal Mage | Necromancer
81 | Confuse | Necromancer
82 | Life Tap | Necromancer
83 | Poison Explosion | Necromancer
84 | Bone Spear | Necromancer
85 | BloodGolem | Necromancer
86 | Attract | Necromancer
87 | Decrepify | Necromancer
88 | Bone Prison | Necromancer
89 | Summon Resist | Necromancer
90 | IronGolem | Necromancer
91 | Lower Resist | Necromancer
92 | Poison Nova | Necromancer
93 | Bone Spirit | Necromancer
94 | FireGolem | Necromancer
95 | Revive | Necromancer
96 | Sacrifice | Paladin
97 | Smite | Paladin
98 | Might | Paladin
99 | Prayer | Paladin
100 | Resist Fire | Paladin
101 | Holy Bolt | Paladin
102 | Holy Fire | Paladin
103 | Thorns | Paladin
104 | Defiance | Paladin
105 | Resist Cold | Paladin
106 | Zeal | Paladin
107 | Charge | Paladin
108 | Blessed Aim | Paladin
109 | Cleansing | Paladin
110 | Resist Lightning | Paladin
111 | Vengeance | Paladin
112 | Blessed Hammer | Paladin
113 | Concentration | Paladin
114 | Holy Freeze | Paladin
115 | Vigor | Paladin
116 | Conversion | Paladin
117 | Holy Shield | Paladin
118 | Holy Shock | Paladin
119 | Sanctuary | Paladin
120 | Meditation | Paladin
121 | Fist of the Heavens | Paladin
122 | Fanaticism | Paladin
123 | Conviction | Paladin
124 | Redemption | Paladin
125 | Salvation | Paladin
126 | Bash | Barbarian
127 | Sword Mastery | Barbarian
128 | Axe Mastery | Barbarian
129 | Mace Mastery | Barbarian
130 | Howl | Barbarian
131 | Find Potion | Barbarian
132 | Leap | Barbarian
133 | Double Swing | Barbarian
134 | Pole Arm Mastery | Barbarian
135 | Throwing Mastery | Barbarian
136 | Spear Mastery | Barbarian
137 | Taunt | Barbarian
138 | Shout | Barbarian
139 | Stun | Barbarian
140 | Double Throw | Barbarian
141 | Increased Stamina | Barbarian
142 | Find Item | Barbarian
143 | Leap Attack | Barbarian
144 | Concentrate | Barbarian
145 | Iron Skin | Barbarian
146 | Battle Cry | Barbarian
147 | Frenzy | Barbarian
148 | Increased Speed | Barbarian
149 | Battle Orders | Barbarian
150 | Grim Ward | Barbarian
151 | Whirlwind | Barbarian
152 | Berserk | Barbarian
153 | Natural Resistance | Barbarian
154 | War Cry | Barbarian
155 | Battle Command | Barbarian
156 | Fire Hit | -
157 | UnHolyBolt | -
158 | SkeletonRaise | -
159 | MaggotEgg | -
160 | ShamanFire | -
161 | MagottUp | -
162 | MagottDown | -
163 | MagottLay | -
164 | AndrialSpray | -
165 | Jump | -
166 | Swarm Move | -
167 | Nest | -
168 | Quick Strike | -
169 | VampireFireball | -
170 | VampireFirewall | -
171 | VampireMeteor | -
172 | GargoyleTrap | -
173 | SpiderLay | -
174 | VampireHeal | -
175 | VampireRaise | -
176 | Submerge | -
177 | FetishAura | -
178 | FetishInferno | -
179 | ZakarumHeal | -
180 | Emerge | -
181 | Resurrect | -
182 | Bestow | -
183 | MissileSkill1 | -
184 | MonTeleport | -
185 | PrimeLightning | -
186 | PrimeBolt | -
187 | PrimeBlaze | -
188 | PrimeFirewall | -
189 | PrimeSpike | -
190 | PrimeIceNova | -
191 | PrimePoisonball | -
192 | PrimePoisonNova | -
193 | DiabLight | -
194 | DiabCold | -
195 | DiabFire | -
196 | FingerMageSpider | -
197 | DiabWall | -
198 | DiabRun | -
199 | DiabPrison | -
200 | PoisonBallTrap | -
201 | AndyPoisonBolt | -
202 | HireableMissile | -
203 | DesertTurret | -
204 | ArcaneTower | -
205 | MonBlizzard | -
206 | Mosquito | -
207 | CursedBallTrapRight | -
208 | CursedBallTrapLeft | -
209 | MonFrozenArmor | -
210 | MonBoneArmor | -
211 | MonBoneSpirit | -
212 | MonCurseCast | -
213 | HellMeteor | -
214 | RegurgitatorEat | -
215 | MonFrenzy | -
216 | QueenDeath | -
217 | Scroll of Identify | -
218 | Book of Identify | -
219 | Scroll of Townportal | -
220 | Book of Townportal | -
221 | Raven | Druid
222 | Plague Poppy | Druid
223 | Wearwolf | Druid
224 | Shape Shifting | Druid
225 | Firestorm | Druid
226 | Oak Sage | Druid
227 | Summon Spirit Wolf | Druid
228 | Wearbear | Druid
229 | Molten Boulder | Druid
230 | Arctic Blast | Druid
231 | Cycle of Life | Druid
232 | Feral Rage | Druid
233 | Maul | Druid
234 | Eruption | Druid
235 | Cyclone Armor | Druid
236 | Heart of Wolverine | Druid
237 | Summon Fenris | Druid
238 | Rabies | Druid
239 | Fire Claws | Druid
240 | Twister | Druid
241 | Vines | Druid
242 | Hunger | Druid
243 | Shock Wave | Druid
244 | Volcano | Druid
245 | Tornado | Druid
246 | Spirit of Barbs | Druid
247 | Summon Grizzly | Druid
248 | Fury | Druid
249 | Armageddon | Druid
250 | Hurricane | Druid
251 | Fire Trauma | Assassin
252 | Claw Mastery | Assassin
253 | Psychic Hammer | Assassin
254 | Tiger Strike | Assassin
255 | Dragon Talon | Assassin
256 | Shock Field | Assassin
257 | Blade Sentinel | Assassin
258 | Quickness | Assassin
259 | Fists of Fire | Assassin
260 | Dragon Claw | Assassin
261 | Charged Bolt Sentry | Assassin
262 | Wake of Fire Sentry | Assassin
263 | Weapon Block | Assassin
264 | Cloak of Shadows | Assassin
265 | Cobra Strike | Assassin
266 | Blade Fury | Assassin
267 | Fade | Assassin
268 | Shadow Warrior | Assassin
269 | Claws of Thunder | Assassin
270 | Dragon Tail | Assassin
271 | Lightning Sentry | Assassin
272 | Inferno Sentry | Assassin
273 | Mind Blast | Assassin
274 | Blades of Ice | Assassin
275 | Dragon Flight | Assassin
276 | Death Sentry | Assassin
277 | Blade Shield | Assassin
278 | Venom | Assassin
279 | Shadow Master | Assassin
280 | Royal Strike | Assassin
281 | Wake Of Destruction Sentry | -
282 | Imp Inferno | -
283 | Imp Fireball | -
284 | Baal Taunt | -
285 | Baal Corpse Explode | -
286 | Baal Monster Spawn | -
287 | Catapult Charged Ball | -
288 | Catapult Spike Ball | -
289 | Suck Blood | -
290 | Cry Help | -
291 | Healing Vortex | -
292 | Teleport 2 | -
293 | Self-resurrect | -
294 | Vine Attack | -
295 | Overseer Whip | -
296 | Barbs Aura | -
297 | Wolverine Aura | -
298 | Oak Sage Aura | -
299 | Imp Fire Missile | -
300 | Impregnate | -
301 | Siege Beast Stomp | -
302 | MinionSpawner | -
303 | CatapultBlizzard | -
304 | CatapultPlague | -
305 | CatapultMeteor | -
306 | BoltSentry | -
307 | CorpseCycler | -
308 | DeathMaul | -
309 | Defense Curse | -
310 | Blood Mana | -
311 | mon inferno sentry | -
312 | mon death sentry | -
313 | sentry lightning | -
314 | fenris rage | -
315 | Baal Tentacle | -
316 | Baal Nova | -
317 | Baal Inferno | -
318 | Baal Cold Missiles | -
319 | MegademonInferno | -
320 | EvilHutSpawner | -
321 | CountessFirewall | -
322 | ImpBolt | -
323 | Horror Arctic Blast | -
324 | death sentry ltng | -
325 | VineCycler | -
326 | BearSmite | -
327 | Resurrect2 | -
328 | BloodLordFrenzy | -
329 | Baal Teleport | -
330 | Imp Teleport | -
331 | Baal Clone Teleport | -
332 | ZakarumLightning | -
333 | VampireMissile | -
334 | MephistoMissile | -
335 | DoomKnightMissile | -
336 | RogueMissile | -
337 | HydraMissile | -
338 | NecromageMissile | -
339 | MonBow | -
340 | MonFireArrow | -
341 | MonColdArrow | -
342 | MonExplodingArrow | -
343 | MonFreezingArrow | -
344 | MonPowerStrike | -
345 | SuccubusBolt | -
346 | MephFrostNova | -
347 | MonIceSpear | -
348 | ShamanIce | -
349 | Diablogeddon | -
350 | Delerium Change | -
351 | NihlathakCorpseExplosion | -
352 | SerpentCharge | -
353 | Trap Nova | -
354 | UnHolyBoltEx | -
355 | ShamanFireEx | -
356 | Imp Fire Missile Ex | -

## Item types

Item types, as `unit.itemType` returns them: the item's base type, counted as the row of the game's ItemTypes.txt from 0 with the "Expansion" separator row left out. `itemType` gives only the first type of the item's base item. Names are the ItemTypes.txt names. For charms they differ from the in-game names: Medium Charm (`mcha`) is the in-game Large Charm and Large Charm (`lcha`) is the Grand Charm.

Index | Item type (ItemTypes.txt ItemType) | Code
---|---|---
0 | None | -
1 | None | -
2 | Shield | shie
3 | Armor | tors
4 | Gold | gold
5 | Bow Quiver | bowq
6 | Crossbow Quiver | xboq
7 | Player Body Part | play
8 | Herb | herb
9 | Potion | poti
10 | Ring | ring
11 | Elixir | elix
12 | Amulet | amul
13 | Charm | char
14 | Not Used | -
15 | Boots | boot
16 | Gloves | glov
17 | Not Used | -
18 | Book | book
19 | Belt | belt
20 | Gem | gem
21 | Torch | torc
22 | Scroll | scro
23 | Not Used | -
24 | Scepter | scep
25 | Wand | wand
26 | Staff | staf
27 | Bow | bow
28 | Axe | axe
29 | Club | club
30 | Sword | swor
31 | Hammer | hamm
32 | Knife | knif
33 | Spear | spea
34 | Polearm | pole
35 | Crossbow | xbow
36 | Mace | mace
37 | Helm | helm
38 | Missile Potion | tpot
39 | Quest | ques
40 | Body Part | body
41 | Key | key
42 | Throwing Knife | tkni
43 | Throwing Axe | taxe
44 | Javelin | jave
45 | Weapon | weap
46 | Melee Weapon | mele
47 | Missile Weapon | miss
48 | Thrown Weapon | thro
49 | Combo Weapon | comb
50 | Any Armor | armo
51 | Any Shield | shld
52 | Miscellaneous | misc
53 | Socket Filler | sock
54 | Second Hand | seco
55 | Staves And Rods | rod
56 | Missile | misl
57 | Blunt | blun
58 | Jewel | jewl
59 | Class Specific | clas
60 | Amazon Item | amaz
61 | Barbarian Item | barb
62 | Necromancer Item | necr
63 | Paladin Item | pala
64 | Sorceress Item | sorc
65 | Assassin Item | assn
66 | Druid Item | drui
67 | Hand to Hand | h2h
68 | Orb | orb
69 | Voodoo Heads | head
70 | Auric Shields | ashd
71 | Primal Helm | phlm
72 | Pelt | pelt
73 | Cloak | cloa
74 | Rune | rune
75 | Circlet | circ
76 | Healing Potion | hpot
77 | Mana Potion | mpot
78 | Rejuv Potion | rpot
79 | Stamina Potion | spot
80 | Antidote Potion | apot
81 | Thawing Potion | wpot
82 | Small Charm | scha
83 | Medium Charm | mcha
84 | Large Charm | lcha
85 | Amazon Bow | abow
86 | Amazon Spear | aspe
87 | Amazon Javelin | ajav
88 | Hand to Hand 2 | h2h2
89 | Magic Bow Quiv | mboq
90 | Magic Xbow Quiv | mxbq
91 | Chipped Gem | gem0
92 | Flawed Gem | gem1
93 | Standard Gem | gem2
94 | Flawless Gem | gem3
95 | Perfect Gem | gem4
96 | Amethyst | gema
97 | Diamond | gemd
98 | Emerald | geme
99 | Ruby | gemr
100 | Sapphire | gems
101 | Topaz | gemt
102 | Skull | gemz

## UI flags

Ids for `getUIFlag(id)`: the slots of the game's UI-state array. A slot reads true while the state in its "Set while" column holds: most slots are panels that read true while open, and 19 and 35 are on during normal play. Only 0-37 are valid: in a game, any other integer id (38 or more, or negative) makes the game exit at once, since D2BS does not check the range. The names are D2BS's C++ `Constants.h` names, which scripts do not have; pass the number. "Seen" marks the ids observed in a 1.14d single-player game (at rest, with each panel open, and with Alt held); the meanings of the other ids follow those names and kolbot and were not observed.

Id | Name | Set while | Seen
---|---|---|---
0 (0x00) | UI_GAME | a game is running | yes
1 (0x01) | UI_INVENTORY | the inventory panel is open | yes
2 (0x02) | UI_CHARACTER | the character (stats) panel is open | yes
3 (0x03) | UI_MINISKILL | the skill selection list is open (a click on the left skill button opens it) | yes
4 (0x04) | UI_SKILLTREE | the skill tree panel is open | yes
5 (0x05) | UI_CHAT_CONSOLE | the chat input box is open | yes
6 (0x06) | UI_UNK | unknown | no
7 (0x07) | UI_UNK_EX | unknown | no
8 (0x08) | UI_NPCMENU (UI_DIALOG) | an NPC's menu is open, and while that NPC's trade screen is open | yes
9 (0x09) | UI_ESCMENU_MAIN | the Escape game menu is open | yes
10 (0x0A) | UI_AUTOMAP | the automap is shown | yes
11 (0x0B) | UI_HOTKEY_CONFIG | the configure-controls screen is open | no
12 (0x0C) | UI_NPCSHOP | an NPC trade (shop) screen is open | yes
13 (0x0D) | UI_ALTDOWN (UI_GROUND_ITEMS) | the ground item labels are shown (Alt held) | yes
14 (0x0E) | UI_ANVIL | the submit-item panel is open (for example the Horadric Staff orifice) | no
15 (0x0F) | UI_QUEST | the quest log is open | yes
16 (0x10) | UI_BARKSCROLL | a scroll text screen is open (for example the Scroll of Inifuss) | no
17 (0x11) | UI_QUEST_LOG | unknown (the quest log sets 15, not 17) | no
18 (0x12) | UI_STATUS_AREA | unknown | no
19 (0x13) | UI_ESCMENU_EX | unknown; on during normal play (seen in town and in Cold Plains) | yes
20 (0x14) | UI_WPMENU | the waypoint menu is open | yes
21 (0x15) | UI_MINIPANEL | set together with 33 when the help screen opens | yes
22 (0x16) | UI_PARTY | the party screen is open | no
23 (0x17) | UI_TRADE | a trade with another player is open (the request prompt or the trade window) | no
24 (0x18) | UI_MESSAGE_LOG | the message log is open | yes
25 (0x19) | UI_STASH | the stash is open (it turns off when the cube is opened from the stash) | yes
26 (0x1A) | UI_CUBE | the Horadric Cube is open | yes
27 (0x1B) | UI_UNUSED | unknown | no
28 (0x1C) | UI_INVENTORY_EX | unknown | no
29 (0x1D) | UI_INVENTORY_EXX | unknown | no
30 (0x1E) | UI_UNUSED_EX | unknown | no
31 (0x1F) | UI_BELT | the expanded belt is shown | yes
32 (0x20) | UI_UNUSED_EXX | unknown | no
33 (0x21) | UI_HELP_MENU | the help screen is open | yes
34 (0x22) | UI_HELP_BUTTON | unknown | no
35 (0x23) | UI_ESCMENU | on during normal play; off while the Escape menu (9) is open | yes
36 (0x24) | UI_MERC | the mercenary screen is open | yes
37 (0x25) | UI_SCROLL | unknown | no

## Out-of-game locations

Values of `getLocation()`: the out-of-game screen the client is on. D2BS recognizes a screen by the buttons and text boxes it shows. `getLocation()` returns null while the client is not on the out-of-game menus: in a game, while a game loads, and between two screens. A popup that D2BS knows reports the popup rather than the screen under it. The names are D2BS's C++ `Constants.h` names, which scripts do not have; compare against the number. The character-select values (12, 13, 20, 23 and 42) need the expansion screen layout; the classic layout does not report them.

Value | Name | Screen
---|---|---
0 | OOG_NONE | On the menus, but no known screen matched
1 | OOG_LOBBY | Battle.net lobby
2 | OOG_INLINE | Waiting in line to create or join a game
3 | OOG_CHAT | Battle.net chat channel
4 | OOG_CREATE | Create Game panel
5 | OOG_JOIN | Join Game panel
6 | OOG_LADDER | Ladder panel, or any other lobby panel with its Cancel button in the same place
7 | OOG_CHANNEL | Channel list panel
8 | OOG_MAIN_MENU | Main menu
9 | OOG_LOGIN | Battle.net account login
10 | OOG_LOGIN_ERROR | Login error popup
11 | OOG_UNABLE_TO_CONNECT | Unable to connect to Battle.net
12 | OOG_CHAR_SELECT | Character select (expansion screen layout), with a character shown
13 | OOG_REALM_DOWN | Character select: the realm could not be reached, or the connection is temporarily restricted from it
14 | OOG_DISCONNECTED | Disconnected from Battle.net popup
15 | OOG_NEW_CHARACTER | Character create screen after a class is picked (its OK button is shown, greyed until a name is typed); also the fallback for any other screen with the Exit button at the bottom left
16 | OOG_CHARACTER_SELECT_PLEASE_WAIT | A "Please Wait" popup
17 | OOG_LOST_CONNECTION | Lost connection to Battle.net popup
18 | OOG_D2SPLASH | Splash screen
19 | OOG_CDKEY_IN_USE | CD key in use popup
20 | OOG_DIFFICULTY | Single player difficulty choice
21 | OOG_MAIN_MENU_CONNECTING | Connecting to Battle.net from the main menu
22 | OOG_INVALID_CDKEY | Invalid CD key popup, or another popup with its OK button in the same place
23 | OOG_CONNECTING | Character select: connecting to the realm
24 | OOG_SERVER_DOWN | Server down popup
25 | OOG_PLEASE_WAIT | A centered popup with a Cancel button and a message box, such as "Please wait" (16 is the same popup with its message higher up)
26 | OOG_GAME_EXIST | A game with that name already exists
27 | OOG_GATEWAY | Gateway (realm) list
28 | OOG_GAME_DOES_NOT_EXIST | Game does not exist
29 | OOG_CHARACTER_CREATE | Character create screen before a class is picked (its OK button is still hidden)
30 | OOG_CHARACTER_CREATE_ALREADY_EXISTS | Any other centered popup with an OK button (for example a duplicate character name)
31 | OOG_AGREE_TO_TERMS | Terms of use
32 | OOG_NEW_ACCOUNT | Create a new Battle.net account
33 | OOG_PLEASE_READ | "Please read" notice
34 | OOG_REGISTER_EMAIL | Register an email address
35 | OOG_CREDITS | Credits
36 | OOG_CINEMATICS | Cinematics menu
37 | OOG_CHARACTOR_CHANGE_REALM | Never returned (the change-realm screen reports 43)
38 | OOG_GAME_IS_FULL | Game is full
39 | OOG_OTHER_MULTIPLAYER | Other Multiplayer menu
40 | OOG_TCP_IP | TCP/IP game menu
41 | OOG_ENTER_IP_ADDRESS | TCP/IP: enter the host's IP address
42 | OOG_CHARACTER_SELECT_NO_CHARS | Character select with no characters (also briefly while the character list is still loading)
43 | OOG_CHARACTER_SELECT_CHANGE_REALM | Character select: change realm
44 | OOG_UNABLE_TO_CONNECT_TCPIP | TCP/IP: unable to connect to the host

## Collision flags

Bits of the 16-bit collision mask that `getCollision()` and `Room.getCollision()` return for each map subtile. `getCollision()` also returns 65535 (every bit set) for a point it cannot read and for barricade-tower spots; that value does not mean the bits below are set. `getPath()` with reduction 0, 1 or 2 treats a subtile as blocked when it or one of the 4 subtiles next to it has 0x1 or 0x8 set. The walking path (reduction 0) also gives a higher cost to a subtile when it or a subtile 2 away in a straight line has 0x1 or 0x8 set, and to subtiles with 0x400 (objects) or 0x800 (closed doors), so it keeps clear of them where it can without treating them as blocked. `getPath()` reads no other bit. The names are D2BS's own, from its pathing code. Where kolbot's `sdk.collision` name differs by more than a plural, it is given in brackets; kolbot's `sdk.collision.BlockWalk` is the combination 0x1805, not 0x1. The notes on where a bit was seen come from a 1.14d single-player game in the Rogue Encampment.

Bit | Name | Meaning
---|---|---
0x0001 | BlockWalk | Blocks walking (walls, cliffs, unwalkable space; also seen on a bonfire; kolbot: BlockWall)
0x0002 | BlockLineOfSight | Blocks line of sight (kolbot: LineOfSight)
0x0004 | Wall | Wall (kolbot calls it Ranged); in the room checked it was always set together with 0x1
0x0008 | BlockPlayer | Blocks players (kolbot: PlayerToWalk)
0x0010 | AlternateTile | Alternate floor tile (kolbot: DarkArea)
0x0020 | Blank | Blank subtile (kolbot: Casting)
0x0040 | Missile | Missile (kolbot: Unknown)
0x0080 | Player | A player's cell and the 4 cells next to it
0x0100 | NPCLocation | A monster's or NPC's cell and the cells next to it (kolbot: Monsters)
0x0200 | Item | An item's cell (seen for a gold pile)
0x0400 | Object | An object's cell (seen for a torch and a standard)
0x0800 | ClosedDoor | A closed door
0x1000 | NPCCollision | The cell the player stands on, and that of some NPCs (seen for Akara, Kashya and Warriv) (kolbot: IsOnFloor)
0x2000 | FriendlyNPC | The cell of some NPCs and animals, and for larger ones (the cows) the 4 cells next to it too (seen for the mercenary, the rogue guards and the cows)
0x4000 | Unknown | Unknown (kolbot: Unknown2)
0x8000 | DeadBody | A corpse, by its name (not seen in the game checked; a D2BS code comment says portals set it too)

## Box opacity

A Box's `opacity` is the game's draw mode for the rectangle (default 0). Only 0-7 are supported; the value is not range-checked, so use nothing else. The effects below were measured in a 1.14d single-player game running windowed (-w), where the game draws through its GDI renderer; other video modes were not checked. Blended results are snapped to the game's 256-color palette.

Mode | Effect
---|---
0 | The color blended over what is underneath, about 25% color (the default)
1 | The color blended over what is underneath, about 50% color
2 | The color blended over what is underneath, about 75% color
3 | Nothing visible: what is underneath is unchanged
4 | Black: the color is ignored
5 | The color, solid
6 | The color is ignored and what is underneath is darkened through a palette table (some colors stay as they are)
7 | The color, solid (the same as 5)

## Text colors

The game's 13 text colors, as `Text.color` takes them. Inside a Text hook's string, the character 0xFF (written `\xFF` in a script string) followed by `c` and one more character switches the color from that point on: `\xFFc0` to `\xFFc9` select 0-9, and `\xFFc:`, `\xFFc;` and `\xFFc<` select 10, 11 and 12. Other values are not defined. In a string, `\xFFc` followed by a character after `<` selects 0, and one before `0` gives an undefined color. A `\xFF` followed by anything other than `c` is not a color code and is drawn as an ordinary character. The names are the conventional ones (kolbot's); the RGB values were measured in a 1.14d game (glyphs also carry a dark shadow).

Value | Code | Color | RGB
---|---|---|---
0 | `\xFFc0` | White (the default) | C4C4C4
1 | `\xFFc1` | Red | B04434
2 | `\xFFc2` | Green (set items) | 18FC00
3 | `\xFFc3` | Blue (magic items) | 5050AC
4 | `\xFFc4` | Dark gold (unique items) | 948064
5 | `\xFFc5` | Gray | 505050
6 | `\xFFc6` | Black | 000000
7 | `\xFFc7` | Light gold | AC9C64
8 | `\xFFc8` | Orange (crafted items) | D08420
9 | `\xFFc9` | Yellow (rare items) | D8B864
10 | `\xFFc:` | Dark green | 186408
11 | `\xFFc;` | Purple | A420FC
12 | `\xFFc<` | Green | 287C14

## Fonts

The game's fonts, as `Text.font` and `getTextSize()` take them. Only 0-13 are valid: a larger value makes the game read past its font table and can crash it. The names are the game's own font names. The height is that of the text "Ag" drawn in a 1.14d game, in pixels, without the glyph shadow.

Value | Font | Height of "Ag"
---|---|---
0 | Font8 (the default) | 11
1 | Font16 | 10
2 | Font30 | 16
3 | Font42 | 24
4 | FontFormal10 | 12
5 | FontFormal12 | 15
6 | Font6 | 9
7 | Font24 | 14
8 | FontFormal11 | 12
9 | FontExocet10 | 13
10 | FontRidiculous | 9
11 | FontExocet8 | 8
12 | ReallyTheLastSucker | 9
13 | FontInGameChat | 12

Generated by [Moxygen](https://sourcey.com/moxygen)
