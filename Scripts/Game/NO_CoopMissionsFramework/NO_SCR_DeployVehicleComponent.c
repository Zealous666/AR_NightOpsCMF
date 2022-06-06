//Script bei Joe Malley to allow a vehicle to deploy a spawn point
//Adjusted and simplified by Zeal

[ComponentEditorProps(category: "Scripts/Game/Gamemode", description: "")]
class NO_SCR_DeployVehicleComponentClass: ScriptComponentClass{
	
};

class NO_SCR_DeployVehicleComponent: ScriptComponent{
	
	[Attribute("0", UIWidgets.CheckBox, "Should the prefab spawn on start")];
	bool spawnAtStart;
	
	[Attribute("0", UIWidgets.CheckBox, "MHQ Faction")];
	protected string factionKey;
	
	[Attribute("", UIWidgets.ResourcePickerThumbnail, desc: "Spawnpoint prefab to use",  params: "et")]
	protected ResourceName m_rnPrefab;
	
	[Attribute("", UIWidgets.EditBox, "Offset from vehicle origin, that spawn should be placed")]
	protected vector m_vSpawnPosition;
	
	protected SCR_SpawnPoint spawnpoint;
	void Setup(IEntity owner){
		this.spawnpoint = createSpawnPoint();

		if(spawnAtStart){
			spawnpoint.SetFactionKey(factionKey);
		}else{
			spawnpoint.SetFactionKey("Inactive");
		}
	}
	bool toggleDeployState(bool state){

		
		if(state == true){
			this.spawnpoint = createSpawnPoint();
			removeFuel();
			SCR_HintManagerComponent hintComponent = SCR_HintManagerComponent.GetInstance();
			hintComponent.ShowCustomHint("Command truck is deployed. Spawnpoint activated. Movement disabled.", "Vehicle deployed", 10);
			return true;
		}else{
			RplComponent.DeleteRplEntity(this.spawnpoint, false);
			returnFuel();
			SCR_HintManagerComponent hintComponent = SCR_HintManagerComponent.GetInstance();
			hintComponent.ShowCustomHint("Command truck is un-deployed. Spawnpoint deactivated. Movement enabled.", "Vehicle un-deployed", 10);
			return false;
		}
	}
	
	void removeFuel(){
		FuelManagerComponent manager = FuelManagerComponent.Cast(GetOwner().FindComponent(FuelManagerComponent));
		array<BaseFuelNode> nodes = {};
		manager.GetFuelNodesList(nodes);
		
		foreach(BaseFuelNode i : nodes){
			SCR_FuelNode scrNode = SCR_FuelNode.Cast(i);
			if(scrNode){
				scrNode.SetFuel(0.0);
			}
		}
	}
	
	void returnFuel(){
		FuelManagerComponent manager = FuelManagerComponent.Cast(GetOwner().FindComponent(FuelManagerComponent));
		array<BaseFuelNode> nodes = {};
		manager.GetFuelNodesList(nodes);
		foreach(BaseFuelNode i : nodes){
			SCR_FuelNode scrNode = SCR_FuelNode.Cast(i);
			if(scrNode){
				scrNode.SetFuel(scrNode.GetMaxFuel());
			}
		}
	}
	string getFactionKey(){
		return factionKey;
	}
	
	SCR_SpawnPoint createSpawnPoint(){
		
		BaseWorld myWorld = GetOwner().GetWorld();
		if (!myWorld || m_rnPrefab.IsEmpty())
			return null;
		
		Resource res = Resource.Load(m_rnPrefab);
		EntitySpawnParams params();
		
		vector mat[4];
		GetOwner().GetWorldTransform(mat);
		mat[3] = mat[3] + m_vSpawnPosition;
		params.Transform = mat;
		IEntity newEnt = GetGame().SpawnEntityPrefab(res, myWorld, params);
		if (!newEnt)
			return null;
		newEnt.SetFlags(EntityFlags.VISIBLE, true);
		
		return SCR_SpawnPoint.Cast(newEnt);
	}
};