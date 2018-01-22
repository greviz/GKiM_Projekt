#ifdef __cplusplus
#include <cstdlib>
#include <ctime>
#else
#include <stdlib.h>
#include <time.h>
#endif

#ifdef __APPLE__
#include <SDL/SDL.h>
#else
#include <SDL.h>
#endif

#include <iostream>
#include <cmath>
#include <fstream>
#include <vector>
using namespace std;


SDL_Window * window;
SDL_Surface *screen;
int width = 450;
int height = 300;
char const* tytul = "GKiM - Lab 1 - Nazwisko Imie";

SDL_Renderer * renderer;


int initSdl();

void Funkcja1();
void Funkcja2();
void Funkcja3();

void ladujBMP(char const* nazwa, int x, int y);

void czyscEkran(Uint8 R, Uint8 G, Uint8 B);

void zapiszBMP(char const * nazwa);

void wyzerujMaciez(double ** m, int x, int y);

double min(double x, double y);
double max(double x, double y);

void ladujButton(char const* nazwa, int x, int y);
void initButtons();
bool isMouseInButton(int bx, int by, int mx, int my);

void saveBitmapToFile();
void readBitmapFromFile();
vector<int> ByteRunCompress(vector<int> a, int length);
void ByteRunDecompress();

int main(int argc, char** argv)
{
	if (initSdl())
	{
		return -1;
	}

	// program main loop
	bool done = false;
	while (!done)
	{
		// message processing loop
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (SDL_QUIT == event.type)
			{
				done = true;
			}
			else if (SDL_MOUSEBUTTONDOWN == event.type)
			{
				if (isMouseInButton(450, 0, event.button.x, event.button.y))
					Funkcja1();
				if (isMouseInButton(450, 60, event.button.x, event.button.y))
					Funkcja2();
				if (isMouseInButton(450, 120, event.button.x, event.button.y))
					Funkcja3();
				if (isMouseInButton(450, 180, event.button.x, event.button.y))
					zapiszBMP("nowy.bmp"); cout << "Zapis pliku poprawny" << endl;
				if (isMouseInButton(450, 240, event.button.x, event.button.y))
					done = true;;
			}
			else if (SDL_KEYDOWN == event.type)
			{
				if (event.key.keysym.sym == SDLK_ESCAPE)
					done = true;
				if (event.key.keysym.sym == SDLK_1)
					Funkcja1();
				if (event.key.keysym.sym == SDLK_2)
					Funkcja2();
				if (event.key.keysym.sym == SDLK_3)
					Funkcja3();
				if (event.key.keysym.sym == SDLK_4)
					ByteRunDecompress();
				if (event.key.keysym.sym == SDLK_z)
					zapiszBMP("nowy.bmp");
				if (event.key.keysym.sym == SDLK_a)
					ladujBMP("obrazek1.bmp", 0, 0);
				if (event.key.keysym.sym == SDLK_s)
					ladujBMP("obrazek2.bmp", 0, 0);
				if (event.key.keysym.sym == SDLK_d)
					ladujBMP("obrazek3.bmp", 0, 0);
				if (event.key.keysym.sym == SDLK_b)
					czyscEkran(0, 0, 0);
			}
		} // end of message processing
	} // end main loop
}

int initSdl()
{
	// initialize SDL video
	if (SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("Unable to init SDL: %s\n", SDL_GetError());
		return 1;
	}

	// make sure SDL cleans up before exit
	atexit(SDL_Quit);

	const int bitDepth = 32;

	// create a new window
	window = SDL_CreateWindow(tytul, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width + 125, height, SDL_WINDOW_SHOWN);
	screen = SDL_GetWindowSurface(window);
	renderer = SDL_CreateRenderer(window, -1, 0);

	initButtons();


	if (!screen)
	{
		printf("Unable to set video: %s\n", SDL_GetError());
		return 1;
	}


	return 0;
}


Uint8* getPixelAddress(int x, int y)
{
	if ((x >= 0) && (x<width) && (y >= 0) && (y<height))
	{
		/* Pobieramy informacji ile bajt�w zajmuje jeden pixel */
		const int bpp = screen->format->BytesPerPixel;

		/* Obliczamy adres pixela */
		return (Uint8*)screen->pixels + y * screen->pitch + x * bpp;
	}
	return NULL;
}

SDL_Color getPixel(int x, int y)
{
	Uint8* pixelAddress = getPixelAddress(x, y);

	SDL_Color color = {};
	if (pixelAddress)
	{
		Uint32 col = 0;

		memcpy(&col, pixelAddress, screen->format->BytesPerPixel);
		SDL_GetRGB(col, screen->format, &color.r, &color.g, &color.b);
	}

	return color;
}

void setPixel(int x, int y, Uint8 R, Uint8 G, Uint8 B)
{
	Uint8* pixelAddress = getPixelAddress(x, y);
	if (pixelAddress)
	{
		Uint32 pixel = SDL_MapRGB(screen->format, R, G, B);

		switch (screen->format->BytesPerPixel)
		{
		case 1: //8-bit
			*pixelAddress = pixel;
			break;

		case 2: //16-bit
			*(Uint16*)pixelAddress = pixel;
			break;

		case 3: //24-bit
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN) {
				pixelAddress[0] = (pixel >> 16) & 0xff;
				pixelAddress[1] = (pixel >> 8) & 0xff;
				pixelAddress[2] = pixel & 0xff;
			}
			else {
				pixelAddress[0] = pixel & 0xff;
				pixelAddress[1] = (pixel >> 8) & 0xff;
				pixelAddress[2] = (pixel >> 16) & 0xff;
			}
			break;
		case 4: //32-bit
			*(Uint32*)pixelAddress = pixel;
			break;
		}
	}
	/* update the screen (aka double buffering) */
}

void ladujBMP(char const* nazwa, int x, int y)
{
	SDL_Surface* bmp = SDL_LoadBMP(nazwa);
	if (!bmp)
	{
		printf("Unable to load bitmap: %s\n", SDL_GetError());
	}
	else
	{
		SDL_Rect dstrect;
		dstrect.x = x;
		dstrect.y = y;


		SDL_BlitSurface(bmp, 0, screen, &dstrect);

		SDL_UpdateWindowSurface(window);


		SDL_FreeSurface(bmp);
	}
}

void czyscEkran(Uint8 R, Uint8 G, Uint8 B)
{
	screen->w = 450;
	SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, R, G, B));
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
}


void Funkcja1()
{
	double wspolczynnikR, wspolczynnikG, wspolczynnikB;
	SDL_Color pomocniczy;

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			pomocniczy = getPixel(x, y);
			wspolczynnikR = round(static_cast<double>(pomocniczy.r) / 85) * 85;
			wspolczynnikG = round(static_cast<double>(pomocniczy.g) / 85) * 85;
			wspolczynnikB = round(static_cast<double>(pomocniczy.b) / 85) * 85;

			setPixel(x, y, wspolczynnikR, wspolczynnikG, wspolczynnikB);
		}
	}


	SDL_UpdateWindowSurface(window);
	saveBitmapToFile();
	czyscEkran(0,0,0);
}

void Funkcja2()
{
	double wspolczynnik;
	SDL_Color pomocniczy;

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			pomocniczy = getPixel(x, y);
			wspolczynnik = static_cast<double>(pomocniczy.r + pomocniczy.g + pomocniczy.b) / 3;
			wspolczynnik = round((wspolczynnik * 64) / 255);
			wspolczynnik = (wspolczynnik * 255) / 64;

			setPixel(x, y, wspolczynnik, wspolczynnik, wspolczynnik);
		}
	}

	SDL_UpdateWindowSurface(window);
}

void Funkcja3()
{
	double wspolczynnikR, wspolczynnikG, wspolczynnikB, najblizszyR, najblizszyG, najblizszyB, eR, eG, eB;
	double ** e_tab_r = new double*[width + 1];
	double ** e_tab_g = new double*[width + 1];
	double ** e_tab_b = new double*[width + 1];
	SDL_Color pomocniczy;

	for (int i = 0; i < width + 1; i++)
	{
		e_tab_r[i] = new double[height + 1];
		e_tab_g[i] = new double[height + 1];
		e_tab_b[i] = new double[height + 1];
	}

	wyzerujMaciez(e_tab_r, width + 1, height + 1);
	wyzerujMaciez(e_tab_g, width + 1, height + 1);
	wyzerujMaciez(e_tab_b, width + 1, height + 1);


	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			pomocniczy = getPixel(x, y);

			wspolczynnikR = (static_cast<double>(pomocniczy.r)) / 85 + e_tab_r[x][y];
			najblizszyR = min(3, round(wspolczynnikR));

			eR = wspolczynnikR - najblizszyR;
			najblizszyR = max(0, najblizszyR);

			wspolczynnikG = (static_cast<double>(pomocniczy.g)) / 85 + e_tab_g[x][y];
			najblizszyG = min(3, round(wspolczynnikG));

			eG = wspolczynnikG - najblizszyG;
			najblizszyG = max(0, najblizszyG);


			wspolczynnikB = (static_cast<double>(pomocniczy.b)) / 85 + e_tab_b[x][y];
			najblizszyB = min(3, round(wspolczynnikB));

			eB = wspolczynnikB - najblizszyB;
			najblizszyB = max(0, najblizszyB);


			e_tab_r[x][y + 1] = e_tab_r[x][y + 1] + 7.0 / 16 * eR;
			e_tab_g[x][y + 1] = e_tab_g[x][y + 1] + 7.0 / 16 * eG;
			e_tab_b[x][y + 1] = e_tab_b[x][y + 1] + 7.0 / 16 * eB;

			e_tab_r[x + 1][y - 1] = e_tab_r[x + 1][y - 1] + 3.0 / 16 * eR;
			e_tab_g[x + 1][y - 1] = e_tab_g[x + 1][y - 1] + 3.0 / 16 * eG;
			e_tab_b[x + 1][y - 1] = e_tab_b[x + 1][y - 1] + 3.0 / 16 * eB;

			e_tab_r[x + 1][y] = e_tab_r[x + 1][y] + 5.0 / 16 * eR;
			e_tab_g[x + 1][y] = e_tab_g[x + 1][y] + 5.0 / 16 * eG;
			e_tab_b[x + 1][y] = e_tab_b[x + 1][y] + 5.0 / 16 * eB;

			e_tab_r[x + 1][y + 1] = e_tab_r[x + 1][y + 1] + 1.0 / 16 * eR;
			e_tab_g[x + 1][y + 1] = e_tab_g[x + 1][y + 1] + 1.0 / 16 * eG;
			e_tab_b[x + 1][y + 1] = e_tab_b[x + 1][y + 1] + 1.0 / 16 * eB;

			setPixel(x, y, najblizszyR * 85, najblizszyG * 85, najblizszyB * 85);
		}
	}


	SDL_UpdateWindowSurface(window);
}

void zapiszBMP(char const * tytul)
{
	screen->w = 450; // zmieniam rozmiar obrazu dla zapisu ( obciecie przyciskow)
	if (SDL_SaveBMP(screen, tytul) < 0)
		cout << "Nie udalo sie zapisac BMP " << endl;
}


void wyzerujMaciez(double ** m, int x, int y)
{
	for (int i = 0; i < x; i++)
		for (int j = 0; j < y; j++)
			m[i][j] = 0;
}

double min(double x, double y)
{
	if (x <= y)
		return x;
	else
		return y;
}

double max(double x, double y)
{
	if (x >= y)
		return x;
	else
		return y;
}
void ladujButton(const char *nazwa, int x, int y)
{
	SDL_Surface* bmp = SDL_LoadBMP(nazwa);
	if (!bmp)
	{
		printf("Unable to load bitmap: %s\n", SDL_GetError());
	}
	else
	{
		SDL_Rect button;
		button.x = x;
		button.y = y;
		button.h = 25;
		button.w = 75;


		SDL_BlitSurface(bmp, 0, screen, &button);

		SDL_UpdateWindowSurface(window);

		SDL_FreeSurface(bmp);
	}
}
void initButtons()
{
	ladujButton("6BitColorButton.bmp", 450, 0);
	ladujButton("6BitGrayButton.bmp", 450, 60);
	ladujButton("DitheringButton.bmp", 450, 120);
	ladujButton("SaveButton.bmp", 450, 180);
	ladujButton("QuitButton.bmp", 450, 240);
}
bool isMouseInButton(int bx, int by, int mx, int my)
{
	if (mx < bx + 125 && mx > bx && my < by + 60 && my>by)
	{
		return true;
	}
	else
	{
		return false;
	}
}

void saveBitmapToFile()
{
	double wspolczynnikR, wspolczynnikG, wspolczynnikB;
	bool typKompresji, kolor, paleta;
	int colorAmount = 0;
	vector<int> pixels, compressedPixelsByteRun, compressedPixelsRLE;
	SDL_Color pomocniczy;
	ofstream file;
	file.open("bitmap.ggps");

	typKompresji = kolor = 0;
	paleta = 1;

	if (file.good())
	{
		
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
				pomocniczy = getPixel(x, y);
				wspolczynnikR = pomocniczy.r / 85;
				wspolczynnikG = pomocniczy.g / 85;
				wspolczynnikB = pomocniczy.b / 85;

				pixels.push_back(wspolczynnikR);
				pixels.push_back(wspolczynnikG);
				pixels.push_back(wspolczynnikB);
			}
		}

		compressedPixelsByteRun = ByteRunCompress(pixels, pixels.size());

		file << "s3g" << " ";
		file << 26 + 3 * 4 * colorAmount + 3 * 4 * width * height << " ";
		file << 10 + colorAmount << " ";
		file << width << " ";
		file << height << " ";
		file << typKompresji << " ";
		file << kolor << " ";
		file << paleta << " ";
		file << colorAmount << " ";
		file << 10 << " ";
		
		//tu jeszcze paleta kolor�w wleci

		for (int i = 0; i < compressedPixelsByteRun.size(); i++)
			file << compressedPixelsByteRun[i] << " ";

		//przygotuj jutro dekompresor z tego formatu

		file.close();
	}
}

void readBitmapFromFile()
{
	int wspolczynnikR, wspolczynnikG, wspolczynnikB;
	ifstream file;
	file.open("bitmap.ggps");

	if (file.good())
		for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{
					file >> wspolczynnikR;
					file >> wspolczynnikG;
					file >> wspolczynnikB;

					setPixel(x, y, wspolczynnikR * 85, wspolczynnikG * 85, wspolczynnikB * 85);
			}
		}
	file.close();

	SDL_UpdateWindowSurface(window);
}

vector<int> ByteRunCompress(vector<int> a, int length)
{
	vector<int> output;
	int i = 0;

	while (i < length)
	{
		//sekwencja powtarzajacych sie bajtow
		if ((i < length - 1) &&
			(a[i] == a[i + 1]))
		{
			//zmierz dlugosc sekwencji
			int j = 0;
			while ((i + j < length - 1) &&
				(a[i + j] == a[i + j + 1]) &&
				(j < 127))
			{
				j++;
			}
			//wypisz spakowana sekwencje
			output.push_back(-j);
			output.push_back(a[i + j]);
			//przesun wskaznik o dlugosc sekwencji
			i += (j + 1);
		}
		//sekwencja roznych bajtow
		else
		{
			//zmierz dlugosc sekwencji
			int j = 0;
			while ((i + j < length - 1) &&
				(a[i + j] != a[j + i + 1]) &&
				(j < 128))
			{
				j++;
			}
			//dodaj jeszcze koncowke
			if ((i + j == length - 1) &&
				(j < 128))
			{
				j++;
			}
			//wypisz spakowana sekwencje
			output.push_back((j - 1));
			for (int k = 0; k<j; k++)
			{
				output.push_back(a[i + k]);
							}
			//przesun wskaznik o dlugosc sekwencji
			i += j;
		}
	}
	return output;
}

void ByteRunDecompress()
{
	fstream in;
	string x;
	int temp;
	vector<int> decompressedData, a;
	int i = 0;
	in.open("compressedBitmap.ggps", ios::in);

	while (in.good())
	{
		in >> temp;
		a.push_back(temp);
	}

	//dopoki wszystkie bajty nie sa zdekompresowane
	while (i < a.size())
	{
		//kod pusty
		if (a[i] == -128)
		{
			i++;
		}
		//sekwencja powtarzajacych sie bajtow
		else if (a[i] < 0)
		{
			for (int j = 0; j<-(a[i] - 1); j++)
			{
				decompressedData.push_back((int)a[i + 1]);
			}
			i += 2;
		}
		//sekwencja roznych bajtow
		else
		{
			for (int j = 0; j<(a[i] + 1) && i + 1 + j < a.size(); j++)
			{
				decompressedData.push_back((int)a[i + 1 + j]);
			}
			i += a[i] + 2;
		}
	}

	in.close();
	i = 0;

		 for (int x = 0; x < width; x++)
		{
			for (int y = 0; y < height; y++)
			{

				setPixel(x, y, decompressedData[i] * 85, decompressedData[i+1] * 85, decompressedData[i+2] * 85);
				i += 3;
			}
		}

	SDL_UpdateWindowSurface(window); 

}