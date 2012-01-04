#!/usr/local/bin/ruby
# $Id: test.rb 244 2009-02-01 08:43:39Z tommy $

require "test/unit"
require 'ostruct'
require "poker_eval"

class TC_PokerEval < Test::Unit::TestCase
  def test_eval()
    pockets = [["tc", "ac"],  ["th", "ah"],  ["8c", "6h"]]
    board = ["7h","3s", "2c", "7s", "7d"]
    game = "holdem"
    result = PokerEval.eval({"game"=>game, "pockets"=>pockets, "board"=>board,  "iterations"=>10000})
    expect = {"info"=>{"samples"=>10000, "haslopot"=>0, "hashipot"=>1}, "eval"=>[{"scoop"=>0, "winhi"=>0, "losehi"=>0, "tiehi"=>10000, "winlo"=>0, "loselo"=>0, "tielo"=>0, "ev"=>500}, {"scoop"=>0, "winhi"=>0, "losehi"=>0, "tiehi"=>10000, "winlo"=>0, "loselo"=>0, "tielo"=>0, "ev"=>500}, {"scoop"=>0, "winhi"=>0, "losehi"=>10000, "tiehi"=>0, "winlo"=>0, "loselo"=>0, "tielo"=>0, "ev"=>0}]}
    assert_equal(result, expect);
  end

  def test_best()
    hand = ["Ac", "As", "Td", "7s", "7h", "3s", "2c"]
    side = "hi"
    result = PokerEval.best({"side"=>side, "hand"=>hand})
    expect = {"value"=>34363392, "combination"=>["TwoPair", "As", "Ac", "7s", "7h", "Td"]}
    assert_equal(result, expect);
  end
end
