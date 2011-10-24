#!/usr/local/bin/ruby
# $Id: test.rb 244 2009-02-01 08:43:39Z tommy $

require "test/unit"
require 'ostruct'


require "poker_eval"
t = PokerEval.new       
# x = t.eval({"game"=>"holdem", "pockets"=>[["Qc","Jh"],["9h","6c"],["4c","4h"],["6h","8c"]], "board"=>["Ac","Kc"],  "iterations" => 1000})
x = t.eval({"game"=>"holdem", "pockets"=>[["tc", "ac"],  ["th", "ah"],  ["8c", "6h"]], "board"=>["7h",
        "3s", "2c", "7s", "7d"],  "iterations" => 10000})
print x["info"]
print "\n"
for i in x["eval"]
  print i
  print "\n"
end

