/*  PROYECTO DE EIGHT PUZZLE  */

#define _CRT_SECURE_NO_WARNINGS //para error por fopen

#include <allegro5/allegro.h>
#include <allegro5/utf8.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_primitives.h>
#include  <allegro5/allegro_native_dialog.h>

#include <iostream>
#include <time.h>
using namespace std;

struct boton {
	float x1, y1, x2, y2;
	string texto;
	bool contiene(float x, float y) const {
		return x >= x1 && x <= x2 && y >= y1 && y <= y2;
	}
};
struct Ficha {
	int valor;
	int m;
	float x1, y1, x2, y2; 

	// Indica si el punto cae dentro de la zona de clic de la ficha
	bool contiene(float x, float y) const {
		return x >= x1 && x <= x2 && y >= y1 && y <= y2;
	}
};


class Pantalla {
protected:
	ALLEGRO_DISPLAY* display;
	ALLEGRO_EVENT_QUEUE* event_queue;

public:
	Pantalla(ALLEGRO_DISPLAY* display, ALLEGRO_EVENT_QUEUE* event_queue) : display(display), event_queue(event_queue) {}
	ALLEGRO_DISPLAY* getDisplay() { return display; }
	ALLEGRO_EVENT_QUEUE* getEventQueue() { return event_queue; }
	virtual ~Pantalla() = default;
	virtual void evento(const ALLEGRO_EVENT& ev, Pantalla*& pantalla) = 0;
	virtual void dibujar(ALLEGRO_FONT* fuente) = 0;
};


class PantallaJuegoManual : public Pantalla {
private:
	Ficha tablero[3][3];
	int tableroFinal[3][3] = { { 1, 2, 3 },{ 4, 5, 6 },{ 7, 8, 0 } };
	float lado = 150;         // tamaño de cada celda
	float origenX = 600;      // esquina superior izquierda del tablero
	float origenY = 300;

	// Asigna a cada celda su zona de clic según el origen y el tamaño del tablero
	void calcularZonasClic() {
		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				tablero[i][j].x1 = origenX + j * lado;
				tablero[i][j].y1 = origenY + i * lado;
				tablero[i][j].x2 = origenX + (j + 1) * lado;
				tablero[i][j].y2 = origenY + (i + 1) * lado;
			}
		}
	}

	// Revisa si el tablero actual tiene solución (inversiones pares)
	bool esResoluble() {
		int lista[9], n = 0;
		for (int i = 0; i < 3; i++)
			for (int j = 0; j < 3; j++)
				if (tablero[i][j].valor != 0)
					lista[n++] = tablero[i][j].valor;   // aplana sin el hueco

		int inversiones = 0;
		for (int a = 0; a < 8; a++)
			for (int b = a + 1; b < 8; b++)
				if (lista[a] > lista[b]) inversiones++;

		return inversiones % 2 == 0;
	}

	// Genera un tablero aleatorio que siempre tiene solución
	void nuevoJuego() {
		int aux = 0;
		bool repetido = false;

		do {
			// Reiniciar los valores en cada intento (las zonas de clic no se tocan)
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					tablero[i][j].valor = -1;
					tablero[i][j].m = 0;
				}
			}

			// Llenar con números del 0 al 8 sin repetir
			for (int i = 0; i < 3; i++) {
				for (int j = 0; j < 3; j++) {
					do {
						aux = rand() % 9;
						repetido = false;
						for (int k = 0; k < 3 && !repetido; k++) {
							for (int l = 0; l < 3; l++) {
								if (tablero[k][l].valor == aux) {
									repetido = true;
									break;
								}
							}
						}
					} while (repetido);

					tablero[i][j].valor = aux;
				}
			}
		} while (!esResoluble());
	}
	void winner(Pantalla*& pantalla); // Si la ficha en (fila, col) tiene el hueco (0) a un lado, intercambia sus valores
	void intentarMover(int fila, int col, Pantalla*& pantalla) {
		if (tablero[fila][col].valor == 0) return;   // clic en el hueco

		int df[4] = { -1, 1, 0, 0 };   // arriba, abajo, izquierda, derecha
		int dc[4] = { 0, 0, -1, 1 };

		for (int d = 0; d < 4; d++) {
			int nf = fila + df[d];
			int nc = col + dc[d];

			if (nf < 0 || nf > 2 || nc < 0 || nc > 2) continue;   // fuera del tablero

			if (tablero[nf][nc].valor == 0) {
				// Intercambio de valores
				tablero[nf][nc].valor = tablero[fila][col].valor;
				tablero[fila][col].valor = 0;

				for (int i = 0; i < 3; i++) {
					for (int j = 0; j < 3; j++) {
						if (tablero[i][j].valor != tableroFinal[i][j]) {
							return; 
						}
					}
				}
				ALLEGRO_FONT* fuente = al_load_ttf_font("fuente1.ttf", 24, 0);
				pantalla->dibujar(fuente);
				al_flip_display();
				al_destroy_font(fuente);
				winner(pantalla);
			}
		}
	}

public:
	PantallaJuegoManual(ALLEGRO_DISPLAY* display, ALLEGRO_EVENT_QUEUE* event_queue) : Pantalla(display, event_queue) {
		calcularZonasClic();
		nuevoJuego();
	}

	
	void evento(const ALLEGRO_EVENT& ev, Pantalla*& pantalla) override {
		if (ev.type != ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) return;

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				if (tablero[i][j].contiene(ev.mouse.x, ev.mouse.y)) {
					intentarMover(i, j, pantalla);
					return;
				}
			}
		}
	}

	void dibujar(ALLEGRO_FONT* fuente) override {
		float altoFuente = al_get_font_line_height(fuente);

		for (int i = 0; i < 3; i++) {
			for (int j = 0; j < 3; j++) {
				const Ficha& f = tablero[i][j];

				if (f.valor != 0) {
					al_draw_filled_rectangle(f.x1, f.y1, f.x2, f.y2, al_map_rgb(102, 51, 0));
					al_draw_textf(fuente, al_map_rgb(255, 240, 200),
						(f.x1 + f.x2) / 2, (f.y1 + f.y2) / 2 - altoFuente / 2,
						ALLEGRO_ALIGN_CENTER, "%d", f.valor);
				}
				else {
					al_draw_filled_rectangle(f.x1, f.y1, f.x2, f.y2, al_map_rgb(40, 20, 0));
				}

				al_draw_rectangle(f.x1, f.y1, f.x2, f.y2, al_map_rgb(0, 0, 0), 4);
			}
		}
	}
};
class PantallaMenu : public Pantalla {
private:
	boton botones[3] = {
		{ 600, 400, 850, 500, "Jugar tradicional" },
		{ 600, 550, 850, 650, "Jugar automático" },
		{ 600, 700, 850, 800, "Puntuaciones" }
	};
public:
	PantallaMenu(ALLEGRO_DISPLAY* display, ALLEGRO_EVENT_QUEUE* event_queue) : Pantalla(display, event_queue) {}
	void evento(const ALLEGRO_EVENT& ev, Pantalla*& pantalla) override {
		if (ev.type != ALLEGRO_EVENT_MOUSE_BUTTON_DOWN) return;

		for (auto b : botones) {
			if (b.contiene(ev.mouse.x, ev.mouse.y)) {
				if (b.texto == "Jugar tradicional") {
					Pantalla* aux = pantalla; 
					pantalla = new PantallaJuegoManual(display, event_queue);
					delete aux;  

					return;
				}
				else if (b.texto == "Jugar automático") {
					// Cambiar a la pantalla de juego automático
					// Implementar la clase PantallaJuegoAutomatico similar a PantallaJuegoManual
					// Pantalla* nuevaPantalla = new PantallaJuegoAutomatico(display, event_queue);
					// delete this;  // Liberar la pantalla actual
					return;
				}
				else if (b.texto == "Puntuaciones") {
					// Cambiar a la pantalla de puntuaciones
					// Implementar la clase PantallaPuntuaciones similar a PantallaJuegoManual
					// Pantalla* nuevaPantalla = new PantallaPuntuaciones(display, event_queue);
					// delete this;  // Liberar la pantalla actual
					return;
				}
			}
		}
	}
	void dibujar(ALLEGRO_FONT* fuente) override {
		for (auto b : botones) {
			al_draw_filled_rectangle(b.x1, b.y1, b.x2, b.y2, al_map_rgb(0, 102, 204));
			al_draw_text(fuente, al_map_rgb(255, 255, 255), (b.x1 + b.x2) / 2, (b.y1 + b.y2) / 2 - al_get_font_line_height(fuente) / 2,
				ALLEGRO_ALIGN_CENTER, b.texto.c_str());
			al_draw_rectangle(b.x1, b.y1, b.x2, b.y2, al_map_rgb(0, 0, 0), 4);
		}
	}
};

void PantallaJuegoManual::winner(Pantalla*& pantalla) {
	al_show_native_message_box(display, "¡Felicidades!", "Has ganado","¡Has completado el puzzle!", NULL, ALLEGRO_MESSAGEBOX_WARN);

	Pantalla* aux = pantalla;
	pantalla = new PantallaMenu(display, event_queue);
	delete aux;   // aux == this: después de esto no se puede tocar ningún atributo
}

int main() {
	al_init();
	al_install_mouse();
	al_init_font_addon();
	al_init_ttf_addon();
	al_init_primitives_addon();  
	al_init_native_dialog_addon();

	srand(time(NULL));

	ALLEGRO_MONITOR_INFO pantalla;
	al_get_monitor_info(0, &pantalla);
	int anchoP = pantalla.x2 - pantalla.x1;
	int altoP = pantalla.y2 - pantalla.y1;

	ALLEGRO_FONT* fuente = al_load_ttf_font("fuente1.ttf", 24, 0);
	Pantalla* pantallaActual = new PantallaMenu(al_create_display(anchoP, altoP - 75), al_create_event_queue());

	bool salir = false;

	ALLEGRO_EVENT ev;
	al_register_event_source(pantallaActual->getEventQueue(), al_get_display_event_source(pantallaActual->getDisplay()));
	al_register_event_source(pantallaActual->getEventQueue(), al_get_mouse_event_source());

	while (!salir) {
		al_clear_to_color(al_map_rgb(224, 224, 224));
		pantallaActual->dibujar(fuente);
		al_flip_display();

		al_wait_for_event(pantallaActual->getEventQueue(), &ev);
		if (ev.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			salir = true;
		}
		else {
			pantallaActual->evento(ev, pantallaActual);   
		}
	}

	ALLEGRO_DISPLAY* d = pantallaActual->getDisplay();
	ALLEGRO_EVENT_QUEUE* q = pantallaActual->getEventQueue();
	delete pantallaActual;
	al_destroy_font(fuente);
	al_destroy_event_queue(q);
	al_destroy_display(d);

	return 0;
}