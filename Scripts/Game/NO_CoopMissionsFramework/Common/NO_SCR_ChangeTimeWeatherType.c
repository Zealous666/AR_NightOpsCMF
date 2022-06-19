[BaseContainerProps(visible: false, insertable: false)]
class NO_SCR_ChangeTimeWeatherType
{
	//! Reference to entity responsible for managing time and weather
	protected TimeAndWeatherManagerEntity m_pTimeAndWeatherManager;

	void NO_SCR_ChangeTimeWeatherType()
	{
		m_pTimeAndWeatherManager = GetGame().GetTimeAndWeatherManager();
		if (!m_pTimeAndWeatherManager)
			Print("Cannot initialize TimeAndWeatherManagerEntity not found!", LogLevel.ERROR);
	}

	void Execute() {};
}


[BaseContainerProps(), SCR_BaseContainerStaticTitleField("Simple Time Skip")]
class NO_SCR_SimpleTimeSkipEntry : NO_SCR_ChangeTimeWeatherType
{
	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "Will skip the time by the desired number of days. Authority only.", params: "0 31 1")]
	protected int m_iSkipTimeByDays;

	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "Will skip the time by the desired number of hours. Authority only.", params: "0 24 1")]
	protected int m_iSkipTimeByHours;

	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "Will skip the time by the desired number of minutes. Authority only.", params: "0 60 1")]
	protected int m_iSkipTimeByMinutes;

	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "Will skip the time by the desired number of seconds. Authority only.", params: "0 60 1")]
	protected int m_iSkipTimeBySeconds;

	override void Execute()
	{
		if (!m_pTimeAndWeatherManager)
			return;

		TimeContainer currentTime = m_pTimeAndWeatherManager.GetTime();

		int hours = currentTime.m_iHours + m_iSkipTimeByHours;
		int minutes = currentTime.m_iMinutes + m_iSkipTimeByMinutes;
		int seconds = currentTime.m_iSeconds + m_iSkipTimeBySeconds;

		hours += m_iSkipTimeByDays * 24;

		m_pTimeAndWeatherManager.SetTime(new TimeContainer(hours, minutes, seconds));
	}
}


enum EWeatherStates
{
	Clear,
	Cloudy,
	Overcast,
	Rainy
}


[BaseContainerProps(), SCR_BaseContainerStaticTitleField("Full Time/Weather Override")]
class NO_SCR_ForceTimeAndWeatherEntry : NO_SCR_ChangeTimeWeatherType
{
	//! If enabled custom date will be used on session start. Authority only.
	[Attribute(defvalue: "0", desc: "If enabled, the settings below are ignored and dice get rolled. Individual On/Off settings are still taken into account! Authority only.", category: "ON/OFF")]
	protected bool m_bRandomize;

	//! If enabled custom date will be used on session start. Authority only.
	[Attribute(defvalue: "0", desc: "If enabled, custom date will be used. Authority only.", category: "ON/OFF")]
	protected bool m_bUseCustomDate;

	//! If enabled custom time of the day will be used on session start. Authority only.
	[Attribute(defvalue: "0", desc: "If enabled, custom time of the day will be used. Authority only.", category: "ON/OFF")]
	protected bool m_bUseCustomTime;

	//! If enabled custom weather Id will be used on session start. Authority only.
	[Attribute(defvalue: "0", desc: "If enabled, custom weather Id will be used. Authority only.", category: "ON/OFF")]
	protected bool m_bUseCustomWeather;

	//! If enabled custom weather Id will be used on session start. Authority only.
	[Attribute(defvalue: "0", desc: "If enabled, custom Latitude/Longitude will be used. Authority only.", category: "ON/OFF")]
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
	[Attribute(SCR_Enum.GetDefault(EWeatherStates.Clear), UIWidgets.ComboBox, desc: "Weather set on game start. Authority only.", category: "SETTINGS", enums: ParamEnumArray.FromEnum(EWeatherStates))]
	protected EWeatherStates m_sCustomWeather;

	//! Latitude set on game start. Authority only.
	[Attribute(defvalue: "-4", UIWidgets.Slider, desc: "Latitude set on game start. Authority only.", category: "SETTINGS", params: "-90 90 0.01")]
	protected float m_fCustomLatitude;

	//! Longitude set on game start. Authority only.
	[Attribute(defvalue: "71", UIWidgets.Slider, desc: "Longitude set on game start. Authority only.", category: "SETTINGS", params: "-180 180 0.01")]
	protected float m_fCustomLongitude;

	override void Execute()
	{
		if (!m_pTimeAndWeatherManager)
			return;

		if (m_bRandomize)
		{
			if (m_bUseCustomDate)
			{
				m_iCustomYear = Math.RandomIntInclusive(1900, 2200);
				m_iCustomMonth = Math.RandomIntInclusive(1, 12);
				m_iCustomDay = Math.RandomIntInclusive(1, 31);
			}

			if (m_bUseCustomTime)
				m_fCustomTimeOfTheDay = Math.RandomFloatInclusive(0, 24);

			if (m_bUseCustomWeather)
				m_sCustomWeather = GetRandomWeather();

			if (m_bUseCustomLatitudeLongitude)
			{
				m_fCustomLatitude = Math.RandomFloatInclusive(-90, 90);
				m_fCustomLongitude = Math.RandomFloatInclusive(-180, 180);
			}
		}

		if (m_bUseCustomDate)
			SetDate(m_iCustomYear, m_iCustomMonth, m_iCustomDay);

		if (m_bUseCustomTime)
			SetTimeOfTheDay(m_fCustomTimeOfTheDay);

		if (m_bUseCustomWeather)
			SetWeather(GetWeatherString(m_sCustomWeather));

		if (m_bUseCustomLatitudeLongitude)
			SetLatLong(m_fCustomLatitude, m_fCustomLongitude);
	}

	protected string GetWeatherString(EWeatherStates state)
	{
		return SCR_Enum.GetEnumName(EWeatherStates, state);
	}

	protected int GetRandomWeather()
	{
		int minimum;
		int maximum;
		SCR_Enum.GetRange(EWeatherStates, minimum, maximum);

		EWeatherStates randomState = Math.RandomIntInclusive(minimum, maximum);

		return randomState;
	}

	// Forcefully sets time of the date to provided value. Authority only.
	protected void SetDate(int year, int month, int day)
	{
		m_pTimeAndWeatherManager.SetDate(year, month, day, true);
	}

	// Forcefully sets time of the day to provided value. Authority only.
	protected void SetTimeOfTheDay(float timeOfTheDay)
	{
		m_pTimeAndWeatherManager.SetTimeOfTheDay(timeOfTheDay, true);
	}

	// Forcefully sets weather to provided weatherId. Authority only.
	protected void SetWeather(string weatherId)
	{
		if (weatherId.IsEmpty())
			return;

		m_pTimeAndWeatherManager.ForceWeatherTo(true, weatherId, 0.0);
	}

	// Forcefully sets latitude/longitude to provided values. Authority only.
	protected void SetLatLong(float latitude, float longitude)
	{
		m_pTimeAndWeatherManager.SetCurrentLatitude(latitude);
		m_pTimeAndWeatherManager.SetCurrentLongitude(longitude);
	}
}