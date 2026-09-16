import { useCallback, useEffect, useMemo, useRef, useState, type FormEvent, type ReactNode } from "react";
import { getCurrentWindow } from "@tauri-apps/api/window";
import {
  nodus,
  withPermissionRetry,
  type ActivityItem,
  type Agent,
  type AgentInput,
  type MemoryEntry,
  type Metrics,
  type PermissionSet,
  type PermissionValue,
  type TaskItem,
  type TaskStatus,
} from "./nodus";

type Tab = "Overview" | "Agents" | "Terminal" | "Tasks" | "Activity" | "Settings";
type AgentSection = "Terminal" | "Logs" | "Tasks" | "Memory" | "Settings";

const tabs: Tab[] = ["Overview", "Agents", "Terminal", "Tasks", "Activity", "Settings"];
const agentSections: AgentSection[] = ["Terminal", "Logs", "Tasks", "Memory", "Settings"];
const avatarByName: Record<string, string> = {
  Clio: "/assets/clio.webp",
  Daedalus: "/assets/daedalus.webp",
  Scribe: "/assets/scribe.webp",
  Athena: "/assets/athena.webp",
};

const emptyMetrics: Metrics = { agentsOnline: 0, agentsTotal: 0, tasksRunning: 0, actionsToday: 0, errorsToday: 0 };
const blankAgent: AgentInput = { name: "", role: "", description: "", command: "", workspace: "", provider: "", model: "" };

function avatarFor(agent?: Agent | null) {
  return agent ? avatarByName[agent.name] ?? "/assets/profile.webp" : "/assets/profile.webp";
}

function formatWhen(value?: string | null) {
  if (!value) return "—";
  const date = new Date(value);
  const diff = Date.now() - date.getTime();
  if (diff < 60_000) return "now";
  if (diff < 3_600_000) return `${Math.floor(diff / 60_000)}m ago`;
  if (diff < 86_400_000) return `${Math.floor(diff / 3_600_000)}h ago`;
  return date.toLocaleDateString();
}

function notify(title: string, body: string) {
  if (!("Notification" in window)) return;
  if (Notification.permission === "granted") new Notification(title, { body });
  else if (Notification.permission === "default") void Notification.requestPermission();
}

function WindowButtons() {
  if (!nodus.isDesktop) return null;
  const appWindow = getCurrentWindow();
  return (
    <div className="window-buttons" data-tauri-drag-region="false">
      <button aria-label="Minimize" onClick={() => appWindow.minimize()}>−</button>
      <button aria-label="Maximize" onClick={() => appWindow.toggleMaximize()}>□</button>
      <button aria-label="Close" className="close" onClick={() => appWindow.close()}>×</button>
    </div>
  );
}

function Header({ active, onTab, onPalette, search, setSearch }: {
  active: Tab;
  onTab: (tab: Tab) => void;
  onPalette: () => void;
  search: string;
  setSearch: (value: string) => void;
}) {
  const [clock, setClock] = useState(new Date());
  useEffect(() => {
    const id = window.setInterval(() => setClock(new Date()), 1000);
    return () => window.clearInterval(id);
  }, []);

  return (
    <header className="topbar" data-tauri-drag-region>
      <button className="brand brand-button" data-tauri-drag-region onClick={() => onTab("Overview")}>
        <div className="brand-title">NODUS</div>
        <div className="brand-subtitle">Ideas · agents · actions</div>
      </button>
      <nav className="main-nav" aria-label="Primary">
        {tabs.map((tab) => (
          <button key={tab} className={active === tab ? "nav-item active" : "nav-item"} onClick={() => onTab(tab)}>{tab}</button>
        ))}
      </nav>
      <div className="header-tools">
        <div className="search-box">
          <span className="search-icon">⌕</span>
          <input aria-label="Search" value={search} onChange={(e) => setSearch(e.target.value)} placeholder="Search Nodus" />
          <button className="kbd-hint" onClick={onPalette} title="Command palette">⌘K</button>
        </div>
        <button className="pulse-medallion" aria-label="Command palette" onClick={onPalette}>✦</button>
        <div className="clock"><strong>{clock.toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" })}</strong><span>{clock.toLocaleDateString([], { month: "short", day: "2-digit" })}</span></div>
        <div className="profile"><img src="/assets/profile.webp" alt="Profile" /></div>
        <WindowButtons />
      </div>
    </header>
  );
}

function MetricCard({ title, value, note, good = false, onClick }: { title: string; value: string; note: string; good?: boolean; onClick?: () => void }) {
  return (
    <button className="metric-card metric-button" onClick={onClick}>
      <div className="metric-title">{title}</div>
      <div className="metric-value">{value}</div>
      <div className={good ? "metric-note good" : "metric-note"}>{note}</div>
    </button>
  );
}

function AgentCard({ agent, onOpen }: { agent: Agent; onOpen: (agent: Agent) => void }) {
  const running = agent.status === "Running";
  return (
    <article className="agent-card clickable-card" onClick={() => onOpen(agent)} tabIndex={0} onKeyDown={(e) => e.key === "Enter" && onOpen(agent)}>
      <div className="agent-topline">
        <div className="avatar-wrap"><img src={avatarFor(agent)} alt="" /><span className={`presence ${running ? "active" : "idle"}`} /></div>
        <span className={`status-pill ${running ? "active" : "idle"}`}>{agent.status}</span>
      </div>
      <h3>{agent.name}</h3>
      <p className="agent-role">{agent.role || "Unassigned role"}</p>
      <p className="agent-description">{agent.description || "Configure this agent to give it a purpose inside Nodus."}</p>
      <div className="agent-footer"><span className="task-count">{agent.pid ? `PID ${agent.pid}` : agent.provider || "Local"}</span><span className="agent-arrow">↗</span></div>
    </article>
  );
}

function StatusBadge({ status }: { status: string }) {
  return <span className={`task-state ${status.toLowerCase()}`}>{status}</span>;
}

function TaskRows({ tasks, agents, onStatus, onDelete, compact = false }: {
  tasks: TaskItem[];
  agents: Agent[];
  onStatus: (task: TaskItem, status: TaskStatus) => void;
  onDelete: (task: TaskItem) => void;
  compact?: boolean;
}) {
  if (!tasks.length) return <EmptyState title="No tasks yet" copy="Create a task and assign it to an agent when you are ready." />;
  return (
    <div className={compact ? "task-table compact" : "task-table"}>
      {tasks.map((task) => {
        const agent = agents.find((a) => a.id === task.agentId);
        return (
          <div className="task-row functional-task-row" key={task.id}>
            <button className="task-check" aria-label="Toggle task" onClick={() => onStatus(task, task.status === "Completed" ? "Queued" : "Completed")}>{task.status === "Completed" ? "✓" : ""}</button>
            <span className="task-name" title={task.title}>{task.title}</span>
            <span className="task-agent">{agent && <img src={avatarFor(agent)} alt="" />}{task.agentName || "Unassigned"}</span>
            <button className="status-button" onClick={() => onStatus(task, task.status === "Running" ? "Completed" : "Running")}><StatusBadge status={task.status} /></button>
            <span className="task-time">{formatWhen(task.startedAt || task.createdAt)}</span>
            <button className="task-menu" aria-label={`Delete ${task.title}`} onClick={() => onDelete(task)}>×</button>
          </div>
        );
      })}
    </div>
  );
}

function ActivityList({ activity, limit }: { activity: ActivityItem[]; limit?: number }) {
  const items = typeof limit === "number" ? activity.slice(0, limit) : activity;
  if (!items.length) return <EmptyState title="Quiet for now" copy="Agent actions, commands and task changes will appear here." />;
  return (
    <div className="activity-list">
      {items.map((item) => (
        <div className="activity-item" key={item.id}>
          <span className={`activity-marker ${item.level === "error" ? "error" : item.level === "success" ? "good" : "paper"}`} />
          <div className="activity-copy"><p><strong>{item.actor}</strong> {item.message}</p><em>{item.detail || item.kind}</em></div>
          <time>{formatWhen(item.createdAt)}</time>
        </div>
      ))}
    </div>
  );
}

function EmptyState({ title, copy }: { title: string; copy: string }) {
  return <div className="empty-state"><strong>{title}</strong><span>{copy}</span></div>;
}

function Overview({ agents, tasks, activity, metrics, onOpenAgent, onNewTask, onNavigate, onTaskStatus, onDeleteTask }: {
  agents: Agent[];
  tasks: TaskItem[];
  activity: ActivityItem[];
  metrics: Metrics;
  onOpenAgent: (agent: Agent) => void;
  onNewTask: () => void;
  onNavigate: (tab: Tab) => void;
  onTaskStatus: (task: TaskItem, status: TaskStatus) => void;
  onDeleteTask: (task: TaskItem) => void;
}) {
  return (
    <main className="overview-stage">
      <img className="left-rail" src="/assets/left-rail.webp" alt="" aria-hidden="true" />
      <img className="right-rail" src="/assets/right-rail.webp" alt="" aria-hidden="true" />
      <div className="center-canvas">
        <section className="hero-art" aria-label="Nodus visual manifesto"><img src="/assets/hero.webp" alt="" aria-hidden="true" /></section>
        <section className="metrics-grid" aria-label="System overview">
          <MetricCard title="Agents online" value={`${metrics.agentsOnline} / ${metrics.agentsTotal}`} note="Live local processes" good={metrics.agentsOnline > 0} onClick={() => onNavigate("Agents")} />
          <MetricCard title="Tasks running" value={`${metrics.tasksRunning}`} note="Tracked in local SQLite" onClick={() => onNavigate("Tasks")} />
          <MetricCard title="Actions today" value={`${metrics.actionsToday}`} note="Commands, tasks and lifecycle" onClick={() => onNavigate("Activity")} />
          <MetricCard title="Errors today" value={`${metrics.errorsToday}`} note={metrics.errorsToday ? "Needs attention" : "No recorded errors"} good={!metrics.errorsToday} onClick={() => onNavigate("Activity")} />
        </section>
        <div className="dashboard-grid">
          <div className="dashboard-main">
            <section className="panel agents-panel">
              <div className="panel-head"><div><div className="panel-title">Agents</div><div className="panel-subtitle">Local processes you can start, stop and inspect</div></div><button className="view-all" onClick={() => onNavigate("Agents")}>View all ↗</button></div>
              <div className="agents-grid">{agents.length ? agents.slice(0, 4).map((agent) => <AgentCard agent={agent} onOpen={onOpenAgent} key={agent.id} />) : <EmptyState title="No agents" copy="Create your first local agent to begin." />}</div>
            </section>
            <section className="panel tasks-panel">
              <div className="panel-head task-head"><div><div className="panel-title">Tasks</div><div className="panel-subtitle">Persistent work across your agents</div></div><div className="panel-actions"><button className="soft-button" onClick={onNewTask}>＋ New</button><button className="view-all" onClick={() => onNavigate("Tasks")}>View all ↗</button></div></div>
              <TaskRows tasks={tasks.slice(0, 5)} agents={agents} onStatus={onTaskStatus} onDelete={onDeleteTask} compact />
            </section>
          </div>
          <aside className="panel activity-panel"><div className="panel-head"><div><div className="panel-title">Activity</div><div className="panel-subtitle">Real events recorded by Nodus</div></div><button className="view-all" onClick={() => onNavigate("Activity")}>View all ↗</button></div><ActivityList activity={activity} limit={8} /></aside>
        </div>
      </div>
      <img className="bottom-art" src="/assets/bottom-center.webp" alt="" aria-hidden="true" />
    </main>
  );
}

function PageShell({ eyebrow, title, subtitle, actions, children }: { eyebrow: string; title: string; subtitle: string; actions?: ReactNode; children: ReactNode }) {
  return <main className="secondary-page"><div className="page-heading-row"><div className="page-heading"><span>{eyebrow}</span><h1>{title}</h1><p>{subtitle}</p></div>{actions && <div className="page-actions">{actions}</div>}</div>{children}</main>;
}

function AgentsPage({ agents, onOpen, onNew }: { agents: Agent[]; onOpen: (agent: Agent) => void; onNew: () => void }) {
  return <PageShell eyebrow="Agent registry" title="Agents" subtitle="Every intelligence connected to Nodus, with its real process state." actions={<button className="primary-button" onClick={onNew}>＋ New agent</button>}><div className="page-agent-grid">{agents.map((agent) => <AgentCard key={agent.id} agent={agent} onOpen={onOpen} />)}{!agents.length && <EmptyState title="Start with one agent" copy="Register a CLI, script, local model or any command-line process." />}</div></PageShell>;
}

function TasksPage({ tasks, agents, onNew, onStatus, onDelete }: { tasks: TaskItem[]; agents: Agent[]; onNew: () => void; onStatus: (task: TaskItem, status: TaskStatus) => void; onDelete: (task: TaskItem) => void }) {
  const [filter, setFilter] = useState<string>("All");
  const filtered = filter === "All" ? tasks : tasks.filter((task) => task.status === filter);
  return <PageShell eyebrow="Operations" title="Tasks" subtitle="Persistent work that survives restarts of Nodus." actions={<button className="primary-button" onClick={onNew}>＋ New task</button>}><section className="panel tasks-panel page-tasks"><div className="panel-head"><div className="task-filters expanded">{["All", "Running", "Queued", "Completed", "Failed"].map((item) => <button key={item} className={filter === item ? "selected" : ""} onClick={() => setFilter(item)}>{item}</button>)}</div></div><TaskRows tasks={filtered} agents={agents} onStatus={onStatus} onDelete={onDelete} /></section></PageShell>;
}

function ActivityPage({ activity }: { activity: ActivityItem[] }) {
  const [query, setQuery] = useState("");
  const filtered = activity.filter((item) => `${item.actor} ${item.message} ${item.detail} ${item.kind}`.toLowerCase().includes(query.toLowerCase()));
  return <PageShell eyebrow="Archive" title="Activity" subtitle="A local audit trail of agent lifecycle, commands, tasks and errors." actions={<input className="inline-search" value={query} onChange={(e) => setQuery(e.target.value)} placeholder="Filter activity" />}><section className="panel wide-activity"><ActivityList activity={filtered} /></section></PageShell>;
}

function GlobalTerminal({ agents, onOpen }: { agents: Agent[]; onOpen: (agent: Agent, section?: AgentSection) => void }) {
  return <PageShell eyebrow="Local shell" title="Terminal" subtitle="Choose an agent to open a command session in its configured workspace."><div className="terminal-launch-grid">{agents.map((agent) => <button className="terminal-launch" key={agent.id} onClick={() => onOpen(agent, "Terminal")}><img src={avatarFor(agent)} alt="" /><span><strong>{agent.name}</strong><small>{agent.workspace || "No workspace configured"}</small></span><i>↗</i></button>)}{!agents.length && <EmptyState title="No terminal targets" copy="Create an agent first." />}</div></PageShell>;
}

function SettingsPage({ dataPath, onBackup, onExport }: { dataPath: string; onBackup: () => void; onExport: () => void }) {
  return <PageShell eyebrow="Control plane" title="Settings" subtitle="Local-first controls, backups and system information."><div className="settings-grid"><section className="settings-card"><h3>Storage</h3><SettingRow label="Database" value="SQLite / local" /><SettingRow label="App data" value={dataPath || "Loading…"} /><SettingRow label="Runtime" value={nodus.isDesktop ? "Tauri desktop" : "Browser preview"} /><div className="settings-actions"><button className="soft-button" onClick={onBackup}>Create backup</button><button className="soft-button" onClick={onExport}>Export JSON</button></div></section><section className="settings-card"><h3>Experience</h3><SettingRow label="Command palette" value="Ctrl + K" /><SettingRow label="Quick tabs" value="Ctrl + 1…6" /><SettingRow label="Persistence" value="Enabled" /><p className="settings-note">Process control, tasks, permissions, memory and activity are stored locally. No cloud account is required.</p></section></div></PageShell>;
}

function SettingRow({ label, value }: { label: string; value: string }) {
  return <div className="setting-row"><span>{label}</span><strong title={value}>{value}</strong></div>;
}

function AgentDetail({ agent, tasks, activity, onClose, onChanged }: { agent: Agent; tasks: TaskItem[]; activity: ActivityItem[]; onClose: () => void; onChanged: () => Promise<void> }) {
  const [section, setSection] = useState<AgentSection>("Terminal");
  const [busy, setBusy] = useState(false);
  const [error, setError] = useState("");
  const [terminalInput, setTerminalInput] = useState("");
  const [terminalLines, setTerminalLines] = useState<string[]>([`Nodus terminal · ${agent.name}`, `Workspace: ${agent.workspace || "not configured"}`]);
  const [permissions, setPermissionsState] = useState<PermissionSet | null>(null);
  const [memory, setMemoryState] = useState<MemoryEntry[]>([]);
  const [memoryKey, setMemoryKey] = useState("");
  const [memoryValue, setMemoryValue] = useState("");
  const terminalEnd = useRef<HTMLDivElement | null>(null);

  const loadAgentExtras = useCallback(async () => {
    const [p, m] = await Promise.all([nodus.getPermissions(agent.id), nodus.listMemory(agent.id)]);
    setPermissionsState(p); setMemoryState(m);
  }, [agent.id]);

  useEffect(() => { void loadAgentExtras(); }, [loadAgentExtras]);
  useEffect(() => { terminalEnd.current?.scrollIntoView({ behavior: "smooth" }); }, [terminalLines]);

  const doAction = async (action: "start" | "stop" | "restart" | "workspace") => {
    setBusy(true); setError("");
    try {
      if (action === "start") await withPermissionRetry(() => nodus.startAgent(agent.id), () => nodus.startAgent(agent.id, true));
      if (action === "stop") await nodus.stopAgent(agent.id);
      if (action === "restart") await withPermissionRetry(() => nodus.restartAgent(agent.id), () => nodus.restartAgent(agent.id, true));
      if (action === "workspace") await withPermissionRetry(() => nodus.openWorkspace(agent.id), () => nodus.openWorkspace(agent.id, true));
      await onChanged();
    } catch (e) { setError(String(e)); }
    finally { setBusy(false); }
  };

  const runTerminal = async (event: FormEvent) => {
    event.preventDefault();
    const command = terminalInput.trim();
    if (!command) return;
    setTerminalInput("");
    setTerminalLines((lines) => [...lines, `> ${command}`]);
    try {
      const result = await withPermissionRetry(() => nodus.runCommand(agent.id, command), () => nodus.runCommand(agent.id, command, true));
      const output = [result.stdout.trim(), result.stderr.trim()].filter(Boolean).join("\n");
      setTerminalLines((lines) => [...lines, output || `[exit ${result.status}]`]);
      await onChanged();
    } catch (e) { setTerminalLines((lines) => [...lines, `ERROR: ${String(e)}`]); }
  };

  const savePermission = async (key: keyof Omit<PermissionSet, "agentId">, value: PermissionValue) => {
    if (!permissions) return;
    const next = { ...permissions, [key]: value };
    setPermissionsState(next);
    await nodus.setPermissions(next);
  };

  const addMemory = async (event: FormEvent) => {
    event.preventDefault();
    if (!memoryKey.trim()) return;
    await nodus.setMemory(agent.id, memoryKey, memoryValue);
    setMemoryKey(""); setMemoryValue("");
    await loadAgentExtras();
  };

  const agentTasks = tasks.filter((task) => task.agentId === agent.id);
  const agentActivity = activity.filter((item) => item.actor === agent.name);

  return (
    <div className="detail-overlay" onMouseDown={(e) => e.target === e.currentTarget && onClose()}>
      <section className="agent-detail">
        <header className="agent-detail-head"><button className="back-button" onClick={onClose}>←</button><img src={avatarFor(agent)} alt="" /><div><span className={`status-pill ${agent.status === "Running" ? "active" : "idle"}`}>{agent.status}</span><h2>{agent.name}</h2><p>{agent.role || "Unassigned role"}</p></div><div className="agent-control-strip"><button disabled={busy || agent.status === "Running"} onClick={() => doAction("start")}>▶ Start</button><button disabled={busy || agent.status !== "Running"} onClick={() => doAction("stop")}>■ Stop</button><button disabled={busy} onClick={() => doAction("restart")}>↻ Restart</button><button disabled={busy} onClick={() => doAction("workspace")}>⌂ Workspace</button></div></header>
        {error && <div className="error-banner">{error}</div>}
        <nav className="detail-tabs">{agentSections.map((item) => <button key={item} className={section === item ? "active" : ""} onClick={() => setSection(item)}>{item}</button>)}</nav>
        <div className="agent-detail-body">
          {section === "Terminal" && <div className="live-terminal"><div className="terminal-screen">{terminalLines.map((line, i) => <pre key={i}>{line}</pre>)}<div ref={terminalEnd} /></div><form className="terminal-input-row" onSubmit={runTerminal}><span>›</span><input autoFocus value={terminalInput} onChange={(e) => setTerminalInput(e.target.value)} placeholder="Run a command in this agent workspace" /><button>Run</button></form></div>}
          {section === "Logs" && <div className="detail-content"><ActivityList activity={agentActivity} /></div>}
          {section === "Tasks" && <div className="detail-content">{agentTasks.length ? agentTasks.map((task) => <div className="detail-task" key={task.id}><div><strong>{task.title}</strong><span>{task.priority} · {formatWhen(task.createdAt)}</span></div><StatusBadge status={task.status} /></div>) : <EmptyState title="No assigned tasks" copy="Create a task and assign it to this agent." />}</div>}
          {section === "Memory" && <div className="detail-content memory-layout"><form className="memory-form" onSubmit={addMemory}><input value={memoryKey} onChange={(e) => setMemoryKey(e.target.value)} placeholder="Key, e.g. project" /><textarea value={memoryValue} onChange={(e) => setMemoryValue(e.target.value)} placeholder="Value or note" /><button className="primary-button">Save memory</button></form><div className="memory-list">{memory.map((entry) => <article key={entry.id}><div><strong>{entry.key}</strong><small>{formatWhen(entry.updatedAt)}</small></div><p>{entry.value}</p><button onClick={async () => { await nodus.deleteMemory(entry.id); await loadAgentExtras(); }}>Delete</button></article>)}{!memory.length && <EmptyState title="No memory entries" copy="Store stable project facts or agent-specific notes here." />}</div></div>}
          {section === "Settings" && <div className="detail-content permissions-layout"><section><h3>Agent configuration</h3><SettingRow label="Command" value={agent.command || "Not configured"} /><SettingRow label="Workspace" value={agent.workspace || "Not configured"} /><SettingRow label="Provider" value={agent.provider || "Local / unspecified"} /><SettingRow label="Model" value={agent.model || "Unspecified"} /></section><section><h3>Permissions</h3>{permissions && (["files", "terminal", "browser", "git", "network"] as const).map((key) => <div className="permission-row" key={key}><span>{key}</span><div>{(["Ask", "Allow", "Deny"] as PermissionValue[]).map((value) => <button className={permissions[key] === value ? "active" : ""} key={value} onClick={() => savePermission(key, value)}>{value}</button>)}</div></div>)}</section></div>}
        </div>
      </section>
    </div>
  );
}

function AgentModal({ onClose, onSave }: { onClose: () => void; onSave: (input: AgentInput) => Promise<void> }) {
  const [form, setForm] = useState(blankAgent);
  const [busy, setBusy] = useState(false);
  const set = (key: keyof AgentInput, value: string) => setForm((current) => ({ ...current, [key]: value }));
  const submit = async (e: FormEvent) => { e.preventDefault(); if (!form.name.trim()) return; setBusy(true); try { await onSave(form); onClose(); } finally { setBusy(false); } };
  return <Modal title="New agent" subtitle="Connect any local CLI, script or long-running process." onClose={onClose}><form className="form-grid" onSubmit={submit}><label>Name<input autoFocus value={form.name} onChange={(e) => set("name", e.target.value)} placeholder="Hermes" /></label><label>Role<input value={form.role} onChange={(e) => set("role", e.target.value)} placeholder="Development agent" /></label><label className="span-2">Description<input value={form.description} onChange={(e) => set("description", e.target.value)} placeholder="What this agent is responsible for" /></label><label className="span-2">Start command<input value={form.command} onChange={(e) => set("command", e.target.value)} placeholder="hermes --resume" /></label><label className="span-2">Workspace<input value={form.workspace} onChange={(e) => set("workspace", e.target.value)} placeholder="D:\\Agents\\Hermes" /></label><label>Provider<input value={form.provider} onChange={(e) => set("provider", e.target.value)} placeholder="Local / OpenRouter / ..." /></label><label>Model<input value={form.model} onChange={(e) => set("model", e.target.value)} placeholder="Optional" /></label><div className="modal-actions span-2"><button type="button" className="soft-button" onClick={onClose}>Cancel</button><button className="primary-button" disabled={busy}>{busy ? "Saving…" : "Create agent"}</button></div></form></Modal>;
}

function TaskModal({ agents, onClose, onSave }: { agents: Agent[]; onClose: () => void; onSave: (input: { title: string; agentId?: string | null; priority: string; notes: string }) => Promise<void> }) {
  const [title, setTitle] = useState(""); const [agentId, setAgentId] = useState(""); const [priority, setPriority] = useState("Normal"); const [notes, setNotes] = useState("");
  const submit = async (e: FormEvent) => { e.preventDefault(); if (!title.trim()) return; await onSave({ title, agentId: agentId || null, priority, notes }); onClose(); };
  return <Modal title="New task" subtitle="Create persistent work and assign it to an agent." onClose={onClose}><form className="form-grid" onSubmit={submit}><label className="span-2">Task<input autoFocus value={title} onChange={(e) => setTitle(e.target.value)} placeholder="Build the first playable prototype" /></label><label>Agent<select value={agentId} onChange={(e) => setAgentId(e.target.value)}><option value="">Unassigned</option>{agents.map((agent) => <option key={agent.id} value={agent.id}>{agent.name}</option>)}</select></label><label>Priority<select value={priority} onChange={(e) => setPriority(e.target.value)}><option>Low</option><option>Normal</option><option>High</option><option>Critical</option></select></label><label className="span-2">Notes<textarea value={notes} onChange={(e) => setNotes(e.target.value)} placeholder="Context, acceptance criteria or instructions" /></label><div className="modal-actions span-2"><button type="button" className="soft-button" onClick={onClose}>Cancel</button><button className="primary-button">Create task</button></div></form></Modal>;
}

function Modal({ title, subtitle, onClose, children }: { title: string; subtitle: string; onClose: () => void; children: ReactNode }) {
  return <div className="modal-backdrop" onMouseDown={(e) => e.target === e.currentTarget && onClose()}><section className="modal"><header><div><h2>{title}</h2><p>{subtitle}</p></div><button onClick={onClose}>×</button></header>{children}</section></div>;
}

function CommandPalette({ agents, tasks, query, setQuery, onClose, onTab, onAgent, onNewAgent, onNewTask }: {
  agents: Agent[]; tasks: TaskItem[]; query: string; setQuery: (v: string) => void; onClose: () => void; onTab: (tab: Tab) => void; onAgent: (a: Agent) => void; onNewAgent: () => void; onNewTask: () => void;
}) {
  const q = query.toLowerCase();
  const results: { label: string; meta: string; run: () => void }[] = [
    { label: "Create agent", meta: "Action", run: onNewAgent }, { label: "Create task", meta: "Action", run: onNewTask },
    ...tabs.map((tab) => ({ label: `Open ${tab}`, meta: "Navigate", run: () => onTab(tab) })),
    ...agents.map((agent) => ({ label: agent.name, meta: `${agent.status} · ${agent.role || "Agent"}`, run: () => onAgent(agent) })),
    ...tasks.slice(0, 20).map((task) => ({ label: task.title, meta: `${task.status} · ${task.agentName || "Unassigned"}`, run: () => onTab("Tasks") })),
  ].filter((item) => `${item.label} ${item.meta}`.toLowerCase().includes(q)).slice(0, 12);
  return <div className="palette-backdrop" onMouseDown={(e) => e.target === e.currentTarget && onClose()}><section className="command-palette"><div className="palette-input"><span>⌕</span><input autoFocus value={query} onChange={(e) => setQuery(e.target.value)} placeholder="Type a command, agent or task…" /><kbd>Esc</kbd></div><div className="palette-results">{results.map((item, index) => <button key={`${item.label}-${index}`} onClick={() => { item.run(); onClose(); }}><span>{item.label}</span><small>{item.meta}</small></button>)}{!results.length && <EmptyState title="No match" copy="Try a different command or name." />}</div></section></div>;
}

export default function App() {
  const [active, setActive] = useState<Tab>("Overview");
  const [agents, setAgents] = useState<Agent[]>([]);
  const [tasks, setTasks] = useState<TaskItem[]>([]);
  const [activity, setActivity] = useState<ActivityItem[]>([]);
  const [metrics, setMetrics] = useState<Metrics>(emptyMetrics);
  const [selectedAgentId, setSelectedAgentId] = useState<string | null>(null);
  const [agentModal, setAgentModal] = useState(false);
  const [taskModal, setTaskModal] = useState(false);
  const [paletteOpen, setPaletteOpen] = useState(false);
  const [paletteQuery, setPaletteQuery] = useState("");
  const [search, setSearch] = useState("");
  const [dataPath, setDataPath] = useState("");
  const [toast, setToast] = useState("");

  const refresh = useCallback(async () => {
    const [nextAgents, nextTasks, nextActivity, nextMetrics] = await Promise.all([nodus.listAgents(), nodus.listTasks(), nodus.listActivity(150), nodus.metrics()]);
    setAgents(nextAgents); setTasks(nextTasks); setActivity(nextActivity); setMetrics(nextMetrics);
  }, []);

  useEffect(() => { void refresh(); void nodus.appDataPath().then(setDataPath); }, [refresh]);
  useEffect(() => {
    const id = window.setInterval(() => void refresh(), 5000);
    return () => window.clearInterval(id);
  }, [refresh]);

  useEffect(() => {
    const onKey = (event: KeyboardEvent) => {
      if ((event.ctrlKey || event.metaKey) && event.key.toLowerCase() === "k") { event.preventDefault(); setPaletteOpen((v) => !v); setPaletteQuery(""); }
      if (event.key === "Escape") { setPaletteOpen(false); setSelectedAgentId(null); setAgentModal(false); setTaskModal(false); }
      if (event.ctrlKey && /^[1-6]$/.test(event.key)) { event.preventDefault(); setActive(tabs[Number(event.key) - 1]); }
    };
    window.addEventListener("keydown", onKey); return () => window.removeEventListener("keydown", onKey);
  }, []);

  const selectedAgent = agents.find((agent) => agent.id === selectedAgentId) ?? null;
  const filteredAgents = useMemo(() => search ? agents.filter((a) => `${a.name} ${a.role} ${a.description}`.toLowerCase().includes(search.toLowerCase())) : agents, [agents, search]);
  const filteredTasks = useMemo(() => search ? tasks.filter((t) => `${t.title} ${t.agentName} ${t.status}`.toLowerCase().includes(search.toLowerCase())) : tasks, [tasks, search]);

  const flash = (message: string) => { setToast(message); window.setTimeout(() => setToast(""), 3200); };
  const createAgent = async (input: AgentInput) => { const agent = await nodus.createAgent(input); await refresh(); setSelectedAgentId(agent.id); flash(`${agent.name} added to Nodus`); };
  const createTask = async (input: { title: string; agentId?: string | null; priority: string; notes: string }) => { await nodus.createTask(input); await refresh(); flash("Task created"); };
  const taskStatus = async (task: TaskItem, status: TaskStatus) => { await nodus.setTaskStatus(task.id, status); if (status === "Completed") notify("Nodus · Task completed", task.title); if (status === "Failed") notify("Nodus · Task failed", task.title); await refresh(); };
  const deleteTask = async (task: TaskItem) => { if (!window.confirm(`Delete “${task.title}”?`)) return; await nodus.deleteTask(task.id); await refresh(); };
  const backup = async () => { try { flash(`Backup: ${await nodus.backupDatabase()}`); } catch (e) { flash(String(e)); } };
  const exportJson = async () => { const json = await nodus.exportConfig(); const blob = new Blob([json], { type: "application/json" }); const url = URL.createObjectURL(blob); const a = document.createElement("a"); a.href = url; a.download = `nodus-export-${new Date().toISOString().slice(0, 10)}.json`; a.click(); URL.revokeObjectURL(url); flash("Configuration exported"); };

  const content = (() => {
    switch (active) {
      case "Overview": return <Overview agents={filteredAgents} tasks={filteredTasks} activity={activity} metrics={metrics} onOpenAgent={(a) => setSelectedAgentId(a.id)} onNewTask={() => setTaskModal(true)} onNavigate={setActive} onTaskStatus={taskStatus} onDeleteTask={deleteTask} />;
      case "Agents": return <AgentsPage agents={filteredAgents} onOpen={(a) => setSelectedAgentId(a.id)} onNew={() => setAgentModal(true)} />;
      case "Terminal": return <GlobalTerminal agents={filteredAgents} onOpen={(a) => setSelectedAgentId(a.id)} />;
      case "Tasks": return <TasksPage tasks={filteredTasks} agents={agents} onNew={() => setTaskModal(true)} onStatus={taskStatus} onDelete={deleteTask} />;
      case "Activity": return <ActivityPage activity={activity} />;
      case "Settings": return <SettingsPage dataPath={dataPath} onBackup={backup} onExport={exportJson} />;
    }
  })();

  return (
    <div className="app-shell">
      <Header active={active} onTab={setActive} onPalette={() => { setPaletteOpen(true); setPaletteQuery(""); }} search={search} setSearch={setSearch} />
      {content}
      {selectedAgent && <AgentDetail agent={selectedAgent} tasks={tasks} activity={activity} onClose={() => setSelectedAgentId(null)} onChanged={refresh} />}
      {agentModal && <AgentModal onClose={() => setAgentModal(false)} onSave={createAgent} />}
      {taskModal && <TaskModal agents={agents} onClose={() => setTaskModal(false)} onSave={createTask} />}
      {paletteOpen && <CommandPalette agents={agents} tasks={tasks} query={paletteQuery} setQuery={setPaletteQuery} onClose={() => setPaletteOpen(false)} onTab={setActive} onAgent={(a) => setSelectedAgentId(a.id)} onNewAgent={() => setAgentModal(true)} onNewTask={() => setTaskModal(true)} />}
      {toast && <div className="toast">{toast}</div>}
    </div>
  );
}
