// C++11
#include <algorithm>
#include <cmath>
#include <cassert>
#include <map>
#include <cstdlib>
#include <iostream>
#include <string.h>
#include <limits.h>
#include <vector>

using namespace std;

const int PLAYER_ID = 0;

const int SIMULATION_TIME = 2000;
const int PERSON_CAP = 1000;
const int MAX_BASE_COUNT = 100;
const int S = 600;

int g_currentTime;
int g_baseCount;
int g_ownerCount;
int g_speed;
int g_baseTime[MAX_BASE_COUNT][MAX_BASE_COUNT];
bool g_troopCheck[S][S][SIMULATION_TIME + 10];

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

struct AttackData {
    int owner;
    int size;

    AttackData(int owner, int size) {
        this->owner = owner;
        this->size = size;
    }
};

vector <AttackLine> g_attackField[S][S];

struct Owner {
    int baseCount;
    int unitSize;
    int totalAttackT;
    int attackUpdateCount;
    double totalPower;
    int powerCount;

    Owner() {
        this->baseCount = -1;
        this->unitSize = -1;
        this->totalAttackT = 0;
        this->attackUpdateCount = 0;
        this->totalPower = 0.0;
        this->powerCount = 0;
    }

    int attackT() {
        if (this->attackUpdateCount == 0) {
            return -1;
        } else {
            return this->totalAttackT / this->attackUpdateCount;
        }
    }

    double power() {
        if (this->powerCount == 0) {
            return 1.0;
        } else {
            return this->totalPower / this->powerCount;
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
    int attackedTime;
    int sizeHistory[SIMULATION_TIME + 10];
    int ownerHistory[SIMULATION_TIME + 10];
    vector <AttackData> attackHistory[SIMULATION_TIME + 10];

    Base() {
        this->y = -1;
        this->x = -1;
        this->owner = -1;
        this->size = -1;
        this->growthRate = -1;
        this->attackedTime = -1;
        memset(this->sizeHistory, -1, sizeof(this->sizeHistory));
    }

    void updateFutureSize() {
        int s = this->size;
        int o = this->owner;
        int attackT = (g_ownerList[this->owner].attackT() == -1) ? 1000 : g_ownerList[this->owner].attackT();

        for (int i = g_currentTime + 1; i < min(g_currentTime + 300, SIMULATION_TIME); i++) {
            s += this->growthRate + s / 100;

            for (int j = 0; j < this->attackHistory[i].size(); j++) {
                AttackData at = this->attackHistory[i][j];

                int size = at.size * (g_ownerList[at.owner].power() / g_ownerList[o].power());

                if (o == at.owner) {
                    s += size;
                } else {
                    s -= size;
                }

                if (s < 0) {
                    o = at.owner;
                    attackT = (g_ownerList[o].attackT() == -1) ? 1000 : g_ownerList[o].attackT();
                    s *= -1;
                }
            }

            s = min(s, PERSON_CAP);

            if (s >= attackT) {
                s /= 2;
            }

            this->sizeHistory[i] = s;
            this->ownerHistory[i] = o;
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

vector <Troop> g_enemyTroopList;

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
            base->ownerHistory[g_currentTime] = ownerId;

            if (g_currentTime == 1) {
                g_ownerCount = max(g_ownerCount, ownerId);
            }

            if (g_currentTime == 2) {
                base->growthRate = size - base->size;
            }

            base->size = bases[2 * i + 1];

            if (g_currentTime > 1) {
                base->updateFutureSize();
            }
        }

        if (g_currentTime == 1) {
            fprintf(stderr, "OwnerCount: %d\n", g_ownerCount + 1);

            for (int i = 0; i <= g_ownerCount; i++) {
                Owner owner;
                g_ownerList.push_back(owner);
            }
        }

        for (int sourceId = 0; sourceId < g_baseCount; sourceId++) {
            Base *source = getBase(sourceId);

            for (int targetId = 0; targetId < g_baseCount; targetId++) {
                Base *target = getBase(targetId);
            }
        }
    }

    void updateTroopData(vector<int> &troops) {
        int tsize = troops.size() / 4;

        if (g_currentTime <= 5) return;

        map<string, bool> troopMap;

        for (int i = 0; i < g_enemyTroopList.size(); i++) {
            Troop *troop = &g_enemyTroopList[i];
            troop->updateStep();
            troopMap[troop->hash()] = true;

            if (g_currentTime == troop->arrivalTime) {
                Base *target = getBase(troop->target);

                if (target->owner != PLAYER_ID) continue;

                if (target->ownerHistory[g_currentTime] == target->ownerHistory[g_currentTime - 1]) {
                    int diff = target->growthRate + target->sizeHistory[g_currentTime - 1] / 100 +
                               target->sizeHistory[g_currentTime - 1] - target->sizeHistory[g_currentTime];
                    double power = diff / (double) troop->size;

                    if (1.0 <= power && power <= 1.2) {
                        g_ownerList[troop->owner].totalPower += power;
                        g_ownerList[troop->owner].powerCount++;

                        // fprintf(stderr, "%4d: Owner %d: power = %f\n", g_currentTime, troop->owner, g_ownerList[troop->owner].power());
                    }
                }
            }
        }

        g_enemyTroopList.erase(std::remove_if(g_enemyTroopList.begin(), g_enemyTroopList.end(),
                                              [](Troop t) { return g_currentTime == t.arrivalTime; }),
                               g_enemyTroopList.end());

        for (int i = 0; i < tsize; i++) {
            Troop troop;
            troop.owner = troops[4 * i];
            troop.size = troops[4 * i + 1];
            troop.x = troops[4 * i + 2];
            troop.y = troops[4 * i + 3];
            g_ownerList[troop.owner].totalAttackT += 2 * troop.size;
            g_ownerList[troop.owner].attackUpdateCount++;

            if (troop.owner == PLAYER_ID) continue;
            if (troopMap[troop.hash()]) {
                continue;
            }

            g_troopCheck[troop.y][troop.x][g_currentTime] = true;
            vector <AttackLine> atl = g_attackField[troop.y][troop.x];
            int s = atl.size();

            for (int j = 0; j < s; j++) {
                AttackLine at = atl[j];

                if (!g_troopCheck[at.beforeY][at.beforeX][g_currentTime - 1]) continue;

                Base *base = getBase(at.target);

                if (atl.size() == 2) {
                    troop.source = at.source;
                    troop.target = at.target;
                    troop.arrivalTime = g_currentTime + at.arrivalTime;
                    troop.created_at = g_currentTime - (g_baseTime[at.source][at.target] - at.arrivalTime);

                    if (troop.arrivalTime <= SIMULATION_TIME) {
                        g_enemyTroopList.push_back(troop);
                        g_baseList[troop.target].attackHistory[troop.arrivalTime].push_back(
                                AttackData(troop.owner, troop.size));

                        if (base->attackedTime >= g_currentTime) {
                            base->attackedTime = min(base->attackedTime, troop.arrivalTime);
                        } else {
                            base->attackedTime = troop.arrivalTime;
                        }
                    }
                    // fprintf(stderr, "%4d: Owner %d attack: %d -> %d (%d)\n", g_currentTime, troop.owner, at.source, at.target, troop.size);
                }
            }
        }
    }

    void updateOwnerData() {
        for (int i = 0; i <= g_ownerCount; i++) {
            Owner *owner = getOwner(i);
            owner->unitSize = 0;
        }

        for (int i = 0; i <= g_baseCount; i++) {
            Base *base = getBase(i);

            g_ownerList[base->owner].unitSize += base->size;
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
        int targetId = -1;
        int minDist = INT_MAX;
        bool warning = (source->attackedTime >= g_currentTime && source->attackedTime - g_currentTime <= 30);

        for (int i = 0; i < (int) others.size(); ++i) {
            int ind = others[i];
            Base *base = getBase(ind);
            double dist = calcDist(source->y, source->x, base->y, base->x);
            int T = g_baseTime[sourceInd][ind];
            int owner = g_baseList[ind].ownerHistory[min(g_currentTime + T, SIMULATION_TIME)];

            if (owner == PLAYER_ID) continue;
            if (g_currentTime <= 40 && T > 50) continue;
            int osize = g_baseList[ind].sizeHistory[min(g_currentTime + T, SIMULATION_TIME)];

            if (minDist > dist && (!warning || osize < source->size * 0.5)) {
                minDist = dist;
                targetId = ind;
            }
        }

        return targetId;
    }

    // ----------------------------------------------
    vector<int> sendTroops(vector<int> bases, vector<int> troops) {
        g_currentTime++;
        updateBaseData(bases);
        updateTroopData(troops);
        updateOwnerData();
        // compile the list of bases owned by other players
        others.resize(0);
        for (int i = 0; i < B; ++i) {
            if (bases[2 * i] != PLAYER_ID) {
                others.push_back(i);
            }
        }

        if (others.size() == 0) {
            return vector<int>(0);
        }

        vector<int> att;
        for (int i = 0; i < B; ++i) {
            Base *base = getBase(i);
            if (base->size < 2) continue;
            if (base->owner != PLAYER_ID) continue;

            int size = base->size;

            for (int j = 0; j < base->attackHistory[g_currentTime + 1].size(); j++) {
                AttackData at = base->attackHistory[g_currentTime + 1][j];

                if (at.owner == PLAYER_ID) {
                    size += at.size;
                } else {
                    size -= at.size;
                }
            }

            if (base->owner == PLAYER_ID && (size > 991 - base->growthRate || g_currentTime <= 40)) {
                // send troops to a random base of different ownership
                int targetId = getRandomBase(i);

                if (targetId != -1) {
                    att.push_back(i);
                    att.push_back(targetId);

                    int arrivalTime = min(g_currentTime + g_baseTime[i][targetId], SIMULATION_TIME);
                    g_baseList[targetId].attackHistory[arrivalTime].push_back(AttackData(PLAYER_ID, base->size / 2));
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
