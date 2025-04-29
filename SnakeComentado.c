#include "ripes_system.h"
#include <stdio.h>

#define SWITCH_0 (0x01) //Switch 0
#define DIRECTION_HORIZONTAL 1 //Movimiento horizontal
#define DIRECTION_VERTICAL 2 //Movimiento vertical
#define INITIAL_POSITION 2 * LED_MATRIX_0_WIDTH + 2 //Posición inicial de la cabeza de la serpiente
#define WALL_COLOR 0xFFFF00 //Color del borde (amarillo)
#define BODY_COLOR 0xFF0000 //Color de la serpiente (rojo)
#define FOOD_COLOR 0x00FF00 //Color de la manzana (verde)
#define LED_MATRIX_0_BASE ((volatile unsigned int *)0xf0000014) //Dirección base de la matriz

//Declaramos punteros y variables globales
volatile unsigned int *head = LED_MATRIX_0_BASE; //Puntero a la posición actual de la cabeza
volatile unsigned int *matrix_cleaner = LED_MATRIX_0_BASE; //Puntero para limpiar la matriz
volatile unsigned int *pad_up = D_PAD_0_UP; 
volatile unsigned int *pad_down = D_PAD_0_DOWN;
volatile unsigned int *pad_left = D_PAD_0_LEFT;
volatile unsigned int *pad_right = D_PAD_0_RIGHT;
volatile unsigned int *food = LED_MATRIX_0_BASE; //puntero para la comida (manzana)
volatile unsigned int *switches = SWITCHES_0_BASE; //lee el estado de los switches 
unsigned int random_seed = 0; //semilla para los números aleatorios
volatile unsigned int *snake_segments[LED_MATRIX_0_WIDTH * LED_MATRIX_0_HEIGHT];//almacena las posiciones de segmentos para la serpiente
unsigned int length = 2; //longitud inicial de la serpiente
unsigned int food_count = 0; //contador de manzanas en la matriz

//Declaracion de funciones
void drawBorders(); //dibuja los bordes en la matriz
int random(); //genera número pseudoaleatorio 
void setSeed(unsigned int root); //semilla para el generador aleatorio
void spawnFood(); //genera la manzana en una posición aleatoria
void disableLEDs(); //apaga leds específicos en la matriz
void resetMatrix(); //limpia la matriz

void main() {
    unsigned int switch_state = 1; //almacena el estado del switch para reiniciar el juego
    unsigned int movement_state = DIRECTION_HORIZONTAL; //indica el estado del movimiento
    unsigned int movement_offset = 1; //offset de movimiento de la cabeza, define el cambio para el sig movimiento

    unsigned int *tail_down = 0; //puntero debajo de la cola
    unsigned int *tail_up = 0; //puntero arriba de la cola
    unsigned int *head_top = 0; //puntero arriba de la cabeza

    unsigned int *clear_segment1 = 0; //segmentos de la matriz que se limpiaran
    unsigned int *clear_segment2 = 0;
    unsigned int *clear_segment3 = 0;

    int iteration = 0; //contador de iteraciones del juego

    head = INITIAL_POSITION; //posición inicial de la cabeza
    drawBorders(); //dibuja los bordes


    while (1) {
        //si no hay manzanas en la matriz
        if (food_count < 1) {
            disableLEDs(); //apaga leds en el centro
        }

//Calcular las posiciones de los segmentos de la serpiente
        tail_down = head - 1; //detrás de la cabeza
        tail_up = tail_down - LED_MATRIX_0_WIDTH; //arriba de la cola
        head_top = head - LED_MATRIX_0_WIDTH; //arriba de la cabeza
        
//Movimientos de la serpiente basado en los botones del pad que se pulsen
        if (*pad_up == 1 && movement_state == DIRECTION_HORIZONTAL) {
            movement_offset = -LED_MATRIX_0_WIDTH; //arriba
            movement_state = DIRECTION_VERTICAL;
        }
        if (*pad_down == 1 && movement_state == DIRECTION_HORIZONTAL) {
            movement_offset = LED_MATRIX_0_WIDTH;//abajo
            movement_state = DIRECTION_VERTICAL;
        }
        if (*pad_left == 1 && movement_state == DIRECTION_VERTICAL) {
            movement_offset = -1; //izquierda
            movement_state = DIRECTION_HORIZONTAL;
        }
        if (*pad_right == 1 && movement_state == DIRECTION_VERTICAL) {
            movement_offset = 1; //derecha
            movement_state = DIRECTION_HORIZONTAL;
        }

//Actualiza la posición de la cabeza y otros segmentos de la serpiente
        head += 2 * movement_offset;
        tail_down += 2 * movement_offset;
        tail_up += 2 * movement_offset;
        head_top += 2 * movement_offset;

//Actualiza los segmentos de la serpiente en el conjunto
        for (int i = length; i > 0; i--) {
            snake_segments[i] = snake_segments[i - 1];
        }

        snake_segments[0] = head; //nueva posición de la cabeza

//Limpia los segmentos pasados de la cola
        clear_segment1 = snake_segments[length] - 1;
        clear_segment2 = clear_segment1 - LED_MATRIX_0_WIDTH;
        clear_segment3 = snake_segments[length] - LED_MATRIX_0_WIDTH;

        *snake_segments[length] = 0x000000; //limpia el color
        *clear_segment1 = 0x000000;
        *clear_segment2 = 0x000000;
        *clear_segment3 = 0x000000;

//Si la cabeza choca con una manzana
        if (*head == FOOD_COLOR) {
            spawnFood(); //genera una nueva
            length++; //la serpiente crece
        }
        
//Si la cabeza choca con algo que no es manzana 
        if (*head != 0x000000 && *head != FOOD_COLOR) {
            //reinicia el juego
            while (switch_state == 0) {
                switch_state = *switches & SWITCH_0;//espera que se active switch 0
                for (int i = 0; i < 3000; i++);
            }

            switch_state = 0;
            resetMatrix();
            spawnFood();
//Reinicia las variables de estado y posición
            matrix_cleaner = LED_MATRIX_0_BASE;
            head = LED_MATRIX_0_BASE;
            head += INITIAL_POSITION;
            tail_down = head - 1;
            tail_up = tail_down - LED_MATRIX_0_WIDTH;
            head_top = head - LED_MATRIX_0_WIDTH;
            snake_segments[0] = 0;
            clear_segment1 = 0;
            clear_segment2 = 0;
            clear_segment3 = 0;
            length = 2;
            food_count = 0;

            movement_offset = 1;
            movement_state = DIRECTION_HORIZONTAL;
        }

//Dibuja la serpiente en la matriz
        *head = BODY_COLOR;
        *tail_down = BODY_COLOR;
        *tail_up = BODY_COLOR;
        *head_top = BODY_COLOR;
  
//Delay
        for (int i = 0; i < 3000; i++);
    }
}

void drawBorders() {
    unsigned int *border_ptr = LED_MATRIX_0_BASE; //puntero de la matriz
    unsigned int *border_aux = 0; //puntero auxiliar para dibujar los bordes

//dibuja el borde de arriba
    for (int i = 0; i < 35; i++) { //valor de 35 columnas
        *border_ptr = WALL_COLOR; //pinta el led con el color del borde
        border_ptr++; //avanza al siguiente led
    }

//dibuja los bordes laterales
    for (int i = 0; i < 25; i++) {  //valor de 25 filas
        *border_ptr = WALL_COLOR; 
        border_aux = border_ptr - 1; 
        *border_aux = WALL_COLOR;
        border_ptr += LED_MATRIX_0_WIDTH; //baja a la siguiente fila
    }

//dibuja el borde de abajo
    for (int i = 0; i < 35; i++) {
        *border_ptr = WALL_COLOR; //pinta el led con el color del borde
        border_aux = border_ptr - LED_MATRIX_0_WIDTH; //pinta la línea superior a esta
        *border_aux = WALL_COLOR;
        border_ptr--; //retrocede al led anterior
    }

//sube los bordes laterales hacia la parte de arriba (cierra el marco)
    for (int i = 0; i < 25; i++) {
        *border_ptr = WALL_COLOR; //pinta la columna derecha
        border_ptr -= LED_MATRIX_0_WIDTH; //sube una fila
    }
}

//Limpia toda la matriz a excepcion de los bordes
void resetMatrix() {
    matrix_cleaner = LED_MATRIX_0_BASE; //puntero al inicio de la matriz
    //Recorre cada led en la matriz
    for (int i = 0; i < LED_MATRIX_0_WIDTH * LED_MATRIX_0_HEIGHT; i++) {
        if (*matrix_cleaner != WALL_COLOR) { //si el led no es del color del borde
            *matrix_cleaner = 0x000000; //apaga el led (vacío/negro)
        }
        matrix_cleaner += 1; //avanza al siguiente LED
    }
}

//genera un número pseudoaleatorio (números que parecen aleatorios, pero en realidad son generados por un algoritmo determinístico)
int random() {
    unsigned int head_position = (unsigned int)head; // Usa la posición actual de la serpiente, lo convierte en entero
    random_seed = random_seed * 435324123 + head_position; // Operaciones básicas para generar el número pseudaleatorio
    return (unsigned int)(random_seed / 65536) % 32768; // Mantiene el resultado en el rango
}

//establece una semilla para el generador de número aleatorio
void setSeed(unsigned int root) {
    random_seed = root;
}

//Genera una manzana en una posición de la matriz que esté libre
void spawnFood() {
    setSeed(random_seed); // Inicializa la semilla del generador

    int num_positions_x = (LED_MATRIX_0_WIDTH - 4) / 2; // Número de bloques 2x2 en X
    int num_positions_y = (LED_MATRIX_0_HEIGHT - 4) / 2; // Número de bloques 2x2 en Y

//variables para guardar las coordenadas de X y Y
    int coord_x = 0;
    int coord_y = 0;

    int apple;                 // Posición inicial de la manzana
    int apple_pos[4];          // Coordenadas del bloque 2x2 de la manzana

    do {
        // Calcula coordenadas alineadas a bloques de 2x2
        coord_x = (random() % num_positions_x) * 2 + 2 -1; // Alineado al segmento
        coord_y = (random() % num_positions_y) * 2 + 2 -1;

        // Calcula las posiciones absolutas en la matriz
        apple = coord_y * LED_MATRIX_0_WIDTH + coord_x;
        
        //define las 4 esquinas del bloque 2x2 de la manzana
        apple_pos[0] = apple;                          // Esquina superior izquierda
        apple_pos[1] = apple + 1;                     // Esquina superior derecha
        apple_pos[2] = apple + LED_MATRIX_0_WIDTH;    // Esquina inferior izquierda
        apple_pos[3] = apple + LED_MATRIX_0_WIDTH + 1;// Esquina inferior derecha

//verifica si algún led del bloque está ocupado
    } while (LED_MATRIX_0_BASE[apple_pos[0]] != 0x000000 || 
             LED_MATRIX_0_BASE[apple_pos[1]] != 0x000000 || 
             LED_MATRIX_0_BASE[apple_pos[2]] != 0x000000 || 
             LED_MATRIX_0_BASE[apple_pos[3]] != 0x000000);

    // Dibuja la manzana completa en el segmento 2x2
    LED_MATRIX_0_BASE[apple_pos[0]] = FOOD_COLOR;
    LED_MATRIX_0_BASE[apple_pos[1]] = FOOD_COLOR;
    LED_MATRIX_0_BASE[apple_pos[2]] = FOOD_COLOR;
    LED_MATRIX_0_BASE[apple_pos[3]] = FOOD_COLOR;

    random_seed++; // Actualiza la semilla
    food_count++; //aumenta el contador de manzanas
}

//Apaga un conjunto específico de leds
void disableLEDs() {
    volatile unsigned int *base_led = LED_MATRIX_0_BASE; //puntero al inicio de la matriz
    volatile unsigned int *led1 = base_led + 1 * LED_MATRIX_0_WIDTH + 1; //led en la esquina superior izq
    volatile unsigned int *led2 = led1 + 1; //led en la esquina superior der
    volatile unsigned int *led3 = led1 + LED_MATRIX_0_WIDTH; //led en la esquina inferior izq
    volatile unsigned int *led4 = led2 + LED_MATRIX_0_WIDTH; //led en la esquina inferior der

    *led1 = 0x000000; //apaga el led 1
    *led2 = 0x000000; //apaga el led 2
    *led3 = 0x000000; //apaga el led 3
    *led4 = 0x000000; //apaga el led 4
}