#include "Scripts/NO_CustomDefines.c"

[EntityEditorProps(category: "GameScripted/Triggers", description: "ScriptWizard generated script file.")]
class NO_SCR_MissionTriggerClass : NO_SCR_PlayerTriggerEntityClass
{
}

class NO_SCR_MissionTrigger : NO_SCR_PlayerTriggerEntity
{
	protected const string WAYPOINT_ENTITY_NAME = "WP";
	protected const int FADE_OUT_BUFFER = 2000;

	// -------------------------------------------------------

	[Attribute("0", UIWidgets.CheckBox, desc: "If enabled, will teleport all players to child SCR_Position's picked at random.", category: "TELEPORT")]
	protected bool m_bEnableTeleport;

	[Attribute("3.0", UIWidgets.Slider, desc: "How far should we look for a safe spot from chosen position.", category: "TELEPORT", params: "0 1000 0.01")]
	protected float m_fSafetyRadius;

	// -------------------------------------------------------

	[Attribute("0", UIWidgets.CheckBox, desc: "If enabled, mission trigger will not disable after first firing!", category: "MISSION CHANGES")]
	protected bool m_bIsRepeatable;

	[Attribute("", UIWidgets.EditBox, desc: "Name of NO_SpawnTriggers to spawn.", category: "MISSION CHANGES")]
	protected ref array<string> m_sNOSpawnTriggerNames;

	[Attribute("", UIWidgets.EditBox, desc: "Name of NO_SpawnTriggers to despawn.", category: "MISSION CHANGES")]
	protected ref array<string> m_sNODespawnTriggerNames;

	[Attribute(desc: "Spawnpoint changes to make.", category: "MISSION CHANGES")]
	protected ref array<ref NO_SCR_SpawnpointChangeEntry> m_aSpawnpointChanges;

	[Attribute("", UIWidgets.EditBox, desc: "Name of a NO_SCR_EditorTask to finish.", category: "MISSION CHANGES")]
	protected ref array<string> m_sFinishTaskNames;

	[Attribute("", UIWidgets.EditBox, desc: "Name of a NO_SCR_EditorTask to unlock.", category: "MISSION CHANGES")]
	protected ref array<string> m_sUnlockTaskNames;

	[Attribute("", UIWidgets.EditBox, desc: "Name of an entity with a MissionSelectionManagerComponent.", category: "MISSION CHANGES")]
	protected string m_sMissionSelectionManagerName;

	[Attribute("", UIWidgets.EditBox, desc: "Name of MissionSelectionAction's to end on the manager above.", category: "MISSION CHANGES")]
	protected ref array<string> m_aEndMissionSelections;

	[Attribute("", UIWidgets.EditBox, desc: "Name of a waypoint location to move the waypoint marker.", category: "MISSION CHANGES")]
	protected string m_sWaypointLocationName;

	[Attribute(desc: "Time/weather changes to make.", category: "MISSION CHANGES")]
	protected ref NO_SCR_ForceTimeAndWeatherEntry m_pChangeTimeAndWeather;

	// -------------------------------------------------------

	[Attribute("0", UIWidgets.CheckBox, desc: "End game on trigger activation!", category: "GAME OVER")]
	protected bool m_bEnableGameOver;

	[Attribute("EDITOR_FACTION_VICTORY", UIWidgets.ComboBox, desc: "Customize these on SCR_GameOverScreenManagerComponent on SCR_BaseGameMode.", category: "GAME OVER", enums: ParamEnumArray.FromEnum(ESupportedEndReasons))]
	protected int m_iGameOverType;

	[Attribute("US", UIWidgets.EditBox, desc: "Key of winning faction, or player faction if draw.", category: "GAME OVER")]
	protected string m_sWinningFactionKey;

	// -------------------------------------------------------

	// Needed for RPC calls
	protected RplComponent m_pRplComponent;

	// Array of SCR_Position entities placed under this trigger
	protected ref array<SCR_Position> m_aChildPositions = {};

	// For forcing blackout effects on teleport
	protected SCR_ScreenEffects m_pLocalScreenEffects;


	override void EOnInit (IEntity owner)
	{
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
	}


	// Called from base class when player quota in the trigger is reached.
	override protected event void OnPlayerQuotaReached()
	{
		super.OnPlayerQuotaReached();

		if (m_pRplComponent.IsMaster())
		{
			NOSpawn();
			NODespawn();

			MakeSpawnpointChanges();

			FinishTask();
			UnlockTask();
			UpdateWaypoint();

			EndMissionSelections();

			MovePlayers();

			GetGame().GetCallqueue().CallLater(ChangeTimeWeather, FADE_OUT_BUFFER + 50, false);

			GameOver();
		}

		if (!m_bIsRepeatable)
		{
			SetActive(false);
			Deactivate();
		}
	}


	protected void NOSpawn()
	{
		#ifdef HAS_DYNAMIC_SPAWN_DEPENDENCY
		foreach(string spawnTriggerName : m_sNOSpawnTriggerNames)
		{
			if (spawnTriggerName.IsEmpty())
				continue;

			NO_SCR_SpawnTrigger spawnTrigger = NO_SCR_SpawnTrigger.Cast(GetGame().GetWorld().FindEntityByName(spawnTriggerName));

			if (spawnTrigger)
				spawnTrigger.Spawn();
		}
		#else
		if (!m_sNOSpawnTriggerNames.IsEmpty())
			Print("Missing 'NightOps - DynamicSpawnFramework' dependency!", LogLevel.ERROR);
		#endif
	}


	protected void NODespawn()
	{
		#ifdef HAS_DYNAMIC_SPAWN_DEPENDENCY
		foreach(string despawnTriggerName : m_sNODespawnTriggerNames)
		{
			if (despawnTriggerName.IsEmpty())
				continue;

			NO_SCR_SpawnTrigger despawnTrigger = NO_SCR_SpawnTrigger.Cast(GetGame().GetWorld().FindEntityByName(despawnTriggerName));

			if (despawnTrigger)
				despawnTrigger.Despawn();
		}
		#else
		if (!m_sNODespawnTriggerNames.IsEmpty())
			Print("Missing 'NightOps - DynamicSpawnFramework' dependency!", LogLevel.ERROR);
		#endif
	}


	protected void FinishTask()
	{
		#ifdef HAS_DYNAMIC_TASK_DEPENDENCY
		foreach (string taskName : m_sFinishTaskNames)
		{
			if (taskName.IsEmpty())
				continue;

			NO_SCR_EditorTask task = NO_SCR_EditorTask.Cast(GetGame().GetWorld().FindEntityByName(taskName));

			if (!task)
				continue;

			task.ChangeStateOfTask(TriggerType.Finish);
		}
		#else
		if (!m_sFinishTaskNames.IsEmpty())
			Print("Missing 'NightOps - DynamicTaskFramework' dependency!", LogLevel.ERROR);
		#endif
	}


	protected void UnlockTask()
	{
		#ifdef HAS_DYNAMIC_TASK_DEPENDENCY
		foreach (string taskName : m_sUnlockTaskNames)
		{
			if (taskName.IsEmpty())
				continue;

			NO_SCR_EditorTask task = NO_SCR_EditorTask.Cast(GetGame().GetWorld().FindEntityByName(taskName));

			if (!task)
				continue;

			task.ChangeStateOfTask(TriggerType.Create);
		}
		#else
		if (!m_sUnlockTaskNames.IsEmpty())
			Print("Missing 'NightOps - DynamicTaskFramework' dependency!", LogLevel.ERROR);
		#endif
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
		if (m_aSpawnpointChanges.Count() == 0)
			return;

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

		vector suggestedPostion = positionEntity.GetOrigin();
		vector rotation = positionEntity.GetAngles();

		// Rotate the player
		IEntity player = SCR_PlayerController.GetLocalControlledEntity();
		if (player)
			player.SetAngles(rotation);

		// Make sure the suggested position is safe
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


[BaseContainerProps(), BaseContainerCustomTitleField("m_sSpawnpointName")]
class NO_SCR_ForceTimeAndWeatherEntry
{
	//! If enabled custom date will be used on session start. Authority only.
	[Attribute(defvalue: "0", desc: "If enabled, custom date will be used. Authority only.", category: "ON/OFF")]
	protected bool m_bUseCustomDate;

	//! If enabled custom time of the day will be used on session start. Authority only.
	[Attribute(defvalue: "0", desc: "If enabled, custom time of the day will be used. Authority only.", category: "ON/OFF")]
	protected bool m_bUseCustomTime;

	//! If enabled custom weather Id will be used on session start. Authority only.
	[Attribute(defvalue: "0", desc: "If enabled, custom weather Id will be used. Authority only.", category: "ON/OFF")]
	protected bool m_bUseCustomWeather;

	//! If enabled custom weather Id will be used on session start. Authority only.
	[Attribute(defvalue: "0", desc: "If enabled, custom Latitude/Longitude will be used. Authority only.", category: "ON/OFF")]
	protected bool m_bUseCustomLatitudeLongitude;

	//! Year set on game start. Authority only.
	[Attribute(defvalue: "1989", UIWidgets.Slider, desc: "Year set on game start. Authority only.", category: "SETTINGS", params: "1900 2200 1")]
	protected int m_iCustomYear;

	//! Month set on game start. Authority only.
	[Attribute(defvalue: "7", UIWidgets.Slider, desc: "Month set on game start. Authority only.", category: "SETTINGS", params: "1 12 1")]
	protected int m_iCustomMonth;

	//! Day set on game start. Authority only.
	[Attribute(defvalue: "24", UIWidgets.Slider, desc: "Day set on game start. Authority only.", category: "SETTINGS", params: "1 31 1")]
	protected int m_iCustomDay;

	//! Time of the day set on game start. Authority only.
	[Attribute(defvalue: "9.6", UIWidgets.Slider, desc: "Time of the day set on game start. Authority only.", category: "SETTINGS", params: "0 24 0.01")]
	protected float m_fCustomTimeOfTheDay;

	//! Weather IDs are the same as used in the TimeAndWeatherManager. Weather set on game start. Authority only.
	[Attribute(defvalue: "Clear", UIWidgets.ComboBox, desc: "Weather set on game start. Authority only.", category: "SETTINGS", enums: { ParamEnum("Clear", "Clear"), ParamEnum("Cloudy", "Cloudy"), ParamEnum("Overcast", "Overcast"), ParamEnum("Rainy", "Rainy") })]
	protected string m_sCustomWeather;

	//! Latitude set on game start. Authority only.
	[Attribute(defvalue: "-4", UIWidgets.Slider, desc: "Latitude set on game start. Authority only.", category: "SETTINGS", params: "-90 90 0.01")]
	protected float m_fCustomLatitude;

	//! Longitude set on game start. Authority only.
	[Attribute(defvalue: "71", UIWidgets.Slider, desc: "Longitude set on game start. Authority only.", category: "SETTINGS", params: "-180 180 0.01")]
	protected float m_fCustomLongitude;

	//! Reference to entity responsible for managing time and weather
	protected TimeAndWeatherManagerEntity m_pTimeAndWeatherManager;

	void NO_SCR_ForceTimeAndWeatherEntry()
	{
		m_pTimeAndWeatherManager = GetGame().GetTimeAndWeatherManager();
		if (!m_pTimeAndWeatherManager)
			Print("Cannot initialize TimeAndWeatherManagerEntity not found!", LogLevel.ERROR);
	}

	void Execute()
	{
		if (!m_pTimeAndWeatherManager)
			return;

		if (m_bUseCustomDate)
			SetDate(m_iCustomYear, m_iCustomMonth, m_iCustomDay);

		if (m_bUseCustomTime)
			SetTimeOfTheDay(m_fCustomTimeOfTheDay);

		if (m_bUseCustomWeather)
			SetWeather(m_sCustomWeather);

		if (m_bUseCustomLatitudeLongitude)
			SetLatLong(m_fCustomLatitude, m_fCustomLongitude);
	}

	// Forcefully sets time of the date to provided value. Authority only.
	protected void SetDate(int year, int month, int day)
	{
		m_pTimeAndWeatherManager.SetDate(year, month, day, true);
	}

	// Forcefully sets time of the day to provided value. Authority only.
	protected void SetTimeOfTheDay(float timeOfTheDay)
	{
		m_pTimeAndWeatherManager.SetTimeOfTheDay(timeOfTheDay, true);
	}

	// Forcefully sets weather to provided weatherId. Authority only.
	protected void SetWeather(string weatherId)
	{
		if (weatherId.IsEmpty())
			return;

		m_pTimeAndWeatherManager.ForceWeatherTo(true, weatherId, 0.0);
	}

	// Forcefully sets latitude/longitude to provided values. Authority only.
	protected void SetLatLong(float latitude, float longitude)
	{
		m_pTimeAndWeatherManager.SetCurrentLatitude(latitude);
		m_pTimeAndWeatherManager.SetCurrentLongitude(longitude);
	}
}