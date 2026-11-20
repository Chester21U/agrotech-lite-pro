# AgroSmart: Monitoreo Agrícola Inteligente

Monitoreo agrícola, riego automático e IA en el borde con ESP8266 y Raspberry Pi 3. Todo funciona en red local, sin internet ni servicios en la nube.

Feria de Hardware y Automatización · Universidad Gerardo Barrios · Grupo A1

## Módulos

| Módulo | Hardware | Función |
|---|---|---|
| A · Nodo de cultivo | NodeMCU ESP8266, sensor capacitivo de suelo, DHT11, relé, mini bomba 5 V | Mide suelo y clima, riega con histéresis 30 % / 60 % y corta la bomba a los 10 s |
| B · Visión e IA | Raspberry Pi 3, cámara USB, LED, zumbador | Clasifica hojas en sana, plaga o fondo y alerta si detecta plaga |
| C · Dashboard | Node-RED + Dashboard 2.0 en la Raspberry Pi 3 | Indicadores, histórico, modo auto/manual, control de bomba, video y estado del cultivo |

## Arquitectura

La Raspberry Pi 3 crea la red Wi-Fi `AgroSmart` y aloja el broker MQTT (Mosquitto) en `10.42.0.1`. El ESP8266, el script de visión y Node-RED se comunican únicamente a través del broker.

| Tópico | Publica | Contenido |
|---|---|---|
| `agrosmart/nodo1/telemetria` | ESP8266 | JSON cada 2 s: `t`, `ha`, `hs`, `bomba`, `modo`, `bloqueo` |
| `agrosmart/nodo1/modo` | Node-RED | `auto` o `manual` |
| `agrosmart/nodo1/bomba` | Node-RED | `on`, `off` o `reset` |
| `agrosmart/nodo1/alarma` | ESP8266 | `anti_inundacion` o `deposito_vacio` |
| `agrosmart/ia/estado` | Visión | JSON: `clase`, `conf`, `alerta` |

## Conexiones del nodo

| Componente | Su pin | NodeMCU |
|---|---|---|
| Sensor capacitivo de suelo | VCC / GND / AOUT | 3V / G / A0 |
| DHT11 | VCC / GND / DATA | 3V / G / D2 |
| Módulo relé 5 V | VCC / GND / IN | VU / G / D1 |

La bomba se alimenta de una fuente de 5 V independiente a través del relé, con un diodo 1N4148 en paralelo sobre el motor.

## Estructura

```
agrosmart/
├── firmware/nodo-cultivo/   Firmware del ESP8266 (PlatformIO)
├── vision/                  Script de visión y modelo
├── dashboard/               flows.json de Node-RED
├── hardware/                Diagramas de conexión y fotos de la maqueta
└── docs/                    Informe, presentación y manual
```

## Configuración local

El archivo `firmware/nodo-cultivo/include/secrets.h` no se sube al repositorio. Cada integrante lo crea a partir del ejemplo:

```
cd firmware/nodo-cultivo/include
cp secrets.example.h secrets.h
```

## Servidor

| Servicio | Dirección |
|---|---|
| Red Wi-Fi del proyecto | `AgroSmart` |
| Broker MQTT | `10.42.0.1:1883` |
| Editor de Node-RED | `http://10.42.0.1:1880` |
| Dashboard | `http://10.42.0.1:1880/dashboard` |

## Flujo de trabajo

- `main`: versiones estables.
- `dev`: integración de módulos.
- `feature/<tarea>`: una rama por tarea, con Pull Request hacia `dev`.

Commits en español con prefijo `feat`, `fix`, `docs` o `chore`.

## Equipo

- Abraham Bladimir Umanzor Vásquez
- Andy Jhoan Navarrete Ramos
- Freddy Samuel Vásquez Álvarez
- Héctor Eduardo Contreras Romero
- Michaell Daniel Chavarria Romero

Universidad Gerardo Barrios · Ingeniería en Sistemas y Redes Informáticas · 2026
