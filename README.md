# EDA-Game
**Bot for the EDA game (Data Structures and Algorithms) that placed in the top 15 (out of 320).**

# Money Heist AI Player: PetaZZ

Este repositorio contiene la implementación de un bot ("PetaZZ") escrito en C++ para el reto de programación **Money Heist**. El objetivo es controlar un equipo de soldados y un profesor en un tablero de 60x60 para maximizar la puntuación recolectando dinero, abriendo cajas fuertes y eliminando enemigos.

## Estrategia y Algoritmos

El jugador utiliza una combinación de algoritmos de búsqueda de caminos, lógica combinatoria y heurísticas de combate.

### El Profesor
El profesor es la unidad clave para obtener grandes puntuaciones mediante las cajas fuertes.
* **Resolución de Cajas Fuertes (Backtracking):** Implementa una clase `tripletas_solucio` que utiliza un algoritmo recursivo de **Backtracking** para resolver el puzzle matemático necesario para abrir las cajas (encontrar tripletas de números que cumplan condiciones de suma).
* **Navegación Inteligente:** Utiliza **Dijkstra** con costes ponderados para encontrar la caja fuerte más segura, evitando casillas adyacentes a soldados enemigos.
* **Gestión de Salud:** Incluye lógica de auto-preservación; si la salud baja del 50%, busca activamente curación en el bote común.

### Los Soldados
Los soldados operan bajo roles dinámicos y mapas de calor de costes para decidir su movimiento.
* **Pathfinding Ponderado:** Se utiliza un algoritmo de búsqueda basado en colas de prioridad (`priority_queue`) que calcula el coste de cada casilla.
    * *Huida:* Si el soldado no tiene veneno, las casillas cerca de enemigos tienen un coste muy alto.
    * *Caza:* Si tiene veneno, el coste hacia los enemigos disminuye drásticamente.
* **Roles Dinámicos:** En cada ronda, se asignan porcentajes de la tropa a tareas específicas:
    * **25%** priorizan buscar kits de veneno.
    * **15%** priorizan atacar enemigos activamente.
    * El resto se centra en la recolección de dinero.
* **Tácticas de Combate:**
Sistema de prioridades:
    * **Detección de Esquinas:** Algoritmo específico para detectar y emboscar enemigos a través de las paredes en los pasillos.
    * **Baiteo:** Lógica para esperar en posición si un enemigo se acerca, forzando al rival a entrar en el rango de ataque.
    * **Baiteo de recursos:** Misma lógica del baiteo, pero aplicada si hay un recurso cerca, utilizándolo de cebo.
    * **Peligro inminente:** Ataque con máxima prioridad si el enemigo se encuentra adyacente.

## Tecnologías

* **Lenguaje:** C++
* **Algoritmos:** Dijkstra, Backtracking, Heurísticas de riesgo.
* **Estructuras de Datos:** `priority_queue`, `unordered_map`, `vector`.

---
*Este bot fue desarrollado como parte de un reto de algoritmia y estructuras de datos.*
