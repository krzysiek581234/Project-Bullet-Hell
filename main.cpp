#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<iostream>
#include<Windows.h>
extern "C"
{
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	960.0
#define SCREEN_HEIGHT	540.0

//The dimensions of the level
#define LEVEL_WIDTH 1920
#define LEVEL_HEIGHT 1080
#define PLAYER_SPEED 1
#define POZA_MAPA -100
#define ILOSC_ZYC 5
using namespace std;
struct Obrazek
{
	char* scierzka_zdjecie;


};

struct Pocisk
{
	int x, y;
	int obrazenia;
	int trajektoria;
	int predkosc;
	int x_poczatkowe;
	int y_poczatkowe;
	char kierunek;
	Pocisk(int arg_x, int arg_y, int arg_trajektoria, int arg_predkosc, int arg_obrazenia, int arg_x_poczatkowe, int arg_y_poczatkowe, char arg_kierunek)
	{
		this->x = arg_x;
		this->y = arg_y;
		this->trajektoria = arg_trajektoria;
		this->predkosc = arg_predkosc;
		this->obrazenia = arg_obrazenia;
		this->x_poczatkowe = arg_x_poczatkowe;
		this->y_poczatkowe = arg_y_poczatkowe;
		this->kierunek = arg_kierunek;
	}


};
struct Przeciwnik
{
	int x, y;
	int zdrowie;
	int predkosc;
	Przeciwnik(int arg_x, int arg_y, int arg_zdrowie, int arg_predkosc)
	{
		this->x = arg_x;
		this->y = arg_y;
		this->zdrowie = arg_zdrowie;
		this->predkosc = arg_predkosc;
	}
};

void DrawString(SDL_Surface* screen, int x, int y, const char* text,
	SDL_Surface* charset) {
	int px, py, c;
	SDL_Rect s, d;
	s.w = 8;
	s.h = 8;
	d.w = 8;
	d.h = 8;
	while (*text) {
		c = *text & 255;
		px = (c % 16) * 8;
		py = (c / 16) * 8;
		s.x = px;
		s.y = py;
		d.x = x;
		d.y = y;
		SDL_BlitSurface(charset, &s, screen, &d);
		x += 8;
		text++;
	};
};

void DrawSurface(SDL_Surface* screen, SDL_Surface* sprite, int x, int y) {
	SDL_Rect dest;
	//x - zmmienna okreslana z wzoru
	dest.x = x - sprite->w / 2;
	dest.y = y - sprite->h / 2;
	dest.w = sprite->w;
	dest.h = sprite->h;
	SDL_BlitSurface(sprite, NULL, screen, &dest);
};

void DrawPixel(SDL_Surface* surface, int x, int y, Uint32 color) {
	int bpp = surface->format->BytesPerPixel;
	Uint8* p = (Uint8*)surface->pixels + y * surface->pitch + x * bpp;
	*(Uint32*)p = color;
};
void DrawLine(SDL_Surface* screen, int x, int y, int l, int dx, int dy, Uint32 color) {
	for (int i = 0; i < l; i++) {
		DrawPixel(screen, x, y, color);
		x += dx;
		y += dy;
	};
};
void DrawRectangle(SDL_Surface* screen, int x, int y, int l, int k,
	Uint32 outlineColor, Uint32 fillColor) {
	int i;
	DrawLine(screen, x, y, k, 0, 1, outlineColor);
	DrawLine(screen, x + l - 1, y, k, 0, 1, outlineColor);
	DrawLine(screen, x, y, l, 1, 0, outlineColor);
	DrawLine(screen, x, y + k - 1, l, 1, 0, outlineColor);
	for (i = y + 1; i < y + k - 1; i++)
		DrawLine(screen, x + 1, i, l - 2, 1, 0, fillColor);
};

void Koniec(SDL_Surface* screen, SDL_Texture* scrtex, SDL_Window* window, SDL_Surface* charset, SDL_Renderer* renderer)
{
	SDL_FreeSurface(screen);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyWindow(window);
	SDL_DestroyRenderer(renderer);
	SDL_Quit();
}
void usuwanie_tla(SDL_Surface* obraz)
{
	uint32_t keyColor = SDL_MapRGB(obraz->format, 150, 150, 150);
	SDL_SetColorKey(obraz, SDL_TRUE, keyColor);
}
bool inicjalizacja_sdl(SDL_Surface*& charset, SDL_Surface*& screen, SDL_Window*& window, SDL_Renderer*& renderer, bool fullscreen)
{
	int rc;
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
	{
		printf("SDL_Init error: %s\n", SDL_GetError());
		return 1;
	}

	if (fullscreen)
	{
		// tryb pełnoekranowy / fullscreen mode
		rc = SDL_CreateWindowAndRenderer(0, 0, SDL_WINDOW_FULLSCREEN_DESKTOP, &window, &renderer);
	}
	else
	{
		// tryb okienkowy
		rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0, &window, &renderer);
	}
	if (rc != 0)
	{
		SDL_Quit();
		printf("SDL_CreateWindowAndRenderer error: %s\n", SDL_GetError());
		return 1;
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
	SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
	SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

	SDL_SetWindowTitle(window, "Pocisk piklo gra");

	screen = SDL_CreateRGBSurface(0, LEVEL_WIDTH, LEVEL_HEIGHT, 32, 0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);
	// wyłączenie widocznoœci kursora myszy
	SDL_ShowCursor(SDL_DISABLE);
	return 0;
}
bool czy_trafiony(int miejsce_pocisku_x, int miejsce_pocisku_y, int miejsce_przeciwnika_x, int miejsce_przeciwnika_y)
{
	if (miejsce_pocisku_x > miejsce_przeciwnika_x -20 && miejsce_pocisku_x < miejsce_przeciwnika_x + 20
		&& miejsce_pocisku_y > miejsce_przeciwnika_y - 20 && miejsce_pocisku_y < miejsce_przeciwnika_y + 20)
	{
		return true;
	}
	return false;
}
SDL_Surface* ladowanie_zdjecia(char* adress)
{
	SDL_Surface* image;
	image = SDL_LoadBMP(adress);
	if (image == NULL) {
		printf("SDL_LoadBMP error: %s\n", SDL_GetError());

		return image;
	};
	return image;
}
bool wczytywanie_mapy(SDL_Surface*& player, SDL_Surface*& charset, SDL_Surface*& zycie, SDL_Surface*& zycie_male , SDL_Window*& window, SDL_Renderer*& renderer, SDL_Texture*& scrtex, SDL_Surface*& screen, SDL_Surface*& tlo, SDL_Surface** tab_potwor, SDL_Surface** rodzaje_pociskow)
{
	// wczytanie obrazka cs8x8.bmp
	charset = ladowanie_zdjecia("./cs8x8.bmp");

	SDL_SetColorKey(charset, true, 0x000000);
	tab_potwor[0] = ladowanie_zdjecia("./oko.bmp");
	tab_potwor[1] = ladowanie_zdjecia("./Brain_of_Cthulhu.bmp");
	tab_potwor[2] = ladowanie_zdjecia("./Skeletron_Head.bmp");

	player = ladowanie_zdjecia("./player_50_z_pikselem.bmp");
	rodzaje_pociskow[0] = ladowanie_zdjecia("./pocisk.bmp");
	rodzaje_pociskow[1] = ladowanie_zdjecia("./demon_oko.bmp");
	rodzaje_pociskow[2] = ladowanie_zdjecia("./hixbox.bmp");
	rodzaje_pociskow[3] = ladowanie_zdjecia("./demon_oko.bmp");
	tlo = ladowanie_zdjecia("./tlo3.bmp");
	zycie = ladowanie_zdjecia("./serce.bmp");
	zycie_male = ladowanie_zdjecia("./serce_male.bmp");
	usuwanie_tla(player);
	usuwanie_tla(tab_potwor[0]);
	usuwanie_tla(tab_potwor[1]);
	usuwanie_tla(tab_potwor[2]);
	usuwanie_tla(rodzaje_pociskow[0]);
	usuwanie_tla(rodzaje_pociskow[1]);
	usuwanie_tla(rodzaje_pociskow[2]);
	usuwanie_tla(rodzaje_pociskow[3]);
	usuwanie_tla(zycie);
	usuwanie_tla(zycie_male);
	return 0;

}
void wczytanie_napisu(SDL_Surface*& screen, SDL_Surface*& charset, SDL_Texture*& scrtex, SDL_Renderer*& renderer, double fps, double czas_od_startu_levela, SDL_Rect camera, SDL_Rect gameScreen,int ilosc_trafien, int zycia)
{
	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	

	// tekst informacyjny / info text
	DrawRectangle(screen, 4 + camera.x, 4 + camera.y, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
	//            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
	sprintf_s(text, "Czas trwania = %.1lf s  %.0lf klatek / s , ilosc trafien %d, Zycia %d", (czas_od_startu_levela)/1.0, fps, ilosc_trafien, zycia);
	//sprintf(text, "cos");
	DrawString(screen, SCREEN_WIDTH / 2 - strlen(text) * 8 / 2 + camera.x, 10 + camera.y, text, charset);
	//	      "Esc - exit, \030 - faster, \031 - slower"
	sprintf_s(text, "Esc - wyjscie, \032  lewo  prawo \033 nowa gra (n)");
	DrawString(screen, SCREEN_WIDTH / 2 - strlen(text) * 8 / 2 + camera.x, 26 + camera.y, text, charset);

	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, scrtex, &camera, &gameScreen);
	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderPresent(renderer);


}
void pobierz_lokalizacje_przeciwnika(Przeciwnik** tablica_przeciwnikow,Pocisk** tablica_pociskow,int lvl, int i)
{
	if (tablica_pociskow[i]->x == POZA_MAPA && tablica_pociskow[i]->y == int(SCREEN_HEIGHT / 4))
	{

		tablica_pociskow[i]->x = tablica_przeciwnikow[lvl]->x;
		tablica_pociskow[i]->y = tablica_przeciwnikow[lvl]->y;
	}
}
void pobierz_lokalizacje_gracza(int player_x,int player_y, Pocisk** tablica_pociskow, int i)
{
	if (tablica_pociskow[i]->x == POZA_MAPA && tablica_pociskow[i]->y == int(SCREEN_HEIGHT / 4))
	{

		tablica_pociskow[i]->x_poczatkowe = player_x;
		tablica_pociskow[i]->y_poczatkowe = player_y;
	}
}
void tworzenie_potwrow(int lvl, Przeciwnik** tablica_przeciwnikow, int lvl_start_time)
{
	double czas_od_startu_levela = (SDL_GetTicks() - lvl_start_time) / 1000.1;
	tablica_przeciwnikow[lvl]->x = SCREEN_WIDTH / 2;
	tablica_przeciwnikow[lvl]->y = SCREEN_HEIGHT / 5;
	if(lvl ==1)
		tablica_przeciwnikow[lvl]->y = SCREEN_HEIGHT / 2;
	
}
void obsluga_zdarzenia(SDL_Event& event, bool& quit, double& player_x, double& player_y, bool& nowa_gra, SDL_Rect& camera, int fps, char &nastepny_kierunek)
{
	while (SDL_PollEvent(&event)) {
		switch (event.type) {
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
			else if (event.key.keysym.sym == SDLK_n)
			{
				nowa_gra = 1;
				quit = 1;
			}
			break;
		case SDL_QUIT:
			quit = 1;
			break;
		}
	}

	const Uint8* currentClick = SDL_GetKeyboardState(NULL);
	if (currentClick[SDL_SCANCODE_RIGHT]) {
		player_x += PLAYER_SPEED * fps / 10;

		if (player_x > int(LEVEL_WIDTH - 25))
		{
			player_x = int(LEVEL_WIDTH - 25);
		}
		camera.x = player_x - SCREEN_WIDTH / 2;
		nastepny_kierunek = 'E';

	}
	if (currentClick[SDL_SCANCODE_LEFT]) {
		player_x -= PLAYER_SPEED * fps / 10;
		if (player_x < 25)
		{
			player_x = 25;
		}
		camera.x = player_x - SCREEN_WIDTH / 2;
		nastepny_kierunek = 'W';
	}
	if (currentClick[SDL_SCANCODE_UP]) {
		player_y -= PLAYER_SPEED * fps / 10;

		if (player_y < 150)
		{
			player_y = 150;
		}
		camera.y = player_y - SCREEN_HEIGHT / 2;
		nastepny_kierunek = 'N';
	}
	if (currentClick[SDL_SCANCODE_DOWN]) {
		player_y += PLAYER_SPEED * fps / 10;
		if (player_y > int(LEVEL_HEIGHT - 300))
		{
			player_y = int(LEVEL_HEIGHT - 300);
		}
		camera.y = player_y - SCREEN_HEIGHT / 2;
		nastepny_kierunek = 'S';

	}

	if (camera.x < 0)
	{
		camera.x = 0;
	}
	if (camera.x >= LEVEL_WIDTH - SCREEN_WIDTH)
	{
		camera.x = LEVEL_WIDTH - SCREEN_WIDTH;
	}
	if (camera.y < 0)
	{
		camera.y = 0;
	}
	if (camera.y >= (LEVEL_HEIGHT - SCREEN_HEIGHT) / 2)
	{
		camera.y = (LEVEL_HEIGHT - SCREEN_HEIGHT) / 2;
	}
}


void strzal(Pocisk** tablica_pociskow, SDL_Surface*& screen, SDL_Window*& window, int* strzel, int& salwa,Przeciwnik** tablica_przeciwnikow, int lvl, int frame, int player_x, int player_y, char nastepny_kierunek, bool &trafiony)
{
	for (int i = 0; i < 40; i++)
	{
		if (tablica_pociskow[i] != nullptr )
		{

			int trasa = tablica_pociskow[i]->trajektoria;
			
			if (i < 10 && lvl ==0)//Strzelanie po łuku
			{
				pobierz_lokalizacje_przeciwnika(tablica_przeciwnikow,tablica_pociskow,lvl,i);
				tablica_pociskow[i]->x += cos(i)*5;
				tablica_pociskow[i]->y += 2;
			}
			if (lvl ==1)
			{

				if (i == 0)
				{
					pobierz_lokalizacje_przeciwnika(tablica_przeciwnikow, tablica_pociskow, lvl, i);
					tablica_pociskow[i]->x += 0;
					tablica_pociskow[i]->y += 1;
				}
				if (i == 1)
				{
					pobierz_lokalizacje_przeciwnika(tablica_przeciwnikow, tablica_pociskow, lvl, i);
					tablica_pociskow[i]->x += 0;
					tablica_pociskow[i]->y += -1;
				}
				if (i == 2)
				{
					pobierz_lokalizacje_przeciwnika(tablica_przeciwnikow, tablica_pociskow, lvl, i);
					tablica_pociskow[i]->x += 1;
					tablica_pociskow[i]->y += 0;
				}
				if (i == 3)
				{
					pobierz_lokalizacje_przeciwnika(tablica_przeciwnikow, tablica_pociskow, lvl, i);
					tablica_pociskow[i]->x += -1;
					tablica_pociskow[i]->y += 0;
				}
				if (i == 4)
				{
					pobierz_lokalizacje_przeciwnika(tablica_przeciwnikow, tablica_pociskow, lvl, i);
					tablica_pociskow[i]->x += -1;
					tablica_pociskow[i]->y += 1;
				}
				if (i == 5)
				{
					pobierz_lokalizacje_przeciwnika(tablica_przeciwnikow, tablica_pociskow, lvl, i);
					tablica_pociskow[i]->x += -1;
					tablica_pociskow[i]->y += -1;
				}
				if (i == 6)
				{
					pobierz_lokalizacje_przeciwnika(tablica_przeciwnikow, tablica_pociskow, lvl, i);
					tablica_pociskow[i]->x += 1;
					tablica_pociskow[i]->y += -1;
				}
				if (i == 7)
				{
					pobierz_lokalizacje_przeciwnika(tablica_przeciwnikow, tablica_pociskow, lvl, i);
					tablica_pociskow[i]->x += 1;
					tablica_pociskow[i]->y += 1;
				}
			}


			if (lvl == 2 && i<10)
			{
				if (frame % 120 == i*9)
				{
					strzel[i] = 1;
				}
				if (strzel[i]  ==1)
				{
					pobierz_lokalizacje_przeciwnika(tablica_przeciwnikow, tablica_pociskow, lvl, i);
					tablica_pociskow[i]->x += cos(i) * 5;
					tablica_pociskow[i]->y += 2;
				}

			}
			
			if (i==30)
			{
				if (tablica_pociskow[i]->kierunek == 'X')
				{
					tablica_pociskow[i]->kierunek = nastepny_kierunek;
				}
				if (tablica_pociskow[i]->kierunek == 'W')
				{
					pobierz_lokalizacje_gracza(player_x, player_y, tablica_pociskow, i);
					tablica_pociskow[30]->x = tablica_pociskow[30]->x_poczatkowe - 10;
					tablica_pociskow[30]->x_poczatkowe = tablica_pociskow[30]->x;
					tablica_pociskow[30]->y = tablica_pociskow[30]->y_poczatkowe;
				}
				else if (tablica_pociskow[i]->kierunek == 'E')
				{
					pobierz_lokalizacje_gracza(player_x, player_y, tablica_pociskow, i);
					tablica_pociskow[30]->x = tablica_pociskow[30]->x_poczatkowe + 10;
					tablica_pociskow[30]->x_poczatkowe = tablica_pociskow[30]->x;
					tablica_pociskow[30]->y = tablica_pociskow[30]->y_poczatkowe;

				}

				else if (tablica_pociskow[i]->kierunek == 'S')
				{
					pobierz_lokalizacje_gracza(player_x, player_y, tablica_pociskow, i);
					tablica_pociskow[30]->y = tablica_pociskow[30]->y_poczatkowe + 10;
					tablica_pociskow[30]->y_poczatkowe = tablica_pociskow[30]->y;
					tablica_pociskow[30]->x = tablica_pociskow[30]->x_poczatkowe;
				}

				else if (tablica_pociskow[i]->kierunek == 'N')
				{
					pobierz_lokalizacje_gracza(player_x, player_y, tablica_pociskow, i);
					tablica_pociskow[30]->y = tablica_pociskow[30]->y_poczatkowe - 10;
					tablica_pociskow[30]->y_poczatkowe = tablica_pociskow[30]->y;
					tablica_pociskow[30]->x = tablica_pociskow[30]->x_poczatkowe;
				}
			}
			if (trafiony == 1)
			{
				trafiony = 0;
				tablica_pociskow[30]->x = POZA_MAPA;
				tablica_pociskow[30]->y = SCREEN_HEIGHT / 4;
			}
			if (tablica_pociskow[i]->y > 850 || tablica_pociskow[i]->y ==0 || tablica_pociskow[i]->x ==0 || tablica_pociskow[i]->x > 1600 
				||(tablica_pociskow[i]->x<0 && tablica_pociskow[i]->x != POZA_MAPA)
				|| (tablica_pociskow[i]->y < 0 && tablica_pociskow[i]->y != SCREEN_HEIGHT / 4))
			{
				if (lvl == 2)
				{
					strzel[i] = 0; // TODO
				}
				if (i == 30)
				{

					tablica_pociskow[i]->kierunek = nastepny_kierunek;
				}

				
				tablica_pociskow[i]->x = POZA_MAPA;
				tablica_pociskow[i]->y = SCREEN_HEIGHT / 4;
				//cout << i << endl;
				/*
				if (i <10 &&(i + 1) % 10 == 0)
				{
					//cout << salwa << endl;
					salwa++;
				}
				if (salwa == 10)
				{
					salwa = 0;
					break;
				}
				*/
			}
		}
	}

}
void gra(SDL_Surface*& player, SDL_Surface*& zycie, SDL_Surface*& zycie_male,  SDL_Surface** rodzaje_pociskow, SDL_Surface*& tlo, SDL_Surface*& charset, SDL_Window*& window, SDL_Renderer*& renderer, SDL_Texture*& scrtex, SDL_Surface*& screen, bool& nowa_gra, Pocisk** tablica_pociskow, SDL_Surface** tab_potwor)
{

	double fpsTimer, fps;
	int t1, t2, frames, strzel[12], poziom = 1,aktualna_ilosc_zyc_gracza =5;
	bool quit;
	int salwa = 0;

	t1 = SDL_GetTicks();//ilosc czasu od startu

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	double player_x = SCREEN_WIDTH / 2;
	double player_y = SCREEN_HEIGHT / 2;
	int ilosc_pociskow = 0;
	SDL_Event event;
	Przeciwnik* tablica_przeciwnikow[3] = { nullptr };

	for (int i = 0; i < 100; i++)
	{
		tablica_pociskow[i] = new Pocisk(POZA_MAPA, SCREEN_HEIGHT / 4, i, 0, 0,0,0,'X');//x,y,trajektoria
	}
	
	for (int i = 0; i < 3; i++)
	{
		tablica_przeciwnikow[i] = new Przeciwnik(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4, (i+1)*2, (i+1));
	}
	
	int lvl = 2;

	SDL_Rect gameScreen = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	const int FRAMES_PER_SECOND = 50;
	int frame = 0, niesmiertelnosc = 0, niesmiertelnosc_potwora = 0, lvl_start_time = SDL_GetTicks(), time = SDL_GetTicks(), ilosc_trafien = 0;;
	char nastepny_kierunek = 'X';
	bool trafiony = 0, zwyciestwo = 0;
	tworzenie_potwrow(lvl, tablica_przeciwnikow, lvl_start_time);

	while (!quit)
	{

		int current_time = SDL_GetTicks();
		int time_delta = current_time - time;

		if (time_delta < (1000 / FRAMES_PER_SECOND))
		{
			//Sleep((1000 / FRAMES_PER_SECOND) / 1000);
			continue;
		}
		//Next Frame
		time = current_time;
		frame++;
		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
		DrawSurface(screen, tlo, LEVEL_WIDTH / 2, SCREEN_HEIGHT / 2);
		DrawSurface(screen, player, player_x, player_y);
		for (int i = 0; i < ILOSC_ZYC; i++)//Wyswietla zdrowie Gracza
		{
			if (i < aktualna_ilosc_zyc_gracza)
			{
				DrawSurface(screen, zycie, camera.x + (50 * i) + 30, camera.y + SCREEN_HEIGHT - 30);
			}
		}
		for (int i = 0; i < tablica_przeciwnikow[lvl]->zdrowie; i++)//Wyswietla zdrowie potwora
		{
				DrawSurface(screen, zycie_male, tablica_przeciwnikow[lvl]->x+(15 * i), tablica_przeciwnikow[lvl]->y+100);
		}
		if (lvl != 1)
		{
			tablica_przeciwnikow[lvl]->x = SCREEN_WIDTH / 2 + cos((lvl+1)*frame/80.0) * 200, SCREEN_HEIGHT / 4;
		}
		DrawSurface(screen, tab_potwor[lvl], tablica_przeciwnikow[lvl]->x, tablica_przeciwnikow[lvl]->y);
		
		strzal(tablica_pociskow, screen, window, strzel, salwa, tablica_przeciwnikow, lvl, frame, player_x, player_y, nastepny_kierunek, trafiony);


		for (int i = 0; i < 20; i++)
		{
			if (czy_trafiony(tablica_pociskow[i]->x, tablica_pociskow[i]->y, player_x, player_y) && (niesmiertelnosc+60)<frame)
			{
				niesmiertelnosc = frame;
				aktualna_ilosc_zyc_gracza--;
				quit = 1;
				cout << "trafiony" << endl;
			}
		}
		
		if (tablica_pociskow[30] != nullptr)
		{
			if (quit == 1) return;
			if (czy_trafiony(tablica_pociskow[30]->x, tablica_pociskow[30]->y, tablica_przeciwnikow[lvl]->x, tablica_przeciwnikow[lvl]->y) && niesmiertelnosc_potwora+60<frame)
			{
				trafiony = 1;
				cout << "trafiony_potwor" << endl;
				niesmiertelnosc_potwora = frame;
				tablica_przeciwnikow[lvl]->zdrowie--;
				ilosc_trafien++;
				quit = 1;
				return;
				if (tablica_przeciwnikow[lvl]->zdrowie == 0)
				{
					if (lvl == 2)
					{
						quit = 1;
					}
					else
					{
						lvl++;
					}
					
				}
				
			}
		}

		
		for (int i = 0; i < 20; i++)
		{
			if (tablica_pociskow[i] != nullptr )
			{
				DrawSurface(screen, rodzaje_pociskow[lvl + 1], tablica_pociskow[i]->x, tablica_pociskow[i]->y);
			}
		}
		if (tablica_pociskow[30] != nullptr)
		{
			DrawSurface(screen, rodzaje_pociskow[0], tablica_pociskow[30]->x, tablica_pociskow[30]->y);
		}
		double czas_od_startu_poziomu = ((double)(SDL_GetTicks() - lvl_start_time))/1000.0;
		double fps = frame / czas_od_startu_poziomu;
		wczytanie_napisu(screen, charset, scrtex, renderer, fps, czas_od_startu_poziomu,camera, gameScreen, ilosc_trafien, aktualna_ilosc_zyc_gracza);
		obsluga_zdarzenia(event, quit, player_x, player_y, nowa_gra, camera,fps, nastepny_kierunek);
		


		
	}
	for (int i = 0; i < 3; i++)
	{
		delete(tablica_przeciwnikow[i]);
	}
	for (int i = 0; i < 100; i++)
	{
		delete(tablica_pociskow[i]);
	}

}
// main


#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv)
{
	SDL_Surface* charset, * screen,* player,*zycie, *zycie_male, * tlo;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* scrtex;
	SDL_Surface* tab_potwor[3];
	SDL_Surface* rodzaje_pociskow[4];
	bool fullscreen = 0;
	bool nowa_gra = 1;
	Pocisk* tablica_pociskow[100] = { nullptr };


	if (inicjalizacja_sdl(charset, screen, window, renderer, fullscreen) == 1)
	{
		return 1;
	}
	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, LEVEL_WIDTH, LEVEL_HEIGHT);
	if (wczytywanie_mapy(player, charset,zycie, zycie_male, window, renderer, scrtex, screen, tlo, tab_potwor, rodzaje_pociskow) == 1)
	{
		return 1;
	}
	//Gra
	while (nowa_gra == 1)
	{
		nowa_gra = 0;
		gra(player,zycie,zycie_male, rodzaje_pociskow, tlo, charset, window, renderer, scrtex, screen, nowa_gra, tablica_pociskow, tab_potwor);
	}

	//Koniec(screen, scrtex, window, charset, renderer);
	exit(0);
	return 0;
};
