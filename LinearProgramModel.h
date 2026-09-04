#ifndef LINEAR_PROGRAM_MODEL_H
#define LINEAR_PROGRAM_MODEL_H

#include <vector>
#include <string>

class LinearProgramModel {
private:
    // 目的関数の係数 (例: 3x_1 + 5x_2 なら [3.0, 5.0])
    std::vector<double> objective_coefficients;

    // 制約条件の左辺の係数行列 (行: 制約式の数, 列: 変数の数)
    // 例: 2x_1 + x_2 <= 4, x_1 + 2x_2 <= 5 なら [[2.0, 1.0], [1.0, 2.0]]
    std::vector<std::vector<double>> constraint_matrix;

    // 制約条件の右辺の値 (定数項) (例: [4.0, 5.0])
    std::vector<double> constraint_rhs;

    // 変数と制約の数を管理
    int num_variables;
    int num_constraints;

public:
    // コンストラクタ（初期状態では変数・制約ともに0）
    LinearProgramModel();

    // 目的関数を設定する関数
    void setObjective(const std::vector<double>& coefficients);

    // 制約条件を1つ追加する関数 (係数のベクトル と 右辺の定数)
    void addConstraint(const std::vector<double>& coefficients, double rhs);

    // ゲッター関数（Solverクラスがデータを参照するために必要）
    int getNumVariables() const { return num_variables; }
    int getNumConstraints() const { return num_constraints; }
    const std::vector<double>& getObjectiveCoefficients() const { return objective_coefficients; }
    const std::vector<std::vector<double>>& getConstraintMatrix() const { return constraint_matrix; }
    const std::vector<double>& getConstraintRhs() const { return constraint_rhs; }

    // モデルの情報をコンソールに綺麗に表示するデバッグ用関数
    void printModel() const;
};

#endif // LINEAR_PROGRAM_MODEL_H