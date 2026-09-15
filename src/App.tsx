import { useEffect, useMemo, useState } from "react";
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
    description: "Finds, reads, and synthesizes knowledge across domains.",
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
  { icon: "●", actor: "Clio", line: "completed research", detail: "“Trends in regenerative cities”", time: "2m ago", tone: "good" },
  { icon: "▤", actor: "Scribe", line: "generated document", detail: "Market analysis brief v2", time: "12m ago", tone: "paper" },
  { icon: "◉", actor: "Daedalus", line: "deployed prototype", detail: "nexus-map v0.3", time: "28m ago", tone: "paper" },
  { icon: "●", actor: "Athena", line: "added a new task", detail: "Evaluate partnership opportunities", time: "1h ago", tone: "paper" },
  { icon: "✓", actor: "Task", line: "completed", detail: "Summarize Q3 learnings", time: "2h ago", tone: "good" },
  { icon: "♟", actor: "You", line: "updated agent settings", detail: "Clio – knowledge sources", time: "3h ago", tone: "paper" },
  { icon: "ϟ", actor: "Daedalus", line: "started a task", detail: "Build data visualisation", time: "3h ago", tone: "gold" },
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

  const time = clock.toLocaleTimeString([], { hour: "2-digit", minute: "2-digit", second: "2-digit" });
  const date = clock.toLocaleDateString([], { month: "short", day: "2-digit", year: "numeric" });

  return (
    <header className="topbar" data-tauri-drag-region>
      <div className="brand" data-tauri-drag-region>
        <div className="brand-title">NODUS</div>
        <div className="brand-subtitle">IDEAS. AGENTS. ACTIONS. TOGETHER.</div>
      </div>
      <nav className="main-nav" aria-label="Primary">
        {tabs.map((tab) => (
          <button key={tab} className={active === tab ? "nav-item active" : "nav-item"} onClick={() => onTab(tab)}>
            {tab.toUpperCase()}
          </button>
        ))}
      </nav>
      <div className="header-tools">
        <div className="search-box">
          <span className="search-icon">⌕</span>
          <input aria-label="Search" placeholder="Search agents, tasks, or knowledge..." />
        </div>
        <div className="pulse-medallion">⌁</div>
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

function MetricCard({ icon, title, value, note, good = false }: { icon: string; title: string; value: string; note: string; good?: boolean }) {
  return (
    <div className="metric-card">
      <div className="metric-icon">{icon}</div>
      <div className="metric-copy">
        <div className="metric-title">{title}</div>
        <div className="metric-value">{value}</div>
        <div className={good ? "metric-note good" : "metric-note"}>{good ? "● " : ""}{note}</div>
      </div>
    </div>
  );
}

function AgentCard({ agent }: { agent: Agent }) {
  return (
    <article className="agent-card">
      <button className="ellipsis" aria-label={`More actions for ${agent.name}`}>•••</button>
      <div className="avatar-wrap">
        <img src={agent.avatar} alt="" />
        <span className={`presence ${agent.state.toLowerCase()}`} />
      </div>
      <h3>{agent.name}</h3>
      <p className="agent-role">{agent.role}</p>
      <p className="agent-description">{agent.description}</p>
      <div className="agent-footer">
        <span className={`status-pill ${agent.state.toLowerCase()}`}>{agent.state === "Working" ? "★" : "●"} {agent.state}</span>
        <span className="task-count">{agent.tasks} tasks</span>
      </div>
    </article>
  );
}

function TaskTable() {
  return (
    <section className="panel tasks-panel">
      <div className="panel-head task-head">
        <div className="panel-title">TASKS <span className="hairline" /></div>
        <div className="task-filters">
          <button className="selected">All (12)</button>
          <button>Running (4)</button>
          <button>Queued (3)</button>
          <button>Completed (5)</button>
        </div>
        <button className="view-all">View all →</button>
      </div>
      <div className="task-table">
        {tasks.map((task) => {
          const avatar = agents.find((agent) => agent.name === task.agent)?.avatar;
          return (
            <div className="task-row" key={task.title}>
              <span className="task-check" />
              <span className="task-name">{task.title}</span>
              <span className="task-agent">{avatar && <img src={avatar} alt="" />} {task.agent}</span>
              <span className={`task-state ${task.state.toLowerCase()}`}>{task.state === "Running" ? "★" : task.state === "Completed" ? "●" : "◆"} {task.state}</span>
              <span className="task-time">{task.timing}</span>
              <span className="task-menu">•••</span>
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
        <div className="panel-title">ACTIVITY <span className="hairline" /></div>
        <button className="view-all">View all →</button>
      </div>
      <div className="activity-list">
        {activity.map((item, index) => (
          <div className="activity-item" key={`${item.actor}-${index}`}>
            <span className={`activity-icon ${item.tone}`}>{item.icon}</span>
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
          <img src="/assets/hero.webp" alt="A more thoughtful tomorrow, composed today" />
        </section>
        <section className="metrics-grid">
          <MetricCard icon="♧" title="Agents Online" value="4 / 6" note="3 active · 1 idle" good />
          <MetricCard icon="☑" title="Tasks Running" value="12" note="↑ 3 completed today" />
          <MetricCard icon="ϟ" title="Total Actions" value="1,428" note="+18% vs. yesterday" />
          <MetricCard icon="⊙" title="System Health" value="98%" note="All systems nominal" good />
        </section>
        <div className="dashboard-grid">
          <div className="dashboard-main">
            <section className="panel agents-panel">
              <div className="panel-head">
                <div className="panel-title">AGENTS <span className="hairline" /></div>
                <button className="view-all">View all →</button>
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
  return (
    <PageShell eyebrow="REGISTRY / AGENT NODES" title="Agents" subtitle="A living directory of every intelligence connected to Nodus.">
      <div className="page-agent-grid">{agents.concat(agents.slice(0, 2).map((agent) => ({ ...agent, name: `${agent.name} II`, state: "Idle" as AgentState }))).map((agent) => <AgentCard agent={agent} key={agent.name} />)}</div>
    </PageShell>
  );
}

function TerminalPage() {
  return (
    <PageShell eyebrow="LOCAL SHELL / ACTIVE SESSION" title="Terminal" subtitle="Direct access to agent processes without leaving the orchestration surface.">
      <div className="terminal-shell">
        <div className="terminal-top"><span>Hermes / primary session</span><span>LOCAL · READY</span></div>
        <pre><span className="prompt">PS C:\Agents\Hermes&gt;</span> hermes --resume{"\n"}<span className="ok">[OK]</span> workspace loaded{"\n"}<span className="muted">[18:42:11] websocket.service.ts indexed</span>{"\n"}<span className="muted">[18:42:14] waiting for next instruction</span>{"\n\n"}<span className="prompt">PS C:\Agents\Hermes&gt;</span> <span className="cursor">_</span></pre>
      </div>
    </PageShell>
  );
}

function TasksPage() {
  return <PageShell eyebrow="OPERATIONS / TASK BOARD" title="Tasks" subtitle="Work moving through Nodus, from intent to completion."><TaskTable /></PageShell>;
}

function ActivityPage() {
  return <PageShell eyebrow="ARCHIVE / AUDIT STREAM" title="Activity" subtitle="Every relevant action, recorded as a quiet operational archive."><div className="wide-activity"><ActivityPanel /></div></PageShell>;
}

function SettingsPage() {
  return (
    <PageShell eyebrow="CONTROL PLANE" title="Settings" subtitle="Adjust Nodus without breaking its visual language.">
      <div className="settings-grid">
        <section className="settings-card"><h3>Interface</h3><SettingRow label="Theme" value="Classical Collage Dark" /><SettingRow label="Density" value="Editorial" /><SettingRow label="Motion" value="Restrained" /></section>
        <section className="settings-card"><h3>Agent defaults</h3><SettingRow label="Workspace" value="C:\Agents" /><SettingRow label="Shell" value="PowerShell" /><SettingRow label="Permissions" value="Ask before action" /></section>
      </div>
    </PageShell>
  );
}

function SettingRow({ label, value }: { label: string; value: string }) {
  return <div className="setting-row"><span>{label}</span><strong>{value}</strong></div>;
}

function PageShell({ eyebrow, title, subtitle, children }: { eyebrow: string; title: string; subtitle: string; children: React.ReactNode }) {
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
