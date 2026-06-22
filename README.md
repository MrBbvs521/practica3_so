# Práctica 3: Comunicación Cliente-Servidor mediante Sockets TCP/IP en C++ y Automatización con Bash

Este repositorio contiene la implementación completa y el despliegue automatizado de una arquitectura de red distribuida bajo el modelo cliente-servidor, desarrollada para la asignatura de Sistemas Operativos del Grado en Tecnologías Digitales para la Empresa (Curso 2025-2026) en la Universidad de Burgos.

# Descripción del Proyecto

El proyecto simula un escenario real de supervisión y monitorización industrial en una planta de manufactura automatizada. Consiste en la ingesta y persistencia de datos procedentes de una Cámara Térmica Industrial (dispositivo Edge) de manera remota.

La solución se estructura en tres componentes clave que interactúan a través de la pila de protocolos TCP/IP:

El Servidor de la Cámara (ServidorCamara.py): Script en Python que simula el hardware de la cámara térmica. Este levanta un socket TCP en la dirección IP local de loopback (127.0.0.1) y el puerto 12345, y queda a la espera de conexiones. Al recibir un cliente, empaqueta en binario un array de 768 lecturas de temperatura en bytes sin signo (768B) y los transmite de forma íntegra.

El Cliente de Ingesta (cliente.cpp): Aplicación modular desarrollada en C++ que actúa como el receptor industrial. Crea de forma nativa un socket TCP/IP, establece la conexión con el servidor, gestiona de forma robusta la fragmentación del buffer de red mediante llamadas iterativas a recv(), almacena las muestras dinámicamente, realiza cálculos de analítica básica (valores máximos, mínimos y promedios térmicos) y genera un archivo de persistencia local en disco.

El Script de Despliegue (bash.sh): Script capataz desarrollado en Bash encargado de la automatización del flujo de trabajo de sistemas. Controla la compilación con C++17, audita la disponibilidad del puerto de red liberándolo si hay conexiones residuales bloqueantes (lsof/fuser), arranca el servidor en segundo plano (&), ejecuta el cliente para consolidar la ingesta y finaliza de manera limpia y sincronizada los hilos de ejecución mediante señales de software.

# Estructura de Archivos

cliente.cpp: Código fuente en C++. Contiene la lógica modularizada para la creación de sockets, la conexión asíncrona, la recepción estructurada de flujos de datos y la analítica matemática de temperaturas.

ServidorCamara.py: Simulador en Python que implementa la parte servidora de la red y emite las 768 muestras térmicas industriales empaquetadas en bytes.

bash.sh: Script de Bash encargado del ciclo de vida del puerto de red y de la orquestación segura de los procesos del sistema operativo.

datos_temperatura.txt: Archivo final de salida donde el cliente C++ vuelca secuencialmente y de forma persistente cada una de las 768 lecturas recibidas.

.gitignore: Archivo de configuración de Git encargado de omitir el seguimiento del binario compilado de C++ y los archivos de caché del sistema operativo.

# Requisitos Previos (Prerrequisitos)

Para compilar, desplegar y monitorizar esta red de forma nativa en tu entorno Linux (Ubuntu o Windows Subsystem for Linux - WSL), asegúrate de instalar las herramientas del sistema esenciales de compilación y monitorización de red ejecuntando:

# Actualizar los paquetes e instalar g++, make y utilidades de puertos de red
sudo apt update && sudo apt install -y build-essential lsof


# Instrucciones de Ejecución

Opción 1: Despliegue Automatizado (Recomendado)

Esta es la opción recomendada para la evaluación, ya que el script de Bash se encarga de evitar condiciones de carrera (Race Conditions) y puertos de red colgados:

Navega hasta tu directorio de trabajo en la terminal de Linux:

cd ~/practica3_so


Concede permisos de ejecución al script capataz:

chmod +x bash.sh


Lanza el script de automatización:

bash bash.sh


El script liberará el puerto de red si estaba bloqueado por una ejecución anterior fallida, compilará tu cliente C++, levantará el servidor en segundo plano, ejecutará la descarga de datos, mostrará las estadísticas térmicas por consola y apagará el servidor de forma segura liberando recursos de memoria y red.

# Opción 2: Despliegue Manual Paso a Paso

Si deseas inspeccionar individualmente el comportamiento del Kernel y las llamadas de red:

En una terminal activa de Linux, arranca el servidor de simulación:

python3 ServidorCamara.py


Abre una segunda pestaña o terminal en paralelo, compila tu código C++ y ejecútalo:

g++ -std=c++17 cliente.cpp -o cliente
./cliente


Una vez completada la ingesta en el cliente, vuelve a la terminal del servidor y detén el proceso de forma manual pulsando Ctrl+C.

# Salida Esperada en Consola

Al realizar la ejecución con el script de automatización de Bash, observarás el siguiente flujo de ejecución síncrono reflejado en tu pantalla:

=====================================================================
       INICIANDO ARQUITECTURA DE RED CLIENTE-SERVIDOR (UBU)         
=====================================================================
[Bash 1/5] Compilando el cliente C++ con estándar C++17...
[OK] Compilación del cliente realizada con éxito.
[Bash 2/5] Lanzando servidor de cámara térmica en segundo plano...
[INFO] Servidor levantado en background (PID: 12560)

=========================================================
       INICIANDO SERVIDOR INDUSTRIAL DE CÁMARA TÉRMICA    
=========================================================
[SERVIDOR] Escuchando en 127.0.0.1:12345 ...

[Bash 3/5] Ejecutando el cliente C++ para realizar la ingesta...
[CLIENTE] Intentando establecer conexión TCP con el servidor 127.0.0.1:12345...
[CLIENTE] ¡Conexión establecida con éxito!
[SERVIDOR] ¡Conexión aceptada desde el cliente ('127.0.0.1', 48590)!
[SERVIDOR] Transmisión exitosa: 768 bytes térmicos enviados.
[CLIENTE] Iniciando ingesta de flujo de datos...
[CLIENTE] Recepción finalizada: Se han cargado 768 bytes en el vector dinámico.
[CLIENTE] Socket de conexión cerrado.
[CLIENTE] Registro persistente guardado de forma segura en: datos_temperatura.txt

=== RESUMEN TÉRMICO DE LA CÁMARA ===
  * Total muestras leídas : 768 puntos.
  * Temperatura Mínima    : 0 ºC
  * Temperatura Máxima    : 255 ºC
  * Temperatura Promedio  : 127.5 ºC
====================================

[Bash 4/5] Adquisición completada. Deteniendo servidor de forma segura...
[Bash 5/5] Despliegue finalizado. Arquitectura de red cerrada limpia ✅


# Conceptos Teóricos de Sistemas Operativos Implementados

Para la correcta interpretación académica de esta solución, el software hace un uso intensivo de llamadas al sistema y conceptos nativos del diseño de sistemas operativos:

Llamada al sistema socket() (Creación de Sockets): Se solicita al núcleo del sistema operativo la creación de un descriptor de socket mediante la llamada socket(AF_INET, SOCK_STREAM, 0). AF_INET asigna la familia de protocolos IPv4 y SOCK_STREAM define una conexión de transporte robusta basada en el protocolo orientado a conexión TCP (fiable, libre de errores y ordenado).

Gestión de Puertos Residuales (SO_REUSEADDR): Por diseño del Kernel, al cerrarse una conexión TCP, el puerto queda bloqueado en un estado de seguridad llamado TIME_WAIT durante varios minutos. Al aplicar la opción setsockopt con la propiedad de reutilización de direcciones, indicamos al sistema operativo que permita reasignar el puerto instantáneamente, agilizando el flujo de pruebas industriales.

Bucle de Recepción de Flujo de Red (recv()): Dado que el protocolo TCP funciona como una transmisión de flujo de bytes continuo (byte stream), la red puede fragmentar los 768 bytes transmitidos en paquetes menores. La función implementada en C++ ejecuta un bucle iterativo que acumula de forma exacta los bytes leídos de la pila de red hasta consolidar la estructura esperada de la trama, blindando al cliente ante pérdidas de sincronía.

Señales de Software y Concurrencia: El script de Bash monitoriza asíncronamente las señales del sistema. Al completarse la ingesta por el cliente C++, se utiliza el comando kill con la señal SIGINT (Señal 2) para permitir un apagado "ordenado" y programado de las rutinas de limpieza del servidor de Python, recurriendo a SIGKILL (Señal 9, destructiva e ineludible) solo en caso de un bloqueo imprevisto del hilo.

Grado en Tecnologías Digitales para la Empresa - Universidad de Burgos.