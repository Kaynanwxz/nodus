use std::process::Command;
use tauri::{menu::{Menu, MenuItem}, tray::TrayIconBuilder, App, AppHandle, Manager};

pub fn setup_tray(app: &mut App) -> Result<(), Box<dyn std::error::Error>> {
    let show = MenuItem::with_id(app, "show", "Open Nodus", true, None::<&str>)?;
    let hide = MenuItem::with_id(app, "hide", "Hide to tray", true, None::<&str>)?;
    let quit = MenuItem::with_id(app, "quit", "Quit", true, None::<&str>)?;
    let menu = Menu::with_items(app, &[&show, &hide, &quit])?;

    TrayIconBuilder::new()
        .tooltip("Nodus")
        .menu(&menu)
        .show_menu_on_left_click(false)
        .on_menu_event(|app, event| match event.id.as_ref() {
            "show" => {
                if let Some(window) = app.get_webview_window("main") {
                    let _ = window.show();
                    let _ = window.unminimize();
                    let _ = window.set_focus();
                }
            }
            "hide" => {
                if let Some(window) = app.get_webview_window("main") {
                    let _ = window.hide();
                }
            }
            "quit" => app.exit(0),
            _ => {}
        })
        .build(app)?;
    Ok(())
}

#[tauri::command]
pub fn hide_to_tray(app: AppHandle) -> Result<(), String> {
    let window = app.get_webview_window("main").ok_or_else(|| "Main window not found".to_string())?;
    window.hide().map_err(|e| e.to_string())
}

#[tauri::command]
pub fn show_main_window(app: AppHandle) -> Result<(), String> {
    let window = app.get_webview_window("main").ok_or_else(|| "Main window not found".to_string())?;
    window.show().map_err(|e| e.to_string())?;
    let _ = window.unminimize();
    window.set_focus().map_err(|e| e.to_string())
}

#[cfg(target_os = "windows")]
fn run_reg(args: &[&str]) -> Result<std::process::Output, String> {
    use std::os::windows::process::CommandExt;
    const CREATE_NO_WINDOW: u32 = 0x08000000;
    Command::new("reg.exe")
        .args(args)
        .creation_flags(CREATE_NO_WINDOW)
        .output()
        .map_err(|e| e.to_string())
}

#[tauri::command]
pub fn is_autostart_enabled() -> Result<bool, String> {
    #[cfg(target_os = "windows")]
    {
        let output = run_reg(&[
            "query",
            r"HKCU\Software\Microsoft\Windows\CurrentVersion\Run",
            "/v",
            "Nodus",
        ])?;
        return Ok(output.status.success());
    }
    #[cfg(not(target_os = "windows"))]
    Ok(false)
}

#[tauri::command]
pub fn set_autostart(enabled: bool) -> Result<bool, String> {
    #[cfg(target_os = "windows")]
    {
        let key = r"HKCU\Software\Microsoft\Windows\CurrentVersion\Run";
        let output = if enabled {
            let exe = std::env::current_exe().map_err(|e| e.to_string())?;
            let value = format!("\"{}\"", exe.to_string_lossy());
            run_reg(&["add", key, "/v", "Nodus", "/t", "REG_SZ", "/d", &value, "/f"])?
        } else {
            run_reg(&["delete", key, "/v", "Nodus", "/f"])?
        };
        if !output.status.success() {
            return Err(String::from_utf8_lossy(&output.stderr).to_string());
        }
        return Ok(enabled);
    }
    #[cfg(not(target_os = "windows"))]
    Err("Autostart is currently implemented for Windows only".into())
}
