[BaseContainerProps()]
class NO_SCR_HintDeployment : NO_SCR_DeploymentInterface
{
	[Attribute(desc: "Show this hint on deploy, blank for none.")]
	protected ref SCR_HintUIInfo m_pDeployHint;

	[Attribute(desc: "Show this hint on undeploy, blank for none.")]
	protected ref SCR_HintUIInfo m_pUndeployHint;

	override void Init(IEntity owner, RplComponent rplComponent)
	{
		super.Init(owner, rplComponent);
		m_bServerOnly = false;
	}

	override bool Deploy()
	{
		if (!m_pDeployHint)
			return true;

		if (RplSession.Mode() != RplMode.Dedicated)
			Hint(true);

		return true;
	}

	override bool Undeploy()
	{
		if (!m_pUndeployHint)
			return true;

		if (RplSession.Mode() != RplMode.Dedicated)
			Hint(false);

		return true;
	}

	protected void Hint(bool isDeployHint)
	{
		SCR_HintManagerComponent hintComponent = SCR_HintManagerComponent.GetInstance();

		if (!hintComponent)
			return;

		if (isDeployHint)
			hintComponent.Show(m_pDeployHint);
		else
			hintComponent.Show(m_pUndeployHint);
	}
}