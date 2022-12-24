# Pokémon FireRed and LeafGreen (With UPR-Speedchoice Support)

[![Build Status][travis-badge]][travis]

[travis]: https://travis-ci.org/pret/pokefirered
[travis-badge]: https://travis-ci.org/pret/pokefirered.svg?branch=master

This is a decompilation of Pokémon FireRed and LeafGreen.

## To set up UPR-Speedchoice support:
1. Build the repo like normal
2. Install capstone with: `sudo apt install libcapstone-dev`
3. Navigate to the tools/inigen folder and run `make inigen`
4. Run `./inigen ../../pokefirered.elf gen3_offsets.ini --code MBDN --name "Fire Red (U) (1.0)"` to build the .ini file every time you make a change (this can be automated in the Makefile if you know what you're doing)
5. Open the randomizer.jar using WinRAR or another program capable of editing .jar files.
6. Navigate to com/dabomstew/pkrandom/config and delete gen3_offsets.ini, and "Add/Import" the gen3_offsets.ini file generated in the tools/inigen folder from step 4

------

It builds the following ROMs:

* [**pokefirered.gba**](https://datomatic.no-intro.org/?page=show_record&s=23&n=1616) `sha1: 41cb23d8dccc8ebd7c649cd8fbb58eeace6e2fdc`
* [**pokeleafgreen.gba**](https://datomatic.no-intro.org/?page=show_record&s=23&n=1617) `sha1: 574fa542ffebb14be69902d1d36f1ec0a4afd71e`
* [**pokefirered_rev1.gba**](https://datomatic.no-intro.org/?page=show_record&s=23&n=1672) `sha1: dd5945db9b930750cb39d00c84da8571feebf417`
* [**pokeleafgreen_rev1.gba**](https://datomatic.no-intro.org/index.php?page=show_record&s=23&n=1668) `sha1: 7862c67bdecbe21d1d69ce082ce34327e1c6ed5e`

To set up the repository, see [INSTALL.md](INSTALL.md).


## See also

Other disassembly and/or decompilation projects:
* [**Pokémon Red and Blue**](https://github.com/pret/pokered)
* [**Pokémon Gold and Silver (Space World '97 demo)**](https://github.com/pret/pokegold-spaceworld)
* [**Pokémon Yellow**](https://github.com/pret/pokeyellow)
* [**Pokémon Trading Card Game**](https://github.com/pret/poketcg)
* [**Pokémon Pinball**](https://github.com/pret/pokepinball)
* [**Pokémon Stadium**](https://github.com/pret/pokestadium)
* [**Pokémon Gold and Silver**](https://github.com/pret/pokegold)
* [**Pokémon Crystal**](https://github.com/pret/pokecrystal)
* [**Pokémon Ruby and Sapphire**](https://github.com/pret/pokeruby)
* [**Pokémon Pinball: Ruby & Sapphire**](https://github.com/pret/pokepinballrs)
* [**Pokémon Emerald**](https://github.com/pret/pokeemerald)
* [**Pokémon Mystery Dungeon: Red Rescue Team**](https://github.com/pret/pmd-red)


## Contacts

You can find us on [Discord](https://discord.gg/d5dubZ3) and [IRC](https://web.libera.chat/?#pret).
