# AgroTech Lite Pro

Monitoreo agrícola, riego automático e IA en el borde con ESP32 y Raspberry Pi 5. Todo funciona en red local, sin internet ni servicios en la nube.

## Módulos

| Módulo | Hardware | Función |
|---|---|---|
| A · Nodo de cultivo | ESP32, sensor capacitivo de suelo, DHT22, relé, mini bomba 5 V | Mide suelo y clima, riega con histéresis 30 % / 60 % y corta la bomba a los 10 s |
| B · Visión e IA | Raspberry Pi 5, cámara USB, LED, zumbador | Clasifica hojas en sana, plaga o fondo con YOLO y alerta si detecta plaga |
| C · Dashboard | Node-RED + Dashboard 2.0 en la Raspberry Pi 5 | Indicadores, histórico, modo auto/manual, control de bomba, video y estado del cultivo |

## Arquitectura

La Raspberry Pi 5 crea la red Wi-Fi `AgroTech` y aloja el broker MQTT (Mosquitto). El ESP32, el script de visión y Node-RED se comunican únicamente a través del broker.

| Tópico | Publica | Contenido |
|---|---|---|
| `agrotech/nodo1/telemetria` | ESP32 | JSON cada 2 s: `t`, `ha`, `hs`, `bomba`, `modo`, `bloqueo` |
| `agrotech/nodo1/modo` | Node-RED | `auto` o `manual` |
| `agrotech/nodo1/bomba` | Node-RED | `on`, `off` o `reset` |
| `agrotech/nodo1/alarma` | ESP32 | `anti_inundacion` o `deposito_vacio` |
| `agrotech/ia/estado` | Visión | JSON: `clase`, `conf`, `alerta` |

## Estructura

```
agrotech-lite-pro/
├── firmware/nodo-cultivo/   Firmware del ESP32 (PlatformIO)
├── vision/                  Script de visión y modelo
├── dashboard/               flows.json de Node-RED
├── hardware/                Diagramas de conexión y fotos de la maqueta
└── docs/                    Informe, presentación y manual
```

## Configuración local

El archivo `firmware/nodo-cultivo/include/secrets.h` no se sube al repositorio. Cada integrante lo crea a partir del ejemplo:

```
cp firmware/nodo-cultivo/include/secrets.example.h firmware/nodo-cultivo/include/secrets.h
```

## Flujo de trabajo

- `main`: versiones estables.
- `dev`: integración de módulos.
- `feature/<tarea>`: una rama por tarea, con Pull Request hacia `dev`.

Commits en español con prefijo `feat`, `fix`, `docs` o `chore`.

## Equipo

| Integrante | Responsable de |
|---|---|
| Abraham Umanzor | Coordinación, Raspberry Pi e integración |
| Por definir | Módulo A: circuito y firmware del ESP32 |
| Por definir | Módulo B: dataset, modelo y visión |
| Por definir | Módulo C: dashboard y maqueta |

Universidad Gerardo Barrios · Ingeniería en Sistemas y Redes Informáticas · 2026
