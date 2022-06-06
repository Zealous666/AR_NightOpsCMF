//Custom script by Zeal, based on ex0's C&H variant
//Modified and adjusted by Biscuits

[ComponentEditorProps(category: "GameScripted/GameMode", description: "Forced time and weather controller.")]
class NO_SCR_ForceTimeAndWeatherComponentClass : SCR_BaseGameModeComponentClass
{
}

//------------------------------------------------------------------------------------------------
/*!
	Forced time and weather component that allows overriding the TimeAndWeatherManager.
	This component must be attached to a SCR_BaseGameMode entity!
*/
class NO_SCR_ForceTimeAndWeatherComponent : SCR_BaseGameModeComponent
{
	//! Manager singleton instance, assigned on first get call
	private static NO_SCR_ForceTimeAndWeatherComponent s_pInstance;

	//! If enabled custom date will be used on session start. Authority only.
	[Attribute(defvalue: "1", desc: "If enabled, custom date will be used. Authority only.", category: "ON/OFF")]
	protected bool m_bUseCustomDate;

	//! If enabled custom time of the day will be used on session start. Authority only.
	[Attribute(defvalue: "1", desc: "If enabled, custom time of the day will be used. Authority only.", category: "ON/OFF")]
	protected bool m_bUseCustomTime;

	//! If enabled custom weather Id will be used on session start. Authority only.
	[Attribute(defvalue: "1", desc: "If enabled, custom weather Id will be used. Authority only.", category: "ON/OFF")]
	protected bool m_bUseCustomWeather;

	//! If enabled custom weather Id will be used on session start. Authority only.
	[Attribute(defvalue: "1", desc: "If enabled, custom Latitude/Longitude will be used. Authority only.", category: "ON/OFF")]
	protected bool m_bUseCustomLatitudeLongitude;

	//! Year set on game start. Authority only.
	[Attribute(defvalue: "1989", UIWidgets.Slider, desc: "Year set on game start. Authority only.", category: "SETTINGS", params: "1900 2200 1")]
	protected int m_iCustomYear;

	//! Month set on game start. Authority only.
	[Attribute(defvalue: "7", UIWidgets.Slider, desc: "Month set on game start. Authority only.", category: "SETTINGS", params: "1 12 1")]
	protected int m_iCustomMonth;

	//! Day set on game start. Authority only.
	[Attribute(defvalue: "24", UIWidgets.Slider, desc: "Day set on game start. Authority only.", category: "SETTINGS", params: "1 31 1")]
	protected int m_iCustomDay;

	//! Time of the day set on game start. Authority only.
	[Attribute(defvalue: "9.6", UIWidgets.Slider, desc: "Time of the day set on game start. Authority only.", category: "SETTINGS", params: "0 24 0.01")]
	protected float m_fCustomTimeOfTheDay;

	//! Weather IDs are the same as used in the TimeAndWeatherManager. Weather set on game start. Authority only.
	[Attribute(defvalue: "Clear", UIWidgets.ComboBox, desc: "Weather set on game start. Authority only.", category: "SETTINGS", enums: { ParamEnum("Clear", "Clear"), ParamEnum("Cloudy", "Cloudy"), ParamEnum("Overcast", "Overcast"), ParamEnum("Rainy", "Rainy") })]
	protected string m_sCustomWeather;

	//! Latitude set on game start. Authority only.
	[Attribute(defvalue: "-4", UIWidgets.Slider, desc: "Latitude set on game start. Authority only.", category: "SETTINGS", params: "-90 90 0.01")]
	protected float m_fCustomLatitude;

	//! Longitude set on game start. Authority only.
	[Attribute(defvalue: "71", UIWidgets.Slider, desc: "Longitude set on game start. Authority only.", category: "SETTINGS", params: "-180 180 0.01")]
	protected float m_fCustomLongitude;

	//! Reference to entity responsible for managing time and weather
	protected TimeAndWeatherManagerEntity m_pTimeAndWeatherManager;

	//------------------------------------------------------------------------------------------------
	/*!
		Initializes this component and hooks up events.
	*/
	protected override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Initialize the manager.
	*/
	protected override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		// Only on Authority
		if (!m_pGameMode.IsMaster())
			return;

		// Is TimeAndWeatherManager entity available
		m_pTimeAndWeatherManager = GetGame().GetTimeAndWeatherManager();
		if (!m_pTimeAndWeatherManager)
		{
			Print("Cannot initialize TimeAndWeatherManagerEntity not found!", LogLevel.WARNING);
			return;
		}

		if (m_bUseCustomDate)
			SetDate(m_iCustomYear, m_iCustomMonth, m_iCustomDay);

		if (m_bUseCustomTime)
			SetTimeOfTheDay(m_fCustomTimeOfTheDay);

		if (m_bUseCustomWeather)
			SetWeather(m_sCustomWeather);

		if (m_bUseCustomLatitudeLongitude)
			SetLatLong(m_fCustomLatitude, m_fCustomLongitude);
	}

	//------------------------------------------------------------------------------------------------
	/*
		Finds SCR_ForceTimeAndWeatherComponent on current game mode and returns it, or null if none.
	*/
	static NO_SCR_ForceTimeAndWeatherComponent GetActiveComponent()
	{
		BaseGameMode gameMode = GetGame().GetGameMode();
		if (!gameMode)
			return null;

		if (!s_pInstance)
			s_pInstance = NO_SCR_ForceTimeAndWeatherComponent.Cast(gameMode.FindComponent(NO_SCR_ForceTimeAndWeatherComponent));

		return s_pInstance;
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Forcefully sets time of the date to provided value. Authority only.
	*/
	void SetDate(int year, int month, int day)
	{
		m_pTimeAndWeatherManager.SetDate(year, month, day, true);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Forcefully sets time of the day to provided value. Authority only.
	*/
	void SetTimeOfTheDay(float timeOfTheDay)
	{
		m_pTimeAndWeatherManager.SetTimeOfTheDay(timeOfTheDay, true);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Forcefully sets weather to provided weatherId. Authority only.
	*/
	void SetWeather(string weatherId)
	{
		if (weatherId.IsEmpty())
			return;

		m_pTimeAndWeatherManager.ForceWeatherTo(true, weatherId, 0.0);
	}

	//------------------------------------------------------------------------------------------------
	/*!
		Forcefully sets latitude/longitude to provided values. Authority only.
	*/
	void SetLatLong(float latitude, float longitude)
	{
		m_pTimeAndWeatherManager.SetCurrentLatitude(latitude);
		m_pTimeAndWeatherManager.SetCurrentLongitude(longitude);
	}
}