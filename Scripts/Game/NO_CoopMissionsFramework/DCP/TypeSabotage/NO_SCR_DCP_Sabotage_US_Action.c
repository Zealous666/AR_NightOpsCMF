class NO_SCR_DCP_Sabotage_US_Action : NO_SCR_MissionSelectionAction
{
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
		{
		
		super.PerformAction(pOwnerEntity, pUserEntity);
			
		//Finish initital task to select a patrol type
		IEntity taskEntityH = GetGame().GetWorld().FindEntityByName("tsk_SelectPatrol_US");
		if (taskEntityH) {
        	NO_SCR_EditorTask taskH = NO_SCR_EditorTask.Cast(taskEntityH);
			taskH.ChangeStateOfTask(TriggerType.Finish);
			
		//Unlock next task to reach mission trigger
		IEntity taskEntity = GetGame().GetWorld().FindEntityByName("tsk_StartPatrol_US");
        NO_SCR_EditorTask task = NO_SCR_EditorTask.Cast(taskEntity);
		task.ChangeStateOfTask(TriggerType.Assign);
			
		//Unlock typeSabotage missionTrigger
		NO_SCR_PlayerTriggerEntity trigger = 
		NO_SCR_PlayerTriggerEntity.Cast(GetGame().GetWorld().FindEntityByName("PatrolStart_typeSabotageTrg_US"));
		if(trigger)
		trigger.SetActive(true);
	
					
		//RANDOM PATROL AREA STUFF:			
		//Pick a random patrol area
			//TO-DO
		
		//spawn the enemies in picked patrol area	
		NO_SCR_SpawnTrigger.Cast(GetGame().GetWorld().FindEntityByName("factionSpawnTrg_01")).Spawn();
			
		//spawns the sabotage truck at picked patrol area
    	 IEntity sabotage1 = GetGame().GetWorld().FindEntityByName("mrk_sabotage_1_01");
        NO_SCR_EnvSpawnerComponent spawner1 = NO_SCR_EnvSpawnerComponent.Cast(sabotage1.FindComponent(NO_SCR_EnvSpawnerComponent));
        spawner1.DoSpawn();
			
	  	IEntity sabotage2 = GetGame().GetWorld().FindEntityByName("mrk_sabotage_2_01");
        NO_SCR_EnvSpawnerComponent spawner2 = NO_SCR_EnvSpawnerComponent.Cast(sabotage2.FindComponent(NO_SCR_EnvSpawnerComponent));
        spawner2.DoSpawn();
			
			
		//move sabotage tasks to picked patrol area marker
		vector sabotage1TasklLocation = GetGame().GetWorld().FindEntityByName("mrk_sabotage_1_01").GetOrigin();
        GetGame().GetWorld().FindEntityByName("tsk_TypeSabotage_1_US").SetOrigin(sabotage1TasklLocation);	
			
		vector sabotage2TasklLocation = GetGame().GetWorld().FindEntityByName("mrk_sabotage_2_01").GetOrigin();
        GetGame().GetWorld().FindEntityByName("tsk_TypeSabotage_2_US").SetOrigin(sabotage2TasklLocation);	

			
		//move infil spawnpoint to picket patrol area marker
		vector infilSpawnLocation = GetGame().GetWorld().FindEntityByName("mrk_infil_01").GetOrigin();
        GetGame().GetWorld().FindEntityByName("SpawnPoint_Infil_US").SetOrigin(infilSpawnLocation);	
			
		//move infil position to picket patrol area marker
		vector infilLocation = GetGame().GetWorld().FindEntityByName("mrk_infil_01").GetOrigin();
        GetGame().GetWorld().FindEntityByName("PatrolInfilPos_typeSabotage_US").SetOrigin(infilLocation);	
				
		//move exfil missionTrigger to picket patrol area marker
			
		}
	}

}