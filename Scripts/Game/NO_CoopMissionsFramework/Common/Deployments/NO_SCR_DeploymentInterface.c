[BaseContainerProps(visible: false, insertable: false)]
class NO_SCR_DeploymentInterface
{
	private IEntity m_pOwner;

	private RplComponent m_pRplComponent;

	protected bool m_bServerOnly = true;

	void Init(IEntity owner, RplComponent rplComponent)
	{
		m_pOwner = owner;
		m_pRplComponent = rplComponent;
	}

	IEntity GetAttachedEntity()
	{
		return m_pOwner;
	}

	RplComponent GetRplComponent()
	{
		return m_pRplComponent;
	}

	bool IsServerOnly()
	{
		return m_bServerOnly;
	}

	bool Deploy() {}
	bool Undeploy() {}
}