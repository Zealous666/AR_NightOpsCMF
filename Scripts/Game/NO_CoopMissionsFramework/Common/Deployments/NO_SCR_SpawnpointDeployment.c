[BaseContainerProps()]
class NO_SCR_SpawnpointDeployment : NO_SCR_DeploymentInterface
{
	[Attribute(desc: "Spawnpoint offset from deployable entity, to avoid being inside it.", params: "inf inf 0 purposeCoords spaceEntity")]
	protected vector m_vSpawnpointOffset;

	//-----------------------------------------------------------------------
	// Spawnpoint attributes as seen in SCR_SpawnPoint
	//-----------------------------------------------------------------------
	[Attribute("0", desc: "Find empty position for spawning within given radius. When none is found, entity position will be used.")]
	protected float m_fSpawnRadius;

	[Attribute("2", UIWidgets.EditBox, "Determines how close a player has to be to disable this spawn point.")]
	private float m_fNoPlayerRadius;

	[Attribute("0", UIWidgets.EditBox, "Determines how close a player looking at the spawn point has to be to disable it.")]
	private float m_fNoSightRadius;

	[Attribute("US", UIWidgets.EditBox, "Determines which faction can spawn on this spawn point.")]
	private string m_sFaction;

	[Attribute("0")]
	protected bool m_bShowInDeployMapOnly;

	[Attribute()]
	protected ref SCR_UIInfo m_Info;
	//-----------------------------------------------------------------------

	protected ref Resource m_pResource;

	protected SCR_SpawnPoint m_pSpawnpoint;

	override bool Deploy()
	{
		// No resource loaded, report a failure to deploy
		if (!m_pResource || !m_pResource.IsValid())
			return false;

		// Spawn params
		EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;
		GetAttachedEntity().GetTransform(params.Transform);
		params.Transform[3] = params.Transform[3] + GetAttachedEntity().VectorToParent(m_vSpawnpointOffset);

		// Do spawn
		m_pSpawnpoint = SCR_SpawnPoint.Cast(GetGame().SpawnEntityPrefab(m_pResource, GetAttachedEntity().GetWorld(), params));

		// Return if successful
		return m_pSpawnpoint;
	}

	override bool Undeploy()
	{
		if (!m_pSpawnpoint)
			return false;

		SCR_EntityHelper.DeleteEntityAndChildren(m_pSpawnpoint);
		return true;
	}

	protected void LoadAndModifySpawnResource()
	{
		// Load the resource
		m_pResource = Resource.Load("{E7F4D5562F48DDE4}Prefabs/MP/Spawning/SpawnPoint_Base.et");
		if (!m_pResource)
			return;

		// Modify the resource with the settings of this deployment
		BaseContainer baseContainer = m_pResource.GetResource().ToBaseContainer();
		if (baseContainer)
		{
			baseContainer.Set("m_fSpawnRadius", m_fSpawnRadius);
			baseContainer.Set("m_fNoPlayerRadius", m_fNoPlayerRadius);
			baseContainer.Set("m_fNoSightRadius", m_fNoSightRadius);
			baseContainer.Set("m_sFaction", m_sFaction);
			baseContainer.Set("m_bShowInDeployMapOnly", m_bShowInDeployMapOnly);
			baseContainer.Set("m_Info", m_Info);
		}
	}

	void NO_SCR_SpawnpointDeployment()
	{
		LoadAndModifySpawnResource();
	}

	void ~NO_SCR_SpawnpointDeployment()
	{
		if (m_pResource && m_pResource.IsValid())
			m_pResource.GetResource().Release();
	}
}