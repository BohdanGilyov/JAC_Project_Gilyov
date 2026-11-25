# Dungeon Escape - JAC projekt (C11 / CLion)

Textova RPG hra v terminali. Hrac prechadza podzemim, bojuje s piatimi druhmi nepriatelov, ziskava skusenosti, plni tri ulohy a pokusa sa uniknut cez vychod X.

Projekt obsahuje **10 roznych prvkov** (Mapa, Levely, Boj, Ulohy, Save/Load, Quality of Life, Inventar, Schopnosti, Obchod, Dialog). Sceny sme nepouzili, lebo su alternativou mapy. O udelenych bodoch rozhoduje vyucujuci po obhajobe.

## Spustenie v CLion na Windows

1. Rozbal ZIP a v CLion otvor priecinok `JAC_Project_Gilyov` cez **File > Open**.
2. Nechaj CLion nacitat `CMakeLists.txt` (potrebny je dostupny C kompilator v toolchaine).
3. V hornej liste vyber ciel **DungeonEscape** (nie `Continuous` ani `DungeonEscapeTests`).
4. Stlac zeleny **Run**. Hra na Windows vzdy vytvori vlastnu konzolu s nazvom Dungeon Escape, nezavisle od konzoly CLion.
5. Ak Run nefunguje interaktivne, spusti `DungeonEscape.exe` dvojklikom v Prieskumnikovi (najdes ho v `cmake-build-debug`). Pri probleme so zostavenim vymaz `cmake-build-debug` a nacitaj CMake znova.

Ovlada sa **jednym stlacenim klavesu, bez Enter**. Klaves Escape sa nepouziva. Vstup zabezpecuje `input.c` (`_getch` na Windows, `termios` na Linuxe). Vykreslenie pouziva `printf`, `puts` a `putchar`. Pred kazdou obrazovkou `ui_clear` vymaze obrazovku a nastavi kurzor na poziciu (0, 0). Windows Terminal pouziva normalnu obrazovku: ANSI prikazy vymazu obrazovku aj historiu a vratia kurzor na zaciatok. Konzolovy handle ma prava na citanie aj zapis a zapnute spracovanie novych riadkov. Starsia konzola Windows bez ANSI pouziva WinAPI na vymazanie celeho buffera. Linux pouziva samostatnu ANSI obrazovku. Premenna prostredia `DUNGEON_STDIN_MODE` je urcena len na automatizovane testy vo Windows; pri beznom hrani ju nenastavuj.

## Ovladanie

| Kontext | Klaves | Akcia |
| --- | --- | --- |
| Hlavne menu | `N` / `L` / `H` / `Q` | Nova hra / Nacitat / Pomoc / Koniec |
| Mapa | `W A S D` | Pohyb po mape |
| Mapa | `I` | Inventar (vybava a elixiry) |
| Mapa | `K` | Zobrazit naucene schopnosti |
| Mapa | `O` | Obchod, ked hrac stoji na policku `M` |
| Mapa | `T` | Rozhovor, ked hrac stoji na policku `N` |
| Mapa | `P` / `U` / `V` / `H` / `Q` | Statistiky / Ulohy / Ulozit / Pomoc / Menu |
| Boj | `A` / `D` | Utok / Obrana |
| Boj | `K`, potom `1-5` | Vykonat schopnost (ak je odomknuta a je dost many) |
| Boj | `I` | Otvorit inventar, nasadit vybavu, pouzit elixir |
| Inventar | `W` / `A` / `P` | Zbrane / Brnenia / Elixiry |
| Inventar | `1-5` | Nasadit vybavu alebo pouzit elixir |
| Obchod | `W` / `A` / `P` | Zbrane / Brnenia / Elixiry |
| Obchod | `B` / `S` | Nakup / Predaj |
| Obchod | `1-5` | Kupit alebo predat vybrany predmet |
| Inventar / obchod | `Q` | Navrat na mapu |
| Dialog | `1`, `2`, `3` a dalsie volby | Vetveny rozhovor; `Q` navrat |

## Mapa (8x8)

```
# # # # # # # #
# @ M . # . . #
# . # N . . E #
# . . H # . . #
# # . . E . # #
# E . # . . E #
# . . . . B X #
# # # # # # # #
```

`@` hrac, `#` stena, `E` bezny nepriatel, `B` boss, `H` lieciva svatyna, `X` vychod, `M` obchodnik, `N` mudrc.

**Ako sa dostat k obchodnikovi:** z uvodnej pozicie stlac `D`, potom `O`.

**Ako sa dostat k mudrcovi:** z uvodnej pozicie `D`, `D`, `S`, potom `T`.

Na odomknutie vychodu treba navstivit svatynu, porazit aspon troch beznych nepriatelov a bossa.

## Dalsie 4 prvky

### Inventar (3 kategorie, 15 predmetov)

- 5 zbrani: Hrdzavy mec, Kratky mec, Ocelovy mec, Elfsky mec, Mec hrdinu.
- 5 brneni: Kozena vesta, Retazova kosela, Rytier brnenie, Ocelovy pancier, Dracia zbroj.
- 5 elixirov: Maly, Stredny, Velky, Kralovsky, Elixir zivota.

Zbrane pridavaju utok, brnenia pridavaju obranu, elixiry doplnaju zivoty. Po predaji posledneho kusa vybaveneho predmetu sa vybava automaticky zlozi. Hraci zacinaju s dvoma malymi elixirmi.

### Schopnosti (5 roznych efektov)

| Level | Schopnost | Efekt | Mana |
| --- | --- | --- | ---: |
| 1 | Silny uder | Vyssi fyzicky utok | 3 |
| 2 | Liecivy dotyk | +30 HP (max. do plneho HP) | 4 |
| 3 | Stitova stena | Zablokuje najblizsi utok nepriatela | 3 |
| 4 | Ohniva gula | 30 poskodenia nezavisle od obrany | 6 |
| 5 | Dvojity zasah | Dva fyzicke zasahy | 7 |

Mana zacina na 20 a kazdy level prida 5 max. many. Po vyhratom boji alebo pri svatyni sa doplni. Kazda schopnost sa odomkne pri danom leveli.

### Obchod

Na poli `M` otvor obchod (`O`). Kupovanie odcita zlato a pridava kus do inventara, predaj odobera predmet a dava polovicu povodnej ceny. Za porazenych nepriatelov dostanes aj zlato. Mozes si vybrat z 15 druhov predmetov.

### Dialog (5 moznych koncov)

Na poli `N` otvor rozhovor (`T`). Podla vyberu v dvoch urovniach dialogu mozes dosiahnut 5 roznych koncov:

1. Mudrc obnovi tvoje HP.
2. Mudrc daruje 25 zlata.
3. Mudrc prezradi tajomstvo (+20 XP).
4. Mudrc daruje kralovsky elixir.
5. Ked mudrca urazis, stratis 10 zlata.

Odmena/trest za kazdy koniec sa uplatni len raz. Rozhovor sa da opakovat.

## Save / Load

Na mape stlac `V`, ulozi sa subor `save.txt` do pracovneho priecinka CLion. Z hlavneho menu stlac `L`.

Format V2 uklada aj zlato, manu, inventar, vybavu a vysledky dialogov. Stary format V1 sa uz nenacitava.

## Kde je ktory prvok v kode

| Prvok | Subory / funkcie |
| --- | --- |
| Quality of Life | `input.c` (`read_key`, `terminal_prepare`), `ui.c` (`ui_clear`) |
| Save/Load | `save.c` (`save_game`, `load_game`) |
| Mapa | `map.c` (`map_tile`, `move_player`, `print_map`) |
| Levely | `game.c` (`gain_xp`) |
| Boj | `combat.c` (`start_combat`) |
| Ulohy | `game.c` a `map.c` (priznanie postupu po bojach a pri svatyni) |
| Inventar | `inventory.c` (`item_equip_or_use`, `inventory_screen`) |
| Schopnosti | `skills.c` (`use_ability`, `battle_ability_menu`) |
| Obchod | `inventory.c` (`item_buy`, `item_sell`, `shop_screen`) |
| Dialog | `dialogue.c` (`dialogue_screen`, `dialogue_finish`) |

## Testovanie

V CLion otvor CMake target `DungeonEscapeTests` alebo `DungeonEscapeFeatureTests`, pripadne spusti v CMake build priecinku `ctest --output-on-failure`.

- `tests/test_game.c`: povodne mechaniky a ulozenie.
- `tests/test_features.c`: 15 predmetov, obchod, vybava, 5 schopnosti, 5 koncov, ulozenie V2.

Hlavny ciel na spustanie hry je vzdy `DungeonEscape`.

## Kompilacia mimo CLion

```sh
gcc -std=c11 -Wall -Wextra main.c game.c map.c combat.c save.c input.c ui.c inventory.c skills.c dialogue.c -o DungeonEscape
```
