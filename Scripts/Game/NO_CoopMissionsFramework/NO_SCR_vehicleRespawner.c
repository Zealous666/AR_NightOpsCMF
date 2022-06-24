//Custom script by Zeal & Biscuit
//Can spawn a vehicle at its own position or, when attached to another object, at that object even when it moves

[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class NO_SCR_VehicleRespawnerClass : GenericEntityClass
{
}

class NO_SCR_VehicleRespawner : GenericEntity
{
    [Attribute("", UIWidgets.ResourcePickerThumbnail, desc: "Vehicle prefab to spawn (usual the same)",  params: "et")]
    protected ResourceName m_rnSpawnPrefab;

	[Attribute("10000", UIWidgets.Auto, desc: "Respawn delay in milliseconds (1000 = 1 second")];
    protected int m_iSpawnDelay;

	[Attribute("1", UIWidgets.CheckBox, "Should the player get a hint notification on respawn?")];
	bool showHint;


    override void EOnInit(IEntity owner)
    {
        if (!GetGame().InPlayMode())
            return;

		// Next frame
        GetGame().GetCallqueue().Call(SpawnVehicle);
    }

    protected bool TryRegister(IEntity targetEntity, string eventName, func callback)
    {
        EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(targetEntity.FindComponent(EventHandlerManagerComponent));
        if (eventHandlerManager)
        {
            eventHandlerManager.RegisterScriptHandler(eventName, targetEntity, callback);
            return true;
        }
        else
        	return false;
    }

    protected bool TryUnregister(IEntity targetEntity, string eventName, func callback)
    {
        EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(targetEntity.FindComponent(EventHandlerManagerComponent));
        if (eventHandlerManager)
        {
            eventHandlerManager.RemoveScriptHandler(eventName, targetEntity, callback);
            return true;
        }
        else
	         return false;
    }

  	void OnVehicleDeystroyed(IEntity targetEntity)
    {
        if (targetEntity)
        {
			TryUnregister(targetEntity, "OnDestroyed", OnVehicleDeystroyed);
			TryUnregister(targetEntity, "OnCompartmentLeft", OnVehicleCompartmentLeft);
		}

        // Delay the vehicle spawn, argument should be ms so 5000 = 5s
        GetGame().GetCallqueue().CallLater(SpawnVehicle, m_iSpawnDelay);
    }

	protected void OnVehicleCompartmentLeft(IEntity vehicle, BaseCompartmentManagerComponent manager, IEntity occupant, int managerID, int slotID)
	{
		GarbageManager garbageManager = GetGame().GetGarbageManager();
		if (garbageManager)
			if (garbageManager.IsInserted(vehicle))
				garbageManager.Withdraw(vehicle);
	}

	protected void PrintInserted(GarbageManager garbageManager, IEntity spawnedVehicle)
	{
		Print(garbageManager, LogLevel.ERROR);
		Print(garbageManager.IsInserted(spawnedVehicle));
	}

    protected void SpawnVehicle()
    {
		// Load prefab
        Resource prefab = Resource.Load(m_rnSpawnPrefab);
        if (!prefab)
        {
            Print(string.Format("SCR_VehicleRespawnComponent could not load '%1'", prefab));
            return;
        }

		// Spawn location stuff
        EntitySpawnParams spawnParams = new EntitySpawnParams();
        spawnParams.TransformMode = ETransformMode.WORLD;

		vector mat[4];
     	GetWorldTransform(mat);
        GetSpawnTransform(spawnParams.Transform);
		spawnParams.Transform = mat;

		// Spawn vehicle
        IEntity spawnedVehicle = GetGame().SpawnEntityPrefab(prefab, GetGame().GetWorld(), spawnParams);
        if (!spawnedVehicle)
        {
            Print(string.Format("SCR_VehicleRespawnComponent could not spawn '%1'", prefab));
            return;
        }

		// Event registration to detect spawned vehicles death
        TryRegister(spawnedVehicle, "OnDestroyed", OnVehicleDeystroyed);

		// If using a garbageManager, make sure an exited vehicle is unset for collection.
		// A deystroyed vehicle can still be collected.
		GarbageManager garbageManager = GetGame().GetGarbageManager();
		if (garbageManager)
		{
			EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(spawnedVehicle.FindComponent(EventHandlerManagerComponent));
			if (eventHandlerManager)
	        	TryRegister(spawnedVehicle, "OnCompartmentLeft", OnVehicleCompartmentLeft);

			GetGame().GetCallqueue().CallLater(PrintInserted, 1000, true, garbageManager, spawnedVehicle);
		}

		// Hint
		if (showHint)
			SCR_HintManagerComponent.GetInstance().ShowCustomHint("The vehicle has been destroyed and is now respawning...", "Vehicle Respawned", 10);
	}

    protected void GetSpawnTransform(out vector transformMatrix[4])
    {
        vector rotation = vector.Zero;
        vector yawPitchRoll = Vector(rotation[1], rotation[0], rotation[2]);
        Math3D.AnglesToMatrix(rotation, transformMatrix);
        transformMatrix[3] = GetOrigin();
    }

    void SCR_VehicleRespawner(IEntitySource src, IEntity parent)
    {
        SetEventMask(EntityEvent.INIT);
    }
}