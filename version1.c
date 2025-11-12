/*
Indique el porcentaje aproximado de l´ıneas de c´odigo del trabajo pr´actico que fueron
realizadas con asistencia de una IA.

Aproximadamente entre un 20 % y un 30 % del código fue desarrollado con asistencia de herramientas de inteligencia artificial. 
Su uso se concentró principalmente en la generación de los tests, la implementación del main y en la comprensión del funcionamiento de la función gameBoardDraw.

¿C´omo verificaron que las sugerencias de la IA eran correctas?

La verificación de la corrección de las sugerencias de la IA se realizó mediante la ejecución y prueba del código, 
observando que el comportamiento obtenido coincidiera con el esperado.

¿Se enfrentaron a alguna dificultad al utilizar las herramientas de IA? ¿C´omo las resolvieron?

No se presentaron dificultades significativas durante el uso de las herramientas de inteligencia artificial, 
por lo que no fue necesario aplicar soluciones adicionales.

¿Consideran que el uso de la IA les ha permitido desarrollar habilidades de programaci´on
en C? ¿Por qu´e?

En cierta medida, el uso de la IA contribuyó al desarrollo de habilidades de programación en C, 
especialmente en lo referente a la comprensión de aspectos sintácticos del lenguaje. No obstante, su aporte fue limitado en cuanto al razonamiento lógico necesario para la implementación de las funciones.

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
    int len = 0;
    while (s && s[len] != '\0'){
        len++;
    } 
    return len;
}

//en la funcion strDup recorremos el string a traves de un ciclo con la condicion de que sea menor o igual al strLen de la palabra, pidiendo memoria para duplicarlo y no perder el string pasado por parametro. 
char* strDuplicate(char* src) { 
    if (!src) return NULL;// Verificar entrada
    
    int largopalabra = strLen(src);
    char* dup = (char*)malloc(sizeof(char) * (largopalabra + 1));
    
    if (!dup) return NULL;
    
    int i = 0;
    while (i <= largopalabra) {
        dup[i] = src[i];
        i++;
    }
    dup[largopalabra] = '\0';
    return dup;
}

//en la funcion strCmp definimos varios casos base, y luego a traves de un ciclo comparamos para calcular su orden lexicografico 
int strCompare(char* a, char* b){
    if(a == NULL && b != NULL){
        return 1;
    }
    if(a != NULL && b == NULL){
        return -1;
    }
    if(a == NULL && b == NULL){
        return 0;
    }
    int i=0;
    while( a[i] != '\0' && b[i] != '\0'){
        if(a[i] < b[i]){
            return 1;
        }
        if(a[i] > b[i]){
            return -1;
        }
        i++; 
    }
    
    if(a[i] == '\0' && b[i] == '\0'){
        return 0;
    }
    if(a[i] == '\0'){
        return 1; 
    }
    else{
        return -1;
    }
}

/*
Concatena los char teniendo en cuenta la liberacion de memoria de los char pasados por parametro
*/
char* strConcatenate(char* src1, char* src2) {
    if (!src1 || !src2) return NULL;
    
    int len1 = 0; 
    int len2 = 0;

    while (src1[len1] != '\0') len1++;
    while (src2[len2] != '\0') len2++;

    char* concatenado = (char*)malloc((len1 + len2 + 1) * sizeof(char));
    if (!concatenado) return NULL;
    
    for (int i = 0; i < len1; i++) {
        concatenado[i] = src1[i];
    }

    for (int j = 0; j < len2; j++) {
        concatenado[len1 + j] = src2[j];
    }
    
    concatenado[len1 + len2] = '\0';
    free(src1);
    free(src2);
    return concatenado;
}

/*
Crea el tablero pidiendo memoria para cada estructura requerida. Por cada row, inicializa el primer y unico segmento hasta ahora, vacio.
Y asigna a cada row al primer segmento, y NULL al primer zombie.
Esto soluciona ya que genera el tablero vacio.
*/

GameBoard* gameBoardNew() {
    GameBoard* board = malloc(sizeof(GameBoard));//pedimos mem para el boards
    if (!board) return NULL;//por si llegara a fallar malloc

    board->zombie_spawn_timer = ZOMBIE_SPAWN_RATE; //temporizador para aparicion de zombiess
    for (int i = 0; i < GRID_ROWS; i++) {
        RowSegment* first = malloc(sizeof(RowSegment));//pedimos mem para el row por cada i
        if (!first) {
            // liberar lo ya creado antes de salir si llega a fallar el malloc
            for (int j = 0; j < i; j++) {
                free(board->rows[j].first_segment);
            }
            free(board);
            return NULL;
            //caso de si llega a fallar el pedido de mem
        }


        first->status = STATUS_VACIO;
        first->start_col = 0;
        first->length = GRID_COLS;
        first->planta_data = NULL;
        first->next = NULL;

        //guardamos el puntero al primer seguenmento
        board->rows[i].first_segment = first;
        //inicializamos la lista de zombies vacia
        board->rows[i].first_zombie = NULL;
    }
    //todas las arvejas las inicializamos como inactivas
    for (int i = 0; i < MAX_ARVEJAS; i++)
        board->arvejas[i].activo = 0;

    return board;
}


/*
funcion aux que borre las rows por separado
Libera cada row teniendo en cuenta si tiene una planta o no. Si tiene una planta, libera tmb la memoria de la planta, y 
luego libera cada zombie node, Liberando asi todo.
*/
void gameRowDelete(GardenRow* row){
    RowSegment* actual_seg= row->first_segment;
    while(actual_seg != NULL){
        RowSegment* sig = actual_seg->next;//nos guardamos el siguiente
        
        if (actual_seg->planta_data != NULL) {
            free(actual_seg->planta_data);
        }//si hay planta, liberamos la memoria de la planta
        
        free(actual_seg);//liberamos el segmento
        actual_seg = sig;//avanzamos
    }

    ZombieNode* z = row->first_zombie;
    while (z) {
        ZombieNode* next = z->next;//guardamos el siguiente
        free(z); //liberamos los zombies
        z = next;//avanzamos
    }
}
/*
Recorre cada row, y utiliza una funcion auxiliar que borre linea por linea, y luego libera el board.
Esto soluciona, dado a que libera dentro de cada row cada segmento(y si tiene planta tambien elimina esa planta). 
Y luego de eliminar cada row, recien ahi libera el board, borrando asi todo.

*/
void gameBoardDelete(GameBoard* board) {
    // TODO: Liberar toda la memoria dinámica.
    // TODO: Recorrer cada GardenRow.
    // TODO: Liberar todos los RowSegment (y los planta_data si existen).
    // TODO: Liberar todos los ZombieNode.
    // TODO: Finalmente, liberar el GameBoard.
    //printf("Función gameBoardDelete no implementada.\n");
        
    if (!board) return;
    
    //ciclo para llegar al final de first segment, OBS: Sabemos por parametro que son 5
    for(int i=0; i<GRID_ROWS; i++){//ciclo para recorrer recorrer hasta el final X CADA NODO
        gameRowDelete(&board->rows[i]);//le estoy pasando la direccion de memoria de donde estan el puntero
    
    }
    free(board);//liberamos todo el board
}


/*funcion aux que indica si el rowsegment "actual" contiene la columna pasada por parametro */
int contiene(RowSegment* actual, int col){
    if(col >= actual->start_col && col < (actual->start_col + actual->length)){ //princpio<= col <fin 
        return 1;
    }
    else return 0;
}

/*Función que divide un segmento vacío para agregar una planta
 Retorna 1 si se agregó exitosamente, 0 si no se pudo.
 Funcion que divide los segmentos dependiendo de donde se quiere plantar. Se fija si contiene una planta ya, y si no contiene, divide el segmento dejando
 al segmento donde se va a plantar de longitud 1, y con los datos de la nueva planta. Esto mantiene la estructura de la consigna dado a que esta funcion no permite lo siguiente:
 -Segmentos vacios consecutivos(si estan vacios y son consecutivos tienen que estar juntos en uno)
 -Que haya segmentos con plantas pero con longitud mayor a 1
 */
int DivisionSeg(GameBoard* board, GardenRow* row, int row_index, int col){
   (void)board;//parametros que al final no utilizamos pero nos dio cosa borrarlos(cavala), pero los hacemos pasar como void para que no molesten
   (void)row;
   RowSegment* actual= row->first_segment;
   RowSegment* prev= NULL;
   
   while(actual != NULL && !contiene(actual, col)){
        prev= actual;
        actual= actual->next;
    }// cuando termine este ciclo, actual va a ser el nodo que contenga a la col


    if(actual == NULL || actual->status == STATUS_PLANTA) {
        return 0;
    }
    
    //creo planta
    // 1. Pedir mem
    Planta* p = (Planta*)malloc(sizeof(Planta));
    if (!p) return 0;
    
    // 2. "Construir" la planta (llenar sus campos)
    //si hay una planta 
    p->rect.x = GRID_OFFSET_X + (col * CELL_WIDTH);
    p->rect.y = GRID_OFFSET_Y + (row_index * CELL_HEIGHT);
    p->rect.w = CELL_WIDTH;
    p->rect.h = CELL_HEIGHT;
    p->activo = 1;
    p->cooldown = rand() % 100;
    p->current_frame = 0;
    p->frame_timer = 0;
    p->debe_disparar = 0;
    

    //caso primer elem es el que divido [1,2,3] -> [1], [2,3]
    if(col==actual->start_col){
        RowSegment* nuevo= (RowSegment*)malloc(sizeof(RowSegment));//creo nuevo
        if (!nuevo) {
            free(p);
            return 0;
        }
        
        nuevo->next= actual; //lo conecto con el que estoy separando

        if(actual==row->first_segment){
            row->first_segment= nuevo;
        }
        else{
            prev->next= nuevo;
        }
       
        actual->start_col= (actual->start_col+1);
        actual->length= (actual->length-1);

        //ahora modifico los datos del nuevo
        nuevo->status= STATUS_PLANTA;
        nuevo->start_col= col;
        nuevo->length= 1;
        nuevo->planta_data= p;

        return 1;
    }


    else if(col == actual->start_col + actual->length - 1){
        RowSegment* nuevo= (RowSegment*)malloc(sizeof(RowSegment));
        if (!nuevo) {
            free(p);
            return 0;
        }
        
        RowSegment* sig= actual->next;
        
        nuevo->next= sig;
        actual->next= nuevo;
        
        actual->length= (actual->length-1);
        
        nuevo->planta_data= p;
        nuevo->status= STATUS_PLANTA;
        nuevo->start_col= col;
        nuevo->length= 1;
        return 1;
    }
    else{
        RowSegment* dondePlanta= (RowSegment*)malloc(sizeof(RowSegment)); //pido mem para plantar
        if (!dondePlanta) {
            free(p);
            return 0;
        }
        
        RowSegment* dspPlanta= (RowSegment*)malloc(sizeof(RowSegment));//pido mem para el segmento siguiente al que planta
        if (!dspPlanta) {
            free(dondePlanta);
            free(p);
            return 0;
        }

        int start_original = actual->start_col;
        int length_original = actual->length;

        dspPlanta->next= actual->next;
        actual->next= dondePlanta;
        dondePlanta->next= dspPlanta;

        actual->length= col - start_original;

        //actualizo datos del segmento donde se planta
        dondePlanta->start_col= col;
        dondePlanta->length= 1;
        dondePlanta->status= STATUS_PLANTA;
        dondePlanta->planta_data= p;
        //actualizo datos del segmento siguiente a donde se planta
        dspPlanta->start_col= col+1;
        dspPlanta->length= (start_original + length_original) - (col + 1);
        dspPlanta->status= STATUS_VACIO;
        dspPlanta->planta_data= NULL;

        return 1;
    }
}

/*
Funcion que se fija si es una row valida. Y si es, llama a una funcion auxiliar que en base a donde se quieea plantar divide los segmentos manteniendo 
la estructura pedida en el enunciado
*/
int gameBoardAddPlant(GameBoard* board, int row, int col) {
    // TODO: Encontrar la GardenRow correcta.
    // TODO: Recorrer la lista de RowSegment hasta encontrar el segmento VACIO que contenga a `col`.
    // TODO: Si se encuentra y tiene espacio, realizar la lógica de DIVISIÓN de segmento.
    // TODO: Crear la nueva `Planta` con memoria dinámica y asignarla al `planta_data` del nuevo segmento.
    //printf("Función gameBoardAddPlant no implementada.\n");
    if(row >= GRID_ROWS || row < 0 || col >= GRID_COLS || col < 0){
        return 0;
    }
    
    GardenRow* RowDada= &board->rows[row];
    return DivisionSeg(board, RowDada, row, col);
}



 /*
Funcion que en base a donde esta la planta que se quiere borrar, unifica los segmentos en los siguientes casos.
Soluciona ya que si no hay una planta, no hace nada, y si hay una planta, divide teniendo en cuenta los siguientes casos
(graficamos teniendo en cuenta que se quiere borrar la planta del medio):

casos: 

[planta], [planta], [planta] => [planta], [vacio], [planta]

[vacio], [planta], [planta] => [vacio, vacio], [planta]

[planta], [planta], [vacio] => [planta], [vacio, vacio]

[vacio], [planta], [vacio] => [vacio, vacio, vacio]
---------

El caso de si es el primer row segment se trato asi:

si la condicion prev!=NULL da falsa, es que es el primero(prev==null)
Entonces, si se quiere borrar el actual, y el siguiente es vacio, entra en el ultimo else if. Donde se unifica el siguiente con el actual, 
pero manteniendo la informacion del actual, por lo que no hay que modificar la informacion del row->first_segment

Y el caso del ultimo segmento, es cuando la condicion next != null da falsa. Donde se libera el actual y se pone prev->next= NULL

*/


void unificacionSeg(GardenRow* row, int col){
   
    RowSegment* actual= row->first_segment;
    RowSegment* prev= NULL;

    while(actual != NULL && actual->start_col != col){
        prev= actual;
        actual= actual->next;
    } //actual queda en la posicion requerida cuando se rompe la guarda

    if(actual == NULL || actual->status != STATUS_PLANTA){
        return;
    }

    //si tenemos 3 plantas consecutivas y queremos liberar solo la del medio, solamente libera esa y no entra en ningun caso
    //[planta], [planta], [planta] => [planta], [vacio], [planta]
    free(actual->planta_data);//liberamos mem de la planta
    actual->planta_data = NULL; //pongo en NULL 
    actual->status = STATUS_VACIO;//actualizo dato porlas
    
    int prev_vacio = (prev != NULL && prev->status == STATUS_VACIO);
    int next_vacio = (actual->next != NULL && actual->next->status == STATUS_VACIO);
    //estas son 0 o 1

    //si el anterior y el siguiente estan vacios ->unifica anterior, actual y siguiente
    //
    if (prev_vacio && next_vacio) {
        RowSegment* sig = actual->next;
        prev->length = prev->length + actual->length + sig->length;
        prev->next = sig->next;
        free(actual);//libera actual y siguiente y utiliza el anterior actualizando todos sus datos
        free(sig);
    }
    //vacio el anterior y el siguiente no ->unifica el anterior y actual
    //[vacio], [planta], [planta] => [vacio, vacio], [planta]
    else if (prev_vacio && !next_vacio) { 
        prev->length = prev->length + actual->length;
        prev->next = actual->next;
        free(actual); //solamente libera el actual unificandolo con el anterior y actualizando los datos
    }
    //si el anterior no esta vacio pero el siguiente si ->unifica el actual y el siguiente
    //[planta], [planta], [vacio] => [planta], [vacio, vacio]
    else if (!prev_vacio && next_vacio) {
        RowSegment* sig = actual->next;
        actual->length = actual->length + sig->length;
        actual->next = sig->next;
        free(sig);//libera el siguiente unificando en actual y actulizando los datos en el actual
    }
}

/*
Se le pasa el gardenrow requerido a una funcion auxiliar que unifica los segmentos segun 
si los segmentos anterior y siguientes tienen plantas o no, o si son el primer o ultimo segmento

*/

void gameBoardRemovePlant(GameBoard* board, int row, int col) {
    // TODO: Similar a AddPlant, encontrar el segmento que contiene `col`.
    // TODO: Si es un segmento de tipo PLANTA, convertirlo a VACIO y liberar el `planta_data`.
    // TODO: Implementar la lógica de FUSIÓN con los segmentos vecinos si también son VACIO.
    //[vacio vacio], [planta], [vacio] -> [vacio, vacio, vacio, vacio]
    //printf("Función gameBoardRemovePlant no implementada.\n");
    if(row >= GRID_ROWS || row < 0 || col >= GRID_COLS || col < 0){
        return;
    }
    
    GardenRow* RowDada= &board->rows[row];
    unificacionSeg(RowDada, col);
}



/*
Crea un nuevo zombie, y lo an~ade ultimo a la lista first_zombie del gardenRow adecuado.
Funciona dado a que si es el primer zombie(first_zombie==NULL), se lo asigna, y sino recorre la lista enlazada hasta que llegue a next->NULL, y el siguiente a ese 
ya esta apuntando a NULL dado a como se inicializo el zombie

*/
void gameBoardAddZombie(GameBoard* board, int row) {
    // TODO: Crear un nuevo ZombieNode con memoria dinámica.
    // TODO: Inicializar sus datos (posición, vida, animación, etc.).
    // TODO: Agregarlo a la lista enlazada simple de la GardenRow correspondiente.
    //printf("Función gameBoardAddZombie no implementada.\n");
    if (row < 0 || row >= GRID_ROWS) return;
    
    ZombieNode* nuevo= (ZombieNode*)malloc(sizeof(ZombieNode));
    if (!nuevo) return; //si falla el pedido de memoria
    
    nuevo->zombie_data.pos_x = GRID_OFFSET_X + GRID_WIDTH - (ZOMBIE_FRAME_WIDTH / 2);
    nuevo->zombie_data.rect.x = (int)nuevo->zombie_data.pos_x;
    nuevo->zombie_data.rect.y = GRID_OFFSET_Y + (row * CELL_HEIGHT) - 20;
    nuevo->zombie_data.rect.w = 80;
    nuevo->zombie_data.rect.h = 100;
    nuevo->zombie_data.activo = 1;
    nuevo->zombie_data.vida = 100;
    nuevo->zombie_data.row = row;
    nuevo->zombie_data.current_frame = 0;
    nuevo->zombie_data.frame_timer = 0;
    nuevo->next = NULL;

    ZombieNode* actual= board->rows[row].first_zombie;
    
    if (actual == NULL) {
        board->rows[row].first_zombie = nuevo;
        return;
    }
    
    ZombieNode* prev= NULL;
    while(actual != NULL){
        prev= actual;
        actual= actual->next; 
    }

    prev->next= nuevo;
}



/*
Funciona dado a que tiene en cuenta lo siguiente:
-movimiento del zombie
-animacion del zombie
-eliminar zombie si murio
-cooldown de plantas
-animacion de plantas
-disparos de plantas
-generacion de zombies
=>las que tienen movientos(zombies y disparos de plantas), se tienen en cuenta las actualizaciones de los frames
*/
void gameBoardUpdate(GameBoard* board) {
    // TODO: Re-implementar la lógica de `actualizarEstado` usando las nuevas estructuras.
    // TODO: Recorrer las listas de zombies de cada fila para moverlos y animarlos.
    // TODO: Recorrer las listas de segmentos de cada fila para gestionar los cooldowns y animaciones de las plantas.
    // TODO: Actualizar la lógica de disparo, colisiones y spawn de zombies.
    if (!board) return;
    
    for (int r = 0; r < GRID_ROWS; r++) {
        //declaracion zombies
        ZombieNode* znode = board->rows[r].first_zombie;
        ZombieNode* prev_znode = NULL;
        
        while (znode != NULL) {
            Zombie* z = &znode->zombie_data;
            
            //mover xombie
            float distance_per_tick = ZOMBIE_DISTANCE_PER_CYCLE / (float)(ZOMBIE_TOTAL_FRAMES * ZOMBIE_ANIMATION_SPEED);
            z->pos_x -= distance_per_tick;
            z->rect.x = (int)z->pos_x;
            
            //animar zombie
            z->frame_timer++;
            if (z->frame_timer >= ZOMBIE_ANIMATION_SPEED) {
                z->frame_timer = 0;
                z->current_frame = (z->current_frame + 1) % ZOMBIE_TOTAL_FRAMES;
            }
            
            //fletar zombie si murio
            if (z->vida <= 0) {
                ZombieNode* to_delete = znode;
                if (prev_znode == NULL) {
                    board->rows[r].first_zombie = znode->next;
                } else {
                    prev_znode->next = znode->next;
                }
                znode = znode->next;
                free(to_delete);
                continue;
            }
            
            prev_znode = znode;
            znode = znode->next;
        }
    }
    

    //actualizar plantas(cooldown, animacion y disparos)
    for (int r = 0; r < GRID_ROWS; r++) {
        RowSegment* seg = board->rows[r].first_segment;
        
        while (seg != NULL) {
            if (seg->status == STATUS_PLANTA && seg->planta_data != NULL) {
                Planta* p = seg->planta_data;
                
                //cooldown
                if (p->cooldown <= 0) {
                    p->debe_disparar = 1;
                } else {
                    p->cooldown--;
                }
                
                //animacion
                p->frame_timer++;
                if (p->frame_timer >= PEASHOOTER_ANIMATION_SPEED) {
                    p->frame_timer = 0;
                    p->current_frame = (p->current_frame + 1) % PEASHOOTER_TOTAL_FRAMES;
                    
                    //disparo de arvejas
                    if (p->debe_disparar && p->current_frame == PEASHOOTER_SHOOT_FRAME) {
                        for (int i = 0; i < MAX_ARVEJAS; i++) {
                            if (!board->arvejas[i].activo) {
                                board->arvejas[i].rect.x = p->rect.x + (CELL_WIDTH / 2);
                                board->arvejas[i].rect.y = p->rect.y + (CELL_HEIGHT / 4);
                                board->arvejas[i].rect.w = 20;
                                board->arvejas[i].rect.h = 20;
                                board->arvejas[i].activo = 1;
                                break;
                            }
                        }
                        p->cooldown = 120;
                        p->debe_disparar = 0;
                    }
                }
            }
            seg = seg->next;
        }
    }
    
    //actualizar arvejas
    for (int i = 0; i < MAX_ARVEJAS; i++) {
        if (board->arvejas[i].activo) {
            board->arvejas[i].rect.x += PEA_SPEED;
            if (board->arvejas[i].rect.x > SCREEN_WIDTH) {
                board->arvejas[i].activo = 0;
            }
        }
    }
    
    for (int r = 0; r < GRID_ROWS; r++) {
        ZombieNode* znode = board->rows[r].first_zombie;
        
        while (znode != NULL) {
            for (int i = 0; i < MAX_ARVEJAS; i++) {
                if (!board->arvejas[i].activo) continue;
                
                int arveja_row = (board->arvejas[i].rect.y - GRID_OFFSET_Y) / CELL_HEIGHT;
                if (r == arveja_row) {
                    if (SDL_HasIntersection(&board->arvejas[i].rect, &znode->zombie_data.rect)) {
                        board->arvejas[i].activo = 0;
                        znode->zombie_data.vida -= 25;
                    }
                }
            }
            znode = znode->next;
        }
    }
    
    //generador de zombies
    board->zombie_spawn_timer--;
    if (board->zombie_spawn_timer <= 0) {
        gameBoardAddZombie(board, rand() % GRID_ROWS);
        board->zombie_spawn_timer = ZOMBIE_SPAWN_RATE;
    }
}

/*
Cumple con lo siguiente:
-renderizacion de las arvejas
-renderizacion de las plantas
-renderizacion de los zombies
-renderizacion del cursor

*/
void gameBoardDraw(GameBoard* board) {
    if (!board) return;
    
    SDL_RenderClear(renderer);
    SDL_RenderCopy(renderer, tex_background, NULL, NULL);
    
    //dibujo plantas
    for (int r = 0; r < GRID_ROWS; r++) {
        RowSegment* seg = board->rows[r].first_segment;
        
        while (seg != NULL) {
            if (seg->status == STATUS_PLANTA && seg->planta_data != NULL) {
                Planta* p = seg->planta_data;
                SDL_Rect src_rect = {
                    p->current_frame * PEASHOOTER_FRAME_WIDTH,
                    0,
                    PEASHOOTER_FRAME_WIDTH,
                    PEASHOOTER_FRAME_HEIGHT
                };
                SDL_RenderCopy(renderer, tex_peashooter_sheet, &src_rect, &p->rect);
            }
            seg = seg->next;
        }
    }
    
    //dibujo arvejas
    for (int i = 0; i < MAX_ARVEJAS; i++) {
        if (board->arvejas[i].activo) {
            SDL_RenderCopy(renderer, tex_pea, NULL, &board->arvejas[i].rect);
        }
    }
    
    //dibujo zombie
    for (int r = 0; r < GRID_ROWS; r++) {
        ZombieNode* znode = board->rows[r].first_zombie;
        
        while (znode != NULL) {
            Zombie* z = &znode->zombie_data;
            SDL_Rect src_rect = {
                z->current_frame * ZOMBIE_FRAME_WIDTH,
                0,
                ZOMBIE_FRAME_WIDTH,
                ZOMBIE_FRAME_HEIGHT
            };
            SDL_RenderCopy(renderer, tex_zombie_sheet, &src_rect, &z->rect);
            znode = znode->next;
        }
    }
    
    //dibujar cursor
    SDL_SetRenderDrawColor(renderer, 255, 255, 0, 200);
    SDL_Rect cursor_rect = {
        GRID_OFFSET_X + cursor.col * CELL_WIDTH,
        GRID_OFFSET_Y + cursor.row * CELL_HEIGHT,
        CELL_WIDTH,
        CELL_HEIGHT
    };
    SDL_RenderDrawRect(renderer, &cursor_rect);
    
    SDL_RenderPresent(renderer);
}

//FUNCIONES SDL

/*
Cargan las texturas y manda aviso si no se pudo cargar textura

*/
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

// ========= TESTS =========

void ejecutarTests() {
    printf("\n========== EJECUTANDO TESTS ==========\n\n");
    int test_num = 1;
    
    // ===== TESTS DE strDuplicate =====
    printf("--- Tests de strDuplicate ---\n");
    
    // Test 1: String vacío
    char* test1 = strDuplicate("");
    printf("Test %d: String vacío: %s\n", test_num++, (test1 && strcmp(test1, "") == 0) ? "PASS" : "FAIL");
    free(test1);
    
    // Test 2: String de un carácter
    char* test2 = strDuplicate("a");
    printf("Test %d: String de un carácter: %s\n", test_num++, (test2 && strcmp(test2, "a") == 0) ? "PASS" : "FAIL");
    free(test2);
    
    // Test 3: String con caracteres variados
    char* test3 = strDuplicate("Hola123!@#$%");
    printf("Test %d: String variado: %s\n", test_num++, (test3 && strcmp(test3, "Hola123!@#$%") == 0) ? "PASS" : "FAIL");
    free(test3);
    
    // ===== TESTS DE strCompare =====
    printf("\n--- Tests de strCompare ---\n");
    
    // Test 4: Dos strings vacíos
    printf("Test %d: Dos strings vacíos: %s\n", test_num++, (strCompare("", "") == 0) ? "PASS" : "FAIL");
    
    // Test 5: Dos strings de un carácter iguales
    printf("Test %d: Strings de 1 char iguales: %s\n", test_num++, (strCompare("a", "a") == 0) ? "PASS" : "FAIL");
    
    // Test 6: Strings iguales hasta un carácter (s1 < s2)
    int cmp1 = strCompare("abc", "abd");
    printf("Test %d: 'abc' vs 'abd' (abc<abd): %s\n", test_num++, (cmp1 == 1) ? "PASS" : "FAIL");
    
    // Test 7: Strings iguales hasta un carácter (s2 < s1)
    int cmp2 = strCompare("abd", "abc");
    printf("Test %d: 'abd' vs 'abc' (abd>abc): %s\n", test_num++, (cmp2 == -1) ? "PASS" : "FAIL");
    
    // Test 8: Strings diferentes (s1 < s2)
    int cmp3 = strCompare("hola", "mundo");
    printf("Test %d: 'hola' vs 'mundo' (hola<mundo): %s\n", test_num++, (cmp3 == 1) ? "PASS" : "FAIL");
    
    // Test 9: Strings diferentes (s2 < s1)
    int cmp4 = strCompare("mundo", "hola");
    printf("Test %d: 'mundo' vs 'hola' (mundo>hola): %s\n", test_num++, (cmp4 == -1) ? "PASS" : "FAIL");
    
    // ===== TESTS DE strConcatenate =====
    printf("\n--- Tests de strConcatenate ---\n");
    
   // Test 10: String vacío + string de 3 caracteres
    char* str10a = strDuplicate("");
    char* str10b = strDuplicate("abc");
    char* test10 = strConcatenate(str10a, str10b);
    printf("Test %d: '' + 'abc': %s\n", test_num++, (test10 && strcmp(test10, "abc") == 0) ? "PASS" : "FAIL");
    free(test10);

    // Test 11: String de 3 caracteres + string vacío
    char* str11a = strDuplicate("xyz");
    char* str11b = strDuplicate("");
    char* test11 = strConcatenate(str11a, str11b);
    printf("Test %d: 'xyz' + '': %s\n", test_num++, (test11 && strcmp(test11, "xyz") == 0) ? "PASS" : "FAIL");
    free(test11);

    // Test 12: Dos strings de 1 carácter
    char* str12a = strDuplicate("a");
    char* str12b = strDuplicate("b");
    char* test12 = strConcatenate(str12a, str12b);
    printf("Test %d: 'a' + 'b': %s\n", test_num++, (test12 && strcmp(test12, "ab") == 0) ? "PASS" : "FAIL");
    free(test12);

    // Test 13: Dos strings de 5 caracteres
    char* str13a = strDuplicate("hello");
    char* str13b = strDuplicate("world");
    char* test13 = strConcatenate(str13a, str13b);
    printf("Test %d: 'hello' + 'world': %s\n", test_num++, (test13 && strcmp(test13, "helloworld") == 0) ? "PASS" : "FAIL");
    free(test13);
    // ===== TESTS DE gameBoardAddPlant =====
    printf("\n--- Tests de gameBoardAddPlant ---\n");
    GameBoard* test_board = gameBoardNew();  // nuevo

    // Test 14: Agregar planta en el medio de fila vacía
    int res14 = gameBoardAddPlant(test_board, 0, 4);
    printf("Test %d: Agregar planta en col 4: %s\n", test_num++, res14 ? "PASS" : "FAIL");

    // Test 15: Agregar planta en extremo izquierdo
    int res15 = gameBoardAddPlant(test_board, 1, 0);
    printf("Test %d: Agregar planta en col 0: %s\n", test_num++, res15 ? "PASS" : "FAIL");

    // Test 16: Agregar planta en extremo derecho
    int res16 = gameBoardAddPlant(test_board, 1, 8);
    printf("Test %d: Agregar planta en col 8: %s\n", test_num++, res16 ? "PASS" : "FAIL");

    // Test 17: Llenar una fila completa
    GameBoard* test_board2 = gameBoardNew();  // nuevo
    int todos_ok = 1;
    for (int c = 0; c < GRID_COLS; c++) {
        if (!gameBoardAddPlant(test_board2, 2, c)) todos_ok = 0;
    }
    printf("Test %d: Llenar fila completa: %s\n", test_num++, todos_ok ? "PASS" : "FAIL");
    gameBoardDelete(test_board2);  // nuevo

    // Test 18: Intentar agregar en celda ocupada
    int res18 = gameBoardAddPlant(test_board, 0, 4); // Ya hay planta ahí
    printf("Test %d: Agregar en celda ocupada: %s\n", test_num++, (!res18) ? "PASS" : "FAIL");

    gameBoardDelete(test_board);  // nuevo

    // ===== TESTS DE gameBoardRemovePlant =====
    printf("\n--- Tests de gameBoardRemovePlant ---\n");
    GameBoard* test_board3 = gameBoardNew();  // nuevo

    // Test 19: Plantar en 3,4,5 y sacar la del medio (4)
    gameBoardAddPlant(test_board3, 0, 3);
    gameBoardAddPlant(test_board3, 0, 4);
    gameBoardAddPlant(test_board3, 0, 5);
    gameBoardRemovePlant(test_board3, 0, 4);
    // Verificar que se puede agregar de nuevo en 4
    int res19 = gameBoardAddPlant(test_board3, 0, 4);
    printf("Test %d: Sacar planta del medio y re-agregar: %s\n", test_num++, res19 ? "PASS" : "FAIL");

    // Test 20b: Secuencia exacta del enunciado - plantar 3,4,5 → sacar 4 → sacar 3
    GameBoard* test_board3b = gameBoardNew();
    gameBoardAddPlant(test_board3b, 0, 3);
    gameBoardAddPlant(test_board3b, 0, 4);
    gameBoardAddPlant(test_board3b, 0, 5);
    gameBoardRemovePlant(test_board3b, 0, 4);  // Sacar del medio
    gameBoardRemovePlant(test_board3b, 0, 3);  // Sacar la 3 (según enunciado)
    // Verificar que se pueden agregar nuevamente en 3 y 4
    int res20b = gameBoardAddPlant(test_board3b, 0, 3) && gameBoardAddPlant(test_board3b, 0, 4);
    printf("Test %d: Secuencia 3,4,5 → quitar 4 → quitar 3: %s\n", test_num++, res20b ? "PASS" : "FAIL");
    gameBoardDelete(test_board3b);

    // Test 20: Sacar la de col 3 (debería fusionar con vacío de 4)
    gameBoardRemovePlant(test_board3, 0, 3);
    int res20 = gameBoardAddPlant(test_board3, 0, 3);
    printf("Test %d: Sacar col 3 tras sacar col 4: %s\n", test_num++, res20 ? "PASS" : "FAIL");

    gameBoardDelete(test_board3);  // nuevo
    // Test 21: Llenar fila y sacar una del medio
    GameBoard* test_board4 = gameBoardNew();  // nuevo
    for (int c = 0; c < GRID_COLS; c++) {
        gameBoardAddPlant(test_board4, 0, c);
    }
    gameBoardRemovePlant(test_board4, 0, 4);
    int res21 = gameBoardAddPlant(test_board4, 0, 4);
    printf("Test %d: Llenar fila, sacar del medio, re-agregar: %s\n", test_num++, res21 ? "PASS" : "FAIL");
    gameBoardDelete(test_board4);  // nuevo

    // ===== TESTS DE gameBoardAddZombie =====
    printf("\n--- Tests de gameBoardAddZombie ---\n");
    GameBoard* test_board5 = gameBoardNew();  // nuevo
    // Test 22: Agregar 3 zombies y luego uno más
    gameBoardAddZombie(test_board5, 0);
    gameBoardAddZombie(test_board5, 0);
    gameBoardAddZombie(test_board5, 0);
    int count_before = 0;
    ZombieNode* z = test_board5->rows[0].first_zombie;
    while (z) { count_before++; z = z->next; }
    gameBoardAddZombie(test_board5, 0);
    int count_after = 0;
    z = test_board5->rows[0].first_zombie;
    while (z) { count_after++; z = z->next; }
    printf("Test %d: Agregar 4to zombie a lista de 3: %s\n", test_num++,
        (count_before == 3 && count_after == 4) ? "PASS" : "FAIL");
    gameBoardDelete(test_board5);  // nuevo

    // Test 23: Crear lista de 10000 zombies (test de estrés)
    GameBoard* test_board6 = gameBoardNew();  // nuevo
    for (int i = 0; i < 10000; i++) {
        gameBoardAddZombie(test_board6, 1);
    }
    int count_10k = 0;
    z = test_board6->rows[1].first_zombie;
    while (z) { count_10k++; z = z->next; }
    printf("Test %d: Crear lista de 10000 zombies: %s\n", test_num++,
        (count_10k == 10000) ? "PASS" : "FAIL");
    gameBoardDelete(test_board6);  // nuevo

    // Liberar memoria de los tableros creados antes
   // gameBoardDelete(test_board);    // nuevo
  //  gameBoardDelete(test_board2);   // nuevo
    // gameBoardDelete(test_board3);   // nuevo

    printf("\n========== TESTS COMPLETADOS ==========\n\n");
}

//parametros que no se usan en el codigo pero si pararecibir parametros en la termianl
int main(int argc, char* args[]) {
    //se los ignoran porque no se usan en el codigo
    (void)argc;
    (void)args;
    ejecutarTests();
    
    printf("Presiona Enter para iniciar el juego...\n");
    getchar(); //esperamos a que el usuario quiera empezar
    
    srand(time(NULL)); //semilla aleatoria para la generacion de zombies
    if (!inicializar()) {
        printf("Error al inicializar SDL!\n");
        return 1;
    }

    //crea el tablero 
    game_board = gameBoardNew();
    if (!game_board) {
        printf("Error al crear el tablero!\n");
        cerrar();
        return 1;
    }

    SDL_Event e;
    int game_over = 0; 

    while (!game_over) {//mientras no termine el juego
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) {
                game_over = 1;
            }
            
            if (e.type == SDL_MOUSEMOTION) {
                int mouse_x = e.motion.x;
                int mouse_y = e.motion.y;
                if (mouse_x >= GRID_OFFSET_X && mouse_x < GRID_OFFSET_X + GRID_WIDTH &&
                    mouse_y >= GRID_OFFSET_Y && mouse_y < GRID_OFFSET_Y + GRID_HEIGHT) {
                    cursor.col = (mouse_x - GRID_OFFSET_X) / CELL_WIDTH;
                    cursor.row = (mouse_y - GRID_OFFSET_Y) / CELL_HEIGHT;
                }
            }
            
            if (e.type == SDL_MOUSEBUTTONDOWN) {
                gameBoardAddPlant(game_board, cursor.row, cursor.col);
            }
        }

        //actualiza el estado del tablero
        gameBoardUpdate(game_board);
        //dibuja en cada estado del tablero
        gameBoardDraw(game_board);

        // Verificar si un zombie llegó a la casa (game over)
        for (int r = 0; r < GRID_ROWS; r++) {
            ZombieNode* znode = game_board->rows[r].first_zombie;
            while (znode != NULL) {
                if (znode->zombie_data.rect.x < GRID_OFFSET_X - znode->zombie_data.rect.w) {
                    printf("\n¡GAME OVER! - Un zombie llegó a tu casa!\n");
                    game_over = 1;
                    break;
                }
                znode = znode->next;
            }
            if (game_over) break;
        }
        SDL_Delay(16);
    }


    //limpieza final
    gameBoardDelete(game_board);
    cerrar();
    return 0;
}


