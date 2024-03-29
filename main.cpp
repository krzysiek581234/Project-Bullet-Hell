﻿#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include<iostream>

extern "C"
{
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH	1024.0
#define SCREEN_HEIGHT	699.0

//The dimensions of the level
#define LEVEL_WIDTH 1324.0
#define LEVEL_HEIGHT 699.0
#define PLAYER_SPEED 6000
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
	Pocisk(int arg_x, int arg_y, int arg_trajektoria, int arg_predkosc, int arg_obrazenia)
	{
		this->x = arg_x;
		this->y = arg_y;
		this->trajektoria = arg_trajektoria;
		this->predkosc = arg_predkosc;
		this->obrazenia = arg_obrazenia;
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
bool wczytywanie_mapy(SDL_Surface*& eti, SDL_Surface*& player, SDL_Surface*& male_oko, SDL_Surface*& charset, SDL_Window*& window, SDL_Renderer*& renderer, SDL_Texture*& scrtex, SDL_Surface*& screen, SDL_Surface*& tlo)
{
	// wczytanie obrazka cs8x8.bmp
	charset = ladowanie_zdjecia("./cs8x8.bmp");

	SDL_SetColorKey(charset, true, 0x000000);
	eti = ladowanie_zdjecia("./oko.bmp");
	player = ladowanie_zdjecia("./player_50.bmp");
	male_oko = ladowanie_zdjecia("./demon_oko.bmp");
	tlo = ladowanie_zdjecia("./tlo.bmp");
	usuwanie_tla(player);
	usuwanie_tla(eti);
	usuwanie_tla(male_oko);
	return 0;

}
void wczytanie_napisu(SDL_Surface*& screen, SDL_Surface*& charset, SDL_Texture*& scrtex, SDL_Renderer*& renderer, double distance, double fps, SDL_Rect camera, SDL_Rect gameScreen)
{
	char text[128];
	int czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
	int zielony = SDL_MapRGB(screen->format, 0x00, 0xFF, 0x00);
	int czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
	int niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);

	// tekst informacyjny / info text
	DrawRectangle(screen, 4 + camera.x, 4 + camera.y, SCREEN_WIDTH - 8, 36, czerwony, niebieski);
	//            "template for the second project, elapsed time = %.1lf s  %.0lf frames / s"
	sprintf_s(text, "Szablon drugiego zadania, czas trwania = %.1lf s  %.0lf klatek / s", distance, fps);
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

void obsluga_zdarzenia(SDL_Event& event, bool& quit, double& player_x, bool& nowa_gra, SDL_Rect camera, double delta)
{
	while (SDL_PollEvent(&event))
	{
		switch (event.type)
		{
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;
			else if (event.key.keysym.sym == SDLK_RIGHT)
			{
				player_x += 15;
				if (player_x > LEVEL_WIDTH - 15)
				{
					player_x = LEVEL_WIDTH - 15;
				}
				camera.x = player_x - SCREEN_WIDTH / 2;
			}
			else if (event.key.keysym.sym == SDLK_LEFT)
			{
				player_x -= 15;
				if (player_x < 15)
				{
					player_x = 15;
				}
				camera.x = player_x - SCREEN_WIDTH / 2;
			}
			else if (event.key.keysym.sym == SDLK_n)
			{
				nowa_gra = 1;
				quit = 1;
			}
			if (camera.x < 0)
			{
				camera.x = 0;
			}
			if (camera.x >= LEVEL_WIDTH - SCREEN_WIDTH)
			{
				camera.x = LEVEL_WIDTH - SCREEN_WIDTH;
			}
			break;
		case SDL_QUIT:
			quit = 1;
			break;
		};
	};
	
}
void strzal(Pocisk** tablica_pociskow, SDL_Surface*& male_oko, SDL_Surface*& screen, SDL_Window*& window, double& worldTime, int& salwa)
{
	int kolejna = 10;
	for (int i = 0; i < 100; i++)
	{
		if (tablica_pociskow[i] != nullptr )
		{
			int predkosc = worldTime * 120;
			//cout << predkosc << endl; /// /// /// // /// // // // //
			int trasa = tablica_pociskow[i]->trajektoria;

			if (i < 10)//Strzelanie po łuku
			{
				tablica_pociskow[i]->x = (SCREEN_WIDTH / 2) + cos(trasa) * predkosc;
				tablica_pociskow[i]->y = (SCREEN_HEIGHT / 4) + predkosc;
			}
			if (i > 10 && i<20)
			{
				tablica_pociskow[i]->y = (SCREEN_HEIGHT / 4) + predkosc;
			}
			//strzelanie w dol
			//cout << tablica_pociskow[i]->y<<endl;
			if (tablica_pociskow[i]->y > 300)
			{
				kolejna++;
			}
			if (tablica_pociskow[i]->y > 700)
			{
				tablica_pociskow[i]->x = SCREEN_WIDTH / 2;
				tablica_pociskow[i]->y = SCREEN_HEIGHT / 4;
				cout << i << endl;
				if ((i + 1) % 10 == 0)
				{
					//cout << salwa << endl;
					salwa++;
				}
				if (salwa == 10)
				{
					worldTime = 0;
					salwa = 0;
					break;
				}
			}
		}
	}

}
void gra(SDL_Surface*& eti, SDL_Surface*& player, SDL_Surface*& male_oko, SDL_Surface*& tlo, SDL_Surface*& charset, SDL_Window*& window, SDL_Renderer*& renderer, SDL_Texture*& scrtex, SDL_Surface*& screen, bool& nowa_gra, Pocisk** tablica_pociskow)
{

	double delta, worldTime, fpsTimer, fps, czas_strzelania =0;
	int t1, t2, frames, poziom = 1;
	bool quit;
	int salwa = 0;
	t1 = SDL_GetTicks();//ilosc czasu od startu

	frames = 0;
	fpsTimer = 0;
	fps = 0;
	quit = 0;
	worldTime = 0;
	double player_x = SCREEN_WIDTH / 2;
	int ilosc_pociskow = 0;
	SDL_Event event;

	for (int i = 0; i < 100; i++)
	{
		tablica_pociskow[i] = new Pocisk(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4, i, 0, 0);//x,y,trajektoria
	}
	Przeciwnik* tablica_przeciwnikow[3] = { nullptr };
	for (int i = 0; i < 3; i++)
	{
		tablica_przeciwnikow[i] = new Przeciwnik(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 4, 1000*(i+1), (i+1));
	}
	int lvl = 0;
	//The gameScreen area
	SDL_Rect gameScreen = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	//The camera area
	SDL_Rect camera = { 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT };
	while (!quit)
	{
		if (lvl == 0)
		{
			tablica_przeciwnikow[0]->x = SCREEN_WIDTH / 2 + cos(worldTime) * 200;
			tablica_przeciwnikow[0]->y = SCREEN_HEIGHT / 4;
		}
		t2 = SDL_GetTicks();
		delta = (t2 - t1) * 0.001;
		t1 = t2;

		worldTime += delta;
		czas_strzelania += delta;

		SDL_FillRect(screen, NULL, SDL_MapRGB(screen->format, 0x00, 0x00, 0x00));
		DrawSurface(screen, tlo, LEVEL_WIDTH / 2, SCREEN_HEIGHT / 2);
		DrawSurface(screen, eti, tablica_przeciwnikow[lvl]->x, tablica_przeciwnikow[lvl]->y);
		DrawSurface(screen, player, player_x, SCREEN_HEIGHT / 2 + SCREEN_HEIGHT / 3);
		strzal(tablica_pociskow, male_oko, screen, window, czas_strzelania, salwa);
		for (int i = 0; i < 100; i++)
		{
			if (tablica_pociskow[i] != nullptr )
			{
				DrawSurface(screen, male_oko, tablica_pociskow[i]->x, tablica_pociskow[i]->y);
			}
		}
		fpsTimer += delta;
		if (fpsTimer > 0.02)
		{
			fps = frames * 2;
			frames = 0;
			fpsTimer -= 0.02;
			wczytanie_napisu(screen, charset, scrtex, renderer, worldTime, fps,camera, gameScreen);
			obsluga_zdarzenia(event, quit, player_x, nowa_gra, camera, delta);


			while (SDL_PollEvent(&event)) {
				switch (event.type) {
				case SDL_KEYDOWN:
					if (event.key.keysym.sym == SDLK_ESCAPE) quit = 1;

					break;
				case SDL_QUIT:
					quit = 1;
					break;
				};
			};

			const Uint8* currentClick = SDL_GetKeyboardState(NULL);
			if (currentClick[SDL_SCANCODE_RIGHT]) {
				player_x += PLAYER_SPEED * delta;

				if (player_x > LEVEL_WIDTH - 25)
				{
					player_x = LEVEL_WIDTH - 25;
				}
				camera.x = player_x - SCREEN_WIDTH / 2;
				std::cout << player_x << " " << camera.x << std::endl;
			}
			if (currentClick[SDL_SCANCODE_LEFT]) {
				player_x -= PLAYER_SPEED * delta;
				if (player_x < 25)
				{
					player_x = 25;
				}
				camera.x = player_x - SCREEN_WIDTH / 2;
				std::cout << player_x << " " << camera.x << std::endl;
			}
			if (camera.x < 0)
			{
				camera.x = 0;
			}
			if (camera.x >= LEVEL_WIDTH - SCREEN_WIDTH)
			{
				camera.x = LEVEL_WIDTH - SCREEN_WIDTH;
			}
		};
		frames++;
	};
}
// main


#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char** argv)
{
	SDL_Surface* charset, * screen, * eti, * player, * male_oko, * tlo;
	SDL_Window* window;
	SDL_Renderer* renderer;
	SDL_Texture* scrtex;
	bool fullscreen = 0;
	bool nowa_gra = 1;
	Pocisk* tablica_pociskow[100] = { nullptr };


	if (inicjalizacja_sdl(charset, screen, window, renderer, fullscreen) == 1)
	{
		return 1;
	}
	scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, LEVEL_WIDTH, LEVEL_HEIGHT);
	if (wczytywanie_mapy(eti, player, male_oko, charset, window, renderer, scrtex, screen, tlo) == 1)
	{
		return 1;
	}
	//Gra
	while (nowa_gra == 1)
	{
		nowa_gra = 0;
		gra(eti, player, male_oko, tlo, charset, window, renderer, scrtex, screen, nowa_gra, tablica_pociskow);
	}

	Koniec(screen, scrtex, window, charset, renderer);

	return 0;
};
