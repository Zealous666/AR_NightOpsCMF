[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class NO_SCR_MissionSelectionManagerComponentClass : ScriptComponentClass
{
}

class NO_SCR_MissionSelectionManagerComponent : ScriptComponent
{
	[Attribute("NO_CMF_SaveFileExample.json", UIWidgets.EditBox, desc: "Filename for the save file for this mission selection manager!")]
	protected string m_sSaveFileName;

	[Attribute("0", UIWidgets.CheckBox,  desc: "Hide the active mission, or show it greyed out with the option to set custom mission text on the actions 'On Mission Text'")]
	protected bool m_bHideActiveMission;


	protected ref NO_SCR_MissionSelectionContainer m_pMissionSelectionContainer = new NO_SCR_MissionSelectionContainer();

	protected bool m_bInDefaultState = true;


	/*
	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}
	override void EOnInit(IEntity owner)
	{
	}
	void NO_SCR_MissionSelectionManagerComponent(IEntityComponentSource src, IEntity ent, IEntity parent)
	{
	}
	void ~NO_SCR_MissionSelectionManagerComponent()
	{
	}
	*/


	void AddMissionSelectionAction(notnull NO_SCR_MissionSelectionAction missionSelection)
	{
		m_pMissionSelectionContainer.AddMissionSelection(missionSelection.GetActionName());
	}


	bool IsActionPerformable(notnull NO_SCR_MissionSelectionAction selection)
	{
		NO_SCR_MissionSelectionData missionSelectionData = m_pMissionSelectionContainer.FindMissionSelection(selection.GetActionName());

		if (missionSelectionData)
			return missionSelectionData.CanPerform();

		return false;
	}


	bool IsActionShowable(notnull NO_SCR_MissionSelectionAction selection)
	{
		NO_SCR_MissionSelectionData missionSelectionData = m_pMissionSelectionContainer.FindMissionSelection(selection.GetActionName());

		if (missionSelectionData)
			return missionSelectionData.CanShow();

		return false;
	}


	void StartMission(notnull NO_SCR_MissionSelectionAction missionSelection)
	{
		string missionSelectionName =  missionSelection.GetActionName();

		foreach (string selectionKey, NO_SCR_MissionSelectionData selectionData : m_pMissionSelectionContainer.GetMissionSelections())
		{
			if (selectionKey == missionSelectionName)
			{
				selectionData.SetPerfomable(false);

				if (m_bHideActiveMission)
					selectionData.SetShowable(false);

				m_pMissionSelectionContainer.SetCurrentMission(missionSelectionName);
			}
			else
			{
				selectionData.SetPerfomable(false);
			}
		}

		if (!m_sSaveFileName.IsEmpty())
			m_pMissionSelectionContainer.SaveState(m_sSaveFileName);
	}


	bool EndMission(string missionSelectionName = "")
	{
		if (!m_pMissionSelectionContainer.GetMissionSelections().Contains(missionSelectionName))
			return false;

		foreach (string selectionKey, NO_SCR_MissionSelectionData selectionData : m_pMissionSelectionContainer.GetMissionSelections())
		{
			if (selectionKey == missionSelectionName)
			{
				selectionData.SetComplete(true);
				selectionData.SetShowable(false);
				m_pMissionSelectionContainer.SetCurrentMission(string.Empty);
			}
			else
				if (!selectionData.IsComplete())
					selectionData.SetPerfomable(true);
		}

		if (!m_sSaveFileName.IsEmpty())
			m_pMissionSelectionContainer.SaveState(m_sSaveFileName);

		m_bInDefaultState = false;
		return true;
	}


	bool CanReset()
	{
		return !m_bInDefaultState;
	}


	void ResetState()
	{
		foreach (string selectionKey, NO_SCR_MissionSelectionData selectionData : m_pMissionSelectionContainer.GetMissionSelections())
		{
			selectionData.SetComplete(false);
			selectionData.SetPerfomable(true);
			selectionData.SetShowable(true);
		}

		m_pMissionSelectionContainer.SetCurrentMission(string.Empty);

		if (!m_sSaveFileName.IsEmpty())
			m_pMissionSelectionContainer.SaveState(m_sSaveFileName);

		m_bInDefaultState = true;

		SCR_MissionHeader currentMissionHeader = SCR_MissionHeader.Cast(GetGame().GetMissionHeader());
		if (currentMissionHeader)
			GetGame().PlayMission(currentMissionHeader);
	}
}



class NO_SCR_MissionSelectionContainer : JsonApiStruct
{
	private ref map<string, ref NO_SCR_MissionSelectionData> m_mMissionSelectionsMap;

	private ref array<ref NO_SCR_MissionSelectionData> m_aMissionSelectionsArray;

	private string m_sCurrentMissionName;

	void NO_SCR_MissionSelectionContainer()
	{
		RegV("m_aMissionSelectionsArray");
		RegV("m_sCurrentMissionName");

		m_mMissionSelectionsMap = new ref map<string, ref NO_SCR_MissionSelectionData>;
		m_aMissionSelectionsArray = new ref array<ref NO_SCR_MissionSelectionData>();
	}

	protected void EstablishMap()
	{
		m_mMissionSelectionsMap.Clear();

		foreach (NO_SCR_MissionSelectionData selectionData : m_aMissionSelectionsArray)
			m_mMissionSelectionsMap.Insert(selectionData.GetName(), selectionData);
	}

	protected void EstablishArray()
	{
		m_aMissionSelectionsArray.Clear();

		foreach (string selectionKey, NO_SCR_MissionSelectionData selectionData : m_mMissionSelectionsMap)
			m_aMissionSelectionsArray.Insert(selectionData);
	}

	void SaveState(string saveName)
	{
		EstablishArray();
		//PackToFile("$profile:.save/" + saveName);
	}

	void LoadState(string saveName)
	{
	}

	map<string, ref NO_SCR_MissionSelectionData> GetMissionSelections()
	{
		return m_mMissionSelectionsMap;
	}

	void AddMissionSelection(string selectionActionName)
	{
		NO_SCR_MissionSelectionData missionSelectionData = new NO_SCR_MissionSelectionData(selectionActionName);
		m_mMissionSelectionsMap.Insert(selectionActionName, missionSelectionData);
	}

	NO_SCR_MissionSelectionData GetCurrentMission()
	{
		return FindMissionSelection(m_sCurrentMissionName);
	}

	void SetCurrentMission(string currentMissionActionName)
	{
		m_sCurrentMissionName = currentMissionActionName;
	}

	NO_SCR_MissionSelectionData FindMissionSelection(string selectionActionName)
	{
		NO_SCR_MissionSelectionData missionSelectionData;
		m_mMissionSelectionsMap.Find(selectionActionName, missionSelectionData);
		return missionSelectionData;
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
}