[EntityEditorProps(category: "GameScripted/CombatPatrol", description: "Patrol manager entity.", visible: false)]
class NO_SCR_PatrolManagerClass : GenericEntityClass
{
}



NO_SCR_PatrolManager g_PatrolManagerInstance;
NO_SCR_PatrolManager GetPatrolManager()
{
	return g_PatrolManagerInstance;
}



class NO_SCR_PatrolManager : GenericEntity
{
	[Attribute("", UIWidgets.Object, desc: "Change items within config file, not through Object Properties!", category: "COMBAT PATROLS", params: "noDetails")]
	protected ref NO_SCR_CombatPatrolsConfig m_pCombatPatrolsConfig;


	// The task prefabs and their spawners are for each faction, the task names are shared among factions
	const string SELECT_PATROL_TASKNAME = "SelectPatrol_Task";
	const string START_PATROL_TASKNAME = "StartPatrol_Task";
	const string END_PATROL_TASKNAME = "EndPatrol_Task";

	const string INTEL_PATROL_TASKNAME = "Intel_Task";
	const string INTEL_PATROL_OBJ1_TASKNAME = "Intel_1_Task";
	const string INTEL_PATROL_OBJ2_TASKNAME = "Intel_2_Task";
	const string SABOTAGE_PATROL_TASKNAME = "Sabotage_Task";

	const string SABOTAGE_PATROL_OBJ1_TASKNAME = "Sabotage_1_Task";
	const string SABOTAGE_PATROL_OBJ2_TASKNAME = "Sabotage_2_Task";

	const string HVT_PATROL_TASKNAME = "HVT_Task";


	// Patrol area related
	protected NO_SCR_PatrolArea m_pActivePatrol;
	protected ENightOpsPatrolType m_eActivePatrolType;

	protected ref array<NO_SCR_PatrolArea> m_aIntelPatrols = new array<NO_SCR_PatrolArea>();
	protected ref array<NO_SCR_PatrolArea> m_aSabotagePatrols = new array<NO_SCR_PatrolArea>();
	protected ref array<NO_SCR_PatrolArea> m_aHVTPatrols = new array<NO_SCR_PatrolArea>();


	// Faction entities/spawners related
	protected NO_SCR_PatrolFactionConfig m_pFactionConfig;
	protected NO_SCR_PatrolAssetsConfig m_pFactionAssets;

	protected ref array<NO_SCR_EnvSpawnerComponent> m_aCoreTaskSpawners = new array<NO_SCR_EnvSpawnerComponent>();
	protected ref array<NO_SCR_EnvSpawnerComponent> m_aIntelTaskSpawners = new array<NO_SCR_EnvSpawnerComponent>();
	protected ref array<NO_SCR_EnvSpawnerComponent> m_aSabotageTaskSpawners = new array<NO_SCR_EnvSpawnerComponent>();
	protected ref array<NO_SCR_EnvSpawnerComponent> m_aHVTTaskSpawners = new array<NO_SCR_EnvSpawnerComponent>();

	protected RplComponent m_pRplComponent;


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
			Print("NO_SCR_PatrolManager requires an RplComponent but has none!", LogLevel.ERROR);
			return;
		}

		if (m_pRplComponent.IsProxy())
			return;

		m_bIsOnPatrol = false;

		if (!m_pCombatPatrolsConfig)
			return;

		ReadConfig();
		Replication.BumpMe();

		GetGame().GetCallqueue().Call(SpawnCoreTasks);
	}


	protected void SetIsOnPatrol(bool state)
	{
		if (m_bIsOnPatrol != state)
		{
			m_bIsOnPatrol = state;
			Replication.BumpMe();
		}
	}


	protected void ReadConfig()
	{
		// Has a valid faction cfg
		m_pFactionConfig = m_pCombatPatrolsConfig.GetFactionConfigByKey();
		if (!m_pFactionConfig || !m_pFactionConfig.IsValid())
			return;

		// Assets for selected faction (names of key entities)
		m_pFactionAssets = m_pFactionConfig.GetAssetsConfig();

		// Find relevant task spawner entities for this faction
		foreach (string taskSpawnerName : m_pFactionConfig.GetCoreTaskSpawnerNames())
		{
			NO_SCR_EnvSpawnerComponent spawnerComponent = FindSpawnerComponent(taskSpawnerName);
			if (spawnerComponent) m_aCoreTaskSpawners.Insert(spawnerComponent);
		}

		foreach (string taskSpawnerName : m_pFactionConfig.GetIntelTaskSpawnerNames())
		{
			NO_SCR_EnvSpawnerComponent spawnerComponent = FindSpawnerComponent(taskSpawnerName);
			if (spawnerComponent) m_aIntelTaskSpawners.Insert(spawnerComponent);
		}

		foreach (string taskSpawnerName : m_pFactionConfig.GetSabotageTaskSpawnerNames())
		{
			NO_SCR_EnvSpawnerComponent spawnerComponent = FindSpawnerComponent(taskSpawnerName);
			if (spawnerComponent) m_aSabotageTaskSpawners.Insert(spawnerComponent);
		}

		foreach (string taskSpawnerName : m_pFactionConfig.GetHVTTaskSpawnerNames())
		{
			NO_SCR_EnvSpawnerComponent spawnerComponent = FindSpawnerComponent(taskSpawnerName);
			if (spawnerComponent) m_aHVTTaskSpawners.Insert(spawnerComponent);
		}

		BaseWorld world = GetWorld();

		// Find PatrolArea's in world based on config data
		foreach (NO_SCR_PatrolAreaConfig patrolAreaCfg : m_pCombatPatrolsConfig.GetPatrolAreas())
		{
			string patrolAreaName = patrolAreaCfg.GetAreaName();
			if (patrolAreaName.IsEmpty())
			{
				Print("Patrol config item has an empty name field!", LogLevel.ERROR);
				continue;
			}

			NO_SCR_PatrolArea patrolArea = NO_SCR_PatrolArea.Cast(world.FindEntityByName(patrolAreaName));
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
	}


	protected void SpawnTasks(array<NO_SCR_EnvSpawnerComponent> taskSpawners)
	{
		foreach (NO_SCR_EnvSpawnerComponent spawner : taskSpawners)
		{
			if (spawner.IsSpawned())
				spawner.RemoveSpawned();

			spawner.DoSpawn();
		}
	}


	protected void SpawnCoreTasks()
	{
		SpawnTasks(m_aCoreTaskSpawners);
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


	protected NO_SCR_PatrolArea ChoosePatrol(ENightOpsPatrolType patrolType)
	{
		NO_SCR_PatrolArea chosenPatrol;

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
		if (m_pRplComponent.IsProxy())
			return;

		NO_SCR_PatrolArea chosenPatrol = ChoosePatrol(pickedPatrolType);

		if (!chosenPatrol)
		{
			Print(string.Format("No patrols found of selected type: 1%", SCR_Enum.GetEnumName(ENightOpsPatrolType, pickedPatrolType)), LogLevel.ERROR);
			return;
		}

		SetIsOnPatrol(true);

		if (pickedPatrolType == ENightOpsPatrolType.INTEL)
			SpawnTasks(m_aIntelTaskSpawners);
		else if (pickedPatrolType == ENightOpsPatrolType.SABOTAGE)
			SpawnTasks(m_aSabotageTaskSpawners);
		else if (pickedPatrolType == ENightOpsPatrolType.HVT)
			SpawnTasks(m_aHVTTaskSpawners);

		// Finish patrol selection task
		NO_SCR_EditorTask selectPatrolTask = FindTaskByName(SELECT_PATROL_TASKNAME);
		if (selectPatrolTask)
			selectPatrolTask.ChangeStateOfTask(TriggerType.Finish);

		// Start patrol group up task
		NO_SCR_EditorTask startPatrolTask = FindTaskByName(START_PATROL_TASKNAME);
		if (startPatrolTask)
			startPatrolTask.ChangeStateOfTask(TriggerType.Assign);

		chosenPatrol.StartPatrolType(pickedPatrolType, m_pFactionAssets);
		m_pActivePatrol = chosenPatrol;
		m_eActivePatrolType = pickedPatrolType;
	}


	void AssignPatrolTasks()
	{
		if (m_pRplComponent.IsProxy())
			return;

		if (m_eActivePatrolType == ENightOpsPatrolType.INTEL)
		{
			NO_SCR_EditorTask task1 = FindTaskByName(INTEL_PATROL_OBJ1_TASKNAME);
	        NO_SCR_EditorTask task2 = FindTaskByName(INTEL_PATROL_OBJ2_TASKNAME);

			if (task1)
				task1.ChangeStateOfTask(TriggerType.Assign);

			if (task2)
				task2.ChangeStateOfTask(TriggerType.Create);
		}
		else if (m_eActivePatrolType == ENightOpsPatrolType.SABOTAGE)
		{
	        NO_SCR_EditorTask task1 = FindTaskByName(SABOTAGE_PATROL_OBJ1_TASKNAME);
	        NO_SCR_EditorTask task2 = FindTaskByName(SABOTAGE_PATROL_OBJ2_TASKNAME);

			if (task1)
				task1.ChangeStateOfTask(TriggerType.Assign);

			if (task2)
				task2.ChangeStateOfTask(TriggerType.Create);
		}
		else if (m_eActivePatrolType == ENightOpsPatrolType.HVT)
		{
	        NO_SCR_EditorTask task = FindTaskByName(HVT_PATROL_TASKNAME);
			if (task)
				task.ChangeStateOfTask(TriggerType.Assign);
		}
	}


	void FinishCurrentPatrol()
	{
		if (m_pRplComponent.IsProxy())
			return;

		if (m_pActivePatrol)
		{
			m_pActivePatrol.EndPatrol(m_pFactionAssets);
			m_pActivePatrol = null;
			m_eActivePatrolType = null;
		}

		IEntity exfilTask = GetGame().GetWorld().FindEntityByName(END_PATROL_TASKNAME);
		if (exfilTask)
		{
        	NO_SCR_EditorTask task = NO_SCR_EditorTask.Cast(exfilTask);
			task.ChangeStateOfTask(TriggerType.Finish);
		}

		GetGame().GetCallqueue().Call(SpawnCoreTasks);

		SetIsOnPatrol(false);
	}


	void NO_SCR_PatrolManager(IEntitySource src, IEntity parent)
	{
		if (!g_PatrolManagerInstance)
			g_PatrolManagerInstance = this;

		SetEventMask(EntityEvent.INIT);
		SetFlags(EntityFlags.STATIC, false);
	}
}



enum ENightOpsPatrolType
{
	INTEL,
	SABOTAGE,
	HVT
}



[BaseContainerProps(configRoot: true)]
class NO_SCR_CombatPatrolsConfig
{
	[Attribute(defvalue: "US", uiwidget: UIWidgets.EditBox, desc: "TEMPORARY, JUST FOR TESTING!")]
	private string m_sDefaultFactionKey;

	[Attribute("", UIWidgets.Object)]
	private ref array<ref NO_SCR_PatrolFactionConfig> m_aPlayableFactions;

	[Attribute("", UIWidgets.Object)]
	private ref array<ref NO_SCR_PatrolAreaConfig> m_aPatrolAreas;

	array<ref NO_SCR_PatrolAreaConfig> GetPatrolAreas() { return m_aPatrolAreas; }

	NO_SCR_PatrolFactionConfig GetFactionConfigByKey(string factionKey = string.Empty)
	{
		if (factionKey.IsEmpty())
			factionKey = m_sDefaultFactionKey;

		foreach (NO_SCR_PatrolFactionConfig factionCfg : m_aPlayableFactions)
			if (factionCfg.GetFactionKey() == factionKey)
				return factionCfg;

		return null;
	}
}



[BaseContainerProps(), BaseContainerCustomTitleField("m_sAreaName")]
class NO_SCR_PatrolAreaConfig
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
class NO_SCR_PatrolFactionConfig
{
	[Attribute(defvalue: "", uiwidget: UIWidgets.EditBox, desc: "Key of a playable faction.")]
	private string m_sFactionKey;

	[Attribute("", UIWidgets.Object)]
	private ref NO_SCR_PatrolAssetsConfig m_pFactionAssets;

	[Attribute("", UIWidgets.Object)]
	private ref NO_SCR_PatrolTasksConfig m_pFactionTaskSpawners;

	string GetFactionKey() { return m_sFactionKey; }
	bool IsValid() { return m_pFactionAssets && m_pFactionTaskSpawners; }

	NO_SCR_PatrolAssetsConfig GetAssetsConfig() { return m_pFactionAssets; }

	array<string> GetCoreTaskSpawnerNames()
	{
		array<string> taskSpawnerNames = new array<string>();
		taskSpawnerNames.Insert(m_pFactionTaskSpawners.SelectPatrol);
		taskSpawnerNames.Insert(m_pFactionTaskSpawners.StartPatrol);
		taskSpawnerNames.Insert(m_pFactionTaskSpawners.EndPatrol);
		return taskSpawnerNames;
	}

	array<string> GetIntelTaskSpawnerNames()
	{
		array<string> taskSpawnerNames = new array<string>();
		taskSpawnerNames.Insert(m_pFactionTaskSpawners.IntelPatrolObj1);
		taskSpawnerNames.Insert(m_pFactionTaskSpawners.IntelPatrolObj2);
		taskSpawnerNames.Insert(m_pFactionTaskSpawners.IntelPatrolMain);
		return taskSpawnerNames;
	}

	array<string> GetSabotageTaskSpawnerNames()
	{
		array<string> taskSpawnerNames = new array<string>();
		taskSpawnerNames.Insert(m_pFactionTaskSpawners.SabotagePatrolObj1);
		taskSpawnerNames.Insert(m_pFactionTaskSpawners.SabotagePatrolObj2);
		taskSpawnerNames.Insert(m_pFactionTaskSpawners.SabotagePatrolMain);
		return taskSpawnerNames;
	}

	array<string> GetHVTTaskSpawnerNames()
	{
		array<string> taskSpawnerNames = new array<string>();
		taskSpawnerNames.Insert(m_pFactionTaskSpawners.HVTPatrolMain);
		return taskSpawnerNames;
	}
}



[BaseContainerProps()]
class NO_SCR_PatrolAssetsConfig
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

	[Attribute(defvalue: "Intel_Spawner_1", uiwidget: UIWidgets.EditBox, desc: "Entity name of intel (obj 1) spawner.")]
	string IntelObjective1;

	[Attribute(defvalue: "Intel_Spawner_2", uiwidget: UIWidgets.EditBox, desc: "Entity name of intel (obj 2) spawner.")]
	string IntelObjective2;

	[Attribute(defvalue: "Sabotage_Spawner_1", uiwidget: UIWidgets.EditBox, desc: "Entity name of sabotage (obj 1) spawner.")]
	string SabotageObjective1;

	[Attribute(defvalue: "Sabotage_Spawner_2", uiwidget: UIWidgets.EditBox, desc: "Entity name of sabotage (obj 2) spawner.")]
	string SabotageObjective2;

	[Attribute(defvalue: "HVT_Spawner", uiwidget: UIWidgets.EditBox, desc: "Entity name of HVT spawner.")]
	string HVTObjective;
}



[BaseContainerProps()]
class NO_SCR_PatrolTasksConfig
{
	[Attribute(defvalue: "SelectPatrol_US", uiwidget: UIWidgets.EditBox, desc: "Entity name of 'select patrol' task spawner.")]
	string SelectPatrol;

	[Attribute(defvalue: "StartPatrol_US", uiwidget: UIWidgets.EditBox, desc: "Entity name of 'start patrol' task spawner.")]
	string StartPatrol;

	[Attribute(defvalue: "EndPatrol_US", uiwidget: UIWidgets.EditBox, desc: "Entity name of 'end patrol' task spawner.")]
	string EndPatrol;

	[Attribute(defvalue: "IntelPatrol_US", uiwidget: UIWidgets.EditBox, desc: "Entity name of 'intel patrol' multitask spawner.")]
	string IntelPatrolMain;

	[Attribute(defvalue: "IntelPatrol_1_US", uiwidget: UIWidgets.EditBox, desc: "Entity name of 'intel patrol' sub objective task spawner.")]
	string IntelPatrolObj1;

	[Attribute(defvalue: "IntelPatrol_2_US", uiwidget: UIWidgets.EditBox, desc: "Entity name of 'intel patrol' sub objective task spawner.")]
	string IntelPatrolObj2;

	[Attribute(defvalue: "SabotagePatrol_US", uiwidget: UIWidgets.EditBox, desc: "Entity name of 'sabotage patrol' multitask spawner.")]
	string SabotagePatrolMain;

	[Attribute(defvalue: "SabotagePatrol_1_US", uiwidget: UIWidgets.EditBox, desc: "Entity name of 'sabotage patrol' sub objective task spawner.")]
	string SabotagePatrolObj1;

	[Attribute(defvalue: "SabotagePatrol_2_US", uiwidget: UIWidgets.EditBox, desc: "Entity name of 'sabotage patrol' sub objective task spawner.")]
	string SabotagePatrolObj2;

	[Attribute(defvalue: "HVTPatrol_US", uiwidget: UIWidgets.EditBox, desc: "Entity name of 'HVT patrol' task spawner.")]
	string HVTPatrolMain;
}