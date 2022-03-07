#/bin/bash

echo "letreco with secrets"
time ./build/native/botle evaluate \
  --allowed-guesses-file data/letreco-words.dict \
  --possible-secrets-file data/letreco-secrets.dict \
  --max-words 1000000 \
  --cache-max-size 100000000 \
  --write-solutions-dir www/public/cache \
  --solutions-max-words 200 "$@"

echo "letreco wo secrets"
time ./build/native/botle evaluate \
  --allowed-guesses-file data/letreco-words.dict \
  --max-words 1000000 \
  --cache-max-size 100000000 \
  --write-solutions-dir www/public/cache \
  --solutions-max-words 200 "$@"

echo "letreco with secrets"
time ./build/native/botle evaluate \
  --allowed-guesses-file data/letreco-words.dict \
  --possible-secrets-file data/letreco-secrets.dict \
  --hard-mode \
  --max-words 1000000 \
  --cache-max-size 100000000 \
  --write-solutions-dir www/public/cache \
  --solutions-max-words 200 "$@"

echo "letreco wo secrets"
time ./build/native/botle evaluate \
  --allowed-guesses-file data/letreco-words.dict \
  --max-words 1000000 \
  --hard-mode \
  --cache-max-size 100000000 \
  --write-solutions-dir www/public/cache \
  --solutions-max-words 200 "$@"
