class NO_SCR_MissionSelectionAction : ScriptedUserAction
{
	[Attribute("", UIWidgets.EditBox, desc: "Text show when mission is active!")]
	protected string m_sOnMissionText;

	[Attribute("0", UIWidgets.CheckBox, desc: "Creates a special action that can reset the mission states!")]
	protected bool m_bIsResetAction;

	protected NO_SCR_MissionSelectionManagerComponent m_pMissionSelectionManagerComponent;


	// Initialisation, runs everywhere
	override event void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);

		if (!GetGame().InPlayMode())
			return;

		m_pMissionSelectionManagerComponent = NO_SCR_MissionSelectionManagerComponent.Cast(pOwnerEntity.FindComponent(NO_SCR_MissionSelectionManagerComponent));
		if (!m_pMissionSelectionManagerComponent)
		{
			Print("Attemping to use a MissionSelectionAction without a MissionSelectionManagerComponent!", LogLevel.ERROR);
			return;
		}

		if (!m_bIsResetAction)
		{
			SetCannotPerformReason("Finish current mission!");

			// Handled on next frame so that the component can also init
			GetGame().GetCallqueue().Call(AddActionToManager);
		}
	}


	protected void AddActionToManager()
	{
		m_pMissionSelectionManagerComponent.AddMissionSelectionAction(this);
	}


	// Asks manager component if this action can currently be performed
	override event bool CanBePerformedScript(IEntity user)
	{
		if (!m_pMissionSelectionManagerComponent)
			return false;

		if (m_bIsResetAction)
			return true;

		return m_pMissionSelectionManagerComponent.IsActionPerformable(this);
	}


	// Asks manager component if this action can currently be shown
	override event bool CanBeShownScript(IEntity user)
	{
		if (!m_pMissionSelectionManagerComponent)
			return true;

		if (m_bIsResetAction)
			return m_pMissionSelectionManagerComponent.CanReset();
		else
			return m_pMissionSelectionManagerComponent.IsActionShowable(this);
	}


	// Action name override for specific cases
	override event bool GetActionNameScript(out string outName)
	{
		if (!m_pMissionSelectionManagerComponent)
		{
			outName = "Missing MissionSelectionManagerComponent!";
			return true;
		}

		if (m_bIsResetAction)
		{
			outName = " %CTX_HACK%RESET ";
			return true;
		}

		return false;
	}

	// Peforms the action, runs everywhere by default
	override event void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_pMissionSelectionManagerComponent)
			return;

		if (m_bIsResetAction)
			m_pMissionSelectionManagerComponent.ResetState(true);
		else
		{
			m_pMissionSelectionManagerComponent.StartMission(this);

			if (!m_sOnMissionText.IsEmpty())
				SetCannotPerformReason(m_sOnMissionText);
		}
	}
}