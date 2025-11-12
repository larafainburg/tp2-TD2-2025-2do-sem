/*

Indique el porcentaje aproximado de lıneas de codigo del trabajo practico que fueron
realizadas con asistencia de una IA.
Aproximadamente un 20 % del código fue desarrollado con asistencia de herramientas de inteligencia artificial. 
La use mas que nada para los tests, ya que era un trabajo bastante repetitivo, donde no habia que pensar ninguna logica. 
Tambien me sirvio ayudarme a entender bien las estructuras de todo el TP, ya que es una de las tareas mas complejas al arrancarlo. 

¿Como verificaron que las sugerencias de la IA eran correctas?
Para verificar que las sugerencias de la inteligencia artificial eran correctas, lo que hicimos fue
complementarlo y compararlo con el material visto en clase, para evaluar si coincidían con los conceptos enseñados.
Ademas, compilando y corriendo el codigo pudimos verificar que funcionaba como esperabamos, y asi confirmar que las sugerencias que nos daba la IA eran correctas o no depende del caso. 

¿Se enfrentaron a alguna dificultad al utilizar las herramientas de IA? ¿Como las resolvieron?
Al utilizar las herramientas de IA, tuve algunas dificultades. Por ejemplo, al
intentar usarlas como guía para identificar nuestros errores de código, a veces la IA tendía a
modificar la originalidad del código y no señalar el origen de los errores que no nos permitían
completar el trabajo.
Desde el inicio del TP, decidi emplear la IA como herramienta de apoyo, planteando consultas
más puntuales y concretas para profundizar en el funcionamiento de ciertas funciones y encontrar
posibles formas de mejorarlas sin depender completamente de sus sugerencias.

¿Consideran que el uso de la IA les ha permitido desarrollar habilidades de programaci´on
en C? ¿Por que?
Considero que el uso de la IA nos aportó información muy valiosa que ayudó a entender
mejor al momento de programar en C. Asimismo, creo que únicamente el uso de Chat GPT (que
fué la principal herramienta de IA que use) no alcanzó para que pueda aprender y entender
bien el funcionamiento de C. Es por eso, que además de usar la inteligencia artificial,
complemente esta ayuda yendo a las clases de consulta, preguntando a los profesores de la
materia durante las clases y trabajando junto con compañeros de otros grupos.

*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// ========= CONSTANTES DEL JUEGO =========
#define SCREEN_WIDTH 900
#define SCREEN_HEIGHT 500

#define GRID_OFFSET_X 220
#define GRID_OFFSET_Y 59
#define GRID_WIDTH 650
#define GRID_HEIGHT 425

#define GRID_ROWS 5
#define GRID_COLS 9
#define CELL_WIDTH (GRID_WIDTH / GRID_COLS)
#define CELL_HEIGHT (GRID_HEIGHT / GRID_ROWS)

#define PEASHOOTER_FRAME_WIDTH 177
#define PEASHOOTER_FRAME_HEIGHT 166
#define PEASHOOTER_TOTAL_FRAMES 31
#define PEASHOOTER_ANIMATION_SPEED 4
#define PEASHOOTER_SHOOT_FRAME 18

#define ZOMBIE_FRAME_WIDTH 164
#define ZOMBIE_FRAME_HEIGHT 203
#define ZOMBIE_TOTAL_FRAMES 90
#define ZOMBIE_ANIMATION_SPEED 2
#define ZOMBIE_DISTANCE_PER_CYCLE 40.0f

#define MAX_ARVEJAS 100
#define PEA_SPEED 5
#define ZOMBIE_SPAWN_RATE 300

// ========= ESTRUCTURAS DE DATOS =========
typedef struct {
    int row, col;
} Cursor;

typedef struct {
    SDL_Rect rect;
    int activo;
    int cooldown;
    int current_frame;
    int frame_timer;
    int debe_disparar;
} Planta;

typedef struct {
    SDL_Rect rect;
    int activo;
} Arveja;

typedef struct {
    SDL_Rect rect;
    int activo;
    int vida;
    int row;
    int current_frame;
    int frame_timer;
    float pos_x;
} Zombie;

// ========= NUEVAS ESTRUCTURAS =========
#define STATUS_VACIO 0
#define STATUS_PLANTA 1

typedef struct RowSegment {
    int status;
    int start_col;
    int length;
    Planta* planta_data;
    struct RowSegment* next;
} RowSegment;

typedef struct ZombieNode {
    Zombie zombie_data;
    struct ZombieNode* next;
} ZombieNode;

typedef struct GardenRow {
    RowSegment* first_segment;
    ZombieNode* first_zombie;
} GardenRow;

typedef struct GameBoard {
    GardenRow rows[GRID_ROWS];
    Arveja arvejas[MAX_ARVEJAS];
    int zombie_spawn_timer;
} GameBoard;

// ========= VARIABLES GLOBALES =========
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
SDL_Texture* tex_background = NULL;
SDL_Texture* tex_peashooter_sheet = NULL;
SDL_Texture* tex_zombie_sheet = NULL;
SDL_Texture* tex_pea = NULL;

Cursor cursor = {0, 0};
GameBoard* game_board = NULL;

// ========= FUNCIONES DE STRING =========

int strLen(char* s) {
    int contador = 0;
    if (s == NULL) return 0;
    while (s[contador] != '\0') {
        contador++;
    }
    return contador;
}

char* strDuplicate(char* src) {
    if (src == NULL) return NULL; 

    int largo = strLen(src);  // uso mi propia función para contar los caracteres
    // pido memoria suficiente para el string + el '\0'
    char* copia = (char*)malloc(sizeof(char) * (largo + 1));
    if (copia == NULL) return NULL;

    // copio carácter por carácter hasta el final (incluyendo el '\0')
    for (int i = 0; i <= largo; i++) {
        copia[i] = src[i];
    }
    // devuelvo el nuevo string duplicado
    return copia;
}

// Compara dos strings letra por letra
// devuelve 0 si son iguales, 1 si 'a' es menor que 'b', y -1 si 'a' es mayor.
int strCompare(char* a, char* b) {
    // si ambos son NULL, los considero iguales
    if (a == NULL && b == NULL) return 0;

    // si uno de los dos es NULL, ya se cuál gana
    if (a == NULL) return 1;   // NULL lo trato como menor
    if (b == NULL) return -1;

    int indice = 0;

    // Recorro ambos hasta que alguno termine o haya diferencia
    while (a[indice] != '\0' && b[indice] != '\0') {
        if (a[indice] < b[indice]) return 1;   // 'a' viene antes alfabéticamente
        if (a[indice] > b[indice]) return -1;  // 'a' viene después
        indice++;
    }

    // Si salgo del while, puede ser que uno terminó antes
    if (a[indice] == '\0' && b[indice] == '\0') return 0; // iguales
    return (a[indice] == '\0') ? 1 : -1; // el más corto es el menor
}

char* strConcatenate(char* src1, char* src2) {
    if (src1 == NULL || src2 == NULL) return NULL;  // si alguno viene vacío, no tiene sentido concatenar

    int largo1 = strLen(src1);
    int largo2 = strLen(src2);

    // pido memoria para ambos strings + el '\0' final
    char* resultado = (char*)malloc((largo1 + largo2 + 1) * sizeof(char));
    if (resultado == NULL){
        free (src1); 
        free (src2); 
    return NULL; 
    } 


    int indice = 0;

    for (int i = 0; i < largo1; i++) { //copio el primero al resultado
        resultado[indice++] = src1[i];
    }

    for (int j = 0; j < largo2; j++) { //copio el segundo justo después
        resultado[indice++] = src2[j];
    }

    resultado[indice] = '\0';
    free(src1);
    free(src2);
    return resultado;
}

//crea un nuevo tablero de juego completamente vacío.
//acá inicializo las filas, los segmentos vacíos y las arvejas pidiendo memoria para cada estructura requerida.
// Esto soluciona el tema de generar el tablero vacio
GameBoard* gameBoardNew() {
    //Pido memoria para el tablero principal
    GameBoard* tablero = malloc(sizeof(GameBoard));
    if (tablero == NULL) return NULL; 

    //seteo el timer de zombies 
    tablero->zombie_spawn_timer = ZOMBIE_SPAWN_RATE;

    //recorro las filas del jardín (5 en total)
    for (int fila = 0; fila < GRID_ROWS; fila++) {
        //cada fila arranca con un solo segmento vacío
        RowSegment* segmento_inicial = malloc(sizeof(RowSegment));
        if (segmento_inicial == NULL) {
            for (int j = 0; j < fila; j++) {
                free(tablero->rows[j].first_segment);
            }
            free(tablero);
            return NULL;
        }

        //inicializo el segmento vacío (toda la fila sin plantas)
        segmento_inicial->status = STATUS_VACIO;
        segmento_inicial->start_col = 0;
        segmento_inicial->length = GRID_COLS;
        segmento_inicial->planta_data = NULL;
        segmento_inicial->next = NULL;

        tablero->rows[fila].first_segment = segmento_inicial;
        tablero->rows[fila].first_zombie = NULL; //sin zombies todavía
    }

    for (int i = 0; i < MAX_ARVEJAS; i++) //inicializo todas las arvejas como inactivas (no hay disparos)
        tablero->arvejas[i].activo = 0;
    return tablero;
}

// Libera toda la memoria asociada a una fila del jardín
// Acá borro los segmentos (vacíos y con plantas) y también los zombies
// importante tener en cuenta q cada fila tiene su propia lista enlazada!
void gameRowDelete(GardenRow* row) {
    RowSegment* segmento_actual = row->first_segment;

    while (segmento_actual != NULL) {// Recorro todos los segmentos de la fila y los libero
        RowSegment* siguiente_segmento = segmento_actual->next; //me guardo el proximo

        if (segmento_actual->planta_data != NULL) { // Si el segmento tenía una planta, también libero esa memoria
            free(segmento_actual->planta_data);
        }

        free(segmento_actual); // Libero el nodo del segmento

        segmento_actual = siguiente_segmento; // Avanzo al siguiente de la lista
    }

    // Ahora hago lo mismo con los zombies de la fila
    ZombieNode* zombie_actual = row->first_zombie;
    while (zombie_actual != NULL) {
        ZombieNode* siguiente_zombie = zombie_actual->next;
        free(zombie_actual); //libero memoria
        zombie_actual = siguiente_zombie;
    }
}

// Libera toda la memoria asociada al tablero completo del juego
// Llama a gameRowDelete() q la hice antes para cada fila, y después libera el tablero 
// Luego de eliminar cada row, recien ahi libera el board, borrando asi todo.
void gameBoardDelete(GameBoard* board) {
    if (board == NULL) return; 

    // Recorro las filas del tablero y las libero una por una (SON 5 LO DICE LA CONSIGNA)
    for (int i = 0; i < GRID_ROWS; i++) {
        gameRowDelete(&board->rows[i]);
    }

    // Al final libero la estructura principal del board
    free(board);
}

// funcion auxiliar q verifica si una columna específica está dentro del rango de un segmento.
int columna_en_rango(RowSegment* segmento, int col) {
    int inicio = segmento->start_col;
    int fin = segmento->start_col + segmento->length;
    return (col >= inicio && col < fin);
}

// Divido un segmento vacío en partes para poder insertar una planta en la columna indicada
// Busco el segmento donde va la nueva planta
// Si la celda está libre, crear una nueva planta
// Cortar el segmento original en uno o dos pedazos según corresponda
int dividir_segmento(GameBoard* board, GardenRow* row, int row_index, int col) {
    RowSegment* actual = row->first_segment;
    RowSegment* anterior = NULL;

    // Busco el segmento donde cae la columna pedida
    while (actual != NULL && !columna_en_rango(actual, col)) {
        anterior = actual;
        actual = actual->next;
    }

    // Si no encontré o ya hay una planta, no hago nada
    if (actual == NULL || actual->status == STATUS_PLANTA) {
        return 0;
    }

    // Creo la nueva planta (cada planta tiene su struct)
    Planta* nueva_planta = (Planta*)malloc(sizeof(Planta));
    if (nueva_planta == NULL) return 0;

    nueva_planta->rect.x = GRID_OFFSET_X + (col * CELL_WIDTH);
    nueva_planta->rect.y = GRID_OFFSET_Y + (row_index * CELL_HEIGHT);
    nueva_planta->rect.w = CELL_WIDTH;
    nueva_planta->rect.h = CELL_HEIGHT;

    // Estado inicial de la planta (activa, sin disparar todavía)
    nueva_planta->activo = 1;
    nueva_planta->cooldown = rand() % 100; // random para q no disparen todas a la vez
    nueva_planta->current_frame = 0;
    nueva_planta->frame_timer = 0;
    nueva_planta->debe_disparar = 0;

    // Caso la planta va justo al principio del segmento
    if (col == actual->start_col) {
        RowSegment* nuevo_seg = (RowSegment*)malloc(sizeof(RowSegment));
        if (nuevo_seg == NULL) {
            free(nueva_planta); //libero memoria
            return 0;
        }

        // Enlazo el nuevo nodo antes del actual
        nuevo_seg->next = actual;
        if (actual == row->first_segment) {
            row->first_segment = nuevo_seg;
        } else {
            anterior->next = nuevo_seg;
        }

        // Ajusto el segmento original (le saco una columna del inicio)
        actual->start_col++;
        actual->length--;

        // Completo el nuevo segmento con la planta
        nuevo_seg->status = STATUS_PLANTA;
        nuevo_seg->start_col = col;
        nuevo_seg->length = 1;
        nuevo_seg->planta_data = nueva_planta;

        return 1;
    }

    // Caso la planta va al final del segmento
    else if (col == actual->start_col + actual->length - 1) {
        RowSegment* nuevo_seg = (RowSegment*)malloc(sizeof(RowSegment));
        if (nuevo_seg == NULL) {
            free(nueva_planta);
            return 0;
        }

        // Enlazo el nuevo segmento al final del actual
        nuevo_seg->next = actual->next;
        actual->next = nuevo_seg;

        // Achico el segmento original (le saco una celda al final)
        actual->length--;

        // Configuro el nuevo segmento con la planta
        nuevo_seg->planta_data = nueva_planta;
        nuevo_seg->status = STATUS_PLANTA;
        nuevo_seg->start_col = col;
        nuevo_seg->length = 1;

        return 1;
    }

    // Caso la planta va en el medio del segmento → lo tengo que partir en 3
    else {
        RowSegment* seg_planta = (RowSegment*)malloc(sizeof(RowSegment));
        RowSegment* seg_derecha = (RowSegment*)malloc(sizeof(RowSegment));

        if (seg_planta == NULL || seg_derecha == NULL) {
            free(nueva_planta);
            if (seg_planta) free(seg_planta);
            if (seg_derecha) free(seg_derecha);
            return 0;
        }

        int inicio_original = actual->start_col;
        int largo_original = actual->length;

        // Enlazo los nuevos nodos correctamente
        seg_derecha->next = actual->next;
        actual->next = seg_planta;
        seg_planta->next = seg_derecha;

        // Ajusto el segmento original (parte izquierda)
        actual->length = col - inicio_original;

        // Segmento de la planta (la del medio)
        seg_planta->start_col = col;
        seg_planta->length = 1;
        seg_planta->status = STATUS_PLANTA;
        seg_planta->planta_data = nueva_planta;

        // Segmento de la derecha (resto vacío)
        seg_derecha->start_col = col + 1;
        seg_derecha->length = (inicio_original + largo_original) - (col + 1);
        seg_derecha->status = STATUS_VACIO;
        seg_derecha->planta_data = NULL;

        return 1;
    }
}


// Agrega una planta en la fila y columna indicadas del tablero. se fija si es una row valida
// Si ya hay una planta o la posición no existe, no hace nada. 
int gameBoardAddPlant(GameBoard* board, int row, int col) {
    // chequeo por si me pasan una fila o columna fuera de rango
    if (row < 0 || row >= GRID_ROWS || col < 0 || col >= GRID_COLS) {
        return 0; 
    }

    // obtengo la fila donde quiero plantar
    GardenRow* fila_objetivo = &board->rows[row];

    // llamo a dividir_segmento (funcion auxiliar que maneja los casos y memoria) para que haga todo
    return dividir_segmento(board, fila_objetivo, row, col);
}

// Une segmentos vacíos después de eliminar una planta
// La idea es dejar la fila lo más limpia posible (sin nodos innecesarios)
// borro la planta, y si los de al lado estaban vacíos, los uno todos
void fusionar_segmentos(GardenRow* row, int col) {
    /*
casos posibles para borrar la planta del medio
[planta], [planta], [planta] => [planta], [vacio], [planta]
[vacio], [planta], [planta] => [vacio, vacio], [planta]
[planta], [planta], [vacio] => [planta], [vacio, vacio]
[vacio], [planta], [vacio] => [vacio, vacio, vacio]
*/

    RowSegment* actual = row->first_segment;
    RowSegment* anterior = NULL;

    //Busco el segmento que empieza justo en la columna que quiero eliminar
    while (actual != NULL && actual->start_col != col) {
        anterior = actual;
        actual = actual->next;
    }

    //si no hay planta ahí, me voy
    if (actual == NULL || actual->status != STATUS_PLANTA) {
        return;
    }

    //libero la memoria de la planta
    //[planta], [planta], [planta] => [planta], [vacio], [planta]
    free(actual->planta_data); //libero memoria de la planta
    actual->planta_data = NULL;
    actual->status = STATUS_VACIO;

    int anterior_vacio = (anterior != NULL && anterior->status == STATUS_VACIO);
    int siguiente_vacio = (actual->next != NULL && actual->next->status == STATUS_VACIO);

    // Caso los dos lados están vacíos: uno los tres en uno solo
    if (anterior_vacio && siguiente_vacio) {
        RowSegment* nodo_derecha = actual->next;
        anterior->length += actual->length + nodo_derecha->length;
        anterior->next = nodo_derecha->next;
        free(actual);
        free(nodo_derecha);
    }

    //caso solo el anterior está vacío 
    //[vacio], [planta], [planta] => [vacio, vacio], [planta]
    else if (anterior_vacio && !siguiente_vacio) {
        anterior->length += actual->length;
        anterior->next = actual->next;
        free(actual);
    }

    // Caso solo el de la derecha está vacío 
    //[planta], [planta], [vacio] => [planta], [vacio, vacio]
    else if (!anterior_vacio && siguiente_vacio) {
        RowSegment* nodo_derecha = actual->next;
        actual->length += nodo_derecha->length;
        actual->next = nodo_derecha->next;
        free(nodo_derecha); //solamente libera el actual unificandolo con el anterior y actualizando los datos
    }
}


// Saca una planta de una fila y columna específicas.
// Acá no hago el borrado directo: delego a fusionar_segmentos()
// para que también se encargue de unir los huecos vacíos si hace falta.
void gameBoardRemovePlant(GameBoard* board, int row, int col) {
    if (row < 0 || row >= GRID_ROWS || col < 0 || col >= GRID_COLS) {
        return;
    }

    // busco la fila correspondiente
    GardenRow* fila_objetivo = &board->rows[row];

    // llamo a fusionar_segmentos que hace todo 
    fusionar_segmentos(fila_objetivo, col);
}

// Agrega un nuevo zombie a una fila del tablero.
// Cada fila tiene su propia lista enlazada de zombies.
// Básicamente creo uno nuevo, lo inicializo y lo encadeno al final de la lista.
void gameBoardAddZombie(GameBoard* board, int row) {
    // chequeo rápido de rango por las dudas
    if (row < 0 || row >= GRID_ROWS) return;

    // reservo memoria para el nuevo nodo del zombie
    ZombieNode* nuevo_zombie = (ZombieNode*)malloc(sizeof(ZombieNode));
    if (nuevo_zombie == NULL) return; // si no hay memoria corto acá
    nuevo_zombie->zombie_data.pos_x = GRID_OFFSET_X + GRID_WIDTH - (ZOMBIE_FRAME_WIDTH / 2);
    nuevo_zombie->zombie_data.rect.x = (int)nuevo_zombie->zombie_data.pos_x;
    nuevo_zombie->zombie_data.rect.y = GRID_OFFSET_Y + (row * CELL_HEIGHT) - 20; // un poquito más arriba
    nuevo_zombie->zombie_data.rect.w = 80;
    nuevo_zombie->zombie_data.rect.h = 100;
    nuevo_zombie->zombie_data.activo = 1;
    nuevo_zombie->zombie_data.vida = 100;
    nuevo_zombie->zombie_data.row = row;
    nuevo_zombie->zombie_data.current_frame = 0;
    nuevo_zombie->zombie_data.frame_timer = 0;
    nuevo_zombie->next = NULL;

    // obtengo la lista de zombies de esa fila
    ZombieNode* puntero = board->rows[row].first_zombie;

    // si no hay ninguno ese pasa a ser el primero
    if (puntero == NULL) {
        board->rows[row].first_zombie = nuevo_zombie;
        return;
    }

    // si ya hay zombies, recorro hasta el final y lo agrego ahí
    ZombieNode* ultimo = NULL;
    while (puntero != NULL) {
        ultimo = puntero;
        puntero = puntero->next;
    }

    // engancho el nuevo zombie al final de la lista
    ultimo->next = nuevo_zombie;
}


//lo que me fije que tenga en cuenta esta funcion para que sea correcta es el movimiento del zombie, la animacion del zombie
//que elimine zombie si murio, el cooldown de plantas, la animacion de plantas, los disparos de plantas, la generacion de zombies
void gameBoardUpdate(GameBoard* board) {
    // Si el tablero no existe, corto por las dudas
    if (board == NULL) return;

    //Actualizo todos los zombies
    for (int fila_idx = 0; fila_idx < GRID_ROWS; fila_idx++) { 
        ZombieNode* nodo_z = board->rows[fila_idx].first_zombie; // primer zombie de la fila
        ZombieNode* nodo_z_prev = NULL; // para manejar eliminaciones en la lista
        
        while (nodo_z != NULL) {
            Zombie* enemigo = &nodo_z->zombie_data; 
            
             // Mover zombie hacia la izquierda (camina)
            float velocidad_por_tick = ZOMBIE_DISTANCE_PER_CYCLE / (float)(ZOMBIE_TOTAL_FRAMES * ZOMBIE_ANIMATION_SPEED);
            enemigo->pos_x -= velocidad_por_tick;
            enemigo->rect.x = (int)enemigo->pos_x;
            
             // Actualizar animación del zombie (cambia de frame)
            enemigo->frame_timer++;
            if (enemigo->frame_timer >= ZOMBIE_ANIMATION_SPEED) {
                enemigo->frame_timer = 0;
                enemigo->current_frame = (enemigo->current_frame + 1) % ZOMBIE_TOTAL_FRAMES;
            }
             // Si el zombie muere (vida <= 0), lo saco de la lista y libero memoria
            if (enemigo->vida <= 0) {
                ZombieNode* nodo_eliminar = nodo_z;
                if (nodo_z_prev == NULL) {
                    // era el primero de la lista
                    board->rows[fila_idx].first_zombie = nodo_z->next;
                } else {
                    nodo_z_prev->next = nodo_z->next;
                }
                nodo_z = nodo_z->next;
                free(nodo_eliminar);
                continue; // sigo al siguiente sin avanzar el prev
            }
            
            // avanzo al siguiente zombie
            nodo_z_prev = nodo_z;
            nodo_z = nodo_z->next;
        }
    }
    //Actualizo las plantas
    for (int fila_idx = 0; fila_idx < GRID_ROWS; fila_idx++) {
        RowSegment* segmento = board->rows[fila_idx].first_segment;
        
        while (segmento != NULL) {
            if (segmento->status == STATUS_PLANTA && segmento->planta_data != NULL) {
                Planta* shooter = segmento->planta_data;
                
                if (shooter->cooldown <= 0) {
                    shooter->debe_disparar = 1;
                } else {
                    shooter->cooldown--;
                }
                
                shooter->frame_timer++;
                if (shooter->frame_timer >= PEASHOOTER_ANIMATION_SPEED) {
                    shooter->frame_timer = 0;
                    shooter->current_frame = (shooter->current_frame + 1) % PEASHOOTER_TOTAL_FRAMES;
                    
                    if (shooter->debe_disparar && shooter->current_frame == PEASHOOTER_SHOOT_FRAME) {
                        for (int idx_pea = 0; idx_pea < MAX_ARVEJAS; idx_pea++) {
                            if (!board->arvejas[idx_pea].activo) {
                                board->arvejas[idx_pea].rect.x = shooter->rect.x + (CELL_WIDTH / 2);
                                board->arvejas[idx_pea].rect.y = shooter->rect.y + (CELL_HEIGHT / 4);
                                board->arvejas[idx_pea].rect.w = 20;
                                board->arvejas[idx_pea].rect.h = 20;
                                board->arvejas[idx_pea].activo = 1;
                                break;
                            }
                        }
                        shooter->cooldown = 120;
                        shooter->debe_disparar = 0;
                    }
                }
            }
            segmento = segmento->next;
        }
    }
    
    for (int idx_pea = 0; idx_pea < MAX_ARVEJAS; idx_pea++) {
        if (board->arvejas[idx_pea].activo) {
            board->arvejas[idx_pea].rect.x += PEA_SPEED;
            if (board->arvejas[idx_pea].rect.x > SCREEN_WIDTH) {
                board->arvejas[idx_pea].activo = 0;
            }
        }
    }
    
    for (int fila_idx = 0; fila_idx < GRID_ROWS; fila_idx++) {
        ZombieNode* nodo_z = board->rows[fila_idx].first_zombie;
        
        while (nodo_z != NULL) {
            for (int idx_pea = 0; idx_pea < MAX_ARVEJAS; idx_pea++) {
                if (!board->arvejas[idx_pea].activo) continue;
                
                int fila_arveja = (board->arvejas[idx_pea].rect.y - GRID_OFFSET_Y) / CELL_HEIGHT;
                if (fila_idx == fila_arveja) {
                    if (SDL_HasIntersection(&board->arvejas[idx_pea].rect, &nodo_z->zombie_data.rect)) {
                        board->arvejas[idx_pea].activo = 0;
                        nodo_z->zombie_data.vida -= 25;
                    }
                }
            }
            nodo_z = nodo_z->next;
        }
    }
    
    board->zombie_spawn_timer--;
    if (board->zombie_spawn_timer <= 0) {
        gameBoardAddZombie(board, rand() % GRID_ROWS);
        board->zombie_spawn_timer = ZOMBIE_SPAWN_RATE;
    }
}

void gameBoardDraw(GameBoard* board) {
    if (board == NULL) return;
    
    // Limpio la pantalla y dibujo el fondo completo antes de actualizar los objetos.
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, tex_background, NULL, NULL);
    
     // Recorro todas las filas del tablero para dibujar las plantas activas
    // Cada planta se representa con un frame de su spritesheet según su animación actual
    for (int fila_idx = 0; fila_idx < GRID_ROWS; fila_idx++) {
        RowSegment* segmento = board->rows[fila_idx].first_segment;
        
        while (segmento != NULL) {
            if (segmento->status == STATUS_PLANTA && segmento->planta_data != NULL) {
                Planta* shooter = segmento->planta_data;
                // Selecciono el frame de la animación que le corresponde a esta planta
                // y lo copio en la posición del tablero donde está ubicada
                SDL_Rect rect_origen = {
                    shooter->current_frame * PEASHOOTER_FRAME_WIDTH, 0,
                    PEASHOOTER_FRAME_WIDTH,
                    PEASHOOTER_FRAME_HEIGHT
                };
                SDL_RenderCopy(renderer, tex_peashooter_sheet, &rect_origen, &shooter->rect);
            }
            segmento = segmento->next;
        }
    }
    
    // Dibujo todas las arvejas que están activas
    // Cada una tiene su propio rect de posición que ya se actualiza en el update del juego
    for (int idx_pea = 0; idx_pea < MAX_ARVEJAS; idx_pea++) {
        if (board->arvejas[idx_pea].activo) {
            SDL_RenderCopy(renderer, tex_pea, NULL, &board->arvejas[idx_pea].rect);
        }
    }
    
    // Recorro las filas devuelta para dibujar los zombies.
    // Igual que con las plantas, cada zombie se dibuja con su frame actual de animación.
    for (int fila_idx = 0; fila_idx < GRID_ROWS; fila_idx++) {
        ZombieNode* nodo_z = board->rows[fila_idx].first_zombie;
        
        while (nodo_z != NULL) {
            Zombie* enemigo = &nodo_z->zombie_data;
            SDL_Rect rect_origen = {
                enemigo->current_frame * ZOMBIE_FRAME_WIDTH,
                0,
                ZOMBIE_FRAME_WIDTH,
                ZOMBIE_FRAME_HEIGHT
            };
            SDL_RenderCopy(renderer, tex_zombie_sheet, &rect_origen, &enemigo->rect);
            nodo_z = nodo_z->next;
        }
    }
    
    // Dibujo un rectángulo para marcar la celda donde está el cursor
    // Es una forma visual de mostrar al jugador en qué celda puede colocar una planta
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 200);
    SDL_Rect rect_cursor = {
        GRID_OFFSET_X + cursor.col * CELL_WIDTH,
        GRID_OFFSET_Y + cursor.row * CELL_HEIGHT,
        CELL_WIDTH,
        CELL_HEIGHT
    };
    SDL_RenderDrawRect(renderer, &rect_cursor);
    
    SDL_RenderPresent(renderer);  // Presento en pantalla todo lo que se dibujó en este ciclo.
}

// Carga una textura desde archivo y la devuelve lista para usar 
// Si falla, avisa con un mensaje de error para facilitar el debug
SDL_Texture* cargarTextura(const char* path) {
    SDL_Texture* newTexture = IMG_LoadTexture(renderer, path);
    if (newTexture == NULL) {
        printf("No se pudo cargar la textura %s! SDL_image Error: %s\n", path, IMG_GetError());
    }
    return newTexture;
}

int inicializar() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 0;
    window = SDL_CreateWindow("Plantas vs Zombies - TP2",
                              SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                              SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) return 0;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) return 0;
    if (!(IMG_Init(IMG_INIT_PNG) & IMG_INIT_PNG)) return 0;
    
    tex_background = cargarTextura("res/Frontyard.png");
    tex_peashooter_sheet = cargarTextura("res/peashooter_sprite_sheet.png");
    tex_zombie_sheet = cargarTextura("res/zombie_sprite_sheet.png");
    tex_pea = cargarTextura("res/pea.png");
    
    return 1;
}

void cerrar() {
    SDL_DestroyTexture(tex_background);
    SDL_DestroyTexture(tex_peashooter_sheet);
    SDL_DestroyTexture(tex_zombie_sheet);
    SDL_DestroyTexture(tex_pea);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    IMG_Quit();
    SDL_Quit();
}

//CASOS DE TEST PEDIDOS! :)
void Tests() {
    int num_test = 1;
    
    printf(" Tests de strDuplicate\n");
    
    // Test 1: Duplicar string vacío
    char* t1 = strDuplicate("");
    printf("Test %d: String vacío: %s\n", num_test++, (t1 && strcmp(t1, "") == 0) ? "PASS" : "FAIL");
    free(t1);
    
    // Test 2: Duplicar string de un solo carácter
    char* t2 = strDuplicate("x");
    printf("Test %d: String de un carácter: %s\n", num_test++, (t2 && strcmp(t2, "x") == 0) ? "PASS" : "FAIL");
    free(t2);
    
    // Test 3: Duplicar string con toods los caracteres validos != 0
    char* t3 = strDuplicate("Test_2025!@#");
    printf("Test %d: String con caracteres variados: %s\n", num_test++, (t3 && strcmp(t3, "Test_2025!@#") == 0) ? "PASS" : "FAIL");
    free(t3);
    
    printf("\n Tests de strCompare\n");
    
    // Test 4: Comparar dos strings vacíos
    printf("Test %d: Dos strings vacíos: %s\n", num_test++, (strCompare("", "") == 0) ? "PASS" : "FAIL");
    
    // Test 5: Comparar strings idénticos de un carácter
    printf("Test %d: Strings de 1 char iguales: %s\n", num_test++, (strCompare("z", "z") == 0) ? "PASS" : "FAIL");
    
    // Test 6 y 7: Comparar strings que difieren en un solo carácter
    // 'perro' < 'perso' porque r viene antes que s en ASCII
    int comp1 = strCompare("perro", "perso");
    printf("Test %d: 'perro' vs 'perso' (perro<perso): %s\n", num_test++, (comp1 == 1) ? "PASS" : "FAIL");
    
    // Verificar que la comparación inversa funcione bien
    int comp2 = strCompare("perso", "perro");
    printf("Test %d: 'perso' vs 'perro' (perso>perro): %s\n", num_test++, (comp2 == -1) ? "PASS" : "FAIL");
    
    // Test 8 y 9: Comparar strings completamente diferentes
    // 'gato' < 'leon' porque g viene antes que l
    int comp3 = strCompare("gato", "leon");
    printf("Test %d: 'gato' vs 'leon' (gato<leon): %s\n", num_test++, (comp3 == 1) ? "PASS" : "FAIL");
    
    int comp4 = strCompare("leon", "gato");
    printf("Test %d: 'leon' vs 'gato' (leon>gato): %s\n", num_test++, (comp4 == -1) ? "PASS" : "FAIL");
    
    printf("\n Tests de strConcatenate \n");
    
    // Test 10: Concatenar string vacío con string de 3 caracteres
    char* s10a = strDuplicate("");
    char* s10b = strDuplicate("xyz");
    char* t10 = strConcatenate(s10a, s10b);
    printf("Test %d: '' + 'xyz': %s\n", num_test++, (t10 && strcmp(t10, "xyz") == 0) ? "PASS" : "FAIL");
    free(t10);

    // Test 11: Concatenar string de 3 caracteres con string vacío
    char* s11a = strDuplicate("def");
    char* s11b = strDuplicate("");
    char* t11 = strConcatenate(s11a, s11b);
    printf("Test %d: 'def' + '': %s\n", num_test++, (t11 && strcmp(t11, "def") == 0) ? "PASS" : "FAIL");
    free(t11);

    // Test 12: Concatenar dos strings de un carácter
    char* s12a = strDuplicate("m");
    char* s12b = strDuplicate("n");
    char* t12 = strConcatenate(s12a, s12b);
    printf("Test %d: 'm' + 'n': %s\n", num_test++, (t12 && strcmp(t12, "mn") == 0) ? "PASS" : "FAIL");
    free(t12);

    // Test 13: Concatenar dos strings de 5 caracteres
    char* s13a = strDuplicate("planta");
    char* s13b = strDuplicate("zombie");
    char* t13 = strConcatenate(s13a, s13b);
    printf("Test %d: 'planta' + 'zombie': %s\n", num_test++, (t13 && strcmp(t13, "plantazombie") == 0) ? "PASS" : "FAIL");
    free(t13);

    printf("\n Tests de gameBoardAddPlant \n");
    GameBoard* tablero_test = gameBoardNew();

    // Test 14: Agregar planta en el medio de una fila vacía (columna 4)
    int r14 = gameBoardAddPlant(tablero_test, 0, 4);
    printf("Test %d: Agregar planta en col 4 (medio): %s\n", num_test++, r14 ? "PASS" : "FAIL");

    // Test 15: Agregar planta en el extremo izquierdo (columna 0)
    int r15 = gameBoardAddPlant(tablero_test, 1, 0);
    printf("Test %d: Agregar planta en col 0 (inicio): %s\n", num_test++, r15 ? "PASS" : "FAIL");

    // Test 16: Agregar planta en el extremo derecho (columna 8)
    int r16 = gameBoardAddPlant(tablero_test, 1, 8);
    printf("Test %d: Agregar planta en col 8 (final): %s\n", num_test++, r16 ? "PASS" : "FAIL");

    // Test 17: Llenar completamente una fila con plantas
    GameBoard* tablero_test2 = gameBoardNew();
    int todas_ok = 1;
    for (int columna = 0; columna < GRID_COLS; columna++) {
        if (!gameBoardAddPlant(tablero_test2, 2, columna)) todas_ok = 0;
    }
    printf("Test %d: Llenar fila completa (9 plantas): %s\n", num_test++, todas_ok ? "PASS" : "FAIL");
    gameBoardDelete(tablero_test2);

    // Test 18: Intentar agregar planta en celda ya ocupada
    int r18 = gameBoardAddPlant(tablero_test, 0, 4);
    printf("Test %d: Agregar en celda ocupada (debe fallar): %s\n", num_test++, (!r18) ? "PASS" : "FAIL");

    gameBoardDelete(tablero_test);

    printf("\n Tests de gameBoardRemovePlant \n");
    GameBoard* tablero_test3 = gameBoardNew();

    // Test 19: Plantar en columnas consecutivas (3, 4, 5), eliminar la del medio (4)
    gameBoardAddPlant(tablero_test3, 0, 3);
    gameBoardAddPlant(tablero_test3, 0, 4);
    gameBoardAddPlant(tablero_test3, 0, 5);
    gameBoardRemovePlant(tablero_test3, 0, 4);
    int r19 = gameBoardAddPlant(tablero_test3, 0, 4);
    printf("Test %d: Eliminar planta del medio y re-agregar: %s\n", num_test++, r19 ? "PASS" : "FAIL");

    // Test 20: Eliminar planta en col 3 después de haber eliminado la de col 4
    gameBoardRemovePlant(tablero_test3, 0, 3);
    int r20 = gameBoardAddPlant(tablero_test3, 0, 3);
    printf("Test %d: Eliminar col 3 tras eliminar col 4 (fusión): %s\n", num_test++, r20 ? "PASS" : "FAIL");

    gameBoardDelete(tablero_test3);
    
    // Test 21: Llenar fila completa, eliminar una del medio, re-agregar
    GameBoard* tablero_test4 = gameBoardNew();
    for (int columna = 0; columna < GRID_COLS; columna++) {
        gameBoardAddPlant(tablero_test4, 0, columna);
    }
    gameBoardRemovePlant(tablero_test4, 0, 4);
    int r21 = gameBoardAddPlant(tablero_test4, 0, 4);
    printf("Test %d: Fila llena, eliminar medio, re-agregar: %s\n", num_test++, r21 ? "PASS" : "FAIL");
    gameBoardDelete(tablero_test4);

    printf("\n Tests de gameBoardAddZombie \n");
    GameBoard* tablero_test5 = gameBoardNew();
    
    // Test 22: Agregar un zombie a una lista que ya tiene 3 zombies
    gameBoardAddZombie(tablero_test5, 0);
    gameBoardAddZombie(tablero_test5, 0);
    gameBoardAddZombie(tablero_test5, 0);
    int conteo_previo = 0;
    ZombieNode* nodo_z = tablero_test5->rows[0].first_zombie;
    while (nodo_z) { conteo_previo++; nodo_z = nodo_z->next; }
    gameBoardAddZombie(tablero_test5, 0);
    int conteo_posterior = 0;
    nodo_z = tablero_test5->rows[0].first_zombie;
    while (nodo_z) { conteo_posterior++; nodo_z = nodo_z->next; }
    printf("Test %d: Agregar 4to zombie a lista de 3: %s\n", num_test++,
        (conteo_previo == 3 && conteo_posterior == 4) ? "PASS" : "FAIL");
    gameBoardDelete(tablero_test5);

    // Test 23: Crear una lista de 10000 zombies
    GameBoard* tablero_test6 = gameBoardNew();
    for (int idx = 0; idx < 10000; idx++) {
        gameBoardAddZombie(tablero_test6, 1);
    }
    int conteo_10k = 0;
    nodo_z = tablero_test6->rows[1].first_zombie;
    while (nodo_z) { conteo_10k++; nodo_z = nodo_z->next; }
    printf("Test %d: Crear lista de 10000 zombies: %s\n", num_test++,
        (conteo_10k == 10000) ? "PASS" : "FAIL");
    gameBoardDelete(tablero_test6);
    
    printf("\n Fin de los tests!!! :) \n");
}

 // main q replica la logica para ver si el juego termino al haber llegado un zombie a la casa
int main(int argc, char* args[]) {
    //no se usan en el codigo
    (void)argc;
    (void)args;
    Tests();
    
    printf("Presiona Enter para iniciar el juego...\n");
    getchar();
    
    srand(time(NULL)); //para q se creen zombies
    if (!inicializar()) {
        printf("Error al inicializar SDL!\n");
        return 1;
    }

    game_board = gameBoardNew();
    if (!game_board) {
        printf("Error al crear el tablero!\n");
        cerrar();
        return 1;
    }

    SDL_Event evento;
    int juego_terminado = 0;

    while (!juego_terminado) {
        while (SDL_PollEvent(&evento) != 0) {
            if (evento.type == SDL_QUIT) {
                juego_terminado = 1;
            }
            
            if (evento.type == SDL_MOUSEMOTION) {
                int pos_mouse_x = evento.motion.x;
                int pos_mouse_y = evento.motion.y;
                if (pos_mouse_x >= GRID_OFFSET_X && pos_mouse_x < GRID_OFFSET_X + GRID_WIDTH &&
                    pos_mouse_y >= GRID_OFFSET_Y && pos_mouse_y < GRID_OFFSET_Y + GRID_HEIGHT) {
                    cursor.col = (pos_mouse_x - GRID_OFFSET_X) / CELL_WIDTH;
                    cursor.row = (pos_mouse_y - GRID_OFFSET_Y) / CELL_HEIGHT;
                }
            }
            
            if (evento.type == SDL_MOUSEBUTTONDOWN) {
                gameBoardAddPlant(game_board, cursor.row, cursor.col);
            }
        }

        gameBoardUpdate(game_board);
        gameBoardDraw(game_board);

        for (int fila_idx = 0; fila_idx < GRID_ROWS; fila_idx++) {
            ZombieNode* nodo_z = game_board->rows[fila_idx].first_zombie;
            while (nodo_z != NULL) {
                if (nodo_z->zombie_data.rect.x < GRID_OFFSET_X - nodo_z->zombie_data.rect.w) {
                    printf("\n¡GAME OVER! - Un zombie llegó a tu casa!\n");
                    juego_terminado = 1;
                    break;
                }
                nodo_z = nodo_z->next;
            }
            if (juego_terminado) break;
        }

        SDL_Delay(16);
    }

    gameBoardDelete(game_board);
    cerrar();
    return 0;
}

