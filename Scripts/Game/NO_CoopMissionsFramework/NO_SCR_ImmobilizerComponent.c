[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class NO_SCR_ImmobilizerComponentClass : ScriptComponentClass
{
}

class NO_SCR_ImmobilizerComponent : ScriptComponent
{
	[Attribute("0", UIWidgets.CheckBox, desc: "Is the vehicle immobilized on game start?", category: "IMMOBILIZER")]
	protected bool m_bStartImmobilized;

	// Keeps track of engine health for if/when immobilizer is removed
	protected float m_fLastEngineHealth;

	protected DamageManagerComponent m_pDamageManagerComponent;


	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}


	// Init and set initial immobilized state based on attribute value
	override void EOnInit(IEntity owner)
	{
		Vehicle vehicle = Vehicle.Cast(owner);
		if (!vehicle) return;

		m_pDamageManagerComponent = DamageManagerComponent.Cast(owner.FindComponent(DamageManagerComponent));

		SetImmobilized(m_bStartImmobilized);
	}


	// Sets the Engine HitZone health in order to immobilize or restore the vehicle
	void SetImmobilized(bool state = true)
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