# Mycron

Домашнее задание номер 1

Описание:
  1) Программа работает на приоритетной очереди - задания из файла mycrontab сортируются по времени, в которое они должны быть запущены, и добавляются в очередь заданий.
  2) При подходе времени выполнения очередного задания, оно изымается из очереди, и создаётся процесс, который выполнит задание. Если задание должно исполняться каждую минуту/каждый час, оно снова будет добавлено в очередь.
  3) При изменении файла очередь удаляется и создаётся новая
  
Ограничения программы:
  1) Если какое-то из заданий должно выполниться в короткое время после запуска программы, то mycrontab не должен содержать много заданий (какое-то время после старта программы создаётся очередь процессов и новые процессы не могут быть запущены - они будут запущены позже)
  2) В одну секунду не должно запускаться много процессов, иначе некоторые из них рискуют не успеть запуститься в своё время (запустятся позже)

Негодин Владислав, 724
