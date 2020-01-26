# コマンドラインオプションの説明

選択肢から選ぶものは、断りがない限り一番最初がデフォルト。

## 入出力

### 入力

`-i input.png`

 - 対応フォーマット
   - 1/2/4/8bit Gray
   - 8/16bit RGB
   - alpha channel対応（ただし現状無視される）
### 出力

`-o output.avif`

## メタデータ

### 切り抜き・回転・反転

適用される順番は「切り抜き→回転→反転」

#### 切り抜き

`--crop-size widthN/widthD,heightN/heightD`  
`--crop-offset horizOffN/horizOffD,vertOffN/vertOffD`

表示時に切り抜くサイズとオフセットを分数で指定する。

デフォルトでは指定されたサイズで中心から切り抜き、offsetが指定されたら、その分移動したところから切り抜く。

例：`--offset-size 1000/3,1000/7`（333.3x142.9で中心から切り抜く）

#### 回転

`--rotation [0, 90, 180, 270]`

表示時に回転する。反時計回り。

#### 反転

`--mirror [vertical, horizontal]`

表示時に反転する

## AV1 シーケンシャルヘッダ

### 静止画ヘッダー

`--full-still-picture-header`

このオプションを指定すると、静止画前提のAV1のシーケンスヘッダーが出力されることになっている（出力されなかった）。もし意図通りに動けば、3バイトぐらいヘッダが短くなる。

## プロファイル

### AV1 シーケンス・プロファイル

`--profile (0=base, 1=high, 2=professional)`

AV1のプロファイルを指定する。[プロファイルごとに使えるピクセルフォーマットとbit depthが異なる](https://aomediacodec.github.io/av1-spec/#sequence-header-obu-semantics)。

<table class="table table-sm table-bordered">
  <thead>
    <tr>
      <th>seq_profile</th>
      <th>Bit depth</th>
      <th>Monochrome support</th>
      <th>Chroma subsampling</th>
    </tr>
  </thead>
  <tbody>
    <tr>
      <td>0</td>
      <td>8 or 10</td>
      <td>Yes</td>
      <td>YUV 4:2:0</td>
    </tr>
    <tr>
      <td>1</td>
      <td>8 or 10</td>
      <td>No</td>
      <td>YUV 4:4:4</td>
    </tr>
    <tr>
      <td>2</td>
      <td>8 or 10</td>
      <td>Yes</td>
      <td>YUV 4:2:2</td>
    </tr>
    <tr>
      <td>2</td>
      <td>12</td>
      <td>Yes</td>
      <td>YUV 4:2:0, YUV 4:2:2, YUV 4:4:4</td>
    </tr>
  </tbody>
</table>

### ピクセルフォーマット

`--pix-fmt [yuv420, yuv422, yuv444]`

出力される画像のピクセルフォーマット。

使えるビット深度とピクセルフォーマットとプロファイルの組み合わせに制限があるので注意。カラー画像をモノクロにするときも、それぞれのLumaの間引き方で結果が変わるので注意。

### ビット深度

`--bit-depth [8, 10, 12]`

出力される画像のビット深度。

使えるビット深度とピクセルフォーマットとプロファイルの組み合わせに制限があるので注意。

### 色域

`--disable-full-color-range`
`--enable-full-color-range`

例えば通常の8bitのYUVのフォーマットでは、Yの値として16-235、UとVの値として16-240しか使わないが、このフラグをenableにすると0-255のすべてを使うようになる。10/12ビットでも同様。デフォルトではfalse。

YUVからRGBへの変換方法を定めているRec.2020などを読んでも扱いが書いておらず、根拠がよくわからない。

## 速度と品質のトレードオフ

### エンコーダの用途

`--encoder-usage [good, realtime]`

エンコーダのモードを指定する。goodの方が遅いが画質はよく、realtimeの方が速いが画質はおざなり。

### 利用スレッド数

`--threads <num-threads>`

変換時に使うスレッド数を指定する。何も指定しないと理論コア数。ただし実際にマルチスレッドされている気配がない。

### row based multi-threading of encoder

`--row-mt`

行単位でマルチスレッドでエンコードしてくれるようになるらしい。これを使うと`--threads`の意味が出てくるのかもしれない。ただし、画質に対して何かしらの影響があるかもしれない。デフォルトはオフ。

### cpu-used

`--cpu-used [0-8]`

小さくすればするほど、CPUを犠牲にして画質を上げようとする。

## ビットレート制御

### レートコントロール・アルゴリズム

`--rate-control [q, cq]`

出力される画像のファイルサイズの制御方法を指定する。

 - q: 品質を固定
 - cq: 品質を守りつつ、次で指定するbit rate限界まで品質を上げる（ただし努力目標？）。

### CQ Level

`--crf [0-63]`

qとcqで守らせたい品質を指定する。値が低いほど画質はよい。

### ビットレート

`--bit-rate <kilo-bits per second>`

`--rate-control cq`で守らせるビットレート。1秒の動画という扱いにしているので、出力されるファイルはここで指定した`kilo-bits`を上回らない…はずだが、努力目標っぽい。

### チューニング・メトリクス

`--tune [psnr|ssim|cdef-dist|daala-dist]`

エンコーダが画質を最適するためにパラメータをチューニングするときに、どの指標をつかって画質を評価するか指定する。

PSNRとSSIMは有名なのでググってください。CDEF-distはよくわからない。daalaはAV1の前身だがdaala-distが何なのかはよくわからない。

ちょろっと試した感じでは正直どれでチューニングしても五十歩百歩だった。

### ロスレスモード

`--lossless`

このフラグをつけると、ロスレスモードでエンコードする。アルファチャンネルはこのほうがよいのでは？

## Pre process

### モノクロ画像

`--monochrome`

モノクロで出力する。エンコーダが色差信号を無視する（モノクロにする）だけなので、入力画像はモノクロでなくてもよい。

### Sharpness

`--sharpness [0-7] (default = 0)`

たぶん、上げれば上げるほどシャープになる。ただしおすすめは0とのこと。

## Post process

### CDEF

`--disable-cdef`
`--enable-cdef`

主観画質を上げるためのポストプロセス・フィルタである[https://arxiv.org/abs/1602.05975](CDEF)を有効にするかどうか決める。デコード時に適用され、無視できないぐらい重い。

デフォルトでdisable。

### Loop Restoration Filter

## Coding parameter

### タイル分割

`--tile-rows [0-6]` `--tile-colums [0-6]`

画像をそれぞれ `pow(2, <tile-rows>)`, `pow(2, <tile-colums>)`個の画像に分割して独立してエンコード・デコードする。

デフォルトではどちらも0で、１枚の画像として扱う。

### スーパーブロックサイズ

`--superblock-size [dynamic, 128, 64]`

AV1では、画像をまずすべて同じ大きさのスーパーブロックに分割してから、その後それぞれのスーパーブロックを再帰的に分割して符号化していく。その大本のスーパーブロックのサイズを指定する。

dynamicを指定すると、短辺が480ピクセル以上の時128x128、それ以下のときは64x64のスーパーブロックで分割する。

### 
