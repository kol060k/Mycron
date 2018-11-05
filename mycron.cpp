#include <iostream>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <vector>
#include <string>
#include <cstring>
#include <queue>
#include <ctime>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

using namespace std;

struct time_info {			// Структура информации о времени запуска задачи
        time_t Start_Time;
        int Repeat_hour;
        int Repeat_min;
};

time_info Calculate_Start_Time(const time_info T0) {
// Функция подбирает задаче ближайшее подходящее время, в которое она может быть исполнена
	time_info T;
	T.Repeat_hour = T0.Repeat_hour;
	T.Repeat_min = T0.Repeat_min;
	int hh = T0.Start_Time / 3600;
	int mm = T0.Start_Time % 3600 / 60;
	int ss = T0.Start_Time % 60;
	time_t seconds = time(NULL);
	tm* timeinfo = localtime(&seconds);
	int local_time = (int) seconds;
	int Timezone = timeinfo->tm_hour - local_time % 86400 / 3600;	// Поправка на часовой пояс

	if (T.Repeat_hour == 0)
		hh -= Timezone;
	if ((T.Repeat_hour == 0) && (T.Repeat_min == 0)) {
		int t = ss + mm * 60 + hh * 3600;
		if (t <=  local_time % 86400)
			T.Start_Time = (time_t) (local_time - local_time % 86400 + 86400 + t);
		else 
			T.Start_Time = (time_t) (local_time - local_time % 86400 + t);
	}

	if ((T.Repeat_hour == 1) && (T.Repeat_min == 0)) {
		int t = ss + mm * 60;
		if (t <= local_time % 3600)
			T.Start_Time = (time_t) (local_time - local_time % 3600 + 3600 + t);
		else
			T.Start_Time = (time_t) (local_time - local_time % 3600 + t);
	}

	if ((T.Repeat_hour == 1) && (T.Repeat_min == 1)) {
		if (ss <= local_time % 60)
			T.Start_Time = (time_t) (local_time - local_time % 60 + 60 + ss);
		else
			T.Start_Time = (time_t) (local_time - local_time % 60 + ss);
	}

	if ((T.Repeat_hour == 0) && (T.Repeat_min == 1)) {
		if (hh > local_time % 86400 / 3600)
			T.Start_Time = (time_t) (local_time - local_time % 86400 + hh * 3600 + ss);
		else if (hh == local_time % 86400 / 3600) {
			if (ss > local_time % 60)
				T.Start_Time = (time_t) (local_time - local_time % 60 + ss);
			else if (local_time % 3600 / 60 == 59)
				T.Start_Time = (time_t) (local_time - local_time % 3600 + 86400 + ss);
			else
				T.Start_Time = (time_t) (local_time - local_time % 60 + 60 + ss);
		}
		else 
			T.Start_Time = (time_t) (local_time - local_time % 86400 + 86400 
					+ hh * 3600 + ss);
	}
	return T;
}


time_info strtotime(const string s) {	// Функция преобразования времени из строки в формат стркутуры time_info
	time_info T;
	string buffer;
	int indent;
	int hh = 0;
	int mm = 0;
	int ss = 0;

	if (s[0] == '*') {	// Пробегаемся по строке, содержащей время
		if (s[1] != ':')
			throw 1;
		T.Repeat_hour = 1;
		indent = 2;
	} 
	else {
		if ((s[0] < '0') || (s[0] > '2') || (s[1] < '0') || (s[1] > '9') ||
				((s[0] == '2') && (s[1] > '3')) || (s[2] != ':'))
			throw 1;
		T.Repeat_hour = 0;
		buffer += s[0];
		buffer += s[1];
		hh = stoi(buffer);
		buffer.clear();
		indent = 3;
	}

	if (s[indent] == '*') {
		if (s[indent + 1] != ':')
			throw 1;
		T.Repeat_min = 1;
		indent += 2;
	}
	else {
		if ((s[indent] < '0') || (s[indent] > '5') || 
				(s[indent + 1] < '0') || (s[indent + 1] > '9') ||
				(s[indent + 2] != ':'))
			throw 1;
		T.Repeat_min = 0;
		buffer += s[indent];
		buffer += s[indent + 1];
		mm = stoi(buffer);
		buffer.clear();
		indent += 3;
	}

	if ((s[indent] < '0') || (s[indent] > '9') || (s[indent + 1] < '0') || s[indent + 1] > '9')
		throw 1;
	buffer += s[indent];
	buffer += s[indent + 1];
	ss = stoi(buffer);
	buffer.clear();

	// Находим время, в которое задача должна выполниться первый (возможно не единственный) раз
	T.Start_Time = hh * 3600 + mm * 60 + ss;
	T = Calculate_Start_Time(T);
	return T;
}


struct Task {				// Структура задания, которое будет добавляться в приоритетную очередь как целое
	time_info TInfo;
	vector<char*> command;
};

bool operator<(const Task& T1, const Task& T2) {	// Оператор сравнения заданий - выяснить, какое должно выполниться раньше
	return T1.TInfo.Start_Time > T2.TInfo.Start_Time;
}

int main() {
	struct stat st;
	time_t last_change = 0;
	priority_queue <Task, vector<Task>, less<Task>> queue;
	// Основной цикл
	while (1) {
		// Если с файлом mycrontab какие-то проблемы - завершаем работу
		if (stat("mycrontab", &st)) {
			cout << "Could not open file";
			break;
		}
		// Создание новой очереди в случае если файл mycrontab обновлён
		if (st.st_mtime > last_change) {
			last_change = st.st_mtime;
			while (!queue.empty())
				queue.pop();
			ifstream Tab;
			Tab.open("mycrontab");
			string str;
			string timestr;
			string word;
			while (Tab >> timestr) {		// Построчно считываем команды из файла
				getline(Tab, str);		// Строку команды раздеряем в слова и кладём в vector пословно
				istringstream iss(str);
				vector <char*> command;
				while (iss >> word) {
					command.push_back(strdup(word.c_str()));
				}
				time_info TInfo;
				try {
					TInfo = strtotime(timestr);	// Преобразуем время из файла в формат структуры time_info
				}
				catch (int err) {	// Выводим ошибку, если формат времени неверный
					cout << "Wrong time format of task!" << endl;
					continue;
				}
				Task T = {TInfo, command};
				queue.push(T);
			}
			Tab.close();
		}
		// Если все задания выполнены - завершение программы
		if (queue.empty())
			break;
		// Создание процессов для выполнения заданий
		time_t curtime = time(NULL);
		Task T = queue.top();
		if (curtime >= T.TInfo.Start_Time) {
			pid_t pid = fork();
			if (pid == 0) {
				execvp(T.command[0], &T.command[0]);
				cout << "Wrong command!" << endl;
				break;
			}
			if (pid > 0) {
				queue.pop();
				if (T.TInfo.Repeat_hour || T.TInfo.Repeat_min) {
					T.TInfo = Calculate_Start_Time(T.TInfo);
					queue.push(T);
				}
				wait(0);
			}
			if (pid < 0)
				perror("Unable to fork");
		}
	}
	return 0;
}
