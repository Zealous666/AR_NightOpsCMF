class NO_SCR_DCP_IntelPickup_Action : ScriptedUserAction
{
	
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		//Finish pickup intel task
		IEntity taskEntity = GetGame().GetWorld().FindEntityByName("IntelPatrol_1_US");
        NO_SCR_EditorTask task = NO_SCR_EditorTask.Cast(taskEntity);
		task.ChangeStateOfTask(TriggerType.Finish);
	
		
		//Removes intel piece from world
		SCR_EntityHelper.DeleteEntityAndChildren(pOwnerEntity);
	}
	
}