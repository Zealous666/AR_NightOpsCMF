[EntityEditorProps(category: "GameScripted/CombatPatrol", description: "Patrol area entity.", visible: false)]
class NO_SCR_DCP_PatrolAreaClass : CommentEntityClass
{
}


class NO_SCR_DCP_PatrolArea : CommentEntity
{
	protected ref array<NO_SCR_EnvSpawnerComponent> m_aInUseEnvSpawners = new array<NO_SCR_EnvSpawnerComponent>();
	protected ref array<NO_SCR_AISpawnerComponent> m_aInUseAiSpawners = new array<NO_SCR_AISpawnerComponent>();

	protected ref array<NO_SCR_DCP_EnvSpawnerComponent> m_aIntelEnvSpawners1;
	protected ref array<NO_SCR_DCP_EnvSpawnerComponent> m_aIntelEnvSpawners2;
	protected ref array<NO_SCR_DCP_AISpawnerComponent> m_aIntelAISpawners1;
	protected ref array<NO_SCR_DCP_AISpawnerComponent> m_aIntelAISpawners2;

	protected ref array<NO_SCR_DCP_EnvSpawnerComponent> m_aSabotageEnvSpawners1;
	protected ref array<NO_SCR_DCP_EnvSpawnerComponent> m_aSabotageEnvSpawners2;
	protected ref array<NO_SCR_DCP_AISpawnerComponent> m_aSabotageAISpawners1;
	protected ref array<NO_SCR_DCP_AISpawnerComponent> m_aSabotageAISpawners2;

	protected ref array<NO_SCR_DCP_AISpawnerComponent> m_aHVTSpawners1;

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

	protected vector m_vHVTPos1;


	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (!GetGame().GetWorldEntity())
  			return;

		ReadMarkers();

		ClearFlags(EntityFlags.ACTIVE, false);
	}


	protected void ReadMarkers()
	{
		// Find all child entities
		IEntity childEntity = GetChildren();
		while (childEntity)
		{
			// Check if child entity is a PatrolMarker
			NO_SCR_DCP_PatrolMarker patrolMarker = NO_SCR_DCP_PatrolMarker.Cast(childEntity);
			if (patrolMarker)
			{
				// Consume the given patrol markers state (origin/rotation)
				switch (patrolMarker.GetMarkerType())
				{
					case ENightOpsPatrolMarkerType.INFIL: { m_vInfilPos = patrolMarker.GetOrigin(); m_vInfilRot = patrolMarker.GetAngles(); break; }
					case ENightOpsPatrolMarkerType.EXFIL: { m_vExfilPos = patrolMarker.GetOrigin(); break; }
					case ENightOpsPatrolMarkerType.INTEL_1: { m_aIntelEnvSpawners1 = patrolMarker.GetEnvSpawners(); m_aIntelAISpawners1 = patrolMarker.GetAISpawners(); m_vIntelPos1 = patrolMarker.GetOrigin(); m_vIntelRot1 = patrolMarker.GetAngles(); break; }
					case ENightOpsPatrolMarkerType.INTEL_2: { m_aIntelEnvSpawners2 = patrolMarker.GetEnvSpawners(); m_aIntelAISpawners2 = patrolMarker.GetAISpawners(); m_vIntelPos2 = patrolMarker.GetOrigin(); m_vIntelRot2 = patrolMarker.GetAngles(); break; }
					case ENightOpsPatrolMarkerType.SABOTAGE_1: { m_aSabotageEnvSpawners1 = patrolMarker.GetEnvSpawners(); m_aSabotageAISpawners1 = patrolMarker.GetAISpawners(); m_vSabotagePos1 = patrolMarker.GetOrigin(); m_vSabotageRot1 = patrolMarker.GetAngles(); break; }
					case ENightOpsPatrolMarkerType.SABOTAGE_2: { m_aSabotageEnvSpawners2 = patrolMarker.GetEnvSpawners(); m_aSabotageAISpawners2 = patrolMarker.GetAISpawners(); m_vSabotagePos2 = patrolMarker.GetOrigin(); m_vSabotageRot2 = patrolMarker.GetAngles(); break; }
					case ENightOpsPatrolMarkerType.HVT: { m_aHVTSpawners1 = patrolMarker.GetAISpawners(); m_vHVTPos1 = patrolMarker.GetOrigin(); break; }
					default: { Print(string.Format("Unconfigured patrol marker detected: ", patrolMarker.GetName())); break; }
				}
			}
			childEntity = childEntity.GetSibling();
		}
	}


	void StartPatrolType(ENightOpsPatrolType patrolType, NO_SCR_DCP_PatrolAssetsConfig patrolAssets)
	{
		NO_SCR_DCP_PatrolManager patMan = GetPatrolManager();

		IEntity baseSpawnpoint = GetGame().GetWorld().FindEntityByName(patrolAssets.BaseSpawnpoint);
		if (!baseSpawnpoint)
		{
			Print(string.Format("Unable to find spawnpoint named '%1', cannot continue!", patrolAssets.BaseSpawnpoint), LogLevel.ERROR);
			return;
		}

		// Spawn FOB at infil location
		SpawnEnv(patrolAssets.FOBSpawner, m_vInfilPos, m_vInfilRot);

		// Move infil spawnpoint
		TryMoveEntity(patrolAssets.InfilSpawnpoint, m_vInfilPos, m_vInfilRot);

		// Move infil trigger / teleport
		TryMoveEntity(patrolAssets.InfilTrigger, baseSpawnpoint.GetOrigin(), vector.Zero);
		TryMoveEntity(patrolAssets.InfilTeleportPoint, m_vInfilPos, m_vInfilRot);

		// Move exfil trigger / teleport
		TryMoveEntity(patrolAssets.ExfilTrigger, m_vExfilPos, vector.Zero);
		TryMoveEntity(patrolAssets.ExfilTeleportPoint, baseSpawnpoint.GetOrigin(), baseSpawnpoint.GetAngles());

		// Setup for selected patrol type
		if (patrolType == ENightOpsPatrolType.INTEL)
		{
			// Spawn intel 1 prefab + guards
			SpawnEnv(m_aIntelEnvSpawners1);
			SpawnAI(m_aIntelAISpawners1);

			// Spawn intel 2 prefab + guards
			SpawnEnv(m_aIntelEnvSpawners2);
			SpawnAI(m_aIntelAISpawners2);

			// Move intel task locations
	        TryMoveEntity(patMan.GetPatrolTask(ENightOpsPatrolTasks.INTEL_OBJ_1), m_vIntelPos1, vector.Zero);
			TryMoveEntity(patMan.GetPatrolTask(ENightOpsPatrolTasks.INTEL_OBJ_2), m_vIntelPos2, vector.Zero);
		}
		else if (patrolType == ENightOpsPatrolType.SABOTAGE)
		{
			SpawnEnv(m_aSabotageEnvSpawners1);
			SpawnAI(m_aSabotageAISpawners1);

			SpawnEnv(m_aSabotageEnvSpawners2);
			SpawnAI(m_aSabotageAISpawners2);

			TryMoveEntity(patMan.GetPatrolTask(ENightOpsPatrolTasks.SABOTAGE_OBJ_1), m_vSabotagePos1, vector.Zero);
			TryMoveEntity(patMan.GetPatrolTask(ENightOpsPatrolTasks.SABOTAGE_OBJ_2), m_vSabotagePos2, vector.Zero);
		}
		else if (patrolType == ENightOpsPatrolType.HVT)
		{
			SpawnAI(m_aHVTSpawners1);

			TryMoveEntity(patMan.GetPatrolTask(ENightOpsPatrolTasks.HVT_MAIN), m_vHVTPos1, vector.Zero);
		}
	}


	void EndPatrol()
	{
		// Despawn any objective assets
		foreach (NO_SCR_EnvSpawnerComponent envSpawner : m_aInUseEnvSpawners)
			envSpawner.RemoveSpawned();

		foreach (NO_SCR_AISpawnerComponent aiSpawner : m_aInUseAiSpawners)
			aiSpawner.RemoveSpawned();

		m_aInUseEnvSpawners.Clear();
		m_aInUseAiSpawners.Clear();
	}


	protected void SpawnEnv(array<NO_SCR_DCP_EnvSpawnerComponent> spawnerComponents)
	{
		foreach (NO_SCR_DCP_EnvSpawnerComponent spawnerComponent : spawnerComponents)
		{
			if (spawnerComponent.GetDCPFactionKey() == GetPatrolManager().GetPlayerFactionKey() ||
				spawnerComponent.GetDCPFactionKey() == "ANY")
			{
				spawnerComponent.DoSpawn();
				m_aInUseEnvSpawners.Insert(spawnerComponent);
			}
		}
	}


	protected void SpawnEnv(string envSpawnerName, vector location, vector rotation)
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


	protected void SpawnAI(array<NO_SCR_DCP_AISpawnerComponent> spawnerComponents)
	{
		foreach (NO_SCR_DCP_AISpawnerComponent spawnerComponent : spawnerComponents)
		{
			if (spawnerComponent.GetDCPFactionKey() == GetPatrolManager().GetPlayerFactionKey() ||
				spawnerComponent.GetDCPFactionKey() == "ANY")
			{
				spawnerComponent.DoSpawn();
				m_aInUseAiSpawners.Insert(spawnerComponent);
			}
		}
	}


	protected bool TryMoveEntity(string enitityName, vector position, vector rotation)
	{
		IEntity entity = GetGame().GetWorld().FindEntityByName(enitityName);
		if (entity)
			return TryMoveEntity(entity, position, rotation);
		else
			return false;
	}


	protected bool TryMoveEntity(IEntity entity, vector position, vector rotation)
	{
		if (entity)
		{
			entity.SetAngles(rotation);
        	entity.SetOrigin(position);
			entity.Update();
			return true;
		}
		return false;
	}


	// Override CommentEntity attributes (WB only), avoids the need for manual settings or prefabs
	void NO_SCR_DCP_PatrolArea(IEntitySource src, IEntity parent)
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