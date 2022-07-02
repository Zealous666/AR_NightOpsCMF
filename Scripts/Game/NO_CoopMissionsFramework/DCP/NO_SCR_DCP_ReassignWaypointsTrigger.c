[EntityEditorProps(category: "GameScripted/Triggers", description: "ScriptWizard generated script file.")]
class NO_SCR_DCP_ReassignWaypointsTriggerClass : SCR_BaseTriggerEntityClass
{
}

class NO_SCR_DCP_ReassignWaypointsTrigger : SCR_BaseTriggerEntity
{
	[Attribute("", UIWidgets.EditBox, "List of AI groups to target, will also look at parent entity.", category: "Waypoints")]
	protected ref array<string> m_aNamedAIGroups;

	[Attribute("", UIWidgets.EditBox, "List of waypoint names to add to AI groups, will also look at children entities.", category: "Waypoints")]
	protected ref array<string> m_aStaticWaypoints;


	protected ref set<SCR_AIGroup> m_aAIGroups = new set<SCR_AIGroup>();
	protected ref set<AIWaypoint> m_aAIWaypoints = new set<AIWaypoint>();

	protected ref array<Managed> m_aAISpawnerComponents = new array<Managed>();


	override void EOnInit (IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;

		IEntity parentEntity = GetParent();
		if (parentEntity)
		{
			SCR_AIGroup aiGroup = SCR_AIGroup.Cast(parentEntity);
			if (aiGroup)
			{
				// If parent is an AIGroup add it to the list
				m_aAIGroups.Insert(aiGroup);
			}
			else
			{
				// Check if parent is instead some generic entity with NO_SCR_AISpawnerComponent's
				parentEntity.FindComponents(NO_SCR_AISpawnerComponent, m_aAISpawnerComponents);
			}
		}

		// If children are AIWaypoints add them to the list
		IEntity childEntity = GetChildren();
		while (childEntity)
		{
			AIWaypoint aiWaypoint = AIWaypoint.Cast(childEntity);
			if (aiWaypoint)
				m_aAIWaypoints.Insert(aiWaypoint);

			childEntity = childEntity.GetSibling();
		}
	}


	// If triggered by a player entity
	override bool ScriptedEntityFilterForQuery(IEntity ent)
	{
		ChimeraCharacter cc = ChimeraCharacter.Cast(ent);
		if (!cc) return false;

		int playerId = GetGame().GetPlayerManager().GetPlayerIdFromControlledEntity(ent);
		if (playerId == 0) return false;

		return true;
	}


	// Activate, on server only call ReassignWaypoints
	override protected event void OnActivate(IEntity ent)
	{
		if (RplSession.Mode() != RplMode.Client)
			ReassignWaypoints();
	}


	// Remove all current waypoints, add new ones
	protected void ReassignWaypoints()
	{
		// Find attribute specified groups/waypoints
		// hierarchy based g/w were added OnInit
		FindNamedAIGroups();
		FindNamedWaypoints();

		if (!m_aAISpawnerComponents.IsEmpty())
			FindDynamicallySpawnedAIGroups();

		// Remove and add new foreach group
		foreach (SCR_AIGroup aiGroup : m_aAIGroups)
		{
			array<AIWaypoint> currentWaypoints = {};
			aiGroup.GetWaypoints(currentWaypoints);

			foreach (AIWaypoint waypoint : currentWaypoints)
				aiGroup.RemoveWaypoint(waypoint);

			foreach (AIWaypoint waypoint : m_aAIWaypoints)
				aiGroup.AddWaypoint(waypoint);
		}
	}


	protected void FindNamedAIGroups()
	{
		foreach (string aiGroupName : m_aNamedAIGroups)
		{
			SCR_AIGroup aiGroup = SCR_AIGroup.Cast(GetGame().GetWorld().FindEntityByName(aiGroupName));
			if (aiGroup)
				m_aAIGroups.Insert(aiGroup);
		}
	}


	protected void FindNamedWaypoints()
	{
		foreach (string waypointName : m_aStaticWaypoints)
		{
			AIWaypoint aiWaypoint = AIWaypoint.Cast(GetGame().GetWorld().FindEntityByName(waypointName));
			if (aiWaypoint)
				m_aAIWaypoints.Insert(aiWaypoint);
		}
	}


	protected void FindDynamicallySpawnedAIGroups()
	{
		foreach (Managed foundComponent : m_aAISpawnerComponents)
		{
			NO_SCR_AISpawnerComponent aiSpawnerComponent = NO_SCR_AISpawnerComponent.Cast(foundComponent);
			if (!aiSpawnerComponent)
				return;

			foreach (AIAgent aiAgent : aiSpawnerComponent.GetSpawnedAgent())
			{
				if (!aiAgent)
					continue;

				if (aiAgent.Type() == SCR_AIGroup)
					m_aAIGroups.Insert(SCR_AIGroup.Cast(aiAgent));
				else
				{
					SCR_AIGroup aiGroup = SCR_AIGroup.Cast(aiAgent.GetParentGroup());
					if (aiGroup)
						m_aAIGroups.Insert(aiGroup);
				}
			}
		}
	}


	void NO_SCR_DCP_ReassignWaypointsTrigger(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
	}
}