[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class NO_SCR_MissionSelectionManagerComponentClass : ScriptComponentClass
{
}

class NO_SCR_MissionSelectionManagerComponent : ScriptComponent
{
	// Attributes
	[Attribute("NO_CMF_SaveFileExample.json", UIWidgets.EditBox, desc: "Filename for the save file for this mission selection manager!", category: "MISSION SELECTION MANAGER")]
	protected string m_sSaveFileName;

	[Attribute("0", UIWidgets.CheckBox,  desc: "Hide the active mission, or show it greyed out with the option to set custom mission text on the actions 'On Mission Text'", category: "MISSION SELECTION MANAGER")]
	protected bool m_bHideActiveMission;

	[Attribute("0", UIWidgets.CheckBox, desc: "End game on all mission selections complete!", category: "GAME OVER")]
	protected bool m_bEnableGameOver;

	[Attribute("EDITOR_FACTION_VICTORY", UIWidgets.ComboBox, desc: "Customize these on SCR_GameOverScreenManagerComponent on SCR_BaseGameMode.", category: "GAME OVER", enums: ParamEnumArray.FromEnum(ESupportedEndReasons))]
	protected int m_iGameOverType;

	[Attribute("US", UIWidgets.EditBox, desc: "Key of winning faction, or player faction if draw.", category: "GAME OVER")]
	protected string m_sWinningFactionKey;


	// Member variables
	protected ref NO_SCR_MissionSelectionPersistence m_pMissionSelectionPersistence;

	protected RplComponent m_pRplComponent;


	// Synced variables (MissionSelections is handled by RplSave/RplLoad methods (bottom of class) since map is an unsupported prop type)
	protected ref map<int, ref NO_SCR_MissionSelectionActionState> m_mMissionSelections;

	[RplProp()]
	protected bool m_bInDefaultState = true;


	protected void SetDefaultState(bool state)
	{
		if (m_bInDefaultState != state)
		{
			m_bInDefaultState = state;
			Replication.BumpMe();
		}
	}


	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}


	override void EOnInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;

		m_mMissionSelections = new ref map<int, ref NO_SCR_MissionSelectionActionState>;

		m_pRplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!m_pRplComponent)
		{
			Print("NO_SCR_MissionSelectionManagerComponent requires an RplComponent on the same entity!", LogLevel.ERROR);
			return;
		}

		// Authority only beyond this point
		if (m_pRplComponent.Role() == RplRole.Proxy)
			return;

		SetDefaultState(true);

		// Get previous state from disk if available
		m_pMissionSelectionPersistence = new NO_SCR_MissionSelectionPersistence();
		ReloadState();
	}


	protected int GetActionNameHash(notnull NO_SCR_MissionSelectionAction selectionAction)
	{
		return selectionAction.GetActionName().Hash();
	}


	// Helper method for retreaving ActionState objects from map
	protected NO_SCR_MissionSelectionActionState FindMissionSelection(int selectionActionNameHash)
	{
		NO_SCR_MissionSelectionActionState actionState;
		m_mMissionSelections.Find(selectionActionNameHash, actionState);
		return actionState;
	}


	// Adding the missions to the map on initialisation
	void AddMissionSelectionAction(notnull NO_SCR_MissionSelectionAction selectionAction)
	{
		// Only the authority does this as clients get potentially updated copies through RplSave/RplLoad
		if (m_pRplComponent.Role() == RplRole.Proxy)
			return;

		string selectionActionName = selectionAction.GetActionName();
		int selectionActionNameHash = selectionActionName.Hash();

		// Checks if a mission has previously been completed according to persistence object
		// If so, its still added but in a completed state
		if (m_pMissionSelectionPersistence.GetCompletedMissions().Contains(selectionActionName))
		{
			m_mMissionSelections.Insert(selectionActionNameHash, new NO_SCR_MissionSelectionActionState(true));
			SetDefaultState(false);
		}
		else
			m_mMissionSelections.Insert(selectionActionNameHash, new NO_SCR_MissionSelectionActionState());
	}


	// For clients action UI (is a given missionSelection performable)
	bool IsActionPerformable(notnull NO_SCR_MissionSelectionAction selectionAction)
	{
		NO_SCR_MissionSelectionActionState actionState = FindMissionSelection(GetActionNameHash(selectionAction));

		if (actionState)
			return actionState.CanPerform();

		return false;
	}


	// For clients action UI (is a given missionSelection shown)
	bool IsActionShowable(notnull NO_SCR_MissionSelectionAction selectionAction)
	{
		NO_SCR_MissionSelectionActionState actionState = FindMissionSelection(GetActionNameHash(selectionAction));

		if (actionState)
			return actionState.CanShow();

		return false;
	}


	// For clients action UI (is reset action shown)
	bool CanReset()
	{
		return !m_bInDefaultState;
	}


	void StartMission(notnull NO_SCR_MissionSelectionAction selectionAction)
	{
		int selectionActionNameHash = GetActionNameHash(selectionAction);

		// All machines start their copy of this missionSelection
		foreach (int actionNameHash, NO_SCR_MissionSelectionActionState actionState : m_mMissionSelections)
		{
			if (actionNameHash == selectionActionNameHash)
			{
				actionState.SetPerfomable(false);

				if (m_bHideActiveMission)
					actionState.SetShowable(false);
			}
			else
				actionState.SetPerfomable(false);
		}
	}


	bool EndMission(string selectionActionName = string.Empty)
	{
		int selectionActionNameHash = selectionActionName.Hash();

		if (!m_mMissionSelections.Contains(selectionActionNameHash))
			return false;

		// All machines will end their copy of this missionSelection
		foreach (int actionNameHash, NO_SCR_MissionSelectionActionState actionState : m_mMissionSelections)
		{
			if (actionNameHash == selectionActionNameHash)
			{
				actionState.SetComplete(true);

				// But only authority will keep a persistent track of it
				if (m_pRplComponent.Role() == RplRole.Authority)
				{
					m_pMissionSelectionPersistence.AddCompletedMission(selectionActionName);

					if (AreAllMissionsComplete())
						GameOver();
				}
			}
			else
				if (!actionState.IsComplete())
					actionState.SetPerfomable(true);
		}

		// Authority state update
		if (m_pRplComponent.Role() == RplRole.Authority)
		{
			PersistState();
			SetDefaultState(false);
		}

		return true;
	}


	void ResetState(bool restartGame = false)
	{
		foreach (int actionNameHash, NO_SCR_MissionSelectionActionState actionState : m_mMissionSelections)
			actionState.SetComplete(false);

		// Clients should not be saving persistent progress
		if (m_pRplComponent.Role() == RplRole.Proxy)
			return;

		m_pMissionSelectionPersistence.ResetCompletedMissions();
		PersistState();

		SetDefaultState(true);

		if (!restartGame)
			return;

		// Attempts to reload the current scenario to reset world state
		SCR_MissionHeader currentMissionHeader = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		if (currentMissionHeader)
			GetGame().PlayMission(currentMissionHeader);
	}


	protected void ReloadState()
	{
		if (m_sSaveFileName.IsEmpty())
			return;

		if(m_pMissionSelectionPersistence.LoadState(m_sSaveFileName))
			Print("Save file loaded!", LogLevel.NORMAL);
		else
			Print("Save file not found!", LogLevel.WARNING);
	}


	protected void PersistState()
	{
		if (!m_sSaveFileName.IsEmpty())
			m_pMissionSelectionPersistence.SaveState(m_sSaveFileName);
	}


	protected bool AreAllMissionsComplete()
	{
		foreach (int actionNameHash, NO_SCR_MissionSelectionActionState actionState : m_mMissionSelections)
		{
			if (!actionState.IsComplete())
				return false;
		}
		return true;
	}


	protected void GameOver()
	{
		if (m_pRplComponent.Role() == RplRole.Proxy)
			return;

		if (!m_bEnableGameOver)
			return;

		ResetState();

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


	// Write current state for JIP clients
	override bool RplSave(ScriptBitWriter writer)
	{
		// Write missionSelectionsCount
		writer.WriteInt(m_mMissionSelections.Count());

		foreach (int actionNameHash, NO_SCR_MissionSelectionActionState actionState : m_mMissionSelections)
		{
			// Write action name hash
			writer.WriteInt(actionNameHash);

			// Write action state
			writer.WriteBool(actionState.CanPerform());
			writer.WriteBool(actionState.CanShow());
			writer.WriteBool(actionState.IsComplete());
		}

		return true;
	}


	// Client JIP sync
	override bool RplLoad(ScriptBitReader reader)
	{
		// Read missionSelectionsCount
		int missionSelectionsCount = 0;
		if (!reader.ReadInt(missionSelectionsCount))
			return false;

		// For the number of MissionSelectionActions
		for (int i = 0; i < missionSelectionsCount; i++)
		{
			// Read action name hash
			int actionNameHash;
			if (!reader.ReadInt(actionNameHash))
				return false;

			// Read action state
			bool canPerform = false;
			if (!reader.ReadBool(canPerform))
				return false;

			bool canShow = false;
			if (!reader.ReadBool(canShow))
				return false;

			bool isComplete = true;
			if (!reader.ReadBool(isComplete))
				return false;

			// Create an actionState to match actual current state
			NO_SCR_MissionSelectionActionState actionState = new NO_SCR_MissionSelectionActionState(isComplete);

			if (!isComplete)
			{
				actionState.SetPerfomable(canPerform);
				actionState.SetShowable(canShow);
			}

			// Insert it to the clients initialised copy of the state map
			m_mMissionSelections.Insert(actionNameHash, actionState);
		}

		return true;
	}
}



class NO_SCR_MissionSelectionActionState : Managed
{
	private bool m_bCanPerform;
	private bool m_bCanShow;
	private bool m_bIsComplete;

	void NO_SCR_MissionSelectionActionState(bool previouslyCompleted = false)
	{
		m_bCanPerform = !previouslyCompleted;
		m_bCanShow = !previouslyCompleted;
		m_bIsComplete = previouslyCompleted;
	}

	bool IsComplete()
	{
		return m_bIsComplete;
	}

	bool CanPerform()
	{
		return m_bCanPerform;
	}

	bool CanShow()
	{
		return m_bCanShow;
	}

	void SetComplete(bool state)
	{
		m_bCanPerform = !state;
		m_bCanShow = !state;
		m_bIsComplete = state;
	}

	void SetPerfomable(bool state)
	{
		m_bCanPerform = state;
	}

	void SetShowable(bool state)
	{
		m_bCanShow = state;
	}
}



class NO_SCR_MissionSelectionPersistence : JsonApiStruct
{
	private ref array<string> m_aCompletedMissions;

	void NO_SCR_MissionSelectionPersistence()
	{
		RegV("m_aCompletedMissions");
		m_aCompletedMissions = new array<string>();
	}

	bool AddCompletedMission(string missionSelectionName)
	{
		return m_aCompletedMissions.Insert(missionSelectionName);
	}

	array<string> GetCompletedMissions()
	{
		return m_aCompletedMissions;
	}

	void ResetCompletedMissions()
	{
		m_aCompletedMissions.Clear();
	}

	bool LoadState(string saveName)
	{
		return LoadFromFile("$profile:.save/" + saveName);
	}

	void SaveState(string saveName)
	{
		PackToFile("$profile:.save/" + saveName);
	}
}