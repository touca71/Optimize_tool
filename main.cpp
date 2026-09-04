#include <bits/stdc++.h>
#include "LinearProgramModel.h"
#include "SimplexSolver.h"
#include <chrono>

using namespace std;
const bool integral = true;
double best_Z = -std::numeric_limits<double>::max(); 
std::vector<double> best_solution;


bool is_all_integer(const std::vector<double>& sol) {
    for (double val : sol) {
        if (std::abs(val - 0.0) > 1e-3 && std::abs(val - 1.0) > 1e-3) {
            return false; 
        }
    }
    return true;
}



void dfs(const LinearProgramModel& base_model, std::vector<int> fixed_vals) {
    int n = base_model.getNumVariables();

    // --- 1. 現在の固定状況をモデルに反映する ---
    LinearProgramModel current_model = base_model;
    for (int i = 0; i < n; ++i) {
        if (fixed_vals[i] == 0) {
            std::vector<double> c(n, 0.0);
            c[i] = 1.0;
            current_model.addConstraint(c, 0.0);
        }
        else if (fixed_vals[i] == 1) {
            auto obj = current_model.getObjectiveCoefficients();
            obj[i] += 999999.0; 
            current_model.setObjective(obj);
        }
    }

    // --- 2. 単体法（線形計画緩和）の実行 ---
    SimplexSolver solver;
    SimplexSolver::Status status = solver.solve(current_model);

    if (status != SimplexSolver::Status::OPTIMAL) {
        return; 
    }

    // 正しい目的関数値 Z の計算（1に固定しているボーナス分を引く）
    double current_Z = solver.getOptimalValue();
    for (int i = 0; i < n; ++i) {
        if (fixed_vals[i] == 1) current_Z -= 999999.0;
    }

    // 強力な枝刈り（すでに暫定最良値以下なら即リターン）
    if (current_Z <= best_Z) {
        return;
    }

    std::vector<double> current_sol = solver.getOptimalSolution();

    // 整数解が見つかった場合の処理
    if (is_all_integer(current_sol)) {
        best_Z = current_Z;
        best_solution = current_sol;
        return; 
    }

    // --- 3. 【最速化の肝】次に分岐させる最適な変数（最も小数に近いもの）を選ぶ ---
    int next_branch_var = -1;
    double min_diff_from_half = 1.0; // 0.5からの距離（最大0.5）

    for (int i = 0; i < n; ++i) {
        // すでに固定済みの変数はスキップ
        if (fixed_vals[i] != -1) continue;

        // 0.5 に最も近い実数解を持つ変数を探す
        double diff = std::abs(current_sol[i] - 0.5);
        if (diff < min_diff_from_half) {
            min_diff_from_half = diff;
            next_branch_var = i;
        }
    }

    // もし未固定の変数が残っていない場合は終了（通常は上のis_all_integerで引っかかります）
    if (next_branch_var == -1) {
        return;
    }

    // --- 4. 選択した変数 next_branch_var で分枝（子ノードの探索） ---
    
    // パターンA: 0 に固定する枝
    fixed_vals[next_branch_var] = 0;
    dfs(base_model, fixed_vals);
    fixed_vals[next_branch_var] = -1; // 状態を戻す（バックトラック）

    // パターンB: 1 に固定する枝
    fixed_vals[next_branch_var] = 1;
    dfs(base_model, fixed_vals);
    fixed_vals[next_branch_var] = -1; // 状態を戻す
}

void LinearPrograming(){
    LinearProgramModel model;
    int n,m; cin >> n >> m ;
    double ans; cin >> ans;
    vector<double> obj(n);
    for(int i = 0; i < n; i++) cin >> obj[i];
    model.setObjective(obj);
    vector<vector<double>> cst(m);
    vector<double> rhs(m);
    for(int i = 0; i < m; i++){
        vector<double> c(n);
        for(int j = 0; j < n; j++) cin >> c[j];
        cst[i] = c;
    }
    for(int i = 0; i < m; i++) cin >> rhs[i];
    for(int i = 0; i < m; i++){
        model.addConstraint(cst[i], rhs[i]);
    }
    model.printModel();

    SimplexSolver solver;
    solver.solve(model);
    vector<double> A = solver.getOptimalSolution();
    for(int i = 0; i < n; i++){
        cout << A[i] << " " ;
    }
}


void IntegralPrograming() {
    LinearProgramModel model;
    int n, m; 
    if (!(std::cin >> n >> m)) return;
    
    double ans; 
    std::cin >> ans;

    // 1. 目的関数の入力
    std::vector<double> obj(n);
    for (int i = 0; i < n; i++) std::cin >> obj[i];
    model.setObjective(obj);

    // 2. 元の制約式の入力 (サイズは m で十分です)
    std::vector<std::vector<double>> cst(m);
    std::vector<double> rhs(m);
    for (int i = 0; i < m; i++) {
        std::vector<double> c(n);
        for (int j = 0; j < n; j++) std::cin >> c[j];
        cst[i] = c;
    }
    for (int i = 0; i < m; i++) std::cin >> rhs[i];

    // 元の制約を追加
    for (int i = 0; i < m; i++) {
        model.addConstraint(cst[i], rhs[i]);
    }

    // 0-1問題の境界条件 x_i <= 1.0 を追加
    for (int i = 0; i < n; i++) {
        std::vector<double> c(n, 0.0);
        c[i] = 1.0;
        model.addConstraint(c, 1.0);
    }

    // ==========================================
    // グローバル変数のリセットと初期整数解（暫定解）の作成
    // ==========================================
    best_solution.assign(n, 0.0);
    best_Z = -std::numeric_limits<double>::max(); // 初期状態は極小値に

    SimplexSolver init_solver;
    init_solver.solve(model);
    
    if (init_solver.getStatus() == SimplexSolver::Status::OPTIMAL) {
        std::vector<double> A = init_solver.getOptimalSolution();
        
        // 【修正】小数の値を正しく四捨五入して、最初の「仮の整数解（実行可能解）」を作る
        for (int i = 0; i < n; i++) {
            best_solution[i] = (A[i] > 0.5) ? 1.0 : 0.0;
        }
        
        // 最初の暫定解の目的関数値を計算
        best_Z = 0.0;
        for (int i = 0; i < n; i++) {
            best_Z += best_solution[i] * obj[i];
        }
    }

    // ==========================================
    // DFS（分枝限定法）の実行
    // ==========================================
    // すべて「-1 (未固定)」で初期化した、サイズ n の固定状態ベクトルを作成
    std::vector<int> initial_fixed_vals(n, -1);

    auto start = std::chrono::high_resolution_clock::now();
    
    // 【修正】新しい設計のDFSを呼び出し（depth 0 は不要）
    dfs(model, initial_fixed_vals);
    
    auto end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    // ==========================================
    // 結果の出力
    // ==========================================
    std::cout << "変数: " << n << "個, 制約式: " << m << "個\n";
    std::cout << "処理時間: " << elapsed << " ms\n";
    
    for (int i = 0; i < n; i++) {
        // 表示を綺麗にするため、0.5基準で0か1に丸めて出力
        int output_val = (best_solution[i] > 0.5) ? 1 : 0;
        std::cout << output_val << " ";
    }
    std::cout << "\n";

    double opt_value = 0.0;
    for (int i = 0; i < n; i++) {
        int output_val = (best_solution[i] > 0.5) ? 1 : 0;
        opt_value += output_val * obj[i];
    }
    std::cout << opt_value << "\n\n";
}

int main(){
    if(integral){
        IntegralPrograming();
    }else{
        LinearPrograming();
    }
}