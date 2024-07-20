#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <math.h>

#pragma region ���� ������
// ����ϴ� �ܼ��� x,y ������
int ConsoleSizeX = 120;
int ConsoleSizeY = 30;
// �÷����ϴ� ������ x,y �ּ� ��
int PlayAreaMinX = 2;
int PlayAreaMinY = 2;

typedef enum ColorType {
	BLACK,  	//0
	DarkBLUE,	//1
	DarkGreen,	//2
	DarkSkyBlue,//3
	DarkRed,  	//4
	DarkPurple,	//5
	DarkYellow,	//6
	GRAY,		//7
	DarkGray,	//8
	BLUE,		//9
	GREEN,		//10
	SkyBlue,	//11
	RED,		//12
	PURPLE,		//13
	YELLOW,		//14
	WHITE		//15
} Color;

// ���� �� �����̴� ���͵��� Ÿ���� ����
typedef enum ActorType
{
	WormHead,
	WormBody,
	Food,
	End,
} ActorType;

// ���͵��� ���������� ����ϴ� �����͸� ����üȭ
typedef struct Actor
{
	ActorType type; // � �������� ������ �뵵
	Color color; // ������ ������ ��Ÿ�� �뵵
	char* shape; // ������ ����� ��Ÿ�� �뵵
	int posX;  // ������ ȭ�� ���� ��ǥ x,y
	int posY;
} Actor;

// ���� �Է¿� ���� �����͸� ��Ÿ�� ����ü
typedef struct Input
{
	int dirX;
	int dirY;
}Input;

// �������� �⺻ ���� ����
const int DefaultBodyCnt = 3;
// �ʿ� ������ �� �ִ� �ִ� ���� ��
const int MaxFoodCnt = 3;
// ���̰� �����Ǵ� ���� (�и���)
const ULONGLONG SpawnFoodInterval = 1000;
// Ư������ ���� �׸� ��, x������ �� ĭ ��� �� ���Ǵ� ����
const int SpaceCnt = 2;
#pragma endregion

void InitConsole(); // ���ӿ� �°Բ� �ܼ��� �ʱ�ȭ�ϴ� ���
void SetPos(int x, int y); // �ܼ� Ŀ���� ��ġ�� �Ķ���ͷ� ���� x, y�� �����ϴ� ���
void SetColor(Color color); // �ܼ� ��� ������ �����ϴ� ���

Actor* CreateWorm(int posX, int posY, int length);
Actor* CreateFood(int posX, int posY);

void InputKey(Input* input);
int MoveWorm(Input* input, Actor* wormBodies, int eatCnt);
void SpawnFood(Actor** foods, ULONGLONG* prevSpawnFoodTime);
void CheckColl(Actor** wormBodies, Actor** foods, int* eatCnt);
void IncreaseBodies(Actor** wormBodies, int* eatCnt);

void DrawWorm(Actor* wormBodies, int eatCnt);
void DrawFood(Actor** foods);
void DrawGameOver();

void main()
{
	srand(GetTickCount64());

	// �ܼ� �ʱ�ȭ ����
	InitConsole();

	// ���� ���� �ݺ� ����
	int isRun = 1;
	// ��ü�� �Ӹ��� �ε������� ����
	int isCollBodies = 0;
	
	// ������� ���� ���� ��
	int eatCnt = 0;
	// ������ ���̰� ������ �ð�
	ULONGLONG prevSpawnFoodTime = GetTickCount64();

	// ���� �Է� ������ ����
	Input input;
	
	// ������ ������ ������ ���� ����
	Actor* wormBodies = CreateWorm(ConsoleSizeX / 2, ConsoleSizeY / 2, DefaultBodyCnt);
	// ���� �������� ��� ���� ����
	Actor** foods = (Actor**)malloc(sizeof(Actor*) * MaxFoodCnt);
	for (int i = 0; i < MaxFoodCnt; ++i)
		foods[i] = NULL;

	// ���� ���� �ݺ�
	while (isRun)
	{
		system("cls");

		// �Ӹ��� ����� �浹���� �ʾҴٸ�
		//  ����
		//  1. Ű �Է�
		//  2. �̵� (�Ӹ� �浹)
		//  3. Ǫ�� ���� ����
		//  4. ���̿��� �浹 üũ
		if (!isCollBodies)
		{
			InputKey(&input);
			isCollBodies = MoveWorm(&input, wormBodies, eatCnt);
			SpawnFood(foods, &prevSpawnFoodTime);
			CheckColl(&wormBodies, foods, &eatCnt);
		}

		// �׸���
		// ������ �׸���
		DrawWorm(wormBodies, eatCnt);
		// ���� �׸���
		DrawFood(foods);

		// �Ӹ��� ������ �浹�ߴٸ�, �ݺ� Ż��
		if (isCollBodies)
			isRun = 0;

		Sleep(60);
	}

	DrawGameOver();

	if (wormBodies != NULL)
	{
		free(wormBodies);
		wormBodies = NULL;
	}

	if (foods != NULL)
	{
		for (int i = 0; i < MaxFoodCnt; ++i)
		{
			if (foods[i] == NULL)
				continue;

			free(foods[i]);
			foods[i] = NULL;
		}

		free(foods);
		foods = NULL;
	}
}

void InitConsole()
{
	// �ܼ� Ŀ���� �� ���̰� ����
	CONSOLE_CURSOR_INFO cursorInfo; // �ܼ� Ŀ�� ������ ��Ÿ���� ����ü
	cursorInfo.dwSize = 1; // Ŀ�� ���� (1~100)
	cursorInfo.bVisible = FALSE; // Ŀ�� ����ȭ ����

	// ���� ���� Ŀ�� ������ ���� ���ۿ� ����
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

void SetPos(int x, int y)
{
	COORD pos = { x - 1, y - 1 };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void SetColor(Color color)
{
	// ������ �����ϸ�, ���� �� ���� �������� �����ߴ� �������� ��� ��� ��
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

Actor* CreateWorm(int posX, int posY, int length)
{
	// ���� �⺻ ���� ���̸�ŭ ���� �Ҵ�
	Actor* wormBodies = (Actor*)malloc(sizeof(Actor) * length);

	// ���뿡 ���� ������ ����
	for (int i = 0; i < length; ++i)
	{
		wormBodies[i].type = WormBody;
		wormBodies[i].color = WHITE;
		wormBodies[i].shape = "��";
	}
	// �Ӹ��� ���� ������ ����
	wormBodies[0].type = WormHead;
	wormBodies[0].color = GREEN;
	// �Ķ���ͷ� ���� ��ġ ���� �Ӹ��� �ʱ� ��ġ
	wormBodies[0].posX = posX;
	wormBodies[0].posY = posY;

	// ������ �Ӹ� ��ġ�� ������� ���Ƿ� ������ ������� ��ġ�� ����
	for (int i = 1; i < length; ++i)
	{
		wormBodies[i].posX = wormBodies[i - 1].posX - SpaceCnt;
		wormBodies[i].posY = wormBodies[i - 1].posY;
	}

	return wormBodies;
}

Actor* CreateFood(int posX, int posY)
{
	Actor* food = (Actor*)malloc(sizeof(Actor));

	food->type = Food;
	food->color = YELLOW;
	food->shape = "��";
	food->posX = posX;
	food->posY = posY;

	return food;
}

void InputKey(Input* input)
{
	input->dirX = 0;
	input->dirY = 0;

	if (GetAsyncKeyState('W') & 0x8000)
	{
		input->dirY = -1;
	}
	else if (GetAsyncKeyState('S') & 0x8000)
	{
		input->dirY = 1;
	}
	else
	{
		if (GetAsyncKeyState('A') & 0x8000)
		{
			input->dirX = -1;
		}
		else if (GetAsyncKeyState('D') & 0x8000)
		{
			input->dirX = 1;
		}
	}
}

int MoveWorm(Input* input, Actor* wormBodies, int eatCnt)
{
	// �̵��� �� �ִ��� ����
	int canMove = 1;

	// x�� �̵��� �ؾ��ϴ���?
	if (input->dirX != 0)
	{
		// ���� �����Ϳ� ���� ���� �̵� ��ġ�� �����Ѵ�.
		int nextPosX = wormBodies[0].posX + input->dirX * SpaceCnt;

		// ���� ��ġ�� �÷��� ������ ������� Ȯ��
		if (nextPosX < PlayAreaMinX || nextPosX >= ConsoleSizeX)
			canMove = 0;
		else
		{
			// �÷��� ������ �ȿ� ������ġ�� ����
			for (int i = 1; i < DefaultBodyCnt + eatCnt; ++i)
			{
				// �Ӹ��� ������ �浹�ϴ��� Ȯ��
				if (nextPosX == wormBodies[i].posX && wormBodies[0].posY == wormBodies[i].posY)
				{
					// �Ӹ� �ٷ� ���� ���� �������δ� �̵����� ���ϰ�
					if (i == 1)
						canMove = 0;
					else
					{
						// �Ӹ��� ������ ������� �浹�ߴٴ� �� (���ӿ��� ó��)
						wormBodies[0].color = RED;
						wormBodies[i].color = RED;
						return 1;
					}
				}
			}
		}

		// �̵��� �� �ִٸ�
		if (canMove)
		{
			for (int i = DefaultBodyCnt + eatCnt - 1; i > 0; --i)
			{
				wormBodies[i].posX = wormBodies[i - 1].posX;
				wormBodies[i].posY = wormBodies[i - 1].posY;
			}

			wormBodies[0].posX = nextPosX;
		}
	}

	// y�� �̵��� �ؾ��ϴ���?
	if (input->dirY != 0)
	{
		canMove = 1;
		int nextPosY = wormBodies[0].posY + input->dirY;

		if (nextPosY < PlayAreaMinY || nextPosY >= ConsoleSizeY)
			canMove = 0;
		else
		{
			for (int i = 1; i < DefaultBodyCnt + eatCnt; ++i)
			{
				if (nextPosY == wormBodies[i].posY && wormBodies[0].posX == wormBodies[i].posX)
				{
					if (i == 1)
						canMove = 0;
					else
					{
						wormBodies[0].color = RED;
						wormBodies[i].color = RED;
						return 1;
					}
				}
			}

		}

		if (canMove)
		{
			for (int i = DefaultBodyCnt + eatCnt - 1; i > 0; --i)
			{
				wormBodies[i].posX = wormBodies[i - 1].posX;
				wormBodies[i].posY = wormBodies[i - 1].posY;
			}

			wormBodies[0].posY = nextPosY;
		}
	}

	return 0;
}

void SpawnFood(Actor** foods, ULONGLONG* prevSpawnFoodTime)
{
	// ���� �ð��� ������
	ULONGLONG currentTime = GetTickCount64();

	// ���� ���� �ð� + ���� ���� �ð��� ���� ����
	// ���� �ð� ���� �۴ٸ�
	// ������ �ð��� �Ǿ��ٴ� ��
	if (*prevSpawnFoodTime + SpawnFoodInterval < currentTime)
	{
		// ���� Ǫ�带 ������ ���̹Ƿ�
		// ���� ���� �ð��� ���� �ð����� ����
		*prevSpawnFoodTime = currentTime;

		// ������ �� �ִ� �ִ� ���� ������ �ʰ����� �ʴ��� Ȯ��
		for (int i = 0; i < MaxFoodCnt; ++i)
		{
			// �Ķ���ͷ� ���� Ǫ�� �迭�� �� ������ �ִٸ�
			if (foods[i] == NULL)
			{
				// Ǫ�带 �ϳ� �����ϰ�, �ݺ� �ߴ�
				foods[i] = CreateFood(
					rand() % (ConsoleSizeX - PlayAreaMinX) + PlayAreaMinX,
					rand() % (ConsoleSizeY - PlayAreaMinY) + PlayAreaMinY);
				break;
			}
		}
	}
}

void CheckColl(Actor** wormBodies, Actor** foods, int* eatCnt)
{
	// ������ ���̿� �Ӹ��� �浹�� Ȯ��
	for (int i = 0; i < MaxFoodCnt; ++i)
	{
		// ���̰� �������� �ʴ´ٸ� ���� �ݺ�����
		if (foods[i] == NULL)
			continue;

		// ���̿� �Ӹ��� y ��ǥ�� �ٸ��ٸ� x��ǥ�� ���� �ʿ� ���� �Ѿ
		if ((*wormBodies)[0].posY != foods[i]->posY)
			continue;

		// ���̿� �Ӹ��� �Ÿ��� SpaceCnt ���� �۴ٸ� �浹 ����
		if (abs((*wormBodies)[0].posX - foods[i]->posX) < SpaceCnt)
		{
			// ���� ���� 
			free(foods[i]);
			foods[i] = NULL;

			// ���� ����
			IncreaseBodies(wormBodies, eatCnt);
		}
	}
}

void IncreaseBodies(Actor** wormBodies, int* eatCnt)
{
	// ������Ű�� ��, ���� ����
	int length = DefaultBodyCnt + (*eatCnt);

	// �� ������ �� ���� ������ ��ġ�� ��, ���� ������ �߰��� ���Ⱚ�� ���Ѵ�.
	int dirX = (*wormBodies)[length - 2].posX - (*wormBodies)[length - 1].posX;
	int dirY = (*wormBodies)[length - 2].posY - (*wormBodies)[length - 1].posY;

	// ���̸� ���� �� �� ���� ���̸� �ø���, ���� ���Ҵ�
	++(*eatCnt);
	++length;
	// realloc�� ���� ���Ҵ� ��, ������ �Ҵ��� �����ּҰ� ������� �����Ƿ�
	// ���������� ���Ҵ��� ������ �����ּҸ� �ޱ� ����, �Ķ���ͷ� Actor**�� ���
	*wormBodies = (Actor*)realloc(*wormBodies, sizeof(Actor) * length);

	// ���� �߰��� ����, ������ ����
	(*wormBodies)[length - 1].type = WormBody;
	(*wormBodies)[length - 1].color = WHITE;
	(*wormBodies)[length - 1].shape = "��";
	(*wormBodies)[length - 1].posX = (*wormBodies)[length - 2].posX - dirX;
	(*wormBodies)[length - 1].posY = (*wormBodies)[length - 2].posY - dirY;
}

void DrawWorm(Actor* wormBodies, int eatCnt)
{
	for (int i = 0; i < DefaultBodyCnt + eatCnt; ++i)
	{
		SetColor(wormBodies[i].color);
		SetPos(wormBodies[i].posX, wormBodies[i].posY);
		printf(wormBodies[i].shape);
	}
}

void DrawFood(Actor** foods)
{
	for (int i = 0; i < MaxFoodCnt; ++i)
	{
		if (foods[i] == NULL)
			continue;

		SetColor(foods[i]->color);
		SetPos(foods[i]->posX, foods[i]->posY);
		printf(foods[i]->shape);
	}
}

void DrawGameOver()
{
	SetColor(RED);
	SetPos(ConsoleSizeX / 2 - 9, ConsoleSizeY / 2);
	printf("@@@ Game Over @@@");
}
