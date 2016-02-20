#include <iostream>
#include <string>
#include <boost/asio.hpp>
#include "server.hpp"

std::string port;
std::string host;
std::string dir;

int main(int argc, char *argv[])
{
	int opt = 0;


	while ((opt = getopt(argc, argv, "h:p:d:")) != -1) {
		switch (opt) {
			case 'h':
				host = optarg;
				break;
			case 'p':
				port = optarg;
				break;
			case 'd':
				dir = optarg;
				break;
			default: /* '?' */
				fprintf(stderr, "Usage: %s [-t nsecs] [-n] name\n",
						argv[0]);
				exit(EXIT_FAILURE);
		}
	}

	// создаем потомка
			int pid = fork();

			if (pid == -1) // если не удалось запустить потомка
			{
				// выведем на экран ошибку и её описание
				printf("Error: Start Daemon failed (%s)\n", strerror(errno));

				return -1;
			}
			else if (!pid) // если это потомок
			{
				// данный код уже выполняется в процессе потомка
				// разрешаем выставлять все биты прав на создаваемые файлы,
				// иначе у нас могут быть проблемы с правами доступа
				umask(0);

				// создаём новый сеанс, чтобы не зависеть от родителя
				setsid();

				try
				  {
					// Initialise the server.
					http::server::server s(host, port, dir);
					// Run the server until stopped.
					s.run();
				  }
				  catch (std::exception& e)
				  {
					std::cerr << "exception: " << e.what() << "\n";
				  }

				return 0;
			}
			else // если это родитель
			{
				// завершим процес, т.к. основную свою задачу (запуск демона) мы выполнили
				return 0;
			}


		return 0;
}
