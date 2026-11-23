# Dungeon Escape — projekt JAC (C11 / CLion)

Jednoducha terminalova RPG hra v jazyku C bez externych kniznic.
Ciel: preskumat podzemie, splnit tri ulohy, porazit bossa a uniknut.

## Spustenie v CLion (odporucany sposob)

1. Rozbalte ZIP a v CLion zvolte **File > Open**.
2. Vyberte priecinok **JAC_Project_Gilyov** (obsahuje `CMakeLists.txt`).
3. CLion nacita CMake projekt. Ak sa zobrazi okno s konfiguraciou, potvrďte ju.
4. Skontrolujte, ci mate v **Settings > Build, Execution, Deployment > Toolchains**
   nastaveny funkcny C kompilator (napr. MinGW alebo MSVC na Windows).
5. Vpravo hore vyberte ciel **DungeonEscape** a stlacte zelene **Run**
   (alebo `Shift+F10`). Hra sa spusti v konzole CLion.
6. V konzole zadavajte prikazy a po kazdom stlacte **Enter**.

**Nie je potrebne rucne vytvarat ani spustat `.exe` subor.**
Na Windows CLion/CMake automaticky zostavi spustitelny `.exe` v priecinku
`cmake-build-debug` alebo inom zostavovacom priecinku. ZIP obsahuje iba
zdrojove subory, nie hotovy program.

Ak ste mali projekt uz otvoreny bez CMake, skopirujte `CMakeLists.txt`
do jeho korenoveho priecinka a zvolte **Reload CMake Project**;
pripadne priecinok znova otvorte cez **File > Open**.

## Ovladanie

- Hlavne menu: `N` - nova hra, `L` - nacitat ulozenu hru, `Q` - koniec.
- Mapa: `W/A/S/D` - pohyb, `P` - statistiky, `U` - ulohy,
  `V` - ulozit do suboru, `H` - pomoc, `Q` - navrat do menu.
- Boj: `A` - utok, `D` - obrana.
- Po kazdom prikaze stlacte **Enter**. Velke aj male pismena su podporovane.

Legenda mapy: `@` hrac, `#` stena, `.` volne miesto, `E` nepriatel,
`B` boss, `H` lieciva svatyna, `h` uz pouzita svatyna, `X` vychod.
Mapa ma rozmery 8 x 8.

## Herny ciel

1. Najdite svatynu (`H`) a ziskajte 20 XP. Svatyna doplni HP.
2. Porazte aspon troch beznych nepriatelov (Rat, Skeleton, Goblin, Orc).
   Za tretiu vyhru ziskate dalsich 35 XP.
3. Porazte bossa (`B`) a vstupte na vychod (`X`). Na unik treba
   splnit aj prve dve ulohy.

Hra obsahuje 5 roznych nepriatelov vratane bossa a 5 urovni postavy.
Pri level-up sa zvysi maximum HP (+10), utok (+2) a obrana (+1)
a HP sa doplni na maximum. Prahy XP pre urovne 1 az 5: 0, 20, 50, 90, 140.

Boj je tahovy, bez nahody. Poskodenie = max(1, utok - obrana).
Pri obrane sa dalsi prijaty utok znizi na polovicu (zaokruhlenie nahor).

## Ulozenie a nacitanie (Save / Load)

Prikaz `V` ulozi hru do `save.txt` v aktualnom **pracovnom priecinku** procesu.
Ulozena hra sa nacita cez `L` v hlavnom menu. CLion moze mat pracovny
priecinok nastaveny na `cmake-build-debug`, preto sa `save.txt` nemusi
objavit priamo vedla `main.c`. Pracovny priecinok mozno zmenit cez
**Run > Edit Configurations > DungeonEscape > Working directory**.
Pri opakovanom ulozeni sa ten isty subor prepise.

## Pozadovane prvky — 5 x 10 bodov

| Prvok | Implementacia | Kde v kode |
|---|---|---|
| Mapa (min. 5 x 5) | 8 x 8, pohyb a kolizie | `map.c`: `map_tile`, `print_map`, `move_player` |
| Boj (min. 5 nepriatelov) | 5 nepriatelov vratane bossa, HP, utok, obrana | `game.c`: `init_game`; `combat.c`: `start_combat` |
| Levely (min. 5) | XP a urovne 1 az 5 | `game.c`: `gain_xp` |
| Ulohy (min. 3) | Svatyna, 3 nepriatelia, boss a vychod | `game.c`, `map.c` |
| Save / Load | Zapis a nacitanie stavu zo suboru | `save.c`: `save_game`, `load_game` |

Podrobnosti zadania: https://ics.science.upjs.sk/jac/poziadavky-na-projekt/

## Struktura projektu

```text
JAC_Project_Gilyov/
  CMakeLists.txt  - zostavenie a spustenie cez CLion / CMake
  main.c          - hlavne menu a herna slucka
  game.h          - konstanty, struktury a deklaracie funkcii
  game.c          - hrac, nepriatelia, statistiky, XP, ulohy
  map.c           - mapa, pohyb, udalosti na polickach
  combat.c        - tahovy boj
  save.c          - ukladanie a nacitanie, validacia suboru
  OBHAJOBA.md     - poznamky na obhajobu
  README.md       - navod a dokumentacia
  .gitignore      - subory, ktore sa nemaju posielat na Git
```

## Alternativna kompilacia cez CMake (bez CLion)

```sh
cmake -S . -B build
cmake --build build
```

Samostatna kompilacia vytvori spustitelny subor (na Windows `.exe`),
pretoze kazdy standardny C program sa musi pre beh skompilovat.
V CLion ho spusta priamo zelene tlacidlo Run.

## Odovzdanie

Zdrojove subory nahrajte do vlastneho Git repozitara (bez priecinkov
`cmake-build-*`, `build` a bez `save.txt`). Pred obhajobou si pripravte
ukazku mapy, boja, levelovania, uloh aj nacitania ulozenej hry.
