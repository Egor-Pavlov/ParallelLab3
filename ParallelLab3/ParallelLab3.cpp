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
			//ожидаем данные для работы
			while (thr->num == 0)
			{
				thr = (Thread*)thrArg;
			}

			Ready[thr->id] = false; //флаг готовности потока к приему данных
			
			t = thr->num;
			cout << t << " ";//для проверки

			p = true;//результат проверки числа на простоту

			//проверяем простое или нет
			for (int i = 2; i < sqrt(abs(t)); i++)
			{
				if (t % i == 0)
				{
					//не простое, так как делится на i
					p = false;
				}
			}
			
			results.push_back(make_pair(t, p));
			//Ready[thr->id] = true;
			thr->num = 0;
		}
		//говорим что хотим печатать
		IneedPrint[thr->id] = true;

		//ждем очередь на доcтуп к файлу
		while (IdToPrint != thr->id);

		cout << "id " << thr->id << "print"<<endl;
		//вывод в файл
		string s;
		//такой вывод чтобы дописывать в файл, а не писать поверх как через fstream
		FILE* F = fopen(resultName.c_str(), "a");
		for (int i = 0; i < maxSize; i++)
		{
			s = to_string(results[i].first) + " " + to_string(results[i].second) + "\n";
			fputs(s.c_str(), F);
		}
		results.clear();
		fclose(F);

		//передаем очередь на печать тому, чей индекс больше и кто хочет печатать
		for (size_t i = thr->id + 1; i < countOfThr; i++)
		{
			if (IneedPrint[i])
			{
				IdToPrint = i;
				break;
			}
		}
		//если никто не хочет печатать то передаем 0-му потоку
		if (IdToPrint == thr->id)
			IdToPrint = 0;

		//сброс флага желания печатать и установка флага готовности к приему данных
		IneedPrint[thr->id] = false;
		Ready[thr->id] = true;
	}
	return NULL;
}

int main()
{
	setlocale(LC_ALL, "russian");
	
	string fname = "test.txt";
	IdToPrint = 0;//потоки будут печататься по очереди

	//выделяем память под массив идентификаторов потоков
	pthread_t* threads = (pthread_t*)malloc(countOfThr * sizeof(pthread_t));
	//выделяем память под массив структур для потоков
	
	Thread* ThrArgs = (Thread*)malloc(countOfThr * sizeof(Thread));

	//cout<<"Ведите количество потоков"<< endl;
	//cin >> countOfThr;
	
	//cout<<"Ведите имя файла c входными данными"<< endl;
	//cin >> fname;

	work = true;
	for (int i = 0; i < countOfThr; i++)
	{
		ThrArgs[i].num = 0;
		ThrArgs[i].id = i;
		//запускаем потоки
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
			Sleep(1);//от этого слипа вообще все зависит если он 1 то первый поток делает всю работу, если он меньше или убрать то ниче не работает
		}
	}
	in.close();
	 
	work = false;
	//ожидаем выполнение всех потоков
	for (int i = 0; i < countOfThr; i++)
		pthread_join(threads[i], NULL);
  	return 0;
}

