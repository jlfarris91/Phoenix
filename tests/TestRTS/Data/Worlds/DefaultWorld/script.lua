local SPAWN_INTERVAL = 30
local frameCount     = 0
local spawnEnabled   = false
local towerUnit
local angle1 = 0.0

function OnWorldInitialize()
    towerUnit = Phoenix.Unit.SpawnUnit("Tower", 0, {X=0.0, Y=0.0}, 0.0)
    print(string.format("Spawned Tower: %d", towerUnit))
    spawnEnabled = true
end

function OnWorldShutdown()
    spawnEnabled = false
    frameCount   = 0
end

function OnWorldUpdate()
    if not spawnEnabled then return end

    frameCount = frameCount + 1

    angle1 = angle1 + 0.01

    Phoenix.FeatureDebug.DrawRay({0.0, 0.0} , Phoenix.Vec2.FromPolar(angle1, 10), {R=255, G=0, B=0, A=255})

    if frameCount % SPAWN_INTERVAL == 0 then
        local wave = math.floor(frameCount / SPAWN_INTERVAL)
        local unitData = (wave % 2 == 0) and "Lancer" or "Archer"
        local owner = 1 -- (wave % 9) + 1
        local angle = math.random() * 2 * math.pi
        local pos = Phoenix.Vec2.FromPolar(angle, 10.0)

        local unit = Phoenix.Unit.SpawnUnit(unitData, owner, pos, 0.0)
        print(string.format("Spawned Unit: %d", unit))

        local command = Phoenix.RTS.Command.new
        {
            Sender = 0,
            TargetEntity = towerUnit,
            TargetLocation = { X=0.0, Y=0.0},
            CommandId = "AttackAbility"
        }
        Phoenix.Orders.IssueCommand(unit, command)
    end
end

function OnPreUpdate()        end
function OnUpdate()           end
function OnPostUpdate()       end
function OnPreWorldUpdate()   end
function OnPostWorldUpdate()  end
