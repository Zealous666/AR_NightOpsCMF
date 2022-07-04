[BaseContainerProps()]
class NO_SCR_ImmobilizerDeployment : NO_SCR_DeploymentInterface
{
	[Attribute("0", UIWidgets.CheckBox, desc: "Flips the immobilizer so that when deployed the vehicle can move.")]
	protected bool m_bFlipImmobilizer;

	[Attribute("1", UIWidgets.CheckBox, desc: "Engages the handbrake when deployed, slowing the vehicle faster.")]
	protected bool m_bUseHandbrake;

	protected DamageManagerComponent m_pDamageManagerComponent;
	protected SCR_CarControllerComponent m_pCarControllerComponent;

	protected float m_fLastEngineHealth;
	protected bool m_bFirstRun = true;

	override void Init(IEntity owner, RplComponent rplComponent)
	{
		super.Init(owner, rplComponent);
		m_bServerOnly = false;
	}

	override bool Deploy()
	{
		if (m_bFirstRun)
		{
			Vehicle vehicle = Vehicle.Cast(GetAttachedEntity());
			if (!vehicle)
				return false;

			m_pDamageManagerComponent = DamageManagerComponent.Cast(GetAttachedEntity().FindComponent(DamageManagerComponent));
			m_pCarControllerComponent = SCR_CarControllerComponent.Cast(GetAttachedEntity().FindComponent(SCR_CarControllerComponent));
		}

		if (m_bUseHandbrake)
			SetHandbrake(!m_bFlipImmobilizer);

		if (RplSession.Mode() == RplMode.Client)
			return true;

		if (!m_pDamageManagerComponent)
			return false;

		SetImmobilized(!m_bFlipImmobilizer);
		return true;
	}

	override bool Undeploy()
	{
		if (m_bUseHandbrake)
			SetHandbrake(m_bFlipImmobilizer);

		if (RplSession.Mode() == RplMode.Client)
			return true;

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

	// Set the handbrake to the suggested state
	protected void SetHandbrake(bool state)
	{
		if (m_pCarControllerComponent)
			m_pCarControllerComponent.SetPersistentHandBrake(state);
	}
}