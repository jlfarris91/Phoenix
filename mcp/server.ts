import { Server } from "@modelcontextprotocol/sdk/server/index.js";
import { StdioServerTransport } from "@modelcontextprotocol/sdk/server/stdio.js";
import {
  CallToolRequestSchema,
  ListToolsRequestSchema,
} from "@modelcontextprotocol/sdk/types.js";

const BASE_URL = process.env.TESTRTS_URL ?? "http://127.0.0.1:8765";

// ── HTTP helper ───────────────────────────────────────────────────────────────

async function api(path: string): Promise<{ ok: boolean; data: unknown }> {
  try {
    const res = await fetch(`${BASE_URL}${path}`);
    const data = await res.json();
    return { ok: res.ok, data };
  } catch (e) {
    return {
      ok: false,
      data: { error: `Cannot reach TestRTS at ${BASE_URL}. Is the game running?` },
    };
  }
}

async function post(path: string, body: unknown): Promise<{ ok: boolean; data: unknown }> {
  try {
    const res = await fetch(`${BASE_URL}${path}`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(body),
    });
    const data = await res.json();
    return { ok: res.ok, data };
  } catch (e) {
    return {
      ok: false,
      data: { error: `Cannot reach TestRTS at ${BASE_URL}. Is the game running?` },
    };
  }
}

function text(data: unknown): { content: [{ type: "text"; text: string }] } {
  return {
    content: [{ type: "text" as const, text: JSON.stringify(data, null, 2) }],
  };
}

// ── Server ────────────────────────────────────────────────────────────────────

const server = new Server(
  { name: "testrts", version: "2.0.0" },
  { capabilities: { tools: {} } }
);

server.setRequestHandler(ListToolsRequestSchema, async () => ({
  tools: [
    {
      name: "get_stats",
      description:
        "Returns performance and entity-count statistics: sim FPS, sim time, " +
        "entity count, entity high-water mark, blackboard KVP count.",
      inputSchema: { type: "object" as const, properties: {} },
    },
    {
      name: "get_archetypes",
      description:
        "Lists all live ECS archetypes and their component composition. " +
        "Useful for understanding what types of entities exist.",
      inputSchema: { type: "object" as const, properties: {} },
    },
    {
      name: "list_entities",
      description:
        "Returns entity IDs (and kind) for all live entities, with optional filters. " +
        "Does NOT return component data — use get_entity or get_component for that.",
      inputSchema: {
        type: "object" as const,
        properties: {
          component: {
            type: "string",
            description: "Only return entities that have this component (e.g. 'UnitComponent').",
          },
          owner: {
            type: "number",
            description: "Only return entities owned by this player index.",
          },
          alive: {
            type: "boolean",
            description: "When true, exclude dead units.",
          },
        },
      },
    },
    {
      name: "get_entity",
      description:
        "Returns all reflected component fields for a single entity, serialised to JSON. " +
        "Use list_entities first to find entity IDs.",
      inputSchema: {
        type: "object" as const,
        required: ["id"],
        properties: {
          id: { type: "number", description: "Entity ID." },
        },
      },
    },
    {
      name: "get_component",
      description:
        "Returns the reflected fields of one specific component on an entity.",
      inputSchema: {
        type: "object" as const,
        required: ["id", "component"],
        properties: {
          id: { type: "number", description: "Entity ID." },
          component: {
            type: "string",
            description: "Component type name (e.g. 'TransformComponent', 'UnitComponent').",
          },
        },
      },
    },
    {
      name: "get_field",
      description:
        "Returns the value of a single reflected field on a component. " +
        "More efficient than get_component when you only need one value.",
      inputSchema: {
        type: "object" as const,
        required: ["id", "component", "field"],
        properties: {
          id: { type: "number", description: "Entity ID." },
          component: { type: "string", description: "Component type name." },
          field: { type: "string", description: "Field name (e.g. 'OwningPlayer', 'Transform')." },
        },
      },
    },
    {
      name: "get_features",
      description:
        "Returns all registered PhoenixSim features and their current reflected property values. " +
        "Useful for inspecting feature configuration and state.",
      inputSchema: { type: "object" as const, properties: {} },
    },
    {
      name: "spawn_unit",
      description:
        "Spawns a unit into the simulation. The action is enqueued thread-safely and executes on " +
        "the next sim tick. unit_type must match a registered LDS unit data name (e.g. 'Lancer', 'Archer', 'Tower').",
      inputSchema: {
        type: "object" as const,
        required: ["unit_type"],
        properties: {
          unit_type: { type: "string", description: "LDS unit data name (e.g. 'Lancer', 'Archer')." },
          owner:   { type: "number", description: "Player index that owns the unit (default 0)." },
          x:       { type: "number", description: "Spawn X position in world units (default 0)." },
          y:       { type: "number", description: "Spawn Y position in world units (default 0)." },
          facing:  { type: "number", description: "Facing angle in degrees (default 0)." },
        },
      },
    },
    {
      name: "issue_command",
      description:
        "Issues an order to a unit (e.g. move, attack). command_id must match a registered " +
        "ability command name (e.g. 'attack', 'move'). target_x/y set the world-space target location.",
      inputSchema: {
        type: "object" as const,
        required: ["entity_id", "command_id"],
        properties: {
          entity_id:  { type: "number", description: "Entity ID of the unit to command." },
          command_id: { type: "string", description: "Ability command name (e.g. 'attack', 'move')." },
          target_x:   { type: "number", description: "Target X position (default 0)." },
          target_y:   { type: "number", description: "Target Y position (default 0)." },
          sender:     { type: "number", description: "Player index issuing the command (default 0)." },
        },
      },
    },
    {
      name: "set_spawning",
      description: "Enables or disables the automatic unit spawner.",
      inputSchema: {
        type: "object" as const,
        required: ["enabled"],
        properties: {
          enabled: { type: "boolean", description: "True to enable spawning, false to disable." },
        },
      },
    },
    {
      name: "run_lua",
      description:
        "Enqueues a Lua code string to be executed in the sim on the next world update tick. " +
        "The script runs inside the FeatureLua state and has access to all Phoenix.* bindings: " +
        "Phoenix.Unit, Phoenix.Orders, Phoenix.Vitals, Phoenix.Physics, Phoenix.ECS, Phoenix.GetSimTime(). " +
        "The world FName is passed to all OnWorldUpdate callbacks as the 'world' argument — " +
        "capture it in OnWorldInitialize or use the world name from get_features. " +
        "Returns immediately; query game state with other tools to observe effects.",
      inputSchema: {
        type: "object" as const,
        required: ["code"],
        properties: {
          code: { type: "string", description: "Lua source code to execute." },
        },
      },
    },
  ],
}));

server.setRequestHandler(CallToolRequestSchema, async (req) => {
  const name = req.params.name;
  const args = (req.params.arguments ?? {}) as Record<string, unknown>;

  if (name === "get_stats") {
    const { data } = await api("/api/stats");
    return text(data);
  }

  if (name === "get_archetypes") {
    const { data } = await api("/api/archetypes");
    return text(data);
  }

  if (name === "list_entities") {
    const params = new URLSearchParams();
    if (args.component) params.set("component", String(args.component));
    if (args.owner !== undefined) params.set("owner", String(args.owner));
    if (args.alive) params.set("alive", "1");
    const qs = params.size ? `?${params}` : "";
    const { data } = await api(`/api/entities${qs}`);
    return text(data);
  }

  if (name === "get_entity") {
    const { data } = await api(`/api/entity/${args.id}`);
    return text(data);
  }

  if (name === "get_component") {
    const { data } = await api(`/api/entity/${args.id}/${args.component}`);
    return text(data);
  }

  if (name === "get_field") {
    const { data } = await api(`/api/entity/${args.id}/${args.component}/${args.field}`);
    return text(data);
  }

  if (name === "get_features") {
    const { data } = await api("/api/features");
    return text(data);
  }

  if (name === "spawn_unit") {
    const { data } = await post("/api/spawn", {
      unit_type: args.unit_type ?? "Lancer",
      owner:     args.owner   ?? 0,
      x:         args.x       ?? 0,
      y:         args.y       ?? 0,
      facing:    args.facing  ?? 0,
    });
    return text(data);
  }

  if (name === "issue_command") {
    const { data } = await post("/api/command", {
      entity_id:  args.entity_id,
      command_id: args.command_id,
      target_x:   args.target_x ?? 0,
      target_y:   args.target_y ?? 0,
      sender:     args.sender   ?? 0,
    });
    return text(data);
  }

  if (name === "set_spawning") {
    const { data } = await post("/api/spawning", { enabled: args.enabled });
    return text(data);
  }

  if (name === "run_lua") {
    const { data } = await post("/api/lua", { code: args.code });
    return text(data);
  }

  return {
    content: [{ type: "text" as const, text: `Unknown tool: ${name}` }],
    isError: true,
  };
});

const transport = new StdioServerTransport();
await server.connect(transport);
