class NO_SCR_DCP_SabotageDestr_Action : NO_SCR_OneTimeAction
{
	[Attribute(defvalue: "{20BE751678372D8B}Prefabs/Items/Satchel.et", uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Satchel prefab to spawn in for visuals.", "et")]
	protected ResourceName m_rnSatchelPrefab;

	[Attribute(defvalue: "{49760B38779DBCB6}Prefabs/Items/Satchel_Warhead.et", uiwidget: UIWidgets.ResourcePickerThumbnail, desc: "Warhead prefab to spawn in for explosion.", "et")]
	protected ResourceName m_rnWarheadPrefab;

	[Attribute("", UIWidgets.EditBox, desc: "OPTIONAL: Name of main parent context to use for satchel location.")]
	protected string m_sParentContext;

	[Attribute("40", UIWidgets.Slider, desc: "Timer for big bada boom [s]", params: "0 90 1")]
	protected int m_iTimer;

	[Attribute("1", UIWidgets.CheckBox, desc: "Show the countdown timer as a hint!")]
	protected bool m_bShowCountdown;

	[Attribute("55", UIWidgets.Slider, desc: "Distance in meters that the coundown will be displayed when under!", params: "0 500 0.1", precision: 1)]
	protected float m_fCoundownDistance;


	protected IEntity m_pSpawnedSatchel;

	protected UserActionContext m_pActionContext;


	override void Init(IEntity pOwnerEntity, GenericComponent pManagerComponent)
	{
		ActionsManagerComponent actionsManager = ActionsManagerComponent.Cast(pManagerComponent);
		if (!actionsManager)
			return;

		m_pActionContext = actionsManager.GetContext(m_sParentContext);
	}


	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		// Required for one time action
		super.PerformAction(pOwnerEntity, pUserEntity);

		// Show popup
		SCR_PopUpNotification.GetInstance().PopupMsg("SATCHEL CHARGE PLANTED", duration: 5);

 		// Start countdown (repeating fn call)
    	GetGame().GetCallqueue().CallLater(Countdown, 1000, true);
		ShowCountdown();

		// Spawn satchel entity
		if (RplSession.Mode() != RplMode.Client)
			m_pSpawnedSatchel = SpawnEntity(m_rnSatchelPrefab);
	}


	// Show GUI hint with time remaining
	protected void ShowCountdown()
	{
		if (m_bShowCountdown && RplSession.Mode() != RplMode.Dedicated)
		{
			IEntity playerEntity = SCR_PlayerController.GetLocalControlledEntity();
			if (!playerEntity)
				return;

			float distanceToDestructable = vector.Distance(GetOwner().GetOrigin(), playerEntity.GetOrigin());

			// Only show countdown when in the set danger distance
			if (distanceToDestructable <= m_fCoundownDistance)
				SCR_HintManagerComponent.GetInstance().ShowCustomHint(string.Format("%1 seconds until satchel explodes!", m_iTimer), "Satchel Placed", 3);
		}
	}


	// Countdown function
	protected void Countdown()
	{
		// Decrement coundown
		m_iTimer--;

		ShowCountdown();

		if (m_iTimer < 1)
		{
			// Remove repeating countdown function
			GetGame().GetCallqueue().Remove(Countdown);

			// Only authority should deystroy the vehicle
			if (RplSession.Mode() != RplMode.Client)
				DestroyVehicle();
		}
	}


	protected void DestroyVehicle()
	{
		IEntity spawnedWarhead = SpawnEntity(m_rnWarheadPrefab);
		if (spawnedWarhead)
		{
			SCR_Global.DeleteEntityAndChildren(m_pSpawnedSatchel);
			GetGame().GetCallqueue().CallLater(StopPhysics, 2000, false);
		}
		else
			Print("Failed to spawn warhead: " + m_rnWarheadPrefab, LogLevel.ERROR);
	}


	protected IEntity SpawnEntity(ResourceName resourceName)
	{
		// Try load warhead resource
		Resource resource = Resource.Load(resourceName);
		if (!resource)
		{
			Print(string.Format("Resource %1 could not be loaded.", resourceName), LogLevel.ERROR);
			return null;
		}

		// Spawn params
		ref EntitySpawnParams params = new EntitySpawnParams();
		params.TransformMode = ETransformMode.WORLD;

		// When possible use action position
		if (m_pActionContext)
			m_pActionContext.GetTransformationWorld(params.Transform);
		else
			GetOwner().GetTransform(params.Transform);

		// Do spawn warhead
		return GetGame().SpawnEntityPrefab(resource, GetOwner().GetWorld(), params);
	}


	// Stops vehicles sliding after being blown up
	protected void StopPhysics()
	{
		Physics physics = GetOwner().GetPhysics();
		if (physics)
			physics.ChangeSimulationState(SimulationState.COLLISION);
	}
}