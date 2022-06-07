[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class NO_SCR_MissionSelectionManagerComponentClass : ScriptComponentClass
{
}

class NO_SCR_MissionSelectionManagerComponent : ScriptComponent
{
	protected ref map<string, ref NO_SCR_MissionSelectionData> m_aMissionSelections = new ref map<string, ref NO_SCR_MissionSelectionData>;

	protected bool m_bInDefaultState = true;


	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
	override void EOnInit(IEntity owner)
	{
	}


	void AddMissionSelectionAction(notnull NO_SCR_MissionSelectionAction missionSelection)
	{
		NO_SCR_MissionSelectionData missionSelectionData = new NO_SCR_MissionSelectionData(missionSelection.GetActionName());

		m_aMissionSelections.Insert(missionSelection.GetActionName(), missionSelectionData);
	}


	bool IsActionPerformable(notnull NO_SCR_MissionSelectionAction selection)
	{
		NO_SCR_MissionSelectionData missionSelectionData;

		if (m_aMissionSelections.Find(selection.GetActionName(), missionSelectionData))
			return missionSelectionData.CanPerform();

		return false;
	}


	bool IsActionShowable(notnull NO_SCR_MissionSelectionAction selection)
	{
		NO_SCR_MissionSelectionData missionSelectionData;

		if (m_aMissionSelections.Find(selection.GetActionName(), missionSelectionData))
			return missionSelectionData.CanShow();

		return false;
	}


	void StartMission(notnull NO_SCR_MissionSelectionAction missionSelection)
	{
		string missionSelectionName =  missionSelection.GetActionName();

		foreach (string selectionKey, NO_SCR_MissionSelectionData selectionData : m_aMissionSelections)
		{
			if (selectionKey == missionSelectionName)
			{
				selectionData.SetPerfomable(false);
				selectionData.SetShowable(false);
				selectionData.SaveState();
			}
			else
			{
				selectionData.SetPerfomable(false);
			}
		}

		m_bInDefaultState = false;
	}


	void EndMission(string missionSelectionName = "")
	{
		if (!m_aMissionSelections.Contains(missionSelectionName))
			return;

		foreach (string selectionKey, NO_SCR_MissionSelectionData selectionData : m_aMissionSelections)
		{
			if (selectionKey == missionSelectionName)
			{
				selectionData.SetComplete(true);
				selectionData.SetShowable(false); // Just encase the manager is asked to complete a task that hasn't been picked!
			}
			else
				if (!selectionData.IsComplete())
					selectionData.SetPerfomable(true);
		}

		m_bInDefaultState = false;
	}


	bool CanReset()
	{
		return !m_bInDefaultState;
	}


	void ResetState()
	{
		foreach (string selectionKey, NO_SCR_MissionSelectionData selectionData : m_aMissionSelections)
		{
			selectionData.SetComplete(false);
			selectionData.SetPerfomable(true);
			selectionData.SetShowable(true);
		}

		m_bInDefaultState = true;
	}

	//------------------------------------------------------------------------------------------------
	void NO_SCR_MissionSelectionManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}
	void ~NO_SCR_MissionSelectionManagerComponent()
	{
	}
}


class NO_SCR_MissionSelectionData : JsonApiStruct
{
	private string m_sActionName;
	private bool m_bCanPerform;
	private bool m_bCanShow;
	private bool m_bIsComplete;

	void NO_SCR_MissionSelectionData(string actionName)
	{
		RegV("m_sActionName");
		RegV("m_bCanPerform");
		RegV("m_bCanShow");
		RegV("m_bIsComplete");

		m_sActionName = actionName;
		m_bCanPerform = true;
		m_bCanShow = true;
		m_bIsComplete = false;

		string saveName = "NO_SaveTesting";
		PackToFile("$profile:.save/" + saveName + ".json");
	}

	string GetName()
	{
		return m_sActionName;
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

	void SetComplete(bool state = true)
	{
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

	void SaveState()
	{
	}
}