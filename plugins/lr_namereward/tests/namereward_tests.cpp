// Tests for plugins/lr_namereward/src/namereward_core.hpp (engine-free core of the ported
// "[LR] Name Reward" module).
//
//   * namereward.ini parsing against the shipped file, driven exactly like the plugin
//     does it (pisex::SetPluginName("levels_ranks") + the Metamod path spelling),
//   * the multiplier body: the strstr() semantics of the original (substring match, empty
//     pattern matches everything, truncating int cast).

#include <plg/plugin.hpp>

#include <pisex/keyvalues.hpp>
#include <pisex/paths.hpp>

#include <cstdio>
#include <string>
#include <string_view>

#include "namereward_core.hpp"

// A real plugin gets these from PLUGIFY_PLUGIN; the test leaves them null (also the state the code
// has to survive: no plugify dirs, no logger). The path resolver is driven through
// pisex::SetPluginName/SetConfigDir instead.
namespace plg {
GetBaseDirFn GetBaseDir{nullptr};
GetExtensionsDirFn GetExtensionsDir{nullptr};
GetConfigsDirFn GetConfigsDir{nullptr};
GetDataDirFn GetDataDir{nullptr};
GetLogsDirFn GetLogsDir{nullptr};
GetCacheDirFn GetCacheDir{nullptr};
IsLoadedFn IsLoaded{nullptr};
LogFn Log{nullptr};
BeginZoneFn BeginZone{nullptr};
EndZoneFn EndZone{nullptr};
} // namespace plg

namespace {

int g_checks = 0;
int g_failures = 0;
std::string_view g_section = "?";

void Section(std::string_view name) {
	g_section = name;
	std::printf("-- %.*s\n", static_cast<int>(name.size()), name.data());
}

#define CHECK(expr)                                                                            \
	do {                                                                                       \
		++g_checks;                                                                            \
		if (!(expr)) {                                                                         \
			++g_failures;                                                                      \
			std::printf("FAIL [%.*s] %s:%d: %s\n", static_cast<int>(g_section.size()),         \
						g_section.data(), __FILE__, __LINE__, #expr);                          \
		}                                                                                      \
	} while (false)

#define CHECK_EQ_INT(actual, expected)                                                         \
	do {                                                                                       \
		++g_checks;                                                                            \
		const long long a_ = static_cast<long long>(actual);                                   \
		const long long e_ = static_cast<long long>(expected);                                 \
		if (a_ != e_) {                                                                        \
			++g_failures;                                                                      \
			std::printf("FAIL [%.*s] %s:%d: got %lld, expected %lld\n",                        \
						static_cast<int>(g_section.size()), g_section.data(), __FILE__,       \
						__LINE__, a_, e_);                                                     \
		}                                                                                      \
	} while (false)

// --- the shipped config ------------------------------------------------------------------------

void TestRealConfig() {
	Section("real namereward.ini");
	pisex::SetPluginName("levels_ranks");
	pisex::SetConfigDir(LR_NAMEREWARD_TEST_DATA_DIR);

	pisex::KeyValues kv("LR");
	CHECK(kv.LoadFromFile("addons/configs/levels_ranks/namereward.ini"));

	lr_namereward::Config cfg;
	lr_namereward::ParseConfig(kv, cfg);
	CHECK_EQ_INT(cfg.multi * 10.0f, 15); // "1.5"
	CHECK(cfg.name == "example.com");
}

// --- the multiplier -----------------------------------------------------------------------------

void TestMultiplier() {
	Section("ApplyMultiplier");
	lr_namereward::Config cfg;
	cfg.multi = 1.5f;
	cfg.name = "example.com";

	// The shipped config's case: a name containing the tag is multiplied, int() truncates (10*1.5=15,
	// 7*1.5=10.5 -> 10).
	CHECK_EQ_INT(lr_namereward::ApplyMultiplier(cfg, 10, "Player example.com | Nick"), 15);
	CHECK_EQ_INT(lr_namereward::ApplyMultiplier(cfg, 7, "[example.com] abc"), 10);
	// A name without the substring is untouched.
	CHECK_EQ_INT(lr_namereward::ApplyMultiplier(cfg, 10, "other player"), 10);
	// strstr() semantics: an empty name never matches a non-empty pattern.
	CHECK_EQ_INT(lr_namereward::ApplyMultiplier(cfg, 10, ""), 10);
	// ... and an empty pattern matches every name (strstr(x, "") == x), multiplying everything.
	lr_namereward::Config empty_pattern = cfg;
	empty_pattern.name.clear();
	CHECK_EQ_INT(lr_namereward::ApplyMultiplier(empty_pattern, 10, "anyone"), 15);
	// GetFloat's default for a missing "multi" is 0.0: every reward then goes to zero, exactly
	// like the original's uninitialised-global path.
	lr_namereward::Config zero;
	zero.name = "x";
	CHECK_EQ_INT(lr_namereward::ApplyMultiplier(zero, 10, "x"), 0);
}

// --- a config without the "name" key ------------------------------------------------------------

void TestMissingKeys() {
	Section("minimal config");
	pisex::KeyValues kv("LR");
	CHECK(kv.LoadFromBuffer("\"LR\"\n{\n\t\"multi\" \"2\"\n}\n"));
	lr_namereward::Config cfg;
	lr_namereward::ParseConfig(kv, cfg);
	CHECK_EQ_INT(cfg.multi, 2);
	CHECK(cfg.name.empty());
}

} // namespace

int main() {
	std::printf("lr_namereward_tests\n");
	TestRealConfig();
	TestMultiplier();
	TestMissingKeys();
	std::printf("%d checks, %d failures\n", g_checks, g_failures);
	return g_failures == 0 ? 0 : 1;
}
