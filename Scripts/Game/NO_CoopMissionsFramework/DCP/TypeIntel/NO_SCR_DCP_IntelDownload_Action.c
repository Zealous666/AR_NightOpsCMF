class NO_SCR_DCP_IntelDownload_Action : NO_SCR_OneTimeAction
{
	[Attribute("SOUND_RADIO_CHATTER_RU", UIWidgets.ComboBox, desc: "", enums: { new ParamEnum("SOUND_RADIO_CHATTER_RU", "SOUND_RADIO_CHATTER_RU"), new ParamEnum("SOUND_RADIO_CHATTER_US", "SOUND_RADIO_CHATTER_US"), new ParamEnum("SOUND_RADIO_CHATTER_EV", "SOUND_RADIO_CHATTER_EV") })]
	protected string m_sRadioChatter;

	protected AudioHandle m_pAudioHandle;


	override void PerformAction(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		// Required for one time action
		super.PerformAction(pOwnerEntity, pUserEntity);

		if (!GetPatrolManager())
		{
			Print("No PatrolManager could be found!", LogLevel.ERROR);
			return;
		}

		// Get intel 2 task
		IEntity taskEntity = GetGame().GetWorld().FindEntityByName(GetPatrolManager().INTEL_PATROL_OBJ2_TASKNAME);
		if (!taskEntity)
		{
			Print("Task could not be found!", LogLevel.ERROR);
			return;
		}

        NO_SCR_EditorTask task = NO_SCR_EditorTask.Cast(taskEntity);
		if (!task) return;

		// Finish intel 2 task
		task.ChangeStateOfTask(TriggerType.Finish);

		// Play the set faction chatter (3D)
		PlaySound(m_sRadioChatter);
	}


	// Taken from toilet/piano example
	protected void PlaySound(string sound)
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

		if (m_pAudioHandle)
			AudioSystem.TerminateSound(m_pAudioHandle);

		m_pAudioHandle = AudioSystem.PlayEvent("{4340FAB152ACD12D}Sounds/UI/UI_Radio_Establish_Action.acp", sound, mat, signalName, signalValue);
	}


	// ------------------------------------------------------------ //
	// Plays radio establish noise for the user while in the action //
	// ------------------------------------------------------------ //
	override void OnActionStart(IEntity pUserEntity)
	{
		super.OnActionStart(pUserEntity);
		PlaySound("SOUND_RADIO_ESTABLISH_ACTION");
	}

	override void OnActionCanceled(IEntity pOwnerEntity, IEntity pUserEntity)
	{
		if (m_pAudioHandle)
			AudioSystem.TerminateSound(m_pAudioHandle);
	}

	// ---------------------------------------------------------------


	void ~NO_SCR_DCP_IntelDownload_Action()
	{
		AudioSystem.TerminateSound(m_pAudioHandle);
	}
}