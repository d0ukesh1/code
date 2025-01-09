#include <stdio.h>
#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <string.h>
#ifndef PORT
# define PORT 8889
#endif


int winner;
int tank1y[9], tank1x[9], tank1[9], tank2y[9], tank2x[9], tank2[9]; 
int gun1y, gun1x, gun2y, gun2x;
int bullets1, bullets2;
int bullets1y[4], bullets1x[4], bulletsst1[4], bullets2y[4], bullets2x[4], bulletsst2[4];
int bullets_move1y[4], bullets_move1x[4], bullets_move2y[4], bullets_move2x[4];
int game_life;
char *ip_1, *ip_2;
pthread_t thread_bullet;
WINDOW *win;



void *bull_move(void *args)
{
	int *num = (int*)args;

	int number = -1;
	if (*num == 1)
	{
		for (int i = 0; i < 4; i++)
		{
			if (bulletsst1[i] == 0)
				number = i;
		}

		if (number == -1)
			pthread_exit(0);
	
		bulletsst1[number] = 1;
		bullets1y[number] = gun1y; bullets_move1y[number] = gun1y - tank1y[4]; 
		bullets1x[number] = gun1x; bullets_move1x[number] = gun1x - tank1x[4];

		while ((bullets1y[number] > 2) && (bullets1y[number] < 37) && (bullets1x[number] > 2) && (bullets1x[number] < 97) && (game_life != 0))
		{
			bullets1y[number] += bullets_move1y[number];
			bullets1x[number] += bullets_move1x[number];

			for (int i = 0; i < 9; i++)
			{
				if ((bullets1y[number] == tank2y[i]) && (bullets1x[number] == tank2x[i]))
				{
					game_life = 0;
					winner = *num;
					pthread_exit(0);
				}
			}

			usleep(50000);
		}
		
		bulletsst1[number] = 0;
		pthread_exit(0); 
	}
	
	if (*num == 2)
	{
		for (int i = 0; i < 4; i++)
		{
			if (bulletsst2[i] == 0)
				number = i;
		}

		if (number == -1)
			pthread_exit(0);
	
		bulletsst2[number] = 1;
		bullets2y[number] = gun2y; bullets_move2y[number] = gun2y - tank2y[4]; 
		bullets2x[number] = gun2x; bullets_move2x[number] = gun2x - tank2x[4];

		while ((bullets2y[number] > 2) && (bullets2y[number] < 37) && (bullets2x[number] > 2) && (bullets2x[number] < 97) && (game_life != 0))
		{
			bullets2y[number] += bullets_move2y[number];
			bullets2x[number] += bullets_move2x[number];

			for (int i = 0; i < 9; i++)
			{
				if ((bullets2y[number] == tank1y[i]) && (bullets2x[number] == tank1x[i]))
				{
					game_life = 0;
					winner = *num;
					pthread_exit(0);
				}
			}

			usleep(50000);
		}
		
		bulletsst2[number] = 0;
		pthread_exit(0); 
		
	}
}



int tank_shoot(int num)
{
	if (num == 1)
		pthread_create(&thread_bullet, NULL, bull_move, (void*)&num);	
	if (num == 2)
		pthread_create(&thread_bullet, NULL, bull_move, (void*)&num);	
}



void *Print_Screen()
{
	while(game_life != 0)
	{
		werase(win);

		for (int i = 0; i < 9; i++)
		{
			mvwaddch(win, tank1y[i], tank1x[i], tank1[i] | COLOR_PAIR(1));
			mvwaddch(win, tank2y[i], tank2x[i], tank2[i] | COLOR_PAIR(2));
			mvwaddch(win, gun1y, gun1x, '+' | COLOR_PAIR(1));
			mvwaddch(win, gun2y, gun2x, '+' | COLOR_PAIR(2));
		}

		for (int i = 0; i < 4; i++)
		{
			if (bulletsst1[i] == 1)
			{
				mvwaddch(win, bullets1y[i], bullets1x[i], '*' | COLOR_PAIR(1));
			}
			if (bulletsst2[i] == 1)
			{
				mvwaddch(win, bullets2y[i], bullets2x[i], '*' | COLOR_PAIR(2));
			}
		}

		box(win, 0, 0);
		wrefresh(win);
		usleep(1000000 / 120);
	}
	pthread_exit(0);
}



int collision(int dir, int num)
{
	int directiony, directionx;
	switch (dir)
	{
		case 1: // up
			directiony = -1;
			directionx = 0;
			break;
		case 2: // down
			directiony = 1;
			directionx = 0;
			break;
		case 3: // left
			directiony = 0;
			directionx = -1;
			break;
		case 4: // right
			directiony = 0;
			directionx = 1;
			break;
	}

	if (num == 1)
	{
		for (int i = -2; i < 3; i++)
		{
			for (int j = -2; j < 3; j++)
			{
				if ((tank1y[4] + 1 * directiony == tank2y[4] + i) && (tank1x[4] + 1 * directionx == tank2x[4] + j))
					return 0;
			}
		}
	}
	if (num == 2)
	{
		for (int i = -2; i < 3; i++)
		{
			for (int j = -2; j < 3; j++)
			{
				if ((tank1y[4] + 1 * directiony == tank2y[4] + i) && (tank1x[4] + 1 * directionx == tank2x[4] + j))
					return 0;
			}
		}
	}
}



void tank_move(int num, int key)
{
	int tanky[9], tankx[9], tank[9];
	int guny, gunx;
	int num_sym;
	if (num == 1)
	{
		num_sym = 2;
		for (int i = 0; i < 9; i++)
		{
			tanky[i] = tank1y[i];
			tankx[i] = tank1x[i];
			tank[i] = tank1[i];
			guny = gun1y;
			gunx = gun1x;
		}
	}
	if (num == 2)
	{
		num_sym = 1;
		for (int i = 0; i < 9; i++)
		{
			tanky[i] = tank2y[i];
			tankx[i] = tank2x[i];
			tank[i] = tank2[i];
			guny = gun2y;
			gunx = gun2x;
		}
	}

	switch (key)
	{
		case KEY_UP:
			if ((tanky[4] > 2) && collision(1, num))
			{
				for (int i = 0; i < 9; i++)
					tanky[i]--;
				guny--;
			}
			if ((tank[3] == ' ') && (tank[5] == ' '))
			{
				tank[3] = '0';
				tank[5] = '0';
				tank[1] = ' ';
				tank[7] = ' ';
			}
			break;
		case KEY_DOWN:
			if ((tanky[4] < 37) && collision(2, num))
			{
				for (int i = 0; i < 9; i++)
					tanky[i]++;
				guny++;
			}
			if ((tank[3] == ' ') && (tank[5] == ' '))
			{
				tank[3] = '0';
				tank[5] = '0';
				tank[1] = ' ';
				tank[7] = ' ';
			}
			break;
		case KEY_LEFT:
			if ((tankx[4] > 2) && collision(3, num))
			{
				for (int i = 0; i < 9; i++)
					tankx[i]--;
				gunx--;
			}
			if ((tank[3] == '0') && (tank[5] == '0'))
			{
				tank[3] = ' ';
				tank[5] = ' ';
				tank[1] = '0';
				tank[7] = '0';
			}
			break;
		case KEY_RIGHT:
			if ((tankx[4] < 97) && collision(4, num))
			{
				for (int i = 0; i < 9; i++)
					tankx[i]++;
				gunx++;
			}
			if ((tank[3] == '0') && (tank[5] == '0'))
			{
				tank[3] = ' ';
				tank[5] = ' ';
				tank[1] = '0';
				tank[7] = '0';
			}
			break;
		case 'q':
			guny = tanky[0];
			gunx = tankx[0];
			break;
		case 'w':
			guny = tanky[1];
			gunx = tankx[1];
			break;
		case 'e':
			guny = tanky[2];
			gunx = tankx[2];
			break;
		case 'a':
			guny = tanky[3];
			gunx = tankx[3];
			break;
		case 'd':
			guny = tanky[5];
			gunx = tankx[5];
			break;
		case 'z':
			guny = tanky[6];
			gunx = tankx[6];
			break;
		case 'x':
			guny = tanky[7];
			gunx = tankx[7];
			break;
		case 'c':
			guny = tanky[8];
			gunx = tankx[8];
			break;
		case ' ':
			tank_shoot(num);
			break;
	}
	
	if (num == 1)
	{
		for (int i = 0; i < 9; i++)
		{
			tank1y[i] = tanky[i];
			tank1x[i] = tankx[i];
			tank1[i] = tank[i];
			gun1y = guny;
			gun1x = gunx;
		}
	}
	if (num == 2)
	{
		for (int i = 0; i < 9; i++)
		{
			tank2y[i] = tanky[i];
			tank2x[i] = tankx[i];
			tank2[i] = tank[i];
			gun2y = guny;
			gun2x = gunx;
		}
	}
}



void* udp_server()
{

	int sock, addr_len, count, ret;
	int key;
	fd_set readfd;
	struct sockaddr_in addr;

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if ( 0 > sock ) {
		perror("socket");
		return NULL;
	}

	addr_len = sizeof(struct sockaddr_in);
	memset((void*)&addr, 0, addr_len);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htons(INADDR_ANY);
	addr.sin_port = htons(PORT);

	if ( bind(sock,(struct sockaddr*)&addr, addr_len) < 0) {
		perror("bind");
		close(sock);
		return NULL;
	}

	char* player_ip;
	while (game_life != 0) {
		FD_ZERO(&readfd);
		FD_SET(sock, &readfd);

		ret = select(sock+1, &readfd, NULL, NULL, 0);
		if (ret > 0) {
			if (FD_ISSET(sock, &readfd)) {
				recvfrom(sock, &key, sizeof(key), 0, (struct sockaddr*)&addr, &addr_len);
				player_ip = inet_ntoa(addr.sin_addr);
				if (strcmp(player_ip, ip_1) == 0)
				{
					tank_move(1, key);
				}
				if (strcmp(player_ip, ip_2) == 0)
				{
					tank_move(2, key);
				}
			}
		}
	}

	close(sock);
	return (void*)NULL;
}



int main(int argc, char **argv)
{
	ip_1 = argv[1];
	ip_2 = argv[2];
	game_life = 1;

	tank1y[0] = 10; tank2y[0] = 10;
	tank1y[1] = 10; tank2y[1] = 10;
	tank1y[2] = 10; tank2y[2] = 10;
	tank1y[3] = 11; tank2y[3] = 11;
	tank1y[4] = 11; tank2y[4] = 11;
	tank1y[5] = 11; tank2y[5] = 11;
	tank1y[6] = 12; tank2y[6] = 12;
	tank1y[7] = 12; tank2y[7] = 12;
	tank1y[8] = 12; tank2y[8] = 12;

	tank1x[0] = 10; tank2x[0] = 20;
	tank1x[1] = 11; tank2x[1] = 21;
	tank1x[2] = 12; tank2x[2] = 22;
	tank1x[3] = 10; tank2x[3] = 20;
	tank1x[4] = 11; tank2x[4] = 21;
	tank1x[5] = 12; tank2x[5] = 22;
	tank1x[6] = 10; tank2x[6] = 20;
	tank1x[7] = 11; tank2x[7] = 21;
	tank1x[8] = 12; tank2x[8] = 22;

	tank1[0] = '0'; tank2[0] = '0';
	tank1[1] = ' '; tank2[1] = ' ';
	tank1[2] = '0'; tank2[2] = '0';
	tank1[3] = '0'; tank2[3] = '0';
	tank1[4] = '0'; tank2[4] = '0';
	tank1[5] = '0'; tank2[5] = '0';
	tank1[6] = '0'; tank2[6] = '0';
	tank1[7] = ' '; tank2[7] = ' ';
	tank1[8] = '0'; tank2[8] = '0';

	gun1y = 10; gun2y = 10;
	gun1x = 11; gun2x = 21;

	bullets1 = 4; bullets2 = 4;

	for (int i = 0; i < 4; i++)
	{
		bulletsst1[i] = 0;
		bulletsst2[i] = 0;
	}

	initscr();
	timeout(1*100);
	noecho();
	keypad(stdscr, TRUE);
	curs_set(0);
	start_color();
	init_pair(1, COLOR_RED, COLOR_BLACK);
	init_pair(2, COLOR_BLUE, COLOR_BLACK);
	int row = 40, col = 100;
	int heightGap = (LINES - row) / 2;
	int weightGap = (COLS - col) / 2;
	win = newwin(row, col, heightGap, weightGap);
	refresh();
	box(win, 0, 0);
	wrefresh(win);

	pthread_t screen;
	pthread_create(&screen, NULL, Print_Screen, NULL);

	int sock, yes = 1;
	pthread_t pid;
	struct sockaddr_in addr;
	int addr_len;
	int count;
	int ret;
	fd_set readfd;
	int i;

	pthread_create(&pid, NULL, udp_server, NULL);

	sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (sock < 0) {
		game_life = 0;
		perror("sock");
		pthread_join(pid, NULL);
		pthread_join(screen, NULL);
		return -1;
	}

	ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, (char*)&yes, sizeof(yes));
	if (ret == -1) {
		game_life = 0;
		perror("setsockopt");
		pthread_join(pid, NULL);
		pthread_join(screen, NULL);
		return 0;
	}

	addr_len = sizeof(struct sockaddr_in);
	memset((void*)&addr, 0, addr_len);
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_BROADCAST);
	addr.sin_port = htons(PORT);
	
	int key = 0;
	while ((key != '0') && (game_life != 0))
	{
		key = getch();

		if (ERR != key)
			sendto(sock, &key, sizeof(key), 0, (struct sockaddr*) &addr, addr_len);
	}
	
	game_life = 0;
	pthread_join(screen, NULL);
	endwin();	
	printf("Player %d WIN!!!\n", winner);
	return 0;
}