#include <iostream>
#include <string>
#include <pthread.h>
#include <cstdio>
#include <fstream>
#include <vector>
#include <windows.h>
#include <chrono>

#pragma warning(disable: 4996)

using namespace std;

vector <bool> Ready;
vector <bool> IneedPrint;
int countOfThr = 1;
int IdToPrint;
int maxSize = 1;
string resultName = "res.txt";
bool work = true;

struct Thread
{
	int id;
	int num;
};

void* ThrFunc(void* thrArg)
{
	vector<pair<int, bool>> results;
	Thread* thr = (Thread*)thrArg;
	int t = 0;
	bool p;
	while (work)
	{
		while (results.size() < maxSize)
		{
			//������� ������ ��� ������
			while (thr->num == t)
			{
				Ready[thr->id] = true;
				thr = (Thread*)thrArg;
				Ready[thr->id] = false;
				
				if (thr->num != t || !work)
				{
					break;
				}
			}
			Ready[thr->id] = false;
					
			t = thr->num;

			p = true;//��������� �������� ����� �� ��������

			//��������� ������� ��� ���
			for (int i = 2; i < sqrt(abs(t)); i++)
			{
				if (t % i == 0)
				{
					//�� �������, ��� ��� ������� �� i
					p = false;
				}
			}
			Sleep(1);
			results.push_back(make_pair(t, p));
			if (!work)
				break;

			Ready[thr->id] = true;
			
		}
		Ready[thr->id] = false;
		if (results.size() > 0)
		{
			//������� ��� ����� ��������
			IneedPrint[thr->id] = true;

			//���� ������� �� ��c��� � �����
			while (IdToPrint != thr->id);
			//����� � ����
			string s;
			//����� ����� ����� ���������� � ����, � �� ������ ������ ��� ����� fstream
			FILE* F = fopen(resultName.c_str(), "a");
			for (int i = 0; i < results.size(); i++)
			{
				if(results[i].second)
					s = to_string(results[i].first) + " - �������\n";
				else
					s = to_string(results[i].first) + " - �� �������\n";

				fputs(s.c_str(), F);
			}
			results.clear();
			fclose(F);

			//�������� ������� �� ������ �� �����
			if(thr->id == countOfThr - 1)
				IdToPrint = 0;

			else IdToPrint = thr->id + 1;
		}
		//����� ����� ������� ��������
		IneedPrint[thr->id] = false;
	}
	return NULL;
}

int main()
{
	setlocale(LC_ALL, "russian");
	
	string fname = "test.txt";
	IdToPrint = 0;//������ ����� ���������� �� �������
	vector<double>times;
	
	int maxcount;
	cout<<"������ ������������ ���������� ������� ��� ������������"<< endl;
	cin >> countOfThr;
	
	cout<<"������ ���������� ����������� ������ ������������� � 1 ������"<< endl;
	cin >> maxSize;
	
	cout<<"������ ��� ����� c �������� �������"<< endl;
	cin >> fname;

	cout<<"������ ��� ����� ��� �������� ������"<< endl;
	cin >> resultName;


	//�������� ������ ��� ������ ��������������� �������
	pthread_t* threads = (pthread_t*)malloc(countOfThr * sizeof(pthread_t));
	//�������� ������ ��� ������ �������� ��� �������
	
	Thread* ThrArgs = (Thread*)malloc(countOfThr * sizeof(Thread));
	
	work = true;
	//������� � ��������� ������
	for (int i = 0; i < countOfThr; i++)
	{
		ThrArgs[i].num = 0;
		ThrArgs[i].id = i;
		//��������� ������
		Ready.push_back(true);
		IneedPrint.push_back(false);
		pthread_create(&(threads[i]), NULL, ThrFunc, &ThrArgs[i]);
	}
	
	ifstream in(fname);
	string line;
	//������ ����� ��� ������
	vector<int> nums;
	if (in.is_open())
	{
		while (getline(in, line))
			nums.push_back(stoi(line));
	}
	in.close();

	//������ ������� ������� ����������
	auto startTime = std::chrono::high_resolution_clock::now();

	while (nums.size() > 0)
	{
		for (int j = 0; j < countOfThr; j++)
		{//���� ������ ��������� �����
			if (Ready[j] == true)
			{
				//���������� � ��� �������� ����� ��������
				ThrArgs[j].num = nums.back();
				Ready[j] = false;
				nums.pop_back();
				break;
			}
		}
	}

	work = false;
	
	//������� ��������� ���������� ���� �������
	for (int i = 0; i < countOfThr; i++)
		pthread_join(threads[i], NULL);

	//��������� ������� �������
	auto endTime = std::chrono::high_resolution_clock::now() - startTime;
	//������� ������� ����������
	double elapseTime = std::chrono::duration<double>(endTime).count();
	times.push_back(elapseTime);
	cout << elapseTime<<endl;

  	return 0;
}

