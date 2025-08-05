# PwmIn - Medición de PWM con PIO para Raspberry Pi Pico en Arduino

Esta librería permite medir con alta precisión las características de una señal PWM (frecuencia, ancho de pulso y ciclo de trabajo) utilizando el hardware **PIO (Programmable I/O)** de la Raspberry Pi Pico, todo ello dentro del entorno de Arduino.

Al delegar la medición al hardware PIO, los núcleos de la CPU quedan completamente libres para otras tareas, permitiendo mediciones no bloqueantes y de muy alta frecuencia sin consumir recursos del procesador.

## Agradecimientos

Este proyecto es una adaptación al entorno de Arduino del trabajo realizado por **GitJer**. El código PIO original y la lógica fundamental se basan en su repositorio [Some_RPI-Pico_stuff](https://github.com/GitJer/Some_RPI-Pico_stuff/tree/main/PwmIn).

## Características Principales

- **Alta Precisión:** Las mediciones se realizan a nivel de hardware, con una resolución dictada por la velocidad del reloj del sistema.
- **No Bloqueante:** El código de tu `loop()` no se detiene a esperar un pulso. Puedes comprobar si hay nuevos datos disponibles y seguir ejecutando otras tareas.
- **Soporte Multicanal:** La librería busca automáticamente máquinas de estado (SM) PIO disponibles, permitiendo instanciar múltiples objetos `PwmIn` para medir varias señales PWM de forma simultánea (hasta un máximo de 8 canales en una Pico).
- **Fácil de Usar:** Proporciona una API simple y encapsulada en una clase `PwmIn`.

## ¿Cómo Funciona? La Magia del PIO

#### ¿Qué es el PIO?
El PIO (Programmable I/O) es una característica única de la Raspberry Pi Pico. Son dos periféricos de hardware con pequeñas pero muy rápidas máquinas de estado que pueden ser programadas para manejar protocolos de E/S de manera autónoma, sin intervención de la CPU. Son perfectos para tareas de temporización precisas como la lectura de señales PWM.

#### Flujo de Trabajo con archivos `.pio`
Para programar el PIO, se escribe un código en un ensamblador específico en un archivo con extensión `.pio`. Este código debe ser "pre-compilado" a un archivo de cabecera de C (`.pio.h`) que contiene el programa como un array de instrucciones.

Una herramienta online como [**pioasm de Wokwi**](https://wokwi.com/tools/pioasm) es perfecta para convertir tu código `.pio` al formato de cabecera necesario.

#### Algoritmo del PIO
En esencia, el programa PIO que utiliza esta librería sigue esta lógica:

```
loop:
    reiniciar contador_pulso_alto (registro y)
    reiniciar contador_pulso_bajo (registro x)

    esperar a que la señal baje a 0
    esperar a que la señal suba a 1 (detectar flanco de subida)

    mientras la señal esté en 1:
        decrementar contador_pulso_alto

    mientras la señal esté en 0:
        decrementar contador_pulso_bajo

    enviar valor de contador_pulso_alto a la CPU
    enviar valor de contador_pulso_bajo a la CPU
```
El valor final del ancho de pulso y del periodo se calcula a partir de los valores de estos contadores.

Puedes ver el código fuente completo del programa PIO, con sus comentarios originales, dentro del archivo [`PwmIn.pio.h`](./PwmIn.pio.h).

## Instalación

1.  Descarga este repositorio como un archivo `.zip`.
2.  Abre tu IDE de Arduino.
3.  Ve a `Programa > Incluir Librería > Añadir biblioteca .ZIP`.
4.  Selecciona el archivo `.zip` que acabas de descargar.
5.  ¡Listo! Ahora puedes usar `#include "PwmIn.h"` en tus sketches.

## API de la Librería (Funciones Públicas)

-   `bool attach(uint pin)`
    Asigna un pin GPIO para la medición de PWM e inicializa una máquina de estado PIO. Devuelve `true` si tuvo éxito o `false` si no hay máquinas de estado disponibles.

-   `bool available(void)`
    Comprueba si hay una nueva medición completa (periodo y ancho de pulso) disponible en la FIFO del PIO. Devuelve `true` si hay datos listos.

-   `bool update(void)`
    Lee los nuevos datos de la FIFO y actualiza los valores internos de la clase. Debe llamarse después de que `available()` devuelva `true`. Devuelve `false` si la FIFO contenía datos inconsistentes.

-   `float get_period(void)`
    Devuelve el periodo de la última señal medida, en **microsegundos (us)**.

-   `float get_pulsewidth(void)`
    Devuelve el ancho del pulso (tiempo en alto) de la última señal medida, en **microsegundos (us)**.

-   `float get_dutycycle(void)`
    Devuelve el ciclo de trabajo de la última señal medida, como un valor entre **0.0 y 1.0**.

## Ejemplos de Uso

### Ejemplo Básico: Medir un solo canal

Este sketch inicializa la medición en el pin 2 y muestra por el puerto serie el periodo, el ancho de pulso y el ciclo de trabajo.

```cpp
#include "PwmIn.h"

PwmIn PWM_test;

void setup() {
  Serial.begin(115200);

  // Adjuntar la medición de PWM al pin 2
  PWM_test.attach(2);
}

void loop() {
  // Comprobar si hay una nueva medición disponible
  if (PWM_test.available()) {
    
    // Leer los datos y actualizar los valores
    PWM_test.update();

    // Imprimir los resultados
    Serial.print("Periodo (us): ");
    Serial.print(PWM_test.get_period());
    Serial.print(" - Ancho de Pulso (us): ");
    Serial.print(PWM_test.get_pulsewidth());
    Serial.print(" - Duty Cycle: ");
    Serial.println(PWM_test.get_dutycycle());
  }
}
```

### Ejemplo Avanzado: Medir múltiples canales

Este ejemplo demuestra la capacidad de la librería para medir 7 canales PWM simultáneamente.

```cpp
#include "PwmIn.h"

#define PWM_CHANNELS 7

PwmIn PWM_IN[PWM_CHANNELS];
int PWM_PIN[PWM_CHANNELS] = { 2, 15, 14, 13, 18, 17, 16 };

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("Iniciando escaneo multicanal PWM...");

  for (int i = 0; i < PWM_CHANNELS; i++) {
    if (PWM_IN[i].attach(PWM_PIN[i])) {
      Serial.print("Canal PWM en pin ");
      Serial.print(PWM_PIN[i]);
      Serial.println(": OK");
    } else {
      Serial.print("Canal PWM en pin ");
      Serial.print(PWM_PIN[i]);
      Serial.println(": FALLO (No hay SM disponibles)");
    }
  }
  delay(1000);
}

void loop() {
  bool all_available = true;
  for (int i = 0; i < PWM_CHANNELS; i++) {
    if (!PWM_IN[i].available()) {
      all_available = false;
      break; // No es necesario seguir comprobando si uno falla
    }
  }

  if (all_available) {
    Serial.print("Mediciones: ");
    for (int i = 0; i < PWM_CHANNELS; i++) {
      PWM_IN[i].update();

      Serial.print("[P");
      Serial.print(PWM_PIN[i]);
      Serial.print(": ");
      Serial.print(PWM_IN[i].get_pulsewidth());
      Serial.print("us] ");
    }
    Serial.println("");
  }
}
```