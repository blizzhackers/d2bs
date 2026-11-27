/** \page reference Reference tables
 *
 * These tables list the ids and flag values that the script API takes and returns. The area, skill and
 * item-type tables come from the game's own 1.14d data tables.
 *
 * \section reference_areas Area ids
 *
 * Area (level) ids, as `unit.area`, `me.area`, `getArea()`, `getRoom()`,
 * `getPresetUnit()` and `getPath()` use them. An area id is the `Id` column of
 * the game's Levels.txt; the table lists every 1.14d area. The seven Tal
 * Rasha's Tomb areas share one name; the internal name tells them apart, and
 * `correcttomb` of any Act II room gives the id of the one that holds Tal
 * Rasha's chamber in the current game. Act is the act the area belongs to
 * (1-5).
 *
 * Id | Name | Internal name (Levels.txt Name) | Act
 * ---|---|---|---
 * 0 | (none) | Null | -
 * 1 | Rogue Encampment | Act 1 - Town | 1
 * 2 | Blood Moor | Act 1 - Wilderness 1 | 1
 * 3 | Cold Plains | Act 1 - Wilderness 2 | 1
 * 4 | Stony Field | Act 1 - Wilderness 3 | 1
 * 5 | Dark Wood | Act 1 - Wilderness 4 | 1
 * 6 | Black Marsh | Act 1 - Wilderness 5 | 1
 * 7 | Tamoe Highland | Act 1 - Wilderness 6 | 1
 * 8 | Den of Evil | Act 1 - Cave 1 | 1
 * 9 | Cave Level 1 | Act 1 - Cave 2 | 1
 * 10 | Underground Passage Level 1 | Act 1 - Cave 3 | 1
 * 11 | Hole Level 1 | Act 1 - Cave 4 | 1
 * 12 | Pit Level 1 | Act 1 - Cave 5 | 1
 * 13 | Cave Level 2 | Act 1 - Cave 2 Treasure | 1
 * 14 | Underground Passage Level 2 | Act 1 - Cave 3 Treasure | 1
 * 15 | Hole Level 2 | Act 1 - Cave 4 Treasure | 1
 * 16 | Pit Level 2 | Act 1 - Cave 5 Treasure | 1
 * 17 | Burial Grounds | Act 1 - Graveyard | 1
 * 18 | Crypt | Act 1 - Crypt 1 A | 1
 * 19 | Mausoleum | Act 1 - Crypt 2 A | 1
 * 20 | Forgotten Tower | Act 1 - Tower 2 | 1
 * 21 | Tower Cellar Level 1 | Act 1 - Crypt 3 A | 1
 * 22 | Tower Cellar Level 2 | Act 1 - Crypt 3 B | 1
 * 23 | Tower Cellar Level 3 | Act 1 - Crypt 3 C | 1
 * 24 | Tower Cellar Level 4 | Act 1 - Crypt 3 D | 1
 * 25 | Tower Cellar Level 5 | Act 1 - Crypt 3 E | 1
 * 26 | Monastery Gate | Act 1 - Monastery | 1
 * 27 | Outer Cloister | Act 1 - Courtyard 1 | 1
 * 28 | Barracks | Act 1 - Barracks | 1
 * 29 | Jail Level 1 | Act 1 - Jail 1 | 1
 * 30 | Jail Level 2 | Act 1 - Jail 2 | 1
 * 31 | Jail Level 3 | Act 1 - Jail 3 | 1
 * 32 | Inner Cloister | Act 1 - Courtyard 2 | 1
 * 33 | Cathedral | Act 1 - Cathedral | 1
 * 34 | Catacombs Level 1 | Act 1 - Catacombs 1 | 1
 * 35 | Catacombs Level 2 | Act 1 - Catacombs 2 | 1
 * 36 | Catacombs Level 3 | Act 1 - Catacombs 3 | 1
 * 37 | Catacombs Level 4 | Act 1 - Catacombs 4 | 1
 * 38 | Tristram | Act 1 - Tristram | 1
 * 39 | Moo Moo Farm | Act 1 - Moo Moo Farm | 1
 * 40 | Lut Gholein | Act 2 - Town | 2
 * 41 | Rocky Waste | Act 2 - Desert 1 | 2
 * 42 | Dry Hills | Act 2 - Desert 2 | 2
 * 43 | Far Oasis | Act 2 - Desert 3 | 2
 * 44 | Lost City | Act 2 - Desert 4 | 2
 * 45 | Valley of Snakes | Act 2 - Desert 5 | 2
 * 46 | Canyon of the Magi | Act 2 - Valley of the Kings | 2
 * 47 | Sewers Level 1 | Act 2 - Sewer 1 A | 2
 * 48 | Sewers Level 2 | Act 2 - Sewer 1 B | 2
 * 49 | Sewers Level 3 | Act 2 - Sewer 1 C | 2
 * 50 | Harem Level 1 | Act 2 - Harem | 2
 * 51 | Harem Level 2 | Act 2 - Corrupt Harem 1 | 2
 * 52 | Palace Cellar Level 1 | Act 2 - Basement 1 | 2
 * 53 | Palace Cellar Level 2 | Act 2 - Basement 2 | 2
 * 54 | Palace Cellar Level 3 | Act 2 - Basement 3 | 2
 * 55 | Stony Tomb Level 1 | Act 2 - Tomb 1 A | 2
 * 56 | Halls of the Dead Level 1 | Act 2 - Tomb 2 A | 2
 * 57 | Halls of the Dead Level 2 | Act 2 - Tomb 2 B | 2
 * 58 | Claw Viper Temple Level 1 | Act 2 - Tomb 3 A | 2
 * 59 | Stony Tomb Level 2 | Act 2 - Tomb 1 Treasure | 2
 * 60 | Halls of the Dead Level 3 | Act 2 - Tomb 2 Treasure | 2
 * 61 | Claw Viper Temple Level 2 | Act 2 - Tomb 3 Treasure | 2
 * 62 | Maggot Lair Level 1 | Act 2 - Lair 1 A | 2
 * 63 | Maggot Lair Level 2 | Act 2 - Lair 1 B | 2
 * 64 | Maggot Lair Level 3 | Act 2 - Lair 1 Treasure | 2
 * 65 | Ancient Tunnels | Act 2 - Sewer 2 A | 2
 * 66 | Tal Rasha's Tomb | Act 2 - Tomb Tal 1 | 2
 * 67 | Tal Rasha's Tomb | Act 2 - Tomb Tal 2 | 2
 * 68 | Tal Rasha's Tomb | Act 2 - Tomb Tal 3 | 2
 * 69 | Tal Rasha's Tomb | Act 2 - Tomb Tal 4 | 2
 * 70 | Tal Rasha's Tomb | Act 2 - Tomb Tal 5 | 2
 * 71 | Tal Rasha's Tomb | Act 2 - Tomb Tal 6 | 2
 * 72 | Tal Rasha's Tomb | Act 2 - Tomb Tal 7 | 2
 * 73 | Duriel's Lair | Act 2 - Duriel's Lair | 2
 * 74 | Arcane Sanctuary | Act 2 - Arcane | 2
 * 75 | Kurast Docktown | Act 3 - Town | 3
 * 76 | Spider Forest | Act 3 - Jungle 1 | 3
 * 77 | Great Marsh | Act 3 - Jungle 2 | 3
 * 78 | Flayer Jungle | Act 3 - Jungle 3 | 3
 * 79 | Lower Kurast | Act 3 - Kurast 1 | 3
 * 80 | Kurast Bazaar | Act 3 - Kurast 2 | 3
 * 81 | Upper Kurast | Act 3 - Kurast 3 | 3
 * 82 | Kurast Causeway | Act 3 - Kurast 4 | 3
 * 83 | Travincal | Act 3 - Travincal | 3
 * 84 | Spider Cave | Act 3 - Spider 1 | 3
 * 85 | Spider Cavern | Act 3 - Spider 2 | 3
 * 86 | Swampy Pit Level 1 | Act 3 - Dungeon 1 A | 3
 * 87 | Swampy Pit Level 2 | Act 3 - Dungeon 1 B | 3
 * 88 | Flayer Dungeon Level 1 | Act 3 - Dungeon 2 A | 3
 * 89 | Flayer Dungeon Level 2 | Act 3 - Dungeon 2 B | 3
 * 90 | Swampy Pit Level 3 | Act 3 - Dungeon 1 Treasure | 3
 * 91 | Flayer Dungeon Level 3 | Act 3 - Dungeon 2 Treasure | 3
 * 92 | Sewers Level 1 | Act 3 - Sewer 1 | 3
 * 93 | Sewers Level 2 | Act 3 - Sewer 2 | 3
 * 94 | Ruined Temple | Act 3 - Temple 1 | 3
 * 95 | Disused Fane | Act 3 - Temple 2 | 3
 * 96 | Forgotten Reliquary | Act 3 - Temple 3 | 3
 * 97 | Forgotten Temple | Act 3 - Temple 4 | 3
 * 98 | Ruined Fane | Act 3 - Temple 5 | 3
 * 99 | Disused Reliquary | Act 3 - Temple 6 | 3
 * 100 | Durance of Hate Level 1 | Act 3 - Mephisto 1 | 3
 * 101 | Durance of Hate Level 2 | Act 3 - Mephisto 2 | 3
 * 102 | Durance of Hate Level 3 | Act 3 - Mephisto 3 | 3
 * 103 | The Pandemonium Fortress | Act 4 - Town | 4
 * 104 | Outer Steppes | Act 4 - Mesa 1 | 4
 * 105 | Plains of Despair | Act 4 - Mesa 2 | 4
 * 106 | City of the Damned | Act 4 - Mesa 3 | 4
 * 107 | River of Flame | Act 4 - Lava 1 | 4
 * 108 | Chaos Sanctum | Act 4 - Diablo 1 | 4
 * 109 | Harrogath | Act 5 - Town | 5
 * 110 | Bloody Foothills | Act 5 - Siege 1 | 5
 * 111 | Rigid Highlands | Act 5 - Barricade 1 | 5
 * 112 | Arreat Plateau | Act 5 - Barricade 2 | 5
 * 113 | Crystalized Cavern Level 1 | Act 5 - Ice Cave 1 | 5
 * 114 | Cellar of Pity | Act 5 - Ice Cave 1A | 5
 * 115 | Crystalized Cavern Level 2 | Act 5 - Ice Cave 2 | 5
 * 116 | Echo Chamber | Act 5 - Ice Cave 2A | 5
 * 117 | Tundra Wastelands | Act 5 - Barricade Snow | 5
 * 118 | Glacial Caves Level 1 | Act 5 - Ice Cave 3 | 5
 * 119 | Glacial Caves Level 2 | Act 5 - Ice Cave 3A | 5
 * 120 | Rocky Summit | Act 5 - Mountain Top | 5
 * 121 | Nihlathaks Temple | Act 5 - Temple Entrance | 5
 * 122 | Halls of Anguish | Act 5 - Temple 1 | 5
 * 123 | Halls of Death's Calling | Act 5 - Temple 2 | 5
 * 124 | Halls of Vaught | Act 5 - Temple Boss | 5
 * 125 | Hell1 | Act 5 - Hell 1 | 5
 * 126 | Hell2 | Act 5 - Hell 2 | 5
 * 127 | Hell3 | Act 5 - Hell 3 | 5
 * 128 | The Worldstone Keep Level 1 | Act 5 - Baal Temple 1 | 5
 * 129 | The Worldstone Keep Level 2 | Act 5 - Baal Temple 2 | 5
 * 130 | The Worldstone Keep Level 3 | Act 5 - Baal Temple 3 | 5
 * 131 | Throne of Destruction | Act 5 - Throne Room | 5
 * 132 | The Worldstone Chamber | Act 5 - World Stone | 5
 * 133 | Pandemonium Run 1 | Act 5 - Pandemonium 1 | 5
 * 134 | Pandemonium Run 2 | Act 5 - Pandemonium 2 | 5
 * 135 | Pandemonium Run 3 | Act 5 - Pandemonium 3 | 5
 * 136 | Tristram | Act 5 - Pandemonium Finale | 5
 *
 * \section reference_skills Skill ids
 *
 * Skill ids, as `getSkill()`, `setSkill()` and `useSkillPoint()` use them: the
 * `Id` column of the game's Skills.txt. Names are the Skills.txt names; the
 * in-game names can differ slightly. Class skills are Amazon 6-35, Sorceress
 * 36-65, Necromancer 66-95, Paladin 96-125, Barbarian 126-155, Druid 221-250
 * and Assassin 251-280; a character can spend skill points only on its own
 * class's skills. The other ids are shared, monster and item skills (for
 * example 217-220, the identify and town portal scrolls and tomes).
 *
 * Id | Skill (Skills.txt skill) | Class
 * ---|---|---
 * 0 | Attack | -
 * 1 | Kick | -
 * 2 | Throw | -
 * 3 | Unsummon | -
 * 4 | Left Hand Throw | -
 * 5 | Left Hand Swing | -
 * 6 | Magic Arrow | Amazon
 * 7 | Fire Arrow | Amazon
 * 8 | Inner Sight | Amazon
 * 9 | Critical Strike | Amazon
 * 10 | Jab | Amazon
 * 11 | Cold Arrow | Amazon
 * 12 | Multiple Shot | Amazon
 * 13 | Dodge | Amazon
 * 14 | Power Strike | Amazon
 * 15 | Poison Javelin | Amazon
 * 16 | Exploding Arrow | Amazon
 * 17 | Slow Missiles | Amazon
 * 18 | Avoid | Amazon
 * 19 | Impale | Amazon
 * 20 | Lightning Bolt | Amazon
 * 21 | Ice Arrow | Amazon
 * 22 | Guided Arrow | Amazon
 * 23 | Penetrate | Amazon
 * 24 | Charged Strike | Amazon
 * 25 | Plague Javelin | Amazon
 * 26 | Strafe | Amazon
 * 27 | Immolation Arrow | Amazon
 * 28 | Dopplezon | Amazon
 * 29 | Evade | Amazon
 * 30 | Fend | Amazon
 * 31 | Freezing Arrow | Amazon
 * 32 | Valkyrie | Amazon
 * 33 | Pierce | Amazon
 * 34 | Lightning Strike | Amazon
 * 35 | Lightning Fury | Amazon
 * 36 | Fire Bolt | Sorceress
 * 37 | Warmth | Sorceress
 * 38 | Charged Bolt | Sorceress
 * 39 | Ice Bolt | Sorceress
 * 40 | Frozen Armor | Sorceress
 * 41 | Inferno | Sorceress
 * 42 | Static Field | Sorceress
 * 43 | Telekinesis | Sorceress
 * 44 | Frost Nova | Sorceress
 * 45 | Ice Blast | Sorceress
 * 46 | Blaze | Sorceress
 * 47 | Fire Ball | Sorceress
 * 48 | Nova | Sorceress
 * 49 | Lightning | Sorceress
 * 50 | Shiver Armor | Sorceress
 * 51 | Fire Wall | Sorceress
 * 52 | Enchant | Sorceress
 * 53 | Chain Lightning | Sorceress
 * 54 | Teleport | Sorceress
 * 55 | Glacial Spike | Sorceress
 * 56 | Meteor | Sorceress
 * 57 | Thunder Storm | Sorceress
 * 58 | Energy Shield | Sorceress
 * 59 | Blizzard | Sorceress
 * 60 | Chilling Armor | Sorceress
 * 61 | Fire Mastery | Sorceress
 * 62 | Hydra | Sorceress
 * 63 | Lightning Mastery | Sorceress
 * 64 | Frozen Orb | Sorceress
 * 65 | Cold Mastery | Sorceress
 * 66 | Amplify Damage | Necromancer
 * 67 | Teeth | Necromancer
 * 68 | Bone Armor | Necromancer
 * 69 | Skeleton Mastery | Necromancer
 * 70 | Raise Skeleton | Necromancer
 * 71 | Dim Vision | Necromancer
 * 72 | Weaken | Necromancer
 * 73 | Poison Dagger | Necromancer
 * 74 | Corpse Explosion | Necromancer
 * 75 | Clay Golem | Necromancer
 * 76 | Iron Maiden | Necromancer
 * 77 | Terror | Necromancer
 * 78 | Bone Wall | Necromancer
 * 79 | Golem Mastery | Necromancer
 * 80 | Raise Skeletal Mage | Necromancer
 * 81 | Confuse | Necromancer
 * 82 | Life Tap | Necromancer
 * 83 | Poison Explosion | Necromancer
 * 84 | Bone Spear | Necromancer
 * 85 | BloodGolem | Necromancer
 * 86 | Attract | Necromancer
 * 87 | Decrepify | Necromancer
 * 88 | Bone Prison | Necromancer
 * 89 | Summon Resist | Necromancer
 * 90 | IronGolem | Necromancer
 * 91 | Lower Resist | Necromancer
 * 92 | Poison Nova | Necromancer
 * 93 | Bone Spirit | Necromancer
 * 94 | FireGolem | Necromancer
 * 95 | Revive | Necromancer
 * 96 | Sacrifice | Paladin
 * 97 | Smite | Paladin
 * 98 | Might | Paladin
 * 99 | Prayer | Paladin
 * 100 | Resist Fire | Paladin
 * 101 | Holy Bolt | Paladin
 * 102 | Holy Fire | Paladin
 * 103 | Thorns | Paladin
 * 104 | Defiance | Paladin
 * 105 | Resist Cold | Paladin
 * 106 | Zeal | Paladin
 * 107 | Charge | Paladin
 * 108 | Blessed Aim | Paladin
 * 109 | Cleansing | Paladin
 * 110 | Resist Lightning | Paladin
 * 111 | Vengeance | Paladin
 * 112 | Blessed Hammer | Paladin
 * 113 | Concentration | Paladin
 * 114 | Holy Freeze | Paladin
 * 115 | Vigor | Paladin
 * 116 | Conversion | Paladin
 * 117 | Holy Shield | Paladin
 * 118 | Holy Shock | Paladin
 * 119 | Sanctuary | Paladin
 * 120 | Meditation | Paladin
 * 121 | Fist of the Heavens | Paladin
 * 122 | Fanaticism | Paladin
 * 123 | Conviction | Paladin
 * 124 | Redemption | Paladin
 * 125 | Salvation | Paladin
 * 126 | Bash | Barbarian
 * 127 | Sword Mastery | Barbarian
 * 128 | Axe Mastery | Barbarian
 * 129 | Mace Mastery | Barbarian
 * 130 | Howl | Barbarian
 * 131 | Find Potion | Barbarian
 * 132 | Leap | Barbarian
 * 133 | Double Swing | Barbarian
 * 134 | Pole Arm Mastery | Barbarian
 * 135 | Throwing Mastery | Barbarian
 * 136 | Spear Mastery | Barbarian
 * 137 | Taunt | Barbarian
 * 138 | Shout | Barbarian
 * 139 | Stun | Barbarian
 * 140 | Double Throw | Barbarian
 * 141 | Increased Stamina | Barbarian
 * 142 | Find Item | Barbarian
 * 143 | Leap Attack | Barbarian
 * 144 | Concentrate | Barbarian
 * 145 | Iron Skin | Barbarian
 * 146 | Battle Cry | Barbarian
 * 147 | Frenzy | Barbarian
 * 148 | Increased Speed | Barbarian
 * 149 | Battle Orders | Barbarian
 * 150 | Grim Ward | Barbarian
 * 151 | Whirlwind | Barbarian
 * 152 | Berserk | Barbarian
 * 153 | Natural Resistance | Barbarian
 * 154 | War Cry | Barbarian
 * 155 | Battle Command | Barbarian
 * 156 | Fire Hit | -
 * 157 | UnHolyBolt | -
 * 158 | SkeletonRaise | -
 * 159 | MaggotEgg | -
 * 160 | ShamanFire | -
 * 161 | MagottUp | -
 * 162 | MagottDown | -
 * 163 | MagottLay | -
 * 164 | AndrialSpray | -
 * 165 | Jump | -
 * 166 | Swarm Move | -
 * 167 | Nest | -
 * 168 | Quick Strike | -
 * 169 | VampireFireball | -
 * 170 | VampireFirewall | -
 * 171 | VampireMeteor | -
 * 172 | GargoyleTrap | -
 * 173 | SpiderLay | -
 * 174 | VampireHeal | -
 * 175 | VampireRaise | -
 * 176 | Submerge | -
 * 177 | FetishAura | -
 * 178 | FetishInferno | -
 * 179 | ZakarumHeal | -
 * 180 | Emerge | -
 * 181 | Resurrect | -
 * 182 | Bestow | -
 * 183 | MissileSkill1 | -
 * 184 | MonTeleport | -
 * 185 | PrimeLightning | -
 * 186 | PrimeBolt | -
 * 187 | PrimeBlaze | -
 * 188 | PrimeFirewall | -
 * 189 | PrimeSpike | -
 * 190 | PrimeIceNova | -
 * 191 | PrimePoisonball | -
 * 192 | PrimePoisonNova | -
 * 193 | DiabLight | -
 * 194 | DiabCold | -
 * 195 | DiabFire | -
 * 196 | FingerMageSpider | -
 * 197 | DiabWall | -
 * 198 | DiabRun | -
 * 199 | DiabPrison | -
 * 200 | PoisonBallTrap | -
 * 201 | AndyPoisonBolt | -
 * 202 | HireableMissile | -
 * 203 | DesertTurret | -
 * 204 | ArcaneTower | -
 * 205 | MonBlizzard | -
 * 206 | Mosquito | -
 * 207 | CursedBallTrapRight | -
 * 208 | CursedBallTrapLeft | -
 * 209 | MonFrozenArmor | -
 * 210 | MonBoneArmor | -
 * 211 | MonBoneSpirit | -
 * 212 | MonCurseCast | -
 * 213 | HellMeteor | -
 * 214 | RegurgitatorEat | -
 * 215 | MonFrenzy | -
 * 216 | QueenDeath | -
 * 217 | Scroll of Identify | -
 * 218 | Book of Identify | -
 * 219 | Scroll of Townportal | -
 * 220 | Book of Townportal | -
 * 221 | Raven | Druid
 * 222 | Plague Poppy | Druid
 * 223 | Wearwolf | Druid
 * 224 | Shape Shifting | Druid
 * 225 | Firestorm | Druid
 * 226 | Oak Sage | Druid
 * 227 | Summon Spirit Wolf | Druid
 * 228 | Wearbear | Druid
 * 229 | Molten Boulder | Druid
 * 230 | Arctic Blast | Druid
 * 231 | Cycle of Life | Druid
 * 232 | Feral Rage | Druid
 * 233 | Maul | Druid
 * 234 | Eruption | Druid
 * 235 | Cyclone Armor | Druid
 * 236 | Heart of Wolverine | Druid
 * 237 | Summon Fenris | Druid
 * 238 | Rabies | Druid
 * 239 | Fire Claws | Druid
 * 240 | Twister | Druid
 * 241 | Vines | Druid
 * 242 | Hunger | Druid
 * 243 | Shock Wave | Druid
 * 244 | Volcano | Druid
 * 245 | Tornado | Druid
 * 246 | Spirit of Barbs | Druid
 * 247 | Summon Grizzly | Druid
 * 248 | Fury | Druid
 * 249 | Armageddon | Druid
 * 250 | Hurricane | Druid
 * 251 | Fire Trauma | Assassin
 * 252 | Claw Mastery | Assassin
 * 253 | Psychic Hammer | Assassin
 * 254 | Tiger Strike | Assassin
 * 255 | Dragon Talon | Assassin
 * 256 | Shock Field | Assassin
 * 257 | Blade Sentinel | Assassin
 * 258 | Quickness | Assassin
 * 259 | Fists of Fire | Assassin
 * 260 | Dragon Claw | Assassin
 * 261 | Charged Bolt Sentry | Assassin
 * 262 | Wake of Fire Sentry | Assassin
 * 263 | Weapon Block | Assassin
 * 264 | Cloak of Shadows | Assassin
 * 265 | Cobra Strike | Assassin
 * 266 | Blade Fury | Assassin
 * 267 | Fade | Assassin
 * 268 | Shadow Warrior | Assassin
 * 269 | Claws of Thunder | Assassin
 * 270 | Dragon Tail | Assassin
 * 271 | Lightning Sentry | Assassin
 * 272 | Inferno Sentry | Assassin
 * 273 | Mind Blast | Assassin
 * 274 | Blades of Ice | Assassin
 * 275 | Dragon Flight | Assassin
 * 276 | Death Sentry | Assassin
 * 277 | Blade Shield | Assassin
 * 278 | Venom | Assassin
 * 279 | Shadow Master | Assassin
 * 280 | Royal Strike | Assassin
 * 281 | Wake Of Destruction Sentry | -
 * 282 | Imp Inferno | -
 * 283 | Imp Fireball | -
 * 284 | Baal Taunt | -
 * 285 | Baal Corpse Explode | -
 * 286 | Baal Monster Spawn | -
 * 287 | Catapult Charged Ball | -
 * 288 | Catapult Spike Ball | -
 * 289 | Suck Blood | -
 * 290 | Cry Help | -
 * 291 | Healing Vortex | -
 * 292 | Teleport 2 | -
 * 293 | Self-resurrect | -
 * 294 | Vine Attack | -
 * 295 | Overseer Whip | -
 * 296 | Barbs Aura | -
 * 297 | Wolverine Aura | -
 * 298 | Oak Sage Aura | -
 * 299 | Imp Fire Missile | -
 * 300 | Impregnate | -
 * 301 | Siege Beast Stomp | -
 * 302 | MinionSpawner | -
 * 303 | CatapultBlizzard | -
 * 304 | CatapultPlague | -
 * 305 | CatapultMeteor | -
 * 306 | BoltSentry | -
 * 307 | CorpseCycler | -
 * 308 | DeathMaul | -
 * 309 | Defense Curse | -
 * 310 | Blood Mana | -
 * 311 | mon inferno sentry | -
 * 312 | mon death sentry | -
 * 313 | sentry lightning | -
 * 314 | fenris rage | -
 * 315 | Baal Tentacle | -
 * 316 | Baal Nova | -
 * 317 | Baal Inferno | -
 * 318 | Baal Cold Missiles | -
 * 319 | MegademonInferno | -
 * 320 | EvilHutSpawner | -
 * 321 | CountessFirewall | -
 * 322 | ImpBolt | -
 * 323 | Horror Arctic Blast | -
 * 324 | death sentry ltng | -
 * 325 | VineCycler | -
 * 326 | BearSmite | -
 * 327 | Resurrect2 | -
 * 328 | BloodLordFrenzy | -
 * 329 | Baal Teleport | -
 * 330 | Imp Teleport | -
 * 331 | Baal Clone Teleport | -
 * 332 | ZakarumLightning | -
 * 333 | VampireMissile | -
 * 334 | MephistoMissile | -
 * 335 | DoomKnightMissile | -
 * 336 | RogueMissile | -
 * 337 | HydraMissile | -
 * 338 | NecromageMissile | -
 * 339 | MonBow | -
 * 340 | MonFireArrow | -
 * 341 | MonColdArrow | -
 * 342 | MonExplodingArrow | -
 * 343 | MonFreezingArrow | -
 * 344 | MonPowerStrike | -
 * 345 | SuccubusBolt | -
 * 346 | MephFrostNova | -
 * 347 | MonIceSpear | -
 * 348 | ShamanIce | -
 * 349 | Diablogeddon | -
 * 350 | Delerium Change | -
 * 351 | NihlathakCorpseExplosion | -
 * 352 | SerpentCharge | -
 * 353 | Trap Nova | -
 * 354 | UnHolyBoltEx | -
 * 355 | ShamanFireEx | -
 * 356 | Imp Fire Missile Ex | -
 *
 * \section reference_itemtypes Item types
 *
 * Item types, as `unit.itemType` returns them: the item's base type, counted
 * as the row of the game's ItemTypes.txt from 0 with the "Expansion" separator
 * row left out. `itemType` gives only the first type of the item's base item.
 * Names are the ItemTypes.txt names. For charms they differ from the in-game
 * names: Medium Charm (`mcha`) is the in-game Large Charm and Large Charm
 * (`lcha`) is the Grand Charm.
 *
 * Index | Item type (ItemTypes.txt ItemType) | Code
 * ---|---|---
 * 0 | None | -
 * 1 | None | -
 * 2 | Shield | shie
 * 3 | Armor | tors
 * 4 | Gold | gold
 * 5 | Bow Quiver | bowq
 * 6 | Crossbow Quiver | xboq
 * 7 | Player Body Part | play
 * 8 | Herb | herb
 * 9 | Potion | poti
 * 10 | Ring | ring
 * 11 | Elixir | elix
 * 12 | Amulet | amul
 * 13 | Charm | char
 * 14 | Not Used | -
 * 15 | Boots | boot
 * 16 | Gloves | glov
 * 17 | Not Used | -
 * 18 | Book | book
 * 19 | Belt | belt
 * 20 | Gem | gem
 * 21 | Torch | torc
 * 22 | Scroll | scro
 * 23 | Not Used | -
 * 24 | Scepter | scep
 * 25 | Wand | wand
 * 26 | Staff | staf
 * 27 | Bow | bow
 * 28 | Axe | axe
 * 29 | Club | club
 * 30 | Sword | swor
 * 31 | Hammer | hamm
 * 32 | Knife | knif
 * 33 | Spear | spea
 * 34 | Polearm | pole
 * 35 | Crossbow | xbow
 * 36 | Mace | mace
 * 37 | Helm | helm
 * 38 | Missile Potion | tpot
 * 39 | Quest | ques
 * 40 | Body Part | body
 * 41 | Key | key
 * 42 | Throwing Knife | tkni
 * 43 | Throwing Axe | taxe
 * 44 | Javelin | jave
 * 45 | Weapon | weap
 * 46 | Melee Weapon | mele
 * 47 | Missile Weapon | miss
 * 48 | Thrown Weapon | thro
 * 49 | Combo Weapon | comb
 * 50 | Any Armor | armo
 * 51 | Any Shield | shld
 * 52 | Miscellaneous | misc
 * 53 | Socket Filler | sock
 * 54 | Second Hand | seco
 * 55 | Staves And Rods | rod
 * 56 | Missile | misl
 * 57 | Blunt | blun
 * 58 | Jewel | jewl
 * 59 | Class Specific | clas
 * 60 | Amazon Item | amaz
 * 61 | Barbarian Item | barb
 * 62 | Necromancer Item | necr
 * 63 | Paladin Item | pala
 * 64 | Sorceress Item | sorc
 * 65 | Assassin Item | assn
 * 66 | Druid Item | drui
 * 67 | Hand to Hand | h2h
 * 68 | Orb | orb
 * 69 | Voodoo Heads | head
 * 70 | Auric Shields | ashd
 * 71 | Primal Helm | phlm
 * 72 | Pelt | pelt
 * 73 | Cloak | cloa
 * 74 | Rune | rune
 * 75 | Circlet | circ
 * 76 | Healing Potion | hpot
 * 77 | Mana Potion | mpot
 * 78 | Rejuv Potion | rpot
 * 79 | Stamina Potion | spot
 * 80 | Antidote Potion | apot
 * 81 | Thawing Potion | wpot
 * 82 | Small Charm | scha
 * 83 | Medium Charm | mcha
 * 84 | Large Charm | lcha
 * 85 | Amazon Bow | abow
 * 86 | Amazon Spear | aspe
 * 87 | Amazon Javelin | ajav
 * 88 | Hand to Hand 2 | h2h2
 * 89 | Magic Bow Quiv | mboq
 * 90 | Magic Xbow Quiv | mxbq
 * 91 | Chipped Gem | gem0
 * 92 | Flawed Gem | gem1
 * 93 | Standard Gem | gem2
 * 94 | Flawless Gem | gem3
 * 95 | Perfect Gem | gem4
 * 96 | Amethyst | gema
 * 97 | Diamond | gemd
 * 98 | Emerald | geme
 * 99 | Ruby | gemr
 * 100 | Sapphire | gems
 * 101 | Topaz | gemt
 * 102 | Skull | gemz
 *
 * \section reference_uiflags UI flags
 *
 * Ids for `getUIFlag(id)`: the slots of the game's UI-state array. A slot
 * reads true while its panel or mode is open. Only 0-37 are valid: any other
 * id (38 or more, or negative) makes the game exit at once. The names are
 * D2BS's `Constants.h` names. "Seen" marks the ids observed in a 1.14d
 * single-player game by opening each panel; the meanings of the other ids
 * follow those names and kolbot and were not observed.
 *
 * Id | Name | Set while | Seen
 * ---|---|---|---
 * 0 (0x00) | UI_GAME | a game is running | yes
 * 1 (0x01) | UI_INVENTORY | the inventory panel is open | yes
 * 2 (0x02) | UI_CHARACTER | the character (stats) panel is open | yes
 * 3 (0x03) | UI_MINISKILL | the skill selection list is open | no
 * 4 (0x04) | UI_SKILLTREE | the skill tree panel is open | yes
 * 5 (0x05) | UI_CHAT_CONSOLE | the chat input box is open | yes
 * 6 (0x06) | UI_UNK | unknown | no
 * 7 (0x07) | UI_UNK_EX | unknown | no
 * 8 (0x08) | UI_NPCMENU (UI_DIALOG) | an NPC's menu is open, and while that NPC's trade screen is open | yes
 * 9 (0x09) | UI_ESCMENU_MAIN | the Escape game menu is open | yes
 * 10 (0x0A) | UI_AUTOMAP | the automap is shown | yes
 * 11 (0x0B) | UI_HOTKEY_CONFIG | the configure-controls screen is open | no
 * 12 (0x0C) | UI_NPCSHOP | an NPC trade (shop) screen is open | yes
 * 13 (0x0D) | UI_ALTDOWN (UI_GROUND_ITEMS) | the ground item labels are shown (Alt held) | yes
 * 14 (0x0E) | UI_ANVIL | the submit-item panel is open (for example the Horadric Staff orifice) | no
 * 15 (0x0F) | UI_QUEST | the quest log is open | yes
 * 16 (0x10) | UI_BARKSCROLL | a scroll text screen is open (for example the Scroll of Inifuss) | no
 * 17 (0x11) | UI_QUEST_LOG | unknown (the quest log sets 15, not 17) | no
 * 18 (0x12) | UI_STATUS_AREA | unknown | no
 * 19 (0x13) | UI_ESCMENU_EX | unknown; on during normal play (seen in town and in Cold Plains) | yes
 * 20 (0x14) | UI_WPMENU | the waypoint menu is open | yes
 * 21 (0x15) | UI_MINIPANEL | set together with 33 when the help screen opens | yes
 * 22 (0x16) | UI_PARTY | the party screen is open | no
 * 23 (0x17) | UI_TRADE | a trade with another player is open (the request prompt or the trade window) | no
 * 24 (0x18) | UI_MESSAGE_LOG | the message log is open | yes
 * 25 (0x19) | UI_STASH | the stash is open (it turns off when the cube is opened from the stash) | yes
 * 26 (0x1A) | UI_CUBE | the Horadric Cube is open | yes
 * 27 (0x1B) | UI_UNUSED | unknown | no
 * 28 (0x1C) | UI_INVENTORY_EX | unknown | no
 * 29 (0x1D) | UI_INVENTORY_EXX | unknown | no
 * 30 (0x1E) | UI_UNUSED_EX | unknown | no
 * 31 (0x1F) | UI_BELT | the expanded belt is shown | yes
 * 32 (0x20) | UI_UNUSED_EXX | unknown | no
 * 33 (0x21) | UI_HELP_MENU | the help screen is open | yes
 * 34 (0x22) | UI_HELP_BUTTON | unknown | no
 * 35 (0x23) | UI_ESCMENU | on during normal play; off while the Escape menu (9) is open | yes
 * 36 (0x24) | UI_MERC | the mercenary screen is open | yes
 * 37 (0x25) | UI_SCROLL | unknown | no
 *
 * \section reference_locations Out-of-game locations
 *
 * Values of `getLocation()`: the out-of-game screen the client is on. D2BS
 * recognizes a screen by the buttons and text boxes it shows. `getLocation()`
 * returns null while the client is not on the out-of-game menus: in a game,
 * while a game loads, and between two screens. A popup that D2BS knows reports
 * the popup rather than the screen under it. The names are D2BS's
 * `Constants.h` names.
 *
 * Value | Name | Screen
 * ---|---|---
 * 0 | OOG_NONE | On the menus, but no known screen matched
 * 1 | OOG_LOBBY | Battle.net lobby
 * 2 | OOG_INLINE | Waiting in line to create or join a game
 * 3 | OOG_CHAT | Battle.net chat channel
 * 4 | OOG_CREATE | Create Game panel
 * 5 | OOG_JOIN | Join Game panel
 * 6 | OOG_LADDER | Ladder panel
 * 7 | OOG_CHANNEL | Channel list panel
 * 8 | OOG_MAIN_MENU | Main menu
 * 9 | OOG_LOGIN | Battle.net account login
 * 10 | OOG_LOGIN_ERROR | Login error popup
 * 11 | OOG_UNABLE_TO_CONNECT | Unable to connect to Battle.net
 * 12 | OOG_CHAR_SELECT | Character select, with a character shown
 * 13 | OOG_REALM_DOWN | Character select: the realm could not be reached, or the connection is temporarily restricted from it
 * 14 | OOG_DISCONNECTED | Disconnected from Battle.net popup
 * 15 | OOG_NEW_CHARACTER | Character create screen while its OK button is disabled
 * 16 | OOG_CHARACTER_SELECT_PLEASE_WAIT | A "Please Wait" popup
 * 17 | OOG_LOST_CONNECTION | Lost connection to Battle.net popup
 * 18 | OOG_D2SPLASH | Splash screen
 * 19 | OOG_CDKEY_IN_USE | CD key in use popup
 * 20 | OOG_DIFFICULTY | Single player difficulty choice
 * 21 | OOG_MAIN_MENU_CONNECTING | Connecting to Battle.net from the main menu
 * 22 | OOG_INVALID_CDKEY | Invalid CD key popup, or another popup with its OK button in the same place
 * 23 | OOG_CONNECTING | Character select: connecting to the realm
 * 24 | OOG_SERVER_DOWN | Server down popup
 * 25 | OOG_PLEASE_WAIT | "Please wait" popup with a Cancel button
 * 26 | OOG_GAME_EXIST | A game with that name already exists
 * 27 | OOG_GATEWAY | Gateway (realm) list
 * 28 | OOG_GAME_DOES_NOT_EXIST | Game does not exist
 * 29 | OOG_CHARACTER_CREATE | Character create screen while its OK button is enabled
 * 30 | OOG_CHARACTER_CREATE_ALREADY_EXISTS | Any other centered popup with an OK button (for example a duplicate character name)
 * 31 | OOG_AGREE_TO_TERMS | Terms of use
 * 32 | OOG_NEW_ACCOUNT | Create a new Battle.net account
 * 33 | OOG_PLEASE_READ | "Please read" notice
 * 34 | OOG_REGISTER_EMAIL | Register an email address
 * 35 | OOG_CREDITS | Credits
 * 36 | OOG_CINEMATICS | Cinematics menu
 * 37 | OOG_CHARACTOR_CHANGE_REALM | Never returned (the change-realm screen reports 43)
 * 38 | OOG_GAME_IS_FULL | Game is full
 * 39 | OOG_OTHER_MULTIPLAYER | Other Multiplayer menu
 * 40 | OOG_TCP_IP | TCP/IP game menu
 * 41 | OOG_ENTER_IP_ADDRESS | TCP/IP: enter the host's IP address
 * 42 | OOG_CHARACTER_SELECT_NO_CHARS | Character select with no characters (also briefly while the character list is still loading)
 * 43 | OOG_CHARACTER_SELECT_CHANGE_REALM | Character select: change realm
 * 44 | OOG_UNABLE_TO_CONNECT_TCPIP | TCP/IP: unable to connect to the host
 *
 * \section reference_collision Collision flags
 *
 * Bits of the 16-bit collision mask that `getCollision()` and
 * `Room.getCollision()` return for each map subtile. D2BS's own pathing treats
 * 0x1 as blocking walking and 0x2 as blocking line of sight, and also treats
 * 0x8, 0x400 and 0x1000 as not walkable. The names are the conventional names
 * used by D2BS and kolbot; the notes on where a bit was seen come from a 1.14d
 * single-player game in the Rogue Encampment.
 *
 * Bit | Name | Meaning
 * ---|---|---
 * 0x0001 | BlockWalk | Blocks walking (walls, cliffs, unwalkable space; also seen on a bonfire)
 * 0x0002 | BlockLineOfSight | Blocks line of sight
 * 0x0004 | Wall | Wall (kolbot calls it Ranged); in the room checked it was always set together with 0x1
 * 0x0008 | BlockPlayer | Blocks players (kolbot: PlayerToWalk)
 * 0x0010 | AlternateTile | Alternate floor tile (kolbot: DarkArea)
 * 0x0020 | Blank | Blank subtile (kolbot: Casting)
 * 0x0040 | Missile | Missile
 * 0x0080 | Player | A player's cell and the 4 cells next to it
 * 0x0100 | NPCLocation | A monster's or NPC's cell and the cells next to it
 * 0x0200 | Item | An item's cell (seen for a gold pile)
 * 0x0400 | Object | An object's cell (seen for a torch and a standard)
 * 0x0800 | ClosedDoor | A closed door
 * 0x1000 | NPCCollision | The cell the player stands on, and that of some NPCs (seen for Akara, Kashya and Warriv) (kolbot: IsOnFloor)
 * 0x2000 | FriendlyNPC | The cell of some NPCs and animals, and for larger ones (the cows) the 4 cells next to it too (seen for the mercenary, the rogue guards and the cows)
 * 0x4000 | Unknown | Unknown
 * 0x8000 | DeadBody | A corpse (also set by portals)
 *
 * \section reference_textcolors Text colors
 *
 * The game's 13 text colors, as `Text.color` takes them. Inside a Text hook's
 * string, the character 0xFF (written `\xFF` in a script string) followed by
 * `c` and one more character switches the color from that point on: `\xFFc0`
 * to `\xFFc9` select 0-9, and `\xFFc:`, `\xFFc;` and `\xFFc<` select 10, 11
 * and 12. Other values are not defined. In a string, `\xFFc` followed by a
 * character after `<` selects 0, and one before `0` gives an undefined color.
 * The names are the conventional ones (kolbot's); the RGB values were measured
 * in a 1.14d game (glyphs also carry a dark shadow).
 *
 * Value | Code | Color | RGB
 * ---|---|---|---
 * 0 | `\xFFc0` | White (the default) | C4C4C4
 * 1 | `\xFFc1` | Red | B04434
 * 2 | `\xFFc2` | Green (set items) | 18FC00
 * 3 | `\xFFc3` | Blue (magic items) | 5050AC
 * 4 | `\xFFc4` | Dark gold (unique items) | 948064
 * 5 | `\xFFc5` | Gray | 505050
 * 6 | `\xFFc6` | Black | 000000
 * 7 | `\xFFc7` | Light gold | AC9C64
 * 8 | `\xFFc8` | Orange (crafted items) | D08420
 * 9 | `\xFFc9` | Yellow (rare items) | D8B864
 * 10 | `\xFFc:` | Dark green | 186408
 * 11 | `\xFFc;` | Purple | A420FC
 * 12 | `\xFFc<` | Green | 287C14
 *
 * \section reference_fonts Fonts
 *
 * The game's fonts, as `Text.font` takes them. Only 0-13 are valid: a larger
 * value makes the game read past its font table and can crash it. The names
 * are the game's own font names. The height is that of the text "Ag" drawn in
 * a 1.14d game, in pixels, without the glyph shadow.
 *
 * Value | Font | Height of "Ag"
 * ---|---|---
 * 0 | Font8 (the default) | 11
 * 1 | Font16 | 10
 * 2 | Font30 | 16
 * 3 | Font42 | 24
 * 4 | FontFormal10 | 12
 * 5 | FontFormal12 | 15
 * 6 | Font6 | 9
 * 7 | Font24 | 14
 * 8 | FontFormal11 | 12
 * 9 | FontExocet10 | 13
 * 10 | FontRidiculous | 9
 * 11 | FontExocet8 | 8
 * 12 | ReallyTheLastSucker | 9
 * 13 | FontInGameChat | 12
 *
 */
