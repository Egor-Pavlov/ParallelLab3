#include <iostream>
#include <Windows.h>
#include <string>
#include <pthread.h>
#include <cstdio>
#include <fstream>
#include <vector>

#pragma warning(disable: 4996)

using namespace std;

vector <bool> Ready;
vector <bool> IneedPrint;
int countOfThr = 2;
int IdToPrint;
int maxSize = 3;
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
			while ((thr->num == t) && (work))
			{
				thr = (Thread*)thrArg;
				
			}
			if (!work)
			{
				break;
			}
			//Ready[thr->id] = false; //���� ���������� ������ � ������ ������
			
			t = thr->num;
			cout << t << " ";//��� ��������

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
			
			results.push_back(make_pair(t, p));
			Ready[thr->id] = true;
		}
		Ready[thr->id] = false;
		if (results.size() > 0)
		{
			
			//������� ��� ����� ��������
			IneedPrint[thr->id] = true;

			//���� ������� �� ��c��� � �����
			while (IdToPrint != thr->id);

			cout << "id " << thr->id << "print" << endl;
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
		//����� ����� ������� �������� � ��������� ����� ���������� � ������ ������
		IneedPrint[thr->id] = false;
		Ready[thr->id] = true;
	}
	return NULL;
}

int main()
{
	setlocale(LC_ALL, "russian");
	
	string fname = "test.txt";
	IdToPrint = 0;//������ ����� ���������� �� �������

	//�������� ������ ��� ������ ��������������� �������
	pthread_t* threads = (pthread_t*)malloc(countOfThr * sizeof(pthread_t));
	//�������� ������ ��� ������ �������� ��� �������
	
	Thread* ThrArgs = (Thread*)malloc(countOfThr * sizeof(Thread));

	//cout<<"������ ���������� �������"<< endl;
	//cin >> countOfThr;
	
	//cout<<"������ ��� ����� c �������� �������"<< endl;
	//cin >> fname;

	work = true;
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
	bool flag = true;

	vector<int> nums;
	if (in.is_open())
	{
		while (getline(in, line))
			nums.push_back(stoi(line));
	}
	in.close();
	 

	while(nums.size()>0)
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
		//Sleep(1);
	}

	work = false;

	//������� ���������� ���� �������
	for (int i = 0; i < countOfThr; i++)
		pthread_join(threads[i], NULL);
  	return 0;
}

