#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <vector>
#include <string>
#include <queue>

using namespace std;

struct Task {			// Структура задания, которое будет добавляться в приоритетную очередь как целое
	time_t Start_Time;
	vector<char*> command;
	int Repeat_hour;
	int Repeat_min;
};

bool operator<(const Task& T1, const Task& T2) {	// Оператор сравнения заданий - выяснить, какое должно выполниться раньше
	return T1.Start_Time < T2.Start_Time;
}

int main() {
	priority_queue <Task, vector<Task>, less<Task>> queue;
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
			command.push_back((char*)word.c_str());
			cout << word << endl;
		}
		time_t Start_Time = 0;		// Преобразуем считанное из файла время в time_t
		int Repeat_hour = 0;
		int Repeat_min = 0;
		struct Task T = {Start_Time, command, Repeat_hour, Repeat_min};
		queue.push(T);
	}
	Tab.close();
	return 0;
}
