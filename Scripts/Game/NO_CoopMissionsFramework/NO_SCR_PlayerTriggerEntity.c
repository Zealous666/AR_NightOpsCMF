[EntityEditorProps(category: "GameScripted/Triggers", description: "ScriptWizard generated script file.")]
class NO_SCR_PlayerTriggerEntityClass : SCR_BaseTriggerEntityClass
{
}

class NO_SCR_PlayerTriggerEntity : SCR_BaseTriggerEntity
{
	protected ref ScriptInvoker m_OnPlayerQuotaReached = new ScriptInvoker();

	[Attribute("0", UIWidgets.ComboBox, desc: "Quota type in use.", category: "PLAYER TRIGGER", ParamEnumArray.FromEnum(EPlayerTriggerQuota))]
	protected EPlayerTriggerQuota m_PlayerTriggerQuota;
	
	// To track if the quota trigger is enabled/disabled, regular trigger will still track players entry/exit
	[Attribute("1", UIWidgets.CheckBox, desc: "Is trigger active at game start?", category: "PLAYER TRIGGER")]
	protected bool m_bIsActive;

	// An array of players currently within the trigger volume
	protected ref set<int> m_aPlayerIdsInTrigger = new set<int>();

	ScriptInvoker GetOnPlayerQuotaReached()
	{
		return m_OnPlayerQuotaReached;
	}

	// Event called on player quota being reached
	// Can insert onto script invoker or override method
	protected event void OnPlayerQuotaReached()
	{
		m_OnPlayerQuotaReached.Invoke();
	}

	bool IsActive()
	{
		return m_bIsActive;
	}

	void SetActive(bool state)
	{
		m_bIsActive = state;
	}

	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		// Only ChimeraCharacters should activate this trigger.
		// They are checked to be players in activation/deactivation of the trigger.
		ChimeraCharacter cc = ChimeraCharacter.Cast(ent);
		if (!cc) return false;

		return true;
	}

	override protected event void OnActivate(IEntity ent)
	{
		if (Replication.IsClient())
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ent);
		if (playerId == 0)
			return;

		// Add to array of players within trigger (IF PLAYER)
		m_aPlayerIdsInTrigger.Insert(playerId);

		// if IsActive: Check if the quota is now reached and the event should fire
		if (m_bIsActive && IsPlayerQuotaReached())
			OnPlayerQuotaReached();
	}

	override protected event void OnDeactivate(IEntity ent)
	{
		if (Replication.IsClient())
			return;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ent);

		// Remove from array of players within trigger (IF PLAYER)
		if (playerId != 0)
		{
			int index = m_aPlayerIdsInTrigger.Find(playerId);

			if (index != -1)
				m_aPlayerIdsInTrigger.Remove(index);
		}
	}

	// Multiple types of quota exist, has the selected type been reached?
	protected bool IsPlayerQuotaReached()
	{
		int conectedPlayersCount = 999;
		int playerCountInside = 0;
		int playerCountOutside = 0;

		conectedPlayersCount = GetGame().GetPlayerManager().GetPlayerCount();
		playerCountInside = m_aPlayerIdsInTrigger.Count();
		playerCountOutside = conectedPlayersCount - playerCountInside;

		switch (m_PlayerTriggerQuota)
		{
			case EPlayerTriggerQuota.ALL :
			{
				if (playerCountOutside == 0)
					return true;

				break;
			}
			case EPlayerTriggerQuota.MAJORITY :
			{
				if (playerCountInside > playerCountOutside)
					return true;

				break;
			}
			case EPlayerTriggerQuota.SINGLE :
			{
				if (playerCountInside > 0)
					return true;

				break;
			}
		}
		return false;
	}
}

enum EPlayerTriggerQuota
{
	ALL,
	MAJORITY,
	SINGLE
}