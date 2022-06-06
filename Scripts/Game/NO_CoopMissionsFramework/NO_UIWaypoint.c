class NO_UIWaypoint : SCR_InfoDisplayExtended
{
	protected ImageWidget m_wWaypoint;
	protected IEntity m_CurrentWaypoint;
	protected float m_fWaypointHeight = 0.2;
	protected float m_fAreaRadius = 1.0;
	//protected ChimeraCharacter m_Player;
	SCR_BaseGameMode m_CampaignGamemode;
	Widget waypointFrame;
	IEntity Owner;


	
	
	override void OnInit(IEntity owner)
	{
		
		super.OnInit(owner);
	}
	//------------------------------------------------------------------------------------------------
	/*!
		Checks the prerequisites for this InfoDisplay.
	*/
	override bool DisplayStartDrawInit(IEntity owner)
	{
		
		return super.DisplayStartDrawInit(owner);
	}


	//------------------------------------------------------------------------------------------------
	/*!
		Creates individual hud elements.
	*/
	override void DisplayStartDraw(IEntity owner)
	{

		super.DisplayStartDraw(owner);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Clears all hud elements.
	*/
	override void DisplayStopDraw(IEntity owner)
	{

		super.DisplayStopDraw(owner);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Updates the progress and state of all available elements.
	*/
	override void DisplayUpdate(IEntity owner, float timeSlice)
	{
		

		auto pGame = GetGame();
		if (!pGame)
			return;		
		
		auto phud = pGame.GetHUDManager();
		if (!phud)
			return;
		
		
		auto powner = owner;
		if (!powner)
			return;
		
		auto pWorld = powner.GetWorld();
		if (!pWorld)
			return;
		
		
		auto pWorkSpace = pGame.GetWorkspace();
		if (!pWorkSpace)
			return;
		
		if(!waypointFrame)
		{
			waypointFrame = m_wRoot;
		}
		
		//if(!m_Player)
		//{
		//	m_Player = ChimeraCharacter.Cast(owner);
		//}
		
		if(!m_wWaypoint)
		{
			//Widget waypointFrame = phud.CreateLayout("{42D927E4F2FF6FE5}UI/layouts/HUD/Tutorial/TutorialWaypoint.layout", EHudLayers.BACKGROUND);
			if(!waypointFrame) {return;}
			m_wWaypoint = ImageWidget.Cast(waypointFrame.FindAnyWidget("Icon"));
		}
		if(!m_CurrentWaypoint)
		{
			IEntity entity = pWorld.FindEntityByName("WP");
			if(!entity) {return;}
			m_CurrentWaypoint = entity;
		}
	
		
		// Render waypoint if allowed
		if (m_wWaypoint)
		{
			if (m_CurrentWaypoint)
			{
				vector WPPos = m_CurrentWaypoint.GetOrigin();
				WPPos[1] = WPPos[1] + m_fWaypointHeight;
				vector pos = pWorkSpace.ProjWorldToScreen(WPPos, GetGame().GetWorld());
				
				// Handle off-screen coords
				WorkspaceWidget workspace = pWorkSpace;
				int winX = workspace.GetWidth();
				int winY = workspace.GetHeight();
				int posX = workspace.DPIScale(pos[0]);
				int posY = workspace.DPIScale(pos[1]);
				
				if (posX < 0)
					pos[0] = 0;
				else if (posX > winX)
					pos[0] = workspace.DPIUnscale(winX);
				
				if (posY < 0)
					pos[1] = 0;
				else if (posY > winY || pos[2] < 0)
					pos[1] = workspace.DPIUnscale(winY);
				
				FrameSlot.SetPos(m_wWaypoint, pos[0], pos[1]);
				m_wWaypoint.SetOpacity(1);
			}
			else
			{
				m_wWaypoint.SetOpacity(0);
			}
		}
		
		// Stage evaluation
		bool waypointReached = true;
		bool stageComplete = false;
		
		if (!owner)
			return;

		
		if (m_CurrentWaypoint && m_fAreaRadius != 0)
			waypointReached = vector.DistanceSq(owner.GetOrigin(), m_CurrentWaypoint.GetOrigin()) <= m_fAreaRadius;
		
		// If stage duration is defined, check the time elapsed
		
		super.DisplayUpdate(owner,timeSlice);
	}
};