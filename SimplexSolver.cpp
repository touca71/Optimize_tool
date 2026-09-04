#include "SimplexSolver.h"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <limits>

SimplexSolver::SimplexSolver() 
    : solver_status(Status::UNSOLVED), optimal_value(0.0) {}

// 1. モデルから初期シンプレックス・タブローを組み立てる
void SimplexSolver::initializeTableau(const LinearProgramModel& model) {
    num_original_vars = model.getNumVariables();
    num_constraints = model.getNumConstraints();
    num_slack_vars = num_constraints; // 各制約に1つのスラック変数
    num_total_vars = num_original_vars + num_slack_vars;

    // タブローのサイズ： 行 = 制約数 + 1(目的関数)// タブローっていうのが計算に使うテーブルのことらしい
    //                 列 = 総変数数 + 1(右辺定数)
    int rows = num_constraints + 1; //目的関数の行の分
    int cols = num_total_vars + 1; //制約の上限を示す値の分
    tableau.assign(rows, std::vector<double>(cols, 0.0));//0で初期化

    // 初期基底変数の設定（初期状態ではすべてのスラック変数が基底に入る）
    basis.resize(num_constraints);
    for (int i = 0; i < num_constraints; ++i) {
        // 元の変数の後ろのインデックスを基底とする
        basis[i] = num_original_vars + i;
    }

    // 制約条件のデータをタブローにコピー
    const auto& matrix = model.getConstraintMatrix();
    const auto& rhs = model.getConstraintRhs();

    for (int i = 0; i < num_constraints; ++i) {
        // 元の変数の係数をコピー
        for (int j = 0; j < num_original_vars; ++j) {
            tableau[i][j] = matrix[i][j];
        }
        // スラック変数の係数（単位行列になるように配置）
        tableau[i][num_original_vars + i] = 1.0;
        // 右辺(b)を最後の列に配置
        tableau[i][cols - 1] = rhs[i];
    }

    // 目的関数のデータをタブローの最下行に配置
    // 単体法では、目的関数 Z = c1*x1 + c2*x2  =>  Z - c1*x1 - c2*x2 = 0 と変形するため
    // 係数の符号を反転して格納します
    const auto& obj = model.getObjectiveCoefficients();
    for (int j = 0; j < num_original_vars; ++j) {
        tableau[num_constraints][j] = -obj[j];
    }
    // 目的関数の右辺は最初は 0
    tableau[num_constraints][cols - 1] = 0.0;
}

// デバッグ用：現在のタブローの状態を綺麗にコンソール表示する
void SimplexSolver::printTableau() const {
    int rows = num_constraints + 1;
    int cols = num_total_vars + 1;

    std::cout << "\n--- Current Simplex Tableau ---\n";
    
    // ヘッダー（変数名）の表示
    std::cout << std::setw(8) << "Basis" << " | ";
    for (int j = 0; j < num_total_vars; ++j) {
        if (j < num_original_vars) {
            std::cout << std::setw(8) << ("x" + std::to_string(j + 1));
        } else {
            std::cout << std::setw(8) << ("s" + std::to_string(j - num_original_vars + 1));
        }
    }
    std::cout << " | " << std::setw(8) << "RHS" << "\n";
    std::cout << std::string(13 + 8 * (cols), '-') << "\n";

    // 各行のデータを表示
    for (int i = 0; i < rows; ++i) {
        if (i < num_constraints) {
            int b_var = basis[i];
            if (b_var < num_original_vars) {
                std::cout << std::setw(8) << ("x" + std::to_string(b_var + 1));
            } else {
                std::cout << std::setw(8) << ("s" + std::to_string(b_var - num_original_vars + 1));
            }
        } else {
            std::cout << std::setw(8) << "Z";
        }
        std::cout << " | ";

        for (int j = 0; j < cols; ++j) {
            if (j == cols - 1) std::cout << " | ";
            // 浮動小数点の丸め誤差で見栄えが悪くなるのを防ぐため、極小の値は0として表示
            double val = tableau[i][j];
            if (std::abs(val) < 1e-9) val = 0.0;
            std::cout << std::setw(8) << std::fixed << std::setprecision(2) << val;
        }
        std::cout << "\n";
    }
    std::cout << "-------------------------------\n";
}

// 2. ピボット（列・行）を選択する
std::pair<int, int> SimplexSolver::selectPivot() const {
    int cols = num_total_vars + 1;
    int pivot_col = -1;
    double min_marginal_cost = 0.0; // 0未満の最も小さい値を探す

    // 【ステップA: ピボット列の選択】 
    // 最下行（目的関数行）のうち、係数が最も負に大きい列（改善余地が最大の列）を選ぶ
    for (int j = 0; j < num_total_vars; ++j) {
        if (tableau[num_constraints][j] < min_marginal_cost) {
            min_marginal_cost = tableau[num_constraints][j];
            pivot_col = j;
        }
    }

    // すべての係数が0以上なら、これ以上目的関数を大きくできないので「最適解」に達した
    if (pivot_col == -1) {
        return {-1, -1};
    }

    // 【ステップB: ピボット行の選択】
    // 最小比テスト（Minimum Ratio Test）を行う
    int pivot_row = -1;
    double min_ratio = std::numeric_limits<double>::max();
    int cols_idx = cols - 1; // 右辺(RHS)の列インデックス

    for (int i = 0; i < num_constraints; ++i) {
        // ピボット列の要素が正の場合のみ比率を計算（0や負ならその変数を増やしても制約に引っかからない）
        if (tableau[i][pivot_col] > 1e-9) {
            double ratio = tableau[i][cols_idx] / tableau[i][pivot_col];
            if (ratio < min_ratio) {
                min_ratio = ratio;
                pivot_row = i;
            }
        }
    }

    // もし正の要素が1つもない場合、変数を無限に大きくできることを意味するため「有界でない（無限大）」
    if (pivot_row == -1) {
        return {-1, pivot_col}; // 行に -1 を入れて返す
    }

    return {pivot_row, pivot_col};
}

// 3. 指定されたピボットを中心に、行基本変形（掃き出し法）を行う
void SimplexSolver::executePivot(int pivot_row, int pivot_col) {
    int rows = num_constraints + 1;
    int cols = num_total_vars + 1;
    double pivot_val = tableau[pivot_row][pivot_col];

    // 基底変数の入れ替え（新しく入る変数のインデックスを記録）
    basis[pivot_row] = pivot_col;

    // ピボット行をピボットの値で割って、ピボット位置の値を 1.0 にする
    for (int j = 0; j < cols; ++j) {
        tableau[pivot_row][j] /= pivot_val;
    }

    // ピボット行以外のすべての行（目的関数行含む）から、ピボット列の要素を消去して 0.0 にする
    for (int i = 0; i < rows; ++i) {
        if (i != pivot_row) {
            double factor = tableau[i][pivot_col];
            for (int j = 0; j < cols; ++j) {
                tableau[i][j] -= factor * tableau[pivot_row][j];
            }
        }
    }
}

// 最適化を実行するメイン関数
SimplexSolver::Status SimplexSolver::solve(const LinearProgramModel& model) {
    // 1. 初期化
    initializeTableau(model);
    solver_status = Status::UNSOLVED;

    //std::cout << "\n[Initial State]";
    //printTableau();

    int iteration = 0;
    const int MAX_ITERATIONS = 1000; // 無限ループ（巡回）対策のセーフティ

    // 2. 単体法のメインループ
    while (iteration < MAX_ITERATIONS) {
        iteration++;
        
        // ピボットの選択
        auto [pivot_row, pivot_col] = selectPivot();

        // 終了判定
        if (pivot_row == -1 && pivot_col == -1) {
            solver_status = Status::OPTIMAL;
            break;
        }
        if (pivot_row == -1 && pivot_col != -1) {
            solver_status = Status::UNBOUNDED;
            break;
        }

        //std::cout << "\n[Iteration " << iteration << "] Pivot selected: Row " << pivot_row << ", Col " << pivot_col << "\n";
        
        // ピボット操作（行基本変形）の実行
        executePivot(pivot_row, pivot_col);
        
        // 毎ステップのタブローを表示（学習用に非常に役立ちます）
        //printTableau();
    }

    if (iteration >= MAX_ITERATIONS) {
        std::cout << "Warning: 最大反復回数に達しました（無限ループの可能性）。\n";
        solver_status = Status::ERROR_INVALID;
    }

    // 3. 結果の回収
    if (solver_status == Status::OPTIMAL) {
        int cols = num_total_vars + 1;
        // 最適目的関数値は最下行の右端
        optimal_value = tableau[num_constraints][cols - 1];

        // 最適解（変数の値）の復元
        optimal_solution.assign(num_original_vars, 0.0);
        for (int i = 0; i < num_constraints; ++i) {
            // 現在の基底変数が、スラック変数ではなく「元の変数」であれば値を回収
            if (basis[i] < num_original_vars) {
                optimal_solution[basis[i]] = tableau[i][cols - 1];
            }
        }
    }

    return solver_status;
}