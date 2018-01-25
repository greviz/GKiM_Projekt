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
char const* tytul = "S3g Kompresor";
int typKompresji, kolor, paleta, dithering;
int counter;
bool isPressedColor, isPressedGray, isPressedDithering, isPressedDedicated, isPressedDefault;
char * fileDir;  // sciezka do pliku
vector<SDL_Color> Colors64; // tablica najcz�stszych 64 kolor�w <kolor>


int initSdl();

vector<int> Bit6Color();
vector<int> Bit6GreyScale();
vector<int> Bit6Dithering();
vector<int> Bit6Dedicated();

void Draw(vector<int> pixels, bool c, bool p, int w, int h, vector<SDL_Color> paleta);
void ladujBMP(char const* nazwa, int x, int y);
void czyscEkran(Uint8 R, Uint8 G, Uint8 B);
void zapiszBMP(char const * nazwa);
void wyzerujMaciez(double ** m, int x, int y);
double min(double x, double y);
double max(double x, double y);
void ladujButton(char const* nazwa, int x, int y);
void initButtons();
bool isMouseInButton(int by, int mx, int my);
void convertMsgBox();
void saveBitmapToFile();
void readBitmapFromFile();
vector<int> ByteRunCompress(vector<int> a, int length);
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

	isPressedColor = isPressedDedicated = isPressedDefault = isPressedDithering = isPressedGray = false;

	kolor = 0; //ustawienie defaultowych warto�ci dla kompresora, gdyby u�ytkownik nie wybra� w�asnych parametr�w
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
				//SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION,"File dropped on window",fileDir,window);
				ladujBMP(fileDir, 0, 0);
			}
			else if (SDL_MOUSEBUTTONDOWN == event.type)
			{
				if (isMouseInButton(0, event.button.x, event.button.y))
				{
					if (isPressedColor == true)
					{
						kolor = 0; isPressedColor = false;
						ladujButton("Buttons/6BitColorButton.bmp", width, 0);
					}
					else
					{
						kolor = 0; isPressedColor = true;
						ladujButton("Buttons/6BitColorButtonH.bmp", width, 0);
					}
				}
				if (isMouseInButton(60, event.button.x, event.button.y))
				{
					if (isPressedGray == true)
					{
						kolor = 0; isPressedGray = false;
						ladujButton("Buttons/6BitGrayButton.bmp", width, 60);
					}
					else
					{
						kolor = 1; isPressedGray = true;
						ladujButton("Buttons/6BitGrayButtonH.bmp", width, 60);
					}
				}
				if (isMouseInButton(120, event.button.x, event.button.y))
				{
					if (isPressedDithering == true)
					{
						dithering = 0; isPressedDithering = false;
						ladujButton("Buttons/DitheringButton.bmp", width, 120);
					}
					else
					{
						dithering = 1; isPressedDithering = true;
						ladujButton("Buttons/DitheringButtonH.bmp", width, 120);
					}
				}
				if (isMouseInButton(180, event.button.x, event.button.y))
				{
					if (isPressedDedicated == true)
					{
						paleta = 1; isPressedDedicated = false;
						ladujButton("Buttons/DedPalette.bmp", width, 180);
					}
					else
					{
						paleta = 0; isPressedDedicated = true;
						ladujButton("Buttons/DedPaletteH.bmp", width, 180);
					}
				}
				if (isMouseInButton(240, event.button.x, event.button.y))
				{
					if (isPressedDefault == true)
					{
						paleta = 1; isPressedDefault = false;
						ladujButton("Buttons/DefPalette.bmp", width, 240);
					}
					else
					{
						paleta = 1; isPressedDefault = true;
						ladujButton("Buttons/DefPaletteH.bmp", width, 240);
					}
				}
				if (counter >= 1)
				{
					ladujButton("Buttons/ConvertButton.bmp", 0, height);
					if (event.button.y > height && event.button.y < height + 60) //przycisk do konwersji pojawi si� dopiero po wybraniu jakiego� parametru
					{
						saveBitmapToFile();
						convertMsgBox();
					}
				}
			}
			else if (SDL_KEYDOWN == event.type)
			{
				if (event.key.keysym.sym == SDLK_ESCAPE)
					done = true;

				if (event.key.keysym.sym == SDLK_1) //przycisk aktywuj�cy funkcj� dekompresuj�c� w kompresorze �eby sprawdzi� czy kompresja zako�czy�a si� powodzeniem
				{
					readBitmapFromFile();
				}
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
	screen = SDL_GetWindowSurface(window); //inicjalizacja wszystkich obiekt�w potrzebnych do wy�wietlania obraz�w
	renderer = SDL_CreateRenderer(window, -1, 0);

	initButtons(); //inicjalizacja przycisk�w


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
		const int bpp = screen->format->BytesPerPixel;

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
		case 1: 
			*pixelAddress = pixel;
			break;

		case 2:
			*(Uint16*)pixelAddress = pixel;
			break;

		case 3:
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
		case 4:
			*(Uint32*)pixelAddress = pixel;
			break;
		}
	}
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

		width = bmp->w;
		height = bmp->h;
		SDL_SetWindowSize(window, width + 125, height + 60); //ustawia rozmiar okna zale�nie od rozmiaru za�adowanego pliku BMP
		screen = SDL_GetWindowSurface(window);
		initButtons();

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
	initButtons();
}

vector<int> Bit6Color() //funkcja do przetworzenia kolor�w na obrazku na 6bit
{
	vector<int> p;
	double wspolczynnikR, wspolczynnikG, wspolczynnikB;
	SDL_Color pomocniczy;

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			pomocniczy = getPixel(x, y);
			wspolczynnikR = round(static_cast<double>(pomocniczy.r) / 85);
			wspolczynnikG = round(static_cast<double>(pomocniczy.g) / 85);
			wspolczynnikB = round(static_cast<double>(pomocniczy.b) / 85);

			p.push_back(wspolczynnikR);
			p.push_back(wspolczynnikG);
			p.push_back(wspolczynnikB);
		}
	}
	return p;
}

vector<int> Bit6GreyScale() //funkcja do przetworzenia kolor�w na obrazku na 64 stopniowa skale szarosci
{
	vector<int> p;
	double wspolczynnik;
	SDL_Color pomocniczy;

	for (int x = 0; x < width; x++)
	{
		for (int y = 0; y < height; y++)
		{
			pomocniczy = getPixel(x, y);
			wspolczynnik = static_cast<double>(pomocniczy.r + pomocniczy.g + pomocniczy.b) / 3;
			wspolczynnik = round((wspolczynnik * 64) / 255);

			p.push_back(wspolczynnik);
		}
	}
	return p;
}

vector<int> Bit6Dithering() //funkcja przetwarzajaca kolory obrazku na 6bit ale z uzyciem ditheringu
{
	vector<int> p;
	double wspolczynnikR, wspolczynnikG, wspolczynnikB, najblizszyR, najblizszyG, najblizszyB, eR, eG, eB;
	double ** e_tab_r = new double*[width + 1];
	double ** e_tab_g = new double*[width + 1]; //3 tablice bledow dla trzech skladowych pixela
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

			p.push_back(najblizszyR);
			p.push_back(najblizszyG);
			p.push_back(najblizszyB);
		}
	}

	for (int i = 0; i < width + 1; i++)
	{
		delete e_tab_r[i];
		delete e_tab_g[i];
		delete e_tab_b[i];
	}

	delete e_tab_r;
	delete e_tab_g;
	delete e_tab_b;

	return p;
}

void zapiszBMP(char const * tytul)
{
	screen->w = width; // zmieniam rozmiar obrazu dla zapisu ( obciecie przyciskow)
	screen->h = height;
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
		button.h = 60;
		button.w = 125;


		SDL_BlitSurface(bmp, 0, screen, &button);

		SDL_UpdateWindowSurface(window);

		SDL_FreeSurface(bmp);
	}
}
void initButtons()
{
	ladujButton("Buttons/6BitColorButton.bmp", width, 0);
	ladujButton("Buttons/6BitGrayButton.bmp", width, 60);
	ladujButton("Buttons/DitheringButton.bmp", width, 120);
	ladujButton("Buttons/DedPalette.bmp", width, 180);
	ladujButton("Buttons/DefPalette.bmp", width, 240);
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
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "File converted succesfully from BMP to S3g!", fileDir, window);
	}
}
void saveBitmapToFile()
{
	vector<int> pixels, compressedPixelsByteRun, compressedPixelsRLE;
	ofstream file;
	file.open("converted_file.s3g"); //zapisuje skompresowany obraz do tego pliku

	typKompresji = 0;

	if (file.good())
	{

		if (kolor == 1) //zaleznie od wybranych przez uzytkownika parametrow dokonuje sie przetworzenie kolorw na obrazku
			pixels = Bit6GreyScale();
		else if (kolor == 0)
			if (dithering == 1)
				pixels = Bit6Dithering();
			else if (paleta == 0)
				pixels = Bit6Dedicated();
			else
				pixels = Bit6Color();

		compressedPixelsByteRun = ByteRunCompress(pixels, pixels.size());

		file << "s3g" << " "; //zapisanie danych do naglowka pliku
		file << pixels.size() << " ";
		file << 10 + Colors64.size() * 3 << " ";
		file << width << " ";
		file << height << " ";
		file << typKompresji << " ";
		file << kolor << " ";
		file << paleta << " ";
		file << Colors64.size() << " ";
		file << 10 << " ";

		for (int i = 0; i < Colors64.size(); i++) // zapisanie palety kolorow jesli zostala taka uzyta
			file << static_cast<int>(Colors64[i].r) << " " << static_cast<int>(Colors64[i].g) << " " << static_cast<int>(Colors64[i].b) << " ";


		for (int i = 0; i < compressedPixelsByteRun.size(); i++) // zapisanie skompresowanych danych o pixelach
			file << compressedPixelsByteRun[i] << " ";


		file.close();
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

		if (id_file == "s3g") //jesli id obrazka sie zgadza to dokonujemy dekompresji
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

			for (int i = 0; i < color_amount; i++) // zapisanie do vectora palety kolorow jesli istnieje
			{
				file >> pomocnicza; temp.r = pomocnicza;
				file >> pomocnicza; temp.g = pomocnicza;
				file >> pomocnicza; temp.b = pomocnicza;
				paleta.push_back(temp);
			}

			for (int i = 0; i < size_file; i++) // zapisanie do vectora skompresowanych danych o pixelach
			{
				file >> pomocnicza;
				compressedPixels.push_back(pomocnicza);
			}

			if (type_comp == 0) //zaleznie od typu kompresji dane o pixelach sa dekompresowane i zapisywane w oddzielnym vectorze
				pixels = ByteRunDecompress(compressedPixels);

			Draw(pixels, gray_scale, type_color, pic_width, pic_height, paleta); // zaleznie od parametrow zdekommpresowany obrazek jest rysowany w oknie
			SDL_SaveBMP(screen, "New.bmp"); // zapisanie zdekompresowanego obrazka z powrotem do pliku bmp

		}
		file.close();
	}
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
	std::map<SDL_Color, int> Colors; // mapa wszystkich kolor�w jakie wyst�puj� <kolor, liczba wyst�pie�>
	vector<int> p;
	const int nbOfColors = 64;  // mog� �atwo zmieni� ile najcz�stszych kolor� bior� pod uwag�

								// przejechanie po wszystkich pixelach w celu wyszukania wszystkich kolor�w
	for (int i = 0; i < width; ++i) {
		for (int j = 0; j < height; ++j) {
			SDL_Color currentColor = getPixel(i, j);

			auto search = Colors.find(currentColor); // szukam czy kolor ju� jest w mapie
			if (search != Colors.end()) search->second += 1; // je�eli jest to zwi�kszam jego warto�� wyst�pie�
			else Colors.emplace(currentColor, 1); // je�eli nie ma to dodaj� go
		}
	}

	// wyszukanie 64 najcz�stszych kolor�w
	for (int k = 0; k < nbOfColors; ++k) {
		int max = 0;  // zmienna do wyszukiwania koloru o maksymalnej ilo�ci wyst�pie�
		SDL_Color maxColor; // kolor kt�ry aktualnie ma max ilo�� wyst�pie� spo�r�d wszystkich kolor�w w mapie

							// przelatuj� wszystkie kolory �eby znale�� max
		for (auto &c : Colors) {
			if (c.second > max) {
				max = c.second;
				maxColor = c.first;
			}
		}

		Colors64.push_back(maxColor);  // wrzucam max do wektora
		Colors.erase(maxColor); // i usuwam go z mapy

								// je�li mapa jest ju� pusta bo np. obrazek ma mniej ni� 64 kolory to wychodz� z p�tli
		if (Colors.empty()) break;
	}

	// zamiana kolor�w na ekranie na te 64 najcz�ciej wyst�puj�ce
	for (int x = 0; x < width; ++x) {
		for (int y = 0; y < height; ++y) {
			SDL_Color currentColor = getPixel(x, y);  // kolor aktualnie rozpatrywanego pixela

			int bestColorID = 0;  // pozycja w wektorze Colors64 najbli�szego koloru, tego kt�ry b�d� szuka�
			double currAvDiff;  // aktualna �rednia r�nica pomi�dzy kolorami
			double minAvDiff = 255.0;  //  minimalne �rednia r�nica pomi�dzy kolorami
			double rDiff, gDiff, bDiff;  // r�nice pomi�dzy sk�adowymi

										 // przelatuj� ca�y wektor w poszukiwaniu najbli�szego koloru
			for (int i = 0; i < Colors64.size(); ++i) {
				// dla ka�dego koloru z wektora wyliczam r�nic� pomi�dzy nim a currentColor
				rDiff = abs(currentColor.r - Colors64[i].r);
				gDiff = abs(currentColor.g - Colors64[i].g);
				bDiff = abs(currentColor.b - Colors64[i].b);
				currAvDiff = (rDiff + gDiff + bDiff) / 3.0; // obliczam �redni� aktualn� r�nic�
															// je�li jest mniejsza to ustawiam j� jako minimaln� i zapami�tuj� pozycj� tego koloru w wektorze
				if (currAvDiff < minAvDiff) {
					minAvDiff = currAvDiff;
					bestColorID = i;
				}
			}

			// ustawiam kolor na znaleziony najbli�szy
			p.push_back(bestColorID);
		}
	}
	return p;
}

void Draw(vector<int> pixels, bool c, bool p, int w, int h, vector<SDL_Color> paleta)
{
	int i = 0;
	if (c == 1) //rysowanie obrazka dla skali szarosci
	{
		for (int x = 0; x < w; x++)
			for (int y = 0; y < h; y++)
			{
				setPixel(x, y, pixels[i] * 255 / 64, pixels[i] * 255 / 64, pixels[i] * 255 / 64);
				i++;
			}
	}
	else if (p == 1) //rysowanie obrazka dla palety defaultowej
	{
		for (int x = 0; x < w; x++)
			for (int y = 0; y < h; y++)
			{
				setPixel(x, y, pixels[i] * 85, pixels[i + 1] * 85, pixels[i + 2] * 85);
				i += 3;
			}
	}
	else if (p == 0) //rysowanie obrazka dla palety dedykowanej
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