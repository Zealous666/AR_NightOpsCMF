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


[BaseContainerProps()]
class NO_SCR_SimpleTimeSkipEntry : NO_SCR_ChangeTimeWeatherType
{
	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "Will skip the time by the desired number of days.", params: "0 31 1")]
	protected int m_iSkipTimeByDays;

	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "Will skip the time by the desired number of hours.", params: "0 24 1")]
	protected int m_iSkipTimeByHours;

	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "Will skip the time by the desired number of minutes.", params: "0 60 1")]
	protected int m_iSkipTimeByMinutes;

	[Attribute(defvalue: "0", UIWidgets.Slider, desc: "Will skip the time by the desired number of seconds.", params: "0 60 1")]
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


[BaseContainerProps()]
class NO_SCR_ForceTimeAndWeatherEntry : NO_SCR_ChangeTimeWeatherType
{
	[Attribute(defvalue: "0", desc: "If enabled, the settings below are ignored and dice get rolled. Individual On/Off settings are still taken into account!", category: "ON/OFF")]
	protected bool m_bRandomize;

	[Attribute(defvalue: "0", desc: "If enabled, custom date will be used.", category: "ON/OFF")]
	protected bool m_bUseCustomDate;

	[Attribute(defvalue: "0", desc: "If enabled, custom time of the day will be used.", category: "ON/OFF")]
	protected bool m_bUseCustomTime;

	[Attribute(defvalue: "0", desc: "If enabled, custom weather Id will be used.", category: "ON/OFF")]
	protected bool m_bUseCustomWeather;

	[Attribute(defvalue: "0", desc: "If enabled, custom Latitude/Longitude will be used.", category: "ON/OFF")]
	protected bool m_bUseCustomLatitudeLongitude;

	[Attribute(defvalue: "1", desc: "If disabled, time will standstill.", category: "ON/OFF")]
	protected bool m_bTimeAdvancement;

	[Attribute(defvalue: "1", desc: "If disabled, the weather state will not transition.", category: "ON/OFF")]
	protected bool m_bWeatherAdvancement;


	[Attribute(defvalue: "1989", UIWidgets.Slider, desc: "Set to specified year.", category: "SETTINGS", params: "1900 2200 1")]
	protected int m_iCustomYear;

	[Attribute(defvalue: "7", UIWidgets.Slider, desc: "Set to specified month.", category: "SETTINGS", params: "1 12 1")]
	protected int m_iCustomMonth;

	[Attribute(defvalue: "24", UIWidgets.Slider, desc: "Set to specified day.", category: "SETTINGS", params: "1 31 1")]
	protected int m_iCustomDay;

	[Attribute(defvalue: "9.6", UIWidgets.Slider, desc: "Set to specified time.", category: "SETTINGS", params: "0 24 0.01")]
	protected float m_fCustomTimeOfTheDay;

	[Attribute(uiwidget: UIWidgets.Object, desc: "Use a time skip rather than the setting above.", category: "SETTINGS")]
	protected ref NO_SCR_SimpleTimeSkipEntry m_pOrTimeSkip;

	[Attribute(SCR_Enum.GetDefault(EWeatherStates.Clear), UIWidgets.ComboBox, desc: "Set to specified weather state.", category: "SETTINGS", enums: ParamEnumArray.FromEnum(EWeatherStates))]
	protected EWeatherStates m_sCustomWeather;

	[Attribute(defvalue: "86400", UIWidgets.Slider, desc: "Number of realtime seconds in one in game day, default is REALTIME.", category: "SETTINGS", params: "1 86400 1")]
	protected float m_fDayCycleDuration;

	[Attribute(defvalue: "-4", UIWidgets.Slider, desc: "Set to specified latitude.", category: "SETTINGS", params: "-90 90 0.01")]
	protected float m_fCustomLatitude;

	[Attribute(defvalue: "71", UIWidgets.Slider, desc: "Set to specified longitude.", category: "SETTINGS", params: "-180 180 0.01")]
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
		{
			if (m_pOrTimeSkip)
				m_pOrTimeSkip.Execute();
			else
				SetTimeOfTheDay(m_fCustomTimeOfTheDay);
		}

		if (m_bUseCustomWeather)
			SetWeather(GetWeatherString(m_sCustomWeather));

		if (m_bUseCustomLatitudeLongitude)
			SetLatLong(m_fCustomLatitude, m_fCustomLongitude);

		// Only changing these if they need changing, just in case it affects the simulation ¯\_(ツ)_/¯
		if (m_pTimeAndWeatherManager.GetDayDuration() != m_fDayCycleDuration)
			m_pTimeAndWeatherManager.SetDayDuration(m_fDayCycleDuration);

		if (m_pTimeAndWeatherManager.GetIsDayAutoAdvanced() != m_bTimeAdvancement)
			m_pTimeAndWeatherManager.SetIsDayAutoAdvanced(m_bTimeAdvancement);

		// If the weather is looping, it is not advancing
		if (m_pTimeAndWeatherManager.IsWeatherLooping() == m_bWeatherAdvancement)
			m_pTimeAndWeatherManager.SetCurrentWeatherLooping(!m_bWeatherAdvancement);
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