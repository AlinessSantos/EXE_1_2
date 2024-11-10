#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <thread>
#include <chrono>
#include <mutex>

// Representação do labirinto
using Maze = std::vector<std::vector<char>>;

// Estrutura para representar uma posição no labirinto
struct Position {
    int row;
    int col;
};

// Variáveis globais
Maze maze;
int num_rows;
int num_cols;
std::stack<Position> valid_positions;
std::mutex mtx;  // Mutex para proteger o acesso ao labirinto

// Função para carregar o labirinto de um arquivo
Position load_maze(const std::string& file_name) {
    std::ifstream arquivo(file_name);
    if(!arquivo){
        std::cerr << "Erro ao abrir arquivo!" << std::endl;
        std::exit(EXIT_FAILURE);
    }

    arquivo >> num_rows >> num_cols;
    maze.resize(num_rows, std::vector<char>(num_cols));

    for(int i = 0; i < num_rows; i++){
        for(int j = 0; j < num_cols; j++){
            arquivo >> maze[i][j];
        }
    }

    arquivo.close();

    for(int i = 0; i < num_rows; i++){
        for(int j = 0; j < num_cols; j++){
            if(maze[i][j] == 'e'){
                return {i, j};
            }
        }
    }

    return {-1, -1}; // Caso a posição inicial não seja encontrada
}

// Função para imprimir o labirinto
void print_maze() {
    //std::lock_guard<std::mutex> lock(mtx);  // Trava o mutex para acessar o labirinto
    for(int i = 0; i < num_rows; i++){
        for(int j = 0; j < num_cols; j++){
            std::cout << maze[i][j];
        }
        std::cout << std::endl;
    }
    std::cout << "------------------------" << std::endl;
}

// Função para verificar se uma posição é válida
bool is_valid_position(int row, int col) {
    return (row >= 0 && row < num_rows && col >= 0 && col < num_cols && 
           (maze[row][col] == 'x' || maze[row][col] == 's'));
}

// Função principal para navegar pelo labirinto (com threads)
bool walk(Position pos);

// Função para lidar com a exploração de caminhos em novas threads
void explore_in_thread(Position pos) {
    walk(pos);  // Inicia a exploração em uma nova thread
}

// Função principal para navegar pelo labirinto
bool walk(Position pos) {
    bool found_exit = false;
    // Marca a posição atual como visitada
    
    //std::lock_guard<std::mutex> lock(mtx); // Protege o acesso ao labirinto
    if (maze[pos.row][pos.col] == 's') {
        found_exit = true;
    }
    maze[pos.row][pos.col] = '.';  // Marca como percorrido
    print_maze();
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    if (found_exit) {
        return true;
    }

    // Vizinhos possíveis (cima, direita, baixo, esquerda)
    std::vector<Position> neighbors = {
        {pos.row - 1, pos.col}, {pos.row, pos.col + 1},
        {pos.row + 1, pos.col}, {pos.row, pos.col - 1}
    };
    std::vector<Position> valid_moves;

    // Verifica os movimentos válidos
    for (const auto& neighbor : neighbors) {
        if (is_valid_position(neighbor.row, neighbor.col)) {
            valid_moves.push_back(neighbor);
        }
    }
    if (valid_moves.empty()) {
        return false;  // Sem mais movimentos
    }

    std::vector<std::thread> threads;
    
    // Se houver mais de um caminho válido, cria threads para explorar
    for (size_t i = 0; i < valid_moves.size(); i++) {
        threads.emplace_back(explore_in_thread, valid_moves[i]);
    }

    // Continua explorando o primeiro caminho na thread atual
    bool result = walk(valid_moves[0]);

    // Aguarda todas as threads terminarem
    for (auto& t : threads) {
        t.join();
    }

    return result;
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <arquivo_labirinto>" << std::endl;
        return 1;
    }
    Position initial_pos = load_maze(argv[1]);
    if (initial_pos.row == -1 || initial_pos.col == -1) {
        std::cerr << "Posição inicial não encontrada no labirinto." << std::endl;
        return 1;
    }
    std::cout << "Oi 1" << std::endl;
    bool exit_found = walk(initial_pos);

    if (exit_found) {
        std::cout << "Saída encontrada!" << std::endl;
    } else {
        std::cout << "Não foi possível encontrar a saída." << std::endl;
    }

    return 0;
}
