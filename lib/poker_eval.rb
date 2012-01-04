# support multiple ruby version (fat binaries under windows)
begin
  require 'poker_eval_api'
end

# define version string to be used internally for the Gem by Hoe.
class PokerEval
  module GemVersion
    VERSION = '0.0.1'
  end

  def self.best args
    results = self.eval_hand(args)
    results["combination"].each_with_index do |i, index|
      if index > 0
        results["combination"][index] = self.card2string(i.to_i)
      end
    end

    return results
  end

  def self.winner args
    index2index = {}
    normalized_pockets = []
    normalized_index = 0
    pockets = args["pockets"]

    pockets.each_with_index do |pocket, index|
      if !args["fill_pockets"]
        if pocket.find_index("__")
          pockets[index] = []
        end
      end

      if !pockets[index].empty?
        normalized_pockets << pockets[index]
        index2index[index] = normalized_index
        normalized_index += 1
      end
    end

    args["pockets"] = normalized_pockets
    results = self.eval(args)


    count = results["info"]["samples"]
    haslopot = results["info"]["haslopot"]
    hashipot = results["info"]["hashipot"]

    winners = { 'low' => [], 'hi' => [] }

    pockets.each_with_index do |pocket, index|
      if !index2index[index].nil?
        result = results["eval"][index2index[index]]
        if result["winhi"] == 1 or result["tiehi"] == 1
            winners["hi"] << index
        end
        if result["winlo"] == 1 || result["tielo"] == 1
          winners["low"] << index
        end
      end
    end

    if !haslopot || winners["low"].empty?
      winners.delete("low")
    end

    if !hashipot
      winners.delete("hi")
    end

    return winners

  end
end
