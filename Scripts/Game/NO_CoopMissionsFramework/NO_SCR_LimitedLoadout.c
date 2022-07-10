//---------WORK IN PROGRESS--------------------------------------------------------------------
[BaseContainerProps(configRoot: true), BaseContainerCustomTitleField("m_sLoadoutName")]
class NO_SCR_LimitedLoadout : SCR_FactionPlayerLoadout
{
	[Attribute(defvalue: "1", UIWidgets.Slider, desc: "Maximum number of loadouts allowed.", params: "1 100 1")]
	protected int m_iNumberOfLoadouts;
	
	[Attribute("0", UIWidgets.CheckBox, desc: "If enabled, the number above will be used as a percentage of players able to use the loadout.")]
	protected bool m_bUsePercentage;
	
	override bool IsLoadoutAvailable(int playerId)
	{
		return CountLoadouts();
	}
	
	//------------------------------------------------------------------------------------------------
	override bool IsLoadoutAvailableClient()
	{
		return CountLoadouts();
	}
	
	protected bool CountLoadouts()
	{
		// Count the loadouts
		int loadoutCount = 0;
		array<int> PlayersArray = new array<int>();
		GetGame().GetPlayerManager().GetPlayers(PlayersArray);
		
		int maxLoadouts = m_iNumberOfLoadouts;
		
		if (m_bUsePercentage)
		{
			maxLoadouts = (GetGame().GetPlayerManager().GetPlayerCount())*(m_iNumberOfLoadouts/100);
		}
		
		foreach(int PlayerId : PlayersArray )
		{
			if (!(PlayerId>0)) continue;
			IEntity PlayerEntity = GetGame().GetPlayerManager().GetPlayerControlledEntity(PlayerId);
			if (!PlayerEntity) continue;
			BaseContainer PlayerPrefab = PlayerEntity.GetPrefabData().GetPrefab();
			if (!PlayerPrefab) continue;
			ResourceName PlayerResource = SCR_BaseContainerTools.GetPrefabResourceName(PlayerPrefab);
			if (PlayerResource==m_sLoadoutResource) ++loadoutCount;
		}
		
		return (maxLoadouts>loadoutCount);
	}
	
}