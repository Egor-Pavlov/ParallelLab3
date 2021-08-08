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
int countOfThr = 1;
int IdToPrint;
int maxSize = 5;
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
	int t;
	bool p;
	while (work)
	{
		while (results.size() < maxSize)
		{
			//������� ������ ��� ������
			while (thr->num == 0)
			{
				thr = (Thread*)thrArg;
			}

			Ready[thr->id] = false; //���� ���������� ������ � ������ ������
			
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
			//Ready[thr->id] = true;
			thr->num = 0;
		}
		//������� ��� ����� ��������
		IneedPrint[thr->id] = true;

		//���� ������� �� ��c��� � �����
		while (IdToPrint != thr->id);

		cout << "id " << thr->id << "print"<<endl;
		//����� � ����
		string s;
		//����� ����� ����� ���������� � ����, � �� ������ ������ ��� ����� fstream
		FILE* F = fopen(resultName.c_str(), "a");
		for (int i = 0; i < maxSize; i++)
		{
			s = to_string(results[i].first) + " " + to_string(results[i].second) + "\n";
			fputs(s.c_str(), F);
		}
		results.clear();
		fclose(F);

		//�������� ������� �� ������ ����, ��� ������ ������ � ��� ����� ��������
		for (size_t i = thr->id + 1; i < countOfThr; i++)
		{
			if (IneedPrint[i])
			{
				IdToPrint = i;
				break;
			}
		}
		//���� ����� �� ����� �������� �� �������� 0-�� ������
		if (IdToPrint == thr->id)
			IdToPrint = 0;

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
	if (in.is_open())
	{
		while (getline(in, line))
		{
			flag = true;
			while (flag)
			{
				for (int i = 0; i < countOfThr; i++)
				{
					if (Ready[i] == true)
					{
						ThrArgs[i].num = stoi(line);
						flag = false;
						break;
					}
				}
				
			}
			Sleep(1);//�� ����� ����� ������ ��� ������� ���� �� 1 �� ������ ����� ������ ��� ������, ���� �� ������ ��� ������ �� ���� �� ��������
		}
	}
	in.close();
	 
	work = false;
	//������� ���������� ���� �������
	for (int i = 0; i < countOfThr; i++)
		pthread_join(threads[i], NULL);
  	return 0;
}

