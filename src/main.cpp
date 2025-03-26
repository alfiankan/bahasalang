#include "lexer/Lexer.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include "lexer/Parser.hpp"
#include "ast/AST.hpp"
#include "ast/ASTPrinter.hpp"
#include "codegen/Codegen.hpp"
#include <unistd.h> // For mkstemp

std::string readFile(const std::string& path) {
    std::ifstream file(path);
    if (!file.is_open()) {
        throw std::runtime_error("Tidak dapat membuka berkas: " + path);
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

std::string getTokenTypeName(bahasa::TokenType type) {
    switch (type) {
        case bahasa::TokenType::FUNCTION: return "FUNCTION";
        case bahasa::TokenType::INT: return "INT";
        case bahasa::TokenType::ARROW: return "ARROW";
        case bahasa::TokenType::LPAREN: return "LPAREN";
        case bahasa::TokenType::RPAREN: return "RPAREN";
        case bahasa::TokenType::LBRACE: return "LBRACE";
        case bahasa::TokenType::RBRACE: return "RBRACE";
        case bahasa::TokenType::COMMA: return "COMMA";
        case bahasa::TokenType::COLON: return "COLON";
        case bahasa::TokenType::PLUS: return "PLUS";
        case bahasa::TokenType::MINUS: return "MINUS";
        case bahasa::TokenType::RETURN_ARROW: return "RETURN_ARROW";
        case bahasa::TokenType::EQUALS: return "EQUALS";
        case bahasa::TokenType::IDENTIFIER: return "IDENTIFIER";
        case bahasa::TokenType::NUMBER: return "NUMBER";
        case bahasa::TokenType::END: return "END";
        case bahasa::TokenType::STRING: return "STRING";
        case bahasa::TokenType::LESS_EQUAL: return "LESS_EQUAL";
        case bahasa::TokenType::MUTASI: return "MUTASI";
        case bahasa::TokenType::IF: return "IF";
        case bahasa::TokenType::MODUL: return "MODUL";
        case bahasa::TokenType::MODULO: return "MODULO";
        case bahasa::TokenType::ADALAH: return "ADALAH";
        default: return "UNKNOWN";
    }
}

void printHorizontalLine(int width) {
    std::cout << '+' << std::string(width - 2, '-') << '+' << std::endl;
}

void printUsage(const char* program) {
    std::cerr << "Penggunaan: " << program << " <perintah> [opsi] <berkas_sumber>\n\n"
              << "Perintah:\n"
              << "  ir       Kompilasi kode sumber ke LLVM IR\n"
              << "  susun    Kompilasi kode sumber ke program\n"
              << "  jalankan Kompilasi dan jalankan program\n"
              << "  ast      Tampilkan AST\n\n"
              << "Opsi:\n"
              << "  -o <berkas>   Berkas keluaran (default: a.out untuk susun/jalankan, <nama_modul>.ll untuk ir)\n";
}

int compileLLVMIR(const std::string& sourcePath, const std::string& outputPath = "") {
    try {
        std::string source = readFile(sourcePath);
        bahasa::Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        bahasa::Parser parser(tokens);
        auto ast = parser.parse();
        
        std::string moduleName = parser.getModuleName();
        bahasa::Codegen codegen(moduleName);
        codegen.generate(ast);

        // Determine output file name
        std::string outFile = outputPath.empty() ? moduleName + ".ll" : outputPath;
        
        // Open output file
        std::ofstream out(outFile);
        if (!out.is_open()) {
            throw std::runtime_error("Tidak dapat membuka berkas keluaran: " + outFile);
        }

        // Redirect LLVM IR to file
        std::string ir;
        llvm::raw_string_ostream irStream(ir);
        codegen.dump(irStream);
        out << ir;
        
        std::cout << "Berhasil dikompilasi ke " << outFile << std::endl;
        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Galat: " << e.what() << std::endl;
        return 1;
    }
}

// Helper function to create temporary file
std::string createTempFile(const std::string& suffix) {
    char tempPath[] = "bahasa_XXXXXX";
    int fd = mkstemp(tempPath);
    if (fd == -1) {
        throw std::runtime_error("Gagal membuat berkas sementara");
    }
    close(fd);
    
    // If there's a suffix, rename the file
    std::string finalPath = tempPath + suffix;
    if (suffix.length() > 0) {
        if (rename(tempPath, finalPath.c_str()) != 0) {
            std::remove(tempPath);
            throw std::runtime_error("Gagal mengubah nama berkas sementara");
        }
    }
    
    return finalPath;
}

int compileToExecutable(const std::string& sourcePath, const std::string& outputPath = "a.out") {
    try {
        // First compile to temporary IR file
        std::string tempIR = createTempFile(".ll");
        
        // Compile to LLVM IR first
        if (int result = compileLLVMIR(sourcePath, tempIR)) {
            return result;
        }
        
        // Then compile IR to executable with proper linking and suppress warnings
        std::string cmd = "clang -w " + tempIR + " -o " + outputPath;
        #ifdef __APPLE__
            cmd += " -L/usr/lib -lSystem";  // Add system library for macOS
        #else
            cmd += " -lc";  // Add C library for Linux
        #endif
        cmd += " 2>/dev/null";
        
        if (int result = system(cmd.c_str())) {
            std::remove(tempIR.c_str());
            throw std::runtime_error("Gagal mengkompilasi berkas IR ke program");
        }
        
        // Clean up temporary file
        std::remove(tempIR.c_str());
        
        std::cout << "Successfully compiled to " << outputPath << std::endl;
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

int runExecutable(const std::string& sourcePath) {
    try {
        // First compile to temporary executable
        std::string tempExe = createTempFile("");
        
        // Compile to executable first
        if (int result = compileToExecutable(sourcePath, tempExe)) {
            return result;
        }
        
        // Run the executable with proper path
        std::string cmd = "./" + tempExe;
        int result = system(cmd.c_str());
        
        // Clean up temporary executable
        std::remove(tempExe.c_str());
        
        return result;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

// Add this function to print the AST
int printAST(const std::string& sourcePath) {
    try {
        std::string source = readFile(sourcePath);
        bahasa::Lexer lexer(source);
        auto tokens = lexer.tokenize();
        
        bahasa::Parser parser(tokens);
        auto ast = parser.parse();
        
        // Print the AST
        std::cout << "Abstract Syntax Tree:\n";
        for (const auto& stmt : ast) {
            bahasa::ASTPrinter::printStmt(stmt);
        }
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        printUsage(argv[0]);
        return 1;
    }

    std::string command = argv[1];
    
    if (command == "ir") {
        std::string outputPath;
        std::string sourcePath;
        
        // Parse options
        for (int i = 2; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "-o") {
                if (i + 1 >= argc) {
                    std::cerr << "Galat: -o membutuhkan nama berkas keluaran\n";
                    return 1;
                }
                outputPath = argv[++i];
            } else {
                sourcePath = arg;
            }
        }
        
        if (sourcePath.empty()) {
            std::cerr << "Galat: Berkas sumber tidak ditemukan\n";
            printUsage(argv[0]);
            return 1;
        }
        
        return compileLLVMIR(sourcePath, outputPath);
    }
    else if (command == "susun") {
        std::string outputPath = "a.out";
        std::string sourcePath;
        
        // Parse options
        for (int i = 2; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "-o") {
                if (i + 1 >= argc) {
                    std::cerr << "Galat: -o membutuhkan nama berkas keluaran\n";
                    return 1;
                }
                outputPath = argv[++i];
            } else {
                sourcePath = arg;
            }
        }
        
        if (sourcePath.empty()) {
            std::cerr << "Galat: Berkas sumber tidak ditemukan\n";
            printUsage(argv[0]);
            return 1;
        }
        
        return compileToExecutable(sourcePath, outputPath);
    }
    else if (command == "jalankan") {
        std::string sourcePath;
        
        // Parse options (ignore -o for jalankan since we use temp file)
        for (int i = 2; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "-o") {
                std::cerr << "Peringatan: opsi -o diabaikan untuk perintah jalankan\n";
                i++; // Skip the next argument
            } else {
                sourcePath = arg;
            }
        }
        
        if (sourcePath.empty()) {
            std::cerr << "Galat: Berkas sumber tidak ditemukan\n";
            printUsage(argv[0]);
            return 1;
        }
        
        return runExecutable(sourcePath);
    }
    else if (command == "ast") {
        std::string sourcePath;
        
        // Parse options
        for (int i = 2; i < argc; i++) {
            std::string arg = argv[i];
            if (arg == "-o") {
                std::cerr << "Warning: -o option is ignored for ast command\n";
                i++; // Skip the next argument
            } else {
                sourcePath = arg;
            }
        }
        
        if (sourcePath.empty()) {
            std::cerr << "Galat: Berkas sumber tidak ditemukan\n";
            printUsage(argv[0]);
            return 1;
        }
        
        return printAST(sourcePath);
    } else {
        std::cerr << "Perintah tidak dikenal: " << command << std::endl;
        printUsage(argv[0]);
        return 1;
    }
} 