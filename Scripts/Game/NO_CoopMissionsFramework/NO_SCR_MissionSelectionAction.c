class NO_SCR_MissionSelectionAction : ScriptedUserAction
{
	[Attribute("0", UIWidgets.CheckBox, desc: "Creates a special action that can reset the mission states!")]
	protected bool m_bIsResetAction;

	protected NO_SCR_MissionSelectionManagerComponent m_pMissionSelectionManagerComponent;

	override event void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		m_pMissionSelectionManagerComponent = NO_SCR_MissionSelectionManagerComponent.Cast(pOwnerEntity.FindComponent(NO_SCR_MissionSelectionManagerComponent));

		if (!m_pMissionSelectionManagerComponent)
		{
			Print("Attemping to use a MissionSelectionAction without a MissionSelectionManagerComponent!", LogLevel.ERROR);
			return;
		}

		if (!m_bIsResetAction)
		{
			m_pMissionSelectionManagerComponent.AddMissionSelectionAction(this);
			SetCannotPerformReason("Finish current mission!");
		}
	}

	override event bool CanBePerformedScript(IEntity user)
	{
		if (!m_pMissionSelectionManagerComponent)
			return false;

		if (m_bIsResetAction)
			return true;

		return m_pMissionSelectionManagerComponent.IsActionPerformable(this);
	}

	override event bool CanBeShownScript(IEntity user)
	{
		if (!m_pMissionSelectionManagerComponent)
			return true;

		if (m_bIsResetAction)
			return m_pMissionSelectionManagerComponent.CanReset();
		else
			return m_pMissionSelectionManagerComponent.IsActionShowable(this);
	}

	override event bool GetActionNameScript(out string outName)
	{
		if (m_pMissionSelectionManagerComponent)
			return false;

		outName = "Missing MissionSelectionManagerComponent!";
		return true;
	}

	override event void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!m_pMissionSelectionManagerComponent)
			return;

		if (m_bIsResetAction)
			m_pMissionSelectionManagerComponent.ResetState();
		else
			m_pMissionSelectionManagerComponent.StartMission(this);
	}
}