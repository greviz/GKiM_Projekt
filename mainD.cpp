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
#include <chrono>
#include <map>
#include <algorithm>
#include <string>
using namespace std;

SDL_Window * window;
SDL_Surface *screen;
SDL_Renderer * renderer;
int width = 450;
int height = 300;
char const* tytul = "S3g Dekompresor";
int typKompresji, kolor, paleta, dithering;
int counter;
char * fileDir;  // sciezka do pliku
vector<SDL_Color> Colors64; // tablica najczêstszych 64 kolorów <kolor>


int initSdl();

void Draw(vector<int> pixels, bool c, bool p, int w, int h, vector<SDL_Color> paleta);
void czyscEkran(Uint8 R, Uint8 G, Uint8 B);
void zapiszBMP(char const * nazwa);
void ladujButton(char const* nazwa, int x, int y);
void initButtons();
bool isMouseInButton(int by, int mx, int my);
void convertMsgBox();
void readBitmapFromFile();
vector<int> ByteRunDecompress(vector<int>);

bool operator ==(const SDL_Color &c1, const SDL_Color &c2)
{
	return c1.r == c2.r && c1.g == c2.g && c1.b == c2.b;
}

bool operator  <(const SDL_Color &c1, const SDL_Color &c2)
{
	if (c1.r == c2.r) {
		if (c1.g == c2.g) {
			return c1.b < c2.b;
		}
		else return c1.g < c2.g;
	}
	else return c1.r < c2.r;
}

int main(int argc, char** argv)
{
	if (initSdl())
	{
		return -1;
	}

	kolor = 0;
	paleta = 1;
	dithering = 0;
	counter = 0;

	bool done = false;
	while (!done)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (SDL_QUIT == event.type)
			{
				done = true;
			}
			else if (SDL_DROPFILE == event.type)
			{
				fileDir = event.drop.file;
				counter++;
				ladujButton("Buttons/ConvertButton.bmp", 0, height);
				SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,"File dropped on window succesfuly",fileDir,window);	
			}
			else if (SDL_MOUSEBUTTONDOWN == event.type)
			{
				if (counter >= 1)
				{
					if (event.button.y > height && event.button.y < height + 60)
					{
						readBitmapFromFile();
						zapiszBMP("converted.bmp");
						convertMsgBox();
					}
				}
			}
			else if (SDL_KEYDOWN == event.type)
			{
				if (event.key.keysym.sym == SDLK_ESCAPE)
					done = true;
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
	window = SDL_CreateWindow(tytul, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width + 125, height + 60, SDL_WINDOW_SHOWN); // dodaje w i h buttonow 
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

void czyscEkran(Uint8 R, Uint8 G, Uint8 B)
{
	SDL_FillRect(screen, 0, SDL_MapRGB(screen->format, R, G, B));
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
	SDL_RenderClear(renderer);
	SDL_RenderPresent(renderer);
	initButtons();
}

void zapiszBMP(char const * tytul)
{
	screen->w = width; // zmieniam rozmiar obrazu dla zapisu ( obciecie przyciskow)
	screen->h = height;
	if (SDL_SaveBMP(screen, tytul) < 0)
		cout << "Nie udalo sie zapisac BMP " << endl;
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
		button.h = 60;
		button.w = 125;


		SDL_BlitSurface(bmp, 0, screen, &button);

		SDL_UpdateWindowSurface(window);

		SDL_FreeSurface(bmp);
	}
}
void initButtons()
{
	ladujButton("Buttons/DirectoryButton.bmp", 0, height);
}
bool isMouseInButton(int by, int mx, int my)
{
	if (mx < width + 125 && mx > width && my < by + 60 && my>by)
	{
		return true;
	}
	else
	{
		return false;
	}
}
void convertMsgBox()
{
	string s = fileDir;
	if (s == "")
	{
		char err[30] = "FILE NOT LOADED!";
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "CRITICAL ERROR!", err, window);
	}
	else
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "File converted succesfully from S3g to BMP!", fileDir, window);
	}
}
void readBitmapFromFile()
{
	int color_amount, size_file, offset_file, pic_width, pic_height, type_comp, gray_scale, type_color, offset_color, pomocnicza;
	string id_file;
	vector<int> pixels, compressedPixels;
	vector<SDL_Color> paleta;
	SDL_Color temp;
	ifstream file;
	file.open(fileDir);


	if (file.good())
	{
		file >> id_file;

		if (id_file == "s3g")
		{
			file >> size_file;
			file >> offset_file;
			file >> pic_width;
			file >> pic_height;
			file >> type_comp;
			file >> gray_scale;
			file >> type_color;
			file >> color_amount;
			file >> offset_color;

			width = pic_width;
			height = pic_height;
			SDL_SetWindowSize(window, width, height);
			screen = SDL_GetWindowSurface(window);

			for (int i = 0; i < color_amount; i++)
			{
				file >> pomocnicza; temp.r = pomocnicza;
				file >> pomocnicza; temp.g = pomocnicza;
				file >> pomocnicza; temp.b = pomocnicza;
				paleta.push_back(temp);
			}

			for (int i = 0; i < size_file; i++)
			{
				file >> pomocnicza;
				compressedPixels.push_back(pomocnicza);
			}

			if (type_comp == 0)
				pixels = ByteRunDecompress(compressedPixels);

			Draw(pixels, gray_scale, type_color, pic_width, pic_height, paleta);
		}
		file.close();
	}
}

vector<int> ByteRunDecompress(vector<int> a)
{
	fstream in;
	string x;
	int temp;
	vector<int> decompressedData;
	int i = 0;
	in.open("converted_file.s3g", ios::in);

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

	return decompressedData;
}

vector<int> Bit6Dedicated()
{
	std::map<SDL_Color, int> Colors; // mapa wszystkich kolorów jakie wystêpuj¹ <kolor, liczba wyst¹pieñ>
	vector<int> p;
	const int nbOfColors = 64;  // mogê ³atwo zmieniæ ile najczêstszych koloró biorê pod uwagê

	// przejechanie po wszystkich pixelach w celu wyszukania wszystkich kolorów
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			SDL_Color currentColor = getPixel(i, j);

			auto search = Colors.find(currentColor); // szukam czy kolor ju¿ jest w mapie
			if (search != Colors.end()) search->second += 1; // je¿eli jest to zwiêkszam jego wartoœæ wyst¹pieñ
			else Colors.emplace(currentColor, 1); // je¿eli nie ma to dodajê go
		}
	}

	// wyszukanie 64 najczêstszych kolorów
	for (int k = 0; k < nbOfColors; ++k) {
		int max = 0;  // zmienna do wyszukiwania koloru o maksymalnej iloœci wyst¹pieñ
		SDL_Color maxColor; // kolor który aktualnie ma max iloœæ wyst¹pieñ spoœród wszystkich kolorów w mapie

		// przelatujê wszystkie kolory ¿eby znaleŸæ max
		for (auto &c : Colors) {
			if (c.second > max) {
				max = c.second;
				maxColor = c.first;
			}
		}

		Colors64.push_back(maxColor);  // wrzucam max do wektora
		Colors.erase(maxColor); // i usuwam go z mapy

		// jeœli mapa jest ju¿ pusta bo np. obrazek ma mniej ni¿ 64 kolory to wychodzê z pêtli
		if (Colors.empty()) break;
	}

	// zamiana kolorów na ekranie na te 64 najczêœciej wystêpuj¹ce
	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			SDL_Color currentColor = getPixel(x, y);  // kolor aktualnie rozpatrywanego pixela

			int bestColorID = 0;  // pozycja w wektorze Colors64 najbli¿szego koloru, tego który bêdê szukaæ
			double currAvDiff;  // aktualna œrednia ró¿nica pomiêdzy kolorami
			double minAvDiff = 255.0;  //  minimalne œrednia ró¿nica pomiêdzy kolorami
			double rDiff, gDiff, bDiff;  // ró¿nice pomiêdzy sk³adowymi

			// przelatujê ca³y wektor w poszukiwaniu najbli¿szego koloru
			for (int i = 0; i < Colors64.size(); ++i) {
				// dla ka¿dego koloru z wektora wyliczam ró¿nicê pomiêdzy nim a currentColor
				rDiff = abs(currentColor.r - Colors64[i].r);
				gDiff = abs(currentColor.g - Colors64[i].g);
				bDiff = abs(currentColor.b - Colors64[i].b);
				currAvDiff = (rDiff + gDiff + bDiff) / 3.0; // obliczam œredni¹ aktualn¹ ró¿nicê
				// jeœli jest mniejsza to ustawiam j¹ jako minimaln¹ i zapamiêtujê pozycjê tego koloru w wektorze
				if (currAvDiff < minAvDiff) {
					minAvDiff = currAvDiff;
					bestColorID = i;
				}
			}

			// ustawiam kolor na znaleziony najbli¿szy
			p.push_back(bestColorID);
		}
	}
	return p;
}

void Draw(vector<int> pixels, bool c, bool p, int w, int h, vector<SDL_Color> paleta)
{
	int i = 0;
	if (c == 1)
	{
		for (int x = 0; x < w; x++)
		for (int y = 0; y < h; y++)
		{
			setPixel(x, y, pixels[i] * 255 / 64, pixels[i] * 255 / 64, pixels[i] * 255 / 64);
			i++;
		}
	}
	else if (p == 1)
	{
		for (int x = 0; x < w; x++)
		for (int y = 0; y < h; y++)
		{
			setPixel(x, y, pixels[i] * 85, pixels[i + 1] * 85, pixels[i + 2] * 85);
			i += 3;
		}
	}
	else if (p == 0)
	{
		for (int x = 0; x < w; x++)
		for (int y = 0; y < h; y++)
		{
			setPixel(x, y, paleta[pixels[i]].r, paleta[pixels[i]].g, paleta[pixels[i]].b);
			i++;
		}
	}
	SDL_UpdateWindowSurface(window);
}