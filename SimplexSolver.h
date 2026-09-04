#ifndef SIMPLEX_SOLVER_H
#define SIMPLEX_SOLVER_H

#include <vector>
#include "LinearProgramModel.h"

class SimplexSolver {
public:
    // ソルバーの計算状態を表す列挙型
    enum class Status {
        UNSOLVED,       // 未計算
        OPTIMAL,        // 最適解を発見
        UNBOUNDED,      // 解が無限大（有界でない）
        ERROR_INVALID   // モデルのエラーなど
    };

private:
    // シンプレックス・タブロー（拡張行列）
    // 行数 = 制約式の数 + 1（目的関数行）
    // 列数 = 元の変数 + スラック変数 + 右辺(b)
    std::vector<std::vector<double>> tableau;

    int num_original_vars; // 元の変数（x1, x2...）の数
    int num_slack_vars;    // スラック変数（s1, s2...）の数
    int num_total_vars;    // 総変数数（オリジナル + スラック）
    int num_constraints;   // 制約式の数

    // 各行にどの変数が基底変数（現在選択されている変数）として入っているかを記録する配列
    // サイズ = 制約式の数
    std::vector<int> basis;

    Status solver_status;  // 現在のステータス
    double optimal_value;  // 最適目的関数値 (Z)
    std::vector<double> optimal_solution; // 最適解（各変数の値）

    // --- 内部補助関数（カプセル化のため private に配置） ---
    
    // 1. モデルから初期シンプレックス・タブローを組み立てる
    void initializeTableau(const LinearProgramModel& model);

    // 2. ピボット（ピボット列・行）を選択する。戻り値は {pivot_row, pivot_col}。
    // もし最適なら行・列に -1 を返す。有界でないなら列だけ選んで行に -1 を返す。
    std::pair<int, int> selectPivot() const;

    // 3. 指定されたピボット（行・列）を中心に、行基本変形（掃き出し法）を行う
    void executePivot(int pivot_row, int pivot_col);

public:
    // コンストラクタ
    SimplexSolver();

    // 最適化を実行するメイン関数
    Status solve(const LinearProgramModel& model);

    // ゲッター関数
    double getOptimalValue() const { return optimal_value; }
    const std::vector<double>& getOptimalSolution() const { return optimal_solution; }
    Status getStatus() const { return solver_status; }

    // デバッグ用：現在のタブローの状態を綺麗にコンソール表示する
    void printTableau() const;
};

#endif // SIMPLEX_SOLVER_H