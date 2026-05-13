local SPAWN_INTERVAL     = 0.1
local ROTATION_SPEED     = 2 * math.pi  -- one full revolution per second of sim time
local spawnTimer         = 0
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
    spawnTimer = spawnTimer + dt
    angle1 = angle1 + ROTATION_SPEED * dt

    Phoenix.Debug.DrawRay({0.0, 0.0}, Phoenix.Vec2.FromPolar(angle1, 10), {R=255, G=0, B=0, A=255})

    if spawnTimer > SPAWN_INTERVAL then
        spawnTimer = spawnTimer - SPAWN_INTERVAL
        waveIndex = waveIndex + 1

        local unitData = (waveIndex % 2 == 0) and "Lancer" or "Archer"
        local owner = (waveIndex % 9) + 1
        local angle = math.random() * 2 * math.pi
        local pos = Phoenix.Vec2.FromPolar(angle, 10.0)

        local unit = Phoenix.Unit.SpawnUnit(unitData, owner, pos, 0.0)
        print(string.format("Spawned Unit: %d", unit._id))

        local command = Phoenix.RTS.Command.new
        {
            Sender = 0,
            TargetEntity = towerUnit._id,
            TargetLocation = { X=0.0, Y=0.0},
            CommandId = "AttackAbility"
        }
        unit:IssueCommand(command)
    end
end

function OnPreUpdate()        end
function OnUpdate()           end
function OnPostUpdate()       end
function OnPreWorldUpdate()   end
function OnPostWorldUpdate()  end
