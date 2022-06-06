//Script by Joe Malley to deploy or undeploy a vehicle unlocking a respawn spanwpoint
//Modified and simplified by Zeal

class NO_SCR_Deploy_Action : ScriptedUserAction
{
	bool stateM = false;
	
	override event bool GetActionNameScript(out string outName){

		if(!stateM){
			outName = "Deploy Command Vehicle" ;
		}else{
			outName = "Undeploy Command Vehicle" ;
		}
		return true;
		//return false; 
	};
	override event bool GetActionDescriptionScript(out string outName) { 
		if(!stateM){
			outName = "(Removes Fuel)" ;
		}else{
			outName = "(Returns Fuel)" ;
		}
		return true;
	};
	override event void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent){
		stateM = false;
	}
	override event void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity){
		stateM = NO_SCR_DeployVehicleComponent.Cast(pOwnerEntity.FindComponent(NO_SCR_DeployVehicleComponent)).toggleDeployState(!stateM);
		
	}
};