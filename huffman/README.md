# ハフマン符号の実装例

* `freq.c`: テキストファイル（英文）から頻度情報を生成する
  `cc -o freq freq.c` で実行ファイル `freq` を生成し，
  `freq -r Earth.txt` などで，頻度情報が出力できる．
  
* `hcode_freq.c`: テキストファイル（英文）からハフマン木を構築
  `cc -o hcode_freq hcode_freq.c` で実行ファイルを生成できる．
  `hcode_freq -r simple.txt` などで文字頻度からハフマン木を構築できる


  
