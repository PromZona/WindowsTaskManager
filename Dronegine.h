#include <windows.h>
#include <thread>
#include <chrono>

using namespace std;

enum COLOUR
{
	FG_BLACK = 0x0000,
	FG_DARK_BLUE = 0x0001,
	FG_DARK_GREEN = 0x0002,
	FG_DARK_CYAN = 0x0003,
	FG_DARK_RED = 0x0004,
	FG_DARK_MAGENTA = 0x0005,
	FG_DARK_YELLOW = 0x0006,
	FG_GREY = 0x0007,
	FG_DARK_GREY = 0x0008,
	FG_BLUE = 0x0009,
	FG_GREEN = 0x000A,
	FG_CYAN = 0x000B,
	FG_RED = 0x000C,
	FG_MAGENTA = 0x000D,
	FG_YELLOW = 0x000E,
	FG_WHITE = 0x000F,
	BG_BLACK = 0x0000,
	BG_DARK_BLUE = 0x0010,
	BG_DARK_GREEN = 0x0020,
	BG_DARK_CYAN = 0x0030,
	BG_DARK_RED = 0x0040,
	BG_DARK_MAGENTA = 0x0050,
	BG_DARK_YELLOW = 0x0060,
	BG_GREY = 0x0070,
	BG_DARK_GREY = 0x0080,
	BG_BLUE = 0x0090,
	BG_GREEN = 0x00A0,
	BG_CYAN = 0x00B0,
	BG_RED = 0x00C0,
	BG_MAGENTA = 0x00D0,
	BG_YELLOW = 0x00E0,
	BG_WHITE = 0x00F0,
};

class Dronegine
{
public:
	Dronegine()
	{
		SizeX = 80;
		SizeY = 50;

		hConsoleOut = GetStdHandle(STD_OUTPUT_HANDLE);
		hConsoleIn = GetStdHandle(STD_INPUT_HANDLE);

		Attribute = FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED;
	}

	~Dronegine()
	{
		delete[] ScreenBuffer;
	}

	int ConstructConsole(int ConsoleSizeX, int ConsoleSizeY)
	{
		SizeX = ConsoleSizeX;
		SizeY = ConsoleSizeY;

		SMALL_RECT zeroWindow = {0, 0, 0, 0};

		SetConsoleWindowInfo(hConsoleOut, TRUE, &zeroWindow);
		SetConsoleScreenBufferSize(hConsoleOut, {1, 1});

		rectWindow = {0, 0, (short)SizeX - 1, (short)SizeY - 1};

		if (!SetConsoleScreenBufferSize(hConsoleOut, {(short)SizeX, (short)SizeY}))
			return -2;
		if (!SetConsoleWindowInfo(hConsoleOut, TRUE, &rectWindow))
			return -1;

		ScreenBuffer = new CHAR_INFO[SizeX * SizeY];
		NullingScreenBuffer();
		return 1;
	}

	int DrawCube(int Xpos, int Ypos, int Size, char symbol = 'X', short color = FG_WHITE)
	{
		if (Xpos >= 0 && Ypos >= 0 && Xpos <= SizeX && Ypos <= SizeY && Xpos + Size <= SizeX && Ypos + Size <= SizeY)
		{
			for (int x = Xpos; x < Xpos + Size; x++)
				for (int y = Ypos; y < Ypos + Size; y++)
					Draw(x, y, symbol, color);
		}
		else
			return -2;

		return 1;
	}

	int Draw(int Xpos, int Ypos, char symbol = 'X', short color = FG_WHITE)
	{
		if (Xpos >= 0 && Ypos >= 0 && Xpos <= SizeX && Ypos <= SizeY)
		{
			ScreenBuffer[Ypos * SizeX + Xpos].Char.AsciiChar = symbol;
			ScreenBuffer[Ypos * SizeX + Xpos].Attributes = color;
		}
		return 1;
	}

	int Render()
	{
		if (!WriteConsoleOutput(hConsoleOut, ScreenBuffer, {(short)SizeX, (short)SizeY}, {0, 0}, &rectWindow))
		{
			return -2;
		}
		NullingScreenBuffer();
		return 1;
	}

	int Start()
	{
		GameIsActive = true;
		thread game = thread(&Dronegine::GameThread, this);

		game.join();

		return 1;
	}

	int GetHeight()
	{
		return SizeY;
	}

	int GetWidth()
	{
		return SizeX;
	}

	void WriteString(int Xpos, int Ypos, string TextYouWant, short color = FG_WHITE)
	{
		if (Xpos >= 0 && Ypos >= 0 && Xpos <= SizeX && Ypos <= SizeY && Xpos + TextYouWant.size() <= SizeX)
		{
			for (int i = 0; i < TextYouWant.size(); i++)
			{
				ScreenBuffer[Ypos * SizeX + Xpos + i].Char.AsciiChar = TextYouWant[i];
				ScreenBuffer[Ypos * SizeX + Xpos + i].Attributes = color;
			}
		}
	}

	void WriteString(int Xpos, int Ypos, TCHAR* TextYouWant, int textSize, short color = FG_WHITE)
	{
		if (Xpos >= 0 && Ypos >= 0 && Xpos <= SizeX && Ypos <= SizeY && Xpos + textSize <= SizeX)
		{
			for (int i = 0; i < textSize; i++)
			{
				ScreenBuffer[Ypos * SizeX + Xpos + i].Char.AsciiChar = TextYouWant[i];
				ScreenBuffer[Ypos * SizeX + Xpos + i].Attributes = color;
			}
		}
	}

	void FillXByText(int Xpos, int Ypos, char symbol, int count, short color = FG_WHITE)
	{
		if (Xpos >= 0 && Ypos >= 0 && Xpos <= SizeX && Ypos <= SizeY && Xpos + count <= SizeX)
		{
			for (int i = 0; i < count; i++)
			{
				ScreenBuffer[Ypos * SizeX + Xpos + i].Char.AsciiChar = symbol;
				ScreenBuffer[Ypos * SizeX + Xpos + i].Attributes = color;
			}
		}
	}

	virtual bool Update(float fElapsedTime) = 0;

private:
	void NullingScreenBuffer()
	{
		for (int i = 0; i < SizeX * SizeY; i++)
		{
			ScreenBuffer[i].Char.AsciiChar = ' ';
			ScreenBuffer[i].Attributes = BG_BLACK;
		}
	}

	void GameThread()
	{
		auto time1 = chrono::system_clock::now();
		auto time2 = chrono::system_clock::now();
		float rendertimer = 0;

		while (GameIsActive)
		{
			time2 = chrono::system_clock::now();
			chrono::duration<float> elapsedTime = time2 - time1;
			time1 = time2;

			Input();

			float fElapsedTime = elapsedTime.count();
			time_t t1 = chrono::system_clock::to_time_t(time2);
			int TimeNow = static_cast<int>(t1);

			if (!Update(fElapsedTime))
				GameIsActive = false;
			Render();

			Sleep(16.666f - fElapsedTime);
		}
	}

	void Input()
	{
		for (int i = 0; i < 256; i++)
		{
			m_keyNewState[i] = GetAsyncKeyState(i);

			m_keys[i].bPressed = false;
			m_keys[i].bReleased = false;

			if (m_keyNewState[i] != m_keyOldState[i])
			{
				if (m_keyNewState[i] & 0x8000)
				{
					m_keys[i].bPressed = !m_keys[i].bHeld;
					m_keys[i].bHeld = true;
				}
				else
				{
					m_keys[i].bReleased = true;
					m_keys[i].bHeld = false;
				}
			}

			m_keyOldState[i] = m_keyNewState[i];
		}
	}

private:
	HANDLE hConsoleOut;
	HANDLE hConsoleIn;
	int SizeX;
	int SizeY;
	CHAR_INFO *ScreenBuffer;
	WORD Attribute;
	SMALL_RECT rectWindow;
	bool GameIsActive;

protected:
	short m_keyOldState[256] = {0};
	short m_keyNewState[256] = {0};
	struct sKeyState
	{
		bool bPressed;
		bool bReleased;
		bool bHeld;
	} m_keys[256];
};
