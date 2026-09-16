mod platform;

use chrono::Utc;
use rusqlite::{params, Connection, OptionalExtension};
use serde::{Deserialize, Serialize};
use std::{
    collections::HashMap,
    fs,
    path::{Path, PathBuf},
    process::{Child, Command, Stdio},
    sync::Mutex,
};
use tauri::{AppHandle, Manager, State};
use uuid::Uuid;

struct AppState {
    db: Mutex<Connection>,
    processes: Mutex<HashMap<String, Child>>,
    data_dir: PathBuf,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct Agent {
    id: String,
    name: String,
    role: String,
    description: String,
    command: String,
    workspace: String,
    provider: String,
    model: String,
    status: String,
    pid: Option<u32>,
    created_at: String,
}

#[derive(Debug, Deserialize)]
#[serde(rename_all = "camelCase")]
struct AgentInput {
    name: String,
    role: String,
    description: String,
    command: String,
    workspace: String,
    provider: String,
    model: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct TaskItem {
    id: String,
    title: String,
    agent_id: Option<String>,
    agent_name: String,
    priority: String,
    status: String,
    created_at: String,
    started_at: Option<String>,
    completed_at: Option<String>,
    notes: String,
}

#[derive(Debug, Deserialize)]
#[serde(rename_all = "camelCase")]
struct TaskInput {
    title: String,
    agent_id: Option<String>,
    priority: String,
    notes: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct ActivityItem {
    id: i64,
    kind: String,
    actor: String,
    message: String,
    detail: String,
    created_at: String,
    level: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct PermissionSet {
    agent_id: String,
    files: String,
    terminal: String,
    browser: String,
    git: String,
    network: String,
}

#[derive(Debug, Clone, Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct MemoryEntry {
    id: i64,
    agent_id: String,
    key: String,
    value: String,
    updated_at: String,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct Metrics {
    agents_online: usize,
    agents_total: usize,
    tasks_running: usize,
    actions_today: usize,
    errors_today: usize,
}

#[derive(Debug, Serialize)]
#[serde(rename_all = "camelCase")]
struct CommandResult {
    status: i32,
    stdout: String,
    stderr: String,
}

#[derive(Serialize, Deserialize)]
#[serde(rename_all = "camelCase")]
struct ExportBundle {
    agents: Vec<Agent>,
    tasks: Vec<TaskItem>,
    permissions: Vec<PermissionSet>,
    memory: Vec<MemoryEntry>,
    exported_at: String,
}

fn now() -> String {
    Utc::now().to_rfc3339()
}

fn init_db(conn: &Connection) -> Result<(), String> {
    conn.execute_batch(
        r#"
        PRAGMA journal_mode=WAL;
        PRAGMA foreign_keys=ON;
        CREATE TABLE IF NOT EXISTS agents (
            id TEXT PRIMARY KEY,
            name TEXT NOT NULL,
            role TEXT NOT NULL DEFAULT '',
            description TEXT NOT NULL DEFAULT '',
            command TEXT NOT NULL DEFAULT '',
            workspace TEXT NOT NULL DEFAULT '',
            provider TEXT NOT NULL DEFAULT '',
            model TEXT NOT NULL DEFAULT '',
            status TEXT NOT NULL DEFAULT 'Stopped',
            pid INTEGER,
            created_at TEXT NOT NULL
        );
        CREATE TABLE IF NOT EXISTS tasks (
            id TEXT PRIMARY KEY,
            title TEXT NOT NULL,
            agent_id TEXT,
            priority TEXT NOT NULL DEFAULT 'Normal',
            status TEXT NOT NULL DEFAULT 'Queued',
            created_at TEXT NOT NULL,
            started_at TEXT,
            completed_at TEXT,
            notes TEXT NOT NULL DEFAULT ''
        );
        CREATE TABLE IF NOT EXISTS activity (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            kind TEXT NOT NULL,
            actor TEXT NOT NULL,
            message TEXT NOT NULL,
            detail TEXT NOT NULL DEFAULT '',
            created_at TEXT NOT NULL,
            level TEXT NOT NULL DEFAULT 'info'
        );
        CREATE TABLE IF NOT EXISTS permissions (
            agent_id TEXT PRIMARY KEY,
            files TEXT NOT NULL DEFAULT 'Ask',
            terminal TEXT NOT NULL DEFAULT 'Ask',
            browser TEXT NOT NULL DEFAULT 'Ask',
            git TEXT NOT NULL DEFAULT 'Ask',
            network TEXT NOT NULL DEFAULT 'Ask'
        );
        CREATE TABLE IF NOT EXISTS memory (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            agent_id TEXT NOT NULL,
            key TEXT NOT NULL,
            value TEXT NOT NULL,
            updated_at TEXT NOT NULL,
            UNIQUE(agent_id, key)
        );
        CREATE TABLE IF NOT EXISTS settings (
            key TEXT PRIMARY KEY,
            value TEXT NOT NULL
        );
        "#,
    )
    .map_err(|e| e.to_string())?;
    Ok(())
}

fn log_activity(
    conn: &Connection,
    kind: &str,
    actor: &str,
    message: &str,
    detail: &str,
    level: &str,
) -> Result<(), String> {
    conn.execute(
        "INSERT INTO activity(kind, actor, message, detail, created_at, level) VALUES (?1, ?2, ?3, ?4, ?5, ?6)",
        params![kind, actor, message, detail, now(), level],
    )
    .map_err(|e| e.to_string())?;
    Ok(())
}

fn row_to_agent(row: &rusqlite::Row<'_>) -> rusqlite::Result<Agent> {
    Ok(Agent {
        id: row.get(0)?,
        name: row.get(1)?,
        role: row.get(2)?,
        description: row.get(3)?,
        command: row.get(4)?,
        workspace: row.get(5)?,
        provider: row.get(6)?,
        model: row.get(7)?,
        status: row.get(8)?,
        pid: row.get::<_, Option<i64>>(9)?.map(|v| v as u32),
        created_at: row.get(10)?,
    })
}

fn get_agent(conn: &Connection, id: &str) -> Result<Agent, String> {
    conn.query_row(
        "SELECT id,name,role,description,command,workspace,provider,model,status,pid,created_at FROM agents WHERE id=?1",
        params![id],
        row_to_agent,
    )
    .optional()
    .map_err(|e| e.to_string())?
    .ok_or_else(|| "Agent not found".to_string())
}

fn permission_value(conn: &Connection, agent_id: &str, key: &str) -> Result<String, String> {
    let sql = match key {
        "files" => "SELECT files FROM permissions WHERE agent_id=?1",
        "terminal" => "SELECT terminal FROM permissions WHERE agent_id=?1",
        "browser" => "SELECT browser FROM permissions WHERE agent_id=?1",
        "git" => "SELECT git FROM permissions WHERE agent_id=?1",
        "network" => "SELECT network FROM permissions WHERE agent_id=?1",
        _ => return Err("Unknown permission".into()),
    };
    Ok(conn
        .query_row(sql, params![agent_id], |row| row.get(0))
        .optional()
        .map_err(|e| e.to_string())?
        .unwrap_or_else(|| "Ask".to_string()))
}

fn require_permission(conn: &Connection, agent_id: &str, key: &str, confirmed: bool) -> Result<(), String> {
    match permission_value(conn, agent_id, key)?.as_str() {
        "Allow" => Ok(()),
        "Deny" => Err(format!("PERMISSION_DENIED:{key}")),
        _ if confirmed => Ok(()),
        _ => Err(format!("PERMISSION_REQUIRED:{key}")),
    }
}

#[tauri::command]
fn list_agents(state: State<'_, AppState>) -> Result<Vec<Agent>, String> {
    let mut dead = Vec::new();
    {
        let mut processes = state.processes.lock().map_err(|_| "process lock poisoned")?;
        for (id, child) in processes.iter_mut() {
            if let Ok(Some(_)) = child.try_wait() {
                dead.push(id.clone());
            }
        }
        for id in &dead {
            processes.remove(id);
        }
    }

    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    for id in dead {
        let _ = conn.execute("UPDATE agents SET status='Stopped', pid=NULL WHERE id=?1", params![id]);
    }
    let mut stmt = conn
        .prepare("SELECT id,name,role,description,command,workspace,provider,model,status,pid,created_at FROM agents ORDER BY created_at ASC")
        .map_err(|e| e.to_string())?;
    let rows = stmt.query_map([], row_to_agent).map_err(|e| e.to_string())?;
    rows.collect::<Result<Vec<_>, _>>().map_err(|e| e.to_string())
}

#[tauri::command]
fn create_agent(input: AgentInput, state: State<'_, AppState>) -> Result<Agent, String> {
    if input.name.trim().is_empty() {
        return Err("Agent name is required".into());
    }
    let id = Uuid::new_v4().to_string();
    let created_at = now();
    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    conn.execute(
        "INSERT INTO agents(id,name,role,description,command,workspace,provider,model,status,created_at) VALUES (?1,?2,?3,?4,?5,?6,?7,?8,'Stopped',?9)",
        params![id, input.name.trim(), input.role, input.description, input.command, input.workspace, input.provider, input.model, created_at],
    ).map_err(|e| e.to_string())?;
    conn.execute("INSERT OR IGNORE INTO permissions(agent_id) VALUES (?1)", params![id])
        .map_err(|e| e.to_string())?;
    log_activity(&conn, "agent", input.name.trim(), "created agent", "", "info")?;
    get_agent(&conn, &id)
}

#[tauri::command]
fn update_agent(id: String, input: AgentInput, state: State<'_, AppState>) -> Result<Agent, String> {
    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    conn.execute(
        "UPDATE agents SET name=?2,role=?3,description=?4,command=?5,workspace=?6,provider=?7,model=?8 WHERE id=?1",
        params![id, input.name, input.role, input.description, input.command, input.workspace, input.provider, input.model],
    ).map_err(|e| e.to_string())?;
    let agent = get_agent(&conn, &id)?;
    log_activity(&conn, "agent", &agent.name, "updated agent", "Configuration changed", "info")?;
    Ok(agent)
}

#[tauri::command]
fn delete_agent(id: String, state: State<'_, AppState>) -> Result<(), String> {
    let _ = stop_agent(id.clone(), state.clone());
    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    let agent = get_agent(&conn, &id)?;
    conn.execute("UPDATE tasks SET agent_id=NULL WHERE agent_id=?1", params![id]).map_err(|e| e.to_string())?;
    conn.execute("DELETE FROM agents WHERE id=?1", params![id]).map_err(|e| e.to_string())?;
    conn.execute("DELETE FROM permissions WHERE agent_id=?1", params![id]).map_err(|e| e.to_string())?;
    conn.execute("DELETE FROM memory WHERE agent_id=?1", params![id]).map_err(|e| e.to_string())?;
    log_activity(&conn, "agent", &agent.name, "deleted agent", "", "warn")
}

#[cfg(target_os = "windows")]
fn shell_command(command: &str, workspace: &str) -> Command {
    use std::os::windows::process::CommandExt;
    const CREATE_NO_WINDOW: u32 = 0x08000000;
    let mut cmd = Command::new("cmd.exe");
    cmd.args(["/C", command]).creation_flags(CREATE_NO_WINDOW);
    if !workspace.trim().is_empty() && Path::new(workspace).exists() {
        cmd.current_dir(workspace);
    }
    cmd
}

#[cfg(not(target_os = "windows"))]
fn shell_command(command: &str, workspace: &str) -> Command {
    let mut cmd = Command::new("sh");
    cmd.args(["-lc", command]);
    if !workspace.trim().is_empty() && Path::new(workspace).exists() {
        cmd.current_dir(workspace);
    }
    cmd
}

#[tauri::command]
fn start_agent(id: String, confirmed: bool, state: State<'_, AppState>) -> Result<Agent, String> {
    {
        let processes = state.processes.lock().map_err(|_| "process lock poisoned")?;
        if processes.contains_key(&id) {
            let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
            return get_agent(&conn, &id);
        }
    }

    let agent = {
        let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
        let agent = get_agent(&conn, &id)?;
        require_permission(&conn, &id, "terminal", confirmed)?;
        agent
    };

    if agent.command.trim().is_empty() {
        return Err("This agent has no start command".into());
    }

    let child = shell_command(&agent.command, &agent.workspace)
        .stdin(Stdio::null())
        .stdout(Stdio::null())
        .stderr(Stdio::null())
        .spawn()
        .map_err(|e| format!("Failed to start {}: {e}", agent.name))?;
    let pid = child.id();
    state.processes.lock().map_err(|_| "process lock poisoned")?.insert(id.clone(), child);

    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    conn.execute("UPDATE agents SET status='Running', pid=?2 WHERE id=?1", params![id, pid as i64])
        .map_err(|e| e.to_string())?;
    log_activity(&conn, "process", &agent.name, "started agent", &format!("PID {pid}"), "success")?;
    get_agent(&conn, &id)
}

#[tauri::command]
fn stop_agent(id: String, state: State<'_, AppState>) -> Result<Agent, String> {
    let agent = {
        let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
        get_agent(&conn, &id)?
    };

    if let Some(mut child) = state.processes.lock().map_err(|_| "process lock poisoned")?.remove(&id) {
        let _ = child.kill();
        let _ = child.wait();
    }

    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    conn.execute("UPDATE agents SET status='Stopped', pid=NULL WHERE id=?1", params![id])
        .map_err(|e| e.to_string())?;
    log_activity(&conn, "process", &agent.name, "stopped agent", "", "warn")?;
    get_agent(&conn, &id)
}

#[tauri::command]
fn restart_agent(id: String, confirmed: bool, state: State<'_, AppState>) -> Result<Agent, String> {
    let _ = stop_agent(id.clone(), state.clone())?;
    start_agent(id, confirmed, state)
}

#[tauri::command]
fn run_agent_command(
    id: String,
    command: String,
    confirmed: bool,
    state: State<'_, AppState>,
) -> Result<CommandResult, String> {
    if command.trim().is_empty() {
        return Err("Command is empty".into());
    }

    let agent = {
        let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
        let agent = get_agent(&conn, &id)?;
        require_permission(&conn, &id, "terminal", confirmed)?;
        agent
    };

    let output = shell_command(&command, &agent.workspace)
        .output()
        .map_err(|e| format!("Failed to run command: {e}"))?;
    let stdout = String::from_utf8_lossy(&output.stdout).to_string();
    let stderr = String::from_utf8_lossy(&output.stderr).to_string();
    let status = output.status.code().unwrap_or(-1);

    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    log_activity(
        &conn,
        "terminal",
        &agent.name,
        "ran command",
        command.trim(),
        if status == 0 { "success" } else { "error" },
    )?;
    Ok(CommandResult { status, stdout, stderr })
}

#[tauri::command]
fn open_workspace(id: String, confirmed: bool, state: State<'_, AppState>) -> Result<(), String> {
    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    let agent = get_agent(&conn, &id)?;
    require_permission(&conn, &id, "files", confirmed)?;
    if agent.workspace.trim().is_empty() || !Path::new(&agent.workspace).exists() {
        return Err("Workspace does not exist".into());
    }
    #[cfg(target_os = "windows")]
    Command::new("explorer.exe").arg(&agent.workspace).spawn().map_err(|e| e.to_string())?;
    #[cfg(target_os = "macos")]
    Command::new("open").arg(&agent.workspace).spawn().map_err(|e| e.to_string())?;
    #[cfg(all(unix, not(target_os = "macos")))]
    Command::new("xdg-open").arg(&agent.workspace).spawn().map_err(|e| e.to_string())?;
    log_activity(&conn, "files", &agent.name, "opened workspace", &agent.workspace, "info")?;
    Ok(())
}

#[tauri::command]
fn attach_files(id: String, paths: Vec<String>, confirmed: bool, state: State<'_, AppState>) -> Result<Vec<String>, String> {
    if paths.is_empty() {
        return Ok(Vec::new());
    }
    let agent = {
        let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
        let agent = get_agent(&conn, &id)?;
        require_permission(&conn, &id, "files", confirmed)?;
        agent
    };
    if agent.workspace.trim().is_empty() || !Path::new(&agent.workspace).exists() {
        return Err("Workspace does not exist".into());
    }

    let inbox = Path::new(&agent.workspace).join(".nodus").join("inbox");
    fs::create_dir_all(&inbox).map_err(|e| e.to_string())?;
    let mut copied = Vec::new();
    for raw in paths {
        let source = PathBuf::from(&raw);
        if !source.is_file() {
            continue;
        }
        let Some(name) = source.file_name() else { continue; };
        let target = inbox.join(name);
        fs::copy(&source, &target).map_err(|e| format!("Failed to copy {}: {e}", source.display()))?;
        copied.push(target.to_string_lossy().to_string());
    }

    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    log_activity(&conn, "files", &agent.name, "attached files", &format!("{} file(s) copied to .nodus/inbox", copied.len()), "info")?;
    Ok(copied)
}

#[tauri::command]
fn list_tasks(state: State<'_, AppState>) -> Result<Vec<TaskItem>, String> {
    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    let mut stmt = conn.prepare(
        "SELECT t.id,t.title,t.agent_id,COALESCE(a.name,''),t.priority,t.status,t.created_at,t.started_at,t.completed_at,t.notes FROM tasks t LEFT JOIN agents a ON a.id=t.agent_id ORDER BY t.created_at DESC",
    ).map_err(|e| e.to_string())?;
    let rows = stmt.query_map([], |row| Ok(TaskItem {
        id: row.get(0)?, title: row.get(1)?, agent_id: row.get(2)?, agent_name: row.get(3)?,
        priority: row.get(4)?, status: row.get(5)?, created_at: row.get(6)?, started_at: row.get(7)?,
        completed_at: row.get(8)?, notes: row.get(9)?,
    })).map_err(|e| e.to_string())?;
    rows.collect::<Result<Vec<_>, _>>().map_err(|e| e.to_string())
}

#[tauri::command]
fn create_task(input: TaskInput, state: State<'_, AppState>) -> Result<TaskItem, String> {
    if input.title.trim().is_empty() { return Err("Task title is required".into()); }
    let id = Uuid::new_v4().to_string();
    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    conn.execute(
        "INSERT INTO tasks(id,title,agent_id,priority,status,created_at,notes) VALUES (?1,?2,?3,?4,'Queued',?5,?6)",
        params![id, input.title.trim(), input.agent_id, input.priority, now(), input.notes],
    ).map_err(|e| e.to_string())?;
    log_activity(&conn, "task", "Nodus", "created task", input.title.trim(), "info")?;
    drop(conn);
    list_tasks(state)?.into_iter().find(|t| t.id == id).ok_or_else(|| "Task not found after creation".into())
}

#[tauri::command]
fn set_task_status(id: String, status: String, state: State<'_, AppState>) -> Result<TaskItem, String> {
    let allowed = ["Queued", "Running", "Completed", "Failed"];
    if !allowed.contains(&status.as_str()) { return Err("Invalid task status".into()); }
    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    let title: String = conn.query_row("SELECT title FROM tasks WHERE id=?1", params![id], |row| row.get(0))
        .optional().map_err(|e| e.to_string())?.ok_or_else(|| "Task not found".to_string())?;
    let started = if status == "Running" { Some(now()) } else { None };
    let completed = if status == "Completed" || status == "Failed" { Some(now()) } else { None };
    conn.execute(
        "UPDATE tasks SET status=?2, started_at=COALESCE(started_at,?3), completed_at=?4 WHERE id=?1",
        params![id, status, started, completed],
    ).map_err(|e| e.to_string())?;
    log_activity(&conn, "task", "Nodus", &format!("task {status}"), &title, if status == "Failed" { "error" } else { "info" })?;
    drop(conn);
    list_tasks(state)?.into_iter().find(|t| t.id == id).ok_or_else(|| "Task not found".into())
}

#[tauri::command]
fn delete_task(id: String, state: State<'_, AppState>) -> Result<(), String> {
    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    let title: Option<String> = conn.query_row("SELECT title FROM tasks WHERE id=?1", params![id], |r| r.get(0)).optional().map_err(|e| e.to_string())?;
    conn.execute("DELETE FROM tasks WHERE id=?1", params![id]).map_err(|e| e.to_string())?;
    if let Some(title) = title { log_activity(&conn, "task", "Nodus", "deleted task", &title, "warn")?; }
    Ok(())
}

#[tauri::command]
fn list_activity(limit: Option<usize>, state: State<'_, AppState>) -> Result<Vec<ActivityItem>, String> {
    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    let limit = limit.unwrap_or(100).clamp(1, 500) as i64;
    let mut stmt = conn.prepare("SELECT id,kind,actor,message,detail,created_at,level FROM activity ORDER BY id DESC LIMIT ?1")
        .map_err(|e| e.to_string())?;
    let rows = stmt.query_map(params![limit], |row| Ok(ActivityItem {
        id: row.get(0)?, kind: row.get(1)?, actor: row.get(2)?, message: row.get(3)?, detail: row.get(4)?, created_at: row.get(5)?, level: row.get(6)?,
    })).map_err(|e| e.to_string())?;
    rows.collect::<Result<Vec<_>, _>>().map_err(|e| e.to_string())
}

#[tauri::command]
fn get_metrics(state: State<'_, AppState>) -> Result<Metrics, String> {
    let agents = list_agents(state.clone())?;
    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    let tasks_running: i64 = conn.query_row("SELECT COUNT(*) FROM tasks WHERE status='Running'", [], |r| r.get(0)).map_err(|e| e.to_string())?;
    let day = Utc::now().format("%Y-%m-%d").to_string();
    let actions_today: i64 = conn.query_row("SELECT COUNT(*) FROM activity WHERE substr(created_at,1,10)=?1", params![day], |r| r.get(0)).map_err(|e| e.to_string())?;
    let errors_today: i64 = conn.query_row("SELECT COUNT(*) FROM activity WHERE level='error' AND substr(created_at,1,10)=?1", params![day], |r| r.get(0)).map_err(|e| e.to_string())?;
    Ok(Metrics {
        agents_online: agents.iter().filter(|a| a.status == "Running").count(),
        agents_total: agents.len(),
        tasks_running: tasks_running as usize,
        actions_today: actions_today as usize,
        errors_today: errors_today as usize,
    })
}

#[tauri::command]
fn get_permissions(agent_id: String, state: State<'_, AppState>) -> Result<PermissionSet, String> {
    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    conn.execute("INSERT OR IGNORE INTO permissions(agent_id) VALUES (?1)", params![agent_id]).map_err(|e| e.to_string())?;
    conn.query_row(
        "SELECT agent_id,files,terminal,browser,git,network FROM permissions WHERE agent_id=?1",
        params![agent_id],
        |row| Ok(PermissionSet { agent_id: row.get(0)?, files: row.get(1)?, terminal: row.get(2)?, browser: row.get(3)?, git: row.get(4)?, network: row.get(5)? }),
    ).map_err(|e| e.to_string())
}

#[tauri::command]
fn set_permissions(value: PermissionSet, state: State<'_, AppState>) -> Result<PermissionSet, String> {
    for v in [&value.files, &value.terminal, &value.browser, &value.git, &value.network] {
        if !["Ask", "Allow", "Deny"].contains(&v.as_str()) { return Err("Permission values must be Ask, Allow or Deny".into()); }
    }
    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    conn.execute(
        "INSERT INTO permissions(agent_id,files,terminal,browser,git,network) VALUES (?1,?2,?3,?4,?5,?6) ON CONFLICT(agent_id) DO UPDATE SET files=excluded.files,terminal=excluded.terminal,browser=excluded.browser,git=excluded.git,network=excluded.network",
        params![value.agent_id, value.files, value.terminal, value.browser, value.git, value.network],
    ).map_err(|e| e.to_string())?;
    Ok(value)
}

#[tauri::command]
fn list_memory(agent_id: String, state: State<'_, AppState>) -> Result<Vec<MemoryEntry>, String> {
    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    let mut stmt = conn.prepare("SELECT id,agent_id,key,value,updated_at FROM memory WHERE agent_id=?1 ORDER BY key ASC").map_err(|e| e.to_string())?;
    let rows = stmt.query_map(params![agent_id], |row| Ok(MemoryEntry { id: row.get(0)?, agent_id: row.get(1)?, key: row.get(2)?, value: row.get(3)?, updated_at: row.get(4)? })).map_err(|e| e.to_string())?;
    rows.collect::<Result<Vec<_>, _>>().map_err(|e| e.to_string())
}

#[tauri::command]
fn set_memory(agent_id: String, key: String, value: String, state: State<'_, AppState>) -> Result<(), String> {
    if key.trim().is_empty() { return Err("Memory key is required".into()); }
    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    conn.execute(
        "INSERT INTO memory(agent_id,key,value,updated_at) VALUES (?1,?2,?3,?4) ON CONFLICT(agent_id,key) DO UPDATE SET value=excluded.value,updated_at=excluded.updated_at",
        params![agent_id, key.trim(), value, now()],
    ).map_err(|e| e.to_string())?;
    Ok(())
}

#[tauri::command]
fn delete_memory(id: i64, state: State<'_, AppState>) -> Result<(), String> {
    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    conn.execute("DELETE FROM memory WHERE id=?1", params![id]).map_err(|e| e.to_string())?;
    Ok(())
}

#[tauri::command]
fn export_config(state: State<'_, AppState>) -> Result<String, String> {
    let agents = list_agents(state.clone())?;
    let tasks = list_tasks(state.clone())?;
    let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
    let mut p_stmt = conn.prepare("SELECT agent_id,files,terminal,browser,git,network FROM permissions").map_err(|e| e.to_string())?;
    let permissions = p_stmt.query_map([], |row| Ok(PermissionSet { agent_id: row.get(0)?, files: row.get(1)?, terminal: row.get(2)?, browser: row.get(3)?, git: row.get(4)?, network: row.get(5)? })).map_err(|e| e.to_string())?.collect::<Result<Vec<_>, _>>().map_err(|e| e.to_string())?;
    let mut m_stmt = conn.prepare("SELECT id,agent_id,key,value,updated_at FROM memory").map_err(|e| e.to_string())?;
    let memory = m_stmt.query_map([], |row| Ok(MemoryEntry { id: row.get(0)?, agent_id: row.get(1)?, key: row.get(2)?, value: row.get(3)?, updated_at: row.get(4)? })).map_err(|e| e.to_string())?.collect::<Result<Vec<_>, _>>().map_err(|e| e.to_string())?;
    serde_json::to_string_pretty(&ExportBundle { agents, tasks, permissions, memory, exported_at: now() }).map_err(|e| e.to_string())
}

#[tauri::command]
fn backup_database(state: State<'_, AppState>) -> Result<String, String> {
    let backup_dir = state.data_dir.join("backups");
    fs::create_dir_all(&backup_dir).map_err(|e| e.to_string())?;
    {
        let conn = state.db.lock().map_err(|_| "database lock poisoned")?;
        conn.execute_batch("PRAGMA wal_checkpoint(FULL);").map_err(|e| e.to_string())?;
    }
    let target = backup_dir.join(format!("nodus-{}.db", Utc::now().format("%Y%m%d-%H%M%S")));
    fs::copy(state.data_dir.join("nodus.db"), &target).map_err(|e| e.to_string())?;
    Ok(target.to_string_lossy().to_string())
}

#[tauri::command]
fn app_data_path(state: State<'_, AppState>) -> String {
    state.data_dir.to_string_lossy().to_string()
}

fn setup_state(app: &AppHandle) -> Result<AppState, String> {
    let data_dir = app.path().app_data_dir().map_err(|e| e.to_string())?;
    fs::create_dir_all(&data_dir).map_err(|e| e.to_string())?;
    let conn = Connection::open(data_dir.join("nodus.db")).map_err(|e| e.to_string())?;
    init_db(&conn)?;
    Ok(AppState { db: Mutex::new(conn), processes: Mutex::new(HashMap::new()), data_dir })
}

#[cfg_attr(mobile, tauri::mobile_entry_point)]
pub fn run() {
    tauri::Builder::default()
        .setup(|app| {
            let state = setup_state(app.handle()).map_err(std::io::Error::other)?;
            app.manage(state);
            platform::setup_tray(app)?;
            Ok(())
        })
        .invoke_handler(tauri::generate_handler![
            list_agents,
            create_agent,
            update_agent,
            delete_agent,
            start_agent,
            stop_agent,
            restart_agent,
            run_agent_command,
            open_workspace,
            attach_files,
            list_tasks,
            create_task,
            set_task_status,
            delete_task,
            list_activity,
            get_metrics,
            get_permissions,
            set_permissions,
            list_memory,
            set_memory,
            delete_memory,
            export_config,
            backup_database,
            app_data_path,
            platform::hide_to_tray,
            platform::show_main_window,
            platform::is_autostart_enabled,
            platform::set_autostart,
        ])
        .run(tauri::generate_context!())
        .expect("error while running Nodus");
}
