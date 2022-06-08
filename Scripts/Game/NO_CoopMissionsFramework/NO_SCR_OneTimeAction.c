class NO_SCR_OneTimeAction : ScriptedUserAction
{
	[Attribute("1", UIWidgets.CheckBox, desc: "Only allows the action to be used one time.")]
	protected bool m_bOneTime;

	protected bool m_bHasFired = false;

	override event bool CanBePerformedScript(IEntity user)
	{
		if (!super.CanBePerformedScript(user))
			return false;

		if (!m_bOneTime || !m_bHasFired)
			return true;

		return false;
	}

	override event bool CanBeShownScript(IEntity user)
	{
		if (!super.CanBeShownScript(user))
			return false;

		if (!m_bOneTime || !m_bHasFired)
			return true;

		return false;
	}

	override event void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.PerformAction(pOwnerEntity, pUserEntity);
		m_bHasFired = true;
	}
}