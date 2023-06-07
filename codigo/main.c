#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ncurses.h>
#include <time.h>
#include <math.h>

#include "state.h"

#define MAP_WIDTH 203
#define MAP_HEIGHT 55
#define NUM_ROOMS 35
#define ROOM_MIN_WIDTH 5
#define ROOM_MAX_WIDTH 20
#define ROOM_MIN_HEIGHT 5
#define ROOM_MAX_HEIGHT 20
#define MAX_MOB_TIMER_VALUE 1

typedef struct {
    int x, y, width, height;
} Room;

typedef struct{
    int x, y;
}Mob;


Room create_room(int x, int y, int width, int height) {
    Room room;
    room.x = x;
    room.y = y;
    room.width = width;
    room.height = height;
    return room;
}

void draw_map(char map[MAP_HEIGHT][MAP_WIDTH], int visibility[MAP_HEIGHT][MAP_WIDTH], Mob mob) {
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            if (visibility[y][x]) {
                if(x == mob.x && y == mob.y){ //exibir caracter do mob
                    attron(COLOR_PAIR(COLOR_RED));
                    mvaddch(y,x, '$'| A_BOLD);
                    attroff(COLOR_PAIR(COLOR_RED));
                }else{
                     //exibir outros caracteres
                     mvaddch(y, x, map[y][x]);
                }
            } else {
                mvaddch(y, x, ' ');  // Não desenhar caracteres fora do raio de visão
            }
        }
    }
    refresh();
}


void draw_player(int x, int y) {
    attron(COLOR_PAIR(COLOR_WHITE)); //liga a cor 
    mvaddch(x, y, '@' | A_BOLD);//desenha o caracter a negrito
    attroff(COLOR_PAIR(COLOR_WHITE));//desliga a cor
}

void draw_light(STATE *st, int visibility[MAP_HEIGHT][MAP_WIDTH],char map[MAP_HEIGHT][MAP_WIDTH]) {
    int playerX = st->playerX; //obtem as coordenadas x e y do jogador
    int playerY = st->playerY;
    int raio = 10; //raio da luz

    if (map[playerX][playerY] =='%'){ //se estiver num arbusto diminui a visao
        raio = 5;
    }

    for (int dx = -raio; dx <= raio; dx++) {
        for (int dy = -raio; dy <= raio; dy++) {
            int x = playerX + dx;
            int y = playerY + dy;

            if (x >= 0 && x < MAP_HEIGHT && y >= 0 && y < MAP_WIDTH) {
                int distance = dx * dx + dy * dy;  // Distância euclidiana

                if (distance <= raio * raio) {
                    visibility[x][y] = 1;  // Tornar tudo dentro do raio visível
                }
            }
        }
    }
}


void do_movement_action(STATE *st, int dx, int dy, char map[MAP_HEIGHT][MAP_WIDTH]) {
    int nextX = st->playerX + dx; //coordenadas da proxima posiçao
    int nextY = st->playerY + dy;

    // Verificar se a próxima posição é uma parede
    if (map[nextX][nextY] != '#') {
        st->playerX = nextX;
        st->playerY = nextY;


        // Reproduzir o som de fundo quando o jogador se desloca
        if (system("aplay somandar.wav > /dev/null 2>&1")){};

    }
}

void mover_mob(Mob *mob) {
    // Gerar valores aleatórios para o deslocamento dx e dy
    int dx = rand() % 3 - 1;  // -1, 0 ou 1
    int dy = rand() % 3 - 1;  // -1, 0 ou 1

    int nextX = mob->x + dx; //proxima posiçao
    int nextY = mob->y + dy;

    // Verificar se a próxima posição está dentro dos limites do mapa
    if (nextX >= 0 && nextX < MAP_HEIGHT && nextY >= 0 && nextY < MAP_WIDTH) {
        mob->x = nextX;
        mob->y = nextY;
    }
}


void desenhar_mob(Mob mob){
    attron(COLOR_PAIR(COLOR_RED));
    mvaddch(mob.y, mob.x, '$' | A_BOLD);
    attroff(COLOR_PAIR(COLOR_RED));
}

void update(STATE *st, char map[MAP_HEIGHT][MAP_WIDTH], Mob *mob) {
    int key = getch();
    int dx=0, dy=0;

    mvaddch(st->playerX, st->playerY, ' ');
    switch(key) {
        case KEY_A1:
        case '7': do_movement_action(st, -1, -1, map); break;
        case KEY_UP:
        case '8': do_movement_action(st, -1, +0, map); break;
        case KEY_A3:
        case '9': do_movement_action(st, -1, +1, map); break;
        case KEY_LEFT:
        case '4': do_movement_action(st, +0, -1, map); break;
        case KEY_B2:
        case '5': break;
        case KEY_RIGHT:
        case '6': do_movement_action(st, +0, +1, map); break;
        case KEY_C1:
        case '1': do_movement_action(st, +1, -1, map); break;
        case KEY_DOWN:
        case '2': do_movement_action(st, +1, +0, map); break;
        case KEY_C3:
        case '3': do_movement_action(st, +1, +1, map); break;
        case 'q': endwin(); exit(0); break;
    }
    do_movement_action(st, dx,dy,map);
    mover_mob(mob); //mover o mob de forma aleatoria
    // Verificar se as coordenadas do jogador são iguais às coordenadas do mob
if (st->playerX == mob->x && st->playerY == mob->y) {
    // Jogador morre e jogo encerra
    st->isGameOver = 1;
}


}

int main() {

    STATE st = {20,20, 0, 0};

    Mob mob;
    mob.x = 30;
    mob.y = 30;


    WINDOW *wnd = initscr();
    noecho();
    curs_set(FALSE);
    int ncols, nrows;
    getmaxyx(wnd,nrows,ncols);

    srand48(time(NULL));
    start_color();

    cbreak();
    nonl();
    intrflush(stdscr, false);
    keypad(stdscr, true);

    init_pair(COLOR_WHITE, COLOR_WHITE, COLOR_BLACK);
    init_pair(COLOR_YELLOW, COLOR_YELLOW, COLOR_BLACK);
    init_pair(COLOR_BLUE, COLOR_BLUE, COLOR_BLACK);
    init_pair(COLOR_RED, COLOR_RED, COLOR_BLACK);

    srand(time(NULL));

    // Initialize ncurses
    char map[MAP_HEIGHT][MAP_WIDTH];
    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            map[y][x] = '#';
        }
    }

// Create some random rooms
    Room rooms[NUM_ROOMS];
    for (int i = 0; i < NUM_ROOMS; i++) {
        int x = rand() % (MAP_WIDTH - ROOM_MAX_WIDTH - 1) + 1;
        int y = rand() % (MAP_HEIGHT - ROOM_MAX_HEIGHT - 1) + 1;
        int width = rand() % (ROOM_MAX_WIDTH - ROOM_MIN_WIDTH + 1) + ROOM_MIN_WIDTH;
        int height = rand() % (ROOM_MAX_HEIGHT - ROOM_MIN_HEIGHT + 1) + ROOM_MIN_HEIGHT;
        rooms[i] = create_room(x, y, width, height);
    }

// Add a fixed room at position (15, 15)
int fixedX = 15;
int fixedY = 15;
int fixedWidth = 10;
int fixedHeight = 10;
rooms[NUM_ROOMS - 1] = create_room(fixedX, fixedY, fixedWidth, fixedHeight);

// Draw the rooms on the map
    for (int i = 0; i < NUM_ROOMS; i++) {
        Room room = rooms[i];
        for (int y = room.y; y < room.y + room.height; y++) {
            for (int x = room.x; x < room.x + room.width; x++) {
                if (x == room.x || x == room.x + room.width - 1 || y == room.y || y == room.y + room.height - 1) {
                  map[y][x] = '#';
                } else {
                    map[y][x] = '.';
              }
            }
        }
    }

            for (int i = 0; i < NUM_ROOMS - 1; i++) {
            Room currentRoom = rooms[i];
            Room nextRoom = rooms[i + 1];

            int startX = currentRoom.x + currentRoom.width / 2;
            int startY = currentRoom.y + currentRoom.height / 2;
            int endX = nextRoom.x + nextRoom.width / 2;
            int endY = nextRoom.y + nextRoom.height / 2;

            while (startX != endX) {
             if (startX < endX) {
                startX++;
            } else {
                startX--;
            }
            map[startY][startX] = '.';
            }

        while (startY != endY) {
            if (startY < endY) {
                startY++;
            } else {
                startY--;
            }
            map[startY][startX] = '.';
        }
    }

        //Gerar arbustos em locais aleatórios 

    for (int y = 1; y < MAP_HEIGHT - 1; y++){
        for (int x = 1; x < MAP_WIDTH - 1; x++){
            if (map[y][x] == '.'){
                int random = rand() % 100;
                if (random < 1){ // Quantidade de arbustos, maior numero -> mais arbustos
                    map[y][x] = '%';
                }
            }
        }
    }

    // Gerar aglomerados de arbustos 

    for (int y = 1; y < MAP_HEIGHT - 1; y++){
        for (int x = 1; x < MAP_WIDTH - 1; x++){
            if (map[y][x] == '.'){
                if ((map[y-1][x] == '%') || (map[y+1][x] == '%') || (map[y][x-1] == '%') || (map[y][x+1] == '%')){ 
                    int random = rand() % 100;
                    if (random < 40){
                        map[y][x] = '%';
                    }
                }
            }
        }
    }

    int visibility[MAP_HEIGHT][MAP_WIDTH] = {0};

    //incializar o timer do mob
    st.mobTimer = rand() % MAX_MOB_TIMER_VALUE;


    st.isGameOver=0;

    while (1) {
    move(nrows - 1, 0);
    attron(COLOR_PAIR(COLOR_BLUE));
    printw("(%d, %d) %d %d", st.playerX, st.playerY, ncols, nrows);
    attroff(COLOR_PAIR(COLOR_BLUE));

    mover_mob(&mob);

    st.mobTimer--;
    if(st.mobTimer <= 0){
        //o temporizador atingiu o limite, atualizar a posiçao do mob
        mover_mob(&mob);
        st.mobTimer =rand() % MAX_MOB_TIMER_VALUE;
    }


     if (mob.x == st.playerX && mob.y == st.playerY) {
        st.isGameOver = 1;
        break;  // Encerra o loop se o mob e o jogador estiverem na mesma posição
    }

    for (int y = 0; y < MAP_HEIGHT; y++) {
        for (int x = 0; x < MAP_WIDTH; x++) {
            visibility[y][x] = 0;
        }
    }

    draw_light(&st, visibility, map);
    draw_map(map, visibility, mob);
    draw_player(st.playerX, st.playerY);
    desenhar_mob(mob);

    move(st.playerX, st.playerY);
    update(&st, map, &mob);
}

    // Após o loop principal
attron(A_BOLD);  // Ativa o atributo A_BOLD para aumentar o tamanho das letras
mvprintw(nrows / 2, ncols / 2 - 5, "Game Over");
attroff(A_BOLD);  // Desativa o atributo A_BOLD
refresh();
getch();
endwin();
return 0;

}
 