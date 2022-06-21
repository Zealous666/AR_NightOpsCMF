[EntityEditorProps(category: "GameScripted/Triggers", description: "ScriptWizard generated script file.")]
class NO_SCR_MissionTriggerClass : NO_SCR_PlayerTriggerEntityClass
{
}

class NO_SCR_MissionTrigger : NO_SCR_PlayerTriggerEntity
{
	protected const string WAYPOINT_ENTITY_NAME = "WP";
	protected const int FADE_OUT_BUFFER = 2000;

	// -------------------------------------------------------

	[Attribute(desc: "Activate once one of the specified tasks reaches the specified state. MultiTasks can also be used.", category: "PLAYER TRIGGER")]
	protected ref array<ref NO_SCR_TaskStateActivatorEntry> m_aActivateOnTaskState;

	// -------------------------------------------------------

	[Attribute("0", UIWidgets.CheckBox, desc: "If enabled, mission trigger will not disable after first firing!", category: "MISSION CHANGES")]
	protected bool m_bIsRepeatable;

	[Attribute("0", UIWidgets.Slider, desc: "Delay the triggers action (in seconds).", category: "MISSION CHANGES", params: "0 60 0.001")]
	protected float m_fDelay;

	[Attribute("", UIWidgets.EditBox, desc: "Name of NO_SpawnTriggers to spawn.", category: "MISSION CHANGES")]
	protected ref array<string> m_sNOSpawnTriggerNames;

	[Attribute("", UIWidgets.EditBox, desc: "Name of NO_SpawnTriggers to despawn.", category: "MISSION CHANGES")]
	protected ref array<string> m_sNODespawnTriggerNames;

	[Attribute(desc: "Task state changes to make.", category: "MISSION CHANGES")]
	protected ref array<ref NO_SCR_TaskStateChangeEntry> m_aTaskStateChanges;

	[Attribute(desc: "Spawnpoint changes to make.", category: "MISSION CHANGES")]
	protected ref array<ref NO_SCR_SpawnpointChangeEntry> m_aSpawnpointChanges;

	[Attribute("", UIWidgets.EditBox, desc: "Name of an entity with a MissionSelectionManagerComponent.", category: "MISSION CHANGES")]
	protected string m_sMissionSelectionManagerName;

	[Attribute("", UIWidgets.EditBox, desc: "Name of MissionSelectionAction's to end on the manager above.", category: "MISSION CHANGES")]
	protected ref array<string> m_aEndMissionSelections;

	[Attribute("", UIWidgets.EditBox, desc: "Name of a waypoint location to move the waypoint marker.", category: "MISSION CHANGES")]
	protected string m_sWaypointLocationName;

	[Attribute(desc: "Time/weather changes to make.", category: "MISSION CHANGES")]
	protected ref NO_SCR_ChangeTimeWeatherType m_pChangeTimeAndWeather;

	// -------------------------------------------------------

	[Attribute("0", UIWidgets.CheckBox, desc: "If enabled, will teleport all players to child SCR_Position's picked at random.", category: "TELEPORT")]
	protected bool m_bEnableTeleport;

	[Attribute("3.0", UIWidgets.Slider, desc: "Random deviation radius from a chosen position.", category: "TELEPORT", params: "0 500 0.01")]
	protected float m_fTeleportRadius;

	[Attribute("15.0", UIWidgets.Slider, desc: "Search radius for safe spot from random chosen position.", category: "TELEPORT", params: "3 300 0.01")]
	protected float m_fSafetyRadius;

	// -------------------------------------------------------

	[Attribute(desc: "Show this hint on trigger, blank for none.", category: "UI")]
	protected ref SCR_HintUIInfo m_pCustomHint;

	[Attribute(desc: "Show this pop-up on trigger, blank for none.", category: "UI")]
	protected ref NO_SCR_PopupUIInfo m_pCustomPopup;

	// -------------------------------------------------------

	[Attribute("0", UIWidgets.CheckBox, desc: "End game on trigger activation!", category: "GAME OVER")]
	protected bool m_bEnableGameOver;

	[Attribute("EDITOR_FACTION_VICTORY", UIWidgets.ComboBox, desc: "Customize these on SCR_GameOverScreenManagerComponent on SCR_BaseGameMode.", category: "GAME OVER", enums: ParamEnumArray.FromEnum(ESupportedEndReasons))]
	protected int m_iGameOverType;

	[Attribute("US", UIWidgets.EditBox, desc: "Key of winning faction, or player faction if draw.", category: "GAME OVER")]
	protected string m_sWinningFactionKey;

	// -------------------------------------------------------

	[Attribute("", UIWidgets.Auto, desc: "Name of a NO_SCR_EditorTask to finish.", category: "LEGACY")]
	protected ref array<string> m_sFinishTaskNames;

	[Attribute("", UIWidgets.Auto, desc: "Name of a NO_SCR_EditorTask to unlock.", category: "LEGACY")]
	protected ref array<string> m_sUnlockTaskNames;

	// -------------------------------------------------------

	// Needed for RPC calls
	protected RplComponent m_pRplComponent;

	// Array of SCR_Position entities placed under this trigger
	protected ref array<SCR_Position> m_aChildPositions = {};

	// For forcing blackout effects on teleport
	protected SCR_ScreenEffects m_pLocalScreenEffects;


	override void EOnInit (IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;

		m_pRplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));

		if (!m_pRplComponent)
		{
			Print("SCR_GroupAndGoTrigger requires a RplComponent!", LogLevel.ERROR);
			return;
		}

		m_pRplComponent.EnableStreaming(false);

		IEntity child = GetChildren();
		while (child)
		{
			SCR_Position position = SCR_Position.Cast(child);

			if (position)
				m_aChildPositions.Insert(position);

			child = child.GetSibling();
		}

		if (m_pRplComponent.IsMaster() && !m_bIsActive)
			TaskStateActivationSetup();
	}


	// Called from base class when player quota in the trigger is reached.
	override protected event void OnPlayerQuotaReached()
	{
		super.OnPlayerQuotaReached();

		GetGame().GetCallqueue().CallLater(DelayedTrigger, m_fDelay * 1000, false);

		if (!m_bIsRepeatable)
			SetActive(false);
	}


	protected void DelayedTrigger()
	{
		if (m_pRplComponent.IsMaster())
		{
			ShowHintsOrPopups();

			NOSpawn();
			NODespawn();

			MakeSpawnpointChanges();

			MakeTaskStateChanges();
			FinishTask(); // LEGACY
			UnlockTask(); // LEGACY

			UpdateWaypoint();

			EndMissionSelections();

			MovePlayers();

			GetGame().GetCallqueue().CallLater(ChangeTimeWeather, FADE_OUT_BUFFER + 50, false);

			GameOver();
		}

		if (!m_bIsRepeatable)
			Deactivate();
	}


	protected void TaskStateActivationSetup()
	{
		if (!m_aActivateOnTaskState.IsEmpty())
			GetTaskManager().s_OnTaskUpdate.Insert(TaskStateActivationCheck);
	}


	protected void TaskStateActivationCheck(SCR_BaseTask task)
	{
		NO_SCR_EditorTask editorTask = NO_SCR_EditorTask.Cast(task);
		if (!editorTask)
			return;

		if (m_aActivateOnTaskState.IsEmpty())
			GetTaskManager().s_OnTaskUpdate.Remove(TaskStateActivationCheck);

		foreach(NO_SCR_TaskStateActivatorEntry activatorEntry : m_aActivateOnTaskState)
		{
			if (editorTask.GetName() != activatorEntry.GetTaskName())
				return;

			if (!editorTask.TaskState)
			{
				Print(string.Format("Null TaskState: %1", editorTask.GetName()), LogLevel.ERROR);
				return;
			}

			if (editorTask.TaskState == activatorEntry.GetTaskState())
			{
				SetActive(true);
				m_aActivateOnTaskState.RemoveItem(activatorEntry);
			}
		}
	}


	protected void ShowHintsOrPopups()
	{
		// Hints
		if (m_pCustomHint)
		{
			// Clients
			Rpc(RpcDo_Hint);

			// Also server if not dedicated (SP)
			if (RplSession.Mode() != RplMode.Dedicated)
				RpcDo_Hint();
		}

		// Popups
		if (m_pCustomPopup)
		{
			Rpc(RpcDo_Popup);

			if (RplSession.Mode() != RplMode.Dedicated)
				RpcDo_Popup();
		}
	}


	// Network friendly player UI hint
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_Hint()
	{
		SCR_HintManagerComponent hintComponent = SCR_HintManagerComponent.GetInstance();

		if (hintComponent)
			hintComponent.Show(m_pCustomHint);
	}


	// Network friendly player UI popup
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_Popup()
	{
		if (m_pCustomPopup)
			m_pCustomPopup.ShowPopup();
	}


	protected void NOSpawn()
	{
		foreach(string spawnTriggerName : m_sNOSpawnTriggerNames)
		{
			if (spawnTriggerName.IsEmpty())
				continue;

			NO_SCR_SpawnTrigger spawnTrigger = NO_SCR_SpawnTrigger.Cast(GetGame().GetWorld().FindEntityByName(spawnTriggerName));

			if (spawnTrigger)
				spawnTrigger.Spawn();
		}
	}


	protected void NODespawn()
	{
		foreach(string despawnTriggerName : m_sNODespawnTriggerNames)
		{
			if (despawnTriggerName.IsEmpty())
				continue;

			NO_SCR_SpawnTrigger despawnTrigger = NO_SCR_SpawnTrigger.Cast(GetGame().GetWorld().FindEntityByName(despawnTriggerName));

			if (despawnTrigger)
				despawnTrigger.Despawn();
		}
	}


	protected void FinishTask()
	{
		foreach (string taskName : m_sFinishTaskNames)
		{
			if (taskName.IsEmpty())
				continue;

			NO_SCR_EditorTask task = NO_SCR_EditorTask.Cast(GetGame().GetWorld().FindEntityByName(taskName));

			if (!task)
				continue;

			task.ChangeStateOfTask(TriggerType.Finish);
		}
	}


	protected void UnlockTask()
	{
		foreach (string taskName : m_sUnlockTaskNames)
		{
			if (taskName.IsEmpty())
				continue;

			NO_SCR_EditorTask task = NO_SCR_EditorTask.Cast(GetGame().GetWorld().FindEntityByName(taskName));

			if (!task)
				continue;

			task.ChangeStateOfTask(TriggerType.Create);
		}
	}


	protected void MakeTaskStateChanges()
	{
		foreach(NO_SCR_TaskStateChangeEntry changeEntry : m_aTaskStateChanges)
		{
			NO_SCR_EditorTask task = NO_SCR_EditorTask.Cast(GetGame().GetWorld().FindEntityByName(changeEntry.GetTaskName()));

			if (task)
				task.ChangeStateOfTask(changeEntry.GetTaskState());
		}
	}


	protected void UpdateWaypoint()
	{
		if (m_sWaypointLocationName.IsEmpty())
			return;

		IEntity waypointLocationEntity = GetGame().GetWorld().FindEntityByName(m_sWaypointLocationName);
		IEntity waypointEntity = GetGame().GetWorld().FindEntityByName(WAYPOINT_ENTITY_NAME);

		if (waypointLocationEntity && waypointEntity)
			waypointEntity.SetOrigin(waypointLocationEntity.GetOrigin());
	}


	protected void MakeSpawnpointChanges()
	{
		foreach(NO_SCR_SpawnpointChangeEntry spawnpointChangeEntry : m_aSpawnpointChanges)
		{
			if (spawnpointChangeEntry.GetSpawnpointName().IsEmpty())
				continue;

			SCR_SpawnPoint spawnpoint = SCR_SpawnPoint.Cast(GetGame().GetWorld().FindEntityByName(spawnpointChangeEntry.GetSpawnpointName()));

			if (spawnpoint)
				spawnpoint.SetFactionKey(spawnpointChangeEntry.GetFactionKey());
		}
	}


	protected void EndMissionSelections()
	{
		if (m_sMissionSelectionManagerName.IsEmpty() || m_aEndMissionSelections.IsEmpty())
			return;

		Rpc(RpcDo_EndMissionSelections);
		RpcDo_EndMissionSelections();
	}


	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_EndMissionSelections()
	{
		IEntity entity = GetGame().GetWorld().FindEntityByName(m_sMissionSelectionManagerName);
		if (!entity)
		{
			Print("MissionSelectionManager Not Found: " + m_sMissionSelectionManagerName, LogLevel.ERROR);
			return;
		}

		NO_SCR_MissionSelectionManagerComponent missionSelectionManager = NO_SCR_MissionSelectionManagerComponent.Cast(entity.FindComponent(NO_SCR_MissionSelectionManagerComponent));
		if (!missionSelectionManager)
			return;

		foreach (string missionSelectionName : m_aEndMissionSelections)
		{
			if (!missionSelectionManager.EndMission(missionSelectionName))
				Print("EndMissionSelection Name Not Found: " + missionSelectionName, LogLevel.ERROR);
		}
	}


	protected void ChangeTimeWeather()
	{
		if (!m_pChangeTimeAndWeather)
			return;

		m_pChangeTimeAndWeather.Execute();
	}


	protected void MovePlayers()
	{
		if (!m_bEnableTeleport)
			return;

		Rpc(RpcDo_Teleport);

		if (RplSession.Mode() != RplMode.Dedicated)
			RpcDo_Teleport();
	}


	// Each player is instructed to teleport themselves.
	// (Had issues when trying to move players directly by the server).
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	void RpcDo_Teleport()
	{
		// No teleport positions available
		if (m_aChildPositions.IsEmpty())
			return;

		// Pick from available positions
		SCR_Position positionEntity = m_aChildPositions.GetRandomElement();
		if (!positionEntity)
			return;

		BlackoutEffect(true, 4.45);

		// Delay for fadeout
		GetGame().GetCallqueue().CallLater(Teleport, FADE_OUT_BUFFER, false, positionEntity);
	}


	protected void Teleport(notnull SCR_Position positionEntity)
	{
		BlackoutEffect(true, 10);

		vector startPostion = positionEntity.GetOrigin();
		vector rotation = positionEntity.GetAngles();

		// Randomize the XZ components of the chosen SCR_Position
		float positionX = startPostion[0] + Math.RandomFloatInclusive(-m_fTeleportRadius, m_fTeleportRadius);
		float positionZ = startPostion[2] + Math.RandomFloatInclusive(-m_fTeleportRadius, m_fTeleportRadius);

		// Keep height component (may now be under terrain)
		vector suggestedPostion = {positionX, startPostion[1], positionZ};

		// Rotate the player
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (player)
			player.SetAngles(rotation);

		// Make sure the suggested position is safe (seems this also includes terrain height)
		vector foundPosition;
		SCR_WorldTools.FindEmptyTerrainPosition(foundPosition, suggestedPostion, m_fSafetyRadius);

		// Teleport there
		SCR_Global.TeleportPlayer(foundPosition);

		BlackoutEffect(false);
	}


	protected void BlackoutEffect(bool state = false, float strength = 0)
	{
		if (!m_pLocalScreenEffects)
		{
			SCR_PlayerController playerController = SCR_PlayerController.Cast(GetGame().GetPlayerController());
			if (!playerController)
				return;

			HUDManagerComponent playerHUD = playerController.GetHUDManagerComponent();
			if (!playerHUD)
				return;

			array<SCR_InfoDisplay> hudElements = playerHUD.GetHUDElements();

			foreach (SCR_InfoDisplay infoDisplay : hudElements)
			{
				SCR_ScreenEffects screenEffectsInfoDisplay = SCR_ScreenEffects.Cast(infoDisplay);

				if (screenEffectsInfoDisplay)
					m_pLocalScreenEffects = screenEffectsInfoDisplay;
			}
		}

		if (m_pLocalScreenEffects)
		{
			if (state)
				m_pLocalScreenEffects.BlackoutEffect(strength);
			else
			{
				m_pLocalScreenEffects.BlackoutEffect(0);
				m_pLocalScreenEffects = null;
			}
		}
	}


	protected void GameOver()
	{
		if (!m_bEnableGameOver)
			return;

		SCR_BaseGameMode gameMode = SCR_BaseGameMode.Cast(GetGame().GetGameMode());
		if (!gameMode)
			return;

		Faction winningFaction = GetGame().GetFactionManager().GetFactionByKey(m_sWinningFactionKey);
		if (!winningFaction)
			return;

		int winningFactionIndex = GetGame().GetFactionManager().GetFactionIndex(winningFaction);

		if (winningFactionIndex != -1)
			gameMode.EndGameMode(SCR_GameModeEndData.CreateSimple(m_iGameOverType, -1, winningFactionIndex));
	}


	void NO_SCR_MissionTrigger(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
}



enum ESupportedEndReasons
{
	UNDEFINED = -1,
	TIMELIMIT = -2,
	SCORELIMIT = -3,
	DRAW = -4,
	SERVER_RESTART = -5,

	EDITOR_NEUTRAL = 1000,
	EDITOR_FACTION_NEUTRAL = 1001,
	EDITOR_FACTION_VICTORY = 1002,
	//EDITOR_FACTION_DEFEAT = 1003,
	EDITOR_FACTION_DRAW = 1004
}



[BaseContainerProps(), BaseContainerCustomTitleField("m_sTaskEntityName")]
class NO_SCR_TaskStateActivatorEntry : Managed
{
	[Attribute("", UIWidgets.EditBox, desc: "Name of a task entity to watch.")]
	protected string m_sTaskEntityName;

	[Attribute(SCR_Enum.GetDefault(TriggerType.Finish), UIWidgets.ComboBox, desc: "State to watch for.", enums: ParamEnumArray.FromEnum(TriggerType))]
	protected TriggerType m_eTaskState;

	string GetTaskName()
	{
		return m_sTaskEntityName;
	}

	TriggerType GetTaskState()
	{
		return m_eTaskState;
	}
}



[BaseContainerProps(), BaseContainerCustomTitleField("m_sTaskEntityName")]
class NO_SCR_TaskStateChangeEntry
{
	[Attribute("", UIWidgets.EditBox, desc: "Name of a task entity to change.")]
	protected string m_sTaskEntityName;

	[Attribute(SCR_Enum.GetDefault(TriggerType.Finish), UIWidgets.ComboBox, desc: "New task state.", enums: ParamEnumArray.FromEnum(TriggerType))]
	protected TriggerType m_eTaskState;

	string GetTaskName()
	{
		return m_sTaskEntityName;
	}

	TriggerType GetTaskState()
	{
		return m_eTaskState;
	}
}



[BaseContainerProps(), BaseContainerCustomTitleField("m_sSpawnpointName")]
class NO_SCR_SpawnpointChangeEntry
{
	[Attribute("", UIWidgets.EditBox, desc: "Name of a SCR_SpawnPoint to modify.")]
	protected string m_sSpawnpointName;

	[Attribute("", UIWidgets.EditBox, desc: "Desired faction key for said SCR_SpawnPoint.")]
	protected string m_sFactionKey;

	string GetSpawnpointName()
	{
		return m_sSpawnpointName;
	}

	string GetFactionKey()
	{
		return m_sFactionKey;
	}
}



[BaseContainerProps(), BaseContainerCustomTitleField("m_sCustomPopupTitle")]
class NO_SCR_PopupUIInfo
{
	[Attribute("", UIWidgets.EditBox, desc: "Pop-up title.")]
	protected string m_sCustomPopupTitle;

	[Attribute("", UIWidgets.EditBox, desc: "Pop-up subtitle.")]
	protected string m_sCustomPopupSubtitle;

	[Attribute("4.0", UIWidgets.Slider, desc: "Pop-up duration.", params: "0.5 60 0.001")]
	protected float m_fCustomPopupDuration;

	[Attribute("0.5", UIWidgets.Slider, desc: "Pop-up fade in/out.", params: "0.1 10 0.001")]
	protected float m_fCustomPopupFade;

	[Attribute("-1", UIWidgets.Slider, desc: "Pop-up priority, (12+ should beat out base game pop-ups).", params: "-1 15")]
	protected int m_iCustomPopupPriority;

	void ShowPopup()
	{
		SCR_PopUpNotification popupEntity = SCR_PopUpNotification.GetInstance();

		if (popupEntity)
			popupEntity.PopupMsg(text: m_sCustomPopupTitle, text2: m_sCustomPopupSubtitle,
				duration: m_fCustomPopupDuration, fade: m_fCustomPopupFade, prio: m_iCustomPopupPriority);
	}
}