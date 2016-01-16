#include "stdlibs.h"
#include "defines.h"

namespace PATHGROUP
{
	char* MACHINE_DATA_PATH = "Machines/M%d.txt";
	char* MODEL_DATA_PATH = "Model/MD%d.txt";
	char* RANDOM_ORIGIN = "Random/Origin/M%d.txt";
}
using namespace PATHGROUP;

int recordindex = 0;

int BSearch(int arry[], int len, int target)
{
	int first = 0;
	int last = len - 1;  
	int mid;
	while (first <= last) 
	{
		mid = (first + last) / 2;
		if (target == arry[mid])
		{
			return mid;
		}
		else
		{
			if (target < arry[mid]) last = mid - 1;
			else first = mid + 1;
		}
	}
	return -1;
}							

inline void ErrorMessageBox(LPCWSTR msg)
{
	MessageBox(0, msg, L"Error", MB_OK);
}

enum { SUNDAY = 0, MONDAY, TUESDAY, WEDNESDAY, THURSDAY, FRIDAY, SATURDAY };
enum { MONTH = 0, DAY, HOUR, MINUTE };

char MonthLimit[13] = { 31, 31, 27, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

int CompareOperRecordPointer(const void* arg1, const void* arg2);

class CTimeRangeCost
{
public :
	WORD from;
	WORD to;
	float cost;
public :
	CTimeRangeCost()
	{
		from = 0;
		to = 0;
		cost = 0;
	}
	CTimeRangeCost(WORD _from, WORD _to, float _cost)
	{
		from = _from;
		to = _to;
		cost = _cost;
	}
};

class CCost
{
public:
	float Cost[13][7][24];	 // [MONTH][WEEKOFDAY][HOUR]
	deque<CTimeRangeCost> range[12][7];	 // 각 날에서 전기비가 같은 구간을 찾아서 큐에 저장
public :
	CCost()
	{
		for (int i = 0; i < 12; ++i) for(int j=0; j<7; ++j) for(int k=0; k<24; ++k) Cost[i][j][k] = 0;
	}
	void ReadFromFile(char *path)
	{
		ifstream input;
		input.open(path);
		if (input.fail())
		{
			ErrorMessageBox(L"FailToReadTimecost");
		}
		istream_iterator<float> it(input);
		for (int i = 1; i < 13; ++i)
		{
			++it; // 표시용으로 몇월을 가리키는지 써놓은 부분을 무시
			for (int j = 0; j < 7; ++j)
			{
				for (int k = 0; k < 24; ++k)
				{
					Cost[i][j][k] = (*it); ++it;
				}
			}
		}
		input.close();
	}
	void SetRange()
	{
		float nowcost;
		WORD from;
		CTimeRangeCost bufctr;
		for (int i = 0; i < 12; ++i)
		{
			for (int j = 0; j < 7; ++j)
			{
				from = 0;
				nowcost = Cost[i][j][0];
				for (int k = 1; k < 24; ++k)
				{
					if (nowcost != Cost[i][j][k])
					{
						bufctr.from = from;
						bufctr.to = k;
						bufctr.cost = nowcost;
						range[i][j].push_back(bufctr);
						from = k;
						nowcost = Cost[i][j][k];
					}
				}
			}
		}
	}
};

class CTime
{
public  :
	short	Year;
	short	Month;
	short	Day;
	short	Hour;
	short	Minute;
	int 	Weekofday;
public :
	CTime(short _Year = 0, short _Month = 0, short _Day = 0, short _Hour = 0, short _Minute = 0, short _Weekofday = 0)
	{
		Year				=	_Year;
		Month			= _Month;
		Day				= _Day;
		Hour				= _Hour;
		Minute			= _Minute;
		Weekofday	= _Weekofday;
	}
	inline void CoverValue(int n)
	{
		Year		= n;
		Month	= n;
		Day		= n;
		Hour		= n;
		Minute	= n;
	}
	tm toTm()
	{
		struct tm timebuf;
		timebuf.tm_year = Year;
		timebuf.tm_mon = Month;
		timebuf.tm_mday = Day;
		timebuf.tm_hour = Hour;
		timebuf.tm_min = Minute;
		return timebuf;
	}	
	double DiffTime(CTime A)
	{
		return (difftime(mktime(&toTm()), mktime(&(A.toTm()))));
	}
	void GetFromTm(tm A)
	{
		Year = A.tm_year;
		Month = A.tm_mon;
		Day = A.tm_mday;
		Hour = A.tm_hour;
		Minute = A.tm_min;
	}
	void operator=(CTime A)
	{
		Year = A.Year;
		Month = A.Month;
		Day = A.Day;
		Hour = A.Hour;
		Minute = A.Minute;
	}	
	bool operator==(CTime A)
	{
		if (Year == A.Year && Month == A.Month && Day == A.Day && Hour == A.Hour && Minute == A.Minute) return true;
		return false;
	}
	CTime operator-(CTime A)
	{
		CTime Return;
		Return.Year = Year - A.Year;
		Return.Month = Month - A.Month;
		Return.Day = Day - A.Day;
		Return.Hour = Hour - A.Hour;
		Return.Minute = Minute - A.Minute;
		Return.CheckRangeZeroOK(MINUTE);
		return Return;
	}
	void CheckRange(int kind)
	{
		switch (kind)
		{
		case MONTH:
			while(Month < 1)
			{
				Month += 12;
				--Year;
			}
			while (Month > 12)
			{
				Month -= 12;
				++Year;
			}
			break;
		case DAY:
			while (Day < 1)
			{
				--Month;
				CheckRange(MONTH);
				Day += MonthLimit[Month];
			}
			while (Day > MonthLimit[Month])
			{
				Day -= MonthLimit[Month];
				++Month;
				CheckRange(MONTH);
			}
			break;
		case HOUR:
			while (Hour < 0)
			{
				Hour += 24;
				--Day;
			}
			while (Hour > 23)
			{
				Hour -= 24;
				++Day;
			}
			CheckRange(DAY);
			break;
		case MINUTE:
			while (Minute < 0)
			{
				Minute += 60;
				--Hour;
			}
			while (Minute > 59)
			{
				Minute -= 60;
				++Hour;
			}
			CheckRange(HOUR);
			break;
		}
	}
	void CheckRangeZeroOK(int kind)
	{
		switch (kind)
		{
		case MONTH:
			while (Month < 0)
			{
				Month += 12;
				--Year;
			}
			while (Month > 12)
			{
				Month -= 12;
				++Year;
			}
			break;
		case DAY:
			while (Day < 0)
			{
				--Month;
				CheckRangeZeroOK(MONTH);
				Day += MonthLimit[Month];
			}
			while (Day > MonthLimit[Month])
			{
				Day -= MonthLimit[Month];
				++Month;
				CheckRangeZeroOK(MONTH);
			}
			break;
		case HOUR:
			while (Hour < 0)
			{
				Hour += 24;
				--Day;
			}
			while (Hour > 23)
			{
				Hour -= 24;
				++Day;
			}
			CheckRangeZeroOK(DAY);
			break;
		case MINUTE:
			while (Minute < 0)
			{
				Minute += 60;
				--Hour;
			}
			while (Minute > 59)
			{
				Minute -= 60;
				++Hour;
			}
			CheckRangeZeroOK(HOUR);
			break;
		}
	}
	inline int HowMinute()
	{
		return Hour * 60 + Minute;
	}
	inline bool isOverADay()
	{
		return (Day > 0) || (Month > 0) || (Year > 0);
	}
	int differbyminute(CTime a)
	{
		int mine = 0;
		int as = 0;
		for (int i = 1; i < Month; ++i)
		{
			mine += MonthLimit[i];
		}
		mine += Day - 1;
		mine *= 24 * 60;
		mine += Hour * 60;
		mine += Minute;
		for (int i = 1; i < a.Month; ++i)
		{
			as += MonthLimit[i];
		}
		as += a.Day - 1;
		as *= 60 * 24;
		as += a.Hour * 60;
		as += a.Minute;
		return abs(as - mine);
	}
	void Print()
	{
		cout << Year << '-' << Month << '-' << Day << '-' << Hour << '-' << Minute << endl;
	}
};

CCost TimeCost;

inline short Zera(CTime ctime)
{
	int a = ctime.Year / 100;
	int b = ctime.Year - (a * 100);
	int c = ctime.Month;
	int d = ctime.Day;
	int rel = ((int)(21 * a / 4) + (int)(5 * b / 4) + (int)(26 * (c + 1) / 10) + d - 1) % 7;
	if (rel < 0 || rel > 6) ErrorMessageBox(L"Zera Error");
	// w=[21a/4]+[5b/4]+[26(c+1)/10]+d-1 
	return (short)(rel);
}// 제라의 공식 : 날짜에서 요일로 

class COperRecord
{
public :
	CTime when;
	int howlong;
	int power;  // W 와트
	float cost;
	int num;
	bool isGlobalFit;
public :
	COperRecord()
	{
		num = -1;
		howlong = -1;
		power = -1;
		cost = -1;
		isGlobalFit = false;
	}
	inline void CoverValue(int n)
	{
		num = n;
		when.CoverValue(n);
		howlong = n;
		power = n;
		cost = (float)n;
	}
	void operator=(COperRecord A)
	{
		num = A.num;
		when = A.when;
		howlong = A.howlong;
		power = A.power;
		cost = A.cost;
	}
	float CalculateCost()
	{
		float Sum = 0.0f;
		int howhour = 0;
		int weekofday = when.Weekofday;
		int lastingminute = howlong;
		Sum += TimeCost.Cost[when.Month][weekofday][when.Hour] * (float)(60 - when.Minute);	  // 처음의 1시간다 못채운는 시간대의 전기세 계산	  (Cost값 * 분)
		lastingminute += when.Minute - 60; // 계산에 더한 시간만큼 뺌
		howhour = lastingminute / 60; // 1시간을 다 채우는 시간대가 얼마나 있는지 계산
		for (int i = 1; i <= howhour; ++i)				   // 1시간 다채우는 시간대에서 전기요금 계산
		{
			if (when.Hour + i >= 24) // 세탁도중에 날짜가 넘어가면 요일 변경
			{
				++weekofday;
				if (weekofday == 7) weekofday = 0;
			}
			Sum += TimeCost.Cost[when.Month][weekofday][(when.Hour) + i] * 60.0f;	   //kWh당을 Wh당으로 변환한후 젼력과 곱
		}
		lastingminute -= howhour * 60;
		Sum += TimeCost.Cost[when.Month][weekofday][(when.Hour) + howhour + 1] * (float)lastingminute;		 // 남은 뒤의 1시간이 안되는 시간의 전기세 계산
		Sum /= 1000.0f * 60.0f;
		Sum *= (float)power;
		cost = Sum;
		return Sum;
	}
};

inline int CompareTime(CTime when1, CTime when2)
{
	int temp;
	temp = when1.Year - when2.Year;
	if (temp > 0) return 1;
	else if (temp < 0) return -1;
	else
	{
		temp = when1.Month - when2.Month;
		if (temp > 0) return 1;
		else if (temp < 0) return -1;
		else
		{
			temp = when1.Day - when2.Day;
			if (temp > 0) return 1;
			else if (temp < 0) return -1;
			else
			{
				temp = when1.Hour - when2.Hour;
				if (temp > 0) return 1;
				else if (temp < 0) return -1;
				else
				{
					temp = when1.Minute - when2.Minute;
					if (temp > 0) return 1;
					else if (temp < 0) return -1;
					else return 0;
				}
			}
		}
	}
}
  
bool isLargerRecordIterator(deque<COperRecord>::iterator when1, deque<COperRecord>::iterator when2)
{
	return (CompareTime(when1->when, when2->when) == -1);
}

int CompareOperRecord(const void *arg1, const void *arg2)
{
	CTime when1 = (*(COperRecord*)arg1).when;
	CTime when2 = (*(COperRecord*)arg2).when;
	return CompareTime(when1, when2);
}

class CMachine
{
public :
	string UsingModel;
	deque<COperRecord> Records;
	deque<int> Intervals;
	float cost;
	int NumUser;
public :
	CMachine(string _UsingModel = "ndefined", int _NumUser = 1)
	{
		UsingModel = _UsingModel;
		NumUser	= _NumUser;
		cost = 0;
	}
	void ReadFromFile(char *path)
	{
		WCHAR wPath[MAX_PATH];
		int num = 1;
		int numofRecords = 0;
		COperRecord RecordBuffer;
		ifstream input;
		input.open(path);
		if (input.fail())										   // 스트림 생성 확인
		{
			ErrorMessageBox(L"Fail to Read MachineData");
			MultiByteToWideChar(CP_ACP, 0, path, -1, wPath, MAX_PATH);
			ErrorMessageBox(wPath);
			UsingModel = "unproper";
			return;
		}
		input >> UsingModel;
		input >> NumUser;
		for (istream_iterator<int> it(input); it != istream_iterator<int>(); )
		{
			RecordBuffer.num = (*it); ++it;
			RecordBuffer.when.Year = (*it); ++it;
			RecordBuffer.when.Month = (*it); ++it;
			RecordBuffer.when.Day = (*it); ++it;
			RecordBuffer.when.Hour = (*it); ++it;
			RecordBuffer.when.Minute = (*it); ++it;
			RecordBuffer.when.Weekofday = Zera(RecordBuffer.when);
			RecordBuffer.power = (*it); ++it;
			RecordBuffer.howlong = (*it); ++it;
			Records.push_back(RecordBuffer);
			RecordBuffer.CoverValue(-1);
		}
		input.close();
	}
	void WriteToFile(char *path)
	{
		WCHAR wPath[MAX_PATH];
		ofstream output;
		output.open(path);
		if (output.fail())
		{
			ErrorMessageBox(L"Fail to Write MachineData");
			MultiByteToWideChar(CP_ACP, 0, path, -1, wPath, MAX_PATH);
			ErrorMessageBox(wPath);
		}
		output << UsingModel << endl
			<< NumUser << endl;
		for (deque<COperRecord>::iterator it = Records.begin(); it != Records.end(); ++it)
		{
			output << it->num << '\t' << it->when.Year << '\t' << it->when.Month << '\t' << it->when.Day << '\t' << it->when.Hour << '\t' << it->when.Minute << '\t' << it->power << '\t' << it->howlong << '\t'<< it->cost << endl;
		}
		output.close();
		output.clear();
	}
	void operator=(CMachine A)
	{
		UsingModel = A.UsingModel;
		Records = A.Records;
		// copy(A.Records.begin(), A.Records.end(), Records.begin());
		NumUser = A.NumUser;
	}
	float CaculateCostRecords()
	{
		cost = 0.0f;
		for (deque<COperRecord>::iterator it = Records.begin(); it != Records.end(); ++it)
		{
			cost += it->CalculateCost();
		}
		return cost;
	}
	void SortByTime()
	{
		int size = Records.size();
		COperRecord *buf = new COperRecord[size];
		int i = 0;
		deque<COperRecord>::iterator it = Records.begin();
		while (it != Records.end())
		{
			buf[i] = *it;
			++i;
			++it;
		}
		qsort((void*)buf, (size_t)size, sizeof(COperRecord), CompareOperRecord);
		Records.clear();
		for (int i = 0; i < size; ++i)
		{
			Records.push_back(buf[i]);
		}
		delete[] buf;
	}
	void CalculateIntervals()
	{
		deque<CTime> integer_que;
		SortByTime();
		Intervals.clear();
		int temp;
		for (deque<COperRecord>::iterator it = Records.begin(); it != Records.end(); ++it)
		{
			integer_que.push_back(it->when);
		}
		for (deque<CTime>::iterator it = integer_que.begin() + 1; it != integer_que.end(); ++it)
		{
			deque<CTime>::iterator leftit = it - 1;
			temp = it->differbyminute(*leftit);
			Intervals.push_back(temp);
		}
	}
};

void MakeRandomMachine(CMachine &mac)
{
	mac.Records.clear();
	mac.NumUser = (rand() % 5) + 1;
	COperRecord recpointer;
	recpointer.when.CoverValue(0);
	recpointer.isGlobalFit = false;
	recpointer.when.Year = 2014;
	recpointer.when.Month = 1;
	recpointer.when.Day = 1;
	int con;
	while (recpointer.when.Year < 2015)
	{
		recpointer.num = recordindex++;
		recpointer.when.Day += 2;
		recpointer.when.Minute += (rand() % 2880);
		recpointer.when.CheckRange(MINUTE);
		if (recpointer.when.Hour < 7)
		{
			con = rand() % 4;
			if (con)
			{
				recpointer.when.Hour = (rand() % 17) + 7;
			}
		}
		recpointer.when.Weekofday = Zera(recpointer.when);
		recpointer.howlong = (rand() % 121) + 60;
		recpointer.howlong /= 10; recpointer.howlong *= 10;
		recpointer.power = (rand() % 80) + 100;
		mac.Records.push_back(recpointer);
	}
}

void MakeRandomMachine(CMachine &mac, int numrec)
{
	mac.Records.clear();
	mac.NumUser = (rand() % 5) + 1;
	COperRecord last;
	COperRecord bufor;
	int con;
	for (int i = 0; i < numrec; ++i)
	{
	k2:
		bufor.num = recordindex++;
		bufor.when.Year = 2014;
		bufor.when.Month = (rand() % 12) + 1;
		bufor.when.Day = (rand() % MonthLimit[bufor.when.Month]) + 1;
		k1:
		bufor.when.Hour = (rand() % 24);
		if (0 <= bufor.when.Hour && bufor.when.Hour <= 6)
		{
			con = (rand() % 3);
			if (con) goto k1;
		}
		bufor.when.Minute = (rand() % 60);
		bufor.when.Weekofday = Zera(bufor.when);
		bufor.howlong = (rand() % 121) + 60;
		bufor.howlong /= 10; bufor.howlong *= 10;
		bufor.power = (rand() % 80) + 100;
		if (bufor.when == last.when) goto k2;
		mac.Records.push_back(bufor);
		last = bufor;
	}
	mac.SortByTime();
}

class CStorage
{
public :
	deque<CMachine>		Machines;		 // 기록된 세탁기들 정보
	deque<int> Intervals;
	int recordnum;
	float costsum;		   // 가장 최근 계산한 총합 비용
	float Cept[144]; // 하루중 분당 전기사용량		   
	bool isCeptSet;
public :
	CStorage()
	{
		costsum = -1.0f;
		isCeptSet = false;
		for (int i = 0; i < 144; ++i)Cept[i] = 0;
	}
	void Clear()
	{
		Machines.clear();
		Intervals.clear();
		recordnum = 0;
		costsum = 0;
		for (int i = 0; i < 144; ++i) Cept[i] = 0;
		isCeptSet = false;
	}
	void ReadFromFile(char *path)
	{
		int numofMachines;
		string stringbuf;
		CMachine MachineBuf;
		char pathbuf[MAX_PATH];
		ifstream input;
		input.open(path);
		if (input.fail())
		{
			ErrorMessageBox(L"FailToReadIndex.txt");
			return;
		}
		input >> numofMachines;
		input.close();
		for (int i = 1; i <= numofMachines; ++i)
		{
			sprintf_s(pathbuf, MAX_PATH, MACHINE_DATA_PATH, i);
			MachineBuf.ReadFromFile(pathbuf);
			MachineBuf.SortByTime();
			Machines.push_back(MachineBuf);
			MachineBuf.Records.clear();
			MachineBuf.cost = 0;
		}
	}
	void WriteToFile(char *path)
	{
		char pathbuf[MAX_PATH];
		int i = 1;
		for (deque<CMachine>::iterator it = Machines.begin(); it != Machines.end(); ++it)
		{
			sprintf_s(pathbuf, MAX_PATH, path, i);
			it->WriteToFile(pathbuf);
			++i;
		}
	}
	void WriteCDFDataToFile(char* path)
	{
		ofstream ofs;
		ofs.open(path);
		for (deque<CMachine>::iterator it_mac = Machines.begin(); it_mac != Machines.end(); ++it_mac)
		{
			ofs << it_mac->cost << endl;
		}
		ofs.close();
	}
	void WriteIntervals(char *path)
	{
		ofstream ofs;
		ofs.open(path);
		if (ofs.fail())
		{
			ErrorMessageBox(L"WriteIntervals Error");
			return;
		}
		for (deque<int>::iterator it = Intervals.begin(); it != Intervals.end(); ++it)
		{
			ofs << (*it) << endl;
		}
		ofs.close();
	}
	void CalculateRecordnum()
	{
		recordnum = 0;
		for (deque<CMachine>::iterator it = Machines.begin(); it != Machines.end(); ++it)
		{
			recordnum = it->Records.size();
		}
	}
	void RenewCostSum()
	{
		costsum = 0.0f;
		for (deque<CMachine>::iterator it = Machines.begin(); it != Machines.end(); ++it)
		{
			costsum += it->CaculateCostRecords();
		}
	}
	void operator=(CStorage A)
	{
		copy(A.Machines.begin(), A.Machines.end(), Machines.begin());
		costsum = A.costsum;
	}
	void NewRandomData(int macnum, int recnum)
	{
		Machines.clear();
		CMachine mac;
		for (int i = 0; i < macnum; ++i)
		{
			MakeRandomMachine(mac, recnum);
			Machines.push_back(mac);
		}
		recordnum = macnum * recnum;
	}
	void NewRandomData(int macnum)
	{
		Machines.clear();
		CMachine mac;
		recordnum = 0;
		for (int i = 0; i < macnum; ++i)
		{
			MakeRandomMachine(mac);
			recordnum += mac.Records.size();
			//printf("%d\n", mac.Records.size());
			Machines.push_back(mac);
		}
	}
	void CalculateCept()
	{
		isCeptSet = true;
		deque<CMachine>::iterator macend = Machines.end();
		deque<COperRecord>::iterator recend;
		int operbegin;
		int operend;
		int temp = 0;
		int a;
		for (deque<CMachine>::iterator it_mac = Machines.begin(); it_mac != macend; ++it_mac)	 // 모든 Records들을 조사해서
		{
			recend = it_mac->Records.end();
			for (deque<COperRecord>::iterator it_or = it_mac->Records.begin(); it_or != recend; ++it_or)
			{
				temp = 0;
				operbegin = (it_or->when.Hour * 6) + ((int)it_or->when.Minute / 10);   // a = (시간 * 60 + 분) / 10
				operend = operbegin + ((int)it_or->howlong / 10);
				for (int i = operbegin; i < operend; ++i)
				{
					a = i;
					while (a > 143) a -= 144;
					Cept[a] += it_or->power;  // Cept[a]에 전력만큼 더함
				}
			}
		}
	}
	void PrintCeptGraph(char *path)
	{
		ofstream output;
		float maxcept = 0;
		float CeptPer[144];
		int cwm = 0;
		output.open(path, ofstream::out);	  
		if (output.fail())
		{
			ErrorMessageBox(L"CeptFile Writing Error");
		}
		if (!isCeptSet)
		{
			ErrorMessageBox(L"Access to Unallocated cept");
			return;
		}	   		   		
		for (int i = 0; i < 144; ++i)
		{
			if (maxcept < Cept[i])
			{
				maxcept = Cept[i];
			}
		}
		for (int i = 0; i < 144; ++i)
		{
			CeptPer[i] = Cept[i] / maxcept;
		}		  
		
		for (int i = 0; i < 144; ++i)
		{
			cwm = (int)(CeptPer[i] * CEPT_WRITING_MAX);
			output << i << " : " << CeptPer[i] << "\t\t";
			if (CeptPer[i] == 1)
			{
				output << "\t";
			}
			for (int j = 0; j < cwm; ++j)
			{
				output << "■";
			}
			output << endl;
		}
		for (int i = 0; i < 144; ++i)
		{
			output << Cept[i] << endl;
		}
		output.close();
		output.clear();
	}
	void CalculateIntervals()
	{
		deque<deque<COperRecord>::iterator> que;
		CTime tbuf;
		for (deque<CMachine>::iterator it_mac = Machines.begin(); it_mac != Machines.end(); ++it_mac)
		{
			for (deque<COperRecord>::iterator it_rec = (*it_mac).Records.begin(); it_rec != it_mac->Records.end(); ++it_rec)
			{
				que.push_back(it_rec);
			}
		}
		sort(que.begin(), que.end(), isLargerRecordIterator);
		Intervals.clear();
		for (deque<deque<COperRecord>::iterator>::iterator it = que.begin()+1; it != que.end(); ++it)
		{
			tbuf = ((*it)->when) - ((*(it-1))->when);
			tbuf.CheckRangeZeroOK(MINUTE);
			Intervals.push_back(tbuf.HowMinute());
		}
	}
};

class CSimulator
{
public :
	CStorage *Original;
	CStorage Optimized;
	deque<int> TimeInterval[144];
	deque<float> Costs[144];
	float ScoreCost;
	double ScoreTime;
	bool isOptimized;
public :
	CSimulator()
	{
		ScoreCost = -1;
		ScoreTime = -1;
		isOptimized = false;
	}
	void virtual Optimization() = 0;
	void Clear()
	{
		Original = 0;
		Optimized.Clear();
		for (int i = 0; i < 144; ++i)
		{
			TimeInterval[i].clear();
			Costs[i].clear();
		}
		ScoreCost = 0;
		ScoreTime = 0;
		isOptimized = false;
	}
	void GetDeltaTime(deque<double> &DeltaTime)
	{
		deque<CMachine>::iterator it_orig = Original->Machines.begin();
		deque<CMachine>::iterator it_opti = Optimized.Machines.begin();
		deque<COperRecord>::iterator it_origrec;
		deque<COperRecord>::iterator it_optirec;
		while (it_opti != Optimized.Machines.end())
		{
			it_origrec = it_orig->Records.begin();
			it_optirec = it_opti->Records.begin();
			while (it_optirec != it_opti->Records.end())
			{
				DeltaTime.push_back(it_origrec->when.DiffTime(it_optirec->when));
				++it_origrec;
				++it_optirec;
			}
			++it_orig;
			++it_opti;
		}
	}
	void TimeIntervalCheck(CStorage *comp)
	{
		deque<deque<COperRecord>::iterator> bList;
		deque<deque<COperRecord>::iterator> aList;
		for (deque<CMachine>::iterator it_mac = comp->Machines.begin(); it_mac != comp->Machines.end(); ++it_mac)
		{
			for (deque<COperRecord>::iterator it_rec = it_mac->Records.begin(); it_rec != it_mac->Records.end(); ++it_rec)
			{
				bList.push_back(it_rec);
			}
		}
		for (deque<CMachine>::iterator it_mac = Optimized.Machines.begin(); it_mac != Optimized.Machines.end(); ++it_mac)
		{
			for (deque<COperRecord>::iterator it_rec = it_mac->Records.begin(); it_rec != it_mac->Records.end(); ++it_rec)
			{
				aList.push_back(it_rec);
			}
		}
		deque<deque<COperRecord>::iterator>::iterator aIt = aList.begin();
		deque<deque<COperRecord>::iterator>::iterator bIt = bList.begin();
		int findinterval;
		int area;
		int tmp;
		CTime interval;
		COperRecord  *reca, *recb;
		deque<COperRecord>::iterator Finder;
		while (aIt != aList.end())
		{
			findinterval = 0;
			while (1)
			{
				Finder = *(bIt + findinterval);
				if (Finder->num == (*aIt)->num)
					break;
				Finder = *(bIt - findinterval);
				if (Finder->num == (*aIt)->num)
					break;
				++findinterval;
			}
			reca = &(*Finder);
			recb = &(*(*aIt));
			tmp = CompareOperRecord((void*)reca, (void*)recb);
			if (tmp==1)
				interval = reca->when - recb->when;
			else	  if (tmp == -1)
				interval = recb->when - reca->when;
			else
				interval.CoverValue(0);
			area = reca->when.HowMinute() / 10;
			TimeInterval[area].push_back(interval.HowMinute());
			++aIt;
			++bIt;
		}
	}
	void SetCosts()
	{
		for (deque<CMachine>::iterator it_mac = Optimized.Machines.begin(); it_mac != Optimized.Machines.end(); ++it_mac)
		{
			for (deque<COperRecord>::iterator it_rec = it_mac->Records.begin(); it_rec != it_mac->Records.end(); ++it_rec)
			{
				CTime *when = &(it_rec->when);
				Costs[(when->Hour * 60 + when->Minute) / 10].push_back(it_rec->cost);
			}
		}
	}
	void WriteAverageIntervalData(char* path)
	{
		int temp;
		ofstream ofs;
		ofs.open(path);
		if (ofs.fail())
		{
			ErrorMessageBox(L"WriteAverageIntervalData OFS Error");
			return;
		}
		int sum;

		deque<int> *nowinterval;

		for (int i = 0; i < 144; ++i)
		{
			nowinterval = &(TimeInterval[i]);
			sum = 0;
			for (deque<int>::iterator it = (*nowinterval).begin(); it != (*nowinterval).end(); ++it)
			{
				sum += (*it);
			}
			temp = (*nowinterval).size();
			if (temp) sum /= temp;
			else sum = 0;
			ofs << sum << endl;
		}

		ofs.close();
	}
	void WriteAverageCost(char* path)
	{
		ofstream ofs;
		ofs.open(path);
		if (ofs.fail())
		{
			ErrorMessageBox(L"WriteAverageCost Error");
			return;
		}
		for (int i = 0; i < 144; ++i)
		{
			float Sum = 0.0f;
			for (deque<float>::iterator it = Costs[i].begin(); it != Costs[i].end(); ++it)
			{
				Sum += (*it);
			}
			if (Costs[i].size()) Sum /= Costs[i].size();
			else Sum = 0.0f;
			ofs << Sum << endl;
		}
		ofs.close();
	}
	void WriteRawIntervalData(char* path)
	{
		ofstream ofs;
		ofs.open(path);
		if (ofs.fail())
		{
			ErrorMessageBox(L"WriteRawIntervalData OFS Error");
			return;
		}
		deque<int> *nowinterval;
		for(int i = 0; i < 144; ++i)
		{
			nowinterval = &(TimeInterval[i]);
			for (deque<int>::iterator it = (*nowinterval).begin(); it != (*nowinterval).end(); ++it)
			{
				ofs << *it << " ";
			}
			ofs << endl;
		}
		ofs.close();
	}
	void Evaluation()
	{
		if (!isOptimized)
		{
			float DeltaCost;
			double AverageDeltaTime = 0;
			Optimized.RenewCostSum();
			DeltaCost = Original->costsum - Optimized.costsum;		// 전기비가 얼마나 바뀌었는지 계산
			deque<double> DeltaTime;
			this->GetDeltaTime(DeltaTime);		  // 최적화로 인한 작동시간의 변화량을 큐에 저장
			for (deque<double>::iterator it = DeltaTime.begin(); it != DeltaTime.end(); ++it)
			{
				AverageDeltaTime += (*it);
			}
			AverageDeltaTime /= DeltaTime.size(); // 평균 시간 변화량을 계산
			AverageDeltaTime /= 60; // 분 단위로 환산
			ScoreTime = (double)1.0 - (AverageDeltaTime / (double)MAX_INTERVAL_MINUTE);
			ScoreCost = DeltaCost;
		}
	}
};

class CBrutalSimulator : public CSimulator
{
public :
public :
	void Optimization()
	{
		Optimized.Machines.clear();
		Optimized.costsum = 0;
		float BestIntervalValue;	// 최근의 가장 좋은 값
		int BestIntervalPos;
		float temp;
		CTime Origin;
		Optimized.Machines = (*Original).Machines; // Original의 데이터를 Optimized에 그대로 복사
		for (deque<CMachine>::iterator it_ma = Optimized.Machines.begin(); it_ma != Optimized.Machines.end(); ++it_ma)		// Optimized에 들어있는 모든 Records들을 조사
		{
			for (deque<COperRecord>::iterator it_or = it_ma->Records.begin(); it_or != it_ma->Records.end(); ++it_or)
			{
				it_or->when.Minute -= MAX_INTERVAL_MINUTE;	it_or->when.CheckRange(MINUTE);
				Origin = it_or->when;
				BestIntervalValue = it_or->CalculateCost();
				BestIntervalPos = 0;
				for (int i = BRUTAL_INTERVAL; i <= 2 * MAX_INTERVAL_MINUTE; i += BRUTAL_INTERVAL)  // 옮길 수 있는 영역에서의 전기비를 계산해서 가장 전기비가 싸고 그 다음으로 처음이랑 가장 가까운 곳을 찾음.
				{
					it_or->when.Minute += BRUTAL_INTERVAL; it_or->when.CheckRange(MINUTE);
					temp = BestIntervalValue - it_or->CalculateCost();
					if (temp > 0.01f || (temp == 0 && abs(MAX_INTERVAL_MINUTE - BestIntervalPos) > abs(MAX_INTERVAL_MINUTE - i)))
					{
						BestIntervalValue = it_or->cost;
						BestIntervalPos = i;
					}
				}	
				Origin.Minute += BestIntervalPos; Origin.CheckRange(MINUTE);	// 찾은 가장 좋은 곳을 Record에 적용시켜줌
				it_or->when = Origin;
				it_or->CalculateCost();
			}
		}
	}
};

int CompareOperRecordPointer(const void* arg1, const void* arg2)
{
	return CompareTime((*(COperRecord**)arg1)->when, (*(COperRecord**)arg2)->when);
}

#define IGNORENUM 1
#define BACKNUM 2

class CGlobalSimulator : public CSimulator
{
public :
	int ignorefirst[IGNORENUM];	   // ignore파트의 첫부분들
	int ignorelast[IGNORENUM];	  //  ignore파트의 뒷부분들
	int gobackfirst[BACKNUM];		 // goback파트의 첫부분들
	int gobacklast[BACKNUM];	   // goback파트의 뒷부분들
public :
	CGlobalSimulator()
	{
		ignorefirst[0] = 720; // 12시 부터 16시 사이의 Record들은 최적화에서 제외
		ignorelast[0] = 960;
		//ignorefirst[1] = 0;
		//ignorelast[1] = 350;
		//ignorefirst[2] = 1350;
		//ignorelast[2] = 1440;
		gobackfirst[0] = 0;		   // 0시 부터 7시20분 사이의 Record들은 최적화된 이후에 다시 원래 값으로 돌아감.
		gobacklast[0] = 440;
		gobackfirst[1] = 1270;	   // 	21시10분 부터 24시 사이의 Record들도 최적화된 이후에 다시 원래 값으로 돌아감.
		gobacklast[1] = 1440;
	}
	void Optimization()		 
	{
		// 포인터(it, ti로 이루어짐)가 일렬로, 크기 순으로 배열된 Record배열을 맨 왼쪽 부터 맨 오른쪽 부터 옮겨다니면서 Record를 끌어오고 Record의 howlong만큼 오른쪽으로 가는 것을 모든 Record의 isGlobalFit이 true가 될 때까지 반복함.
		// IGNORE파트는 말그대로 최적화에서 제외할 시간대이다.
		COperRecord **its;	// Records들의 포인터를 일렬로 저장할 버퍼
		COperRecord *cortemp;
		int left, right;
		int intervalright = 0;
		int  intervalleft = 0;
		int temp;
		int temp2;
		int itPointer;			// 포인터 보다 큰 Record중 가장 작은 Record의 its에서의 index
		CTime  tiPointer;	// 현재 포인터의 위치
		CTime timetemp;
		CTime StartTime;
		int index_its = 0;
		int maxindex_its = 0;
		int sumofdatas = 0;
		Optimized.Machines = Original->Machines;
		for(deque<CMachine>::iterator it_mac = Optimized.Machines.begin(); it_mac != Optimized.Machines.end(); ++it_mac)   // Records.size()들 총합 구함
			sumofdatas += it_mac->Records.size();
		its = new COperRecord*[sumofdatas];
		for (deque<CMachine>::iterator it_mac = Optimized.Machines.begin(); it_mac != Optimized.Machines.end(); ++it_mac)	  // Records들을 버퍼에 일렬로 복사
		{
			for(deque<COperRecord>::iterator it_rec = it_mac->Records.begin(); it_rec != it_mac->Records.end(); ++it_rec)
			{
				its[index_its++] = &(*it_rec);
			}
		}
		maxindex_its = index_its - 1;
		qsort((void*)its, (size_t)index_its, sizeof(COperRecord*), CompareOperRecordPointer);	  // Records를 시간 순으로 정렬
		StartTime = its[0]->when;
		itPointer = 0;
		tiPointer = StartTime;
		for (int i = 0; i < index_its; ++i)		   // IGNORE파트에 있는 Record의 isGlobalFit을 true로 바꿔줌
		{
			cortemp = &(*its[i]);
			cortemp->isGlobalFit = false;
			temp = cortemp->when.HowMinute();
			temp2 = temp + cortemp->howlong;
			for (int j = 0; j < IGNORENUM; ++j)
			{
				if (ignorefirst[j] <= temp && temp < ignorelast[j]){
					cortemp->isGlobalFit = true;
					break;
				}
			}
		}
		while (1)
		{
		l1:
			for (int i = 0; i < IGNORENUM; ++i)						// ti포인터가 IGNORE파트에 있으면 IGNORE파트 뒤로 뜀
			{
				if (tiPointer.Hour >= ignorefirst[i] && tiPointer.Hour < ignorelast[i])		 // IGNORE파트에 있는지 확인
				{
					tiPointer.Hour = ignorelast[i];
					while (CompareTime(tiPointer, (*its[itPointer]).when) == 1)	   // itPointer를 tiPointer가 가리키고 있는 시간과 큰 쪽으로 가장 가까운 시간으로함
					{
						++itPointer;
						if (itPointer >= index_its)
						{
							for (int j = 0; j < index_its; ++j)
							{
								if (!(its[j]->isGlobalFit))
								{
									itPointer = j;
									// cout << i << endl;
									tiPointer = its[j]->when;
									goto l1;
								}
							}
							delete[] its;
							return;
						}
					}
				}
			}  
			temp = itPointer;
			while (1)				   // ti포인터 오른쪽으로 가장 가까운 Record탐색
			{
				if (temp >= index_its)
				{
					intervalright = 99999; break;
				}
				if (!(its[temp]->isGlobalFit))	   // isGlobalFIt이 false인지 검사
				{
					timetemp = its[temp]->when - tiPointer;		  // ti포인터와 너무 거리가 먼지 검사
					timetemp.CheckRangeZeroOK(MINUTE);
					if (!(timetemp.isOverADay()))
					{
						intervalright = timetemp.HowMinute();
						if (intervalright <= MAX_INTERVAL_MINUTE_GLOBAL)
						{
							right = temp;
							// cout << "Right : " << right << endl;
							break;
						}
						else
						{
							intervalright = 99999; break;	   // 99999로 해놓아서 절대로 오른쪽에 있는 Record를 선택하지 못하게 함
						}
					}
					else
					{
						intervalright = 99999; break;
					}
				}	 
				++temp;
			}
			temp = itPointer - 1;
			while (1)			  // it포인터 왼쪽으로 가장 가까운 Record탐색
			{
				if (temp < 0)
				{
					intervalleft = 99999; break;
				}
				if (!(its[temp]->isGlobalFit))	   // isGlobalFIt이 false인지 검사
				{
					timetemp = tiPointer - its[temp]->when;
					timetemp.CheckRangeZeroOK(MINUTE);
					if (!timetemp.isOverADay())						 // ti포인터와 너무 거리가 먼지 검사
					{
						intervalleft = timetemp.HowMinute();
						if (intervalleft <= MAX_INTERVAL_MINUTE_GLOBAL)
						{
							left = temp;
							// cout << "Left : " << left << endl;
							break;
						}
						else
						{
							intervalleft = 99999; break;
						}
					}
					else
					{
						intervalleft = 99999; break;
					}
				}
				--temp;
			}
			if (intervalleft == 99999 && intervalright == 99999)	 // 왼쪽으로도 오른쪽으도 옮겨올만한 적당한 Record가 없을 경우
			{
				tiPointer.Minute += 10;				// tiPointer를 10분 오른쪽으로 옮김
				tiPointer.CheckRange(MINUTE);
				timetemp = its[maxindex_its]->when - tiPointer;
				timetemp.CheckRangeZeroOK(MINUTE);
				if (timetemp.Year < 0)								// tiPointer가 Records의 when의 최대값을 넘어섰을경우
				{
					for (int i = 0; i < index_its; ++i)		   // isGlobalFit가 false인 Record중 가장 왼쪽에 있는것으로 itPointer, tiPointer를 옮김
					{
						if (!(its[i]->isGlobalFit))		// 모든 Records의 isGlobalFit가 true인 경우 최적화를 끝냄
						{
							itPointer = i;
							tiPointer = its[i]->when;
							goto l1;
						}
					}
					delete[] its;
					return;
				}
				continue;
			}
			
			if (intervalright > intervalleft) // 왼쪽의 가장 가까운 isGlobalFit가 false인 Record 가 오른쪽의 것보다 tiPointer간의 거리가 짧음. (왼쪽 것이 오른쪽 것 보다 더 좋음)
			{
				its[left]->when = tiPointer;		  // 왼쪽의 것의 when을 tiPointer와 같은 값으로 함.
				its[left]->isGlobalFit = true;		   // 왼쪽의 것의 isGlobalFit를 true로 함.
				tiPointer.Minute += its[left]->howlong;	// tiPointer를 왼쪽의 것의 howlong만큼 오른쪽으로 옮김
				tiPointer.CheckRange(MINUTE);
			}
			else		  // 오른쪽 것이 더 좋음
			{
				its[right]->when = tiPointer;	 // 오른쪽의 것의 when을 tiPointer와 같은 값으로 함.
				its[right]->isGlobalFit = true;		  // 오른쪽의 것의 isGlobalFit를 true로 함.
				tiPointer.Minute += its[right]->howlong;  // tiPointer를 오른쪽의 것의 howlong만큼 오른쪽으로 옮김
				tiPointer.CheckRange(MINUTE);
			}
			while (CompareTime(tiPointer, (*its[itPointer]).when) == 1)	   // itPointer를 tiPointer가 가리키고 있는 시간과 큰 쪽으로 가장 가까운 시간으로함
			{
				++itPointer;
				if (itPointer >= index_its) // itPointer가 최대값을 넘어버렸을 경우	  
				{
					for(int i=0; i < index_its; ++i)  // isGlobalFit가 false인 Record중 가장 왼쪽의 것을 찾아 ti, it포인터에 넣어줌
					{
						if(!(its[i]->isGlobalFit))	 
						{
							itPointer = i;
							// cout << i << endl;
							tiPointer = its[i]->when;
							goto l1;
						}
					}
					delete[] its;	   // isGlobalFit가 모두 true면 최적화를 종료.
					return;
				}
			}
		}
		delete[] its;
	}
	void GoBack(CStorage *comp)
	{
		deque<COperRecord>::iterator *pasts;		   //  최적화 전의 예전 Record들을 pasts에 저장
		int temp = 0;
		for (deque<CMachine>::iterator it = comp->Machines.begin(); it != comp->Machines.end(); ++it)
		{
			temp += it->Records.size();
		}
		pasts = new deque<COperRecord>::iterator[temp];
		int index_pasts = 0;
		for (deque<CMachine>::iterator it = comp->Machines.begin(); it != comp->Machines.end(); ++it)
		{
			for (deque<COperRecord>::iterator it_rec = it->Records.begin(); it_rec != it->Records.end(); ++it_rec)
			{
				pasts[index_pasts++] = it_rec;
			}
		}
		for (deque<CMachine>::iterator it_mac = Optimized.Machines.begin(); it_mac != Optimized.Machines.end(); ++it_mac)  	 // Optimized 의 모든 Record들을 확인하면서  goback영역 안에 있으면 예전으로 복원
		{
			for (deque<COperRecord>::iterator it_rec = it_mac->Records.begin(); it_rec != it_mac->Records.end(); ++it_rec)
			{
				CTime *when = &(it_rec->when);
				int area = when->Hour * 60 + when->Minute;
				bool goback = false;
				for (int i = 0; i < BACKNUM; ++i)  //goback영역에 있는지 확인
				{
					if (gobackfirst[i] <= area && area <= gobacklast[i]) goback = true;
				}
				if (goback)		//goback영역에 있으면
				{
					int recnum = it_rec->num;
					for (int i = 0; i < index_pasts; ++i)		// pasts에 있는 Record들중 num가 같은 기록과 같은 값으로 해 줌.
					{
						if (pasts[i]->num == recnum) it_rec->when = pasts[i]->when;
					}
				}										 
			}
		}
	}
};
							
CStorage OriginalData;
CStorage RandomData;
CBrutalSimulator Brutal;
CGlobalSimulator Global;

class trint
{
public:
	float a;
	float b;
	float c;
	trint()
	{
		a = 0;
		b = 0;
		c = 0;
	}
};

void PrintCostSums(char *path)
{
	ofstream ofs;
	int num = RandomData.Machines.size();
	trint *buf = new trint[num];
	int i = 0;
	for (deque<CMachine>::iterator it = RandomData.Machines.begin(); it != RandomData.Machines.end(); ++it)
	{
		buf[i++].a = it->cost;
	}
	i = 0;
	for (deque<CMachine>::iterator it = Brutal.Optimized.Machines.begin(); it != Brutal.Optimized.Machines.end(); ++it)
	{
		buf[i++].b = it->cost;
	}
	i = 0;
	for (deque<CMachine>::iterator it = Global.Optimized.Machines.begin(); it != Global.Optimized.Machines.end(); ++it)
	{
		buf[i++].c = it->cost;
	}
	ofs.open(path);
	if (ofs.fail())
	{
		ErrorMessageBox(L"PrintCostSum Fail");
		return;
	}
	for (i = 0; i < num; ++i)
	{
		ofs << buf[i].a << "\t" << buf[i].b << "\t" << buf[i].c << endl;
	}
	ofs.close();
	delete[] buf;
}

void PrintMachineRecordIntervals(char *path)
{
	ofstream ofs;
	ofs.open(path);
	if (ofs.fail())
	{
		ErrorMessageBox(L"PrintMachineRecordIntervals Error");
		return;
	}
	int recnum = RandomData.Machines.size();
	deque<CMachine>::iterator it_mac_random = RandomData.Machines.begin();
	deque<CMachine>::iterator it_mac_local = Brutal.Optimized.Machines.begin();
	deque<CMachine>::iterator it_mac_global = Brutal.Optimized.Machines.begin();
	int housenum = 1;
	while (it_mac_random != RandomData.Machines.end())
	{
		it_mac_random->CalculateIntervals();
		it_mac_local->CalculateIntervals();
		it_mac_global->CalculateIntervals();
		ofs << "House" << housenum++ << '\t';
		deque<int>::iterator it_int_random = it_mac_random->Intervals.begin();
		deque<int>::iterator it_int_local = it_mac_local->Intervals.begin();
		deque<int>::iterator it_int_global = it_mac_global->Intervals.begin();
		while (it_int_random != it_mac_random->Intervals.end())
		{
			ofs << *it_int_random << '\t';
			++it_int_random;
		}
		ofs << endl << '\t';
		while (it_int_local != it_mac_local->Intervals.end())
		{
			ofs << *it_int_local << '\t';
			++it_int_local;
		}
		ofs << endl << '\t';
		while (it_int_global != it_mac_global->Intervals.end())
		{
			ofs <<*it_int_global << '\t';
			++it_int_global;
		}
		ofs << endl;
		++it_mac_random;
		++it_mac_local;
		++it_mac_global;
	}
	ofs.close();
}

int main(int argc, char **argv)
{
	ofstream ofs;
	ofs.open("Console.txt");
	CTime aa, bb;
	aa.Hour = 3;
	bb.Hour = 4;
	CTime cc = bb - aa;
	printf("%d\n", cc.HowMinute());
	DWORD a, b;
	system("title seatak");
	srand((unsigned int)time(NULL));
	TimeCost.ReadFromFile("Costdata.txt");	  // 데이터 입력
	OriginalData.ReadFromFile("Index.txt");
	OriginalData.RenewCostSum();
	cout << "The Cost from original records : " << OriginalData.costsum << endl;	
	ofs << "The Cost from original records : " << OriginalData.costsum << endl;

	OriginalData.WriteToFile("Output/Original/M%d.txt");

	Brutal.Original = &OriginalData;	 // Original의 Local Optimization
	Brutal.Optimization();
	Brutal.Evaluation();
	Brutal.Optimized.WriteToFile("Output/Brutal/M%d.txt");
	cout << "The Cost from Brutally Optimized Records : " << Brutal.Optimized.costsum << endl
		<< "The Time Score from optimized records : " << Brutal.ScoreTime << endl << endl;
	ofs << "The Cost from Brutally Optimized Records : " << Brutal.Optimized.costsum << endl
		<< "The Time Score from optimized records : " << Brutal.ScoreTime << endl << endl;

	Global.Original = &(Brutal.Optimized);	 // Original의 Global Optimization
	Global.Optimization();
	Global.Evaluation();
	Global.Optimized.WriteToFile("Output/Global/M%d.txt");
	cout << "The Cost from Globally Optimized Records : " << Global.Optimized.costsum << endl
		<< "The Time Score from optimized records : " << Global.ScoreTime << endl << endl;
	ofs << "The Cost from Globally Optimized Records : " << Global.Optimized.costsum << endl
		<< "The Time Score from optimized records : " << Global.ScoreTime << endl << endl;

	Brutal.Optimized.CalculateCept();						   // Original CEPT 출력
	Brutal.Optimized.PrintCeptGraph("Cept/OriginalBrutal.txt");

	Global.Optimized.CalculateCept();
	Global.Optimized.PrintCeptGraph("Cept/OriginalGlobal.txt");

	Brutal.Optimized.WriteCDFDataToFile("CDFData/Brutal.txt");	 // Original CDF자료 출력																 
	Global.Optimized.WriteCDFDataToFile("CDFData/Global.txt");

	Brutal.TimeIntervalCheck(&OriginalData);		 //Original  Interval데이터 계산및 출력 
	Brutal.WriteAverageIntervalData("AverageIntervals/OriginalBrutal.txt");
	Brutal.WriteRawIntervalData("RawIntervals/OriginalBrutal.txt");
	Global.TimeIntervalCheck(&OriginalData);
	Global.WriteAverageIntervalData("AverageIntervals/OriginalGlobal.txt");
	Global.WriteRawIntervalData("RawIntervals/OriginalGlobal.txt");

	Brutal.SetCosts();						  // Original 시간대별 전기세평균 출력
	Brutal.WriteAverageCost("Costsbytime/OriginalBrutal.txt");
	Global.SetCosts();
	Global.WriteAverageCost("Costsbytime/OriginalGlobal.txt");

	Brutal.Clear();
	Global.Clear();

	RandomData.NewRandomData(1000);	// 랜덤 데이터 생성
	RandomData.WriteToFile(RANDOM_ORIGIN);
	RandomData.RenewCostSum();
	cout << "The Cost from Random original records : " << RandomData.costsum << endl;
	ofs << "The Cost from Random original records : " << RandomData.costsum << endl;

	Brutal.Original = &RandomData;		// Random의 Brutal Optimziation
	a = GetTickCount();
	Brutal.Optimization();
	b = GetTickCount();
	Brutal.Optimized.WriteToFile("Random/Brutal/M%d.txt");
	Brutal.Evaluation();
	cout << "The Cost from Brutally Optimized Random Records : " << Brutal.Optimized.costsum << endl
		<< "The Time Score from optimized records : " << Brutal.ScoreTime << endl
		<< "Time for optimization  : " << b-a << endl;
	ofs << "The Cost from Brutally Optimized Random Records : " << Brutal.Optimized.costsum << endl
		<< "The Time Score from optimized records : " << Brutal.ScoreTime << endl
		<< "Time for optimization  : " << b - a << endl;

	Global.Original = &(Brutal.Optimized);		// Random의 Global Optimization
	a = GetTickCount();
	Global.Optimization();
	Global.GoBack(&RandomData);
	b = GetTickCount();
	Global.Evaluation();
	Global.Optimized.WriteToFile("Random/Global/M%d.txt");
	cout << "The Cost from Globally Optimized Random Records : " << Global.Optimized.costsum << endl
		<< "The Time Score from optimized records : " << Global.ScoreTime << endl
		<< "Time for optimization : " << b-a << endl;
	ofs << "The Cost from Globally Optimized Random Records : " << Global.Optimized.costsum << endl
		<< "The Time Score from optimized records : " << Global.ScoreTime << endl
		<< "Time for optimization : " << b - a << endl;

	PrintCostSums("costsums.txt"); // 랜덤데이터의 각 가정의 전기세합의 변화출력

	Brutal.Optimized.CalculateCept();						   // CEPT 출력
	Brutal.Optimized.PrintCeptGraph("Cept/Brutal.txt");

	OriginalData.CalculateCept();
	OriginalData.PrintCeptGraph("Cept/Original.txt");

	RandomData.CalculateCept();
	RandomData.PrintCeptGraph("Cept/Random.txt");

	Global.Optimized.CalculateCept();
	Global.Optimized.PrintCeptGraph("Cept/Global.txt");

	RandomData.WriteCDFDataToFile("CDFData/Random.txt"); // CDF자료 출력
	Brutal.Optimized.WriteCDFDataToFile("CDFData/Brutal.txt");
	Global.Optimized.WriteCDFDataToFile("CDFData/Global.txt");

	Brutal.TimeIntervalCheck(&RandomData);		 // Interval데이터 계산및 출력 
	Brutal.WriteAverageIntervalData("AverageIntervals/Brutal.txt");
	Brutal.WriteRawIntervalData("RawIntervals/Brutal.txt");
	Global.TimeIntervalCheck(&RandomData);
	Global.WriteAverageIntervalData("AverageIntervals/Global.txt");
	Global.WriteRawIntervalData("RawIntervals/Global.txt");

	Brutal.SetCosts();						  // 시간대별 전기세평균 출력
	Brutal.WriteAverageCost("Costsbytime/Brutal.txt");
	Global.SetCosts();
	Global.WriteAverageCost("Costsbytime/Global.txt");

	RandomData.CalculateIntervals();	  // 기록들 간의 Interval 출력
	Brutal.Optimized.CalculateIntervals();
	Global.Optimized.CalculateIntervals();
	RandomData.WriteIntervals("RecordIntervals/Random.txt");   
	Brutal.Optimized.WriteIntervals("RecordIntervals/Local.txt");
	Global.Optimized.WriteIntervals("RecordIntervals/Global.txt");

	PrintMachineRecordIntervals("MachineIntervals.txt"); // 

	cout << "Done!" << endl;
	ofs.close();
	_getch();

	return 0;
}
