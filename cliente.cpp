// ==============================================================================
// Práctica 3: Cliente TCP/IP para Adquisición de Datos de Sensores en C++
// ==============================================================================

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

using namespace std;

// Constantes de configuración de la red y almacenamiento
const string SERVER_IP = "127.0.0.1";
const int SERVER_PORT = 12345;
const size_t TAMANO_ESPERADO = 768; // 768 bytes (muestras térmicas)
const string ARCHIVO_SALIDA = "datos_temperatura.txt";

// ==============================================================================
// 1. FUNCIÓN: Crear socket y establecer conexión TCP con el servidor
// ==============================================================================
int conectarConServidor(const string& ip, int puerto) {
    // Crear el descriptor del socket
    // AF_INET: Protocolo IPv4 | SOCK_STREAM: Orientado a conexión (TCP)
    int socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0) {
        cerr << "[ERROR CLIENTE] No se pudo crear el socket." << endl;
        return -1;
    }

    // Configurar la estructura de dirección del servidor remoto
    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(puerto); // Convertir puerto a orden de bytes de red (big-endian)

    // Convertir dirección IP de texto a formato binario de red
    if (inet_pton(AF_INET, ip.c_str(), &serv_addr.sin_addr) <= 0) {
        cerr << "[ERROR CLIENTE] Dirección IP inválida o no soportada." << endl;
        close(socket_fd);
        return -1;
    }

    cout << "[CLIENTE] Intentando establecer conexión TCP con el servidor " << ip << ":" << puerto << "..." << endl;

    // Conectar con el servidor remoto (Llamada al sistema connect)
    if (connect(socket_fd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "[ERROR CLIENTE] Conexión fallida. Asegúrate de que el servidor está escuchando." << endl;
        close(socket_fd);
        return -1;
    }

    cout << "[CLIENTE] ¡Conexión establecida con éxito!" << endl;
    return socket_fd;
}

// ==============================================================================
// 2. FUNCIÓN: Recepción de datos de red controlando fragmentación
// ==============================================================================
bool recibirDatosRed(int socket_fd, vector<uint8_t>& buffer, size_t total_esperado) {
    size_t total_recibido = 0;
    buffer.resize(total_esperado, 0);

    cout << "[CLIENTE] Iniciando ingesta de flujo de datos..." << endl;

    // En TCP, los datos pueden fragmentarse. Debemos iterar con recv() hasta recibir todo.
    while (total_recibido < total_esperado) {
        ssize_t bytes_leidos = recv(socket_fd, buffer.data() + total_recibido, total_esperado - total_recibido, 0);
        
        if (bytes_leidos < 0) {
            cerr << "[ERROR CLIENTE] Error en la llamada al sistema recv()." << endl;
            return false;
        } else if (bytes_leidos == 0) {
            cerr << "[ERROR CLIENTE] El servidor cerró la conexión prematuramente." << endl;
            return false;
        }
        
        total_recibido += bytes_leidos;
    }

    cout << "[CLIENTE] Recepción finalizada: Se han cargado " << total_recibido << " bytes en el vector dinámico." << endl;
    return true;
}

// ==============================================================================
// 3. FUNCIÓN: Guardar el flujo recibido en el disco local
// ==============================================================================
bool guardarDatosDisco(const string& ruta_archivo, const vector<uint8_t>& datos) {
    ofstream archivo(ruta_archivo, ios::out | ios::trunc);
    if (!archivo.is_open()) {
        cerr << "[ERROR CLIENTE] No se pudo crear el archivo de salida: " << ruta_archivo << endl;
        return false;
    }

    // Guardar los datos de temperatura secuencialmente
    archivo << "=========================================================\n";
    archivo << "   REGISTRO ADQUIRIDO - CÁMARA TÉRMICA INDUSTRIAL\n";
    archivo << "=========================================================\n\n";
    archivo << "Total de lecturas térmicas: " << datos.size() << " muestras.\n\n";

    archivo << "LECTURAS DETECTADAS (Secuencial):\n";
    archivo << "Índice\tValor Térmico\n";
    for (size_t i = 0; i < datos.size(); ++i) {
        archivo << i << "\t" << static_cast<int>(datos[i]) << " ºC\n";
    }

    archivo.close();
    cout << "[CLIENTE] Registro persistente guardado de forma segura en: " << ruta_archivo << endl;
    return true;
}

// ==============================================================================
// 4. FUNCIÓN: Mostrar estadísticas básicas de las lecturas por terminal
// ==============================================================================
void mostrarEstadisticasConsola(const vector<uint8_t>& datos) {
    if (datos.empty()) return;

    int sumatoria = 0;
    int maximo = datos[0];
    int minimo = datos[0];

    for (uint8_t valor : datos) {
        sumatoria += valor;
        if (valor > maximo) maximo = valor;
        if (valor < minimo) minimo = valor;
    }

    double promedio = static_cast<double>(sumatoria) / datos.size();

    cout << "\n=== RESUMEN TÉRMICO DE LA CÁMARA ===" << endl;
    cout << "  * Total muestras leídas : " << datos.size() << " puntos." << endl;
    cout << "  * Temperatura Mínima    : " << minimo << " ºC" << endl;
    cout << "  * Temperatura Máxima    : " << maximo << " ºC" << endl;
    cout << "  * Temperatura Promedio  : " << promedio << " ºC" << endl;
    cout << "====================================\n" << endl;
}

// ==============================================================================
// FUNCIÓN PRINCIPAL
// ==============================================================================
int main() {
    // Paso 1: Inicializar y conectar socket TCP
    int socket_fd = conectarConServidor(SERVER_IP, SERVER_PORT);
    if (socket_fd < 0) {
        return 1;
    }

    // Paso 2: Recibir flujo de bytes de la cámara térmica
    vector<uint8_t> datos_adquiridos;
    if (!recibirDatosRed(socket_fd, datos_adquiridos, TAMANO_ESPERADO)) {
        close(socket_fd);
        return 1;
    }

    // Paso 3: Cerrar el socket de red al terminar de leer (liberar recursos del SO)
    close(socket_fd);
    cout << "[CLIENTE] Socket de conexión cerrado." << endl;

    // Paso 4: Guardar lecturas en datos_temperatura.txt
    if (!guardarDatosDisco(ARCHIVO_SALIDA, datos_adquiridos)) {
        return 1;
    }

    // Paso 5: Mostrar analítica en pantalla
    mostrarEstadisticasConsola(datos_adquiridos);

    return 0;
}