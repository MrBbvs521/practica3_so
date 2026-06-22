#!/usr/bin/env python3
# ==============================================================================
# Práctica 3: Simulador de Cámara Térmica Industrial (Servidor TCP)
# ==============================================================================

import socket
import struct
import sys

# Configuración del servidor local
HOST = '127.0.0.1'  # Usamos localhost para desarrollo local estable en WSL
PORT = 12345

# Generar un array simulado de 768 lecturas (por ejemplo, resolución térmica de 32x24)
# Limitamos los valores al rango válido para un byte (0-255)
data_array = [i % 256 for i in range(768)]

print("=========================================================")
print("       INICIANDO SERVIDOR INDUSTRIAL DE CÁMARA TÉRMICA    ")
print("=========================================================")

try:
    # 1. CREACIÓN DEL SOCKET (Famila: AF_INET, Tipo: SOCK_STREAM para TCP)
    with socket.socket(socket.AF_INET, socket.SOCK_STREAM) as server_socket:
        
        # Permitir la reutilización del puerto inmediatamente tras cerrar el script
        server_socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        
        # 2. ENLAZAR EL PUERTO (Bind)
        server_socket.bind((HOST, PORT))
        
        # 3. ESCUCHAR CONEXIONES ENTRANTES (Listen)
        server_socket.listen(1)
        print(f"[SERVIDOR] Escuchando en {HOST}:{PORT} ...")
        
        # 4. ACEPTAR CONEXIÓN (Accept - Llamada bloqueante)
        conn, addr = server_socket.accept()
        with conn:
            print(f"[SERVIDOR] ¡Conexión aceptada desde el cliente {addr}!")
            
            # 5. EMPAQUETAR DATOS (Convertir 768 enteros a bytes)
            # El formato '768B' indica 768 enteros sin signo de 1 byte cada uno
            data_bytes = struct.pack('768B', *data_array)
            
            # 6. ENVIAR DATOS (Sendall garantiza que se transmitan todos los bytes)
            conn.sendall(data_bytes)
            print("[SERVIDOR] Transmisión exitosa: 768 bytes térmicos enviados.")
            
except KeyboardInterrupt:
    print("\n[SERVIDOR] Apagando el servidor de forma ordenada (SIGINT).")
except Exception as e:
    print(f"[SERVIDOR] Error inesperado en el socket: {e}", file=sys.stderr)