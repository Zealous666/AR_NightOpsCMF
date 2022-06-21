[BaseContainerProps()]
class NO_SCR_SpawnpointDeployment : NO_SCR_DeploymentInterface
{
	[Attribute(defvalue: "", UIWidgets.EditBox, desc: "Key to set the spawnpoint to on deployment")]
	protected FactionKey m_sFactionKey;

	protected SCR_SpawnPoint m_pSpawnpoint;

	protected bool m_bFirstRun = true;

	override bool Deploy()
	{
		if (m_bFirstRun)
		{
			TryFindSpawnpoint();
			m_bFirstRun = false;
		}

		if (!m_pSpawnpoint)
			return false;

		m_pSpawnpoint.SetFactionKey(m_sFactionKey);
		return true;
	}

	override bool Undeploy()
	{
		if (!m_pSpawnpoint)
			return false;

		m_pSpawnpoint.SetFactionKey(string.Empty);
		return true;
	}

	protected void TryFindSpawnpoint()
	{
		IEntity child = GetAttachedEntity().GetChildren();
		while (child)
		{
			SCR_SpawnPoint spawnpoint = SCR_SpawnPoint.Cast(child);

			if (spawnpoint)
			{
				m_pSpawnpoint = spawnpoint;
				return;
			}

			child = child.GetSibling();
		}
	}
}