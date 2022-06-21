class NO_SCR_DeploymentAction : ScriptedUserAction
{
	[Attribute("DEPLOY: ", desc: "Renames the 'DEPLOY: ' part of the action.")]
	protected string m_sDeployText;

	[Attribute("UNDEPLOY: ", desc: "Renames the 'UNDEPLOY: ' part of the action.")]
	protected string m_sUndeployText;


	protected NO_SCR_DeploymentComponent m_pDeploymentComponent;

	protected string m_sDefaultActionName;


	// Initialisation, runs everywhere
	override event void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		super.Init(pOwnerEntity, pManagerComponent);

		if (!GetGame().InPlayMode())
			return;

		m_sDefaultActionName = GetUIInfo().GetName();

		m_pDeploymentComponent = NO_SCR_DeploymentComponent.Cast(pOwnerEntity.FindComponent(NO_SCR_DeploymentComponent));
		if (!m_pDeploymentComponent)
		{
			Print("Attemping to use a DeploymentAction without a DeploymentComponent!", LogLevel.ERROR);
			return;
		}
	}

	// Asks manager component if this action can currently be performed
	override event bool CanBePerformedScript(IEntity user)
	{
		if (!m_pDeploymentComponent)
			return false;

		return m_pDeploymentComponent.IsActionPerformable();
	}

	// Asks manager component if this action can currently be shown
	override event bool CanBeShownScript(IEntity user)
	{
		if (!m_pDeploymentComponent)
			return true;

		return m_pDeploymentComponent.IsActionShowable();
	}

	// Action name override for specific cases
	override event bool GetActionNameScript(out string outName)
	{
		if (!m_pDeploymentComponent)
		{
			outName = "Missing DeploymentComponent!";
			return true;
		}
		else
		{
			if (m_pDeploymentComponent.IsCurrentlyDeployed())
				outName = m_sUndeployText + "%CTX_HACK%" + m_sDefaultActionName;
			else
				outName = m_sDeployText + "%CTX_HACK%" + m_sDefaultActionName;

			return true;
		}

		return false;
	}

	// Only perform the actual action on the server, clients still 'perform it'
	override event bool CanBroadcastScript()
	{
		return false;
	}

	// Peforms the actual action, server only due to override above (everywhere by default)
	override event void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (m_pDeploymentComponent)
			m_pDeploymentComponent.ToggleDeployment();
	}
}