[ComponentEditorProps(category: "GameScripted/CombatPatrol", description: "Combat patrol wrapper for NO_SCR_AISpawnerComponent.")]
class NO_SCR_DCP_AISpawnerComponentClass : NO_SCR_AISpawnerComponentClass
{
}

class NO_SCR_DCP_AISpawnerComponent : NO_SCR_AISpawnerComponent
{
	[Attribute("", UIWidgets.EditBox, desc: "If the played faction matches this key, this spawner will be used!", category: "Dynamic Combat Patrol")]
	protected string m_sDCPFactionKey;

	string GetDCPFactionKey() { return m_sDCPFactionKey; }
}