## AbstractWars

You are playing an abstract wargame against one or several AI opponents.

あなたは他のAIと共に擬似的な戦争ゲームを行ってもらいます

The landscape is represented by a plane which contains B bases at the given points.

平野が与えられ幾つかのポイントには基地が設置されています

Each base is occupied by a certain number of units and is controlled by one of the players.

各基地はそれぞれのプレイヤーに占領されています。

The players can perform only one kind of action: pick a base controlled by this player,

プレイヤーは1種類のアクションしか行動できません、基地を選択し管理下に置く。

create a troop of half of the units which occupy that base and send this troop to another base.

その基地を占めるユニットの半分の部隊を作り、この部隊を別の基地に送る。

The troops move in a straight line between their base of origin and target base and don't

軍隊の行動は直線的に移動を行い、途中に境涯物や敵と接近しても何も起こりません。

interact with any other entities they might encounter on the way. Upon troop arrival to the target base,

軍隊が目的の基地に到着したら。2つのうち1つのアクションが発生します。

one of two scenarios occurs. If the target base is controlled by the same player as the troop,

到着した基地が自軍の場合、ユニットに合流を行います。

the units of the troop join the units of the base. If the target base is controlled by a different player,

もし異なるプレイヤーの基地の場合は、戦闘が発生します。

a battle occurs, in which either the units of the base win and eliminate the troop or the troop overpowers the base and the base changes ownership.

互いの戦闘力を比較し、より大きい値の軍が勝利します。基地は勝利した軍の管理下におかれます。

Your task is to maximize the percentage of units controlled by you throughout the game (see Scoring section for an exact definition).

あなたの目標はこのゲームを通してより多くの基地を管理下に置くことです。



## 実装

あなたには `init` および `sendTroops` を実装してもらいます

* baseLocations: 基地の位置が格納されています。

speed gives you the speed of troop movements. If the distance between the base of origin and the target base is D,

and the troop is created at time step T, it will arrive to the target base at time step T+ceil(D/speed).

* speed:  軍隊の移動速度が与えられます。目的地までの距離が D で軍隊の生成が T 時間に行われた場合、到着時刻は T + ceil(D/speed) となります。

`sendTroops` は以下の通りです　

* bases: 各基地の情報を提供します [基地を管理しているID, 基地に存在している軍隊の数]

* troops: 軍隊の情報を表します [どちらの軍かを示すID, 軍隊の数, y座標, x座標]

*

You can use Server.getRemainingTime library method to get the remaining time in milliseconds.

あなたは `Server.getRemainingTime` を使用することで残り時間を把握することが出来ます

This is the time that was left before your solution was called last, and the result will only change



when sendTroops method is called again. You can pass any integer to this method. Please note that

there is some overhead in calling this method which will be part of your 15 seconds time limit.

It is advised not to call this method within every sendTroops call.


## シミュレーション

全部で 2000 stepあります。

### 開始フェーズ

The number of units on each base is increased depending on growth rate for that base and the current number of units on

各基地の軍隊の数を増加させます。増加数は「growthRate + numberOfUnits/100」で計算されます。もし、軍隊の数が1000を超えた場合は、1000に減らされます。

it: an extra (growthRate + numberOfUnits / 100) units are added. If the number of units on any base exceeds 1000,

it is reduced to 1000. Coordinates of troop positions are updated based on their progress towards their target bases: an exact position is rounded to the nearest integer coordinates.


### 移動フェーズ

各軍目標に向かって行動を行います。

### 戦闘フェーズ

各プレイヤーは "戦闘力" を持っています。プレイヤーは 1.0 で固定ですが、敵は 1.0 - 1.2 の範囲でランダムに決められます。軍隊の力は "戦闘力" * "軍隊の数" で算出されます。



### テストケース生成

* 基地の数 (B) は 20 - 100 の間から選択されます

* 基地の座標は 1 - 598 からランダムで選択されます

* それぞれの基地の軍隊の数は 1 - 10 の間から選択されます

* growthRate の値は 1 - 3 の間から選択されます

* speedの値は 1 - 3 の間から選択されます

* 敵の数 (NOpp) は 1 - 4 の間から選択されます

* 各基地の初期管理者はランダムで決定されます

* ビジュアライザのソースを確認してどのように生成されるのかを調べてください


### 注意点

各テストケースの制限時間は 15 秒です


## 戦略

- [x] 基地の初期化を行う
- [x] 毎ターンの基地情報の更新
- [x] 軍隊情報を構造化
- [ ] どの軍隊がどこに向かっているかを調べる
- [ ] 軍隊の情報を更新する
- [ ] 基地の情報を更新する


## 考察

基地によって軍隊の増加率が異なる
複数の敵が存在する場合に、どの順番で倒すべきか
同時期に同じ基地から軍隊を生成することは可能
1 step で生成出来る軍隊の数は最大で B (マップ上に存在する基地の数)
