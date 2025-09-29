/*
Page Replacement Simulator (C++)
Supports: FIFO, Optimal (OPT), LRU, Clock (Second Chance)

Usage:
  Compile (g++): g++ -std=c++17 page_replacement.cpp -O2 -o page_sim
  Run:           ./page_sim

Example input:
  frames: 3
  n: 13
  seq: 7 0 1 2 0 3 0 4 2 3 0 3 2
*/

#include <iostream>
#include <vector>
#include <queue>
#include <unordered_map>
#include <fstream>
#include <iomanip>
#include <climits>

using namespace std;

void print_step(const vector<int>& frames, int cur_page, int step, bool hit) {
    cout << "Step " << setw(2) << step << ": ref=" << setw(2) << cur_page << " | ";
    for (int f : frames) {
        if (f == INT_MIN) cout << " . ";
        else cout << setw(2) << f << " ";
    }
    cout << (hit ? "  (HIT)" : "  (FAULT)") << '\n';
}

int simulate_fifo(const vector<int>& seq, int frames_count, bool verbose = true) {
    vector<int> frames(frames_count, INT_MIN);
    queue<int> q;
    int faults = 0;
    for (int i = 0;i < (int)seq.size();++i) {
        int pg = seq[i];
        bool hit = false;
        for (int f : frames) if (f == pg) { hit = true; break; }
        if (!hit) {
            ++faults;
            if ((int)q.size() < frames_count) {
                for (int j = 0;j < frames_count;++j)
                    if (frames[j] == INT_MIN) { frames[j] = pg; break; }
                q.push(pg);
            }
            else {
                int victim = q.front(); q.pop();
                for (int j = 0;j < frames_count;++j)
                    if (frames[j] == victim) { frames[j] = pg; break; }
                q.push(pg);
            }
        }
        if (verbose) print_step(frames, pg, i + 1, hit);
    }
    return faults;
}

int simulate_opt(const vector<int>& seq, int frames_count, bool verbose = true) {
    vector<int> frames(frames_count, INT_MIN);
    int faults = 0;
    for (int i = 0;i < (int)seq.size();++i) {
        int pg = seq[i];
        bool hit = false;
        for (int f : frames) if (f == pg) { hit = true; break; }
        if (!hit) {
            ++faults;
            bool placed = false;
            for (int j = 0;j < frames_count;++j)
                if (frames[j] == INT_MIN) { frames[j] = pg; placed = true; break; }
            if (!placed) {
                int victim_idx = 0; int farthest = -1;
                for (int j = 0;j < frames_count;++j) {
                    int nextpos = -1;
                    for (int k = i + 1;k < (int)seq.size();++k)
                        if (seq[k] == frames[j]) { nextpos = k; break; }
                    if (nextpos == -1) { victim_idx = j; farthest = INT_MAX; break; }
                    if (nextpos > farthest) { farthest = nextpos; victim_idx = j; }
                }
                frames[victim_idx] = pg;
            }
        }
        if (verbose) print_step(frames, pg, i + 1, hit);
    }
    return faults;
}

int simulate_lru(const vector<int>& seq, int frames_count, bool verbose = true) {
    vector<int> frames(frames_count, INT_MIN);
    unordered_map<int, int> last_used;
    int time = 0; int faults = 0;
    for (int i = 0;i < (int)seq.size();++i) {
        ++time;
        int pg = seq[i];
        bool hit = false;
        for (int f : frames) if (f == pg) { hit = true; break; }
        if (hit) {
            last_used[pg] = time;
        }
        else {
            ++faults;
            bool placed = false;
            for (int j = 0;j < frames_count;++j)
                if (frames[j] == INT_MIN) { frames[j] = pg; last_used[pg] = time; placed = true; break; }
            if (!placed) {
                int lru_idx = 0; int lru_time = INT_MAX;
                for (int j = 0;j < frames_count;++j) {
                    int p = frames[j];
                    int t = last_used.count(p) ? last_used[p] : 0;
                    if (t < lru_time) { lru_time = t; lru_idx = j; }
                }
                last_used.erase(frames[lru_idx]);
                frames[lru_idx] = pg;
                last_used[pg] = time;
            }
        }
        if (verbose) print_step(frames, pg, i + 1, hit);
    }
    return faults;
}

int simulate_clock(const vector<int>& seq, int frames_count, bool verbose = true) {
    vector<int> frames(frames_count, INT_MIN);
    vector<int> use(frames_count, 0);
    int pointer = 0; int faults = 0;
    for (int i = 0;i < (int)seq.size();++i) {
        int pg = seq[i];
        bool hit = false;
        for (int j = 0;j < frames_count;++j)
            if (frames[j] == pg) { hit = true; use[j] = 1; break; }
        if (!hit) {
            ++faults;
            while (true) {
                if (frames[pointer] == INT_MIN) {
                    frames[pointer] = pg; use[pointer] = 1; pointer = (pointer + 1) % frames_count; break;
                }
                if (use[pointer] == 0) {
                    frames[pointer] = pg; use[pointer] = 1; pointer = (pointer + 1) % frames_count; break;
                }
                else {
                    use[pointer] = 0; pointer = (pointer + 1) % frames_count;
                }
            }
        }
        if (verbose) print_step(frames, pg, i + 1, hit);
    }
    return faults;
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    ifstream fin("input.txt");
    if (!fin.is_open()) {
        cerr << "Không mở được file input.txt\n";
        return 1;
    }

    int frames_count, n;
    fin >> frames_count >> n;
    vector<int> seq(n);
    for (int i = 0;i < n;++i) fin >> seq[i];
    fin.close();

    // Hien thi du lieu dau vao bang
    cout << "\n=== DU LIEU DAU VAO ===\n";
    cout << left << setw(20) << "So khung trang" << ": " << frames_count << "\n";
    cout << left << setw(20) << "So tham chieu" << ": " << n << "\n";

    cout << left << setw(20) << "Chuoi tham chieu" << ": ";
    for (int x : seq) {
        cout << setw(3) << x;  // Moi so chiem 3 ky tu de thang hang
    }
    cout << "\n";
    cout << string(50, '=') << "\n\n";

    // Chay mo phong
    cout << "--- FIFO Simulation ---\n";
    int f_fifo = simulate_fifo(seq, frames_count, true);
    cout << "Total page faults (FIFO): " << f_fifo << "\n\n";

    cout << "--- OPTIMAL Simulation ---\n";
    int f_opt = simulate_opt(seq, frames_count, true);
    cout << "Total page faults (OPT): " << f_opt << "\n\n";

    cout << "--- LRU Simulation ---\n";
    int f_lru = simulate_lru(seq, frames_count, true);
    cout << "Total page faults (LRU): " << f_lru << "\n\n";

    cout << "--- CLOCK Simulation ---\n";
    int f_clock = simulate_clock(seq, frames_count, true);
    cout << "Total page faults (CLOCK): " << f_clock << "\n\n";

    // Save summary
    ofstream ofs("results.txt");
    ofs << "Frames: " << frames_count << "\n";
    ofs << "References: ";
    for (int x : seq) ofs << x << " "; ofs << "\n";
    ofs << "FIFO faults: " << f_fifo << "\n";
    ofs << "OPT faults: " << f_opt << "\n";
    ofs << "LRU faults: " << f_lru << "\n";
    ofs << "CLOCK faults: " << f_clock << "\n";
    ofs.close();

    cout << "Results written to results.txt\n";
    return 0;
}
