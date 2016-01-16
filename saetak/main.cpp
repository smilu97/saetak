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
	deque<CTimeRangeCost> range[12][7];	 // �� ������ ����� ���� ������ ã�Ƽ� ť�� ����
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
			++it; // ǥ�ÿ����� ����� ����Ű���� ����� �κ��� ����
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
}// ������ ���� : ��¥���� ���Ϸ� 

class COperRecord
{
public :
	CTime when;
	int howlong;
	int power;  // W ��Ʈ
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
		Sum += TimeCost.Cost[when.Month][weekofday][when.Hour] * (float)(60 - when.Minute);	  // ó���� 1�ð��� ��ä��� �ð����� ���⼼ ���	  (Cost�� * ��)
		lastingminute += when.Minute - 60; // ��꿡 ���� �ð���ŭ ��
		howhour = lastingminute / 60; // 1�ð��� �� ä��� �ð��밡 �󸶳� �ִ��� ���
		for (int i = 1; i <= howhour; ++i)				   // 1�ð� ��ä��� �ð��뿡�� ������ ���
		{
			if (when.Hour + i >= 24) // ��Ź���߿� ��¥�� �Ѿ�� ���� ����
			{
				++weekofday;
				if (weekofday == 7) weekofday = 0;
			}
			Sum += TimeCost.Cost[when.Month][weekofday][(when.Hour) + i] * 60.0f;	   //kWh���� Wh������ ��ȯ���� ���°� ��
		}
		lastingminute -= howhour * 60;
		Sum += TimeCost.Cost[when.Month][weekofday][(when.Hour) + howhour + 1] * (float)lastingminute;		 // ���� ���� 1�ð��� �ȵǴ� �ð��� ���⼼ ���
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
		if (input.fail())										   // ��Ʈ�� ���� Ȯ��
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
	mac.Records.pop_back();
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
	deque<CMachine>		Machines;		 // ��ϵ� ��Ź��� ����
	deque<int> Intervals;
	int recordnum;
	float costsum;		   // ���� �ֱ� ����� ���� ���
	float Cept[144]; // �Ϸ��� �д� �����뷮		   
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
		for (deque<CMachine>::iterator it_mac = Machines.begin(); it_mac != macend; ++it_mac)	 // ��� Records���� �����ؼ�
		{
			recend = it_mac->Records.end();
			for (deque<COperRecord>::iterator it_or = it_mac->Records.begin(); it_or != recend; ++it_or)
			{
				temp = 0;
				operbegin = (it_or->when.Hour * 6) + ((int)it_or->when.Minute / 10);   // a = (�ð� * 60 + ��) / 10
				operend = operbegin + ((int)it_or->howlong / 10);
				for (int i = operbegin; i < operend; ++i)
				{
					a = i;
					while (a > 143) a -= 144;
					Cept[a] += it_or->power;  // Cept[a]�� ���¸�ŭ ����
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
				output << "��";
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
			DeltaCost = Original->costsum - Optimized.costsum;		// ����� �󸶳� �ٲ������ ���
			deque<double> DeltaTime;
			this->GetDeltaTime(DeltaTime);		  // ����ȭ�� ���� �۵��ð��� ��ȭ���� ť�� ����
			for (deque<double>::iterator it = DeltaTime.begin(); it != DeltaTime.end(); ++it)
			{
				AverageDeltaTime += (*it);
			}
			AverageDeltaTime /= DeltaTime.size(); // ��� �ð� ��ȭ���� ���
			AverageDeltaTime /= 60; // �� ������ ȯ��
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
		float BestIntervalValue;	// �ֱ��� ���� ���� ��
		int BestIntervalPos;
		float temp;
		CTime Origin;
		Optimized.Machines = (*Original).Machines; // Original�� �����͸� Optimized�� �״�� ����
		for (deque<CMachine>::iterator it_ma = Optimized.Machines.begin(); it_ma != Optimized.Machines.end(); ++it_ma)		// Optimized�� ����ִ� ��� Records���� ����
		{
			for (deque<COperRecord>::iterator it_or = it_ma->Records.begin(); it_or != it_ma->Records.end(); ++it_or)
			{
				it_or->when.Minute -= MAX_INTERVAL_MINUTE;	it_or->when.CheckRange(MINUTE);
				Origin = it_or->when;
				BestIntervalValue = it_or->CalculateCost();
				BestIntervalPos = 0;
				for (int i = BRUTAL_INTERVAL; i <= 2 * MAX_INTERVAL_MINUTE; i += BRUTAL_INTERVAL)  // �ű� �� �ִ� ���������� ����� ����ؼ� ���� ����� �ΰ� �� �������� ó���̶� ���� ����� ���� ã��.
				{
					it_or->when.Minute += BRUTAL_INTERVAL; it_or->when.CheckRange(MINUTE);
					temp = BestIntervalValue - it_or->CalculateCost();
					if (temp > 0.01f || (temp == 0 && abs(MAX_INTERVAL_MINUTE - BestIntervalPos) > abs(MAX_INTERVAL_MINUTE - i)))
					{
						BestIntervalValue = it_or->cost;
						BestIntervalPos = i;
					}
				}	
				Origin.Minute += BestIntervalPos; Origin.CheckRange(MINUTE);	// ã�� ���� ���� ���� Record�� ���������
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
	int ignorefirst[IGNORENUM];	   // ignore��Ʈ�� ù�κе�
	int ignorelast[IGNORENUM];	  //  ignore��Ʈ�� �޺κе�
	int gobackfirst[BACKNUM];		 // goback��Ʈ�� ù�κе�
	int gobacklast[BACKNUM];	   // goback��Ʈ�� �޺κе�
public :
	CGlobalSimulator()
	{
		ignorefirst[0] = 720; // 12�� ���� 16�� ������ Record���� ����ȭ���� ����
		ignorelast[0] = 960;
		//ignorefirst[1] = 0;
		//ignorelast[1] = 350;
		//ignorefirst[2] = 1350;
		//ignorelast[2] = 1440;
		gobackfirst[0] = 0;		   // 0�� ���� 7��20�� ������ Record���� ����ȭ�� ���Ŀ� �ٽ� ���� ������ ���ư�.
		gobacklast[0] = 440;
		gobackfirst[1] = 1270;	   // 	21��10�� ���� 24�� ������ Record�鵵 ����ȭ�� ���Ŀ� �ٽ� ���� ������ ���ư�.
		gobacklast[1] = 1440;
	}
	void Optimization()		 
	{
		// ������(it, ti�� �̷����)�� �Ϸķ�, ũ�� ������ �迭�� Record�迭�� �� ���� ���� �� ������ ���� �Űܴٴϸ鼭 Record�� ������� Record�� howlong��ŭ ���������� ���� ���� ��� Record�� isGlobalFit�� true�� �� ������ �ݺ���.
		// IGNORE��Ʈ�� ���״�� ����ȭ���� ������ �ð����̴�.
		COperRecord **its;	// Records���� �����͸� �Ϸķ� ������ ����
		COperRecord *cortemp;
		char tempstring[128];
		int left, right;
		int count_optimized = 0;
		int lastprint_count_optimized = 0;
		int intervalright = 0;
		int  intervalleft = 0;
		int temp;
		int temp2;
		int itPointer;			// ������ ���� ū Record�� ���� ���� Record�� its������ index
		CTime  tiPointer;	// ���� �������� ��ġ
		CTime timetemp;
		CTime StartTime;
		int index_its = 0;
		int maxindex_its = 0;
		int sumofdatas = 0;
		Optimized.Machines = Original->Machines;
		for(deque<CMachine>::iterator it_mac = Optimized.Machines.begin(); it_mac != Optimized.Machines.end(); ++it_mac)   // Records.size()�� ���� ����
			sumofdatas += it_mac->Records.size();
		its = new COperRecord*[sumofdatas];
		for (deque<CMachine>::iterator it_mac = Optimized.Machines.begin(); it_mac != Optimized.Machines.end(); ++it_mac)	  // Records���� ���ۿ� �Ϸķ� ����
		{
			for(deque<COperRecord>::iterator it_rec = it_mac->Records.begin(); it_rec != it_mac->Records.end(); ++it_rec)
			{
				its[index_its++] = &(*it_rec);
			}
		}
		maxindex_its = index_its - 1;
		qsort((void*)its, (size_t)index_its, sizeof(COperRecord*), CompareOperRecordPointer);	  // Records�� �ð� ������ ����
		StartTime = its[0]->when;
		itPointer = 0;
		tiPointer = StartTime;
		for (int i = 0; i < index_its; ++i)		   // IGNORE��Ʈ�� �ִ� Record�� isGlobalFit�� true�� �ٲ���
		{
			cortemp = &(*its[i]);
			cortemp->isGlobalFit = false;
			temp = cortemp->when.HowMinute();
			temp2 = temp + cortemp->howlong;
			for (int j = 0; j < IGNORENUM; ++j)
			{
				if (ignorefirst[j] <= temp && temp < ignorelast[j]){
					cortemp->isGlobalFit = true;
					++count_optimized;
					break;
				}
			}
		}
		while (1)
		{
		l1:
			temp = count_optimized * 100 / index_its;
			if (lastprint_count_optimized != temp)
			{
				sprintf_s(tempstring,128, "title g%d", temp);
				system(tempstring);
				lastprint_count_optimized = temp;
			}
			for (int i = 0; i < IGNORENUM; ++i)						// ti�����Ͱ� IGNORE��Ʈ�� ������ IGNORE��Ʈ �ڷ� ��
			{
				if (tiPointer.Hour >= ignorefirst[i] && tiPointer.Hour < ignorelast[i])		 // IGNORE��Ʈ�� �ִ��� Ȯ��
				{
					tiPointer.Hour = ignorelast[i];
					while (CompareTime(tiPointer, (*its[itPointer]).when) == 1)	   // itPointer�� tiPointer�� ����Ű�� �ִ� �ð��� ū ������ ���� ����� �ð�������
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
			while (1)				   // ti������ ���������� ���� ����� RecordŽ��
			{
				if (temp >= index_its)
				{
					intervalright = 99999; break;
				}
				if (!(its[temp]->isGlobalFit))	   // isGlobalFIt�� false���� �˻�
				{
					timetemp = its[temp]->when - tiPointer;		  // ti�����Ϳ� �ʹ� �Ÿ��� ���� �˻�
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
							intervalright = 99999; break;	   // 99999�� �س��Ƽ� ����� �����ʿ� �ִ� Record�� �������� ���ϰ� ��
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
			while (1)			  // it������ �������� ���� ����� RecordŽ��
			{
				if (temp < 0)
				{
					intervalleft = 99999; break;
				}
				if (!(its[temp]->isGlobalFit))	   // isGlobalFIt�� false���� �˻�
				{
					timetemp = tiPointer - its[temp]->when;
					timetemp.CheckRangeZeroOK(MINUTE);
					if (!timetemp.isOverADay())						 // ti�����Ϳ� �ʹ� �Ÿ��� ���� �˻�
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
			if (intervalleft == 99999 && intervalright == 99999)	 // �������ε� ���������� �Űܿø��� ������ Record�� ���� ���
			{
				tiPointer.Minute += 10;				// tiPointer�� 10�� ���������� �ű�
				tiPointer.CheckRange(MINUTE);
				timetemp = its[maxindex_its]->when - tiPointer;
				timetemp.CheckRangeZeroOK(MINUTE);
				if (timetemp.Year < 0)								// tiPointer�� Records�� when�� �ִ밪�� �Ѿ�����
				{
					for (int i = 0; i < index_its; ++i)		   // isGlobalFit�� false�� Record�� ���� ���ʿ� �ִ°����� itPointer, tiPointer�� �ű�
					{
						if (!(its[i]->isGlobalFit))		// ��� Records�� isGlobalFit�� true�� ��� ����ȭ�� ����
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
			
			if (intervalright > intervalleft) // ������ ���� ����� isGlobalFit�� false�� Record �� �������� �ͺ��� tiPointer���� �Ÿ��� ª��. (���� ���� ������ �� ���� �� ����)
			{
				its[left]->when = tiPointer;		  // ������ ���� when�� tiPointer�� ���� ������ ��.
				its[left]->isGlobalFit = true;		   // ������ ���� isGlobalFit�� true�� ��.
				++count_optimized;
				tiPointer.Minute += its[left]->howlong;	// tiPointer�� ������ ���� howlong��ŭ ���������� �ű�
				tiPointer.CheckRange(MINUTE);
			}
			else		  // ������ ���� �� ����
			{
				its[right]->when = tiPointer;	 // �������� ���� when�� tiPointer�� ���� ������ ��.
				its[right]->isGlobalFit = true;		  // �������� ���� isGlobalFit�� true�� ��.
				++count_optimized;
				tiPointer.Minute += its[right]->howlong;  // tiPointer�� �������� ���� howlong��ŭ ���������� �ű�
				tiPointer.CheckRange(MINUTE);
			}
			while (CompareTime(tiPointer, (*its[itPointer]).when) == 1)	   // itPointer�� tiPointer�� ����Ű�� �ִ� �ð��� ū ������ ���� ����� �ð�������
			{
				++itPointer;
				if (itPointer >= index_its) // itPointer�� �ִ밪�� �Ѿ������ ���	  
				{
					for(int i=0; i < index_its; ++i)  // isGlobalFit�� false�� Record�� ���� ������ ���� ã�� ti, it�����Ϳ� �־���
					{
						if(!(its[i]->isGlobalFit))	 
						{
							itPointer = i;
							// cout << i << endl;
							tiPointer = its[i]->when;
							goto l1;
						}
					}
					delete[] its;	   // isGlobalFit�� ��� true�� ����ȭ�� ����.
					return;
				}
			}
		}
		delete[] its;
	}
	void GoBack(CStorage *comp)
	{
		deque<COperRecord>::iterator *pasts;		   //  ����ȭ ���� ���� Record���� pasts�� ����
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
		for (deque<CMachine>::iterator it_mac = Optimized.Machines.begin(); it_mac != Optimized.Machines.end(); ++it_mac)  	 // Optimized �� ��� Record���� Ȯ���ϸ鼭  goback���� �ȿ� ������ �������� ����
		{
			for (deque<COperRecord>::iterator it_rec = it_mac->Records.begin(); it_rec != it_mac->Records.end(); ++it_rec)
			{
				CTime *when = &(it_rec->when);
				int area = when->Hour * 60 + when->Minute;
				bool goback = false;
				for (int i = 0; i < BACKNUM; ++i)  //goback������ �ִ��� Ȯ��
				{
					if (gobackfirst[i] <= area && area <= gobacklast[i]) goback = true;
				}
				if (goback)		//goback������ ������
				{
					int recnum = it_rec->num;
					for (int i = 0; i < index_pasts; ++i)		// pasts�� �ִ� Record���� num�� ���� ��ϰ� ���� ������ �� ��.
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
	deque<CMachine>::iterator it_mac_global = Global.Optimized.Machines.begin();
	int housenum = 1;
	while (it_mac_random != RandomData.Machines.end()
		&& it_mac_local != Brutal.Optimized.Machines.end()
		&& it_mac_global != Global.Optimized.Machines.end())
	{
		it_mac_random->CalculateIntervals();
		it_mac_local->CalculateIntervals();
		it_mac_global->CalculateIntervals();
		deque<int>::iterator it_int_random = it_mac_random->Intervals.begin();
		deque<int>::iterator it_int_local = it_mac_local->Intervals.begin();
		deque<int>::iterator it_int_global = it_mac_global->Intervals.begin();
		while (it_int_random != it_mac_random->Intervals.end()
			&& it_int_local != it_mac_local->Intervals.end()
			&& it_int_global != it_mac_global->Intervals.end())
		{
			ofs << "House" << housenum << '\t';
			ofs << *it_int_random << endl << '\t';
			ofs << *it_int_local << endl << '\t';
			ofs << *it_int_global << endl;
			++it_int_random;
			++it_int_local;
			++it_int_global;
		}
		++it_mac_random;
		++it_mac_local;
		++it_mac_global;
		++housenum;
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
	TimeCost.ReadFromFile("Costdata.txt");	  // ������ �Է�
	OriginalData.ReadFromFile("Index.txt");
	OriginalData.RenewCostSum();
	cout << "The Cost from original records : " << OriginalData.costsum << endl;	
	ofs << "The Cost from original records : " << OriginalData.costsum << endl;

	OriginalData.WriteToFile("Output/Original/M%d.txt");

	Brutal.Original = &OriginalData;	 // Original�� Local Optimization
	Brutal.Optimization();
	Brutal.Evaluation();
	Brutal.Optimized.WriteToFile("Output/Brutal/M%d.txt");
	cout << "The Cost from Brutally Optimized Records : " << Brutal.Optimized.costsum << endl
		<< "The Time Score from optimized records : " << Brutal.ScoreTime << endl << endl;
	ofs << "The Cost from Brutally Optimized Records : " << Brutal.Optimized.costsum << endl
		<< "The Time Score from optimized records : " << Brutal.ScoreTime << endl << endl;

	Global.Original = &(Brutal.Optimized);	 // Original�� Global Optimization
	Global.Optimization();
	Global.Evaluation();
	Global.Optimized.WriteToFile("Output/Global/M%d.txt");
	cout << "The Cost from Globally Optimized Records : " << Global.Optimized.costsum << endl
		<< "The Time Score from optimized records : " << Global.ScoreTime << endl << endl;
	ofs << "The Cost from Globally Optimized Records : " << Global.Optimized.costsum << endl
		<< "The Time Score from optimized records : " << Global.ScoreTime << endl << endl;

	Brutal.Optimized.CalculateCept();						   // Original CEPT ���
	Brutal.Optimized.PrintCeptGraph("Cept/OriginalBrutal.txt");

	Global.Optimized.CalculateCept();
	Global.Optimized.PrintCeptGraph("Cept/OriginalGlobal.txt");

	Brutal.Optimized.WriteCDFDataToFile("CDFData/Brutal.txt");	 // Original CDF�ڷ� ���																 
	Global.Optimized.WriteCDFDataToFile("CDFData/Global.txt");

	Brutal.TimeIntervalCheck(&OriginalData);		 //Original  Interval������ ���� ��� 
	Brutal.WriteAverageIntervalData("AverageIntervals/OriginalBrutal.txt");
	Brutal.WriteRawIntervalData("RawIntervals/OriginalBrutal.txt");
	Global.TimeIntervalCheck(&OriginalData);
	Global.WriteAverageIntervalData("AverageIntervals/OriginalGlobal.txt");
	Global.WriteRawIntervalData("RawIntervals/OriginalGlobal.txt");

	Brutal.SetCosts();						  // Original �ð��뺰 ���⼼��� ���
	Brutal.WriteAverageCost("Costsbytime/OriginalBrutal.txt");
	Global.SetCosts();
	Global.WriteAverageCost("Costsbytime/OriginalGlobal.txt");

	Brutal.Clear();
	Global.Clear();

	RandomData.NewRandomData(1000);	// ���� ������ ����
	RandomData.WriteToFile(RANDOM_ORIGIN);
	RandomData.RenewCostSum();
	cout << "The Cost from Random original records : " << RandomData.costsum << endl;
	ofs << "The Cost from Random original records : " << RandomData.costsum << endl;

	Brutal.Original = &RandomData;		// Random�� Brutal Optimziation
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

	Global.Original = &(Brutal.Optimized);		// Random�� Global Optimization
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

	PrintCostSums("costsums.txt"); // ������������ �� ������ ���⼼���� ��ȭ���

	Brutal.Optimized.CalculateCept();						   // CEPT ���
	Brutal.Optimized.PrintCeptGraph("Cept/Brutal.txt");

	OriginalData.CalculateCept();
	OriginalData.PrintCeptGraph("Cept/Original.txt");

	RandomData.CalculateCept();
	RandomData.PrintCeptGraph("Cept/Random.txt");

	Global.Optimized.CalculateCept();
	Global.Optimized.PrintCeptGraph("Cept/Global.txt");

	RandomData.WriteCDFDataToFile("CDFData/Random.txt"); // CDF�ڷ� ���
	Brutal.Optimized.WriteCDFDataToFile("CDFData/Brutal.txt");
	Global.Optimized.WriteCDFDataToFile("CDFData/Global.txt");

	Brutal.TimeIntervalCheck(&RandomData);		 // Interval������ ���� ��� 
	Brutal.WriteAverageIntervalData("AverageIntervals/Brutal.txt");
	Brutal.WriteRawIntervalData("RawIntervals/Brutal.txt");
	Global.TimeIntervalCheck(&RandomData);
	Global.WriteAverageIntervalData("AverageIntervals/Global.txt");
	Global.WriteRawIntervalData("RawIntervals/Global.txt");

	Brutal.SetCosts();						  // �ð��뺰 ���⼼��� ���
	Brutal.WriteAverageCost("Costsbytime/Brutal.txt");
	Global.SetCosts();
	Global.WriteAverageCost("Costsbytime/Global.txt");

	RandomData.CalculateIntervals();	  // ��ϵ� ���� Interval ���
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
