[EntityEditorProps(category: "GameScripted/CombatPatrol", description: "Patrol manager entity.", visible: false)]
class NO_SCR_DCP_PatrolManagerClass : GenericEntityClass
{
}


NO_SCR_DCP_PatrolManager g_PatrolManagerInstance;
NO_SCR_DCP_PatrolManager GetPatrolManager()
{
	return g_PatrolManagerInstance;
}


class NO_SCR_DCP_PatrolManager : GenericEntity
{
	[Attribute("US", UIWidgets.EditBox, desc: "Player faction key, this is overriden by MissionHeader values (use for editor testing)!", category: "COMBAT PATROLS")]
	protected string m_sPlayerFactionKey;

	[Attribute("", UIWidgets.Object, desc: "Change items within config file, not through Object Properties!", category: "COMBAT PATROLS", params: "noDetails")]
	protected ref NO_SCR_DCP_CombatPatrolsConfig m_pCombatPatrolsConfig;

	protected RplComponent m_pRplComponent;

	// Patrol area related
	protected NO_SCR_DCP_PatrolArea m_pActivePatrol;
	protected ENightOpsPatrolType m_eActivePatrolType;

	protected ref array<NO_SCR_DCP_PatrolArea> m_aIntelPatrols = new array<NO_SCR_DCP_PatrolArea>();
	protected ref array<NO_SCR_DCP_PatrolArea> m_aSabotagePatrols = new array<NO_SCR_DCP_PatrolArea>();
	protected ref array<NO_SCR_DCP_PatrolArea> m_aHVTPatrols = new array<NO_SCR_DCP_PatrolArea>();

	// Config related
	protected NO_SCR_DCP_PatrolAssetsConfig m_pFactionAssets;
	protected NO_SCR_DCP_PatrolTasksConfig m_pFactionTasksConfig;
	protected bool m_bIsValidConfig = false;

	// Config entities
	protected ref map<ENightOpsPatrolTasks, NO_SCR_EditorTask> m_mFactionTasks = new map<ENightOpsPatrolTasks, NO_SCR_EditorTask>();
	protected NO_SCR_MissionTrigger m_pInfilTrigger;
	protected NO_SCR_MissionTrigger m_pExfilTrigger;

	// Synchronised variables
	[RplProp()]
	protected ref array<ENightOpsPatrolType> m_aAvailablePatrolTypes = new array<ENightOpsPatrolType>();

	[RplProp()]
	protected bool m_bIsOnPatrol = true;


	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (!GetGame().GetWorldEntity())
  			return;

		m_pRplComponent = RplComponent.Cast(FindComponent(RplComponent));
		if (!m_pRplComponent)
		{
			Print("NO_SCR_DCP_PatrolManager requires an RplComponent but has none!", LogLevel.ERROR);
			return;
		}

		if (m_pRplComponent.IsProxy())
			return;

		m_bIsOnPatrol = false;

		ReadMissionHeader();

		if (!m_pCombatPatrolsConfig)
			return;

		ReadConfig();
		Replication.BumpMe();

		if (m_bIsValidConfig)
		{
			// Existed but wasn't visible when called on same frame ¯\_(ツ)_/¯
			GetGame().GetCallqueue().Call(SpawnPlayerHQ);
			GetGame().GetCallqueue().CallLater(AssignInitialTask, 500);
		}
	}


	protected void SetIsOnPatrol(bool state)
	{
		if (m_bIsOnPatrol != state)
		{
			m_bIsOnPatrol = state;
			Replication.BumpMe();
		}
	}


	protected void ReadMissionHeader()
	{
		SCR_MissionHeader header = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		if (!header)
			return;

		NO_SCR_DCP_MissionHeader dcpHeader = NO_SCR_DCP_MissionHeader.Cast(header);
		if (dcpHeader)
		{
			FactionKey desiredFactionKey = dcpHeader.m_sPlayerFactionKey;
			if (GetGame().GetFactionManager().GetFactionByKey(desiredFactionKey))
			{
				// MissionHeader supplied faction key is valid so use it
				m_sPlayerFactionKey = desiredFactionKey;
			}
		}
	}


	string GetPlayerFactionKey()
	{
		return m_sPlayerFactionKey;
	}


	protected void ReadConfig()
	{
		// Has a valid faction cfg
		NO_SCR_DCP_PatrolFactionConfig factionConfig = m_pCombatPatrolsConfig.GetFactionConfigByKey(GetPlayerFactionKey());
		if (factionConfig && factionConfig.IsValid())
		{
			m_bIsValidConfig = true;
		}
		else
		{
			Print("Improperly configured patrol faction detected, aborting setup!", LogLevel.ERROR);
			return;
		}

		// Assets for selected faction (names of key entities)
		m_pFactionAssets = factionConfig.GetAssetsConfig();

		// Tasks for selected faction (names of key tasks)
		m_pFactionTasksConfig = factionConfig.GetTasksConfig();

		BaseWorld world = GetWorld();

		// Find PatrolArea's in world based on config data
		foreach (NO_SCR_DCP_PatrolAreaConfig patrolAreaCfg : m_pCombatPatrolsConfig.GetPatrolAreas())
		{
			string patrolAreaName = patrolAreaCfg.GetAreaName();
			if (patrolAreaName.IsEmpty())
			{
				Print("Patrol config item has an empty name field!", LogLevel.ERROR);
				continue;
			}

			NO_SCR_DCP_PatrolArea patrolArea = NO_SCR_DCP_PatrolArea.Cast(world.FindEntityByName(patrolAreaName));
			if (!patrolArea)
			{
				Print(string.Format("Named PatrolArea not found in world: %1", patrolAreaName), LogLevel.ERROR);
				continue;
			}

			if (patrolAreaCfg.SupportsIntel())
				m_aIntelPatrols.Insert(patrolArea);

			if (patrolAreaCfg.SupportsSabotage())
				m_aSabotagePatrols.Insert(patrolArea);

			if (patrolAreaCfg.SupportsHVT())
				m_aHVTPatrols.Insert(patrolArea);
		}

		if (!m_aIntelPatrols.IsEmpty())
			m_aAvailablePatrolTypes.Insert(ENightOpsPatrolType.INTEL);

		if (!m_aSabotagePatrols.IsEmpty())
			m_aAvailablePatrolTypes.Insert(ENightOpsPatrolType.SABOTAGE);

		if (!m_aHVTPatrols.IsEmpty())
			m_aAvailablePatrolTypes.Insert(ENightOpsPatrolType.HVT);

		// Register method calls to key MissionTrigger events
		m_pInfilTrigger = NO_SCR_MissionTrigger.Cast(world.FindEntityByName(m_pFactionAssets.InfilTrigger));
		m_pExfilTrigger = NO_SCR_MissionTrigger.Cast(world.FindEntityByName(m_pFactionAssets.ExfilTrigger));

		if (m_pInfilTrigger && m_pExfilTrigger)
		{
			m_pInfilTrigger.GetOnPlayerQuotaReached().Insert(DelayedAssignPatrolTasks);
			m_pExfilTrigger.GetOnPlayerQuotaReached().Insert(FinishCurrentPatrol);
		}
		else
		{
			Print("Unable to find either infil or exfil trigger in world!", LogLevel.ERROR);
			m_bIsValidConfig = false;
		}
	}


	protected void SpawnPlayerHQ()
	{
		NO_SCR_EnvSpawnerComponent playerHQSpawner = FindSpawnerComponent(m_pFactionAssets.HQSpawner);
		if (playerHQSpawner)
			playerHQSpawner.DoSpawn();
	}


	protected void AssignInitialTask()
	{
		NO_SCR_EditorTask initialTask = FindTaskByName(m_pFactionTasksConfig.SelectPatrol);
		if (initialTask)
			initialTask.ChangeStateOfTask(TriggerType.Assign);
	}


	protected NO_SCR_EnvSpawnerComponent FindSpawnerComponent(string entityName)
	{
		IEntity spawnerEntity = GetGame().GetWorld().FindEntityByName(entityName);
		if (!spawnerEntity)
		{
			Print(string.Format("Cannot find spawner entity: %1", entityName), LogLevel.ERROR);
			return null;
		}

		NO_SCR_EnvSpawnerComponent spawnerComponent = NO_SCR_EnvSpawnerComponent.Cast(spawnerEntity.FindComponent(NO_SCR_EnvSpawnerComponent));
		if (!spawnerComponent)
		{
			Print(string.Format("Found '%1' but it is without a NO_SCR_EnvSpawnerComponent!", entityName), LogLevel.ERROR);
			return null;
		}

		return spawnerComponent;
	}


	protected NO_SCR_EditorTask FindTaskByName(string taskName)
	{
		IEntity taskEntity = GetGame().GetWorld().FindEntityByName(taskName);
		if (!taskEntity)
		{
			Print(string.Format("Could not find task entity named: %1", taskName), LogLevel.ERROR);
			return null;
		}

		NO_SCR_EditorTask task = NO_SCR_EditorTask.Cast(taskEntity);
		if (!task)
		{
			Print(string.Format("Entity %1 found, but is not of type NO_SCR_EditorTask!", taskName), LogLevel.ERROR);
			return null;
		}

		return task;
	}


	protected void AddTaskToMap(ENightOpsPatrolTasks key, string taskName)
	{
		NO_SCR_EditorTask taskEntity = FindTaskByName(taskName);

		if (taskEntity)
			m_mFactionTasks.Insert(key, taskEntity);
		else
			Print(string.Format("Could not find '%1' task: %2", key, taskName), LogLevel.ERROR);
	}


	NO_SCR_EditorTask GetPatrolTask(ENightOpsPatrolTasks taskType)
	{
		return m_mFactionTasks.Get(taskType);
	}


	protected NO_SCR_DCP_PatrolArea ChoosePatrol(ENightOpsPatrolType patrolType)
	{
		NO_SCR_DCP_PatrolArea chosenPatrol;

		if (patrolType == ENightOpsPatrolType.INTEL)
		{
			chosenPatrol = m_aIntelPatrols.Get(Math.RandomInt(0, m_aIntelPatrols.Count()));
		}
		else if (patrolType == ENightOpsPatrolType.SABOTAGE)
		{
			chosenPatrol = m_aSabotagePatrols.Get(Math.RandomInt(0, m_aSabotagePatrols.Count()));
		}
		else if (patrolType == ENightOpsPatrolType.HVT)
		{
			chosenPatrol = m_aHVTPatrols.Get(Math.RandomInt(0, m_aHVTPatrols.Count()));
		}

		return chosenPatrol;
	}


	protected int GetRandomPatrolType()
	{
		int minimum;
		int maximum;
		SCR_Enum.GetRange(ENightOpsPatrolType, minimum, maximum);

		return Math.RandomIntInclusive(minimum, maximum);
	}


	protected int GetRandomAvailablePatrolType()
	{
		return m_aAvailablePatrolTypes.Get(Math.RandomInt(0, m_aAvailablePatrolTypes.Count()));
	}


	bool ArePatrolsAvailable()
	{
		return !m_aAvailablePatrolTypes.IsEmpty();
	}


	bool IsPatrolTypeAvailable(ENightOpsPatrolType patrolType)
	{
		if (patrolType == ENightOpsPatrolType.INTEL)
			return m_aAvailablePatrolTypes.Contains(ENightOpsPatrolType.INTEL);
		else if (patrolType == ENightOpsPatrolType.SABOTAGE)
			return m_aAvailablePatrolTypes.Contains(ENightOpsPatrolType.SABOTAGE);
		else if (patrolType == ENightOpsPatrolType.HVT)
			return m_aAvailablePatrolTypes.Contains(ENightOpsPatrolType.HVT);

		return false;
	}


	bool IsOnPatrol()
	{
		return m_bIsOnPatrol;
	}


	void StartPatrol()
	{
		if (ArePatrolsAvailable())
		{
			ENightOpsPatrolType randomPatrolType = GetRandomAvailablePatrolType();
			StartPatrol(randomPatrolType);
		}
	}


	void StartPatrol(ENightOpsPatrolType pickedPatrolType)
	{
		if (m_pRplComponent.IsProxy() || !m_bIsValidConfig)
			return;

		NO_SCR_DCP_PatrolArea chosenPatrol = ChoosePatrol(pickedPatrolType);

		if (!chosenPatrol)
		{
			Print(string.Format("No patrols found of selected type: 1%", SCR_Enum.GetEnumName(ENightOpsPatrolType, pickedPatrolType)), LogLevel.ERROR);
			return;
		}

		SetIsOnPatrol(true);

		// First time?
		if (m_mFactionTasks.IsEmpty())
		{
			AddTaskToMap(ENightOpsPatrolTasks.SELECT_PATROL, m_pFactionTasksConfig.SelectPatrol);
			AddTaskToMap(ENightOpsPatrolTasks.START_PATROL, m_pFactionTasksConfig.StartPatrol);
			AddTaskToMap(ENightOpsPatrolTasks.END_PATROL, m_pFactionTasksConfig.EndPatrol);
			AddTaskToMap(ENightOpsPatrolTasks.INTEL_MAIN, m_pFactionTasksConfig.IntelPatrolMain);
			AddTaskToMap(ENightOpsPatrolTasks.INTEL_OBJ_1, m_pFactionTasksConfig.IntelPatrolObj1);
			AddTaskToMap(ENightOpsPatrolTasks.INTEL_OBJ_2, m_pFactionTasksConfig.IntelPatrolObj2);
			AddTaskToMap(ENightOpsPatrolTasks.SABOTAGE_MAIN, m_pFactionTasksConfig.SabotagePatrolMain);
			AddTaskToMap(ENightOpsPatrolTasks.SABOTAGE_OBJ_1, m_pFactionTasksConfig.SabotagePatrolObj1);
			AddTaskToMap(ENightOpsPatrolTasks.SABOTAGE_OBJ_2, m_pFactionTasksConfig.SabotagePatrolObj2);
			AddTaskToMap(ENightOpsPatrolTasks.HVT_MAIN, m_pFactionTasksConfig.HVTPatrolMain);
		}

		// Finish patrol selection task
		NO_SCR_EditorTask selectPatrolTask = GetPatrolTask(ENightOpsPatrolTasks.SELECT_PATROL);
		if (selectPatrolTask)
			selectPatrolTask.ChangeStateOfTask(TriggerType.Finish);

		// Set position of the start patrol task then assign it
		NO_SCR_EditorTask startPatrolTask = GetPatrolTask(ENightOpsPatrolTasks.START_PATROL);
		if (startPatrolTask && m_pInfilTrigger)
		{
			startPatrolTask.SetOrigin(m_pInfilTrigger.GetOrigin());
			startPatrolTask.ChangeStateOfTask(TriggerType.Assign);
		}

		// Have DCP_PatrolArea perform its setup
		chosenPatrol.StartPatrolType(pickedPatrolType, m_pFactionAssets);
		m_pActivePatrol = chosenPatrol;
		m_eActivePatrolType = pickedPatrolType;

		// Unlock infilTrigger
		if (m_pInfilTrigger)
			m_pInfilTrigger.SetActive(true);
	}


	void DelayedAssignPatrolTasks(int delay = 1500)
	{
		GetGame().GetCallqueue().CallLater(AssignPatrolTasks, delay);
	}


	void AssignPatrolTasks()
	{
		if (m_pRplComponent.IsProxy() || !m_bIsValidConfig)
			return;

		if (m_eActivePatrolType == ENightOpsPatrolType.INTEL)
		{
			NO_SCR_EditorTask taskMain = GetPatrolTask(ENightOpsPatrolTasks.INTEL_MAIN);
			NO_SCR_EditorTask task1 = GetPatrolTask(ENightOpsPatrolTasks.INTEL_OBJ_1);
	        NO_SCR_EditorTask task2 = GetPatrolTask(ENightOpsPatrolTasks.INTEL_OBJ_2);

			if (taskMain)
				taskMain.ChangeStateOfTask(TriggerType.Create);
			else
				Print("MULTITASK MISSING", LogLevel.ERROR);

			if (task1)
				task1.ChangeStateOfTask(TriggerType.Assign);

			if (task2)
				task2.ChangeStateOfTask(TriggerType.Create);
		}
		else if (m_eActivePatrolType == ENightOpsPatrolType.SABOTAGE)
		{
			NO_SCR_EditorTask taskMain = GetPatrolTask(ENightOpsPatrolTasks.SABOTAGE_MAIN);
			NO_SCR_EditorTask task1 = GetPatrolTask(ENightOpsPatrolTasks.SABOTAGE_OBJ_1);
	        NO_SCR_EditorTask task2 = GetPatrolTask(ENightOpsPatrolTasks.SABOTAGE_OBJ_2);

			if (taskMain)
				taskMain.ChangeStateOfTask(TriggerType.Create);
			else
				Print("MULTITASK MISSING", LogLevel.ERROR);

			if (task1)
				task1.ChangeStateOfTask(TriggerType.Assign);

			if (task2)
				task2.ChangeStateOfTask(TriggerType.Create);
		}
		else if (m_eActivePatrolType == ENightOpsPatrolType.HVT)
		{
	        NO_SCR_EditorTask taskMain = GetPatrolTask(ENightOpsPatrolTasks.HVT_MAIN);
			if (taskMain)
				taskMain.ChangeStateOfTask(TriggerType.Assign);
		}
	}


	void FinishCurrentPatrol()
	{
		if (m_pRplComponent.IsProxy() || !m_bIsValidConfig)
			return;

		if (m_pInfilTrigger)
			m_pInfilTrigger.SetActive(false);

		if (m_pExfilTrigger)
			m_pExfilTrigger.SetActive(false);

		if (m_pActivePatrol)
		{
			m_pActivePatrol.EndPatrol();
			m_pActivePatrol = null;
			m_eActivePatrolType = null;
		}

		NO_SCR_EditorTask endPatrolTask = GetPatrolTask(ENightOpsPatrolTasks.END_PATROL);
		if (endPatrolTask)
			endPatrolTask.ChangeStateOfTask(TriggerType.Finish);

		NO_SCR_EditorTask selectPatrolTask = GetPatrolTask(ENightOpsPatrolTasks.SELECT_PATROL);
		if (selectPatrolTask)
			selectPatrolTask.ChangeStateOfTask(TriggerType.Assign);

		SetIsOnPatrol(false);
	}


	void NO_SCR_DCP_PatrolManager(IEntitySource src, IEntity parent)
	{
		if (!g_PatrolManagerInstance)
			g_PatrolManagerInstance = this;

		SetEventMask(EntityEvent.INIT);
		SetFlags(EntityFlags.STATIC, false);
	}


	void ~NO_SCR_DCP_PatrolManager()
	{
		if (m_pInfilTrigger && m_pExfilTrigger)
		{
			m_pInfilTrigger.GetOnPlayerQuotaReached().Remove(DelayedAssignPatrolTasks);
			m_pExfilTrigger.GetOnPlayerQuotaReached().Remove(FinishCurrentPatrol);
		}
	}
}



enum ENightOpsPatrolType
{
	INTEL,
	SABOTAGE,
	HVT
}



enum ENightOpsPatrolTasks
{
	SELECT_PATROL,
	START_PATROL,
	END_PATROL,
	INTEL_MAIN,
	INTEL_OBJ_1,
	INTEL_OBJ_2,
	SABOTAGE_MAIN,
	SABOTAGE_OBJ_1,
	SABOTAGE_OBJ_2,
	HVT_MAIN
}



[BaseContainerProps(configRoot: true)]
class NO_SCR_DCP_CombatPatrolsConfig
{
	[Attribute(defvalue: "US", uiwidget: UIWidgets.EditBox, desc: "TEMPORARY, JUST FOR TESTING!")]
	private string m_sDefaultFactionKey;

	[Attribute("", UIWidgets.Object)]
	private ref array<ref NO_SCR_DCP_PatrolFactionConfig> m_aPlayableFactions;

	[Attribute("", UIWidgets.Object)]
	private ref array<ref NO_SCR_DCP_PatrolAreaConfig> m_aPatrolAreas;

	array<ref NO_SCR_DCP_PatrolAreaConfig> GetPatrolAreas() { return m_aPatrolAreas; }

	NO_SCR_DCP_PatrolFactionConfig GetFactionConfigByKey(string factionKey)
	{
		// See if any factions match the provided key
		foreach (NO_SCR_DCP_PatrolFactionConfig factionCfg : m_aPlayableFactions)
			if (factionCfg.GetFactionKey() == factionKey)
				return factionCfg;

		// See if any factions match the default set key
		foreach (NO_SCR_DCP_PatrolFactionConfig factionCfg : m_aPlayableFactions)
			if (factionCfg.GetFactionKey() == m_sDefaultFactionKey)
				return factionCfg;

		return null;
	}
}



[BaseContainerProps(), BaseContainerCustomTitleField("m_sAreaName")]
class NO_SCR_DCP_PatrolAreaConfig
{
	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBox, desc: "Entity name of a preplaced patrol area.")]
	private string m_sAreaName;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Area supports intel operations.")]
	private bool m_bHasIntel;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Area supports sabotage operations.")]
	private bool m_bHasSabotage;

	[Attribute(defvalue: "1", uiwidget: UIWidgets.CheckBox, desc: "Area supports 'High Value Target' operations.")]
	private bool m_bHasHVT;

	string GetAreaName() { return m_sAreaName; }
	bool SupportsIntel() { return m_bHasIntel; }
	bool SupportsSabotage() { return m_bHasSabotage; }
	bool SupportsHVT() { return m_bHasHVT; }
}



[BaseContainerProps(), BaseContainerCustomTitleField("m_sFactionKey")]
class NO_SCR_DCP_PatrolFactionConfig
{
	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBox, desc: "Key of a playable faction.")]
	private string m_sFactionKey;

	[Attribute("", UIWidgets.Object)]
	private ref NO_SCR_DCP_PatrolAssetsConfig m_pFactionAssets;

	[Attribute("", UIWidgets.Object)]
	private ref NO_SCR_DCP_PatrolTasksConfig m_pFactionTasks;

	string GetFactionKey() { return m_sFactionKey; }
	bool IsValid() { return m_pFactionAssets && m_pFactionTasks; }

	NO_SCR_DCP_PatrolAssetsConfig GetAssetsConfig() { return m_pFactionAssets; }
	NO_SCR_DCP_PatrolTasksConfig GetTasksConfig() { return m_pFactionTasks; }
}



[BaseContainerProps()]
class NO_SCR_DCP_PatrolAssetsConfig
{
	[Attribute(defvalue: "Base_Spawnpoint_US", uiwidget: UIWidgets.EditBox, desc: "Entity name of main base spawnpoint.")]
	string BaseSpawnpoint;

	[Attribute(defvalue: "Infil_Spawnpoint_US", uiwidget: UIWidgets.EditBox, desc: "Entity name of infil spawnpoint.")]
	string InfilSpawnpoint;

	[Attribute(defvalue: "Infil_Trigger_US", uiwidget: UIWidgets.EditBox, desc: "Entity name of infil trigger.")]
	string InfilTrigger;

	[Attribute(defvalue: "Infil_TP_Point_US", uiwidget: UIWidgets.EditBox, desc: "Entity name of infil teleport point.")]
	string InfilTeleportPoint;

	[Attribute(defvalue: "Exfil_Trigger_US", uiwidget: UIWidgets.EditBox, desc: "Entity name of exfil trigger.")]
	string ExfilTrigger;

	[Attribute(defvalue: "Exfil_TP_Point_US", uiwidget: UIWidgets.EditBox, desc: "Entity name of exfil teleport point.")]
	string ExfilTeleportPoint;

	[Attribute(defvalue: "HQ_Spawner_US", uiwidget: UIWidgets.EditBox, desc: "Entity name of faction HQ spawner.")]
	string HQSpawner;

	[Attribute(defvalue: "FOB_Spawner_US", uiwidget: UIWidgets.EditBox, desc: "Entity name of faction FOB spawner.")]
	string FOBSpawner;
}



[BaseContainerProps()]
class NO_SCR_DCP_PatrolTasksConfig
{
	[Attribute(defvalue: "SelectPatrol_Task_US", uiwidget: UIWidgets.EditBox, desc: "Entity name/rename of 'select patrol' task.")]
	string SelectPatrol;

	[Attribute(defvalue: "StartPatrol_Task_US", uiwidget: UIWidgets.EditBox, desc: "Entity name/rename of 'start patrol' task.")]
	string StartPatrol;

	[Attribute(defvalue: "EndPatrol_Task_US", uiwidget: UIWidgets.EditBox, desc: "Entity name/rename of 'end patrol' task.")]
	string EndPatrol;

	[Attribute(defvalue: "Intel_Task_US", uiwidget: UIWidgets.EditBox, desc: "Entity name/rename of 'intel' multitask.")]
	string IntelPatrolMain;

	[Attribute(defvalue: "Intel_1_Task_US", uiwidget: UIWidgets.EditBox, desc: "Entity name/rename of 'intel sub objective 1' task.")]
	string IntelPatrolObj1;

	[Attribute(defvalue: "Intel_2_Task_US", uiwidget: UIWidgets.EditBox, desc: "Entity name/rename of 'intel sub objective 2' task.")]
	string IntelPatrolObj2;

	[Attribute(defvalue: "Sabotage_Task_US", uiwidget: UIWidgets.EditBox, desc: "Entity name/rename of 'sabotage' multitask.")]
	string SabotagePatrolMain;

	[Attribute(defvalue: "Sabotage_1_Task_US", uiwidget: UIWidgets.EditBox, desc: "Entity name/rename of 'sabotage sub objective 1' task.")]
	string SabotagePatrolObj1;

	[Attribute(defvalue: "Sabotage_2_Task_US", uiwidget: UIWidgets.EditBox, desc: "Entity name/rename of 'sabotage sub objective 2' task.")]
	string SabotagePatrolObj2;

	[Attribute(defvalue: "HVT_Task_US", uiwidget: UIWidgets.EditBox, desc: "Entity name/rename of 'HVT' task.")]
	string HVTPatrolMain;
}