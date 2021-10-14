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

//числа для работы
vector<int> nums;

//массив флагов готовности к приему задач.
//каждый элемент соответствет определенному потоку
vector <bool> Ready;

int countOfThr = 1;

//id потока который будет печатать
int IdToPrint;

//размер очереди внутри потока
int maxSize = 1;

string resultName = "res.txt";
bool work = true;


pthread_mutex_t g_mutex;


//структура агрументов потока
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
			//ожидаем данные для работы
			while (thr->index == t)
			{
				//открываем и сразу закрываем возможность передать в поток данные
				//если окно будет большим, то передастся много данных и они не обработаются
				Ready[thr->id] = true;
				Ready[thr->id] = false;
				//читаем полученные данные
				thr = (Thread*)thrArg;
				
				//проверяем есть ли смысл их обрабатывать, или надо подождать еще.
				//так как потоки получают номера задач, то число всегда будет уникальное
				if (thr->index != t || !work)
				{
					//выходим из цикла приема задач
					break;
				}
			}
			//так как выход из цикла приема задач может произойти при сигнале об окончании работы,
			//то надо проверить, является ли последнее принятое число новым и обработать его, если да
			if (thr->index != t)
			{
				t = thr->index;
				number = nums[t];
				p = true;//результат проверки числа на простоту

				//проверяем простое или нет
				for (int i = 2; i < sqrt(abs(number)); i++)
				{
					if (number % i == 0)
					{
						//не простое, так как делится на i
						p = false;
					}
				}
				Sleep(1);//имитация долгих и сложных вычислений :D
				results.push_back(make_pair(number, p));
			}
			if (!work)//выходим из цикла накопления результатов, если потоку пора завершаться
				break;			
		}
		//печатаем результаты в файл
		if (results.size() > 0)
		{
			pthread_mutex_lock(&g_mutex);

			//вывод в файл
			string s;
			//такой вывод чтобы дописывать в файл, а не писать поверх как через fstream
			FILE* F = fopen(resultName.c_str(), "a");
			for (int i = 0; i < results.size(); i++)
			{
				if(results[i].second)
					s = to_string(results[i].first) + " - простое. Поток № " + to_string(thr->id) + "\n";
				else
					s = to_string(results[i].first) + " - не простое. Поток №  " + to_string(thr->id) + "\n";

				fputs(s.c_str(), F);
			}

			results.clear();
			fclose(F);
			pthread_mutex_unlock(&g_mutex);	

			//при такой очереди невозможна одновременная запись данных в общую переменную
			//следовательно все безопасно
		}
	}
	return NULL;
}

int main()
{
	setlocale(LC_ALL, "russian");
	
	pthread_mutex_init(&g_mutex, NULL);

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
		
	//удаляем файл с результатами, если он существует
	if (FILE* file = fopen(resultName.c_str(), "r")) 
	{
		fclose(file);
		system(("del " + resultName).c_str());
	}

	//выделяем память под массив идентификаторов потоков
	pthread_t* threads = (pthread_t*)malloc(countOfThr * sizeof(pthread_t));

	//выделяем память под массив структур для потоков
	Thread* ThrArgs = (Thread*)malloc(countOfThr * sizeof(Thread));
	
	work = true;

	//создаем и запускаем потоки
	for (int i = 0; i < countOfThr; i++)
	{
		ThrArgs[i].index = -1;
		ThrArgs[i].id = i;

		//запускаем потоки
		Ready.push_back(true);
		pthread_create(&(threads[i]), NULL, ThrFunc, &ThrArgs[i]);
	}
	
	ifstream in(fname);
	string line;

	//читаем все числа из файла в общую память
	if (in.is_open())
	{
		while (getline(in, line))
			nums.push_back(stoi(line));
	}
	in.close();

	//начало отсчета времени вычисления
	auto startTime = std::chrono::high_resolution_clock::now();

	//распределяем задачи по потокам
	//в поток передается индекс, читают потоки по индексу и не меняют общий массив
	//следовательно каждый получит только свою задачу и не будет опасности записи 
	//в одну память из разных потоков
	for (int i = 0; i < nums.size(); ++i)
	{
		for (int j = 0; j < countOfThr; ++j)
		{//ищем первый свободный поток
			if (Ready[j] == true)
			{
				//записываем в его аргумент новое значение
				ThrArgs[j].index = i;
				Ready[j] = false;
				break;
			}
			//если свободного потока не нашлось надо поискать еще
			else if (j == countOfThr - 1)
				j = -1;
		}
	}

	//говорим потокам, что пора заканчивать работу
	work = false;
	
	//ожидаем окончание выполнения всех потоков
	for (int i = 0; i < countOfThr; i++)
		pthread_join(threads[i], NULL);

	//окончание отсчета времени
	auto endTime = std::chrono::high_resolution_clock::now() - startTime;

	//подсчет времени вычисления
	double elapseTime = std::chrono::duration<double>(endTime).count();
	times.push_back(elapseTime);
	cout << "Время работы: " << elapseTime << endl;

	nums.clear();
  	return 0;
}