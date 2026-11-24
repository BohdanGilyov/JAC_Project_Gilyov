# OBHAJOBA - Dungeon Escape (JAC)

## Kratke predstavenie

"Vytvoril som terminalovu tahovu RPG hru Dungeon Escape v jazyku C11. Hrac sa pohybuje po mape 8x8, bojuje s piatimi nepriatelmi, plni tri ulohy, ziskava XP a levely. Pridal som inventar s 15 predmetmi, 5 schopnosti, obchod s nakupom a predajom a rozvetveny dialog s piatimi koncami. Hru je mozne ulozit a nacitat."

## Kde ukazat kod pre kazdy prvok

1. **Quality of Life:** `input.c` - `read_key` na nacitanie znaku bez Enter; `ui.c` - `ui_clear`, `ui_present` na prekreslenie terminalu. Tieto dva subory boli pri rozsirovani zachovane bez uprav.
2. **Save/Load:** `save.c` - `save_game` zapisuje polia do suboru `save.txt`, `load_game` ich nacita a skontroluje. Podporuje format V2 i povodny V1.
3. **Mapa:** `map.c` - `dungeon[8][9]`, `map_tile`, `move_player`, `print_map`. Steny sa nepovolia prekrocit.
4. **Levely:** `game.c` - pole `level_xp` a `gain_xp`; XP od nepriatelov a uloh, automaticky level-up od 1 po 5.
5. **Boj:** `combat.c` - `start_combat`: HP, utok, obrana, utocny a obranny tah. `game.c` inicializuje 5 protivnikov.
6. **Ulohy:** `game.c` - `on_enemy_defeated`, `map.c` - navsteva svatyne a vychod. Tri rozne ulohy.
7. **Inventar:** `inventory.c` - `item_catalog` obsahuje 15 predmetov, `inventory_screen` spristupnuje tri kategorie, `item_equip_or_use` realne meni vybavu alebo HP. `game.h` ma pole `inventory[15]`.
8. **Schopnosti:** `skills.c` - `ability_unlocked` odomyka schopnosti podla levelu, `use_ability` vykonava pat odlisnych efektov, `battle_ability_menu` umoznuje vyber.
9. **Obchod:** `inventory.c` - `item_buy`, `item_sell`, `shop_screen`; cena sa odcita/pripocita a meni sa pocet predmetov v inventari.
10. **Dialog:** `dialogue.c` - `dialogue_screen` ma vetvenie cez 1/2/3 a druhe rozhodnutie; `dialogue_finish` realizuje pat roznych koncov a jednorazove odmeny.

## Navrhovane ukazky pri obhajobe

- **Mapa:** Nova hra (`N`), `D` k obchodnikovi. Pohyb do steny nie je mozny.
- **Obchod:** Na `M` stlac `O`, vyber `1` na nakup meca a `Q` na navrat.
- **Inventar:** Stlac `I`, `W`, `1` na nasadenie zbrane. V statistikach sveta uvidis vyssi utok.
- **Dialog:** Z `M` prejdi `D`, `S` k `N`, potom `T`. Volba `1` a nasledne `2` da 25 zlata. Opakovanim rovnakeho konca nedostanes dalsiu odmenu.
- **Schopnosti:** Stlac `K`, skontroluj 5 schopnosti a podmienky odomknutia. Pri boji stlac `K`, vyber cislo schopnosti.
- **Boj:** Dotkni sa `E`. `A` je bezny utok, `D` je obrana, `I` spotrebuje elixir a `K` vyberie schopnost.
- **Ulohy:** Na mape `U` ukaze tri ulohy. Postup sa aktualizuje po navsteve H, troch vitazstvach a po bossovi.
- **Save/Load:** `V` ulozi hru. V hlavnom menu `L` ju obnovi s peniazmi, inventarom, manou a dialogmi.

## Typicke otazky a odpovede

**Preco je inventar v poli?** Katalog ma pevny pocet 15 poloziek; `inventory[i]` je pocet kusov polozky `item_catalog[i]`.

**Preco pouzivas struktury?** `GameState` obsahuje vsetky udaje o jednej hre; pri ulozeni staci zapisat jeho vlastnosti v definovanom poradi.

**Ako funguje vyhodnotenie utoku?** Zakladne zranenie je `max(1, utok - obrana)`, pricom utok a obrana mozu byt zvysene vybavou.

**Ako odomykas schopnosti?** Schopnost s indexom `i` potrebuje level aspon `i + 1`, a pri pouziti sa odcita mana.

**Co ak predam nasadenu zbran?** Ak predam posledny kus, v `item_sell` sa nastavi index vybavenej zbrane na `-1`.

**Co ak sa dialog zopakuje?** `dialogue_seen` brani viacnasobnemu ziskaniu tej istej odmeny. Hrac moze dosiahnut ine konce.

**Preco nacitas aj V1?** Aby stare subory `save.txt` z predoslej verzie stale fungovali; nove udaje dostanu vychodiskove hodnoty.

**Ako zmenis cenu predmetu?** V `inventory.c` v poli `item_catalog` upravis cislo `price` u prislusneho predmetu.

**Ako pridat many pri level-upe?** V `game.c` vo funkcii `gain_xp` sa zvysuje `max_mana` o 5.

**Kde sa rozhoduje, ci hrac stoji v obchode?** V `main.c` sa pri klavese `O` kontroluje `map_tile` a porovnava sa so znakom `M`.

Projekt netvrdi, ze vsetky prvky musia byt uznane. Hodnotenie urcuje vyucujuci po obhajobe.
