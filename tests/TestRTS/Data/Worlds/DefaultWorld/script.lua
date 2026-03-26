local SPAWN_INTERVAL = 30
local frameCount     = 0
local spawnEnabled   = false

function OnWorldInitialize()
    Phoenix.Unit.SpawnUnit("Tower", 0, {X=0.0, Y=0.0}, 0.0)
    spawnEnabled = true
end

function OnWorldShutdown()
    spawnEnabled = false
    frameCount   = 0
end

function OnWorldUpdate()
    if not spawnEnabled then return end

    frameCount = frameCount + 1

    if frameCount % SPAWN_INTERVAL == 0 then
        local wave = math.floor(frameCount / SPAWN_INTERVAL)
        local unitData = (wave % 2 == 0) and "Lancer" or "Archer"
        local owner = (wave % 9) + 1
        Phoenix.Unit.SpawnUnit(unitData, owner, {X=0.0, Y=0.0}, 0.0)
    end
end

function OnPreUpdate()        end
function OnUpdate()           end
function OnPostUpdate()       end
function OnPreWorldUpdate()   end
function OnPostWorldUpdate()  end
