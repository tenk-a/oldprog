こちら
http://www.wht.mmtr.or.jp/~hirotsu/
の方が BeOS用ですが
ttfcnv_a ver0.01
という、sjisコードの ttf を unicode に変換する
ツールをソース付きで公開されております。
(ttfcnv_a.cpp ttfcnv_a.doc)

で、コマンドラインツールだし、ソース付ってことで
コンパイルしてみたところ、ちょっとの修正、
というより、Be.h が用意している型名と関数を
(マクロぽいが副作用を許容してくれてるので:-)
用意することでコンパイル＆実行できました。

つーことで、その偽 be.h です。

ソースや使い方については、配布元から取ってきて
この be.h と コンパイルバッチでコンパイルしてやって
ください。

※元のドキュメントにあるjisx0213code.txtについては
http://www.jca.apc.org/~earthian/aozora/0213.html
にある all ファイルを落としてきて中のを使いました。



