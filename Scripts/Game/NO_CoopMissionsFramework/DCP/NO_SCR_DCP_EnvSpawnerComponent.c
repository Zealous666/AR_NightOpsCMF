[ComponentEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class NO_SCR_DCP_EnvSpawnerComponentClass : NO_SCR_EnvSpawnerComponentClass
{
}

class NO_SCR_DCP_EnvSpawnerComponent : NO_SCR_EnvSpawnerComponent
{
	[Attribute("", UIWidgets.EditBox, desc: "If the played faction matches this key, this spawner will be used!", category: "Dynamic Combat Patrol")]
	protected string m_sDCPFactionKey;

	string GetDCPFactionKey() { return m_sDCPFactionKey; }
}