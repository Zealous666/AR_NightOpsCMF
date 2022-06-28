class NO_SCR_DCP_IntelDownload_Action : NO_SCR_OneTimeAction
{
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		super.PerformAction(pOwnerEntity, pUserEntity);
		
		//Finish pickup intel task
		IEntity taskEntity = GetGame().GetWorld().FindEntityByName("IntelPatrol_2_US");
        NO_SCR_EditorTask task = NO_SCR_EditorTask.Cast(taskEntity);
		task.ChangeStateOfTask(TriggerType.Finish);
	
	}
	
}