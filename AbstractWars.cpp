// C++11
#include <algorithm>
#include <cmath>
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <vector>

using namespace std;

const int PLAYER_ID = 0;

const int SIMULATION_TIME = 2010;
const int MAX_BASE_COUNT = 100;
const int S = 600;

int g_currentTime;
int g_baseCount;
int g_ownerCount;
int g_speed;
int g_baseTime[MAX_BASE_COUNT][MAX_BASE_COUNT];
bool g_attackCheck[MAX_BASE_COUNT][MAX_BASE_COUNT][SIMULATION_TIME];
bool g_troopCheck[S][S][SIMULATION_TIME];

double calcDist(int y1, int x1, int y2, int x2) {
    double dy = y1 - y2;
    double dx = x1 - x2;
    return sqrt(dy * dy + dx * dx);
}

struct Coord {
    int y;
    int x;

    Coord(int y, int x) {
        this->y = y;
        this->x = x;
    }
};

struct AttackLine {
    int source;
    int target;
    int beforeY;
    int beforeX;
    int arrivalTime;

    AttackLine(int source, int target, int beforeY, int beforeX, int arrivalTime) {
        this->source = source;
        this->target = target;
        this->beforeY = beforeY;
        this->beforeX = beforeX;
        this->arrivalTime = arrivalTime;
    }
};

vector <AttackLine> g_attackField[S][S];

struct Owner {
    double power;
    int baseCount;
    int totalAttackT;
    int attackUpdateCount;

    Owner() {
        this->power = -1.0;
        this->baseCount = -1;
        this->totalAttackT = 0;
        this->attackUpdateCount = 0;
    }

    int attackT() {
        if (this->attackUpdateCount == 0) {
            return -1;
        } else {
            return this->totalAttackT / this->attackUpdateCount;
        }
    }
};

vector <Owner> g_ownerList;

struct Base {
    int y;
    int x;
    int owner;
    int size;
    int growthRate;
    int sizeHistory[SIMULATION_TIME];
    int attackHistory[SIMULATION_TIME];

    Base() {
        this->y = -1;
        this->x = -1;
        this->owner = -1;
        this->size = -1;
        this->growthRate = -1;
        memset(this->sizeHistory, -1, sizeof(this->sizeHistory));
        memset(this->attackHistory, 0, sizeof(this->attackHistory));
    }

    void updateFutureSize() {
        int s = this->size;
        int attackT = (g_ownerList[this->owner].attackT() == -1) ? 1000 : g_ownerList[this->owner].attackT();
        bool occupy = false;

        for (int i = g_currentTime + 1; i < min(g_currentTime + 300, SIMULATION_TIME); i++) {
            s += this->growthRate + s / 100;
            s -= this->attackHistory[i];

            if (s < 0) {
                occupy = true;
            }

            if (s >= attackT) {
                s /= 2;
            }

            this->sizeHistory[i] = (occupy) ? -1 : s;
        }
    }
};

vector <Base> g_baseList;

struct Troop {
    int owner;
    int source;
    int target;
    int y;
    int x;
    int size;
    int arrivalTime;
    int created_at;

    Troop() {
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
        this->x = (int) x;
        this->y = (int) y;
    }

    string hash() {
        return to_string(this->owner) + ":" + to_string(this->y) + ":" + to_string(this->x) + ":" +
               to_string(this->size);
    }
};

class AbstractWars {
public:
    int B;
    vector<int> baseX, baseY;

    // ----------------------------------------------
    int init(vector<int> baseLocations, int speed) {
        g_baseCount = baseLocations.size() / 2;
        g_speed = speed;
        g_currentTime = 0;
        g_ownerCount = 0;

        fprintf(stderr, "B = %d, speed = %d\n", g_baseCount, g_speed);

        srand(123);
        B = baseLocations.size() / 2;
        for (int i = 0; i < B; ++i) {
            Base base;
            base.x = baseLocations[2 * i];
            base.y = baseLocations[2 * i + 1];
            g_baseList.push_back(base);
            baseX.push_back(baseLocations[2 * i]);
            baseY.push_back(baseLocations[2 * i + 1]);
        }

        for (int fromId = 0; fromId < g_baseCount; fromId++) {
            Base *from = getBase(fromId);
            for (int toId = fromId + 1; toId < g_baseCount; toId++) {
                Base *to = getBase(toId);
                double dist = calcDist(from->y, from->x, to->y, to->x);
                int T = ceil(dist / g_speed);

                g_baseTime[fromId][toId] = T;
                g_baseTime[toId][fromId] = T;
            }
        }

        for (int fromId = 0; fromId < g_baseCount; fromId++) {
            for (int targetId = 0; targetId < g_baseCount; targetId++) {
                if (fromId == targetId) continue;

                int T = g_baseTime[fromId][targetId];
                for (int t = 2; t < T; t++) {
                    Coord coord = updateTroopCoord(fromId, targetId, t);
                    Coord bcoord = updateTroopCoord(fromId, targetId, t - 1);
                    g_attackField[coord.y][coord.x].push_back(AttackLine(fromId, targetId, bcoord.y, bcoord.x, T - t));
                }
            }
        }

        return 0;
    }

    void updateBaseData(vector<int> &bases) {
        for (int i = 0; i < g_baseCount; i++) {
            Base *base = getBase(i);
            int ownerId = bases[2 * i];
            int size = bases[2 * i + 1];
            base->owner = ownerId;
            base->sizeHistory[g_currentTime] = size;

            if (g_currentTime == 1) {
                g_ownerCount = max(g_ownerCount, ownerId);
            }

            if (g_currentTime == 2) {
                base->growthRate = size - base->size;
            }

            if (g_currentTime > 1 && ownerId != PLAYER_ID && base->size > size) {
                Owner *owner = getOwner(ownerId);

                if (abs(base->size - 2 * size) <= 15) {
                    owner->totalAttackT += base->size;
                    owner->attackUpdateCount++;
                }
            }

            base->size = bases[2 * i + 1];

            if (g_currentTime > 1 && base->owner != PLAYER_ID) {
                base->updateFutureSize();
            }
        }

        if (g_currentTime == 1) {
            fprintf(stderr, "OwnerCount: %d\n", g_ownerCount + 1);

            for (int i = 0; i <= g_ownerCount; i++) {
                Owner owner;
                if (i == PLAYER_ID) {
                    owner.power = 1.0;
                }
                g_ownerList.push_back(owner);
            }
        }
    }

    void updateTroopData(vector<int> &troops) {
        int tsize = troops.size() / 4;

        if (g_currentTime <= 5) return;

        fprintf(stderr, "%4d: updateTroopData =>\n", g_currentTime);

        for (int i = 0; i < tsize; i++) {
            int owner = troops[4 * i];
            int size = troops[4 * i + 1];
            int x = troops[4 * i + 2];
            int y = troops[4 * i + 3];

            if (owner == PLAYER_ID) continue;

            g_troopCheck[y][x][g_currentTime] = true;
            vector <AttackLine> atl = g_attackField[y][x];
            int s = atl.size();

            for (int j = 0; j < s; j++) {
                AttackLine at = atl[j];
                if (g_baseList[at.target].owner != PLAYER_ID) continue;

                g_attackCheck[at.source][at.target][g_currentTime] = true;

                bool b1 = g_troopCheck[at.beforeY][at.beforeX][g_currentTime - 1];

                if (b1) {
                    fprintf(stderr, "Owner %d attack: %d -> %d (%d)\n", owner, at.source, at.target, size);
                }
            }
        }
    }

    Coord updateTroopCoord(int from, int to, int time) {
        assert(time > 0);

        int T = g_baseTime[from][to];
        double partMoved = time * 1.0 / T;
        double x = g_baseList[from].x + (g_baseList[to].x - g_baseList[from].x) * partMoved;
        double y = g_baseList[from].y + (g_baseList[to].y - g_baseList[from].y) * partMoved;

        return Coord((int) y, (int) x);
    }

    // ----------------------------------------------
    vector<int> others;

    // picks a random base to attack based on distance to the opponent bases: the closer the base, the higher the chances are
    int getRandomBase(int sourceInd) {
        Base *source = getBase(sourceInd);
        vector<double> probs(others.size());
        double sp = 0;
        int targetId = -1;
        int minDist = INT_MAX;

        for (int i = 0; i < (int) others.size(); ++i) {
            int ind = others[i];
            Base *base = getBase(ind);
            probs[i] = 1 / (pow(baseX[sourceInd] - baseX[ind], 2) + pow(baseY[sourceInd] - baseY[ind], 2));
            sp += probs[i];
            double dist = calcDist(source->y, source->x, base->y, base->x);
            int T = g_baseTime[sourceInd][ind];
            int osize = g_baseList[ind].sizeHistory[min(g_currentTime + T, SIMULATION_TIME)];

            if (osize < 0) continue;

            if (minDist > dist && osize < source->size * 0.5) {
                minDist = dist;
                targetId = ind;
            }
        }

        return targetId;

        double r = rand() * 1.0 / RAND_MAX * sp;
        double s = 0;
        for (int i = 0; i < (int) others.size(); ++i) {
            s += probs[i];
            if (s >= r) {
                return others[i];
            }
        }
        return others[others.size() - 1];
    }

    // ----------------------------------------------
    vector<int> sendTroops(vector<int> bases, vector<int> troops) {
        g_currentTime++;
        updateBaseData(bases);
        updateTroopData(troops);
        // compile the list of bases owned by other players
        others.resize(0);
        for (int i = 0; i < B; ++i)
            if (bases[2 * i] != 0)
                others.push_back(i);
        if (others.size() == 0) {
            // noone to fight!
            return vector<int>(0);
        }

        vector<int> att;
        for (int i = 0; i < B; ++i) {
            if (bases[2 * i] == 0 && bases[2 * i + 1] >= 1000) {
                // send troops to a random base of different ownership
                int targetId = getRandomBase(i);

                if (targetId != -1) {
                    att.push_back(i);
                    att.push_back(targetId);

                    int arrivalTime = min(g_currentTime + g_baseTime[i][targetId], SIMULATION_TIME);
                    g_baseList[targetId].attackHistory[arrivalTime] += g_baseList[i].size / 2;
                }
            }
        }
        return att;
    }

    Base *getBase(int id) {
        return &g_baseList[id];
    }

    Owner *getOwner(int id) {
        return &g_ownerList[id];
    }
};
// -------8<------- end of solution submitted to the website -------8<-------

template<class T>
void getVector(vector <T> &v) { for (int i = 0; i < v.size(); ++i) cin >> v[i]; }

int main() {
    AbstractWars aw;
    int N;
    cin >> N;
    vector<int> baseLoc(N);
    getVector(baseLoc);
    int S;
    cin >> S;
    int retInit = aw.init(baseLoc, S);
    cout << retInit << endl;
    cout.flush();
    for (int st = 0; st < 2000; ++st) {
        int B;
        cin >> B;
        vector<int> bases(B);
        getVector(bases);
        int T;
        cin >> T;
        vector<int> troops(T);
        getVector(troops);
        vector<int> ret = aw.sendTroops(bases, troops);
        cout << ret.size() << endl;
        for (int i = 0; i < (int) ret.size(); ++i) { cout << ret[i] << endl; }
        cout.flush();
    }
}
