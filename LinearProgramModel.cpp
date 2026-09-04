#include "LinearProgramModel.h"
#include <iostream>
#include <stdexcept>

// コンストラクタ
LinearProgramModel::LinearProgramModel() 
    : num_variables(0), num_constraints(0) {}

// 目的関数を設定する
void LinearProgramModel::setObjective(const std::vector<double>& coefficients) {
    if (coefficients.empty()) {
        throw std::invalid_argument("目的関数の係数ベクトルが空です。");
    }
    
    // すでに制約条件が追加されている場合、変数の数が一致するかチェック
    if (num_variables != 0 && coefficients.size() != static_cast<size_t>(num_variables)) {
        throw std::invalid_argument("目的関数の変数変数の数が、既存の制約条件の変数の数と一致しません。");
    }

    objective_coefficients = coefficients;
    num_variables = static_cast<int>(coefficients.size());
}

// 制約条件を1つ追加する
void LinearProgramModel::addConstraint(const std::vector<double>& coefficients, double rhs) {
    if (coefficients.empty()) {
        throw std::invalid_argument("制約条件の係数ベクトルが空です。");
    }

    // 初めての入力、または既存の変数の数と一致するかチェック
    if (num_variables == 0) {
        num_variables = static_cast<int>(coefficients.size());
    } else if (coefficients.size() != static_cast<size_t>(num_variables)) {
        throw std::invalid_argument("追加された制約条件の変数の数が、モデルの変数の数と一致しません。");
    }

    // 学習用（標準形）のため、右辺が負になる場合はエラーとする（2段階単体法が必要になるため）
    if (rhs < 0) {
        throw std::invalid_argument("学習用のため、制約条件の右辺(rhs)は0以上である必要があります。");
    }

    constraint_matrix.push_back(coefficients);
    constraint_rhs.push_back(rhs);
    num_constraints++;
}

// モデルの情報をコンソールに表示する（デバッグ用）
void LinearProgramModel::printModel() const {
    std::cout << "========================================\n";
    std::cout << "  Linear Programming Model (標準形)\n";
    std::cout << "========================================\n";
    
    // 目的関数の表示
    std::cout << "Maximize: Z = ";
    for (int i = 0; i < num_variables; ++i) {
        std::cout << objective_coefficients[i] << "*x" << (i + 1);
        if (i < num_variables - 1) std::cout << " + ";
    }
    std::cout << "\n\nSubject to:\n";

    // 制約条件の表示
    for (int i = 0; i < num_constraints; ++i) {
        std::cout << "  ";
        for (int j = 0; j < num_variables; ++j) {
            std::cout << constraint_matrix[i][j] << "*x" << (j + 1);
            if (j < num_variables - 1) std::cout << " + ";
        }
        std::cout << " <= " << constraint_rhs[i] << "\n";
    }

    // 非負制約の表示
    std::cout << "  ";
    for (int i = 0; i < num_variables; ++i) {
        std::cout << "x" << (i + 1);
        if (i < num_variables - 1) std::cout << ", ";
    }
    std::cout << " >= 0\n";
    std::cout << "========================================\n";
}