#### How to install clang-format-22 on Ubuntu

clang-format-18+ has some new features. Such as .clang-format-ignore.

The default apt repository may not contain the clang-format-22.
So you cannot use apt install to install the clang-format-22.

To install clang-format-22:

wget https://apt.llvm.org/llvm.sh
chmod u+x llvm.sh
sudo ./llvm.sh 22

Then run apt search clang-format, you will be able to see the clang-format-22.
Then run:
sudo apt install clang-format-22
