[EntityEditorProps(category: "GameScripted/CombatPatrol", description: "Patrol area entity.", visible: false)]
class NO_SCR_PatrolAreaClass : CommentEntityClass
{
}

class NO_SCR_PatrolArea : CommentEntity
{
	protected ref array<NO_SCR_EnvSpawnerComponent> m_aInUseEnvSpawners = new array<NO_SCR_EnvSpawnerComponent>();

	protected ENightOpsPatrolType m_eLastUsedPatrolType;

	protected vector m_vInfilPos;
	protected vector m_vInfilRot;
	protected vector m_vExfilPos;

	protected vector m_vIntelPos1;
	protected vector m_vIntelPos2;
	protected vector m_vIntelRot1;
	protected vector m_vIntelRot2;

	protected vector m_vSabotagePos1;
	protected vector m_vSabotagePos2;
	protected vector m_vSabotageRot1;
	protected vector m_vSabotageRot2;

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (!GetGame().GetWorldEntity())
  			return;

		ReadMarkers();

		ClearFlags(EntityFlags.ACTIVE, false);
	}

	//------------------------------------------------------------------------------------------------

	protected void ReadMarkers()
	{
		// Find all child entities
		IEntity childEntity = GetChildren();
		while (childEntity)
		{
			// Check if child entity is a PatrolMarker
			NO_SCR_PatrolMarker patrolMarker = NO_SCR_PatrolMarker.Cast(childEntity);
			if (patrolMarker)
			{
				// Consume the given patrol markers state (origin/rotation)
				switch (patrolMarker.GetMarkerType())
				{
					case ENightOpsPatrolMarkerType.INFIL: { m_vInfilPos = patrolMarker.GetOrigin(); m_vInfilRot = patrolMarker.GetAngles(); break; }
					case ENightOpsPatrolMarkerType.EXFIL: { m_vExfilPos = patrolMarker.GetOrigin(); break; }
					case ENightOpsPatrolMarkerType.INTEL_1: { m_vIntelPos1 = patrolMarker.GetOrigin(); m_vIntelRot1 = patrolMarker.GetAngles(); break; }
					case ENightOpsPatrolMarkerType.INTEL_2: { m_vIntelPos2 = patrolMarker.GetOrigin(); m_vIntelRot2 = patrolMarker.GetAngles(); break; }
					case ENightOpsPatrolMarkerType.SABOTAGE_1: { m_vSabotagePos1 = patrolMarker.GetOrigin(); m_vSabotageRot1 = patrolMarker.GetAngles(); break; }
					case ENightOpsPatrolMarkerType.SABOTAGE_2: { m_vSabotagePos2 = patrolMarker.GetOrigin(); m_vSabotageRot2 = patrolMarker.GetAngles(); break; }
					default: { Print(string.Format("Unconfigured patrol marker detected: ", patrolMarker.GetName())); break; }
				}
			}

			childEntity = childEntity.GetSibling();
		}
	}

	bool StartPatrolType(ENightOpsPatrolType patrolType)
	{
		m_eLastUsedPatrolType = patrolType;

		// TEMPORARY TASK STUFF
		//Finish initital task to select a patrol type
		IEntity taskEntityH = GetGame().GetWorld().FindEntityByName("SelectPatrol_Task_US");
		if (taskEntityH)
		{
        	NO_SCR_EditorTask taskH = NO_SCR_EditorTask.Cast(taskEntityH);
			taskH.ChangeStateOfTask(TriggerType.Finish);
		}

		//Unlock next task to reach mission trigger
		IEntity taskEntity = GetGame().GetWorld().FindEntityByName("StartPatrol_Task_US");
        NO_SCR_EditorTask task = NO_SCR_EditorTask.Cast(taskEntity);
		task.ChangeStateOfTask(TriggerType.Assign);

		NO_SCR_PlayerTriggerEntity trigger = NO_SCR_PlayerTriggerEntity.Cast(GetGame().GetWorld().FindEntityByName("infil_trigger"));
		if (trigger)
			trigger.SetActive(true);

		// Move infil spawnpoint
		IEntity infilSpawnpoint = GetGame().GetWorld().FindEntityByName("infil_spawnpoint");
		if (infilSpawnpoint)
		{
			infilSpawnpoint.SetAngles(m_vInfilRot);
        	infilSpawnpoint.SetOrigin(m_vInfilPos);
		}

		// Move infil teleport
		IEntity infilTeleportPoint = GetGame().GetWorld().FindEntityByName("infil_tp_point");
		if (infilTeleportPoint)
		{
			infilTeleportPoint.SetAngles(m_vInfilRot);
        	infilTeleportPoint.SetOrigin(m_vInfilPos);
		}

		// Move exfil trigger
		IEntity exfilTrigger = GetGame().GetWorld().FindEntityByName("exfil_trigger");
		if (exfilTrigger)
		{
        	exfilTrigger.SetOrigin(m_vExfilPos);
		}

		// Move exfil teleport
		IEntity exfilTeleportPoint = GetGame().GetWorld().FindEntityByName("exfil_tp_point");
		if (exfilTeleportPoint)
		{
			IEntity baseSpawnpoint = GetGame().GetWorld().FindEntityByName("SpawnPoint_Base_US");
			if (baseSpawnpoint)
        	{
				exfilTeleportPoint.SetAngles(baseSpawnpoint.GetAngles());
				exfilTeleportPoint.SetOrigin(baseSpawnpoint.GetOrigin());
			}
		}

		// Setup patrol type
		if (patrolType == ENightOpsPatrolType.INTEL)
		{
			// Spawn intel 1 prefab
			SpawnPatrolObjective("intel_spawner_1", m_vIntelPos1, m_vIntelRot1);

			// Spawn intel 2 prefab
			SpawnPatrolObjective("intel_spawner_2", m_vIntelPos2, m_vIntelRot2);

			// Move intel task locations
	        GetGame().GetWorld().FindEntityByName("Intel_1_Task_US").SetOrigin(m_vIntelPos1);
	        GetGame().GetWorld().FindEntityByName("Intel_2_Task_US").SetOrigin(m_vIntelPos2);
		}
		else if (patrolType == ENightOpsPatrolType.SABOTAGE)
		{
			// Spawn sabotage 1 prefab
			SpawnPatrolObjective("sabotage_spawner_1", m_vSabotagePos1, m_vSabotageRot1);

			// Spawn sabotage 2 prefab
			SpawnPatrolObjective("sabotage_spawner_2", m_vSabotagePos2, m_vSabotageRot2);

			// Move sabotage task locations
	        GetGame().GetWorld().FindEntityByName("Sabotage_1_Task_US").SetOrigin(m_vSabotagePos1);
	        GetGame().GetWorld().FindEntityByName("Sabotage_2_Task_US").SetOrigin(m_vSabotagePos2);
		}
		else if (patrolType == ENightOpsPatrolType.HVT)
		{
			// TODO
		}

		return true;
	}

	protected void SpawnPatrolObjective(string envSpawnerName, vector location, vector rotation)
	{
		IEntity spawnerEntity = GetGame().GetWorld().FindEntityByName(envSpawnerName);

		if (spawnerEntity)
		{
			spawnerEntity.SetAngles(rotation);
			spawnerEntity.SetOrigin(location);

			NO_SCR_EnvSpawnerComponent spawnerComponent = NO_SCR_EnvSpawnerComponent.Cast(spawnerEntity.FindComponent(NO_SCR_EnvSpawnerComponent));
        	if (spawnerComponent)
			{
				spawnerEntity.GetTransform(spawnerComponent.parentVector);
				spawnerComponent.DoSpawn();
				m_aInUseEnvSpawners.Insert(spawnerComponent);
			}
		}
	}

	void AssignTasks()
	{
		if (m_eLastUsedPatrolType == ENightOpsPatrolType.INTEL)
		{
			NO_SCR_EditorTask task1 = NO_SCR_EditorTask.Cast(GetGame().GetWorld().FindEntityByName("Intel_1_Task_US"));
	        NO_SCR_EditorTask task2 = NO_SCR_EditorTask.Cast(GetGame().GetWorld().FindEntityByName("Intel_2_Task_US"));
	        task1.ChangeStateOfTask(TriggerType.Assign);
			task2.ChangeStateOfTask(TriggerType.Create);
		}
		else if (m_eLastUsedPatrolType == ENightOpsPatrolType.SABOTAGE)
		{
	        NO_SCR_EditorTask task1 = NO_SCR_EditorTask.Cast(GetGame().GetWorld().FindEntityByName("Sabotage_1_Task_US"));
	        NO_SCR_EditorTask task2 = NO_SCR_EditorTask.Cast(GetGame().GetWorld().FindEntityByName("Sabotage_2_Task_US"));
			task1.ChangeStateOfTask(TriggerType.Assign);
			task2.ChangeStateOfTask(TriggerType.Create);
		}
		else if (m_eLastUsedPatrolType == ENightOpsPatrolType.HVT)
		{
	        NO_SCR_EditorTask task = NO_SCR_EditorTask.Cast(GetGame().GetWorld().FindEntityByName("HVT_Task_US"));
			task.ChangeStateOfTask(TriggerType.Assign);
		}
	}

	void EndPatrol()
	{
		foreach (NO_SCR_EnvSpawnerComponent envSpawner : m_aInUseEnvSpawners)
			envSpawner.RemoveSpawned();

		m_aInUseEnvSpawners.Clear();

		NO_SCR_PlayerTriggerEntity infilTrigger = NO_SCR_PlayerTriggerEntity.Cast(GetGame().GetWorld().FindEntityByName("infil_trigger"));
		if (infilTrigger)
			infilTrigger.SetActive(false);

		NO_SCR_PlayerTriggerEntity exfilTrigger = NO_SCR_PlayerTriggerEntity.Cast(GetGame().GetWorld().FindEntityByName("exfil_trigger"));
		if (exfilTrigger)
			exfilTrigger.SetActive(false);
	}

	//------------------------------------------------------------------------------------------------

	void NO_SCR_PatrolArea(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		SetFlags(EntityFlags.STATIC, true);

		#ifdef WORKBENCH
		m_Comment = "PATROL: " + GetName();
		m_Color = "0.13 0.13 0.13 1.0";
		m_Size = 0.15;
		m_ScaleByDistance = true;
		m_FaceCamera = true;
		m_VisibleOverall = true;
		#endif
	}
}