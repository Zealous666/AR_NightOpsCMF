[EntityEditorProps(category: "GameScripted/CombatPatrol", description: "Patrol marker entity.", visible: false)]
class NO_SCR_PatrolMarkerClass : SCR_PositionClass
{
}

class NO_SCR_PatrolMarker : GenericEntity
{
	[Attribute(SCR_Enum.GetDefault(ENightOpsPatrolMarkerType.NONE), UIWidgets.ComboBox, desc: "What does this mark within a patrol area?", category: "COMBAT PATROLS", enums: ParamEnumArray.FromEnum(ENightOpsPatrolMarkerType))]
	protected ENightOpsPatrolMarkerType m_eMarkerType;

	[Attribute("1", UIWidgets.CheckBox, desc: "Allows quick disabling of guards, DO NOT USE ON HVT!", category: "COMBAT PATROLS")]
	protected bool m_bCanSpawnAI;

	override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (!GetGame().GetWorldEntity())
  			return;

		ClearFlags(EntityFlags.ACTIVE, false);
	}

	ENightOpsPatrolMarkerType GetMarkerType()
	{
		return m_eMarkerType;
	}

	array<NO_SCR_DCP_AISpawnerComponent> GetAISpawners()
	{
		array<NO_SCR_DCP_AISpawnerComponent> spawnerComponents = new array<NO_SCR_DCP_AISpawnerComponent>();

		if (!m_bCanSpawnAI)
			return spawnerComponents;

		array<Managed> components = {};
		FindComponents(NO_SCR_DCP_AISpawnerComponent, components);

		foreach (Managed validComponent : components)
			spawnerComponents.Insert(NO_SCR_DCP_AISpawnerComponent.Cast(validComponent));

		return spawnerComponents;
	}

	array<NO_SCR_DCP_EnvSpawnerComponent> GetEnvSpawners()
	{
		array<NO_SCR_DCP_EnvSpawnerComponent> spawnerComponents = new array<NO_SCR_DCP_EnvSpawnerComponent>();

		array<Managed> components = {};
		FindComponents(NO_SCR_DCP_EnvSpawnerComponent, components);

		foreach (Managed validComponent : components)
			spawnerComponents.Insert(NO_SCR_DCP_EnvSpawnerComponent.Cast(validComponent));

		return spawnerComponents;
	}

	void NO_SCR_PatrolMarker(IEntitySource src, IEntity parent)
	{
		SetEventMask(EntityEvent.INIT);
		SetFlags(EntityFlags.STATIC, true);
	}


//------------------------------------------------------------------------------------------------
// Taken and tweaked from SCR_Position and CommentEntity
//------------------------------------------------------------------------------------------------
#ifdef WORKBENCH
	int m_iColor = Color.WHITE;
	string m_sText = string.Empty;

	void SetColorAndText()
	{
		m_sText = SCR_Enum.GetEnumName(ENightOpsPatrolMarkerType, m_eMarkerType);
		m_iColor = Color.FromRGBA(34, 34, 34, 255).PackToInt();
	}

	override void _WB_AfterWorldUpdate(float timeSlice)
	{
		SetColorAndText();

		vector mat[4];
		GetWorldTransform(mat);

		vector position = mat[3];
		ref Shape pointShape = Shape.CreateSphere(m_iColor, ShapeFlags.ONCE | ShapeFlags.NOOUTLINE, position, 0.2);
		ref Shape arrowShape = Shape.CreateArrow(position, position + mat[2], 0.2, m_iColor, ShapeFlags.ONCE);

		vector textMat[4];
		GetWorld().GetCurrentCamera(textMat);

		float distScale = 1;
		distScale = vector.Distance(textMat[3], GetOrigin()) * 0.1;

		if (distScale > 15)
			return;

		distScale = Math.Clamp(distScale, 4, 5);

		if (!m_sText.IsEmpty())
		{
			ref DebugTextWorldSpace textShape = DebugTextWorldSpace.Create(
				GetWorld(),
				m_sText,
				DebugTextFlags.ONCE | DebugTextFlags.CENTER | DebugTextFlags.FACE_CAMERA,
				position[0],
				position[1] + 0.4,
				position[2],
				5.0 * distScale,
				m_iColor);
		}
	}
#endif
}


enum ENightOpsPatrolMarkerType
{
	NONE,
	INFIL,
	EXFIL,
	INTEL_1,
	INTEL_2,
	SABOTAGE_1,
	SABOTAGE_2,
	HVT
}