#!/usr/bin/env bash
# ==============================================================================
# Práctica 3: Script de Automatización de Despliegue de Red TCP/IP
# ==============================================================================

set -euo pipefail

CLIENT_SRC="cliente.cpp"
CLIENT_BIN="./cliente"
SERVER_SCRIPT="ServidorCamara.py"
PORT=12345

echo "====================================================================="
echo "       INICIANDO ARQUITECTURA DE RED CLIENTE-SERVIDOR (UBU)         "
echo "====================================================================="

# 1. COMPILACIÓN DEL CLIENTE C++
if [[ ! -f "$CLIENT_SRC" ]]; then
    echo "[ERROR BASH] No se encuentra el archivo fuente $CLIENT_SRC" >&2
    exit 1
fi

echo "[Bash 1/5] Compilando el cliente C++ con estándar C++17..."
g++ -std=c++17 "$CLIENT_SRC" -o cliente
echo "[OK] Compilación del cliente realizada con éxito."

# 2. LIMPIAR PUERTOS DE CONEXIÓN PREVIOS (Failsafe de Red)
# Evita el molesto error "Address already in use" de sockets residuales en el Kernel.
if lsof -i :$PORT >/dev/null 2>&1; then
    echo "[Bash] Detectado puerto $PORT ocupado. Liberando puerto de red..."
    fuser -k $PORT/tcp 2>/dev/null || true
    sleep 1
fi

# 3. LANZAR EL SERVIDOR PYTHON EN SEGUNDO PLANO
if [[ ! -f "$SERVER_SCRIPT" ]]; then
    echo "[ERROR BASH] No se encuentra el script del servidor $SERVER_SCRIPT" >&2
    exit 1
fi

echo "[Bash 2/5] Lanzando servidor de cámara térmica en segundo plano..."
python3 "$SERVER_SCRIPT" &
SERVER_PID=$!
echo "[INFO] Servidor levantado en background (PID: $SERVER_PID)"

# Esperar un instante para asegurar que el servidor abra el socket y escuche (listen)
sleep 2

# 4. EJECUTAR EL CLIENTE C++
echo "[Bash 3/5] Ejecutando el cliente C++ para realizar la ingesta..."
if ! $CLIENT_BIN; then
    echo "[ERROR BASH] El cliente falló durante la adquisición de red." >&2
    # Asegurar el apagado del servidor si el cliente falla
    kill -SIGINT "$SERVER_PID" 2>/dev/null || true
    exit 1
fi

# 5. FINALIZACIÓN Y LIMPIEZA DE PROCESOS (Evitar sockets huérfanos)
echo "[Bash 4/5] Adquisición completada. Deteniendo servidor de forma segura..."

# Enviamos señal amigable de terminación al servidor
if kill -0 "$SERVER_PID" 2>/dev/null; then
    kill -SIGINT "$SERVER_PID" 2>/dev/null
    sleep 1
    
    # Failsafe con SIGKILL si se queda bloqueado el hilo de ejecución
    if kill -0 "$SERVER_PID" 2>/dev/null; then
        echo "[BASH ALERTA] Servidor bloqueado. Forzando cierre con SIGKILL..."
        kill -SIGKILL "$SERVER_PID" 2>/dev/null
    fi
fi

# Esperar la recolección del proceso hijo por parte de este script padre
wait "$SERVER_PID" 2>/dev/null || true

echo "[Bash 5/5] Despliegue finalizado. Arquitectura de red cerrada limpia"