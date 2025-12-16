#include "Player.hh"

#include <unordered_map>

/**
 * Write the name of your player and save this file
 * with the same name and .cc extension.
 */
#define PLAYER_NAME PetaZZ


struct PLAYER_NAME : public Player {

  /**
   * Factory: returns a new instance of this class.
   * Do not modify this function.
   */
  static Player* factory () {
    return new PLAYER_NAME;
  }

  // Clase para resolver el problema de las tripletas
  class tripletas_solucio {
    public:
      tripletas_solucio(const vector<int> &Hint) : Hint_(Hint) { BackTracking(); }; // Al crearlo le pasas el vector y ejecutas el backtraknig

      vector<int> solve() { // devuelve el resultado
        return Solve_;
      }

    private:
      vector<int> Hint_;
      vector<int> Solve_;

      // markas 
      int num_tripleta_total;

      bool BackTracking_recursivo(int num_tripleta) {
        if (num_tripleta == num_tripleta_total) return true;

        for (int a = 0; a < Hint_.size(); a++) {
          if (Solve_[a] != -1) continue; // no estigui agafat ya
          for (int b = 1; b < Hint_.size(); b++) {
            if (Solve_[b] != -1 || a == b) continue; // no estigui agafat y no sigue igual als altres
            for (int c = 2; c < Hint_.size(); c++) {
              if (Solve_[c] != -1 || a == c || b == c) continue; // no estigui agafat y no sigue igual als altres

              bool correcto = (Hint_[a] + Hint_[b] == Hint_[c]) || (Hint_[a] + Hint_[c] == Hint_[b]) || (Hint_[c] + Hint_[b] == Hint_[a]);

              if (correcto) {
                // marcage
                Solve_[a] = num_tripleta; 
                Solve_[b] = num_tripleta;
                Solve_[c] = num_tripleta;

                if (BackTracking_recursivo(num_tripleta + 1)) {
                  return true; // Si s'ha arribat al final retorna true y se acaba
                }

                Solve_[a] = -1; 
                Solve_[b] = -1;
                Solve_[c] = -1;
              }         
            }
          }
        }

        return false; // si no se encuentra solucion retrocede
      }

      // Per fer l'inmercio
      void BackTracking() {
        Solve_ = vector<int>(Hint_.size(), -1);
        num_tripleta_total = Hint_.size() / 3;

        BackTracking_recursivo(0);

      }

  };

  /**
   * Types and attributes for your player can be defined here.
   */
  const vector<Dir> dirs = {Up,Down,Left,Right};

  const int distancia_maxima_busqueda = 1500;
  const int distancia_maxima_busqueda_prof_cajafuerte = 5000; // sin maximo

  /**
   * Funciones complementarias
   */

      // Soldado
  /*
  Busca dos objetivos con un limite de distancia 
  Despues el que este mas cercano al objetivo va hacia el 
  si el obj ya esta seleccionado por otro se dirige al segundo (aunque este pillado)
  en caso de llegar al limite se dirigue hacia alli
  */
  // estructuras para el dijkstra del soldado 
  map<Pos, int> pos_dist_soldier; // Posicion moneda a su min distancia
  unordered_map<int, pair<Pos, Pos>> id_obj_soldier; // id indicando sus dos objetivos 
  unordered_map<int, pair<Dir, Dir>> id_dir_soldier; // id indicando su direccion para los objetivos
  unordered_map<int, pair<int, int>> id_distancia_soldier; // id indicando la distancia
  // costes
  const int soldier_coste_cell_vacia      = 5;
  const int soldier_coste_kit_veneno      = 0; // para los que buscan monedas solo
  const int soldier_coste_monedas         = 1; // para los que buscan veneno solo
  const int soldier_coste_kit_salud       = 3;

  const int soldier_coste_enemigo_bien    = 1;  // si tú tienes veneno
  const int soldier_coste_enemigo_mal     = 45;  // si NO tienes veneno
  const int soldier_tabla_coste_enemigo[3] = {200, 25, 15}; // para las cercanas, indice = dist

  const int soldier_coste_amic            = 18;
  const int soldier_objetivo_ocupado       = 40; 

  // cuantos soldados buscan veneno solo en %
  const int numero_soldados_busca_veneno = 25;
  const int numero_soldados_busca_atacar = 15;


  // var que indica el veneno que nos queda esta ronda
  int veneno_restante = available_poison(me());

  // calcula distancia de dos pos
  int calc_dist(Pos a, Pos b) {
    return abs(a.i - b.i) + abs(a.j-b.j);
  }

  // Distancia de un enemigo (no cuenta un profesor no es una amenaza) a una casilla (tambien la usa el profe)
  int hay_enemigo_cerca(Pos &current_p) {
    for (int di = -2; di <= 2; ++di) {
        for (int dj = -2; dj <= 2; ++dj) {
            int dist_manhattan = abs(di) + abs(dj);
            if (dist_manhattan == 0 || dist_manhattan > 2) continue; 
            
            Pos check_p = Pos(current_p.i + di, current_p.j + dj);

            if (pos_ok(check_p.i, check_p.j)) {
                Cell target_cell = cell(check_p);
                
                if (target_cell.id != -1) {
                    Unit target_unit = unit(target_cell.id);
                    
                    if (target_unit.player != me() && target_unit.type == Soldier) {
                        return dist_manhattan; 
                    }
                }
            }
        }
    }
    return -1;
  }
  // Funcion coste enemigo cerca 
  int soldier_coste_enemigo_cerca(Pos &v_next) {
    int dist = hay_enemigo_cerca(v_next);

    if (dist != -1) {
      return soldier_tabla_coste_enemigo[dist];
    }

    return -1;
  }
  // Funcion para calcular costes de buscando monedas
  int soldier_calcular_costes_monedas(Cell &v_next_cell, bool &me_veneno, Pos &v_next) {
    // Calcular coste de la casilla
    int coste = soldier_coste_cell_vacia; // por default el valor de vacia

    if (v_next_cell.id != -1) { // soldado o profesor
      Unit u = unit(v_next_cell.id);
      if (u.player != me()) {
        coste = soldier_coste_enemigo_bien; // si es un enemigo sin veneno o profesor
        if (!me_veneno) coste = soldier_coste_enemigo_mal; // soldado con veneno, yo sin veneno
      }
      else coste = soldier_coste_amic;
    }
    else if (v_next_cell.health_kit) { // coste si hay un kit de salud
      coste = soldier_coste_kit_salud;
    }
    else if (v_next_cell.poison_kit) { // coste si hay un kit de veneno
      coste = soldier_coste_kit_veneno;
    }
    else if (v_next_cell.money) coste = 0;
    else if (pos_dist_soldier.count(v_next)) coste = soldier_objetivo_ocupado;
    
    if (!me_veneno) { // si hay algun enemigo cerca suma un pequeño coste
      int sol_cerca = soldier_coste_enemigo_cerca(v_next);
      if (sol_cerca != -1) coste += sol_cerca;
    }


    return coste;
  }
  // Funcion principal para el movimiento (busca monedas)
  //No cuenta las casillas con nuestro profesor
  void soldier_move_ponderado_monedas(Pos &s_pos, bool &me_veneno, int &id_soldier) {
    priority_queue<pair<int, Pos>, std::vector<std::pair<int, Pos>>, std::greater<std::pair<int, Pos>>> Q; // prio Q, coste y pos
    int mejor_coste[60][60];
    Dir primera_dir[60][60];
    bool visited[60][60]; 
    for (int i = 0; i < 60; i++) {
      for (int e = 0; e < 60; e++) visited[i][e] = false; 
    }


    Q.push({0, s_pos}); // añadimos inicial
    mejor_coste[s_pos.i][s_pos.j] = 0;
    primera_dir[s_pos.i][s_pos.j] = Down;
    visited[s_pos.i][s_pos.j] = true;

    int numero_obj = 0;

    while (!Q.empty()) {
      Pos v = Q.top().second;
      int v_coste = Q.top().first;
      Q.pop();

      // comprovacion de no util, si hay asignado un coste mayor salir
      if (visited[v.i][v.j] && v_coste > mejor_coste[v.i][v.j]) continue;

      Cell v_cell = cell(v);

      if (v_cell.money > 0) { // si hay monedas (guarda dos objetivos)
        numero_obj++;
        if (numero_obj == 1) {
          id_obj_soldier[id_soldier] = {v, v};
          id_dir_soldier[id_soldier] = {primera_dir[v.i][v.j], Up};
          id_distancia_soldier[id_soldier] = {v_coste, -1};
          if (!pos_dist_soldier.count(v) || pos_dist_soldier.at(v) > v_coste) pos_dist_soldier[v] = v_coste;
        }
        else {
          id_obj_soldier[id_soldier].second = v;
          id_dir_soldier[id_soldier].second = primera_dir[v.i][v.j];
          id_distancia_soldier[id_soldier].second = v_coste;
          if (!pos_dist_soldier.count(v) || pos_dist_soldier.at(v) > v_coste) pos_dist_soldier[v] = v_coste;
          
          return;
        }
      }
      else if (v_coste > distancia_maxima_busqueda) { // limite de distancia
        if (numero_obj == 0) {
          id_obj_soldier[id_soldier] = {{-1, -1}, {-1, -1}};
          id_dir_soldier[id_soldier] = {primera_dir[v.i][v.j], primera_dir[v.i][v.j]};
        }
        else {
          id_obj_soldier[id_soldier].second = {-1, -1};
          id_dir_soldier[id_soldier].second = primera_dir[v.i][v.j];          
        }
        return;
      }
      

      for (Dir d : dirs) {
        Pos v_next = v + d;

        if (pos_ok(v_next.i, v_next.j)) {
          Cell v_next_cell = cell(v_next);

          if (v_next_cell.type == Corridor) {
            if (v_next_cell.id != -1 && unit(v_next_cell.id).type == Professor && unit(v_next_cell.id).player == me() && calc_dist(v_next, s_pos) <= 10) continue; // donde este el soldado no puedas pasar

            int coste = soldier_calcular_costes_monedas(v_next_cell, me_veneno, v_next);

            coste += v_coste; // se acumula el coste

            if (!visited[v_next.i][v_next.j] || coste < mejor_coste[v_next.i][v_next.j]) {
              Q.push({coste, v_next});
            
              if (v == s_pos) {
                visited[v_next.i][v_next.j] = true;
                primera_dir[v_next.i][v_next.j] = d;
                mejor_coste[v_next.i][v_next.j] = coste;
              } else {
                visited[v_next.i][v_next.j] = true;
                primera_dir[v_next.i][v_next.j] = primera_dir[v.i][v.j];
                mejor_coste[v_next.i][v_next.j] = coste;
              }  
            }          
          }
        }
      }
    }

    id_obj_soldier[id_soldier] = {{-1, -1}, {-1, -1}};
    id_dir_soldier[id_soldier] = {dirs[random(0, 3)], Up};
  }


  // Funcion para calcular costes de buscando monedas
  int soldier_calcular_costes_veneno(Cell &v_next_cell, bool &me_veneno, Pos &v_next) {
    // Calcular coste de la casilla
    int coste = soldier_coste_cell_vacia; // por default el valor de vacia

    if (v_next_cell.id != -1) { // soldado o profesor
      Unit u = unit(v_next_cell.id);
      if (u.player != me()) {
        coste = soldier_coste_enemigo_bien; // si es un enemigo sin veneno o profesor
        if (!me_veneno) coste = soldier_coste_enemigo_mal; // soldado con veneno, yo sin veneno
      }
      else coste = soldier_coste_amic;
    }
    else if (v_next_cell.health_kit) { // coste si hay un kit de salud
      coste = soldier_coste_kit_salud;
    }
    else if (v_next_cell.poison_kit) { // coste si hay un kit de veneno
      coste = 0;
    }
    else if (v_next_cell.money) coste = soldier_coste_monedas;
    else if (pos_dist_soldier.count(v_next)) coste = soldier_objetivo_ocupado;
    
    if (!me_veneno) { // si hay algun enemigo cerca suma un pequeño coste
      int sol_cerca = soldier_coste_enemigo_cerca(v_next);
      if (sol_cerca != -1) coste += sol_cerca;
    }

    return coste;
  }
  /*
  Busqueda del soldado pero focalizando los venenos 
  se le asignaran a un par para intentar no quedarnos sin venenos
  No cuenta las casillas con nuestro profesor
  */
  void soldier_move_ponderado_veneno(Pos &s_pos, bool &me_veneno, int &id_soldier) {
    priority_queue<pair<int, Pos>, std::vector<std::pair<int, Pos>>, std::greater<std::pair<int, Pos>>> Q; // prio Q, coste y pos
    int mejor_coste[60][60];
    Dir primera_dir[60][60];
    bool visited[60][60]; 
    for (int i = 0; i < 60; i++) {
      for (int e = 0; e < 60; e++) visited[i][e] = false;
    }

    Q.push({0, s_pos}); // añadimos inicial
    mejor_coste[s_pos.i][s_pos.j] = 0;
    primera_dir[s_pos.i][s_pos.j] = Down;
    visited[s_pos.i][s_pos.j] = true;

    int numero_obj = 0;

    while (!Q.empty()) {
      Pos v = Q.top().second;
      int v_coste = Q.top().first;
      Q.pop();

      // comprovacion de no util, si hay asignado un coste mayor salir
      if (visited[v.i][v.j] && v_coste > mejor_coste[v.i][v.j]) continue;

      Cell v_cell = cell(v);

      if (v_cell.poison_kit) { // si hay monedas (guarda dos objetivos)
        numero_obj++;
        if (numero_obj == 1) {
          id_obj_soldier[id_soldier] = {v, v};
          id_dir_soldier[id_soldier] = {primera_dir[v.i][v.j], Up};
          id_distancia_soldier[id_soldier] = {v_coste, -1};
          if (!pos_dist_soldier.count(v) || pos_dist_soldier.at(v) > v_coste) pos_dist_soldier[v] = v_coste;
        }
        else {
          id_obj_soldier[id_soldier].second = v;
          id_dir_soldier[id_soldier].second = primera_dir[v.i][v.j];
          id_distancia_soldier[id_soldier].second = v_coste;
          if (!pos_dist_soldier.count(v) || pos_dist_soldier.at(v) > v_coste) pos_dist_soldier[v] = v_coste;
          
          return;
        }
      }
      else if (v_coste > distancia_maxima_busqueda) { // limite de distancia
        if (numero_obj == 0) {
          id_obj_soldier[id_soldier] = {{-1, -1}, {-1, -1}};
          id_dir_soldier[id_soldier] = {primera_dir[v.i][v.j], primera_dir[v.i][v.j]};
        }
        else {
          id_obj_soldier[id_soldier].second = {-1, -1};
          id_dir_soldier[id_soldier].second = primera_dir[v.i][v.j];          
        }
        return;
      }
      

      for (Dir d : dirs) {
        Pos v_next = v + d;

        if (pos_ok(v_next.i, v_next.j)) {
          Cell v_next_cell = cell(v_next);

          if (v_next_cell.type == Corridor) {
            if (v_next_cell.id != -1 && unit(v_next_cell.id).type == Professor && unit(v_next_cell.id).player == me() && calc_dist(v_next, s_pos) <= 10) continue; // donde este el soldado no puedas pasar

            int coste = soldier_calcular_costes_veneno(v_next_cell, me_veneno, v_next);

            coste += v_coste; // se acumula el coste

            if (!visited[v_next.i][v_next.j] || coste < mejor_coste[v_next.i][v_next.j]) {
              Q.push({coste, v_next});
            
              if (v == s_pos) {
                visited[v_next.i][v_next.j] = true;
                primera_dir[v_next.i][v_next.j] = d;
                mejor_coste[v_next.i][v_next.j] = coste;
              } else {
                visited[v_next.i][v_next.j] = true;
                primera_dir[v_next.i][v_next.j] = primera_dir[v.i][v.j];
                mejor_coste[v_next.i][v_next.j] = coste;
              }  
            }          
          }
        }
      }
    }

    id_obj_soldier[id_soldier] = {{-1, -1}, {-1, -1}};
    id_dir_soldier[id_soldier] = {dirs[random(0, 3)], Up};
  }


  /*
  Busqueda del soldado que le objetivo es un soldado enemigo
  Esta se aplica en menor medida simplemente para dar un toque agresivo
  */
  int soldier_calcular_costes_ataque(Cell &v_next_cell, Pos &v_next) {
    // Calcular coste de la casilla
    int coste = soldier_coste_cell_vacia; // por default el valor de vacia

    if (v_next_cell.id != -1) { // soldado o profesor
      Unit u = unit(v_next_cell.id);
      if (u.player != me()) {
        coste = 0; 
      }
      else coste = soldier_coste_amic;
    }
    else if (v_next_cell.health_kit) { // coste si hay un kit de salud
      coste = soldier_coste_kit_salud;
    }
    else if (v_next_cell.poison_kit) { // coste si hay un kit de veneno
      coste = soldier_coste_kit_veneno;
    }
    else if (v_next_cell.money) coste = soldier_coste_monedas;
    else if (pos_dist_soldier.count(v_next)) coste = soldier_objetivo_ocupado;

    return coste;
  }
  void soldier_move_ponderado_atacar(Pos &s_pos, int &id_soldier) {
    priority_queue<pair<int, Pos>, std::vector<std::pair<int, Pos>>, std::greater<std::pair<int, Pos>>> Q; // prio Q, coste y pos
    int mejor_coste[60][60];
    Dir primera_dir[60][60];
    bool visited[60][60]; 
    for (int i = 0; i < 60; i++) {
      for (int e = 0; e < 60; e++) visited[i][e] = false;
    }

    Q.push({0, s_pos}); // añadimos inicial
    mejor_coste[s_pos.i][s_pos.j] = 0;
    primera_dir[s_pos.i][s_pos.j] = Down;
    visited[s_pos.i][s_pos.j] = true;

    int numero_obj = 0;

    while (!Q.empty()) {
      Pos v = Q.top().second;
      int v_coste = Q.top().first;
      Q.pop();

      // comprovacion de no util, si hay asignado un coste mayor salir
      if (visited[v.i][v.j] && v_coste > mejor_coste[v.i][v.j]) continue;

      Cell v_cell = cell(v);

      if (v_cell.id != -1 && unit(v_cell.id).player != me()) { // si hay monedas (guarda dos objetivos)
        numero_obj++;
        if (numero_obj == 1) {
          id_obj_soldier[id_soldier] = {v, v};
          id_dir_soldier[id_soldier] = {primera_dir[v.i][v.j], Up};
          id_distancia_soldier[id_soldier] = {v_coste, -1};
          if (!pos_dist_soldier.count(v) || pos_dist_soldier.at(v) > v_coste) pos_dist_soldier[v] = v_coste;
        }
        else {
          id_obj_soldier[id_soldier].second = v;
          id_dir_soldier[id_soldier].second = primera_dir[v.i][v.j];
          id_distancia_soldier[id_soldier].second = v_coste;
          if (!pos_dist_soldier.count(v) || pos_dist_soldier.at(v) > v_coste) pos_dist_soldier[v] = v_coste;
          
          return;
        }
      }
      else if (v_coste > distancia_maxima_busqueda) { // limite de distancia
        if (numero_obj == 0) {
          id_obj_soldier[id_soldier] = {{-1, -1}, {-1, -1}};
          id_dir_soldier[id_soldier] = {primera_dir[v.i][v.j], primera_dir[v.i][v.j]};
        }
        else {
          id_obj_soldier[id_soldier].second = {-1, -1};
          id_dir_soldier[id_soldier].second = primera_dir[v.i][v.j];          
        }
        return;
      }
      

      for (Dir d : dirs) {
        Pos v_next = v + d;

        if (pos_ok(v_next.i, v_next.j)) {
          Cell v_next_cell = cell(v_next);

          if (v_next_cell.type == Corridor) {
            if (v_next_cell.id != -1 && unit(v_next_cell.id).type == Professor && unit(v_next_cell.id).player == me() && calc_dist(v_next, s_pos) <= 10) continue; // donde este el soldado no puedas pasar

            int coste = soldier_calcular_costes_ataque(v_next_cell, v_next);

            coste += v_coste; // se acumula el coste

            if (!visited[v_next.i][v_next.j] || coste < mejor_coste[v_next.i][v_next.j]) {
              Q.push({coste, v_next});
            
              if (v == s_pos) {
                visited[v_next.i][v_next.j] = true;
                primera_dir[v_next.i][v_next.j] = d;
                mejor_coste[v_next.i][v_next.j] = coste;
              } else {
                visited[v_next.i][v_next.j] = true;
                primera_dir[v_next.i][v_next.j] = primera_dir[v.i][v.j];
                mejor_coste[v_next.i][v_next.j] = coste;
              }  
            }          
          }
        }
      }
    }

    id_obj_soldier[id_soldier] = {{-1, -1}, {-1, -1}};
    id_dir_soldier[id_soldier] = {dirs[random(0, 3)], Up};
  }

  // detecta enemigos en un radio de 2 (contando esquinas)
  bool check_enemigo_cerca_soldier(Pos current_p) {
    for (int di = -2; di <= 2; ++di) {
        for (int dj = -2; dj <= 2; ++dj) {
            int dist_manhattan = abs(di) + abs(dj);
            if (dist_manhattan == 0 || dist_manhattan > 3) continue; 
            
            Pos check_p = Pos(current_p.i + di, current_p.j + dj);

            if (pos_ok(check_p.i, check_p.j)) {
                Cell target_cell = cell(check_p);
                
                if (target_cell.id != -1) {
                    Unit target_unit = unit(target_cell.id);
                    
                    if (target_unit.player != me()) {
                        return true; 
                    }
                }
            }
        }
    }
    return false;
  }

  // estrategia ofensiva, si estan a 1 casilla ataca lo mas rapido posible, si estan a dos ataca lo mas lento posible (esperar a que se acerquen)
  queue<pair<int, Dir>> ultimo_move;
  queue<pair<int, Dir>> less_prio;

  Dir left_gira(Dir d) {
    switch (d) {
      case Down:  return Right;
      case Right: return Up;
      case Up:    return Left;
      case Left:  return Down;
    }
    return Down; 
  }


  Dir right_gira(Dir d) {
    switch (d) {
      case Down:  return Left;
      case Left:  return Up;
      case Up:    return Right;
      case Right: return Down;
    }
    return Down; 
  }


  // sirve para decidir movimientos ofensivos contra las esquinas
  bool Comrpobar_esquinas(int id_soldado, const Unit& soldado_unit) {
    for (Dir d : dirs) {
      Pos delante = soldado_unit.pos + d;
      if (!pos_ok(delante) || cell(delante).type != Corridor) continue;

      // direcciones laterales
      Dir d1 = left_gira(d);
      Dir d2 = right_gira(d);

      Pos pared1 = soldado_unit.pos + d1;
      Pos diagonal1 = pared1 + d;

      if (pos_ok(pared1) && pos_ok(diagonal1) &&
        cell(pared1).type == Wall &&
        cell(diagonal1).id != -1 &&
        unit(cell(diagonal1).id).player != me() &&
        unit(cell(diagonal1).id).poison > 0) {
        ultimo_move.push({id_soldado, d});
        return true;
      }

      Pos pared2 = soldado_unit.pos + d2;
      Pos diagonal2 = pared2 + d;

      if (pos_ok(pared2) && pos_ok(diagonal2) &&
        cell(pared2).type == Wall &&
        cell(diagonal2).id != -1 &&
        unit(cell(diagonal2).id).player != me() &&
        unit(cell(diagonal2).id).poison > 0) {
        ultimo_move.push({id_soldado, d});
        return true;
      }
    }

    return false;
  }

  bool hasValue(Cell c) {
    if (c.money > 0 || c.poison_kit > 0 || c.health_kit > 0) return true;
    return false;
  }

  bool enemigo_check(Pos p) {
    if (!pos_ok(p)) return false;

    Cell posc = cell(p);
    if (posc.id == -1) return false;

    Unit u = unit(posc.id);
    return (u.player != me() && u.type == Soldier);
  }


  bool movimiento_ofensivo(int &soldier_id, Unit &soldier_unit) {

    if (soldier_unit.poison <= 0) return false; // no ataca
  
    for (Dir d : dirs) {
      Pos new_pos = soldier_unit.pos + d;

      if (!pos_ok(new_pos)) continue;

      if (cell(new_pos).id != -1 && unit(cell(new_pos).id).player != me()) { // si hay uno en dist 1 atacar
        move(soldier_id, d);
        return true;
      }
    }

    for (Dir d : dirs) {
      Pos new_pos = soldier_unit.pos + d;
      if (!pos_ok(new_pos)) continue;

      // baitear amb monedas veri o cures
      if (hasValue(cell(new_pos))) {
        Dir ld = left_gira(d);
        Dir rd = right_gira(d); 
        Pos lPos = new_pos + ld;
        Pos dPos = new_pos + d;
        Pos rPos = new_pos + rd;

        if ((pos_ok(lPos) && enemigo_check(lPos)) || (pos_ok(dPos) && enemigo_check(dPos)) || (pos_ok(rPos) && enemigo_check(rPos))) {
          ultimo_move.push({soldier_id, d});
          return true;
        }
      }
    }

    for (Dir d : dirs) { // si hay uno a distancia 2, espero que se acerque y ataco
      Pos new_pos_1 = soldier_unit.pos + d;
      Pos new_pos_2 = new_pos_1 + d;

      if (!pos_ok(new_pos_1) || !pos_ok(new_pos_2) || cell(new_pos_1).type == Wall || cell(new_pos_2).id== -1) continue;

      Unit obj = unit(cell(new_pos_2).id);

      if (obj.player != me() && obj.poison > 0) { 
        ultimo_move.push({soldier_id, d});
        return true;
      }
    }

    // esquinas 
    if (Comrpobar_esquinas(soldier_id, soldier_unit)) return true;
     
    return false;
  }

  // ejecuta los move que deben ser lo ultimo
  void ejecutar_ultimos_mov() {
    while (!less_prio.empty()) {
        pair<int, Dir> mov = less_prio.front();
        less_prio.pop();
        move(mov.first, mov.second);
    }

    while (!ultimo_move.empty()) {
      pair<int, Dir> i = ultimo_move.front(); ultimo_move.pop();

      move(i.first, i.second);
    }

  }

  bool enemigo_check_veneno(Pos p) {
    if (!pos_ok(p)) return false;

    Cell posc = cell(p);
    if (posc.id == -1) return false;

    Unit u = unit(posc.id);
    return (u.player != me() && u.type == Soldier && u.poison > 0);
  }

  bool enemigo_next(Pos p) {
    Pos lp = p + Left;
    Pos rp = p + Right;
    Pos up = p + Up;
    Pos dp = p + Down;

    if (enemigo_check_veneno(lp) || enemigo_check_veneno(rp) || enemigo_check_veneno(up) || enemigo_check_veneno(dp)) {
        return true;
    }
    return false;
  }

  // Funcion que centraliza la logica del movimiento de los soldados 
  void act_soldier() {
    int contador_cuantos_veneno = 0;
    int contador_cuantos_atacan = 0;
    vector<int> soldier_id_v = soldiers(me());

    veneno_restante = available_poison(me());

    int num_soldados_veneno = (soldier_id_v.size() * numero_soldados_busca_veneno)/100;
    int num_soldados_ataque = (soldier_id_v.size() * numero_soldados_busca_atacar)/100;

    for (int id : soldier_id_v) { 
      Unit soldier_unit = unit(id);
      if (soldier_unit.poison == 0 && veneno_restante > 0 && check_enemigo_cerca_soldier(soldier_unit.pos)) {
        charge(id);
        veneno_restante--;
      } else {
        if (!movimiento_ofensivo(id, soldier_unit)) { // si necesita hacer algo urgente lo hace sino busca oro
          bool tiene_veneno = (soldier_unit.poison > 0);
          if (contador_cuantos_veneno < num_soldados_veneno) {
            soldier_move_ponderado_veneno(soldier_unit.pos, tiene_veneno, id);
            contador_cuantos_veneno++;
          }
          else if (contador_cuantos_atacan < num_soldados_ataque && tiene_veneno) { // busca atacar quien tiene veneno
            soldier_move_ponderado_atacar(soldier_unit.pos, id);
            contador_cuantos_atacan++;
          }
          else soldier_move_ponderado_monedas(soldier_unit.pos, tiene_veneno, id);

        }
      }

    }

    queue<pair<int, Dir>> normal_move;
    
    for (const auto& [id_soldado, objetivos] : id_obj_soldier) {
      Pos objetivo_C1 = objetivos.first;

      // Primero se hacen los movimientos con mas prioridad (distancia 1)
      if (id_distancia_soldier[id_soldado].first == pos_dist_soldier[objetivo_C1] || id_obj_soldier[id_soldado].first == Pos(-1, -1)) { // es el mas cercano a la pos
        if (!enemigo_next((unit(id_soldado).pos + id_dir_soldier[id_soldado].first))) {
          normal_move.push({id_soldado, id_dir_soldier[id_soldado].first});
        }
        else {
          less_prio.push({id_soldado, id_dir_soldier[id_soldado].first});
        }
        
      }
      else {
        if (!enemigo_next((unit(id_soldado).pos + id_dir_soldier[id_soldado].second))) {
          normal_move.push({id_soldado, id_dir_soldier[id_soldado].second});
        }
        else {
          less_prio.push({id_soldado, id_dir_soldier[id_soldado].second});
        }
      }
      
    }

    // movimiento rutinario, sin ninguna prioridad
    while (!normal_move.empty()) {
      pair<int, Dir> mov = normal_move.front(); 
      normal_move.pop();
      move(mov.first, mov.second);
    }

    pos_dist_soldier.clear();
    id_obj_soldier.clear();  
    id_dir_soldier.clear();  
    id_distancia_soldier.clear();
  }

      // Profesor
  /*
  Algoritmo de movimiento del profesor hacia cajas fuertes 
  Como los enemigos no se contabilizan como posible casilla puede no encontrar solucion
  En dicho caso se aplica un segundo algoritmo de busqueda que dirije hacia monedas o venenos, para no desaprovechar-lo aunque no de su maximo
  */
  // costes
  const int profesor_coste_cell_vacia      = 5;
  const int profesor_coste_kit_veneno      = 0; 
  const int profesor_coste_monedas         = 0; 
  const int profesor_coste_kit_salud       = 0;

  const int profesor_coste_enemigo_mal     = 60;  // si NO tienes veneno
  const int profesor_tabla_coste_enemigo[3] = {200, 25, 15}; // para las cercanas, indice = dist

  const int profesor_coste_amic            = 6; // simbolitza los compañeros o profesores rivales que no suponen amenaza

  // Comprueba si hay una puerta accesible
  bool check_door_profe(Pos prof_pos) {
    Dir door_dir;
    for (Dir d : dirs) {
      Pos new_pos = prof_pos + d; if (!pos_ok(new_pos)) continue;
      Pos new_pos2 = new_pos + d; if (!pos_ok(new_pos2)) continue;
      Cell new_pos_cell = cell(new_pos);
      Cell new_pos_cell2 = cell(new_pos2);
      if (new_pos_cell.box != -1 && new_pos_cell2.gold) {
        return true;
      } 
    }

    return false;
  }
  // Funcion coste enemigo cerca
  int profesor_coste_enemigo_cerca(Pos &v_next) {
    int dist = hay_enemigo_cerca(v_next);

    if (dist != -1) {
      return profesor_tabla_coste_enemigo[dist];
    }

    return -1;
  }
  // Funcion para calcular costes de cada casilla de la busqueda de cajas fuertes
  int profesor_calcular_costes_cajafuerte(Cell &v_next_cell, Pos &v_next) {
    // Calcular coste de la casilla
    int coste = profesor_coste_cell_vacia; // por default el valor de vacia

    if (v_next_cell.id != -1) { // soldado o profesor
      Unit u = unit(v_next_cell.id);
      if (u.player != me() && u.type == Soldier) {
        coste = profesor_coste_enemigo_mal; // soldado con veneno, yo sin veneno
      }
      else coste = profesor_coste_amic;
    }
    else if (v_next_cell.health_kit) { // coste si hay un kit de salud
      coste = profesor_coste_kit_salud;
    }
    else if (v_next_cell.poison_kit) { // coste si hay un kit de veneno
      coste = profesor_coste_kit_veneno;
    }
    else if (v_next_cell.money) coste = profesor_coste_monedas;
    
    int sol_cerca = profesor_coste_enemigo_cerca(v_next);
    if (sol_cerca != -1) coste += sol_cerca;


    return coste;
  }
  // calcula el path
  bool fail = false; // por si no encuentra nada
  Dir profesor_move_ponderado_cajafuerte(Pos &s_pos, int &id_prof) {
    fail = false;
    priority_queue<pair<int, Pos>, std::vector<std::pair<int, Pos>>, std::greater<std::pair<int, Pos>>> Q; // prio Q, coste y pos
    int mejor_coste[60][60];
    Dir primera_dir[60][60];
    bool visited[60][60]; 
    for (int i = 0; i < 60; i++) {
      for (int e = 0; e < 60; e++) visited[i][e] = false;
    }

    Q.push({0, s_pos}); // añadimos inicial

    while (!Q.empty()) {
      Pos v = Q.top().second;
      int v_coste = Q.top().first;
      Q.pop();

      // comprovacion de no util, si hay asignado un coste mayor salir
      if (visited[v.i][v.j] && v_coste > mejor_coste[v.i][v.j]) continue;

      // falta añadir comprovacion de si es una pos con acceso a una puerta 
      if (check_door_profe(v)) { // si hay monedas (guarda dos objetivos)
        return primera_dir[v.i][v.j];
      }
      else if (v_coste > distancia_maxima_busqueda_prof_cajafuerte) { // limite de distancia
        fail = true;
        return dirs[random(0, 3)];
      }
      

      for (Dir d : dirs) {
        Pos v_next = v + d;

        if (pos_ok(v_next.i, v_next.j)) {
          Cell v_next_cell = cell(v_next);

          if (v_next_cell.type == Corridor) {
            if (v_next_cell.id != -1 && unit(v_next_cell.id).type == Soldier && unit(v_next_cell.id).player != me() && calc_dist(v_next, s_pos) <= 2) continue;

            int coste = profesor_calcular_costes_cajafuerte(v_next_cell, v_next);

            coste += v_coste; // se acumula el coste

            if (!visited[v_next.i][v_next.j] || coste < mejor_coste[v_next.i][v_next.j]) {
              Q.push({coste, v_next});
            
              if (v == s_pos) {
                visited[v_next.i][v_next.j] = true;
                primera_dir[v_next.i][v_next.j] = d;
                mejor_coste[v_next.i][v_next.j] = coste;
              } else {
                visited[v_next.i][v_next.j] = true;
                primera_dir[v_next.i][v_next.j] = primera_dir[v.i][v.j];
                mejor_coste[v_next.i][v_next.j] = coste;
              } 
            }          
          }
        }
      }
    }

    fail = true;

    return Up; // no deberia llegar, si llega buscara monedas
  }
  // Comprueba si hay una puerta accesible y si la hay la abre
  bool check_open_door_profe(int &id_prof, Pos prof_pos) {
    Dir door_dir;
    for (Dir d : dirs) {
      Pos new_pos = prof_pos + d; if (!pos_ok(new_pos)) continue;
      Pos new_pos2 = new_pos + d; if (!pos_ok(new_pos2)) continue;
      Cell new_pos_cell = cell(new_pos);
      Cell new_pos_cell2 = cell(new_pos2);
      if (new_pos_cell.box != -1 && new_pos_cell2.gold) {
        vector<int> hints = safety_box_hints(new_pos_cell.box);

        tripletas_solucio solucio(hints); // lo resuelves 
        vector<int> sol = solucio.solve();  // lees el valor

        open(id_prof, sol, d); // Calcula la solucion y abre la caja

        return true;
      } 
    }

    return false;
  }
  // Una funcion para moverse hacia lo mas cercano de interes, se utiliza por si no se encuentra caja fuerte
  Dir move_to_anything(Pos &s_pos, int &id_prof) {
    priority_queue<pair<int, Pos>, std::vector<std::pair<int, Pos>>, std::greater<std::pair<int, Pos>>> Q; // prio Q, coste y pos
    int mejor_coste[60][60];
    Dir primera_dir[60][60];
    bool visited[60][60]; 
    for (int i = 0; i < 60; i++) {
      for (int e = 0; e < 60; e++) visited[i][e] = false;
    }

    Q.push({0, s_pos}); // añadimos inicial

    while (!Q.empty()) {
      Pos v = Q.top().second;
      int v_coste = Q.top().first;
      Q.pop();

      // comprovacion de no util, si hay asignado un coste mayor salir
      if (visited[v.i][v.j] && v_coste > mejor_coste[v.i][v.j]) continue;

      // falta añadir comprovacion de si es una pos con acceso a una puerta 
      if (cell(v).money > 0 || cell(v).poison_kit > 0) { // si hay monedas (guarda dos objetivos)
        return primera_dir[v.i][v.j];
      }
      else if (v_coste > distancia_maxima_busqueda_prof_cajafuerte) { // limite de distancia
        return dirs[random(0, 3)];
      }
      

      for (Dir d : dirs) {
        Pos v_next = v + d;

        if (pos_ok(v_next.i, v_next.j)) {
          Cell v_next_cell = cell(v_next);

          if (v_next_cell.type == Corridor) {
            if (v_next_cell.id != -1 && unit(v_next_cell.id).type == Soldier && unit(v_next_cell.id).player != me()) continue;

            int coste = profesor_calcular_costes_cajafuerte(v_next_cell, v_next);

            coste += v_coste; // se acumula el coste

            if (!visited[v_next.i][v_next.j] || coste < mejor_coste[v_next.i][v_next.j]) {
              Q.push({coste, v_next});
            
              if (v == s_pos) {
                visited[v_next.i][v_next.j] = true;
                primera_dir[v_next.i][v_next.j] = d;
                mejor_coste[v_next.i][v_next.j] = coste;
              } else {
                visited[v_next.i][v_next.j] = true;
                primera_dir[v_next.i][v_next.j] = primera_dir[v.i][v.j];
                mejor_coste[v_next.i][v_next.j] = coste;
              } 
            }          
          }
        }
      }
    }

    return dirs[random(0, 3)]; // no deberia llegar
  }
  // Gestiona los movimientos del profesor
  void act_prof() {
    int id_prof = professor(me());

    Unit prof_unit = unit(id_prof);
    
    if (prof_unit.health < 50 && available_health(me()) > 0) {
      heal(id_prof);
    }
    else {
      // movimiento a las cajas y mirar si hay una a su lado 
      if (!check_open_door_profe(id_prof, prof_unit.pos)) { // si no ha abierto una puerta se mueve
        Dir move_prof = profesor_move_ponderado_cajafuerte(prof_unit.pos, id_prof);
        if (fail) {
          move_prof = move_to_anything(prof_unit.pos, id_prof);
        }
        else 
        move(id_prof, move_prof);
      }
    }
  }



  /**
   * Play method, invoked once per each round.
   */
  virtual void play () {

    act_soldier(); // logica de los soldados
    act_prof(); // logica del profesor
    ejecutar_ultimos_mov(); // movimientos que por estrategia deben ser los ultimos
  }

};


/**
 * Do not modify the following line.
 */
RegisterPlayer(PLAYER_NAME);


// play -> ./Game Demo Demo Demo PetaZ -s 30 < default.cnf > default.res
// ./Game Demo Dummy Dummy PetaZ_v0 -s 334 < default.cnf > default.res

