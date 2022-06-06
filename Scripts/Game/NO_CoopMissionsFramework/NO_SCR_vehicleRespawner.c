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
 
    protected IEntity m_pLastSpawnedVehicle;
    
	
    override void EOnInit(IEntity owner)
    {
        if (!GetGame().InPlayMode())
            return;
 
        GetGame().GetCallqueue().CallLater(QueryLater, 0);
    }
    
    void QueryLater()
    {
        
        SpawnVehicle();
    }
    
    protected bool TryRegister(IEntity targetEntity)
    {
        EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(targetEntity.FindComponent(EventHandlerManagerComponent));
        if (eventHandlerManager)
        {
            eventHandlerManager.RegisterScriptHandler("OnDestroyed", targetEntity, OnVehicleDeystroyed);
            return true;
        }
        else
        {
            return false;
        }
    }
    
    protected bool TryUnregister(IEntity targetEntity)
    {
        EventHandlerManagerComponent eventHandlerManager = EventHandlerManagerComponent.Cast(targetEntity.FindComponent(EventHandlerManagerComponent));
        if (eventHandlerManager)
        {
            eventHandlerManager.RemoveScriptHandler("OnDestroyed", targetEntity, OnVehicleDeystroyed);
            return true;
        }
        else
        {
            return false;
        }
    }
	
    
  	void OnVehicleDeystroyed(IEntity targetEntity)
    {
        if (m_pLastSpawnedVehicle)
            TryUnregister(m_pLastSpawnedVehicle);
        
        //SpawnVehicle();
        // Delay the vehicle spawn, argument should be ms so 5000 = 5s
        GetGame().GetCallqueue().CallLater(SpawnVehicle, m_iSpawnDelay);
    }
    
    protected void SpawnVehicle()
    {
	
        Resource prefab = Resource.Load(m_rnSpawnPrefab);
        if (!prefab)
        {
            Print(string.Format("SCR_VehicleRespawnComponent could not load '%1'", prefab));
            return;
        }
        
        EntitySpawnParams spawnParams = new EntitySpawnParams();
        spawnParams.TransformMode = ETransformMode.WORLD;
		
		vector mat[4];
     	GetWorldTransform(mat);
        GetSpawnTransform(spawnParams.Transform);
		spawnParams.Transform = mat;
 
        IEntity spawnedVehicle = GetGame().SpawnEntityPrefab(prefab, GetGame().GetWorld(), spawnParams);
        if (!spawnedVehicle)
        {
            Print(string.Format("SCR_VehicleRespawnComponent could not spawn '%1'", prefab));
            return;
        }
        
        TryRegister(spawnedVehicle);
		
		if (showHint)
		{
			SCR_HintManagerComponent hintComponent = SCR_HintManagerComponent.GetInstance();
			hintComponent.ShowCustomHint("The vehicle has been destroyed and is now respawning...", "Vehicle Respawned", 10);
		}
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
    
    void ~NO_SCR_VehicleRespawner()
    {
    }
 
}