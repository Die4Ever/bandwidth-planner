/*TODO
-fix my allowance calculations, it needs to take into account how many of each weekday there are
-fix my average usage per day calculations, it shouldn't require being on at midnight
	-this may not be an issue if I can scrape my daily usage from att
-how do I count the time that this program isn't open?
	-this program should always be open when my computer is on, but I still need to count the 0s for when my pc is off...
	-best way would be to remember the last recorded bandwidth usage of the previous day and do the averages based on entire days instead of per hour or so

-maybe moving averages aren't really suitable?
	-I should probably switch to doing a total for each day and do a moving average on those totals

*/
#include "stdafx.h"

#include <fstream>
#include <string.h>
#include <vector>
#include <sstream>
#include <time.h>
#include <cstdlib>
#include <math.h>
#include <array>
using namespace std;

#define RAYHEADERSMAIN

#include <util.h>
#include <oswrapper.h>
#include <raystringstuff.h>
#include <rayheap.h>
#include <rayheapnew.h>
#include <raycontainers2.h>
#include <sockets4.h>
using namespace std;

#include "bandwidthtracker.h"

extern float DailyUsage[7];// = {10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f, 10.0f};
extern float used;// = 107.0f;
extern float cap;// = 250.0f;

extern float usedtoday;

extern int billstartyear;
extern int billstartmonth;
extern int billstartday;
extern int billendyear;
extern int billendmonth;
extern int billendday;

unsigned __int64 rx=0,tx=0;//reset these each billing cycle
unsigned __int64 lastrx=0,lasttx=0,tlastrx=0,tlasttx=0;//don't reset these
unsigned __int64 dayrx=0,daytx=0;

//unsigned __int64 deltarx=0,deltatx=0;
int today = -1;

unsigned __int64 trx,ttx;
int tbillstartyear;
int tbillstartmonth;
int tbillstartday;
int tbillendyear;
int tbillendmonth;
int tbillendday;

time_t lasttime = time(0);
unsigned long long lastupdate = time(0);

HANDLE thread=INVALID_HANDLE_VALUE;

struct SavedUsageStats
{
	int version;
	float DailyAvgs[7];
	time_t LastTime;
	unsigned __int64 rx,tx;
	unsigned __int64 dayrx,daytx;
	unsigned __int64 lastrx,lasttx;

	int startyear, startmonth, startday, endyear, endmonth, endday;
};

void SaveUsageStats()
{
	//if(thread!=INVALID_HANDLE_VALUE)
		//WaitForSingleObject(thread, INFINITE);//just to avoid atomicity issues, this should only wait when saving on close
	//atomicity is not an issue, the thread works on temporary variables

	SavedUsageStats stats;
	stats.version=3;
	//stats.DailyAvgs = DailyUsage;
	memcpy(stats.DailyAvgs, DailyUsage, sizeof(float)*7);
	stats.LastTime=lasttime;
	stats.rx=rx;
	stats.tx=tx;
	stats.dayrx=dayrx;
	stats.daytx=daytx;
	stats.lastrx=lastrx;
	stats.lasttx=lasttx;

	stats.startyear=billstartyear;
	stats.startmonth=billstartmonth;
	stats.startday=billstartday;
	stats.endyear=billendyear;
	stats.endmonth=billendmonth;
	stats.endday=billendday;

	const char *userprofile = getenv("USERPROFILE");
	string path = string(userprofile) + "\\My Documents\\bandwidthusagestats.dat";
	ofstream file(path.c_str(), ios::binary);
	file.write((char*)&stats, sizeof(stats));
}

void InitUsageStats()
{
	SavedUsageStats stats;
	stats.version=-1;
	const char *userprofile = getenv("USERPROFILE");
	string path = string(userprofile) + "\\My Documents\\bandwidthusagestats.dat";
	ifstream file(path.c_str(), ios::binary);

	if (file.good())
		file.read((char*)&stats, sizeof(stats));
	else
		return;

	if(stats.version==1)
	{
		memcpy(DailyUsage, stats.DailyAvgs, sizeof(float)*7);
		lasttime=stats.LastTime;
		rx=stats.rx;
		tx=stats.tx;
		lastrx=rx;
		lasttx=tx;
		dayrx=stats.dayrx;
		daytx=stats.daytx;
	}
	else if(stats.version==2)
	{
		memcpy(DailyUsage, stats.DailyAvgs, sizeof(float)*7);
		lasttime=stats.LastTime;
		rx=stats.rx;
		tx=stats.tx;
		lastrx=stats.lastrx;
		lasttx=stats.lasttx;
		dayrx=stats.dayrx;
		daytx=stats.daytx;
	}
	else if(stats.version==3)
	{
		memcpy(DailyUsage, stats.DailyAvgs, sizeof(float)*7);
		lasttime=stats.LastTime;
		rx=stats.rx;
		tx=stats.tx;
		lastrx=stats.lastrx;
		lasttx=stats.lasttx;
		dayrx=stats.dayrx;
		daytx=stats.daytx;

		billstartyear=stats.startyear;
		billstartmonth=stats.startmonth;
		billstartday=stats.startday;
		billendyear=stats.endyear;
		billendmonth=stats.endmonth;
		billendday=stats.endday;
	}
	else
		SaveUsageStats();

	//billstartmonth = 12;
	//billstartyear = 2016;
	//billendmonth--;
}

string GetHTML(string host, string file)
{
	BasicSocket sock;

	sock.Connect(host.c_str(), 80);
	string send = "GET " + file + " HTTP/1.0\r\nHost: " + host + "\r\nConnection: close\r\nUser-Agent: Die4Ever Stats\r\n\r\n";
	//cout << send << "\n";
	sock.Send(send.c_str(), send.length());
	string response;
	while(sock.Status==sock.Connected)
	{
		char buff[65536];
		if(sock.Recv(buff, 65535)>0)
		{
			buff[sock.iLastRecv] = '\0';
			response += buff;
			//cout << buff <<"\n-\n";
		}
	}
	return response;
}

void ScrapePage()
{
	/*tbillstartyear=2012;
	tbillstartmonth=4;
	tbillstartday=21;
	tbillendyear=2012;
	tbillendmonth=5;
	tbillendday=21;*/

	time_t t;
	time(&t);
	tm now;
	localtime_s(&now, &t);

	uint nowdays = (now.tm_year + 1900) * 366 + (now.tm_mon+1) * 35 + now.tm_mday;
	uint billenddays = tbillendyear * 366 + tbillendmonth * 35 + tbillendday;
	//while(now.tm_year+1900 >= tbillendyear && now.tm_mon+1 >= tbillendmonth && now.tm_mday >= tbillendday)
	while (nowdays >= billenddays)
	{
		tbillstartmonth++;
		tbillendmonth++;
		rx=tx=0;
		if(tbillstartmonth>12)
		{
			tbillstartmonth=1;
			tbillstartyear++;
		}
		if(tbillendmonth>12)
		{
			tbillendmonth=1;
			tbillendyear++;
		}
		billenddays = tbillendyear * 366 + tbillendmonth * 35 + tbillendday;
	}

	//trx = 150000*1024;
	//ttx = 100000*1024;

	string res = GetHTML("192.168.1.254", "/cgi-bin/broadbandstatistics.ha");

	string iptraffic = SubStr(res.c_str(), "summary=\"IPv4 Table\">",
		"</table>",
		false);
	//::MessageBoxA(NULL, iptraffic.c_str(), "...", 0);

	/*ttx = ToUInt64( SubStr(res.c_str(), "<td class=\"fieldlabelsmallbold\">Transmit:</td>\n                        <td class=\"datasmall\">", "</td>", false).c_str() );
	trx = ToUInt64( SubStr(res.c_str(), "<td class=\"fieldlabelsmallbold\">Receive:</td>\n                        <td class=\"datasmall\">", "</td>", false).c_str() );
	string uptime = SubStr(res.c_str(), "Collected for", "</span>", false);*/

	string transmit = SubStr(iptraffic.c_str(), "<th scope=\"row\">Transmit Bytes</th>", "</tr>", false);
	string receive = SubStr(iptraffic.c_str(), "<th scope=\"row\">Receive Bytes</th>", "</tr>", false);

	//::MessageBoxA(NULL, transmit.c_str(), "...", 0);
	//::MessageBoxA(NULL, receive.c_str(), "...", 0);

	if(transmit.length()==0 || receive.length()==0)
	{
		trx=ttx=0;
		return;
	}

	unsigned __int64 ttrx = ToUInt64( SubStr(receive.c_str(), "<td class=\"col2\">", "</td>", false).c_str() );
	unsigned __int64 tttx = ToUInt64( SubStr(transmit.c_str(), "<td class=\"col2\">", "</td>", false).c_str() );

	trx = min(ttrx, ttrx-lastrx);
	ttx = min(tttx, tttx-lasttx);

	tlastrx=ttrx;
	tlasttx=tttx;
	lastupdate = time(0);
}

DWORD WINAPI ScrapeThread(void *ti)
{
	//Sleep(1000 * 10);
	ScrapePage();
	return 0;
}

void UpdateUsage(int dayofweek)
{
	time_t t;
	time(&t);
	tm lt;
	tm now;
	localtime_s(&now, &t);
	//localtime_s(&lt, &t);
	assert(now.tm_wday == dayofweek);
	localtime_s(&lt, &lasttime);//wtf windows?
	//int today = lt.tm_wday;
	//int wday = gmtime(&lasttime)->tm_wday;
	if(today==-1)
		today=lt.tm_wday;

	static time_t threadstart=0;
	unsigned __int64 deltarx,deltatx;
	if(thread==INVALID_HANDLE_VALUE)
	{
		tbillstartyear=billstartyear;
		tbillstartmonth=billstartmonth;
		tbillstartday=billstartday;
		tbillendyear=billendyear;
		tbillendmonth=billendmonth;
		tbillendday=billendday;

		time(&threadstart);
		thread = CreateThread(NULL, 0, ScrapeThread, 0, 0, 0);
		return;
	}
	else
	{
		DWORD status = WaitForSingleObject(thread, 0);
		if(status==WAIT_TIMEOUT && threadstart < time(0)-60)
		{
			CloseHandle(thread);
			thread = INVALID_HANDLE_VALUE;
			//MessageBeep(1);
			Beep(440, 500);
			return;
		}
		else if(status==WAIT_TIMEOUT)
			return;
		else if(status!=WAIT_OBJECT_0)
		{
			CloseHandle(thread);
			thread=INVALID_HANDLE_VALUE;
			return;
		}
		else
		{
			CloseHandle(thread);
			thread = INVALID_HANDLE_VALUE;
		}
	}

	//ScrapePage(deltarx, deltatx);

	billstartyear=tbillstartyear;
	billstartmonth=tbillstartmonth;
	billstartday=tbillstartday;
	billendyear=tbillendyear;
	billendmonth=tbillendmonth;
	billendday=tbillendday;

	lastrx=tlastrx;
	lasttx=tlasttx;

	deltarx = trx;
	deltatx = ttx;

	rx+=deltarx;
	tx+=deltatx;

	used = (float)((rx+tx)/(1024*1024))/1024.0f;

	if(lt.tm_wday == today)
	{
		dayrx+=deltarx;
		daytx+=deltatx;
		deltarx=deltatx=0;
	}

	while(today != now.tm_wday || lasttime<t-86400)
	{
		float avgscale = 4.0f;

		float newavg = (float)((dayrx+daytx)/(1024*1024));//MB
		newavg /= 1024.0f;//GB

		newavg = min(newavg, DailyUsage[today]*2.0f);//don't count anything past a 100% growth?
		newavg = max(newavg, DailyUsage[today]*0.50f);//don't count anything past a 50% shrink?

		//avgscale = 5.0f / abs(newavg - DailyUsage[today]);
		//avgscale = min(30.0f, max(1.0f, avgscale));
		newavg /= avgscale;//the running average scale

		float avg = DailyUsage[today]/avgscale;
		DailyUsage[today] -= avg;
		
		DailyUsage[today] += newavg;
		/*avg = DailyUsage[today];
		for(uint i=0;i<7;i++)
			DailyUsage[i] = avg;*/

		dayrx=daytx=0;
		lasttime+=86400;//time(0);
		lasttime = min(lasttime, time(0));
		//lasttime=time(0);
		localtime_s(&lt, &lasttime);
		today = lt.tm_wday;

		SaveUsageStats();
	}

	/*for(uint i=1;i<6;i++)
			DailyUsage[i] = 3.0f;
	DailyUsage[5] = 7.0f;
	dayrx=0;
	daytx=0;
	rx=lastrx;
	tx=lasttx;
	SaveUsageStats();*/

	if(lt.tm_wday == today)
	{
		dayrx+=deltarx;
		daytx+=deltatx;
		deltarx=deltatx=0;
	}

	usedtoday = (float)((dayrx+daytx)/(1024*1024))/1024.0f;

	static time_t lastsave=time(0);
	if(lastsave<time(0)-600)
		SaveUsageStats(),lastsave=time(0);
}
