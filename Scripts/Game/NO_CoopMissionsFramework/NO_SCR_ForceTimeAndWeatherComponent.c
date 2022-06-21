//Custom script by Zeal, based on ex0's C&H variant
//Modified and adjusted by Biscuits

[ComponentEditorProps(category: "GameScripted/GameMode", description: "Forced time and weather controller.")]
class NO_SCR_ForceTimeAndWeatherComponentClass : SCR_BaseGameModeComponentClass
{
}

class NO_SCR_ForceTimeAndWeatherComponent : SCR_BaseGameModeComponent
{
	[Attribute(desc: "Time/weather changes to make.", category: "FORCE TIME AND WEATHER")]
	protected ref NO_SCR_ForceTimeAndWeatherEntry m_pChangeTimeAndWeather;

	//------------------------------------------------------------------------------------------------

	[Attribute(defvalue: "1", desc: "If enabled, custom date will be used. Authority only.", category: "LEGACY - NOT ACTUALLY USED")]
	protected bool m_bUseCustomDate;

	[Attribute(defvalue: "1", desc: "If enabled, custom time of the day will be used. Authority only.", category: "LEGACY - NOT ACTUALLY USED")]
	protected bool m_bUseCustomTime;

	[Attribute(defvalue: "1", desc: "If enabled, custom weather Id will be used. Authority only.", category: "LEGACY - NOT ACTUALLY USED")]
	protected bool m_bUseCustomWeather;

	[Attribute(defvalue: "1", desc: "If enabled, custom Latitude/Longitude will be used. Authority only.", category: "LEGACY - NOT ACTUALLY USED")]
	protected bool m_bUseCustomLatitudeLongitude;

	[Attribute(defvalue: "1989", UIWidgets.Slider, desc: "Year set on game start. Authority only.", category: "LEGACY - NOT ACTUALLY USED", params: "1900 2200 1")]
	protected int m_iCustomYear;

	[Attribute(defvalue: "7", UIWidgets.Slider, desc: "Month set on game start. Authority only.", category: "LEGACY - NOT ACTUALLY USED", params: "1 12 1")]
	protected int m_iCustomMonth;

	[Attribute(defvalue: "24", UIWidgets.Slider, desc: "Day set on game start. Authority only.", category: "LEGACY - NOT ACTUALLY USED", params: "1 31 1")]
	protected int m_iCustomDay;

	[Attribute(defvalue: "9.6", UIWidgets.Slider, desc: "Time of the day set on game start. Authority only.", category: "LEGACY - NOT ACTUALLY USED", params: "0 24 0.01")]
	protected float m_fCustomTimeOfTheDay;

	[Attribute(defvalue: "Clear", UIWidgets.ComboBox, desc: "Weather set on game start. Authority only.", category: "LEGACY - NOT ACTUALLY USED", enums: { ParamEnum("Clear", "Clear"), ParamEnum("Cloudy", "Cloudy"), ParamEnum("Overcast", "Overcast"), ParamEnum("Rainy", "Rainy") })]
	protected string m_sCustomWeather;

	[Attribute(defvalue: "-4", UIWidgets.Slider, desc: "Latitude set on game start. Authority only.", category: "LEGACY - NOT ACTUALLY USED", params: "-90 90 0.01")]
	protected float m_fCustomLatitude;

	[Attribute(defvalue: "71", UIWidgets.Slider, desc: "Longitude set on game start. Authority only.", category: "LEGACY - NOT ACTUALLY USED", params: "-180 180 0.01")]
	protected float m_fCustomLongitude;

	//------------------------------------------------------------------------------------------------

	protected override void OnPostInit(IEntity owner)
	{
		super.OnPostInit(owner);
		SetEventMask(owner, EntityEvent.INIT);
	}

	protected override void EOnInit(IEntity owner)
	{
		super.EOnInit(owner);

		if (!GetGame().InPlayMode())
			return;

		// If authority and a Time/Weather change is socketed
		if (GetGameMode().IsMaster() && m_pChangeTimeAndWeather)
		{
			// Random generator seems to be based on running time, not system time
			// This means randomizing on first frame gets the same result

			int hour;
			int minute;
			int second;
			System.GetHourMinuteSecondUTC(hour, minute, second);

			// Here we use the current system time as a random seed
			Math.Randomize(hour * minute * second);

			// Call the function that may use random generation
			m_pChangeTimeAndWeather.Execute();

			// Then change the seed back to 'time'
			Math.Randomize(-1);
		}
	}
}