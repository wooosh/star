(declare-project
  :name "www"
  :description ```build system for wooo.sh ```
  :version "0.0.0")

(declare-source
  :prefix "www"
  :source ["build.janet"]
  :dependencies [:spork "https://github.com/wooosh/janet-openssl-hash"])