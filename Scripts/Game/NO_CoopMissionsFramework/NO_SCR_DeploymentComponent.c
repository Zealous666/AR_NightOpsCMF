[EntityEditorProps(category: "GameScripted/ScriptWizard", description: "ScriptWizard generated script file.")]
class NO_SCR_DeploymentComponentClass : ScriptComponentClass
{
}

class NO_SCR_DeploymentComponent : ScriptComponent
{
	[Attribute("0", UIWidgets.CheckBox, desc: "Is deployed on game start.", category: "Deployment"), RplProp()]
	protected bool m_bIsDeployed;

	[Attribute("0", UIWidgets.CheckBox, desc: "Can only be deployed.", category: "Deployment")]
	protected bool m_bDeployOnly;

	[Attribute(desc: "Use provided or design your own deployments.", category: "Deployment")]
	protected ref array<ref NO_SCR_DeploymentInterface> m_aDeployments;


	protected RplComponent m_pRplComponent;


	override void OnPostInit(IEntity owner)
	{
		SetEventMask(owner, EntityEvent.INIT);
	}

	override void EOnInit(IEntity owner)
	{
		if (!GetGame().InPlayMode())
			return;

		// Check for RplComponent and clear deployments / abort if not found
		m_pRplComponent = RplComponent.Cast(owner.FindComponent(RplComponent));
		if (!m_pRplComponent)
		{
			Print("NO_SCR_DeploymentComponent requires an RplComponent on the same entity!", LogLevel.ERROR);
			m_aDeployments.Clear();
			return;
		}

		// Let each deployment know the entity its attached to
		foreach (NO_SCR_DeploymentInterface deployment : m_aDeployments)
			deployment.Init(owner, m_pRplComponent);

		// Can start in deployed state, server only
		if (m_pRplComponent.IsMaster() && m_bIsDeployed)
		{
			if (!TryDeploy())
				TryUndeploy();
		}
	}

	// Manages the IsDeployed state (RplProp)
	protected void SetDeployed(bool state)
	{
		if (m_bIsDeployed != state)
		{
			m_bIsDeployed = state;
			Replication.BumpMe();
		}
	}

	// Action helper
	bool IsActionPerformable()
	{
		if (m_bDeployOnly && m_bIsDeployed)
			return false;

		return true;
	}

	// Action helper
	bool IsActionShowable()
	{
		if (m_bDeployOnly && m_bIsDeployed)
			return false;

		return true;
	}

	// Action helper
	bool IsCurrentlyDeployed()
	{
		return m_bIsDeployed;
	}

	// Called once by server when action performed by any client
	void ToggleDeployment()
	{
		if (m_pRplComponent.IsProxy())
			return;

		// Changes deployed state and attempts a rollback if a deployment fails
		if (m_bIsDeployed)
		{
			if (TryUndeploy())
				SetDeployed(false);
			else
				TryDeploy();
		}
		else
		{
			if (TryDeploy())
				SetDeployed(true);
			else
				TryUndeploy();
		}
	}


	protected bool TryDeploy()
	{
		bool result = true;

		foreach (NO_SCR_DeploymentInterface deployment : m_aDeployments)
		{
			if (!deployment.IsServerOnly())
				ClientDeployment(deployment, true);

			if (!deployment.Deploy())
				result = false;
		}

		return result;
	}


	protected bool TryUndeploy()
	{
		bool result = true;

		foreach (NO_SCR_DeploymentInterface deployment : m_aDeployments)
		{
			if (!deployment.IsServerOnly())
				ClientDeployment(deployment, false);

			if (!deployment.Undeploy())
				result = false;
		}

		return result;
	}

	// Instructs clients to do deployments
	protected void ClientDeployment(NO_SCR_DeploymentInterface deployment, bool tryDeploy)
	{
		int deploymentIndex = m_aDeployments.Find(deployment);

		if (deploymentIndex != -1)
			Rpc(RpcDo_ClientDeployment, deploymentIndex, tryDeploy);
	}

	// For client deployments, such as client side hints
	[RplRpc(RplChannel.Reliable, RplRcver.Broadcast)]
	protected void RpcDo_ClientDeployment(int deploymentIndex, bool tryDeploy)
	{
		NO_SCR_DeploymentInterface deployment = m_aDeployments.Get(deploymentIndex);
		if (!deployment)
			return;

		if (tryDeploy)
			deployment.Deploy();
		else
			deployment.Undeploy();
	}
}