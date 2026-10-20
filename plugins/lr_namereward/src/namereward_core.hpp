#pragma once
// Engine-free core of the lr_namereward port: the configuration model of namereward.ini and the
// body of the original's OnPlayerKilledPreHook (the original's lr_namereward.cpp).
//
// The original kept two globals (fMulti, pszName) filled in Load() and multiplied the experience
// of a kill whenever the attacker's name contains "name" - it read the attacker's "name" convar
// and ran a substring search on it.
//
// The port keeps exactly that (the convar read becomes pisexcompat's GetPlayerName, the same
// "name" convar), with the model split out so it can be unit-tested without an engine.

#include <string>
#include <string_view>

namespace pisex {
class KeyValues;
}

namespace lr_namereward {

struct Config {
	// "multi" of namereward.ini. The original's GetFloat returns 0.0 for a missing key, which
	// zeroes every reward - the port keeps the engine's GetFloat semantics (see ParseConfig).
	float multi = 0.0f;
	// "name": the (case-sensitive) substring looked for in the attacker's name.
	std::string name;
};

// Reads the config from the "LR" node of namereward.ini, exactly like Load(): the "multi" and
// "name" keys fill the two fields.
void ParseConfig(const pisex::KeyValues& kv, Config& out);

// The OnPlayerKilledPreHook() body: returns the (possibly scaled) experience caused.
// int(exp * multi) truncates towards zero, like the original's C-style cast.
int ApplyMultiplier(const Config& cfg, int exp_caused, std::string_view player_name);

} // namespace lr_namereward
