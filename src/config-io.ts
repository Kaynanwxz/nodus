import { nodus, type Agent, type MemoryEntry, type PermissionSet, type TaskItem } from "./nodus";

type ImportShape = {
  agents?: Agent[];
  tasks?: TaskItem[];
  permissions?: PermissionSet[];
  memory?: MemoryEntry[];
};

export async function importNodusConfig(raw: string, replace = true) {
  const parsed = JSON.parse(raw) as ImportShape;
  if (!Array.isArray(parsed.agents)) throw new Error("Invalid Nodus export: agents array is missing");

  if (replace) {
    const [tasks, agents] = await Promise.all([nodus.listTasks(), nodus.listAgents()]);
    for (const task of tasks) await nodus.deleteTask(task.id);
    for (const agent of agents) await nodus.deleteAgent(agent.id);
  }

  const idMap = new Map<string, string>();
  for (const source of parsed.agents) {
    const created = await nodus.createAgent({
      name: source.name,
      role: source.role ?? "",
      description: source.description ?? "",
      command: source.command ?? "",
      workspace: source.workspace ?? "",
      provider: source.provider ?? "",
      model: source.model ?? "",
    });
    idMap.set(source.id, created.id);
  }

  for (const permission of parsed.permissions ?? []) {
    const agentId = idMap.get(permission.agentId);
    if (!agentId) continue;
    await nodus.setPermissions({ ...permission, agentId });
  }

  for (const entry of parsed.memory ?? []) {
    const agentId = idMap.get(entry.agentId);
    if (!agentId) continue;
    await nodus.setMemory(agentId, entry.key, entry.value);
  }

  for (const task of [...(parsed.tasks ?? [])].reverse()) {
    const agentId = task.agentId ? idMap.get(task.agentId) ?? null : null;
    const created = await nodus.createTask({
      title: task.title,
      agentId,
      priority: task.priority ?? "Normal",
      notes: task.notes ?? "",
    });
    if (task.status === "Running" || task.status === "Completed" || task.status === "Failed") {
      await nodus.setTaskStatus(created.id, task.status);
    }
  }

  return {
    agents: parsed.agents.length,
    tasks: parsed.tasks?.length ?? 0,
    memory: parsed.memory?.length ?? 0,
  };
}
