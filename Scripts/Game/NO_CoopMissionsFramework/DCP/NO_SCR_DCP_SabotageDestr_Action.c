class NO_SCR_DCP_SabotageDestr_Action : NO_SCR_OneTimeAction
{
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
		{
		
			super.PerformAction(pOwnerEntity, pUserEntity);
		
		//Show popup
		SCR_PopUpNotification.GetInstance().PopupMsg("SATCHEL CHARGE PLANTED", duration: 5);
		
		//Show hint
		SCR_HintManagerComponent hintComponent = SCR_HintManagerComponent.GetInstance();
		hintComponent.ShowCustomHint("40 Seconds till satchel explodes", "Satchels placed", 15);
		
 		//Set delay-timer to destroy
    	GetGame().GetCallqueue().CallLater(DelayedDestruction, 10000, false);
		
	}

	//Destroy after delay
	protected void DelayedDestruction()
	{
		
    SCR_PopUpNotification.GetInstance().PopupMsg("BOOM", duration: 60);
	//TO-DO Destroy/Explode pOwnerEntity
	
	}

}