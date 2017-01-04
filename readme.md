# MachiKaniaで動画再生テスト
## 必要ファイル

1.dist/default/production/*.hex 
実行ファイルです。コピーしなさい。
2.video
動画本体です。
3.music.raw
MUSIC FILE

## 注意
2.3.はルートに置きなさい。

## 仕様
video
- 256x196
- 29.97fps

music
- 32ksps
- 1ch
- 8bits
です。

Made By Gombe



## おまけの動画作成方法

**この説明の環境はLinuxベースですが**Windows on bashでもバイナリそのままで行けると思います。読み替えればWindowsのDOSだっけ？でも動くと思います。ソースコードのコンパイルが必要です。ffmpegもsoxもimagemagickも入れられると思います。
Windows on Bashの方は以下から入れてください。Cygwinでも行けるかなあ。でも遅いから非推奨。これらの処理は時間がかかります。

```
	sudo apt-get install ffmpeg sox imagemagick
```

- 1.連続画像を作成する。
動画から切り出すときは`ffmpeg`を使うと良いでしょう。
-rでフレームレートを指定します。データサイズが半端じゃないほど大きいのでpngを使ったほうがいいと思います。数分で10GBとかザラにあります。SSDユーザーなので辛いです。
``` sh
ffmpeg -i <動画パス> -r 29.97 -f image2 <outputの書式(ex. bmps/frame%05d.png)>
```

- 2.画像を減色する
好きなソフトをお使いください。ただし`BMP3`かつ`無圧縮ファイル`,`16色`かつ`4ビットカラー`の解像度が`256*xxx`のみ対応します。
```
mkdir bmps/converted -p && for f in bmps/*.png; do convert $f -resize 256x -colors 16 -depth 4 -bpp=4 -define bmp:format=bmp3 -compress none bmps/converted/$(basename ${f%.*}).bmp; done
```
Makefileとか書けば並列化して高速に処理できます。そうすればだいぶ早く終わると思います。現状だとこの作業にかなり時間がかかります。これは最適化パレットを検索しているためだと思います。

 - 3. パレット、データ抽出と合成
(bmp2raw:In this project)を使います。ソートしたい人はソートしてね。多分標準でもうすでにされていると思う(コラ
Windowsってリダイレクトってあったっけ？なければファイルに書き込むように変更してください。

```
cd bmpconverter
./bmp2raw ../bmps/converted/*.bmp > video
```

 - 4. SDに移動
生成したファイル`video`をルートに移動してください。

 - 5.音声データの作成
 rawデータ、32000ksps、1ch、8bitの`music.raw`を作ってください。以下作成例(雑
 ```
 sox --norm=0 tiruno.mp3 -r32000 -b8 -u output.raw
 ```
 作成したファイルは以下のコマンドで再生できます。
```
aplay output.raw -r32000 -c1
```
