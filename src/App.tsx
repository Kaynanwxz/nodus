import { useEffect, useMemo, useState, type ReactNode } from "react";
import { getCurrentWindow } from "@tauri-apps/api/window";

type Tab = "Overview" | "Agents" | "Terminal" | "Tasks" | "Activity" | "Settings";
type AgentState = "Active" | "Working" | "Idle";

type Agent = {
  name: string;
  role: string;
  description: string;
  state: AgentState;
  tasks: number;
  avatar: string;
};

type Task = {
  title: string;
  agent: string;
  state: "Running" | "Queued" | "Completed";
  timing: string;
};

const tabs: Tab[] = ["Overview", "Agents", "Terminal", "Tasks", "Activity", "Settings"];

const agents: Agent[] = [
  {
    name: "Clio",
    role: "Research & Analysis",
    description: "Finds, reads and synthesizes knowledge across domains.",
    state: "Active",
    tasks: 3,
    avatar: "/assets/clio.webp",
  },
  {
    name: "Daedalus",
    role: "Build & Prototype",
    description: "Turns ideas into working artifacts and experiments.",
    state: "Working",
    tasks: 4,
    avatar: "/assets/daedalus.webp",
  },
  {
    name: "Scribe",
    role: "Write & Communicate",
    description: "Creates clear, refined content in your voice.",
    state: "Active",
    tasks: 2,
    avatar: "/assets/scribe.webp",
  },
  {
    name: "Athena",
    role: "Strategy & Planning",
    description: "Breaks down complexity and maps the next steps.",
    state: "Active",
    tasks: 3,
    avatar: "/assets/athena.webp",
  },
];

const tasks: Task[] = [
  { title: "Compile competitive landscape", agent: "Clio", state: "Running", timing: "14m left" },
  { title: "Build interactive map prototype", agent: "Daedalus", state: "Running", timing: "32m left" },
  { title: "Write launch announcement", agent: "Scribe", state: "Queued", timing: "—" },
  { title: "Plan Q4 strategic initiatives", agent: "Athena", state: "Queued", timing: "—" },
  { title: "Summarize user feedback (Sept)", agent: "Clio", state: "Completed", timing: "2h ago" },
];

const activity = [
  { actor: "Clio", line: "completed research", detail: "Trends in regenerative cities", time: "2m ago", tone: "good" },
  { actor: "Scribe", line: "generated document", detail: "Market analysis brief v2", time: "12m ago", tone: "paper" },
  { actor: "Daedalus", line: "deployed prototype", detail: "nexus-map v0.3", time: "28m ago", tone: "paper" },
  { actor: "Athena", line: "added a new task", detail: "Evaluate partnership opportunities", time: "1h ago", tone: "paper" },
  { actor: "Task", line: "completed", detail: "Summarize Q3 learnings", time: "2h ago", tone: "good" },
  { actor: "You", line: "updated agent settings", detail: "Clio — knowledge sources", time: "3h ago", tone: "paper" },
  { actor: "Daedalus", line: "started a task", detail: "Build data visualisation", time: "3h ago", tone: "gold" },
];

function WindowButtons() {
  const appWindow = getCurrentWindow();
  return (
    <div className="window-buttons" data-tauri-drag-region="false">
      <button aria-label="Minimize" onClick={() => appWindow.minimize()}>−</button>
      <button aria-label="Maximize" onClick={() => appWindow.toggleMaximize()}>□</button>
      <button aria-label="Close" className="close" onClick={() => appWindow.close()}>×</button>
    </div>
  );
}

function Header({ active, onTab }: { active: Tab; onTab: (tab: Tab) => void }) {
  const [clock, setClock] = useState(new Date());

  useEffect(() => {
    const id = window.setInterval(() => setClock(new Date()), 1000);
    return () => window.clearInterval(id);
  }, []);

  const time = clock.toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" });
  const date = clock.toLocaleDateString([], { month: "short", day: "2-digit" });

  return (
    <header className="topbar" data-tauri-drag-region>
      <div className="brand" data-tauri-drag-region>
        <div className="brand-title">NODUS</div>
        <div className="brand-subtitle">Ideas · agents · actions</div>
      </div>

      <nav className="main-nav" aria-label="Primary">
        {tabs.map((tab) => (
          <button
            key={tab}
            className={active === tab ? "nav-item active" : "nav-item"}
            onClick={() => onTab(tab)}
          >
            {tab}
          </button>
        ))}
      </nav>

      <div className="header-tools">
        <div className="search-box">
          <span className="search-icon">⌕</span>
          <input aria-label="Search" placeholder="Search Nodus" />
        </div>
        <button className="pulse-medallion" aria-label="Quick actions">✦</button>
        <div className="clock">
          <strong>{time}</strong>
          <span>{date}</span>
        </div>
        <div className="profile"><img src="/assets/profile.webp" alt="Profile" /></div>
        <WindowButtons />
      </div>
    </header>
  );
}

function MetricCard({ title, value, note, good = false }: { title: string; value: string; note: string; good?: boolean }) {
  return (
    <div className="metric-card">
      <div className="metric-title">{title}</div>
      <div className="metric-value">{value}</div>
      <div className={good ? "metric-note good" : "metric-note"}>{note}</div>
    </div>
  );
}

function AgentCard({ agent }: { agent: Agent }) {
  return (
    <article className="agent-card">
      <button className="ellipsis" aria-label={`More actions for ${agent.name}`}>•••</button>
      <div className="agent-topline">
        <div className="avatar-wrap">
          <img src={agent.avatar} alt="" />
          <span className={`presence ${agent.state.toLowerCase()}`} />
        </div>
        <span className={`status-pill ${agent.state.toLowerCase()}`}>{agent.state}</span>
      </div>
      <h3>{agent.name}</h3>
      <p className="agent-role">{agent.role}</p>
      <p className="agent-description">{agent.description}</p>
      <div className="agent-footer">
        <span className="task-count">{agent.tasks} active tasks</span>
        <span className="agent-arrow">↗</span>
      </div>
    </article>
  );
}

function TaskTable() {
  return (
    <section className="panel tasks-panel">
      <div className="panel-head task-head">
        <div>
          <div className="panel-title">Tasks</div>
          <div className="panel-subtitle">Current work across your agents</div>
        </div>
        <div className="task-filters">
          <button className="selected">All</button>
          <button>Running</button>
          <button>Queued</button>
          <button>Completed</button>
        </div>
        <button className="view-all">View all ↗</button>
      </div>
      <div className="task-table">
        {tasks.map((task) => {
          const avatar = agents.find((agent) => agent.name === task.agent)?.avatar;
          return (
            <div className="task-row" key={task.title}>
              <span className="task-check" />
              <span className="task-name">{task.title}</span>
              <span className="task-agent">{avatar && <img src={avatar} alt="" />} {task.agent}</span>
              <span className={`task-state ${task.state.toLowerCase()}`}>{task.state}</span>
              <span className="task-time">{task.timing}</span>
              <button className="task-menu" aria-label={`More actions for ${task.title}`}>•••</button>
            </div>
          );
        })}
      </div>
    </section>
  );
}

function ActivityPanel() {
  return (
    <aside className="panel activity-panel">
      <div className="panel-head">
        <div>
          <div className="panel-title">Activity</div>
          <div className="panel-subtitle">Recent changes and agent events</div>
        </div>
        <button className="view-all">View all ↗</button>
      </div>
      <div className="activity-list">
        {activity.map((item, index) => (
          <div className="activity-item" key={`${item.actor}-${index}`}>
            <span className={`activity-marker ${item.tone}`} />
            <div className="activity-copy">
              <p><strong>{item.actor}</strong> {item.line}</p>
              <em>{item.detail}</em>
            </div>
            <time>{item.time}</time>
          </div>
        ))}
      </div>
    </aside>
  );
}

function Overview() {
  return (
    <div className="overview-stage">
      <img className="left-rail" src="/assets/left-rail.webp" alt="" aria-hidden="true" />
      <img className="right-rail" src="/assets/right-rail.webp" alt="" aria-hidden="true" />

      <div className="center-canvas">
        <section className="hero-art" aria-label="A more thoughtful tomorrow, composed today">
          <img src="/assets/hero.webp" alt="" aria-hidden="true" />
        </section>

        <section className="metrics-grid" aria-label="System overview">
          <MetricCard title="Agents online" value="4 / 6" note="3 active · 1 idle" good />
          <MetricCard title="Tasks running" value="12" note="3 completed today" />
          <MetricCard title="Total actions" value="1,428" note="18% more than yesterday" />
          <MetricCard title="System health" value="98%" note="All systems nominal" good />
        </section>

        <div className="dashboard-grid">
          <div className="dashboard-main">
            <section className="panel agents-panel">
              <div className="panel-head">
                <div>
                  <div className="panel-title">Agents</div>
                  <div className="panel-subtitle">Your active intelligence layer</div>
                </div>
                <button className="view-all">View all ↗</button>
              </div>
              <div className="agents-grid">{agents.map((agent) => <AgentCard agent={agent} key={agent.name} />)}</div>
            </section>
            <TaskTable />
          </div>
          <ActivityPanel />
        </div>
      </div>

      <img className="bottom-art" src="/assets/bottom-center.webp" alt="" aria-hidden="true" />
    </div>
  );
}

function AgentsPage() {
  const extendedAgents = agents.concat(
    agents.slice(0, 2).map((agent) => ({ ...agent, name: `${agent.name} II`, state: "Idle" as AgentState })),
  );

  return (
    <PageShell eyebrow="Agent registry" title="Agents" subtitle="Every intelligence connected to Nodus, in one calm workspace.">
      <div className="page-agent-grid">{extendedAgents.map((agent) => <AgentCard agent={agent} key={agent.name} />)}</div>
    </PageShell>
  );
}

function TerminalPage() {
  return (
    <PageShell eyebrow="Local shell" title="Terminal" subtitle="Direct access to agent processes without leaving the orchestration surface.">
      <div className="terminal-shell">
        <div className="terminal-top"><span>Hermes · primary session</span><span>Local · ready</span></div>
        <pre><span className="prompt">PS C:\Agents\Hermes&gt;</span> hermes --resume{"\n"}<span className="ok">[OK]</span> workspace loaded{"\n"}<span className="muted">[18:42:11] websocket.service.ts indexed</span>{"\n"}<span className="muted">[18:42:14] waiting for next instruction</span>{"\n\n"}<span className="prompt">PS C:\Agents\Hermes&gt;</span> <span className="cursor">_</span></pre>
      </div>
    </PageShell>
  );
}

function TasksPage() {
  return <PageShell eyebrow="Operations" title="Tasks" subtitle="Work moving through Nodus, from intent to completion."><TaskTable /></PageShell>;
}

function ActivityPage() {
  return <PageShell eyebrow="Archive" title="Activity" subtitle="A quiet record of the work happening across Nodus."><div className="wide-activity"><ActivityPanel /></div></PageShell>;
}

function SettingsPage() {
  return (
    <PageShell eyebrow="Control plane" title="Settings" subtitle="Tune the workspace without disturbing its visual rhythm.">
      <div className="settings-grid">
        <section className="settings-card"><h3>Interface</h3><SettingRow label="Theme" value="Classical collage dark" /><SettingRow label="Density" value="Comfortable" /><SettingRow label="Motion" value="Restrained" /></section>
        <section className="settings-card"><h3>Agent defaults</h3><SettingRow label="Workspace" value="C:\Agents" /><SettingRow label="Shell" value="PowerShell" /><SettingRow label="Permissions" value="Ask before action" /></section>
      </div>
    </PageShell>
  );
}

function SettingRow({ label, value }: { label: string; value: string }) {
  return <div className="setting-row"><span>{label}</span><strong>{value}</strong></div>;
}

function PageShell({ eyebrow, title, subtitle, children }: { eyebrow: string; title: string; subtitle: string; children: ReactNode }) {
  return (
    <main className="secondary-page">
      <div className="page-heading"><span>{eyebrow}</span><h1>{title}</h1><p>{subtitle}</p></div>
      {children}
    </main>
  );
}

export default function App() {
  const [active, setActive] = useState<Tab>("Overview");

  const body = useMemo(() => {
    switch (active) {
      case "Overview": return <Overview />;
      case "Agents": return <AgentsPage />;
      case "Terminal": return <TerminalPage />;
      case "Tasks": return <TasksPage />;
      case "Activity": return <ActivityPage />;
      case "Settings": return <SettingsPage />;
    }
  }, [active]);

  return (
    <div className="app-shell">
      <Header active={active} onTab={setActive} />
      {body}
    </div>
  );
}
