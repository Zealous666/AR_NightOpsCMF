[BaseContainerProps()]
class NO_SCR_SpawnpointDeployment : NO_SCR_DeploymentInterface
{
	[Attribute(desc: "Spawnpoint offset from deployable entity, to avoid being inside it.", params: "inf inf 0 purposeCoords spaceEntity")]
	protected vector m_vSpawnpointOffset;

	[Attribute("{911857823BB92DAC}Prefabs/MP/Spawning/CommandVehicleSpawnpoint_US.et", UIWidgets.ResourceNamePicker, desc: "Spawnpoint prefab to spawn/despawn.", params: "et")]
	protected ResourceName m_rnSpawnpointPrefab;

	protected ref Resource m_pResource;
	protected SCR_SpawnPoint m_pSpawnpoint;

	override bool Deploy()
	{
		// Using a prefab
		if (m_rnSpawnpointPrefab && !m_rnSpawnpointPrefab.IsEmpty())
		{
			// Try load prefab, return if failed
			if (!TryLoadResourceName(m_rnSpawnpointPrefab))
			{
				Print(string.Format("Resource '%1', was unable to be loaded!", m_rnSpawnpointPrefab), LogLevel.ERROR);
				return false;
			}
		}
		else
		{
			// Not filled out correctly
			Print("Spawnpoint ResouceName is missing/empty!", LogLevel.ERROR);
			return false;
		}

		// Spawn location
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

	protected bool TryLoadResourceName(ResourceName resourceName)
	{
		if (!m_pResource || !m_pResource.IsValid())
			m_pResource = Resource.Load(resourceName);

		return m_pResource && m_pResource.IsValid();
	}

	void ~NO_SCR_SpawnpointDeployment()
	{
		if (m_pResource && m_pResource.IsValid())
		{
			m_pResource.GetResource().Release();
			m_pResource = null;
		}
	}
}