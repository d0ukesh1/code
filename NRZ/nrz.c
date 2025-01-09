#include <ncurses.h>
#include <stdio.h>
#include <string.h> 
// argv 0 = исполняемый файл
// argv 1 = кодирование/декодирование ("-e" / "-d")
// argv 2 = название файла для кодирования/декодирования
// argv 3 = название файла для сохранения кодированного/декодированного файла
int main(int argc, char* argv[]){FILE *f_1, *f_2;
if((f_1 = fopen(argv[2], "r+")) == NULL){
printf("Not such file in this directory");
return 0;
	}
f_2 = fopen(argv[3], "w"); 
initscr();
for(int i = 0; i < COLS-1; i++){
	mvaddch(2, i, '_');
	mvaddch(4, i, '_');
	mvaddch(10, i, '_');
	}
mvaddstr(1, COLS/2-11 , "RZ code visualization");
refresh;
curs_set(0);
noecho();
keypad(stdscr, TRUE);
int j, c, i = 1;
	if(strcmp(argv[1], "-e") == 0){
//кодирование
	while((c = fgetc(f_1)) != EOF){ 33.		if(c == 48){
	mvaddch(3, i+2, '1');
	mvaddch(5, i+1, '_');
	mvaddstr(6, i, "| |");
	mvaddstr(7, i, "| | ");
	fputc('1', f_2);
	fputc(',', f_2);
	}
	if(c == 49){
	mvaddstr(3, i+2, "-1");
	mvaddstr(7, i+3, " ");	mvaddstr(8, i, "| |");

	mvaddstr(9, i, "|_|");
	fputc('-', f_2);
	fputc('1', f_2);
	fputc(',', f_2);
	}
	for(j = 0; j < 8; j++)
	mvaddch(3+j, i-1, '|');
	i=i+6;
	refresh;
}	
}
if(strcmp(argv[1], "-d") == 0){
//декодирование
	while(1){
	c = fgetc(f_1);
	if(c == EOF){
	if( feof(f_1) != 0)
	break;
	else
	break;
	}
	if(c == 49){
	mvaddstr(7, i, "	");
	fseek(f_1, 1, SEEK_CUR);
	fputc('0', f_2);
	mvaddch(3, i+2, '0');
	
	}
	if(c == '-'){
	mvaddch(3, i+2, '1');
	mvaddch(5, i+1, '_');
	mvaddstr(6, i, "| |");
	mvaddstr(7, i, "| | ");
	fseek(f_1, 2, SEEK_CUR);
	fputc('1', f_2);
	mvaddch(3, i+2, '1');
	}
	for(j = 0; j < 8; j++)
	mvaddch(3+j, i-1, '|');
	i=i+6;
	refresh;
	}
	}
	getch();
	endwin();
	fclose(f_1);
	fclose(f_2);
	return 0;
}	
