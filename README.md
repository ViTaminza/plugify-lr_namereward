# lr_namereward (Plugify)

[LR] Name Reward: multiplies the experience of a kill when the attacker's name contains the configured substring (configs/levels_ranks/namereward.ini: 'multi' and 'name'). Ported from the Pisex Levels Ranks module LR_NameReward. Requires the levels_ranks core; no exported methods.

Plugify port (fork) of **LR_NameReward ([LR] Name Reward)** by Pisex (Metamod (C++)).
Original: https://github.com/Pisex/cs2-lvl_ranks_modules
The code was rewritten for Plugify / s2sdk; behaviour follows the original.

## Requirements
- [Plugify](https://github.com/untrustedmodders/plugify) with [s2sdk](https://github.com/untrustedmodders/plugify-plugin-s2sdk)
- Plugify dependencies: s2sdk, levels_ranks, hudmenu (optional)

## Installation
Download `lr_namereward-1.0.1.zip` from Releases and extract it into `game/csgo/`.

## Building
Requires Docker. `./build.sh` builds in the Steam Runtime 3 (sniper) SDK image; the package
is written to `build/package/lr_namereward`.

`plugins/levels_ranks` carry only the API headers of the plugins this one uses.

## License
GPL-3.0, see LICENSE.

