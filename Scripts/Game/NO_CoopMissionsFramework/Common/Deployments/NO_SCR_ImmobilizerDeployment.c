[BaseContainerProps()]
class NO_SCR_ImmobilizerDeployment : NO_SCR_DeploymentInterface
{
	[Attribute("0", UIWidgets.CheckBox, desc: "Flips the immobilizer so that when deployed the vehicle can move.")]
	protected bool m_bFlipImmobilizer;

	protected DamageManagerComponent m_pDamageManagerComponent;

	protected float m_fLastEngineHealth;

	protected bool m_bFirstRun = true;

	override bool Deploy()
	{
		if (m_bFirstRun)
		{
			Vehicle vehicle = Vehicle.Cast(GetAttachedEntity());
			if (!vehicle)
				return false;

			m_pDamageManagerComponent = DamageManagerComponent.Cast(GetAttachedEntity().FindComponent(DamageManagerComponent));
		}

		if (!m_pDamageManagerComponent)
			return false;

		SetImmobilized(!m_bFlipImmobilizer);
		return true;
	}

	override bool Undeploy()
	{
		if (!m_pDamageManagerComponent)
			return false;

		SetImmobilized(m_bFlipImmobilizer);
		return true;
	}

	// Sets the Engine HitZone health in order to immobilize or restore the vehicle
	protected void SetImmobilized(bool state = true)
	{
		HitZone engineHitZone = GetEngineHitZone();
		if (!engineHitZone) return;

		if (state)
		{
			m_fLastEngineHealth = engineHitZone.GetHealth();
			engineHitZone.SetHealth(0);
		}
		else
		{
			if (m_fLastEngineHealth)
				engineHitZone.SetHealth(m_fLastEngineHealth);
		}
	}

	// Returns the Engine HitZone or null
	protected HitZone GetEngineHitZone()
	{
		if (m_pDamageManagerComponent)
			return m_pDamageManagerComponent.GetHitZoneByName("Engine");
		else
			return null;
	}
}