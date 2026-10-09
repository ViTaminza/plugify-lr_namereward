// lr_namereward — Plugify C++ port of the Pisex LR module "[LR] Name Reward"
// (ref/src/cs2-lvl_ranks_modules/LR_NameReward/lr_namereward.cpp, 116 lines + lr_namereward.h).
//
// What it does (identical to the original): reads configs/levels_ranks/namereward.ini
// ("multi" and "name"), and when a kill's experience is computed, multiplies it whenever the
// attacker's name contains "name".
//
// Port notes (the rest is parity):
//   * engine->GetClientConVarValue(slot, "name") is pisex::g_pPlayers->GetPlayerName(slot) - the
//     same "name" convar - and the hook is registered through the LR core's OnCoreIsReady, as
//     before.
//   * The original registered HookOnPlayerKilledPre from OnCoreIsReadyHook and never checked
//     CoreIsLoaded() first; the port keeps the sibling modules' "if CoreIsLoaded() run now else
//     hook" shape so a plugin (re)loaded after the core finished still initialises.
//   * The original loaded the config in Load() (plugin load) and kept running with the globals
//     the failed load left behind; the port logs and keeps the defaults, never fails the plugin
//     (a Failed OnPluginStart stops *every* Plugify plugin).
//   * lr_namereward.cpp declared `IVIPApi* g_pVIPCore` and included include/vip.h, but never used
//     the VIP API (the production binary is the same file); the port therefore declares no vip
//     dependency.

#include <pisex/pisex.hpp>

#include <plg/plugin.hpp>

#include <lr_api.hpp>

#include <chrono>
#include <exception>
#include <string>

#include "namereward_core.hpp"

namespace {

// GetLogTag() of the original.
constexpr char kLogTag[] = "[LR-NAME-REWARD]";

// The original's g_pLRCore, moved to the consumer wrapper (the PluginId argument is gone).
levels_ranks::LrApi* g_pLRCore = &levels_ranks::LrApi::Get();

// The original's fMulti / pszName.
lr_namereward::Config g_cfg;

// False when namereward.ini could not be read. The original's Load() returned false then, so the
// module was not loaded and never rescaled anything; here the hook stays registered and must do
// nothing (an empty pattern matches every name, and multi would be 0 - every kill worth 0 exp).
bool g_configLoaded = false;

void LoadConfig() {
	// Load() of the original: KeyValues("LR") -> addons/configs/levels_ranks/namereward.ini.
	pisex::KeyValues kv("LR");
	if (!kv.LoadFromFile("addons/configs/levels_ranks/namereward.ini")) {
		pisex::ErrorLog("%s Failed to load addons/configs/levels_ranks/namereward.ini", kLogTag);
		return;
	}
	g_configLoaded = true;
	lr_namereward::ParseConfig(kv, g_cfg);
}

// OnPlayerKilledPreHook() of the original.
void OnPlayerKilledPreHook(pisex::IGameEvent* hEvent, int& iExpCaused, int /*iExpVictim*/, int /*iExpAttacker*/) {
	if (hEvent == nullptr || !g_configLoaded) {
		return;
	}
	const int iClient = hEvent->GetInt("attacker");
	if (iClient < 0 || iClient >= 64) {
		return; // the original fed the raw event value to the engine; the port guards the slot
	}
	iExpCaused = lr_namereward::ApplyMultiplier(g_cfg, iExpCaused, pisex::g_pPlayers->GetPlayerName(iClient));
}

// OnCoreIsReadyHook() of the original.
void OnCoreIsReadyHook() {
	g_pLRCore->HookOnPlayerKilledPre(OnPlayerKilledPreHook);
}

class LrNameRewardPlugin final : public plg::Plugin {
public:
	plg::PluginResult OnPluginStart() override;
	plg::PluginResult OnPluginUpdate(std::chrono::milliseconds dt) override;
	plg::PluginResult OnPluginEnd() override;
};

LrNameRewardPlugin g_plugin;

plg::PluginResult LrNameRewardPlugin::OnPluginStart() {
	try {
		// <configs>/levels_ranks (+ translations): the shared LR directory the core packages.
		pisex::Init("levels_ranks");
		LoadConfig();

		// AllPluginsLoaded() of the original: the core chain, then OnCoreIsReady.
		if (g_pLRCore->CoreIsLoaded()) {
			OnCoreIsReadyHook();
		} else {
			g_pLRCore->HookOnCoreIsReady(OnCoreIsReadyHook);
		}
		return plg::PluginResult{};
	} catch (const std::exception& ex) {
		pisex::ErrorLog("%s OnPluginStart failed: %s", kLogTag, ex.what());
	} catch (...) {
		pisex::ErrorLog("%s OnPluginStart failed with an unknown exception", kLogTag);
	}
	return plg::PluginResult{}; // never Failed: that stops every plugify plugin
}

plg::PluginResult LrNameRewardPlugin::OnPluginUpdate(std::chrono::milliseconds /*dt*/) {
	try {
		pisex::Poll();
	} catch (const std::exception& ex) {
		pisex::ErrorLog("%s OnPluginUpdate failed: %s", kLogTag, ex.what());
	} catch (...) {
		pisex::ErrorLog("%s OnPluginUpdate failed with an unknown exception", kLogTag);
	}
	return plg::PluginResult{};
}

plg::PluginResult LrNameRewardPlugin::OnPluginEnd() {
	try {
		// The Unhook* are no-ops when levels_ranks is not loaded (lr_api.hpp Unregister checks the
		// null binding slot); the Available() guard only keeps that intent visible here.
		if (g_pLRCore->Available()) {
			g_pLRCore->UnhookOnPlayerKilledPre();
			g_pLRCore->UnhookOnCoreIsReady();
		}
		pisex::Shutdown();
	} catch (...) {
		// nothing left to do at unload
	}
	return plg::PluginResult{};
}

} // namespace

PLUGIFY_PLUGIN(PLUGIN_API, &g_plugin)
