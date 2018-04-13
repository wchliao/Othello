# Othello

<img src="/pic/cover.png" title="Othello" width="300px" height="300px">

## Introduction

An Othello program implemented in C++.

The core search method is Monte-Carlo tree search.

Plus, open books, bitboard, and fast magic number mapping are used to optimize the search speed.

## Usage

Simply execute `make`, and the Othello program `Othello` will be generated.

Subsequently, execute `Othello`, and then you can enjoy the game.

## Parameters

In `OTP.h`, there are several parameters that can be tuned.

* `UCB_c`: the constant `c` used in Upper Confidence Bound (UCB)

* `simulateN`: the number of samples per simulation

* `OpenBookDepth`: the maximum depth for open book

* `SearchDepth`: the maximum search depth

* `SearchTime`: the maximum search time (in secs)

* `TotalTimeLimit`: the time limit for a player in one game (in secs)
