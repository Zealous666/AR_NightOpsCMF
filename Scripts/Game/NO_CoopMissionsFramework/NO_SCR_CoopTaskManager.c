[EntityEditorProps(category: "GameScripted/Coop", description: "Allows task management.")]
class NO_SCR_CoopTaskManagerClass : SCR_BaseTaskManagerClass
{
}

//------------------------------------------------------------------------------------------------
class NO_SCR_CoopTaskManager : SCR_BaseTaskManager
{
	[Attribute("", UIWidgets.Auto, "Entity names of intial tasks - assigned to connecting players automatically.", category: "TaskManager: COOP")]
	protected ref array<string> m_aInitialTaskNames;

	[Attribute("US", UIWidgets.EditBox, "Faction key to assign to tasks.", category: "TaskManager: COOP")]
	protected FactionKey m_sAssignedFaction;

	//! Runtime instances for tasks created from initial task names
	protected ref array<SCR_BaseTask> m_aInitialTasks;
	
	protected ref array<SCR_BaseTask> m_AdditionalTasks;
	protected ref array<SCR_BaseTask> m_FailedTasks;
	
	protected RplComponent m_pRplComponent;
	
	[Attribute()]
	protected ref array<ref SolidObjectives> AdditionalObjectives;
	
	//------------------------------------------------------------------------------------------------
	protected override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);
		
		// Do not initialize these tasks out of runtime
		if (!GetGame().InPlayMode())
			return;

		m_aInitialTasks = {};
		BaseWorld world = owner.GetWorld();
		foreach (string taskName : m_aInitialTaskNames)
		{
			IEntity entity = world.FindEntityByName(taskName);
			SCR_BaseTask task = SCR_BaseTask.Cast(entity);
			if (!task)
				continue;
			
			m_aInitialTasks.Insert(task);
		}

		m_pRplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!m_pRplComponent)
		{
			Print("SCR_CoopTaskManager is missing m_pRplComponent!", LogLevel.ERROR);
			return;
		}

		// Authority only
		if (!m_pRplComponent.IsMaster())
			return;

		SetInitialTasksFaction();
	}
	
	protected void SetInitialTasksFaction()
	{
		Faction targetFaction = GetGame().GetFactionManager().GetFactionByKey(m_sAssignedFaction);
		if (!targetFaction)
		{
			Print("SCR_CoopTaskManager is missing Faction to assign tasks to!", LogLevel.ERROR);
			return;
		}

		foreach (SCR_BaseTask task : m_aInitialTasks)
		{
			if (!task)
				continue;

			//task.SetTargetFaction(targetFaction);
			SetTaskFaction(task, targetFaction);
		}
	}

	
	void UnlockObjective(string unlockKey)
	{
	
		// Do not initialize these tasks out of runtime
		if (!GetGame().InPlayMode())
			return;

		m_AdditionalTasks = {};
		BaseWorld world = this.GetWorld();

		SolidObjectives objData;
		foreach (SolidObjectives obj : AdditionalObjectives)
		{
			if(obj.m_UnlockKey == unlockKey)
			{
				objData = obj;
				break;
			}
		}
		if(objData==null)
		{
			//We are out of objectives??
			return;
		}
		
		foreach (string taskName : objData.m_TaskNames)
		{
			IEntity entity = world.FindEntityByName(taskName);
			SCR_BaseTask task = SCR_BaseTask.Cast(entity);
			if (!task)
				continue;
			
			m_AdditionalTasks.Insert(task);
		}
		
		m_pRplComponent = RplComponent.Cast(this.FindComponent(RplComponent));
		if (!m_pRplComponent)
		{
			Print("SCR_CoopTaskManager is missing m_pRplComponent!", LogLevel.ERROR);
			return;
		}

		// Authority only
		if (!m_pRplComponent.IsMaster())
			return;

		Faction targetFaction = GetGame().GetFactionManager().GetFactionByKey(m_sAssignedFaction);
		if (!targetFaction)
		{
			Print("SCR_CoopTaskManager is missing Faction to assign tasks to!", LogLevel.ERROR);
			return;
		}

		foreach (SCR_BaseTask task : m_AdditionalTasks)
		{
			if (!task)
				continue;

			//task.SetTargetFaction(targetFaction);
			SetTaskFaction(task, targetFaction);
		}
		m_FailedTasks = {};
		foreach (string taskName : objData.m_FailedTaskNames)
		{
			IEntity entity = world.FindEntityByName(taskName);
			SCR_BaseTask task = SCR_BaseTask.Cast(entity);
			if (!task)
				continue;
			
			m_FailedTasks.Insert(task);
		}
		foreach (SCR_BaseTask fTask : m_FailedTasks)
		{
			GetTaskManager().FailTask(fTask);
		}
		
		if(objData.m_UnlockSpawnpoint!="")
		{
			SCR_SpawnPoint.Cast(GetWorld().FindEntityByName(objData.m_UnlockSpawnpoint)).SetFactionKey("US");
		}
	}

	////////////////////////////////////////////////////////////////
	///// CALLED ON EVERY MACHINE FOR EVERY PLAYER REGISTERED //////
	// INCLUDING WHEN JIP, PREVIOUS REGISTERED PLAYERS ARE CALLED //
	////////////////////////////////////////////////////////////////
	protected override void OnPlayerRegistered(int registeredPlayerID)
	{
		super.OnPlayerRegistered(registeredPlayerID);

		// Reset faction on initial tasks for JIP (Authority only)
		if (m_pRplComponent.IsMaster())
			SetInitialTasksFaction();
	}
}

[BaseContainerProps(), BaseContainerCustomTitleField("m_UnlockKey")]
class SolidObjectives 
{
	[Attribute("", UIWidgets.EditBox, "Unlock keyword.", category: "TaskManager: COOP")]
	string m_UnlockKey;
	
	[Attribute("", UIWidgets.Auto, "Entity names of tasks to be unlocked.", category: "TaskManager: COOP")]
	ref array<string> m_TaskNames;
	
	[Attribute("", UIWidgets.Auto, "Entity names of tasks that fails when this activates.", category: "TaskManager: COOP")]
	ref array<string> m_FailedTaskNames;
	
	[Attribute("", UIWidgets.EditBox, "Unlock spawnpoint name.", category: "TaskManager: COOP")]
	string m_UnlockSpawnpoint;
}