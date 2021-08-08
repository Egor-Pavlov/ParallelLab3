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
			//ожидаем данные дл€ работы
			while ((thr->num == t) && (work))
			{
				thr = (Thread*)thrArg;
				
			}
			if (!work)
			{
				break;
			}
			//Ready[thr->id] = false; //флаг готовности потока к приему данных
			
			t = thr->num;
			cout << t << " ";//дл€ проверки

			p = true;//результат проверки числа на простоту

			//провер€ем простое или нет
			for (int i = 2; i < sqrt(abs(t)); i++)
			{
				if (t % i == 0)
				{
					//не простое, так как делитс€ на i
					p = false;
				}
			}
			
			results.push_back(make_pair(t, p));
			Ready[thr->id] = true;
		}
		Ready[thr->id] = false;
		if (results.size() > 0)
		{
			
			//говорим что хотим печатать
			IneedPrint[thr->id] = true;

			//ждем очередь на доcтуп к файлу
			while (IdToPrint != thr->id);

			cout << "id " << thr->id << "print" << endl;
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
		//сброс флага желани€ печатать и установка флага готовности к приему данных
		IneedPrint[thr->id] = false;
		Ready[thr->id] = true;
	}
	return NULL;
}

int main()
{
	setlocale(LC_ALL, "russian");
	
	string fname = "test.txt";
	IdToPrint = 0;//потоки будут печататьс€ по очереди

	//выдел€ем пам€ть под массив идентификаторов потоков
	pthread_t* threads = (pthread_t*)malloc(countOfThr * sizeof(pthread_t));
	//выдел€ем пам€ть под массив структур дл€ потоков
	
	Thread* ThrArgs = (Thread*)malloc(countOfThr * sizeof(Thread));

	//cout<<"¬едите количество потоков"<< endl;
	//cin >> countOfThr;
	
	//cout<<"¬едите им€ файла c входными данными"<< endl;
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
		//Sleep(1);
	}

	work = false;

	//ожидаем выполнение всех потоков
	for (int i = 0; i < countOfThr; i++)
		pthread_join(threads[i], NULL);
  	return 0;
}

