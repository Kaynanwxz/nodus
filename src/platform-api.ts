import { invoke } from "@tauri-apps/api/core";
import { nodus } from "./nodus";

export const platformApi = {
  async attachFiles(agentId: string, paths: string[], confirmed = false): Promise<string[]> {
    if (!nodus.isDesktop) return paths;
    return invoke<string[]>("attach_files", { id: agentId, paths, confirmed });
  },

  async isAutostartEnabled(): Promise<boolean> {
    if (!nodus.isDesktop) return localStorage.getItem("nodus-autostart-preview") === "1";
    return invoke<boolean>("is_autostart_enabled");
  },

  async setAutostart(enabled: boolean): Promise<boolean> {
    if (!nodus.isDesktop) {
      localStorage.setItem("nodus-autostart-preview", enabled ? "1" : "0");
      return enabled;
    }
    return invoke<boolean>("set_autostart", { enabled });
  },

  async hideToTray(): Promise<void> {
    if (!nodus.isDesktop) return;
    return invoke<void>("hide_to_tray");
  },

  async showMainWindow(): Promise<void> {
    if (!nodus.isDesktop) return;
    return invoke<void>("show_main_window");
  },
};
