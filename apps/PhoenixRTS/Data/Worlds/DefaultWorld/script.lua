local TARGET_UNIT_COUNT  = 4000   -- total units to spawn before stopping
local SPAWN_PER_FRAME    = 50     -- units spawned per sim tick until target is reached
local SPAWN_RADIUS       = 80.0
local ROTATION_SPEED     = 2 * math.pi
local waveIndex          = 0
local spawnEnabled       = false
local towerUnit
local angle1             = 0.0

function OnWorldInitialize()
    towerUnit = Phoenix.Unit.SpawnUnit("Tower", 0, {X=0.0, Y=0.0}, 0.0)
    print(string.format("Spawned Tower: %d", towerUnit._id))
    spawnEnabled = true
end

function OnWorldShutdown()
    spawnEnabled = false
end

function OnWorldUpdate()
    if not spawnEnabled then return end

    local dt = Phoenix.DeltaTime
    angle1 = angle1 + ROTATION_SPEED * dt

    Phoenix.Debug.DrawRay({0.0, 0.0}, Phoenix.Vec2.FromPolar(angle1, 10), {R=255, G=0, B=0, A=255})

    if waveIndex < TARGET_UNIT_COUNT then
        local batch = math.min(SPAWN_PER_FRAME, TARGET_UNIT_COUNT - waveIndex)
        for i = 1, batch do
            waveIndex = waveIndex + 1
            local owner = (waveIndex % 9) + 1
            local angle = math.random() * 2 * math.pi
            local pos   = Phoenix.Vec2.FromPolar(angle, math.random() * SPAWN_RADIUS)
            local unit  = Phoenix.Unit.SpawnUnit("Archer", owner, pos, 0.0)
            local command = Phoenix.RTS.Command.new
            {
                Sender         = 0,
                TargetEntity   = towerUnit._id,
                TargetLocation = { X=0.0, Y=0.0 },
                CommandId      = "AttackAbility"
            }
            unit:IssueCommand(command)
        end
        if waveIndex >= TARGET_UNIT_COUNT then
            print(string.format("All %d units spawned", TARGET_UNIT_COUNT))
        end
    end
end

function OnPreUpdate()        end
function OnUpdate()           end
function OnPostUpdate()       end
function OnPreWorldUpdate()   end
function OnPostWorldUpdate()  end
