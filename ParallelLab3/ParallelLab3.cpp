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

//����� ��� ������
vector<int> nums;

//������ ������ ���������� � ������ �����.
//������ ������� ������������ ������������� ������
vector <bool> Ready;

int countOfThr = 1;

//id ������ ������� ����� ��������
int IdToPrint;

//������ ������� ������ ������
int maxSize = 1;

string resultName = "res.txt";
bool work = true;


pthread_mutex_t g_mutex;


//��������� ���������� ������
struct Thread
{
	int id;
	int index;
};

void* ThrFunc(void* thrArg)
{
	vector<pair<int, bool>> results;
	Thread* thr = (Thread*)thrArg;
	int t = -1, number;
	bool p;
	while (work)
	{
		while (results.size() < maxSize)
		{
			//������� ������ ��� ������
			while (thr->index == t)
			{
				//��������� � ����� ��������� ����������� �������� � ����� ������
				//���� ���� ����� �������, �� ���������� ����� ������ � ��� �� ������������
				Ready[thr->id] = true;
				Ready[thr->id] = false;
				//������ ���������� ������
				thr = (Thread*)thrArg;
				
				//��������� ���� �� ����� �� ������������, ��� ���� ��������� ���.
				//��� ��� ������ �������� ������ �����, �� ����� ������ ����� ����������
				if (thr->index != t || !work)
				{
					//������� �� ����� ������ �����
					break;
				}
			}
			//��� ��� ����� �� ����� ������ ����� ����� ��������� ��� ������� �� ��������� ������,
			//�� ���� ���������, �������� �� ��������� �������� ����� ����� � ���������� ���, ���� ��
			if (thr->index != t)
			{
				t = thr->index;
				number = nums[t];
				p = true;//��������� �������� ����� �� ��������

				//��������� ������� ��� ���
				for (int i = 2; i < sqrt(abs(number)); i++)
				{
					if (number % i == 0)
					{
						//�� �������, ��� ��� ������� �� i
						p = false;
					}
				}
				Sleep(1);//�������� ������ � ������� ���������� :D
				results.push_back(make_pair(number, p));
			}
			if (!work)//������� �� ����� ���������� �����������, ���� ������ ���� �����������
				break;			
		}
		//�������� ���������� � ����
		if (results.size() > 0)
		{
			pthread_mutex_lock(&g_mutex);

			//����� � ����
			string s;
			//����� ����� ����� ���������� � ����, � �� ������ ������ ��� ����� fstream
			FILE* F = fopen(resultName.c_str(), "a");
			for (int i = 0; i < results.size(); i++)
			{
				if(results[i].second)
					s = to_string(results[i].first) + " - �������. ����� � " + to_string(thr->id) + "\n";
				else
					s = to_string(results[i].first) + " - �� �������. ����� �  " + to_string(thr->id) + "\n";

				fputs(s.c_str(), F);
			}

			results.clear();
			fclose(F);
			pthread_mutex_unlock(&g_mutex);	

			//��� ����� ������� ���������� ������������� ������ ������ � ����� ����������
			//������������� ��� ���������
		}
	}
	return NULL;
}

int main()
{
	setlocale(LC_ALL, "russian");
	
	pthread_mutex_init(&g_mutex, NULL);

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
		
	//������� ���� � ������������, ���� �� ����������
	if (FILE* file = fopen(resultName.c_str(), "r")) 
	{
		fclose(file);
		system(("del " + resultName).c_str());
	}

	//�������� ������ ��� ������ ��������������� �������
	pthread_t* threads = (pthread_t*)malloc(countOfThr * sizeof(pthread_t));

	//�������� ������ ��� ������ �������� ��� �������
	Thread* ThrArgs = (Thread*)malloc(countOfThr * sizeof(Thread));
	
	work = true;

	//������� � ��������� ������
	for (int i = 0; i < countOfThr; i++)
	{
		ThrArgs[i].index = -1;
		ThrArgs[i].id = i;

		//��������� ������
		Ready.push_back(true);
		pthread_create(&(threads[i]), NULL, ThrFunc, &ThrArgs[i]);
	}
	
	ifstream in(fname);
	string line;

	//������ ��� ����� �� ����� � ����� ������
	if (in.is_open())
	{
		while (getline(in, line))
			nums.push_back(stoi(line));
	}
	in.close();

	//������ ������� ������� ����������
	auto startTime = std::chrono::high_resolution_clock::now();

	//������������ ������ �� �������
	//� ����� ���������� ������, ������ ������ �� ������� � �� ������ ����� ������
	//������������� ������ ������� ������ ���� ������ � �� ����� ��������� ������ 
	//� ���� ������ �� ������ �������
	for (int i = 0; i < nums.size(); ++i)
	{
		for (int j = 0; j < countOfThr; ++j)
		{//���� ������ ��������� �����
			if (Ready[j] == true)
			{
				//���������� � ��� �������� ����� ��������
				ThrArgs[j].index = i;
				Ready[j] = false;
				break;
			}
			//���� ���������� ������ �� ������� ���� �������� ���
			else if (j == countOfThr - 1)
				j = -1;
		}
	}

	//������� �������, ��� ���� ����������� ������
	work = false;
	
	//������� ��������� ���������� ���� �������
	for (int i = 0; i < countOfThr; i++)
		pthread_join(threads[i], NULL);

	//��������� ������� �������
	auto endTime = std::chrono::high_resolution_clock::now() - startTime;

	//������� ������� ����������
	double elapseTime = std::chrono::duration<double>(endTime).count();
	times.push_back(elapseTime);
	cout << "����� ������: " << elapseTime << endl;

	nums.clear();
  	return 0;
}