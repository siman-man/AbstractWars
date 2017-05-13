// C++11
#include <algorithm>
#include <cmath>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

using namespace std;

const int PLAYER_ID = 0;

const int MAX_B = 100;

int g_currentTime;
int g_baseCount;
int g_speed;
int g_baseTime[MAX_B][MAX_B];

double calcDist(int y1, int x1, int y2, int x2) {
  double dy = y1 - y2;
  double dx = x1 - x2;
  return sqrt(dy*dy + dx*dx);
}

struct Base {
    int y;
    int x;
    int owner;
    int size;
    int growthRate;

    Base () {
      this->y = -1;
      this->x = -1;
      this->owner = -1;
      this->size = -1;
      this->growthRate = -1;
    }
};

vector<Base> g_baseList;

struct Troop {
    int owner;
    int source;
    int target;
    int y;
    int x;
    int size;
    int arrivalTime;
    int created_at;

    Troop () {
      this->owner = -1;
      this->source = -1;
      this->target = -1;
      this->y = -1;
      this->x = -1;
      this->size = -1;
      this->arrivalTime = -1;
      this->created_at = -1;
    }

    void updateStep() {
      if (this->arrivalTime == g_currentTime) {
        return;
      }
      // if the troop has not arrived yet, approximate its position based on time it moved
      double partMoved = (g_currentTime - this->created_at) * 1.0 / (this->arrivalTime - this->created_at);
      double x = g_baseList[this->source].x + (g_baseList[this->target].x - g_baseList[this->source].x) * partMoved;
      double y = g_baseList[this->source].y + (g_baseList[this->target].y - g_baseList[this->source].y) * partMoved;
      this->x = (int)x;
      this->y = (int)y;
    }

    string hash() {
        return to_string(this->owner) + ":" + to_string(this->y) + ":" + to_string(this->x) + ":" + to_string(this->size);
    }
};

class AbstractWars {
public:
    int B;
    vector<int> baseX, baseY;
    // ----------------------------------------------
    int init(vector <int> baseLocations, int speed) {
        g_baseCount = baseLocations.size() / 2;
        g_speed = speed;
        g_currentTime = 0;

        fprintf(stderr, "B = %d, speed = %d\n", g_baseCount, g_speed);

        srand(123);
        B = baseLocations.size() / 2;
        for (int i = 0; i < B; ++i) {
            Base base;
            base.x = baseLocations[2*i];
            base.y = baseLocations[2*i+1];
            g_baseList.push_back(base);
            baseX.push_back(baseLocations[2*i]);
            baseY.push_back(baseLocations[2*i+1]);
        }

        for (int fromId = 0; fromId < g_baseCount; fromId++) {
          Base *from = getBase(fromId);
          for (int toId = fromId+1; toId < g_baseCount; toId++) {
            Base *to = getBase(toId);
            double dist = calcDist(from->y, from->x, to->y, to->x);
            int T = ceil(dist / g_speed);

            g_baseTime[fromId][toId] = T;
            g_baseTime[toId][fromId] = T;
          }
        }

        return 0;
    }

    void updateBaseData(vector<int> &bases) {
        for (int i = 0; i < g_baseCount; i++) {
            Base* base = getBase(i);
            base->owner = bases[2*i];

            if (g_currentTime == 2) {
              base->growthRate = bases[2*i+1] - base->size;
            }

            base->size = bases[2*i+1];
        }
    }

    void updateTroopData(vector<int> &troops) {
        int tsize = troops.size() / 4;

        for (int i = 0; i < tsize; i++) {
            int owner = troops[4*i];
            int size = troops[4*i+1];
            int x = troops[4*i+2];
            int y = troops[4*i+3];
        }
    }
    // ----------------------------------------------
    vector<int> others;
    // picks a random base to attack based on distance to the opponent bases: the closer the base, the higher the chances are
    int getRandomBase(int sourceInd) {
        vector<double> probs(others.size());
        double sp = 0;
        for (int i = 0; i < (int)others.size(); ++i) {
            int ind = others[i];
            probs[i] = 1 / (pow(baseX[sourceInd] - baseX[ind], 2) + pow(baseY[sourceInd] - baseY[ind], 2));
            sp += probs[i];
        }

        double r = rand() * 1.0/RAND_MAX * sp;
        double s = 0;
        for (int i = 0; i < (int)others.size(); ++i) {
            s += probs[i];
            if (s >= r)
                return others[i];
        }
        return others[others.size() - 1];
    }
    // ----------------------------------------------
    vector <int> sendTroops(vector <int> bases, vector <int> troops) {
        g_currentTime++;
        updateBaseData(bases);
        updateTroopData(troops);
        // compile the list of bases owned by other players
        others.resize(0);
        for (int i = 0; i < B; ++i)
            if (bases[2*i] != 0)
                others.push_back(i);
        if (others.size() == 0) {
            // noone to fight!
            return vector<int>(0);
        }

        vector<int> att;
        for (int i = 0; i < B; ++i) {
            if (bases[2*i] == 0 && bases[2*i+1] >= 1000) {
                // send troops to a random base of different ownership
                att.push_back(i);
                att.push_back(getRandomBase(i));
            }
        }
        return att;
    }

    Base *getBase(int id) {
        return &g_baseList[id];
    }
};
// -------8<------- end of solution submitted to the website -------8<-------

template<class T> void getVector(vector<T>& v) { for (int i = 0; i < v.size(); ++i) cin >> v[i]; }

int main() {
    AbstractWars aw;
    int N; cin >> N;
    vector<int> baseLoc(N);
    getVector(baseLoc);
    int S; cin >> S;
    int retInit = aw.init(baseLoc, S);
    cout << retInit << endl; cout.flush();
    for (int st = 0; st < 2000; ++st) {
        int B; cin >> B;
        vector<int> bases(B);
        getVector(bases);
        int T; cin >> T;
        vector<int> troops(T); getVector(troops);
        vector<int> ret = aw.sendTroops(bases, troops);
        cout << ret.size() << endl;
        for (int i = 0; i < (int)ret.size(); ++i) { cout << ret[i] << endl; }
        cout.flush();
    }
}
