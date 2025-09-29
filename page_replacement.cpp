/*
Page Replacement Simulator (C++)
Supports: FIFO, Optimal (OPT), LRU, Clock (Second Chance)

Usage:
  Compile: g++ -std=c++17 page_replacement.cpp -O2 -o page_sim
  Run:     ./page_sim

Input format (interactive):
  # of frames (integer)
  # of reference pages (integer)
  sequence of page numbers separated by spaces or newlines

Example:
  frames: 3
  n: 13
  seq: 7 0 1 2 0 3 0 4 2 3 0 3 2

The program prints step-by-step frames for each algorithm and the total page faults.
It also saves results to `results.txt`.

Notes for GitHub collaboration (add to README):
 - Each member should create a separate branch and make at least one meaningful commit
   (examples: improve output formatting, add comments, add unit tests, add sample inputs).
 - Include CONTRIBUTING.md with instructions how to run the program.

*/

#include <bits/stdc++.h>
using namespace std;

void print_step(const vector<int>& frames, int cur_page, int step, bool hit) {
    cout << "Step " << setw(2) << step << ": ref=" << setw(2) << cur_page << " | ";
    for (int f : frames) {
        if (f == INT_MIN) cout << " . ";
        else cout << setw(2) << f << " ";
    }
    cout << (hit ? "  (HIT)" : "  (FAULT)") << '\n';
}

int simulate_fifo(const vector<int>& seq, int frames_count, bool verbose=true) {
    vector<int> frames(frames_count, INT_MIN);
    queue<int> q;
    int faults=0;
    for (int i=0;i<(int)seq.size();++i) {
        int pg = seq[i];
        bool hit=false;
        for (int f: frames) if (f==pg) { hit=true; break; }
        if (!hit) {
            ++faults;
            if ((int)q.size() < frames_count) {
                // find first empty slot and push
                for (int j=0;j<frames_count;++j) if (frames[j]==INT_MIN) { frames[j]=pg; break; }
                q.push(pg);
            } else {
                int victim = q.front(); q.pop();
                // replace victim in frames
                for (int j=0;j<frames_count;++j) if (frames[j]==victim) { frames[j]=pg; break; }
                q.push(pg);
            }
        }
        if (verbose) print_step(frames, pg, i+1, hit);
    }
    return faults;
}

int simulate_opt(const vector<int>& seq, int frames_count, bool verbose=true) {
    vector<int> frames(frames_count, INT_MIN);
    int faults=0;
    for (int i=0;i<(int)seq.size();++i) {
        int pg = seq[i];
        bool hit=false;
        for (int f: frames) if (f==pg) { hit=true; break; }
        if (!hit) {
            ++faults;
            // if space available
            bool placed=false;
            for (int j=0;j<frames_count;++j) if (frames[j]==INT_MIN) { frames[j]=pg; placed=true; break; }
            if (!placed) {
                // choose victim with farthest next use (or never used again)
                int victim_idx=0; int farthest=-1;
                for (int j=0;j<frames_count;++j) {
                    int nextpos = -1;
                    for (int k=i+1;k<(int)seq.size();++k) if (seq[k]==frames[j]) { nextpos=k; break; }
                    if (nextpos==-1) { victim_idx=j; farthest = INT_MAX; break; }
                    if (nextpos>farthest) { farthest = nextpos; victim_idx=j; }
                }
                frames[victim_idx]=pg;
            }
        }
        if (verbose) print_step(frames, pg, i+1, hit);
    }
    return faults;
}
// Thuật toán LRU (Least Recently Used):
// - Thay thế trang ít được sử dụng nhất trong quá khứ gần đây.
// - Sử dụng biến thời gian (timestamp) hoặc stack/map để lưu lần truy cập cuối cùng.
int simulate_lru(const vector<int>& seq, int frames_count, bool verbose=true) {
    vector<int> frames(frames_count, INT_MIN);
    unordered_map<int,int> last_used; // page -> time
    int time=0; int faults=0;
    for (int i=0;i<(int)seq.size();++i) {
        ++time;
        int pg = seq[i];
        bool hit=false;
        for (int f: frames) if (f==pg) { hit=true; break; }
        if (hit) {
            last_used[pg]=time;
        } else {
            ++faults;
            bool placed=false;
            for (int j=0;j<frames_count;++j) if (frames[j]==INT_MIN) { frames[j]=pg; last_used[pg]=time; placed=true; break; }
            if (!placed) {
                // replace least recently used
                int lru_idx=0; int lru_time=INT_MAX;
                for (int j=0;j<frames_count;++j) {
                    int p = frames[j];
                    int t = last_used.count(p)? last_used[p] : 0;
                    if (t < lru_time) { lru_time = t; lru_idx = j; }
                }
                last_used.erase(frames[lru_idx]);
                frames[lru_idx]=pg;
                last_used[pg]=time;
            }
        }
        if (verbose) print_step(frames, pg, i+1, hit);
    }
    return faults;
}
// Thuật toán Clock (Second Chance):
// - Giả lập con trỏ xoay vòng qua các frame như kim đồng hồ.
// - Trang có bit sử dụng (use bit) = 1 thì được "cho cơ hội thứ hai" (reset về 0).
// - Nếu bit = 0 thì trang đó sẽ bị thay thế.
int simulate_clock(const vector<int>& seq, int frames_count, bool verbose=true) {
    vector<int> frames(frames_count, INT_MIN);
    vector<int> use(frames_count,0);
    int pointer=0; int faults=0;
    for (int i=0;i<(int)seq.size();++i) {
        int pg = seq[i];
        bool hit=false;
        for (int j=0;j<frames_count;++j) if (frames[j]==pg) { hit=true; use[j]=1; break; }
        if (!hit) {
            ++faults;
            // find victim
            while (true) {
                if (frames[pointer]==INT_MIN) {
                    frames[pointer]=pg; use[pointer]=1; pointer = (pointer+1)%frames_count; break;
                }
                if (use[pointer]==0) {
                    frames[pointer]=pg; use[pointer]=1; pointer = (pointer+1)%frames_count; break;
                } else {
                    use[pointer]=0; pointer = (pointer+1)%frames_count;
                }
            }
        }
        if (verbose) print_step(frames, pg, i+1, hit);
    }
    return faults;
}

int main(){
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    cout << "Page Replacement Simulator (C++)\n";
    int frames_count; cout << "Enter number of frames: ";
    if (!(cin >> frames_count)) return 0;
    int n; cout << "Enter number of references: "; cin >> n;
    vector<int> seq(n);
    cout << "Enter reference string ("<<n<<" integers): ";
    for (int i=0;i<n;++i) cin >> seq[i];

    cout << "\n--- FIFO Simulation ---\n";
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

    // Save summary to results.txt
    ofstream ofs("results.txt");
    ofs << "Frames: " << frames_count << "\n";
    ofs << "References: ";
    for (int x: seq) ofs << x << " "; ofs << "\n";
    ofs << "FIFO faults: " << f_fifo << "\n";
    ofs << "OPT faults: " << f_opt << "\n";
    ofs << "LRU faults: " << f_lru << "\n";
    ofs << "CLOCK faults: " << f_clock << "\n";
    ofs.close();

    cout << "Results written to results.txt\n";
    return 0;
}
