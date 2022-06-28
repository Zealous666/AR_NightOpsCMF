class NO_SCR_StartPatrolAction : ScriptedUserAction
{
	[Attribute(SCR_Enum.GetDefault(ENightOpsPatrolType.INTEL), UIWidgets.ComboBox, desc: "What patrol type should this action fire?", enums: ParamEnumArray.FromEnum(ENightOpsPatrolType))]
	protected ENightOpsPatrolType m_ePatrolType;

	[Attribute("0", UIWidgets.CheckBox, desc: "Makes this action ignore the Patrol Type setting, picks a type at random!")]
	protected bool m_bIsRandomPatrolType;

	// Asks proxy patrol manager if this action currently be performed
	override event bool CanBePerformedScript(IEntity user)
	{
		if (!super.CanBePerformedScript(user))
			return false;

		if (GetPatrolManager())
			return !GetPatrolManager().IsOnPatrol();

		return false;
	}

	// Asks proxy patrol manager if this action currently be shown
	override event bool CanBeShownScript(IEntity user)
	{
		if (!super.CanBeShownScript(user))
			return false;

		if (GetPatrolManager())
		{
			if (m_bIsRandomPatrolType)
				return GetPatrolManager().ArePatrolsAvailable();

			return GetPatrolManager().IsPatrolTypeAvailable(m_ePatrolType);
		}

		return false;
	}

	// Only perform the actual action on the server, clients still 'perform it'
	override event bool CanBroadcastScript() { return false; }

	// Peforms the actual action, server only due to override above (everywhere by default)
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.PerformAction(pOwnerEntity, pUserEntity);

		// Start this patrol type or random if checkbox is enabled
		if (m_bIsRandomPatrolType)
			GetPatrolManager().StartPatrol();
		else
			GetPatrolManager().StartPatrol(m_ePatrolType);
	}
}