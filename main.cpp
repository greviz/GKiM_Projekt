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
	window = SDL_CreateWindow(tytul, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_SHOWN);
	screen = SDL_GetWindowSurface(window);
	renderer = SDL_CreateRenderer(window, -1, 0);

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
		/* Pobieramy informacji ile bajtów zajmuje jeden pixel */
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
