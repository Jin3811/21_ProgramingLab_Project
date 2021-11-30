#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#include <conio.h>

// 색상 정의
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

#define PLAYER 'M' // player1 표시
#define BLANK ' ' // ' ' 로하면 흔적이 지워진다 

#define ESC 0x1b //  ESC 누르면 종료

#define UP		'w' // WASD로 이동
#define DOWN	's'
#define LEFT	'a'
#define RIGHT	'd'
#define SPACE	' ' // 스페이스바로 발사

#define WIDTH 80
#define HEIGHT 24

int Delay = 100; // 100 msec delay, 이 값을 줄이면 속도가 빨라진다.
int keep_moving = 1; // 1:계속이동, 0:한칸씩이동.
int life = 5; // 기회
int score = 0;
int brick[WIDTH][HEIGHT - 2] = { 0 }; // 1이면 Gold 있다는 뜻
int brick_print_interval = 3; // 벽돌 표시 간격
int brick_count;
int called; 
int frame_count = 0; // game 진행 frame count 로 속도 조절용으로 사용된다.
int player_frame_sync = 10; // 처음 시작은 10 frame 마다 이동, 즉, 100msec 마다 이동
int player_frame_sync_count = 0;
int gold_frame_sync = 1; // 50 frame 마다 한번씩 gold 를 움직인다.

void removeCursor(void) { // 커서를 안보이게 한다

	CONSOLE_CURSOR_INFO curInfo;
	GetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
	curInfo.bVisible = 0;
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &curInfo);
}

void gotoxy(int x, int y) //내가 원하는 위치로 커서 이동
{
	COORD pos = { x, y };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);// WIN32API 함수입니다. 이건 알필요 없어요
}
/*문자열의 경우도 고찰해야 한다.*/
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
// 화면 지우고 원하는 배경색으로 설정한다.
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

void showLife() // 남은 기회를 보여준다.
{
	int i = 0;
	textcolor(YELLOW2, GRAY2);
	gotoxy(28, 0);
	printf("남은 생명 : ");
	for (; i < life; i++) {
		printf("♥");
	}
	for (; i < 5; i++) {
		printf("♡");
	}
	textcolor(WHITE, BLACK);
}

#define BRICK	"□"
void show_brick()
{
	int x, y;
	x = rand() % WIDTH;
	y = rand() % (HEIGHT / 4) + 1;  // 제일 상단은 피한다
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

	// gold 수가 없을 수 있다.
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
				newgolds[newx][newy] = 1; // 이동된 golds의 좌표
				textcolor(BLACK, WHITE);
			}
		}
	}
	memcpy(brick, newgolds, sizeof(newgolds)); // 한번에 gold 위치를 조정한다.
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

	if (!called) { // 처음 또는 Restart
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
		if (oldy > 1) // 0 은 Status Line
			newy = oldy - 1;
		else { // 벽에 부딛치면 방향을 반대로 이동
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
		erasestar(oldx, oldy); // 마지막 위치의 * 를 지우고
		putstar(newx, newy, PLAYER); // 새로운 위치에서 * 를 표시한다.
		oldx = newx; // 마지막 위치를 기억한다.
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
	
	draw_hline(HEIGHT - 2, 0, WIDTH - 1, '─');

	called = 0; // set player
	player(0); 
}

void firstWindows() {
	removeCursor();
	draw_box(0, 0, WIDTH - 1, HEIGHT - 1, "▒");
	gotoxy(25, HEIGHT / 2);
	printf("현재 초기화면 디자인중입니다.");
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

	flush_key(); // 버퍼 한번 비우기

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
			default:// 방향 전환이 아니면
				if (frame_count % player_frame_sync == 0)
					player(0);
			}
		}
		if (frame_count % gold_frame_sync == 0)
			move_brick(); // 벽돌의 위치를 변경한다.
		Sleep(Delay); // Delay 값을 줄이고
		frame_count++; // frame_count 값으로 속도 조절을 한다.
	}





	// end
	gotoxy(30, 7);
	textcolor(GREEN2, WHITE);
	printf("   YOUR SCORE : %2d   ", score);

	while (1) {
		int c1, c2;
		do { // 색을 변경하면서 Game Over 출력
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