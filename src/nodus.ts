import { invoke } from "@tauri-apps/api/core";

export type PermissionValue = "Ask" | "Allow" | "Deny";
export type AgentStatus = "Running" | "Stopped" | "Error";
export type TaskStatus = "Queued" | "Running" | "Completed" | "Failed";

export type Agent = {
  id: string;
  name: string;
  role: string;
  description: string;
  command: string;
  workspace: string;
  provider: string;
  model: string;
  status: string;
  pid?: number | null;
  createdAt: string;
};

export type AgentInput = Omit<Agent, "id" | "status" | "pid" | "createdAt">;

export type TaskItem = {
  id: string;
  title: string;
  agentId?: string | null;
  agentName: string;
  priority: string;
  status: TaskStatus | string;
  createdAt: string;
  startedAt?: string | null;
  completedAt?: string | null;
  notes: string;
};

export type ActivityItem = {
  id: number;
  kind: string;
  actor: string;
  message: string;
  detail: string;
  createdAt: string;
  level: string;
};

export type PermissionSet = {
  agentId: string;
  files: PermissionValue;
  terminal: PermissionValue;
  browser: PermissionValue;
  git: PermissionValue;
  network: PermissionValue;
};

export type MemoryEntry = {
  id: number;
  agentId: string;
  key: string;
  value: string;
  updatedAt: string;
};

export type Metrics = {
  agentsOnline: number;
  agentsTotal: number;
  tasksRunning: number;
  actionsToday: number;
  errorsToday: number;
};

export type CommandResult = { status: number; stdout: string; stderr: string };

const browserMode = !("__TAURI_INTERNALS__" in window);
const STORE = "nodus-browser-state-v1";

type BrowserState = {
  agents: Agent[];
  tasks: TaskItem[];
  activity: ActivityItem[];
  permissions: PermissionSet[];
  memory: MemoryEntry[];
};

const now = () => new Date().toISOString();
const uid = () => crypto.randomUUID?.() ?? `${Date.now()}-${Math.random()}`;

const defaults: BrowserState = {
  agents: [
    { id: "clio", name: "Clio", role: "Research & Analysis", description: "Finds, reads and synthesizes knowledge across domains.", command: "", workspace: "", provider: "", model: "", status: "Stopped", pid: null, createdAt: now() },
    { id: "daedalus", name: "Daedalus", role: "Build & Prototype", description: "Turns ideas into working artifacts and experiments.", command: "", workspace: "", provider: "", model: "", status: "Stopped", pid: null, createdAt: now() },
    { id: "scribe", name: "Scribe", role: "Write & Communicate", description: "Creates clear, refined content in your voice.", command: "", workspace: "", provider: "", model: "", status: "Stopped", pid: null, createdAt: now() },
    { id: "athena", name: "Athena", role: "Strategy & Planning", description: "Breaks down complexity and maps the next steps.", command: "", workspace: "", provider: "", model: "", status: "Stopped", pid: null, createdAt: now() },
  ],
  tasks: [],
  activity: [],
  permissions: [],
  memory: [],
};

function loadBrowser(): BrowserState {
  try {
    const raw = localStorage.getItem(STORE);
    return raw ? { ...defaults, ...JSON.parse(raw) } : structuredClone(defaults);
  } catch {
    return structuredClone(defaults);
  }
}

function saveBrowser(state: BrowserState) {
  localStorage.setItem(STORE, JSON.stringify(state));
}

function addBrowserActivity(state: BrowserState, actor: string, message: string, detail = "", level = "info", kind = "system") {
  state.activity.unshift({ id: Date.now(), kind, actor, message, detail, createdAt: now(), level });
  state.activity = state.activity.slice(0, 500);
}

function permissionFor(state: BrowserState, agentId: string): PermissionSet {
  return state.permissions.find((p) => p.agentId === agentId) ?? {
    agentId, files: "Ask", terminal: "Ask", browser: "Ask", git: "Ask", network: "Ask",
  };
}

async function native<T>(command: string, args?: Record<string, unknown>): Promise<T> {
  return invoke<T>(command, args);
}

export const nodus = {
  isDesktop: !browserMode,

  async listAgents(): Promise<Agent[]> {
    if (!browserMode) return native("list_agents");
    return loadBrowser().agents;
  },

  async createAgent(input: AgentInput): Promise<Agent> {
    if (!browserMode) return native("create_agent", { input });
    const state = loadBrowser();
    const agent: Agent = { ...input, id: uid(), status: "Stopped", pid: null, createdAt: now() };
    state.agents.push(agent);
    addBrowserActivity(state, agent.name, "created agent", "", "info", "agent");
    saveBrowser(state);
    return agent;
  },

  async updateAgent(id: string, input: AgentInput): Promise<Agent> {
    if (!browserMode) return native("update_agent", { id, input });
    const state = loadBrowser();
    const index = state.agents.findIndex((a) => a.id === id);
    if (index < 0) throw new Error("Agent not found");
    state.agents[index] = { ...state.agents[index], ...input };
    saveBrowser(state);
    return state.agents[index];
  },

  async deleteAgent(id: string) {
    if (!browserMode) return native<void>("delete_agent", { id });
    const state = loadBrowser();
    const agent = state.agents.find((a) => a.id === id);
    state.agents = state.agents.filter((a) => a.id !== id);
    state.permissions = state.permissions.filter((p) => p.agentId !== id);
    state.memory = state.memory.filter((m) => m.agentId !== id);
    if (agent) addBrowserActivity(state, agent.name, "deleted agent", "", "warn", "agent");
    saveBrowser(state);
  },

  async startAgent(id: string, confirmed = false): Promise<Agent> {
    if (!browserMode) return native("start_agent", { id, confirmed });
    const state = loadBrowser();
    const agent = state.agents.find((a) => a.id === id);
    if (!agent) throw new Error("Agent not found");
    const p = permissionFor(state, id).terminal;
    if (p === "Deny") throw new Error("PERMISSION_DENIED:terminal");
    if (p === "Ask" && !confirmed) throw new Error("PERMISSION_REQUIRED:terminal");
    agent.status = "Running";
    agent.pid = Math.floor(1000 + Math.random() * 8000);
    addBrowserActivity(state, agent.name, "started agent", `PID ${agent.pid}`, "success", "process");
    saveBrowser(state);
    return agent;
  },

  async stopAgent(id: string): Promise<Agent> {
    if (!browserMode) return native("stop_agent", { id });
    const state = loadBrowser();
    const agent = state.agents.find((a) => a.id === id);
    if (!agent) throw new Error("Agent not found");
    agent.status = "Stopped";
    agent.pid = null;
    addBrowserActivity(state, agent.name, "stopped agent", "", "warn", "process");
    saveBrowser(state);
    return agent;
  },

  async restartAgent(id: string, confirmed = false): Promise<Agent> {
    if (!browserMode) return native("restart_agent", { id, confirmed });
    await this.stopAgent(id);
    return this.startAgent(id, confirmed);
  },

  async runCommand(id: string, command: string, confirmed = false): Promise<CommandResult> {
    if (!browserMode) return native("run_agent_command", { id, command, confirmed });
    const state = loadBrowser();
    const agent = state.agents.find((a) => a.id === id);
    if (!agent) throw new Error("Agent not found");
    const p = permissionFor(state, id).terminal;
    if (p === "Deny") throw new Error("PERMISSION_DENIED:terminal");
    if (p === "Ask" && !confirmed) throw new Error("PERMISSION_REQUIRED:terminal");
    addBrowserActivity(state, agent.name, "ran command", command, "success", "terminal");
    saveBrowser(state);
    return { status: 0, stdout: `[browser preview] ${command}\nDesktop mode executes this command in ${agent.workspace || "the configured workspace"}.`, stderr: "" };
  },

  async openWorkspace(id: string, confirmed = false) {
    if (!browserMode) return native<void>("open_workspace", { id, confirmed });
    const state = loadBrowser();
    const p = permissionFor(state, id).files;
    if (p === "Deny") throw new Error("PERMISSION_DENIED:files");
    if (p === "Ask" && !confirmed) throw new Error("PERMISSION_REQUIRED:files");
  },

  async listTasks(): Promise<TaskItem[]> {
    if (!browserMode) return native("list_tasks");
    return loadBrowser().tasks;
  },

  async createTask(input: { title: string; agentId?: string | null; priority: string; notes: string }): Promise<TaskItem> {
    if (!browserMode) return native("create_task", { input });
    const state = loadBrowser();
    const agent = state.agents.find((a) => a.id === input.agentId);
    const task: TaskItem = { id: uid(), ...input, agentName: agent?.name ?? "", status: "Queued", createdAt: now(), startedAt: null, completedAt: null };
    state.tasks.unshift(task);
    addBrowserActivity(state, "Nodus", "created task", task.title, "info", "task");
    saveBrowser(state);
    return task;
  },

  async setTaskStatus(id: string, status: TaskStatus): Promise<TaskItem> {
    if (!browserMode) return native("set_task_status", { id, status });
    const state = loadBrowser();
    const task = state.tasks.find((t) => t.id === id);
    if (!task) throw new Error("Task not found");
    task.status = status;
    if (status === "Running" && !task.startedAt) task.startedAt = now();
    if (["Completed", "Failed"].includes(status)) task.completedAt = now();
    addBrowserActivity(state, "Nodus", `task ${status.toLowerCase()}`, task.title, status === "Failed" ? "error" : "info", "task");
    saveBrowser(state);
    return task;
  },

  async deleteTask(id: string) {
    if (!browserMode) return native<void>("delete_task", { id });
    const state = loadBrowser();
    state.tasks = state.tasks.filter((t) => t.id !== id);
    saveBrowser(state);
  },

  async listActivity(limit = 100): Promise<ActivityItem[]> {
    if (!browserMode) return native("list_activity", { limit });
    return loadBrowser().activity.slice(0, limit);
  },

  async metrics(): Promise<Metrics> {
    if (!browserMode) return native("get_metrics");
    const state = loadBrowser();
    const today = new Date().toISOString().slice(0, 10);
    return {
      agentsOnline: state.agents.filter((a) => a.status === "Running").length,
      agentsTotal: state.agents.length,
      tasksRunning: state.tasks.filter((t) => t.status === "Running").length,
      actionsToday: state.activity.filter((a) => a.createdAt.startsWith(today)).length,
      errorsToday: state.activity.filter((a) => a.level === "error" && a.createdAt.startsWith(today)).length,
    };
  },

  async getPermissions(agentId: string): Promise<PermissionSet> {
    if (!browserMode) return native("get_permissions", { agentId });
    return permissionFor(loadBrowser(), agentId);
  },

  async setPermissions(value: PermissionSet): Promise<PermissionSet> {
    if (!browserMode) return native("set_permissions", { value });
    const state = loadBrowser();
    state.permissions = state.permissions.filter((p) => p.agentId !== value.agentId).concat(value);
    saveBrowser(state);
    return value;
  },

  async listMemory(agentId: string): Promise<MemoryEntry[]> {
    if (!browserMode) return native("list_memory", { agentId });
    return loadBrowser().memory.filter((m) => m.agentId === agentId);
  },

  async setMemory(agentId: string, key: string, value: string) {
    if (!browserMode) return native<void>("set_memory", { agentId, key, value });
    const state = loadBrowser();
    const existing = state.memory.find((m) => m.agentId === agentId && m.key === key);
    if (existing) { existing.value = value; existing.updatedAt = now(); }
    else state.memory.push({ id: Date.now(), agentId, key, value, updatedAt: now() });
    saveBrowser(state);
  },

  async deleteMemory(id: number) {
    if (!browserMode) return native<void>("delete_memory", { id });
    const state = loadBrowser();
    state.memory = state.memory.filter((m) => m.id !== id);
    saveBrowser(state);
  },

  async exportConfig(): Promise<string> {
    if (!browserMode) return native("export_config");
    return JSON.stringify(loadBrowser(), null, 2);
  },

  async backupDatabase(): Promise<string> {
    if (!browserMode) return "Browser preview uses localStorage; desktop backups are stored in the Nodus app-data folder.";
    return native("backup_database");
  },

  async appDataPath(): Promise<string> {
    if (!browserMode) return native("app_data_path");
    return "Browser preview · localStorage";
  },
};

export async function withPermissionRetry<T>(action: () => Promise<T>, retryConfirmed: () => Promise<T>): Promise<T> {
  try {
    return await action();
  } catch (error) {
    const message = String(error);
    if (message.includes("PERMISSION_REQUIRED:")) {
      const area = message.split("PERMISSION_REQUIRED:")[1]?.split(/\s/)[0] ?? "this action";
      if (window.confirm(`Allow ${area} access for this action?`)) return retryConfirmed();
    }
    throw error;
  }
}
