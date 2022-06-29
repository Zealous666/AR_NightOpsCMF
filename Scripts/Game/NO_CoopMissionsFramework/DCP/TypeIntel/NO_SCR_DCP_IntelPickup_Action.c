class NO_SCR_DCP_IntelPickup_Action : ScriptedUserAction
{
	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (!GetPatrolManager())
		{
			Print("No PatrolManager could be found!", LogLevel.ERROR);
			return;
		}

		// Get intel 1 task
		IEntity taskEntity = GetGame().GetWorld().FindEntityByName(GetPatrolManager().INTEL_PATROL_OBJ1_TASKNAME);
		if (!taskEntity)
		{
			Print("Task could not be found!", LogLevel.ERROR);
			return;
		}

        NO_SCR_EditorTask task = NO_SCR_EditorTask.Cast(taskEntity);
		if (!task) return;

		// Finish intel 1 task
		task.ChangeStateOfTask(TriggerType.Finish);

		// Play the map pickup sound (3D)
		PlaySound();

		// Removes intel piece from world
		SCR_EntityHelper.DeleteEntityAndChildren(pOwnerEntity);
	}

	// Taken from toilet/piano example
	protected void PlaySound()
	{
		GameSignalsManager globalSignalsManager = GetGame().GetSignalsManager();

		ref array<string> signalName = new array<string>;
		ref array<float> signalValue = new array<float>;

		signalName.Insert("GInterior");
		signalName.Insert("GIsThirdPersonCam");
		signalName.Insert("GCurrVehicleCoverage");

		foreach(string signal : signalName)
			signalValue.Insert(globalSignalsManager.GetSignalValue(globalSignalsManager.AddOrFindSignal(signal)));

		vector mat[4];
		mat[3] = GetOwner().GetOrigin();

		AudioSystem.PlayEvent("{13F8D0DE67678118}Sounds/Items/_SharedData/PickUp/Items_PickUp_Map.acp", "SOUND_PICK_UP", mat, signalName, signalValue);
	}
}