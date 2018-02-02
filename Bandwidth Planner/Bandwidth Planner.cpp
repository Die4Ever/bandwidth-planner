// Bandwidth Planner.cpp : main project file.

#include "stdafx.h"

float DailyUsage[7] = {20.0f, 10.0f, 10.0f, 10.0f, 10.0f, 15.0f, 20.0f};
float used = 0.0f;
float cap = 1024.0f;

float usedtoday=0.0f;

int billstartyear=2016;
int billstartmonth=1;
int billstartday=16;
int billendyear=2016;
int billendmonth=2;
int billendday=16;

extern unsigned long long lastupdate;

#include "bandwidthtracker.h"
#include <Windows.h>
#include "taskbar.h"

class BillingDay
{
public:
	int year;
	int month;
	int day;
	int dow;
	int week;
	int daysinmonth;

	BillingDay(int Year, int Month, int Day)
	{
		year=Year;
		month=Month;
		day=Day;
		dow = (int)System::DateTime::DateTime(year, month, day).DayOfWeek;
		daysinmonth=System::DateTime::DaysInMonth(year, month);
		week=1;
	}

	BillingDay& operator++()
	{
		day++;
		dow++;//(dow+1)%7;
		if(dow>=7)
		{
			dow=0;
			week++;
		}
		if(day>daysinmonth)
		{
			day=1;
			month=month++;
			if(month>12)
			{
				year++;
				month=1;
			}
			daysinmonth=System::DateTime::DaysInMonth(year, month);
		}
		return *this;
	}

	bool operator!=(BillingDay &b)
	{
		return (year != b.year) || (month != b.month) || (day != b.day);
	}
};

class BillingCycle
{
public:
	BillingDay startday;
	BillingDay endday;

	BillingCycle(BillingDay Start, BillingDay End) : startday(Start), endday(End)
	{
	}

	int NumDays()
	{
		if(startday.month!=endday.month)
			return (startday.daysinmonth - startday.day) + (endday.day);
		else
			return endday.day - startday.day;
	}

	BillingDay begin()
	{
		return startday;
	}

	BillingDay end()
	{
		return endday;
	}
};


#include "Form1.h"

using namespace BandwidthPlanner;

[STAThreadAttribute]
int main(array<System::String ^> ^args)
{
	InitUsageStats();
	// Enabling Windows XP visual effects before any controls are created
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false); 

	// Create the main window and run it
	Application::Run(gcnew Form1());
	return 0;
}
