#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <conio.h>
#include <math.h>

#pragma region 게임 데이터
// 사용하는 콘솔의 x,y 사이즈
int ConsoleSizeX = 120;
int ConsoleSizeY = 30;
// 플레이하는 영역의 x,y 최소 값
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

// 게임 내 움직이는 액터들의 타입을 열거
typedef enum ActorType
{
	WormHead,
	WormBody,
	Food,
	End,
} ActorType;

// 액터들이 공통적으로 사용하는 데이터를 구조체화
typedef struct Actor
{
	ActorType type; // 어떤 액터인지 구분할 용도
	Color color; // 액터의 색상을 나타낼 용도
	char* shape; // 액터의 모양을 나타낼 용도
	int posX;  // 액터의 화면 상의 좌표 x,y
	int posY;
} Actor;

// 유저 입력에 따른 데이터를 나타낼 구조체
typedef struct Input
{
	int dirX;
	int dirY;
}Input;

// 지렁이의 기본 몸통 길이
const int DefaultBodyCnt = 3;
// 맵에 존재할 수 있는 최대 먹이 수
const int MaxFoodCnt = 3;
// 먹이가 스폰되는 간격 (밀리초)
const ULONGLONG SpawnFoodInterval = 1000;
// 특문으로 액터 그릴 때, x축으로 한 칸 출력 시 사용되는 길이
const int SpaceCnt = 2;
#pragma endregion

void InitConsole(); // 게임에 맞게끔 콘솔을 초기화하는 기능
void SetPos(int x, int y); // 콘솔 커서의 위치를 파라미터로 받은 x, y로 설정하는 기능
void SetColor(Color color); // 콘솔 출력 색상을 변경하는 기능

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

	// 콘솔 초기화 실행
	InitConsole();

	// 게임 로직 반복 여부
	int isRun = 1;
	// 몸체와 머리가 부딪혔는지 여부
	int isCollBodies = 0;
	
	// 현재까지 먹은 먹이 수
	int eatCnt = 0;
	// 이전에 먹이가 스폰된 시간
	ULONGLONG prevSpawnFoodTime = GetTickCount64();

	// 유저 입력 데이터 선언
	Input input;
	
	// 유저가 조작할 지렁이 정보 생성
	Actor* wormBodies = CreateWorm(ConsoleSizeX / 2, ConsoleSizeY / 2, DefaultBodyCnt);
	// 먹이 정보들을 들고 있을 변수
	Actor** foods = (Actor**)malloc(sizeof(Actor*) * MaxFoodCnt);
	for (int i = 0; i < MaxFoodCnt; ++i)
		foods[i] = NULL;

	// 게임 로직 반복
	while (isRun)
	{
		system("cls");

		// 머리가 몸통과 충돌하지 않았다면
		//  연산
		//  1. 키 입력
		//  2. 이동 (머리 충돌)
		//  3. 푸드 스폰 로직
		//  4. 먹이와의 충돌 체크
		if (!isCollBodies)
		{
			InputKey(&input);
			isCollBodies = MoveWorm(&input, wormBodies, eatCnt);
			SpawnFood(foods, &prevSpawnFoodTime);
			CheckColl(&wormBodies, foods, &eatCnt);
		}

		// 그리기
		// 지렁이 그리기
		DrawWorm(wormBodies, eatCnt);
		// 먹이 그리기
		DrawFood(foods);

		// 머리와 몸통이 충돌했다면, 반복 탈출
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
	// 콘솔 커서를 안 보이게 숨김
	CONSOLE_CURSOR_INFO cursorInfo; // 콘솔 커서 정보를 나타내는 구조체
	cursorInfo.dwSize = 1; // 커서 굵기 (1~100)
	cursorInfo.bVisible = FALSE; // 커서 가시화 여부

	// 위의 만든 커서 정보를 현재 버퍼에 적용
	SetConsoleCursorInfo(GetStdHandle(STD_OUTPUT_HANDLE), &cursorInfo);
}

void SetPos(int x, int y)
{
	COORD pos = { x - 1, y - 1 };
	SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

void SetColor(Color color)
{
	// 색상을 변경하면, 다음 번 색상 설정까지 변경했던 색상으로 계속 출력 됨
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

Actor* CreateWorm(int posX, int posY, int length)
{
	// 벌레 기본 몸통 길이만큼 최초 할당
	Actor* wormBodies = (Actor*)malloc(sizeof(Actor) * length);

	// 몸통에 대한 데이터 설정
	for (int i = 0; i < length; ++i)
	{
		wormBodies[i].type = WormBody;
		wormBodies[i].color = WHITE;
		wormBodies[i].shape = "■";
	}
	// 머리에 대한 데이터 설정
	wormBodies[0].type = WormHead;
	wormBodies[0].color = GREEN;
	// 파라미터로 받은 위치 값은 머리의 초기 위치
	wormBodies[0].posX = posX;
	wormBodies[0].posY = posY;

	// 설정된 머리 위치를 기반으로 임의로 나머지 몸통들의 위치를 설정
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
	food->shape = "♥";
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
	// 이동할 수 있는지 여부
	int canMove = 1;

	// x축 이동을 해야하는지?
	if (input->dirX != 0)
	{
		// 방향 데이터에 따른 다음 이동 위치를 연산한다.
		int nextPosX = wormBodies[0].posX + input->dirX * SpaceCnt;

		// 다음 위치가 플레이 영역을 벗어나는지 확인
		if (nextPosX < PlayAreaMinX || nextPosX >= ConsoleSizeX)
			canMove = 0;
		else
		{
			// 플레이 영역에 안에 다음위치가 존재
			for (int i = 1; i < DefaultBodyCnt + eatCnt; ++i)
			{
				// 머리와 몸통이 충돌하는지 확인
				if (nextPosX == wormBodies[i].posX && wormBodies[0].posY == wormBodies[i].posY)
				{
					// 머리 바로 뒤쪽 몸통 방향으로는 이동하지 못하게
					if (i == 1)
						canMove = 0;
					else
					{
						// 머리와 뒤쪽의 몸통들이 충돌했다는 뜻 (게임오버 처리)
						wormBodies[0].color = RED;
						wormBodies[i].color = RED;
						return 1;
					}
				}
			}
		}

		// 이동할 수 있다면
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

	// y축 이동을 해야하는지?
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
	// 현재 시간을 가져옴
	ULONGLONG currentTime = GetTickCount64();

	// 이전 스폰 시간 + 스폰 간격 시간을 더한 값이
	// 현재 시간 보다 작다면
	// 스폰할 시간이 되었다는 뜻
	if (*prevSpawnFoodTime + SpawnFoodInterval < currentTime)
	{
		// 새로 푸드를 스폰할 것이므로
		// 이전 스폰 시간을 현재 시간으로 갱신
		*prevSpawnFoodTime = currentTime;

		// 스폰할 수 있는 최대 먹이 개수를 초과하지 않는지 확인
		for (int i = 0; i < MaxFoodCnt; ++i)
		{
			// 파라미터로 받은 푸드 배열에 빈 공간이 있다면
			if (foods[i] == NULL)
			{
				// 푸드를 하나 생성하고, 반복 중단
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
	// 복수의 먹이와 머리의 충돌을 확인
	for (int i = 0; i < MaxFoodCnt; ++i)
	{
		// 먹이가 존재하지 않는다면 다음 반복으로
		if (foods[i] == NULL)
			continue;

		// 먹이와 머리의 y 좌표가 다르다면 x좌표를 비교할 필요 없이 넘어감
		if ((*wormBodies)[0].posY != foods[i]->posY)
			continue;

		// 먹이와 머리의 거리가 SpaceCnt 보다 작다면 충돌 판정
		if (abs((*wormBodies)[0].posX - foods[i]->posX) < SpaceCnt)
		{
			// 먹이 디스폰 
			free(foods[i]);
			foods[i] = NULL;

			// 몸통 증가
			IncreaseBodies(wormBodies, eatCnt);
		}
	}
}

void IncreaseBodies(Actor** wormBodies, int* eatCnt)
{
	// 증가시키기 전, 몸통 길이
	int length = DefaultBodyCnt + (*eatCnt);

	// 맨 마지막 두 몸통 사이의 위치를 빼, 새로 몸통을 추가할 방향값을 구한다.
	int dirX = (*wormBodies)[length - 2].posX - (*wormBodies)[length - 1].posX;
	int dirY = (*wormBodies)[length - 2].posY - (*wormBodies)[length - 1].posY;

	// 먹이를 먹은 수 및 몸통 길이를 늘리고, 몸통 재할당
	++(*eatCnt);
	++length;
	// realloc을 통한 재할당 시, 기존에 할당한 시작주소가 보장되지 않으므로
	// 정상적으로 재할당한 공간의 시작주소를 받기 위해, 파라미터로 Actor**를 사용
	*wormBodies = (Actor*)realloc(*wormBodies, sizeof(Actor) * length);

	// 새로 추가된 몸통, 데이터 설정
	(*wormBodies)[length - 1].type = WormBody;
	(*wormBodies)[length - 1].color = WHITE;
	(*wormBodies)[length - 1].shape = "■";
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
