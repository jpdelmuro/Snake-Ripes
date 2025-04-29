# Snake en Ripes Simulator

Este proyecto es una implementación del clásico juego **Snake**, programado en C para ejecutarse en el **simulador Ripes** utilizando una **LED Matrix** y controles tipo **D-Pad**.

## Características
- Movimiento horizontal y vertical de la serpiente.
- Generación aleatoria de comida (manzana) en bloques 2x2.
- Crecimiento de la serpiente al comer.
- Reinicio del juego al chocar con un obstáculo o consigo misma.
- Detección de colisiones

## Requisitos
- **Ripes** (Simulador de arquitectura RISC-V).
- LED Matrix y D-Pad mapeados a las direcciones utilizadas:
  - `LED_MATRIX_0_BASE`
  - `D_PAD_0_UP`, `D_PAD_0_DOWN`, `D_PAD_0_LEFT`, `D_PAD_0_RIGHT`
  - `SWITCHES_0_BASE`

## Controles
- **D-Pad Arriba:** Mover hacia arriba.
- **D-Pad Abajo:** Mover hacia abajo.
- **D-Pad Izquierda:** Mover hacia la izquierda.
- **D-Pad Derecha:** Mover hacia la derecha.
- **Switch 0:** Reiniciar el juego después de perder.

## Comentarios
- Programado por Juan Pablo Del Muro y Luisa Ikram Zaldivar para la clase de Arquitectura de Computadoras.

