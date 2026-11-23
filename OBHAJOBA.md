# Dungeon Escape - poznamky k obhajobe

## Zakladna architektura

`main.c` obsahuje hlavne menu a hernu slucku `play`.
`game.h` obsahuje tri struktury: `Player`, `Enemy`, `GameState`.
Herny stav sa predava funkciam cez ukazovatel `GameState *`, nevyuzivame globalnu premennu pre stav hry.

## Kde ukazat 5 prvkov

1. **Mapa:** `map.c`, pole `dungeon` (8 x 8), `map_tile` kontroluje hranice,
   `print_map` vykresluje mapu a `move_player` kontroluje steny, nepriatelov a vychod.
2. **Boj:** `combat.c`, `start_combat` je cyklus tahov, `damage` zabezpecuje minimalne
   1 poskodenie. V `game.c` je definovanych 5 roznych nepriatelov.
3. **Levely:** `game.c`, `gain_xp` pridava XP a cyklus `while` zvysuje level
   (aj ked hrac naraz ziska dost XP na viac levelov). Maximalny level je 5.
4. **Ulohy:** `game.c` (`show_quests`, `on_enemy_defeated`) a `map.c` (`move_player`).
   Prva sa splni navstevou svatyne, druha po 3 vyhrach, tretia po bossovi a vychode.
5. **Save/Load:** `save.c`, `save_game` zapisuje postup do `save.txt` pomocou `fprintf`;
   `load_game` cita pomocou `fscanf`, zacina inicializovanym stavom a overuje nacitane hodnoty.

## Typicke otazky

**Preco je mapa `static const`?** Mapa je pevne dana, pocas hry sa nemeni.
Nepriatelia su ulozeni osobitne v `GameState` a vykresluju sa cez mapu.

**Preco `GameState *`?** Funkcie mozu menit povodny stav hry bez jeho kopirovania.
`const GameState *` pouzivame tam, kde sa stav iba cita.

**Ako funguje naraz viac level-upov?** V `gain_xp` je `while`, ktory opakuje
povysenie, kym XP postacuju a nebol dosiahnuty level 5.

**Preco je obrana vyhodna?** Pri volbe D hrac neudiera, ale polovici prichadzajuce
poskodenie; je to jednoducha taktika v tahovom boji.

**Co ak save neexistuje alebo je chybny?** `load_game` vrati 0 a hra
vypise chybu. Pri uspesnom nacitani sa overi pozicia, statistiky, ulohy aj nepriatelia.

## Rychla demonstracia

- Spustite `N` a ukazte mapu 8x8 a pohyb `W/A/S/D`.
- Z chodby sa dostante k svatyni: `D`, `D`, `S`, `S` (quest 1, level 2).
- Ukazte `P`, `U` a prikaz `V` na ulozenie.
- Vratte sa `Q` do menu, zvolte `L` a ukazte obnovenie pozicie a XP.
- Prejdite k nepriatelovi, v boji pouzite `A` a ukazte statistiky nepriatela.
- Porazte troch beznych nepriatelov a ukazte splnenie druhej ulohy.
- Porazte bossa, navstivte `X`, ukazte koniec hry.

Pred odovzdanim projekt prejdite a precvicte si jeho upravy podla pokynov vyucujuceho.

## Spustenie v CLion

Projekt je CMake aplikacia. CLion nacita `CMakeLists.txt` a ciel `DungeonEscape`
spusti priamo cez zelene tlacidlo Run. Na Windows je `.exe` interny vystup
kompilacie, netreba ho vytvarat ani spustat rucne.
