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

#define PLAYER "<-*->" // player1 표시
#define BLANK ' ' // ' ' 로하면 흔적이 지워진다 
#define BRICK	"□" // 벽돌. 특수문자이다.

#define ESC 0x1b //  ESC 누르면 종료

#define UP		'w' // WASD로 이동
#define DOWN	's'
#define LEFT	'a'
#define RIGHT	'd'
#define SPACE	' ' // 스페이스바로 발사

#define WIDTH 80
#define HEIGHT 24

int Delay = 1;// msec
int keep_moving = 1; // 1:계속이동, 0:한칸씩이동.
int life = 5; // 기회
int score = 0; // 점수
int brick[WIDTH / 2][HEIGHT - 2] = { 0 }; // 1이면 벽돌이 있다는 뜻, 벽돌이 2byte 문자열이므로, 행을 반으로하고 이후에 2를 곱한다.
int bullet[WIDTH][HEIGHT - 2] = { 0 }; // 1이면 총알이 있다는 뜻
int brick_count;
int called; 
int frame_count = 1; // game 진행 frame count 로 속도 조절용으로 사용된다.
int brick_create_frame_sync = 10; // 벽돌 생성 간격
int player_frame_sync = 7; // 처음 시작은 10 frame 마다 이동, 즉, 100msec 마다 이동
int brick_frame_sync = 1; //  frame 마다 한번씩 brick를 움직인다.
// int bullet_frame_sync = 1; // 1 frame 마다 한번씩 bullet을 움직인다. -> 굳이?

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

void putplayer(int x, int y, char * ch)
{
	gotoxy(x, y);
	printf(ch);
}

void erasestar(int x, int y)
{
	gotoxy(x, y);
	putchar(BLANK);
}

void eraseplayer(int x, int y) {
	gotoxy(x - 2, y);
	printf("       ");
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

void show_brick()
{
	int x = rand() % (WIDTH / 2); // 벽돌이 2byte이므로, 너비의 절반까지의 랜덤값 생성
	int y = rand() % (HEIGHT / 4) + 1;  // 제일 상단은 피한다
	gotoxy(2 * x, y); // 2byte이므로 각 벽돌이 서로의 출력을 지우지 않게 하기 위해 랜덤값 * 2
	printf(BRICK);
	brick[x][y] = 1;
	++brick_count;
}

void move_brick() {
	int x, y, newy = 0;
	int newbricks[WIDTH/2][HEIGHT-2] = { 0 };
	static int call_count = 0;

	if (brick_count == 0) // 벽돌이 없는 경우 그냥 return
		return;

	for (x = 0; x < WIDTH/2; x++) {
		for (y = 0; y < HEIGHT-1; y++) {
			if (brick[x][y]) { // 블럭이 있는 경우
				newy = y + 1; // y좌표 
				if (newy < HEIGHT - 2) { // 벽돌이 바닥에 닿지 않는다면
					gotoxy(2*x, y);
					printf("  "); // erase brick
					gotoxy(2*x, newy);
					printf(BRICK);
					newbricks[x][newy] = 1; // 이동된 golds의 좌표
				}
				else { // 바닥에 닿았다면
					gotoxy(2*x, y);
					printf("  "); // erase brick
					--life; // life - 1
				}
			}
		}
	}
	memcpy(brick, newbricks, sizeof(newbricks)); // 한번에 gold 위치를 조정한다.
}

void flush_key()
{
	while (kbhit())
		getch();
}

void draw_box(int x1, int y1, int x2, int y2, char* ch)
{
	for (int x = x1; x < x2; ++x) {
		gotoxy(x, y1);
		printf("%s", ch);
		gotoxy(x, y2);
		printf("%s", ch);
	}
	for (int y = y1; y < y2; ++y) {
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
		putplayer(oldx, oldy, PLAYER);
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
		//erasestar(oldx, oldy); // 마지막 위치의 player 를 지우고
		eraseplayer(oldx, oldy);
		if (newx & 1) {// 홀수, 벽돌을 같이 지우기 위함
			erasestar(newx - 1, newy);
			erasestar(newx, newy);
		}
		else {
			erasestar(newx + 1, newy);
			erasestar(newx, newy);
		}
		putplayer(newx, newy, PLAYER); // 새로운 위치에서 player를 표시한다.
		oldx = newx; // 마지막 위치를 기억한다.
		oldy = newy;
		if (brick[newx/2][newy]) {
			score++;
			brick[newx/2][newy] = 0;
			--brick_count;
			showscore();
		}
	}

}

void init_game()
{
	int x, y;
	char cmd[100];

	srand(time(NULL));
	score = 0;

	for (x = 0; x < WIDTH / 2; x++)
		for (y = 0; y < HEIGHT; y++)
			brick[x][y] = 0;

	brick_create_frame_sync = 3;

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

	// draw_hline(HEIGHT - 2, 0, WIDTH - 2, '-');

}

void firstWindows(int sec) {
	if (sec & 1) {
		gotoxy(3, 2);
		textcolor(RED1, BLACK);
		printf("     ▣▣▣     ");
		textcolor(GREEN1, BLACK);
		printf("▣        ▣");
		textcolor(MAGENTA1, BLACK);
		printf("      ▣▣▣        ");
		textcolor(YELLOW1, BLACK);
		printf("▣▣▣");
		textcolor(CYAN1, BLACK);
		printf("    ▣▣▣▣▣   "); 
		textcolor(WHITE, BLACK);
		printf("▣");

		gotoxy(3, 3);
		textcolor(RED1, BLACK);
		printf("   ▣           "); 
		textcolor(GREEN1, BLACK);
		printf("▣        ▣    ");
		textcolor(MAGENTA1, BLACK);
		printf("▣      ▣    ");
		textcolor(YELLOW1, BLACK);
		printf("▣      ▣      ");
		textcolor(CYAN1, BLACK);
		printf("▣       ");
		textcolor(WHITE, BLACK);
		printf("▣");

		gotoxy(3, 4);
		textcolor(RED1, BLACK);
		printf("   ▣           ");
		textcolor(GREEN1, BLACK);
		printf("▣        ▣    ");
		textcolor(MAGENTA1, BLACK);
		printf("▣      ▣    ");
		textcolor(YELLOW1, BLACK);
		printf("▣      ▣      ");
		textcolor(CYAN1, BLACK);
		printf("▣       ");
		textcolor(WHITE, BLACK);
		printf("▣");

		gotoxy(3, 5);
		textcolor(RED1, BLACK);
		printf("   ▣           ");
		textcolor(GREEN1, BLACK);
		printf("▣        ▣    ");
		textcolor(MAGENTA1, BLACK);
		printf("▣      ▣    ");
		textcolor(YELLOW1, BLACK);
		printf("▣      ▣      ");
		textcolor(CYAN1, BLACK);
		printf("▣       ");
		textcolor(WHITE, BLACK);
		printf("▣");

		gotoxy(3, 6);
		textcolor(RED1, BLACK);
		printf("     ▣▣▣       ");
		textcolor(GREEN1, BLACK);
		printf("▣▣▣▣      "); 
		textcolor(MAGENTA1, BLACK);
		printf("▣      ▣    "); 
		textcolor(YELLOW1, BLACK);
		printf("▣      ▣      ");
		textcolor(CYAN1, BLACK);
		printf("▣       ");
		textcolor(WHITE, BLACK);
		printf("▣");
	
		gotoxy(3, 7);
		textcolor(RED1, BLACK);
		printf("           ▣   ");
		textcolor(GREEN1, BLACK);
		printf("▣        ▣    ");
		textcolor(MAGENTA1, BLACK);
		printf("▣      ▣    ");
		textcolor(YELLOW1, BLACK);
		printf("▣      ▣      ");
		textcolor(CYAN1, BLACK);
		printf("▣       ");
		textcolor(WHITE, BLACK);
		printf("▣");

		gotoxy(3, 8);
		textcolor(RED1, BLACK);
		printf("           ▣   ");
		textcolor(GREEN1, BLACK);
		printf("▣        ▣    ");
		textcolor(MAGENTA1, BLACK);
		printf("▣      ▣    ");
		textcolor(YELLOW1, BLACK);
		printf("▣      ▣      ");
		textcolor(CYAN1, BLACK);
		printf("▣       ");
		textcolor(WHITE, BLACK);
		printf("▣");

		gotoxy(3, 9);
		textcolor(RED1, BLACK);
		printf("           ▣   ");
		textcolor(GREEN1, BLACK);
		printf("▣        ▣    ");
		textcolor(MAGENTA1, BLACK);
		printf("▣      ▣    ");
		textcolor(YELLOW1, BLACK);
		printf("▣      ▣      ");
		textcolor(CYAN1, BLACK);
		printf("▣");

		gotoxy(3, 10);
		textcolor(RED1, BLACK);
		printf("     ▣▣▣     ");
		textcolor(GREEN1, BLACK);
		printf("▣        ▣      ");
		textcolor(MAGENTA1, BLACK);
		printf("▣▣▣        ");
		textcolor(YELLOW1, BLACK);
		printf("▣▣▣        ");
		textcolor(CYAN1, BLACK);
		printf("▣       ");
		textcolor(WHITE, BLACK);
		printf("▣");
	}
	else {
		textcolor(WHITE, BLACK);
		for (int y = 2; y <= 10; ++y) {
			gotoxy(3, y);
			printf("                                                                         ");
		}
	}
	textcolor(WHITE, BLACK);
}


void main() {
	unsigned char ch;
	int run_time, start_time, brick_time;
	int brick_spawn_time;

	int firstWindow_CallCount;

START: // initialize
	removeCursor();
	firstWindow_CallCount = 0;
	brick_time = 0;

	draw_box(0, 0, WIDTH - 2, HEIGHT - 1, "■");
	while (1) { 
		firstWindows(firstWindow_CallCount);
		gotoxy(33, HEIGHT - 3);
		if (firstWindow_CallCount++ & 1) printf("press to start");
		else printf("              ");
		Sleep(300);
		if (kbhit()) {
			ch = getch();
			if (ch == ESC) goto END;
			else break;
		}
	}

	//flush_key(); // 버퍼 한번 비우기
	init_game();
	showscore();
	showLife();
	start_time = time(NULL);

// main loop
	while (1) {
		life = 5;
		showLife();
		if (life == 0) break;
		run_time = time(NULL) - start_time;
		//if (run_time > brick_time && (run_time % brick_print_interval == 0)) {
		//	show_brick();
		//	brick_time = run_time; // 마지막 GOLD 표시 시간 기억
		//}
		if (frame_count % brick_create_frame_sync == 0) {
			show_brick();
		}

		if (kbhit()) {
			ch = getch();
			if (ch == ESC) break;
			//player(ch);
	
			switch (ch) {
			case UP: // 이동
			case DOWN:
			case LEFT:
			case RIGHT:
				player(ch);
				if (!(frame_count % player_frame_sync == 0))
					player(0);
				break;
			case SPACE: // 발사

			default:// 방향 전환이 아니면
				if (frame_count % player_frame_sync == 0)
					player(0);
			}
		}
		if (frame_count % brick_frame_sync == 0)
			move_brick(); // 벽돌의 위치를 변경한다.
		Sleep(Delay); // Delay 값을 줄이고
		++frame_count; // frame_count 값으로 속도 조절을 한다.
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

END:
	gotoxy(0, HEIGHT-1);
}