#ifndef ___STATE_H___
#define ___STATE_H___


/**
 *
 * Falta muita coisa, incluindo por exemplo:
 * - o mapa
 * - os monstros
 * - a fila de prioridade para saber quem se move
 * - o que está em cada casa
 *
 */
typedef struct state {
	int playerX;
	int playerY;
	int isGameOver;
	int mobTimer;
} STATE;


#endif



