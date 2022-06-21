class NO_SCR_DCP_Sabotage_US_Action : NO_SCR_MissionSelectionAction
{
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
		{
		
		super.PerformAction(pOwnerEntity, pUserEntity);
			
		//Finish initital task to select a patrol type
		IEntity taskEntityH = GetGame().GetWorld().FindEntityByName("tsk_selectPatrol_US");
		if (taskEntityH) {
        	NO_SCR_EditorTask taskH = NO_SCR_EditorTask.Cast(taskEntityH);
			taskH.ChangeStateOfTask(TriggerType.Finish);
			
		//Unlock next task to reach mission trigger
		IEntity taskEntity = GetGame().GetWorld().FindEntityByName("tsk_startPatrol_US");
        NO_SCR_EditorTask task = NO_SCR_EditorTask.Cast(taskEntity);
		task.ChangeStateOfTask(TriggerType.Assign);
			
		//Spawns sabotage trucks
        NO_SCR_SpawnTrigger.Cast(GetGame().GetWorld().FindEntityByName("sabotage_spawnTrg")).Spawn();
		}
	}

}