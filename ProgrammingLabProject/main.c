#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#include <conio.h>

// ���� ����
#define BLACK	0
#define BLUE1	1
#define GREEN1	2
#define CYAN1	3
#define RED1	4
#define MAGENTA1 5
#define YELLOW1	6
#define GRAY1	7
#define GRAY2	8
#define BLUE2	9
#define GREEN2	10
#define CYAN2	11
#define RED2	12
#define MAGENTA2 13
#define YELLOW2	14
#define WHITE	15

#define PLAYER 'M' // player1 ǥ��
#define BLANK ' ' // ' ' ���ϸ� ������ �������� 

#define ESC 0x1b //  ESC ������ ����

#define UP		'w' // WASD�� �̵�
#define DOWN	's'
#define LEFT	'a'
#define RIGHT	'd'
#define SPACE	' ' // �����̽��ٷ� �߻�

#define WIDTH 80
#define HEIGHT 24

int Delay = 100; // 100 msec delay, �� ���� ���̸� �ӵ��� ��������.
int keep_moving = 1; // 1:����̵�, 0:��ĭ���̵�.
int life = 5; // ��ȸ
int score = 0;
int brick[WIDTH][HEIGHT - 2] = { 0 }; // 1�̸� Gold �ִٴ� ��
int brick_print_interval = 3; // ���� ǥ�� ����
int brick_count;
int called; 
int frame_count = 0; // game ���� frame count �� �ӵ� ���������� ���ȴ�.
int player_frame_sync = 10; // ó�� ������ 10 frame ���� �̵�, ��, 100msec ���� �̵�
int player_frame_sync_count = 0;
int gold_frame_sync = 1; // 50 frame ���� �ѹ��� gold �� �����δ�.

void removeCursor(void) { // Ŀ���� �Ⱥ��̰� �Ѵ�

	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 0;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}

void gotoxy(int x, int y) //���� ���ϴ� ��ġ�� Ŀ�� �̵�
{
	COORD pos = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);// WIN32API �Լ��Դϴ�. �̰� ���ʿ� �����
}
/*���ڿ��� ��쵵 �����ؾ� �Ѵ�.*/
void putstar(int x, int y, char ch)
{
	gotoxy(x, y);
	putchar(ch);
}

void erasestar(int x, int y)
{
	gotoxy(x, y);
	putchar(BLANK);
}

void textcolor(int fg_color, int bg_color)
{
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), fg_color | bg_color << 4);
}
// ȭ�� ����� ���ϴ� �������� �����Ѵ�.
void cls(int bg_color, int text_color)
{
	char cmd[100];
	system("cls");
	sprintf(cmd, "COLOR %x%x", bg_color, text_color);
	system(cmd);
}

void showscore()
{
	textcolor(GREEN2, GRAY2);
	gotoxy(10, 0);
	printf("SCORE : %d", score);
	textcolor(WHITE, BLACK);
}

void showLife() // ���� ��ȸ�� �����ش�.
{
	int i = 0;
	textcolor(YELLOW2, GRAY2);
	gotoxy(28, 0);
	printf("���� ���� : ");
	for (; i < life; i++) {
		printf("��");
	}
	for (; i < 5; i++) {
		printf("��");
	}
	textcolor(WHITE, BLACK);
}

#define BRICK	"��"
void show_brick()
{
	int x, y;
	x = rand() % WIDTH;
	y = rand() % (HEIGHT / 4) + 1;  // ���� ����� ���Ѵ�
	//textcolor(YELLOW2, GRAY1);
	gotoxy(x, y);
	printf(BRICK);
	brick[x][y] = 1;
	textcolor(WHITE, BLACK);
}

void move_brick() {
	int x, y, dx, dy, newx, newy;
	int newgolds[WIDTH][HEIGHT] = { 0 };
	static call_count = 0;

	// gold ���� ���� �� �ִ�.
	if (brick_count == 0)
		return;
	for (x = 0; x < WIDTH; x++) {
		for (y = 0; y < HEIGHT; y++) {
			if (brick[x][y]) {
				dx = rand() % 3 - 1; // -1 0 1
				dy = rand() % 3 - 1; // -1 0 1
				newx = x + dx;
				newy = y + dy;
				if (newx == WIDTH) newx = WIDTH - 1;
				if (newx < 0) newx = 0;
				if (newy > HEIGHT - 1) newy = HEIGHT - 1;
				if (newy < 1) newy = 1;
				gotoxy(x, y);
				textcolor(WHITE, WHITE);
				printf(" "); // erase gold
				textcolor(YELLOW2, GRAY1);
				gotoxy(newx, newy);
				printf(BRICK);
				newgolds[newx][newy] = 1; // �̵��� golds�� ��ǥ
				textcolor(BLACK, WHITE);
			}
		}
	}
	memcpy(brick, newgolds, sizeof(newgolds)); // �ѹ��� gold ��ġ�� �����Ѵ�.
}

void flush_key()
{
	while (kbhit())
		getch();
}

void draw_box(int x1, int y1, int x2, int y2, char* ch)
{
	int x, y;
	for (x = x1; x <= x2; x += 1) {
		gotoxy(x, y1);
		printf("%s", ch);
		gotoxy(x, y2);
		printf("%s", ch);
	}
	for (y = y1; y <= y2; y++) {
		gotoxy(x1, y);
		printf("%s", ch);
		gotoxy(x2, y);
		printf("%s", ch);
	}
}

void draw_hline(int y, int x1, int x2, char ch)
{
	gotoxy(x1, y);
	for (; x1 <= x2; x1++)
		putchar(ch);
}

void player(unsigned char ch)
{
	static int oldx = 40, oldy = 20, newx = 40, newy = 20;
	int move_flag = 0;
	static unsigned char last_ch = 0;

	if (!called) { // ó�� �Ǵ� Restart
		oldx = 40, oldy = 20, newx = 40, newy = 20;
		putstar(oldx, oldy, PLAYER);
		called = 1;
		last_ch = 0;
		ch = 0;
	}
	if (keep_moving && ch == 0)
		ch = last_ch;
	last_ch = ch;

	switch (ch) {
	case UP:
		if (oldy > 1) // 0 �� Status Line
			newy = oldy - 1;
		else { // ���� �ε�ġ�� ������ �ݴ�� �̵�
			newy = oldy + 1;
			last_ch = DOWN;
		}
		move_flag = 1;
		break;
	case DOWN:
		if (oldy < HEIGHT - 1)
			newy = oldy + 1;
		else {
			newy = oldy - 1;
			last_ch = UP;
		}
		move_flag = 1;
		break;
	case LEFT:
		if (oldx > 0)
			newx = oldx - 1;
		else {
			newx = oldx + 1;
			last_ch = RIGHT;
		}
		move_flag = 1;
		break;
	case RIGHT:
		if (oldx < WIDTH - 1)
			newx = oldx + 1;
		else {
			newx = oldx - 1;
			last_ch = LEFT;
		}
		move_flag = 1;
		break;
	}
	if (move_flag) {
		erasestar(oldx, oldy); // ������ ��ġ�� * �� �����
		putstar(newx, newy, PLAYER); // ���ο� ��ġ���� * �� ǥ���Ѵ�.
		oldx = newx; // ������ ��ġ�� ����Ѵ�.
		oldy = newy;
		if (brick[newx][newy]) {
			score++;
			brick[newx][newy] = 0;
			showscore(0);
		}
	}

}

void init_game()
{
	int x, y;
	char cmd[100];

	srand(time(NULL));
	score = 0;

	for (x = 0; x < WIDTH; x++)
		for (y = 0; y < HEIGHT; y++)
			brick[x][y] = 0;

	brick_print_interval = 3;

	keep_moving = 1;
	Delay = 100;

	//cls(WHITE, BLACK);
	cls(BLACK, WHITE);
	removeCursor();
	textcolor(BLACK, GRAY2);
	//textcolor(BLACK, WHITE);
	textcolor(WHITE, BLACK);
	sprintf(cmd, "mode con cols=%d lines=%d", WIDTH, HEIGHT);
	system(cmd);
	
	draw_hline(HEIGHT - 2, 0, WIDTH - 1, '��');

	called = 0; // set player
	player(0); 
}

void firstWindows() {
	removeCursor();
	draw_box(0, 0, WIDTH - 1, HEIGHT - 1, "��");
	gotoxy(25, HEIGHT / 2);
	printf("���� �ʱ�ȭ�� ���������Դϴ�.");
}


void main() {
	unsigned char ch;
	int run_time, start_time, gold_time;
	int brick_spawn_time;

	int firstWindow_CallCount;

START:

// initialize
	firstWindows();
	firstWindow_CallCount = 0;
	gold_time = 0;

	while (1) { 
		gotoxy(33, HEIGHT / 2 + 5);
		Sleep(250);
		if (firstWindow_CallCount & 1) printf("press to start");
		else printf("              ");
		if (kbhit()) break;
		++firstWindow_CallCount;
	}

	flush_key(); // ���� �ѹ� ����

	init_game();
	showscore();
	showLife();
	start_time = time(NULL);

// main loop
	while (1) {
		run_time = time(NULL) - start_time;
		if (run_time > gold_time && !(run_time % brick_print_interval)){
			show_brick();
			gold_time = run_time;
		}

		if (kbhit()) {
			ch = getch();
			if (ch == ESC) break;
			//player(ch);
	
			switch (ch) {
			case UP:
			case DOWN:
			case LEFT:
			case RIGHT:
				player(ch);
				if (!(frame_count % player_frame_sync == 0))
					player(0);
				break;
			default:// ���� ��ȯ�� �ƴϸ�
				if (frame_count % player_frame_sync == 0)
					player(0);
			}
		}
		if (frame_count % gold_frame_sync == 0)
			move_brick(); // ������ ��ġ�� �����Ѵ�.
		Sleep(Delay); // Delay ���� ���̰�
		frame_count++; // frame_count ������ �ӵ� ������ �Ѵ�.
	}





	// end
	gotoxy(30, 7);
	textcolor(GREEN2, WHITE);
	printf("   YOUR SCORE : %2d   ", score);

	while (1) {
		int c1, c2;
		do { // ���� �����ϸ鼭 Game Over ���
			c1 = rand() % 16;
			c2 = rand() % 16;
		} while (c1 == c2);
		textcolor(c1, c2);
		gotoxy(32, 10);
		printf("** Game Over **");
		gotoxy(24, 13);
		textcolor(BLACK, WHITE);
		printf("  Hit (R) to Restart (Q) to Quit  ");
		Sleep(300);
		if (kbhit()) {
			ch = getch();
			if (ch == 'r') goto START;
			else if(ch == 'q') break;
		}


	}
	textcolor(WHITE, BLACK);
	gotoxy(0, HEIGHT-1);
}