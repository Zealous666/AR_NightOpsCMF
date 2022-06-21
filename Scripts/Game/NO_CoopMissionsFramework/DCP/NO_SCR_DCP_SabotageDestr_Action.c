class NO_SCR_DCP_SabotageDestr_Action : NO_SCR_OneTimeAction
{
	[Attribute("40", UIWidgets.Slider, desc: "Timer for big bada boom [s]", params: "0 90 1")]
	protected int m_iTimer;

	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		// Required for one time action
		super.PerformAction(pOwnerEntity, pUserEntity);

		// Show popup
		SCR_PopUpNotification.GetInstance().PopupMsg("SATCHEL CHARGE PLANTED", duration: 5);

 		// Start countdown (repeating fn call)
    	GetGame().GetCallqueue().CallLater(Countdown, 1000, true);
		ShowCountdown();
	}

	// Show GUI hint with time remaining
	protected void ShowCountdown()
	{
		SCR_HintManagerComponent.GetInstance().ShowCustomHint(string.Format("%1 seconds until satchel explodes!", m_iTimer), "Satchel Placed", 3);
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

	// Trigger BaseTriggerComponent
	protected void DestroyVehicle()
	{
		BaseTriggerComponent trigger = BaseTriggerComponent.Cast(GetOwner().FindComponent(BaseTriggerComponent));
		trigger.OnUserTrigger(GetOwner());
	}
}