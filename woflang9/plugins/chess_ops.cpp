// chess_ops.cpp - Minimal Working Neural Chess Plugin
// Replace your entire plugins/chess_ops.cpp with this file

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

namespace woflang {

// Simple neural network
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
        
        return true; // Simplified validation
    }
    
    bool make_move(const Move& move) {
        if (!is_valid_move(move)) {
            return false;
        }
        
        Piece piece = get_piece(move.from_x, move.from_y);
        set_piece(move.to_x, move.to_y, piece);
        set_piece(move.from_x, move.from_y, Piece());
        
        current_turn = (current_turn == Color::WHITE) ? Color::BLACK : Color::WHITE;
        return true;
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
    int training_games_played_;
    
public:
    NeuralChessEngine() : training_games_played_(0) {
        position_evaluator_ = std::make_unique<SimpleNeuralNetwork>(64, 1);
        std::cout << "🧠 Neural Chess Engine initialized!\n";
    }
    
    std::vector<float> board_to_neural_input(const ChessBoard& board) const {
        std::vector<float> input(64, 0.0f);
        for (int rank = 0; rank < 8; rank++) {
            for (int file = 0; file < 8; file++) {
                auto piece = board.get_piece(file, rank);
                int index = rank * 8 + file;
                
                if (!piece.is_empty()) {
                    float piece_value = static_cast<float>(piece.get_value()) / 1000.0f;
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
        
        return neural_eval * 0.3f + traditional_eval * 0.7f;
    }
    
    Move select_best_move(const ChessBoard& board, const std::vector<Move>& legal_moves) {
        if (legal_moves.empty()) {
            return Move();
        }
        
        std::vector<std::pair<Move, float>> move_scores;
        
        for (const auto& move : legal_moves) {
            ChessBoard test_board = board;
            test_board.make_move(move);
            
            float position_eval = -evaluate_position_neural(test_board);
            move_scores.emplace_back(move, position_eval);
        }
        
        std::sort(move_scores.begin(), move_scores.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });
        
        return move_scores[0].first;
    }
    
    void train_quick() {
        for (int game = 0; game < 5; game++) {
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
                
                training_board.make_move(move);
            }
            
            // Simple training
            for (const auto& pos : game_positions) {
                auto input = board_to_neural_input(pos);
                float target = static_cast<float>(pos.evaluate_position()) / 1000.0f;
                position_evaluator_->train(input, {target});
            }
            
            training_games_played_++;
        }
        
        std::cout << "🧠 Quick training complete! Games trained: " << training_games_played_ << "\n";
    }
    
    int get_training_games() const { return training_games_played_; }
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
        
        (*op_table)["chess_new"] = [](std::stack<WofValue>& stack) {
            (void)stack;
            g_chess_board = std::make_unique<ChessBoard>();
            
            std::cout << "\n";
            std::cout << "╔═══════════════════════════════════════════════════════════════╗\n";
            std::cout << "║  ╦ ╦┌─┐┌─┐┬  ╦ ┌─┐  ╔╗╔┌─┐┬ ┬┬─┐┌─┐┬  ╔═╗┬ ┬┌─┐┌─┐┌─┐     ║\n";
            std::cout << "║  ║║║│ │├┤ │  ╚═╝└─┐  ║║║├┤ │ │├┬┘├─┤│  ║  ├─┤├┤ └─┐└─┐     ║\n";
            std::cout << "║  ╚╩╝└─┘└  ┴─┘   └─┘  ╝╚╝└─┘└─┘┴└─┴ ┴┴─┘╚═╝┴ ┴└─┘└─┘└─┘     ║\n";
            std::cout << "║              🧠 Neural Networks Enabled 🧠                  ║\n";
            std::cout << "╚═══════════════════════════════════════════════════════════════╝\n";
            std::cout << "\n🎯 New neural chess game started!\n";
            std::cout << g_chess_board->to_string() << std::endl;
        };
        
        (*op_table)["chess_show"] = [](std::stack<WofValue>& stack) {
            (void)stack;
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
                std::cout << "Usage: push two squares then call chess_move\n";
                std::cout << "Example: e2 e4 chess_move  (pushes squares as tokens)\n";
                return;
            }
            
            auto to_val = stack.top(); stack.pop();
            auto from_val = stack.top(); stack.pop();
            
            // Try to get string values - if they're not strings, convert from numbers
            std::string from_square, to_square;
            
            if (!to_val.s.empty()) {
                to_square = to_val.s;
            } else {
                // If it's a number, we'll need a different approach
                std::cout << "Error: Expected string values for squares\n";
                std::cout << "Try: e2 e4 chess_move (if e2/e4 are recognized as tokens)\n";
                return;
            }
            
            if (!from_val.s.empty()) {
                from_square = from_val.s;
            } else {
                std::cout << "Error: Expected string values for squares\n";
                return;
            }
            
            auto [from_x, from_y] = parse_square(from_square);
            auto [to_x, to_y] = parse_square(to_square);
            
            if (from_x == -1 || to_x == -1) {
                std::cout << "Invalid square notation. Use format like 'e2' or 'e4'.\n";
                std::cout << "Got: '" << from_square << "' -> '" << to_square << "'\n";
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
            (void)stack;
            if (!g_chess_board || !g_neural_engine) {
                std::cout << "No chess game in progress. Use 'chess_new' to start.\n";
                return;
            }
            
            std::cout << "🧠 Neural engine thinking...\n";
            
            auto legal_moves = g_chess_board->generate_legal_moves();
            if (legal_moves.empty()) {
                std::cout << "No legal moves available!\n";
                return;
            }
            
            Move selected_move = g_neural_engine->select_best_move(*g_chess_board, legal_moves);
            
            if (g_chess_board->make_move(selected_move)) {
                std::cout << "🧠 Neural move: " << selected_move.to_algebraic() << std::endl;
                std::cout << g_chess_board->to_string() << std::endl;
            } else {
                std::cout << "❌ Neural engine error!\n";
            }
        };
        
        (*op_table)["chess_neural_eval"] = [](std::stack<WofValue>& stack) {
            if (!g_chess_board || !g_neural_engine) {
                std::cout << "No chess game in progress. Use 'chess_new' to start.\n";
                return;
            }
            
            float neural_eval = g_neural_engine->evaluate_position_neural(*g_chess_board);
            int traditional_eval = g_chess_board->evaluate_position();
            
            std::cout << "🧠 Neural eval: " << std::fixed << std::setprecision(1) << neural_eval << "\n";
            std::cout << "📊 Traditional: " << traditional_eval << "\n";
            
            WofValue result;
            result.d = static_cast<double>(neural_eval);
            stack.push(result);
        };
        
        (*op_table)["chess_quick_train"] = [](std::stack<WofValue>& stack) {
            (void)stack;
            if (!g_neural_engine) {
                std::cout << "Neural engine not initialized!\n";
                return;
            }
            
            std::cout << "🚀 Quick neural training...\n";
            g_neural_engine->train_quick();
            
            WofValue result;
            result.d = static_cast<double>(g_neural_engine->get_training_games());
            stack.push(result);
        };
        
        // Alternative move commands for easier input
        (*op_table)["e2e4"] = [](std::stack<WofValue>& stack) {
            (void)stack;
            if (!g_chess_board) { std::cout << "Use chess_new first\n"; return; }
            Move move(4, 1, 4, 3); // e2 to e4
            if (g_chess_board->make_move(move)) {
                std::cout << "Move: e2e4\n" << g_chess_board->to_string();
            } else { std::cout << "❌ Invalid move: e2e4\n"; }
        };
        
        (*op_table)["d2d4"] = [](std::stack<WofValue>& stack) {
            (void)stack;
            if (!g_chess_board) { std::cout << "Use chess_new first\n"; return; }
            Move move(3, 1, 3, 3); // d2 to d4
            if (g_chess_board->make_move(move)) {
                std::cout << "Move: d2d4\n" << g_chess_board->to_string();
            } else { std::cout << "❌ Invalid move: d2d4\n"; }
        };
        
        (*op_table)["e7e5"] = [](std::stack<WofValue>& stack) {
            (void)stack;
            if (!g_chess_board) { std::cout << "Use chess_new first\n"; return; }
            Move move(4, 6, 4, 4); // e7 to e5
            if (g_chess_board->make_move(move)) {
                std::cout << "Move: e7e5\n" << g_chess_board->to_string();
            } else { std::cout << "❌ Invalid move: e7e5\n"; }
        };
        
        (*op_table)["chess_legal_moves"] = [](std::stack<WofValue>& stack) {
            (void)stack;
            if (!g_chess_board) {
                std::cout << "No chess game in progress.\n";
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
        };
        std::cout << "Commands:\n";
        std::cout << "  chess_new         - Start new game\n";
        std::cout << "  chess_show        - Display board\n";
        std::cout << "  chess_move        - Make human moves\n";
        std::cout << "  chess_neural_move - AI makes a move\n";
        std::cout << "  chess_neural_eval - Get neural evaluation\n";
        std::cout << "  chess_quick_train - Quick training\n";
        std::cout << "\n🎮 Try: chess_new → chess_quick_train → chess_neural_move\n";
    }
}
