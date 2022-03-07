#/bin/bash

function run() {

  echo "wordle with secrets"
  time ./build/native/botle evaluate \
    --allowed-guesses-file data/en-words.dict \
    --possible-secrets-file data/en-secret-words.dict \
    --max-words 1000000 \
    --cache-max-size 400000000 \
    --write-solutions-dir www/public/cache \
    --solutions-max-words 200 "$@"
  
  echo "termooo with secrets"
  time ./build/native/botle evaluate \
    --allowed-guesses-file data/pt-words.dict \
    --possible-secrets-file data/pt-secret-words.dict \
    --max-words 1000000 \
    --cache-max-size 400000000 \
    --write-solutions-dir www/public/cache \
    --solutions-max-words 200 "$@"
  
  echo "wordle wo secrets"
  time ./build/native/botle evaluate \
    --allowed-guesses-file data/en-words.dict \
    --max-words 1000000 \
    --cache-max-size 400000000 \
    --write-solutions-dir www/public/cache \
    --solutions-max-words 200 "$@"
  
  echo "termoo wo secrets"
  time ./build/native/botle evaluate \
    --allowed-guesses-file data/pt-words.dict \
    --max-words 1000000 \
    --cache-max-size 400000000 \
    --write-solutions-dir www/public/cache \
    --solutions-max-words 200 "$@"
  
  echo "xingo wo secrets"
  time ./build/native/botle evaluate \
    --allowed-guesses-file data/xingo-words.dict \
    --max-words 1000000 \
    --cache-max-size 400000000 \
    --write-solutions-dir www/public/cache \
    --solutions-max-words 200 "$@"
  
  echo "wiki 2k wo secrets"
  time ./build/native/botle evaluate \
    --allowed-guesses-file data/en-wiki-2k.dict \
    --max-words 1000000 \
    --cache-max-size 400000000 \
    --write-solutions-dir www/public/cache \
    --solutions-max-words 200 "$@"
  
  echo "wiki 4k wo secrets"
  time ./build/native/botle evaluate \
    --allowed-guesses-file data/en-wiki-4k.dict \
    --max-words 1000000 \
    --cache-max-size 400000000 \
    --write-solutions-dir www/public/cache \
    --solutions-max-words 200 "$@"
  
  echo "wiki 10k wo secrets"
  time ./build/native/botle evaluate \
    --allowed-guesses-file data/en-wiki-10k.dict \
    --max-words 1000000 \
    --cache-max-size 400000000 \
    --write-solutions-dir www/public/cache \
    --solutions-max-words 200 "$@"
}

echo "Running easy mode"
run

echo "Running hard mode"
run --hard-mode

