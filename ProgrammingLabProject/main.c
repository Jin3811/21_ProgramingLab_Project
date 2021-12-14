#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <Windows.h>
#include <conio.h>
#include <mmsystem.h>
#pragma comment(lib,"winmm.lib") // mmsystem.h

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

#define PLAYER "<-*->" // player1 ǥ��
#define BLANK	' ' // ' ' ���ϸ� ������ �������� 
#define BRICK	"��" // ����. Ư�������̴�.
#define BULLET	"��" // �Ѿ�

#define ESC 0x1b //  ESC ������ ����

#define UP		'w' // WASD�� �̵�
#define DOWN	's'
#define LEFT	'a'
#define RIGHT	'd'
#define SPACE	' ' // �����̽��ٷ� �߻�

#define WIDTH 80
#define HEIGHT 24

#define PLAYSOUND(str) PlaySound(TEXT(str), NULL, SND_FILENAME | SND_ASYNC); // �뷡 ����Լ�. ��ũ�η� ���� ���� ����.

// �ʱ�ȭ�� init �Լ�����
int Delay;// msec
int life; // ��ȸ
int score; // ����
int brick_count;
int brick[WIDTH / 2][HEIGHT - 2] = { 0 }; // 1�̸� ������ �ִٴ� ��, ������ 2byte ���ڿ��̹Ƿ�, ���� �������ϰ� ���Ŀ� 2�� ���Ѵ�.
int bullet_count;
int bullet[WIDTH][HEIGHT - 2] = { 0 }; // 1�̸� �Ѿ��� �ִٴ� ��
int called;

// frame var
int frame_count; // ��ü frame
int brick_create_frame_sync; // ���� ���� ����
int brick_frame_sync; //  frame ���� �ѹ��� brick�� �����δ�.
int player_frame_sync; // ó�� ������ 10 frame ���� �̵�, ��, 100msec ���� �̵�
int bullet_frame_sync; // 1 frame ���� �ѹ��� bullet�� �����δ�.

// player�� ��ǥ. bullet�� ������.
int p_oldx = 40, p_oldy = 20, p_newx = 40, p_newy = 20;

int crash; // ������ �÷��̾ �΋H������ �Ǵ��ϱ� ���� ����
int breakBrick; // ������ �ν����� �Ǵ��ϴ� ����

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

void putplayer(int x, int y)
{
	gotoxy(p_newx - 2, p_newy);
	
	if (p_newx == 2) {
		gotoxy(p_newx, p_newy);
		printf("*->");
	}
	else if (p_newx == 3) {
		gotoxy(p_newx-1, p_newy);
		printf("-*->");
	}
	else if (p_newx == 76) {
		printf("<-*-");
	}
	else if (p_newx == 77) {
		printf("<-*");
	}
	else {
		printf(PLAYER);
	}
}

void erasestar(int x, int y)
{
	gotoxy(x, y);
	putchar(BLANK);
}

void eraseplayer(int x, int y) {
	gotoxy(p_oldx - 2, p_oldy);

	if (p_oldx == 2) {
		gotoxy(p_oldx, p_oldy);
		printf("   ");
	}

	else if (p_oldx == 3) {
		gotoxy(p_oldx - 1, p_oldy);
		printf("    ");
	}

	else if (p_oldx == 76) {
		printf("    ");
	}

	else if (p_oldx == 77) {
		printf("   ");
	}

	else {
		printf("     ");
	}
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

void show_brick()
{
	int x = rand() % (WIDTH / 2 - 3) + 1; // ������ 2byte�̹Ƿ�, �ʺ��� ���ݱ����� ������ ����
	int y = rand() % (HEIGHT / 5) + 2;  // ���� ������ ���Ѵ�. ���� �ʹ� �Ʒ����� �������� �ʵ��� ���������� ���δ�.
	gotoxy(2 * x, y); // 2byte�̹Ƿ� �� ������ ������ ����� ������ �ʰ� �ϱ� ���� ������ * 2
	printf(BRICK);
	brick[x][y] = 1;
	++brick_count;
}

void move_brick() {
	int x, y, newy = 0;
	int newbricks[WIDTH/2][HEIGHT-2] = { 0 };
	static int call_count = 0;

	if (brick_count == 0) // ������ ���� ��� �׳� return
		return;

	for (x = 0; x < WIDTH/2; x++) {
		for (y = 0; y < HEIGHT-1; y++) {
			if (brick[x][y]) { // ���� �ִ� ���
				newy = y + 1; // y��ǥ 
				if (newy < HEIGHT - 1) { // ������ �ٴڿ� ���� �ʴ´ٸ�
					gotoxy(2*x, y);
					printf("  "); // erase brick
					gotoxy(2*x, newy);
					printf(BRICK);
					newbricks[x][newy] = 1; // �̵��� golds�� ��ǥ
				}
				else { // �ٴڿ� ��Ҵٸ�
					gotoxy(2*x, y);
					printf("  "); // erase brick
					--life; // life - 1
					score = score > 10 ? score - 10 : 0;
				}
				if((2*x >= p_newx - 2 && 2*x <= p_newx + 2) && newy == p_newy) { // player�� ��Ҵٸ�
					if (crash == 0) {
						gotoxy(2 * x, y);
						printf("  ");
						--life;
						score = score > 5 ? score - 5 : 0;
						brick[x][y] = 0;
						--brick_count;
					}
					else crash = 0;
				}
			}
		}
	}
	memcpy(brick, newbricks, sizeof(newbricks)); // �ѹ��� gold ��ġ�� �����Ѵ�.
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
	int move_flag = 0;
	static unsigned char last_ch = 0;

	if (!called) { // ó�� �Ǵ� Restart
		p_oldx = 40, p_oldy = 20, p_newx = 40, p_newy = 20;
		putplayer(p_oldx - 2, p_oldy);
		called = 1;
		last_ch = 0;
		ch = 0;
	}

	switch (ch) {
	case UP:
		if (p_oldy > 2) // 0 �� Status Line
			p_newy = p_oldy - 1;
		else { // ���� �ε�ġ�� ������ �ݴ�� �̵�
			p_newy = p_oldy + 1;
		}
		move_flag = 1;
		break;
	case DOWN:
		if (p_oldy < HEIGHT - 2)
			p_newy = p_oldy + 1;
		else {
			p_newy = p_oldy - 1;
		}
		move_flag = 1;
		break;
	case LEFT:
		if (p_oldx > 2)
			p_newx = p_oldx - 1;
		else {
			p_newx = p_oldx + 1;
		}
		move_flag = 1;
		break;
	case RIGHT:
		if (p_oldx < WIDTH - 3)
			p_newx = p_oldx + 1;
		else {
			p_newx = p_oldx - 1;
		}
		move_flag = 1;
		break;
	}
	if (move_flag) {
		eraseplayer(p_oldx, p_oldy); // ���� ��ġ�� 
		if (p_newx & 1) {// Ȧ��, ������ ���� ����� ����
			erasestar(p_newx - 1, p_newy);
			erasestar(p_newx, p_newy);
		}
		else {
			erasestar(p_newx + 1, p_newy);
			erasestar(p_newx, p_newy);
		}
		putplayer(p_newx-2, p_newy); // ���ο� ��ġ���� player�� ǥ���Ѵ�.
		p_oldx = p_newx; // ������ ��ġ�� ����Ѵ�.
		p_oldy = p_newy;
		
		if (brick[p_newx/2][p_newy]) { // �ε��� üũ
			if (crash == 0) {
				score = score > 5 ? score - 5 : 0;
				brick[p_newx / 2][p_newy] = 0;
				--brick_count;
				showscore();
				--life;
			}
			else crash = 0;
		}
	}
}

void init_game()
{
	int x, y;
	char cmd[100];

	srand(time(NULL));

	brick_count = 0; // ���� �ʱ�ȭ
	for (x = 0; x < WIDTH / 2; x++)
		for (y = 0; y < HEIGHT; y++)
			brick[x][y] = 0;

	bullet_count = 0; // �Ѿ� �ʱ�ȭ
	for (x = 0; x < WIDTH; x++)
		for (y = 0; y < HEIGHT; y++)
			bullet[x][y] = 0;

	// ���� �ʱ�ȭ
	score = 0;
	life = 5;
	Delay = 1;
	frame_count = 0;
	brick_create_frame_sync = 25;
	brick_frame_sync = 75;
	player_frame_sync = 1;
	bullet_frame_sync = 1;
	crash = 0;
	breakBrick = 0;

	cls(BLACK, WHITE);
	sprintf(cmd, "mode con cols=%d lines=%d", WIDTH, HEIGHT);
	system(cmd);

	called = 0; // set player
	player(0); 
}

void firstWindows(int sec) {
	if (sec & 1) {
		gotoxy(3, 2);
		textcolor(RED1, BLACK);
		printf("     �ââ�     ");
		textcolor(GREEN1, BLACK);
		printf("��        ��");
		textcolor(MAGENTA1, BLACK);
		printf("      �ââ�        ");
		textcolor(YELLOW1, BLACK);
		printf("�ââ�");
		textcolor(CYAN1, BLACK);
		printf("    �ââââ�   "); 
		textcolor(WHITE, BLACK);
		printf("��");

		gotoxy(3, 3);
		textcolor(RED1, BLACK);
		printf("   ��           "); 
		textcolor(GREEN1, BLACK);
		printf("��        ��    ");
		textcolor(MAGENTA1, BLACK);
		printf("��      ��    ");
		textcolor(YELLOW1, BLACK);
		printf("��      ��      ");
		textcolor(CYAN1, BLACK);
		printf("��       ");
		textcolor(WHITE, BLACK);
		printf("��");

		gotoxy(3, 4);
		textcolor(RED1, BLACK);
		printf("   ��           ");
		textcolor(GREEN1, BLACK);
		printf("��        ��    ");
		textcolor(MAGENTA1, BLACK);
		printf("��      ��    ");
		textcolor(YELLOW1, BLACK);
		printf("��      ��      ");
		textcolor(CYAN1, BLACK);
		printf("��       ");
		textcolor(WHITE, BLACK);
		printf("��");

		gotoxy(3, 5);
		textcolor(RED1, BLACK);
		printf("   ��           ");
		textcolor(GREEN1, BLACK);
		printf("��        ��    ");
		textcolor(MAGENTA1, BLACK);
		printf("��      ��    ");
		textcolor(YELLOW1, BLACK);
		printf("��      ��      ");
		textcolor(CYAN1, BLACK);
		printf("��       ");
		textcolor(WHITE, BLACK);
		printf("��");

		gotoxy(3, 6);
		textcolor(RED1, BLACK);
		printf("     �ââ�       ");
		textcolor(GREEN1, BLACK);
		printf("�âââ�      "); 
		textcolor(MAGENTA1, BLACK);
		printf("��      ��    "); 
		textcolor(YELLOW1, BLACK);
		printf("��      ��      ");
		textcolor(CYAN1, BLACK);
		printf("��       ");
		textcolor(WHITE, BLACK);
		printf("��");
	
		gotoxy(3, 7);
		textcolor(RED1, BLACK);
		printf("           ��   ");
		textcolor(GREEN1, BLACK);
		printf("��        ��    ");
		textcolor(MAGENTA1, BLACK);
		printf("��      ��    ");
		textcolor(YELLOW1, BLACK);
		printf("��      ��      ");
		textcolor(CYAN1, BLACK);
		printf("��       ");
		textcolor(WHITE, BLACK);
		printf("��");

		gotoxy(3, 8);
		textcolor(RED1, BLACK);
		printf("           ��   ");
		textcolor(GREEN1, BLACK);
		printf("��        ��    ");
		textcolor(MAGENTA1, BLACK);
		printf("��      ��    ");
		textcolor(YELLOW1, BLACK);
		printf("��      ��      ");
		textcolor(CYAN1, BLACK);
		printf("��       ");
		textcolor(WHITE, BLACK);
		printf("��");

		gotoxy(3, 9);
		textcolor(RED1, BLACK);
		printf("           ��   ");
		textcolor(GREEN1, BLACK);
		printf("��        ��    ");
		textcolor(MAGENTA1, BLACK);
		printf("��      ��    ");
		textcolor(YELLOW1, BLACK);
		printf("��      ��      ");
		textcolor(CYAN1, BLACK);
		printf("��");

		gotoxy(3, 10);
		textcolor(RED1, BLACK);
		printf("     �ââ�     ");
		textcolor(GREEN1, BLACK);
		printf("��        ��      ");
		textcolor(MAGENTA1, BLACK);
		printf("�ââ�        ");
		textcolor(YELLOW1, BLACK);
		printf("�ââ�        ");
		textcolor(CYAN1, BLACK);
		printf("��       ");
		textcolor(WHITE, BLACK);
		printf("��");
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

void shotBullet() {
	if (p_newy > 2) {
		bullet[p_newx - 2][p_newy - 1] = 1;
		bullet[p_newx][p_newy - 1] = 1;
		bullet[p_newx + 2][p_newy - 1] = 1;
		bullet_count+=3;
		gotoxy(p_newx - 2, p_newy-1);
		printf(BULLET);
		gotoxy(p_newx, p_newy-1);
		printf(BULLET);
		gotoxy(p_newx - 2, p_newy-1);
		printf(BULLET);
	}
}

void moveBullet() {
	int newy = 0;
	int newbullet[WIDTH][HEIGHT - 2] = { 0 };

	if (bullet_count == 0) return; // �Ѿ��� ���ٸ� �׳� return

	for (int x = 0; x < WIDTH; ++x) {
		for (int y = 0; y < HEIGHT - 2; ++y) {
			if (bullet[x][y]) {
				newy = y - 1;
				if (newy > 1) {
					if (brick[x / 2][newy]) {
						if (x & 1) { // Ȧ����
							erasestar(x-1, newy);
							erasestar(x, newy);
						}
						else {
							erasestar(x+1, newy);
							erasestar(x, newy);
						}
						brick[x / 2][newy] = 0;
						--brick_count;
						bullet[x][y] = 0;
						--bullet_count;
						erasestar(x, y);
						++score;
						breakBrick = 1;
					}
					else {
						gotoxy(x, y);
						putchar(BLANK);
						gotoxy(x, newy);
						printf(BULLET);
						newbullet[x][newy] = 1;
						++bullet_count;
					}
				}
				else {
					gotoxy(x, y);
					putchar(BLANK);	
					newbullet[x][y] = 0;
					--bullet_count;
				}
			}
		}
	}
	memcpy(bullet, newbullet, sizeof(newbullet));
}

void main() {
	unsigned char ch;
	int firstWindow_CallCount;
	int tempScore = 0;

START: // initialize
	removeCursor();
	cls(BLACK, WHITE);
	firstWindow_CallCount = 0;

	draw_box(0, 0, WIDTH - 2, HEIGHT - 1, "��");
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

	flush_key(); // ���� �ѹ� ����
	init_game();
	showscore();
	showLife();
	
// main loop
	while (1) {
		draw_box(0, 1, WIDTH - 2, HEIGHT - 1, "��");
		showscore();
		showLife();
		if (life <= 0) break;
		if (frame_count % brick_create_frame_sync == 0)
			show_brick();

		if (kbhit()) {
			ch = getch();
			if (ch == ESC) break;
	
			switch (ch) {
			case UP: // �̵�
			case DOWN:
			case LEFT:
			case RIGHT:
				if (frame_count % player_frame_sync == 0)
				player(ch);
				break;
			case SPACE: // �߻�
				PLAYSOUND("shoot.wav")
				shotBullet();
			default:// ���� ��ȯ�� �ƴϸ�
				player(0);
			}
		}
		if (frame_count % brick_frame_sync == 0)
			move_brick(); // ������ ��ġ�� �����Ѵ�.
		if (frame_count % bullet_frame_sync == 0)
			moveBullet();
		Sleep(Delay);
		++frame_count;
		if (tempScore >= 5) {
			brick_frame_sync = 75 - score / 5 > brick_frame_sync ? brick_frame_sync : 75 - score/5; // �ӵ� ����
			tempScore = 0;
		}
		else if (breakBrick) {
			++tempScore;
			breakBrick = 0;
		}
		/*gotoxy(50, 0); // ���� ��Ȳ�� ���� ���� �ڵ�.
		printf("%d %d %d", frame_count, brick_frame_sync, tempScore);*/
	}

	// end
	PLAYSOUND("gameEnd.wav");
	flush_key();
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

END:
	gotoxy(0, HEIGHT-1);
}