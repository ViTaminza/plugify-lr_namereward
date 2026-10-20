#include "namereward_core.hpp"

#include <pisex/keyvalues.hpp>

namespace lr_namereward {

void ParseConfig(const pisex::KeyValues& kv, Config& out) {
	// The original read the "multi" key into fMulti and the "name" key into pszName.
	out.multi = kv.GetFloat("multi", 0.0f);
	out.name = std::string(kv.GetString("name", ""));
}

int ApplyMultiplier(const Config& cfg, int exp_caused, std::string_view player_name) {
	// strstr(name, cfg.name) of the original, kept for the same edge cases: an empty pattern
	// matches everything (strstr(x, "") == x), a name without the substring never matches.
	// The search runs on the view directly - a std::string copy here was one allocation per kill.
	if (player_name.find(cfg.name) == std::string_view::npos) {
		return exp_caused;
	}
	return static_cast<int>(static_cast<float>(exp_caused) * cfg.multi);
}

} // namespace lr_namereward
