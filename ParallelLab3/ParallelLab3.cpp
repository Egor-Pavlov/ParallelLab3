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
			//ожидаем данные для работы
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
			Sleep(1);
			results.push_back(make_pair(t, p));
			if (!work)
				break;

			Ready[thr->id] = true;
			
		}
		Ready[thr->id] = false;
		if (results.size() > 0)
		{
			//говорим что хотим печатать
			IneedPrint[thr->id] = true;

			//ждем очередь на доcтуп к файлу
			while (IdToPrint != thr->id);
			//вывод в файл
			string s;
			//такой вывод чтобы дописывать в файл, а не писать поверх как через fstream
			FILE* F = fopen(resultName.c_str(), "a");
			for (int i = 0; i < results.size(); i++)
			{
				if(results[i].second)
					s = to_string(results[i].first) + " - простое\n";
				else
					s = to_string(results[i].first) + " - не простое\n";

				fputs(s.c_str(), F);
			}
			results.clear();
			fclose(F);

			//передаем очередь на печать по кругу
			if(thr->id == countOfThr - 1)
				IdToPrint = 0;

			else IdToPrint = thr->id + 1;
		}
		//сброс флага желания печатать
		IneedPrint[thr->id] = false;
	}
	return NULL;
}

int main()
{
	setlocale(LC_ALL, "russian");
	
	string fname = "test.txt";
	IdToPrint = 0;//потоки будут печататься по очереди
	vector<double>times;
	
	int maxcount;
	cout<<"Ведите максимальное количество потоков для исследования"<< endl;
	cin >> countOfThr;
	
	cout<<"Ведите количество результатов работы накапливаемых в 1 потоке"<< endl;
	cin >> maxSize;
	
	cout<<"Ведите имя файла c входными данными"<< endl;
	cin >> fname;

	cout<<"Ведите имя файла для выходных данных"<< endl;
	cin >> resultName;


	//выделяем память под массив идентификаторов потоков
	pthread_t* threads = (pthread_t*)malloc(countOfThr * sizeof(pthread_t));
	//выделяем память под массив структур для потоков
	
	Thread* ThrArgs = (Thread*)malloc(countOfThr * sizeof(Thread));
	
	work = true;
	//создаем и запускаем потоки
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
	//читаем сразу все данные
	vector<int> nums;
	if (in.is_open())
	{
		while (getline(in, line))
			nums.push_back(stoi(line));
	}
	in.close();

	//начало отсчета времени вычисления
	auto startTime = std::chrono::high_resolution_clock::now();

	while (nums.size() > 0)
	{
		for (int j = 0; j < countOfThr; j++)
		{//ищем первый свободный поток
			if (Ready[j] == true)
			{
				//записываем в его аргумент новое значение
				ThrArgs[j].num = nums.back();
				Ready[j] = false;
				nums.pop_back();
				break;
			}
		}
	}

	work = false;
	
	//ожидаем окончание выполнения всех потоков
	for (int i = 0; i < countOfThr; i++)
		pthread_join(threads[i], NULL);

	//окончание отсчета времени
	auto endTime = std::chrono::high_resolution_clock::now() - startTime;
	//подсчет времени вычисления
	double elapseTime = std::chrono::duration<double>(endTime).count();
	times.push_back(elapseTime);
	cout << elapseTime<<endl;

  	return 0;
}

