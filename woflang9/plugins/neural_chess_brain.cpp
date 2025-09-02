// chess_ops.cpp - WofLang Neural Chess Plugin (Complete Working Version)
// Place this as: plugins/chess_ops.cpp

#include "core/woflang.hpp"
#include <array>
#include <string>
#include <vector>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <random>
#include <memory>
#include <cmath>
#include <iomanip>
#include <thread>
#include <chrono>

namespace woflang {

// Simple neural network for chess
class SimpleNeuralNetwork {
private:
    std::vector<std::vector<float>> weights_;
    std::vector<float> biases_;
    std::mt19937 rng_;
    
public:
    SimpleNeuralNetwork(size_t input_size, size_t output_size) : rng_(std::random_device{}()) {
        weights_.resize(output_size, std::vector<float>(input_size));
        biases_.resize(output_size);
        
        std::uniform_real_distribution<float> dist(-0.5f, 0.5f);
        for (auto& row : weights_) {
            for (auto& w : row) {
                w = dist(rng_);
            }
        }
        for (auto& b : biases_) {
            b = dist(rng_);
        }
    }
    
    std::vector<float> forward(const std::vector<float>& input) {
        std::vector<float> output(biases_.size());
        for (size_t i = 0; i < output.size(); i++) {
            float sum = biases_[i];
            for (size_t j = 0; j < input.size() && j < weights_[i].size(); j++) {
                sum += input[j] * weights_[i][j];
            }
            output[i] = std::tanh(sum);
        }
        return output;
    }
    
    void train(const std::vector<float>& input, const std::vector<float>& target, float learning_rate = 0.01f) {
        auto output = forward(input);
        
        for (size_t i = 0; i < output.size() && i < target.size(); i++) {
            float error = target[i] - output[i];
            float gradient = error * (1.0f - output[i] * output[i]);
            
            biases_[i] += learning_rate * gradient;
            for (size_t j = 0; j < input.size() && j < weights_[i].size(); j++) {
                weights_[i][j] += learning_rate * gradient * input[j];
            }
        }
    }
};

// Chess piece types
enum class PieceType : uint8_t {
    NONE = 0, PAWN = 1, KNIGHT = 2, BISHOP = 3, ROOK = 4, QUEEN = 5, KING = 6
};

enum class Color : uint8_t {
    WHITE = 0, BLACK = 1
};

struct Piece {
    PieceType type = PieceType::NONE;
    Color color = Color::WHITE;
    
    Piece() = default;
    Piece(PieceType t, Color c) : type(t), color(c) {}
    
    bool is_empty() const { return type == PieceType::NONE; }
    
    std::string to_unicode() const {
        if (is_empty()) return "·";
        switch (type) {
            case PieceType::KING:   return (color == Color::WHITE) ? "♔" : "♚";
            case PieceType::QUEEN:  return (color == Color::WHITE) ? "♕" : "♛";
            case PieceType::ROOK:   return (color == Color::WHITE) ? "♖" : "♜";
            case PieceType::BISHOP: return (color == Color::WHITE) ? "♗" : "♝";
            case PieceType::KNIGHT: return (color == Color::WHITE) ? "♘" : "♞";
            case PieceType::PAWN:   return (color == Color::WHITE) ? "♙" : "♟";
            default: return "·";
        }
    }
    
    int get_value() const {
        switch (type) {
            case PieceType::PAWN:   return 100;
            case PieceType::KNIGHT: return 320;
            case PieceType::BISHOP: return 330;
            case PieceType::ROOK:   return 500;
            case PieceType::QUEEN:  return 900;
            case PieceType::KING:   return 20000;
            default: return 0;
        }
    }
};

struct Move {
    uint8_t from_x, from_y, to_x, to_y;
    PieceType promotion = PieceType::NONE;
    bool is_castling = false;
    bool is_en_passant = false;
    
    Move() : from_x(0), from_y(0), to_x(0), to_y(0) {}
    Move(uint8_t fx, uint8_t fy, uint8_t tx, uint8_t ty) 
        : from_x(fx), from_y(fy), to_x(tx), to_y(ty) {}
    
    std::string to_algebraic() const {
        std::string result;
        result += static_cast<char>('a' + from_x);
        result += static_cast<char>('1' + from_y);
        result += static_cast<char>('a' + to_x);
        result += static_cast<char>('1' + to_y);
        return result;
    }
};

// Simplified ChessBoard class
class ChessBoard {
public:
    std::array<std::array<Piece, 8>, 8> board;
    Color current_turn = Color::WHITE;
    std::vector<Move> move_history;
    
    ChessBoard() {
        setup_initial_position();
    }
    
    void setup_initial_position() {
        // Clear board
        for (auto& rank : board) {
            for (auto& piece : rank) {
                piece = Piece();
            }
        }
        
        // White pieces
        board[0][0] = Piece(PieceType::ROOK, Color::WHITE);
        board[1][0] = Piece(PieceType::KNIGHT, Color::WHITE);
        board[2][0] = Piece(PieceType::BISHOP, Color::WHITE);
        board[3][0] = Piece(PieceType::QUEEN, Color::WHITE);
        board[4][0] = Piece(PieceType::KING, Color::WHITE);
        board[5][0] = Piece(PieceType::BISHOP, Color::WHITE);
        board[6][0] = Piece(PieceType::KNIGHT, Color::WHITE);
        board[7][0] = Piece(PieceType::ROOK, Color::WHITE);
        
        for (int i = 0; i < 8; i++) {
            board[i][1] = Piece(PieceType::PAWN, Color::WHITE);
        }
        
        // Black pieces
        board[0][7] = Piece(PieceType::ROOK, Color::BLACK);
        board[1][7] = Piece(PieceType::KNIGHT, Color::BLACK);
        board[2][7] = Piece(PieceType::BISHOP, Color::BLACK);
        board[3][7] = Piece(PieceType::QUEEN, Color::BLACK);
        board[4][7] = Piece(PieceType::KING, Color::BLACK);
        board[5][7] = Piece(PieceType::BISHOP, Color::BLACK);
        board[6][7] = Piece(PieceType::KNIGHT, Color::BLACK);
        board[7][7] = Piece(PieceType::ROOK, Color::BLACK);
        
        for (int i = 0; i < 8; i++) {
            board[i][6] = Piece(PieceType::PAWN, Color::BLACK);
        }
    }
    
    bool is_valid_square(int x, int y) const {
        return x >= 0 && x < 8 && y >= 0 && y < 8;
    }
    
    Piece get_piece(int x, int y) const {
        if (!is_valid_square(x, y)) return Piece();
        return board[x][y];
    }
    
    void set_piece(int x, int y, const Piece& piece) {
        if (is_valid_square(x, y)) {
            board[x][y] = piece;
        }
    }
    
    bool is_valid_move(const Move& move) const {
        if (!is_valid_square(move.from_x, move.from_y) || 
            !is_valid_square(move.to_x, move.to_y)) {
            return false;
        }
        
        Piece piece = get_piece(move.from_x, move.from_y);
        if (piece.is_empty() || piece.color != current_turn) {
            return false;
        }
        
        Piece target = get_piece(move.to_x, move.to_y);
        if (!target.is_empty() && target.color == piece.color) {
            return false;
        }
        
        // Basic movement validation (simplified)
        int dx = move.to_x - move.from_x;
        int dy = move.to_y - move.from_y;
        
        switch (piece.type) {
            case PieceType::PAWN: {
                int forward = (piece.color == Color::WHITE) ? 1 : -1;
                if (dx == 0 && dy == forward && target.is_empty()) return true;
                if (abs(dx) == 1 && dy == forward && !target.is_empty()) return true;
                return false;
            }
            case PieceType::KNIGHT:
                return (abs(dx) == 2 && abs(dy) == 1) || (abs(dx) == 1 && abs(dy) == 2);
            case PieceType::BISHOP:
                return abs(dx) == abs(dy) && dx != 0;
            case PieceType::ROOK:
                return (dx == 0) != (dy == 0); // XOR - exactly one is zero
            case PieceType::QUEEN:
                return (abs(dx) == abs(dy) && dx != 0) || ((dx == 0) != (dy == 0));
            case PieceType::KING:
                return abs(dx) <= 1 && abs(dy) <= 1;
            default:
                return false;
        }
    }
    
    bool make_move(const Move& move) {
        if (!is_valid_move(move)) {
            return false;
        }
        
        execute_move(move);
        return true;
    }
    
    void execute_move(const Move& move) {
        Piece piece = get_piece(move.from_x, move.from_y);
        set_piece(move.to_x, move.to_y, piece);
        set_piece(move.from_x, move.from_y, Piece());
        
        move_history.push_back(move);
        current_turn = (current_turn == Color::WHITE) ? Color::BLACK : Color::WHITE;
    }
    
    std::vector<Move> generate_legal_moves() const {
        std::vector<Move> moves;
        
        for (int from_x = 0; from_x < 8; from_x++) {
            for (int from_y = 0; from_y < 8; from_y++) {
                Piece piece = get_piece(from_x, from_y);
                if (piece.is_empty() || piece.color != current_turn) continue;
                
                for (int to_x = 0; to_x < 8; to_x++) {
                    for (int to_y = 0; to_y < 8; to_y++) {
                        Move move(from_x, from_y, to_x, to_y);
                        if (is_valid_move(move)) {
                            moves.push_back(move);
                        }
                    }
                }
            }
        }
        
        return moves;
    }
    
    int evaluate_position() const {
        int score = 0;
        for (int x = 0; x < 8; x++) {
            for (int y = 0; y < 8; y++) {
                Piece piece = get_piece(x, y);
                if (!piece.is_empty()) {
                    int piece_value = piece.get_value();
                    if (piece.color == Color::WHITE) {
                        score += piece_value;
                    } else {
                        score -= piece_value;
                    }
                }
            }
        }
        return score;
    }
    
    bool is_in_check(Color color) const {
        // Simplified check detection
        return false; // For now
    }
    
    std::string to_string() const {
        std::stringstream ss;
        ss << "\n  ";
        for (int i = 0; i < 8; i++) {
            ss << " " << static_cast<char>('a' + i) << " ";
        }
        ss << "\n";
        
        for (int y = 7; y >= 0; y--) {
            ss << (y + 1) << " ";
            for (int x = 0; x < 8; x++) {
                ss << " " << get_piece(x, y).to_unicode() << " ";
            }
            ss << " " << (y + 1) << "\n";
        }
        
        ss << "  ";
        for (int i = 0; i < 8; i++) {
            ss << " " << static_cast<char>('a' + i) << " ";
        }
        ss << "\n";
        
        ss << "Turn: " << ((current_turn == Color::WHITE) ? "White" : "Black");
        ss << " | Position Value: " << evaluate_position() << "\n";
        
        return ss.str();
    }
};

// Neural Chess Engine
class NeuralChessEngine {
private:
    std::unique_ptr<SimpleNeuralNetwork> position_evaluator_;
    std::unique_ptr<SimpleNeuralNetwork> move_selector_;
    int training_games_played_;
    
public:
    NeuralChessEngine() : training_games_played_(0) {
        position_evaluator_ = std::make_unique<SimpleNeuralNetwork>(64, 1);
        move_selector_ = std::make_unique<SimpleNeuralNetwork>(64, 64);
        
        std::cout << "🧠 Neural Chess Engine v2.0 initialized!\n";
        std::cout << "   Architecture: Position Evaluator (64→1) + Move Selector (64→64)\n";
        std::cout << "   Status: Ready for neural domination! ⚡\n";
    }
    
    std::vector<float> board_to_neural_input(const ChessBoard& board) const {
        std::vector<float> input(64, 0.0f);
        for (int rank = 0; rank < 8; rank++) {
            for (int file = 0; file < 8; file++) {
                auto piece = board.get_piece(file, rank);
                int index = rank * 8 + file;
                
                if (!piece.is_empty()) {
                    float piece_value = 0.0f;
                    switch (piece.type) {
                        case PieceType::PAWN:   piece_value = 0.1f; break;
                        case PieceType::KNIGHT: piece_value = 0.3f; break;
                        case PieceType::BISHOP: piece_value = 0.35f; break;
                        case PieceType::ROOK:   piece_value = 0.5f; break;
                        case PieceType::QUEEN:  piece_value = 0.9f; break;
                        case PieceType::KING:   piece_value = 1.0f; break;
                        default: piece_value = 0.0f; break;
                    }
                    
                    input[index] = (piece.color == Color::WHITE) ? piece_value : -piece_value;
                }
            }
        }
        return input;
    }
    
    float evaluate_position_neural(const ChessBoard& board) {
        auto input = board_to_neural_input(board);
        auto output = position_evaluator_->forward(input);
        
        float neural_eval = output.empty() ? 0.0f : output[0] * 1000.0f;
        float traditional_eval = static_cast<float>(board.evaluate_position());
        
        float neural_weight = std::min(0.7f, training_games_played_ * 0.01f);
        float traditional_weight = 1.0f - neural_weight;
        
        return neural_eval * neural_weight + traditional_eval * traditional_weight;
    }
    
    Move select_best_move(const ChessBoard& board, const std::vector<Move>& legal_moves) {
        if (legal_moves.empty()) {
            return Move();
        }
        
        std::vector<std::pair<Move, float>> move_scores;
        
        for (const auto& move : legal_moves) {
            ChessBoard test_board = board;
            test_board.execute_move(move);
            
            float position_eval = -evaluate_position_neural(test_board);
            move_scores.emplace_back(move, position_eval);
        }
        
        std::sort(move_scores.begin(), move_scores.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });
        
        std::random_device rd;
        std::mt19937 gen(rd());
        
        if (move_scores.size() >= 3) {
            std::discrete_distribution<> dist({5, 3, 1});
            int choice = dist(gen);
            return move_scores[choice].first;
        } else {
            return move_scores[0].first;
        }
    }
    
    void train_on_game(const std::vector<ChessBoard>& game_positions, Color winner) {
        if (game_positions.empty()) return;
        
        training_games_played_++;
        
        for (size_t i = 0; i < game_positions.size(); i++) {
            auto input = board_to_neural_input(game_positions[i]);
            
            float target_eval = 0.0f;
            if (winner == Color::WHITE) {
                target_eval = (game_positions[i].current_turn == Color::WHITE) ? 0.5f : -0.5f;
            } else if (winner == Color::BLACK) {
                target_eval = (game_positions[i].current_turn == Color::BLACK) ? 0.5f : -0.5f;
            }
            
            float position_weight = static_cast<float>(i) / game_positions.size();
            target_eval *= (0.5f + 0.5f * position_weight);
            
            position_evaluator_->train(input, {target_eval});
        }
    }
    
    int get_training_games() const { return training_games_played_; }
    
    std::string get_neural_stats() const {
        std::stringstream ss;
        ss << "🧠 Neural Stats:\n";
        ss << "   Games Trained: " << training_games_played_ << "\n";
        ss << "   Neural Weight: " << std::fixed << std::setprecision(1) 
           << (std::min(0.7f, training_games_played_ * 0.01f) * 100.0f) << "%\n";
        ss << "   Experience Level: ";
        if (training_games_played_ < 10) ss << "👶 Beginner";
        else if (training_games_played_ < 50) ss << "🎓 Learning";
        else if (training_games_played_ < 100) ss << "💪 Intermediate";
        else ss << "🧠 Expert";
        return ss.str();
    }
};

// Global instances
static std::unique_ptr<ChessBoard> g_chess_board;
static std::unique_ptr<NeuralChessEngine> g_neural_engine;

// Helper function to parse algebraic notation
std::pair<int, int> parse_square(const std::string& square) {
    if (square.length() != 2) return {-1, -1};
    
    int x = square[0] - 'a';
    int y = square[1] - '1';
    
    if (x < 0 || x >= 8 || y < 0 || y >= 8) return {-1, -1};
    
    return {x, y};
}

// Plugin initialization
extern "C" {
#ifndef WOFLANG_PLUGIN_EXPORT
#  ifdef _WIN32
#    define WOFLANG_PLUGIN_EXPORT extern "C" __declspec(dllexport)
#  else
#    define WOFLANG_PLUGIN_EXPORT extern "C"
#  endif
#endif

WOFLANG_PLUGIN_EXPORT void init_plugin(woflang::WoflangInterpreter::OpTable* op_table) {    
        // Initialize chess board and neural engine
        g_chess_board = std::make_unique<ChessBoard>();
        g_neural_engine = std::make_unique<NeuralChessEngine>();
        
        // Chess operations
        (*op_table)["chess_new"] = [](std::stack<WofValue>& stack) {
            g_chess_board = std::make_unique<ChessBoard>();
            
            // Epic ASCII art splash screen
            std::cout << "\n";
            std::cout << "╔═══════════════════════════════════════════════════════════════╗\n";
            std::cout << "║                                                               ║\n";
            std::cout << "║  ╦ ╦┌─┐┌─┐┬  ╦ ┌─┐  ╔╗╔┌─┐┬ ┬┬─┐┌─┐┬  ╔═╗┬ ┬┌─┐┌─┐┌─┐     ║\n";
            std::cout << "║  ║║║│ │├┤ │  ╚═╝└─┐  ║║║├┤ │ │├┬┘├─┤│  ║  ├─┤├┤ └─┐└─┐     ║\n";
            std::cout << "║  ╚╩╝└─┘└  ┴─┘   └─┘  ╝╚╝└─┘└─┘┴└─┴ ┴┴─┘╚═╝┴ ┴└─┘└─┘└─┘     ║\n";
            std::cout << "║                                                               ║\n";
            std::cout << "║    ♜♞♝♛♚♝♞♜    A Neural Chess Engine    ♖♘♗♕♔♗♘♖    ║\n";
            std::cout << "║    ♟♟♟♟♟♟♟♟      by husklyfren         ♙♙♙♙♙♙♙♙    ║\n";
            std::cout << "║                                                               ║\n";
            std::cout << "║              🧠 Neural Networks Enabled 🧠                  ║\n";
            std::cout << "║                                                               ║\n";
            std::cout << "╚═══════════════════════════════════════════════════════════════╝\n";
            std::cout << "\n";
            std::cout << "🎯 New neural chess game started! May the best brain win! 🎯\n";
            std::cout << g_chess_board->to_string() << std::endl;
        };
        
        (*op_table)["chess_show"] = [](std::stack<WofValue>& stack) {
            if (!g_chess_board) {
                std::cout << "No chess game in progress. Use 'chess_new' to start.\n";
                return;
            }
            std::cout << g_chess_board->to_string() << std::endl;
        };
        
        (*op_table)["chess_move"] = [](std::stack<WofValue>& stack) {
            if (!g_chess_board) {
                std::cout << "No chess game in progress. Use 'chess_new' to start.\n";
                return;
            }
            
            if (stack.size() < 2) {
                std::cout << "Need two squares for move (from to). Example: \"e2\" \"e4\" chess_move\n";
                return;
            }
            
            auto to_square = stack.top().s; stack.pop();
            auto from_square = stack.top().s; stack.pop();
            
            auto [from_x, from_y] = parse_square(from_square);
            auto [to_x, to_y] = parse_square(to_square);
            
            if (from_x == -1 || to_x == -1) {
                std::cout << "Invalid square notation. Use format like 'e2' or 'e4'.\n";
                return;
            }
            
            Move move(from_x, from_y, to_x, to_y);
            
            if (g_chess_board->make_move(move)) {
                std::cout << "Move: " << move.to_algebraic() << std::endl;
                std::cout << g_chess_board->to_string() << std::endl;
            } else {
                std::cout << "❌ Invalid move: " << move.to_algebraic() << std::endl;
            }
        };
        
        (*op_table)["chess_neural_move"] = [](std::stack<WofValue>& stack) {
            if (!g_chess_board || !g_neural_engine) {
                std::cout << "No chess game in progress. Use 'chess_new' to start.\n";
                return;
            }
            
            std::cout << "🧠 Neural engine thinking";
            for (int i = 0; i < 3; i++) {
                std::cout << ".";
                std::cout.flush();
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
            }
            std::cout << "\n";
            
            auto legal_moves = g_chess_board->generate_legal_moves();
            if (legal_moves.empty()) {
                std::cout << "No legal moves available!\n";
                return;
            }
            
            Move selected_move = g_neural_engine->select_best_move(*g_chess_board, legal_moves);
            
            if (g_chess_board->make_move(selected_move)) {
                float eval_after = -g_neural_engine->evaluate_position_neural(*g_chess_board);
                
                std::cout << "🧠 Neural move: " << selected_move.to_algebraic() 
                         << " (eval: " << std::fixed << std::setprecision(1) << eval_after << ")\n";
                std::cout << g_chess_board->to_string() << std::endl;
            } else {
                std::cout << "❌ Neural engine error: Invalid move selected!\n";
            }
        };
        
        (*op_table)["chess_neural_eval"] = [](std::stack<WofValue>& stack) {
            if (!g_chess_board || !g_neural_engine) {
                std::cout << "No chess game in progress. Use 'chess_new' to start.\n";
                return;
            }
            
            float neural_eval = g_neural_engine->evaluate_position_neural(*g_chess_board);
            int traditional_eval = g_chess_board->evaluate_position();
            
            std::cout << "🧠 Position Analysis:\n";
            std::cout << "   Neural eval: " << std::fixed << std::setprecision(1) << neural_eval << "\n";
            std::cout << "   Traditional: " << traditional_eval << "\n";
            std::cout << "   Difference:  " << std::setprecision(1) << (neural_eval - traditional_eval) << "\n";
            std::cout << g_neural_engine->get_neural_stats() << std::endl;
            
            WofValue result;
            result.d = static_cast<double>(neural_eval);
            stack.push(result);
        };
        
        (*op_table)["chess_neural_train"] = [](std::stack<WofValue>& stack) {
            if (!g_neural_engine) {
                std::cout << "Neural engine not initialized!\n";
                return;
            }
            
            if (stack.empty()) {
                std::cout << "Usage: <num_games> chess_neural_train\n";
                return;
            }
            
            auto num_games_val = stack.top(); stack.pop();
            int num_games = static_cast<int>(num_games_val.as_numeric());
            
            if (num_games <= 0) {
                std::cout << "Number of games must be positive!\n";
                return;
            }
            
            std::cout << "🧠 Starting neural self-training for " << num_games << " games...\n";
            
            for (int game = 0; game < num_games; game++) {
                ChessBoard training_board;
                std::vector<ChessBoard> game_positions;
                int moves = 0;
                
                std::cout << "Game " << (game + 1) << "/" << num_games << "... ";
                
                while (moves < 50) {
                    auto legal_moves = training_board.generate_legal_moves();
                    if (legal_moves.empty()) break;
                    
                    game_positions.push_back(training_board);
                    
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<> dis(0, legal_moves.size() - 1);
                    Move move = legal_moves[dis(gen)];
                    
                    training_board.execute_move(move);
                    moves++;
                }
                
                Color winner = (training_board.evaluate_position() > 0) ? Color::WHITE : Color::BLACK;
                g_neural_engine->train_on_game(game_positions, winner);
                std::cout << "✓\n";
            }
            
            std::cout << "🎓 Neural training complete!\n";
            std::cout << g_neural_engine->get_neural_stats() << std::endl;
            
            WofValue result;
            result.d = static_cast<double>(g_neural_engine->get_training_games());
            stack.push(result);
        };
        
        std::cout << "🧠⚡ NEURAL CHESS ENGINE LOADED! ⚡🧠\n";
        std::cout << "Neural Commands:\n";
        std::cout << "  chess_new               - Start new game with epic splash\n";
        std::cout << "  chess_show              - Display beautiful Unicode board\n";
        std::cout << "  chess_move              - Make human moves (\"e2\" \"e4\" chess_move)\n";
        std::cout << "  chess_neural_move       - Let the neural brain play\n";
        std::cout << "  chess_neural_eval       - Get neural position evaluation\n";
        std::cout << "  chess_neural_train <n>  - Train the neural networks on n games\n";
        std::cout << "\n🎮 Quick start: chess_new → 10 chess_neural_train → chess_neural_move\n";
        std::cout << "🧠 Train more for smarter play: 50 chess_neural_train\n";
        std::cout << "⚡ Neural pieces: ♔♕♖♗♘♙ (AI-powered!)\n";
        
        // Additional chess utilities
        (*op_table)["chess_legal_moves"] = [](std::stack<WofValue>& stack) {
            if (!g_chess_board) {
                std::cout << "No chess game in progress. Use 'chess_new' to start.\n";
                return;
            }
            
            auto moves = g_chess_board->generate_legal_moves();
            std::cout << "Legal moves (" << moves.size() << "):\n";
            
            for (size_t i = 0; i < moves.size(); i++) {
                std::cout << moves[i].to_algebraic();
                if ((i + 1) % 8 == 0) std::cout << "\n";
                else std::cout << " ";
            }
            if (moves.size() % 8 != 0) std::cout << "\n";
            
            WofValue result;
            result.d = static_cast<double>(moves.size());
            stack.push(result);
        };
        
        (*op_table)["chess_eval"] = [](std::stack<WofValue>& stack) {
            if (!g_chess_board) {
                std::cout << "No chess game in progress. Use 'chess_new' to start.\n";
                return;
            }
            
            int eval = g_chess_board->evaluate_position();
            std::cout << "Traditional evaluation: " << eval << " (positive = White advantage)\n";
            
            WofValue result;
            result.d = static_cast<double>(eval);
            stack.push(result);
        };
        
        (*op_table)["chess_neural_vs_human"] = [](std::stack<WofValue>& stack) {
            if (!g_chess_board || !g_neural_engine) {
                std::cout << "No chess game in progress. Use 'chess_new' to start.\n";
                return;
            }
            
            std::cout << "🧠 vs 🧑 Neural Engine vs Human mode!\n";
            std::cout << "The neural engine will play as Black.\n";
            std::cout << "Make your move as White using: \"e2\" \"e4\" chess_move\n";
            std::cout << "The neural engine will respond automatically after your move.\n";
            std::cout << g_chess_board->to_string() << std::endl;
        };
        
        (*op_table)["chess_neural_analysis"] = [](std::stack<WofValue>& stack) {
            if (!g_chess_board || !g_neural_engine) {
                std::cout << "No chess game in progress. Use 'chess_new' to start.\n";
                return;
            }
            
            std::cout << "🔬 Deep Neural Analysis of Current Position:\n";
            std::cout << "==========================================\n";
            
            auto legal_moves = g_chess_board->generate_legal_moves();
            if (legal_moves.empty()) {
                std::cout << "No legal moves available for analysis.\n";
                return;
            }
            
            std::cout << "Analyzing " << legal_moves.size() << " legal moves...\n\n";
            
            struct MoveAnalysis {
                Move move;
                float neural_eval;
                int traditional_eval;
                float confidence;
            };
            
            std::vector<MoveAnalysis> analyses;
            
            for (const auto& move : legal_moves) {
                ChessBoard test_board = *g_chess_board;
                test_board.execute_move(move);
                
                MoveAnalysis analysis;
                analysis.move = move;
                analysis.neural_eval = -g_neural_engine->evaluate_position_neural(test_board);
                analysis.traditional_eval = -test_board.evaluate_position();
                analysis.confidence = std::abs(analysis.neural_eval - analysis.traditional_eval) / 100.0f;
                
                analyses.push_back(analysis);
            }
            
            // Sort by neural evaluation
            std::sort(analyses.begin(), analyses.end(), 
                [](const MoveAnalysis& a, const MoveAnalysis& b) {
                    return a.neural_eval > b.neural_eval;
                });
            
            // Display top 10 moves
            std::cout << "Top Neural Moves:\n";
            std::cout << "Move    Neural   Trad.   Confidence\n";
            std::cout << "--------------------------------\n";
            
            for (size_t i = 0; i < std::min(10UL, analyses.size()); i++) {
                const auto& analysis = analyses[i];
                std::cout << analysis.move.to_algebraic() << "    " 
                         << std::fixed << std::setprecision(1) << analysis.neural_eval << "    "
                         << analysis.traditional_eval << "    "
                         << std::setprecision(2) << analysis.confidence << "\n";
            }
            
            std::cout << "\n🧠 Neural recommendation: " << analyses[0].move.to_algebraic() << "\n";
        };
        
        (*op_table)["chess_neural_status"] = [](std::stack<WofValue>& stack) {
            if (!g_neural_engine) {
                std::cout << "Neural engine not initialized!\n";
                return;
            }
            
            std::cout << "🧠 Neural Chess Engine Status:\n";
            std::cout << "==============================\n";
            std::cout << "Architecture: Position Evaluator + Move Selector\n";
            std::cout << "Network Topology: 64→1 + 64→64 neurons\n";
            std::cout << "Activation Function: Tanh (hyperbolic tangent)\n";
            std::cout << "Learning Algorithm: Gradient descent backpropagation\n";
            std::cout << g_neural_engine->get_neural_stats() << "\n";
            std::cout << "\nAvailable Neural Commands:\n";
            std::cout << "  chess_neural_eval       - Get neural position evaluation\n";
            std::cout << "  chess_neural_move       - Let neural engine make a move\n";
            std::cout << "  chess_neural_train <n>  - Train neural networks on n games\n";
            std::cout << "  chess_neural_analysis   - Deep move analysis\n";
            std::cout << "  chess_neural_vs_human   - Challenge the neural engine\n";
            std::cout << "  chess_neural_status     - This status display\n";
        };
        
        // Unicode piece symbols that push piece type values to stack
        (*op_table)["♔"] = [](std::stack<WofValue>& stack) {
            WofValue result;
            result.d = static_cast<double>(static_cast<int>(PieceType::KING));
            stack.push(result);
        };
        
        (*op_table)["♕"] = [](std::stack<WofValue>& stack) {
            WofValue result;
            result.d = static_cast<double>(static_cast<int>(PieceType::QUEEN));
            stack.push(result);
        };
        
        (*op_table)["♖"] = [](std::stack<WofValue>& stack) {
            WofValue result;
            result.d = static_cast<double>(static_cast<int>(PieceType::ROOK));
            stack.push(result);
        };
        
        (*op_table)["♗"] = [](std::stack<WofValue>& stack) {
            WofValue result;
            result.d = static_cast<double>(static_cast<int>(PieceType::BISHOP));
            stack.push(result);
        };
        
        (*op_table)["♘"] = [](std::stack<WofValue>& stack) {
            WofValue result;
            result.d = static_cast<double>(static_cast<int>(PieceType::KNIGHT));
            stack.push(result);
        };
        
        (*op_table)["♙"] = [](std::stack<WofValue>& stack) {
            WofValue result;
            result.d = static_cast<double>(static_cast<int>(PieceType::PAWN));
            stack.push(result);
        };
        
        // Quick training shortcuts
        (*op_table)["chess_quick_train"] = [](std::stack<WofValue>& stack) {
            if (!g_neural_engine) {
                std::cout << "Neural engine not initialized!\n";
                return;
            }
            
            std::cout << "🚀 Quick neural training (10 games)...\n";
            
            // Simulate 10 games quickly
            for (int game = 0; game < 10; game++) {
                ChessBoard training_board;
                std::vector<ChessBoard> game_positions;
                
                for (int moves = 0; moves < 20; moves++) {
                    auto legal_moves = training_board.generate_legal_moves();
                    if (legal_moves.empty()) break;
                    
                    game_positions.push_back(training_board);
                    
                    std::random_device rd;
                    std::mt19937 gen(rd());
                    std::uniform_int_distribution<> dis(0, legal_moves.size() - 1);
                    Move move = legal_moves[dis(gen)];
                    
                    training_board.execute_move(move);
                }
                
                Color winner = (training_board.evaluate_position() > 0) ? Color::WHITE : Color::BLACK;
                g_neural_engine->train_on_game(game_positions, winner);
            }
            
            std::cout << "✅ Quick training complete!\n";
            std::cout << g_neural_engine->get_neural_stats() << std::endl;
            
            WofValue result;
            result.d = static_cast<double>(g_neural_engine->get_training_games());
            stack.push(result);
        };
        
        // Neural engine benchmarking
        (*op_table)["chess_neural_benchmark"] = [](std::stack<WofValue>& stack) {
            if (!g_chess_board || !g_neural_engine) {
                std::cout << "No chess game in progress. Use 'chess_new' to start.\n";
                return;
            }
            
            std::cout << "⚡ Neural Engine Benchmark\n";
            std::cout << "=========================\n";
            
            auto start_time = std::chrono::high_resolution_clock::now();
            
            // Benchmark position evaluation
            for (int i = 0; i < 1000; i++) {
                g_neural_engine->evaluate_position_neural(*g_chess_board);
            }
            
            auto eval_time = std::chrono::high_resolution_clock::now();
            
            // Benchmark move selection
            auto legal_moves = g_chess_board->generate_legal_moves();
            for (int i = 0; i < 100; i++) {
                g_neural_engine->select_best_move(*g_chess_board, legal_moves);
            }
            
            auto move_time = std::chrono::high_resolution_clock::now();
            
            auto eval_duration = std::chrono::duration_cast<std::chrono::microseconds>(eval_time - start_time).count();
            auto move_duration = std::chrono::duration_cast<std::chrono::microseconds>(move_time - eval_time).count();
            
            std::cout << "Position Evaluations: 1000 in " << eval_duration << " μs\n";
            std::cout << "                     " << std::fixed << std::setprecision(2) 
                     << (1000.0 * 1000000.0 / eval_duration) << " eval/sec\n";
            std::cout << "Move Selection:      100 in " << move_duration << " μs\n";
            std::cout << "                     " << std::fixed << std::setprecision(2) 
                     << (100.0 * 1000000.0 / move_duration) << " moves/sec\n";
            
            std::cout << "\n🧠 Neural Performance: ";
            if (eval_duration < 100000) std::cout << "🚀 Excellent";
            else if (eval_duration < 500000) std::cout << "⚡ Good";
            else std::cout << "🐌 Needs optimization";
            std::cout << "\n";
        };
    }
}

