git add .
git commit -m "$2"
git push https://hairmachine:$1@github.com/HairMachine/megarogue master

ssh macair@139.59.173.86 << EOF
  cd gendev/megarogue
  git pull origin master
  make
  git add out.bin
  git commit -m "Automated build commit"
  git push https://hairmachine:$1@github.com/HairMachine/megarogue master
  exit
EOF

git pull origin master
/./Applications/Kega\ Fusion.app/Contents/MacOS/Kega\ Fusion out.bin